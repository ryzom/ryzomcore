// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "stdnet.h"
#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/inet_address.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/net/module_socket.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLNET
{

	//////////////////////////////////////
	// Module interceptor implementation
	//////////////////////////////////////
	IModuleInterceptable::IModuleInterceptable()
		: _Registrar(NULL)
	{
	}

	IModuleInterceptable::~IModuleInterceptable()
	{
		if (_Registrar != NULL)
			_Registrar->unregisterInterceptor(this);
	}

	void IModuleInterceptable::registerInterceptor(IInterceptorRegistrar *registrar)
	{
		nlassert(registrar != NULL);

		_Registrar = registrar;

		_Registrar->registerInterceptor(this);
	}

	void IModuleInterceptable::interceptorUnregistered(IInterceptorRegistrar *registrar)
	{
		nlassert(registrar == _Registrar);

		_Registrar = NULL;
	}

	IInterceptorRegistrar *IModuleInterceptable::getRegistrar()
	{
		return _Registrar;
	}


	//////////////////////////////////////
	// Module factory implementation
	//////////////////////////////////////

	IModuleFactory::IModuleFactory(const std::string &moduleClassName)
		: _ModuleClassName(moduleClassName)
	{
	}

	IModuleFactory::~IModuleFactory()
	{
		// Delete any module that still exist
		while (!_ModuleInstances.empty())
		{
			CRefPtr<IModule> sanityCheck(*(_ModuleInstances.begin()));

			IModuleManager::getInstance().deleteModule(sanityCheck);

			// container is cleared by deleteModule (see below)
			// make sure the module is effectively destroyed
			// NB : is the code assert here, this mean that some user code
			// (or eventualy NeL code) have kept a smart pointer on the module
			// and this is bad. All smart pointer MUST be released when the
			// factory is about to be removed.
			nlassertex(sanityCheck == NULL, ("Some code have kept pointer on module '%s'", sanityCheck->getModuleName().c_str()));
		}

		// if the context is still active
		if (INelContext::isContextInitialised() && IModuleManager::isInitialized())
			// This factory is no longer available
			IModuleManager::getInstance().unregisterModuleFactory(this);
	}

	const std::string &IModuleFactory::getModuleClassName() const
	{
		return _ModuleClassName;
	}


	void IModuleFactory::deleteModule(IModule *module)
	{
		set<TModulePtr>::iterator it(_ModuleInstances.find(module));
		nlassert(it != _ModuleInstances.end());

		CRefPtr<IModule> sanityCheck(module);

		// removing this smart ptr must release the module
		_ModuleInstances.erase(it);

		nlassert(sanityCheck == NULL);
	}

	void IModuleFactory::registerModuleInFactory(TModulePtr module)
	{
		nlassert(module != NULL);

		nlassert(_ModuleInstances.find(module) == _ModuleInstances.end());

		// keep track of the module
		_ModuleInstances.insert(module);

		module->setFactory(this);
	}

	//////////////////////////////////////
	// CModuleTask implementation
	//////////////////////////////////////
	CModuleTask::CModuleTask (class CModuleBase * /*module */)
		: _FailInvoke(false)
	{
//		module->queueModuleTask(this);
	}


	void CModuleTask::initMessageQueue(CModuleBase * /* module */)
	{
	}

	void CModuleTask::flushMessageQueue(CModuleBase *module)
	{
		// process any queued message
		while (!module->_SyncMessages.empty())
		{
			IModuleProxy *proxy = module->_SyncMessages.front().first;
			CMessage &msg = module->_SyncMessages.front().second;

			module->_onProcessModuleMessage(proxy, msg);

			module->_SyncMessages.pop_front();
		}
	}

	void CModuleTask::processPendingMessage(CModuleBase *module)
	{
		flushMessageQueue(module);
	}


	//////////////////////////////////////
	// Module base implementation
	//////////////////////////////////////

	CModuleBase::CModuleBase()
		: _CurrentSender(NULL),
		  _CurrentMessage(NULL),
		  _CurrentMessageFailed(false),
		  _MessageDispatchTask(NULL),
		  _ModuleFactory(NULL),
		  _ModuleId(INVALID_MODULE_ID)
	{
		// register module itself in the interceptor list
		IModuleInterceptable::registerInterceptor(this);
	}

	CModuleBase::~CModuleBase()
	{
		// deleting a module from it's own current task is forbiden
		nlassert(_ModuleTasks.empty()
			||  CCoTask::getCurrentTask() != _ModuleTasks.front());

		// terminate and release any pending module task
		while (!_ModuleTasks.empty())
		{
			CModuleTask *task = _ModuleTasks.front();

			// deleting the task will waiting it to terminate
			delete task;

			_ModuleTasks.erase(_ModuleTasks.begin());
		}

		if (_MessageDispatchTask)
		{
			// delete reception task
			delete _MessageDispatchTask;
		}

		// unregister all interceptors
		while (!_ModuleInterceptors.empty())
		{
			IModuleInterceptable *interceptor = *(_ModuleInterceptors.begin());
			unregisterInterceptor(interceptor);
		}
	}

	void CModuleBase::registerInterceptor(IModuleInterceptable *interceptor)
	{
		// check that this interceptor not already registered
		nlassert(find(_ModuleInterceptors.begin(), _ModuleInterceptors.end(), interceptor) == _ModuleInterceptors.end());

		// insert the interceptor in the list
		_ModuleInterceptors.push_back(interceptor);
	}

	void CModuleBase::unregisterInterceptor(IModuleInterceptable *interceptor)
	{
		TInterceptors::iterator it = find(_ModuleInterceptors.begin(), _ModuleInterceptors.end(), interceptor);
		nlassert(it != _ModuleInterceptors.end());

		_ModuleInterceptors.erase(it);

		interceptor->interceptorUnregistered(this);
	}

	TModuleId			CModuleBase::getModuleId() const
	{
		return _ModuleId;
	}

	const std::string	&CModuleBase::getModuleName() const
	{
		return _ModuleName;
	}

	const std::string	&CModuleBase::getModuleClassName() const
	{
		return _ModuleFactory->getModuleClassName();
	}

	const std::string	&CModuleBase::getModuleFullyQualifiedName() const
	{
		if (_FullyQualifedModuleName.empty())
		{
			nlassertex(!_ModuleName.empty(), ("Call to CModuleBase::getModuleFullyQualifiedName before module name have been set (did you call from module constructor ?)"));
			// build the name
			string hostName;
			if (IService::isServiceInitialized())
				hostName = IService::getInstance()->getHostName();
			else
				hostName = ::NLNET::CInetAddress::localHost().hostName();
//			int pid = ::getpid();

			_FullyQualifedModuleName = IModuleManager::getInstance().getUniqueNameRoot()+":"+_ModuleName;
		}

		return _FullyQualifedModuleName;
	}

	std::string	CModuleBase::getModuleManifest() const
	{
		string manifest;

		// call each interceptor in order to build the manifest
		TInterceptors::const_iterator first(_ModuleInterceptors.begin()), last(_ModuleInterceptors.end());
		for (; first != last; ++first)
		{
			IModuleInterceptable *interceptor = *first;

			manifest += interceptor->buildModuleManifest() + " ";
		}

		if (!manifest.empty() && manifest[manifest.size()-1] == ' ')
			manifest.resize(manifest.size()-1);

		return manifest;
	}

	void	CModuleBase::onReceiveModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
	{
		H_AUTO(CModuleBase_onReceiveModuleMessage);

		if (!_ModuleTasks.empty())
		{
			// there is a task running, queue in the message
			_SyncMessages.push_back(make_pair(senderModuleProxy, message));
		}
		else
		{
			// go in user code for processing
			if (_MessageDispatchTask != NULL)
			{
				// process the message in the co task
				_CurrentSender = senderModuleProxy;
				_CurrentMessage = &message;

				_MessageDispatchTask->resume();

				if (_CurrentMessageFailed)
					throw IModule::EInvokeFailed();
			}
			else
			{
				// normal processing by the main task
				_onProcessModuleMessage(senderModuleProxy, message);
			}
		}
	}

	void CModuleBase::_receiveModuleMessageTask()
	{
		H_AUTO(CModuleBase__receiveModuleMessageTask);

		while (!_MessageDispatchTask->isTerminationRequested())
		{
			// we have a message to dispatch
			try
			{
				// take a copy of the message to dispatch
				IModuleProxy *currentSender = _CurrentSender;
				CMessage currentMessage = *_CurrentMessage;
				_onProcessModuleMessage(currentSender, currentMessage);
				_CurrentMessageFailed = false;
			}
			catch (const NLMISC::Exception &e)
			{
				nlwarning("In module task '%s' (cotask message receiver), exception '%e' thrown", typeid(this).name(), e.what());
				// an exception have been thrown
				_CurrentMessageFailed = true;
			}
			catch (...)
			{
				nlwarning("In module task '%s' (cotask message receiver), unknown exception thrown", typeid(this).name());
				// an exception have been thrown
				_CurrentMessageFailed = true;
			}
			// switch to main task
			_MessageDispatchTask->yield();
		}
	}


	void CModuleBase::queueModuleTask(CModuleTask *task)
	{
		_ModuleTasks.push_back(task);
	}

	CModuleTask *CModuleBase::getActiveModuleTask()
	{
		if (_ModuleTasks.empty())
			return NULL;

		return _ModuleTasks.front();
	}


	const std::string &CModuleBase::getInitStringHelp()
	{
		static string help;
		return help;
	}

	// Init base module, init module name
	bool	CModuleBase::initModule(const TParsedCommandLine &initInfo)
	{
		// read module init param for base module .

		if (initInfo.getParam("base.useCoTaskDispatch"))
		{
			// init the message dispatch task
//			NLNET_START_MODULE_TASK(CModuleBase, _receiveModuleMessageTask);
//	TModuleTask<className> *task = new TModuleTask<className>(this, &className::methodName);
			 _MessageDispatchTask = new TModuleTask<CModuleBase>(this,  TModuleTask<CModuleBase>::TMethodPtr(&CModuleBase::_receiveModuleMessageTask));
		}

		// register this module in the command executor
		registerCommandsHandler();

		return true;
	}

	const std::string &CModuleBase::getCommandHandlerName() const
	{
		return getModuleName();
	}


	void CModuleBase::plugModule(IModuleSocket *moduleSocket) throw (EModuleAlreadyPluggedHere)
	{
		CModuleSocket *sock = dynamic_cast<CModuleSocket*>(moduleSocket);
		nlassert(sock != NULL);

		TModuleSockets::iterator it(_ModuleSockets.find(moduleSocket));
		if (it != _ModuleSockets.end())
			throw EModuleAlreadyPluggedHere();


		// ok, we can plug the module

		sock->_onModulePlugged(this);

		// all fine, store the socket pointer.
		_ModuleSockets.insert(moduleSocket);
	}

	void CModuleBase::unplugModule(IModuleSocket *moduleSocket)  throw (EModuleNotPluggedHere)
	{
		CModuleSocket *sock = dynamic_cast<CModuleSocket*>(moduleSocket);
		nlassert(sock != NULL);

		TModuleSockets::iterator it(_ModuleSockets.find(moduleSocket));
		if (it == _ModuleSockets.end())
			throw EModuleNotPluggedHere();

		sock->_onModuleUnplugged(TModulePtr(this));

		_ModuleSockets.erase(it);
	}

	void CModuleBase::getPluggedSocketList(std::vector<IModuleSocket*> &resultList)
	{
		TModuleSockets::iterator first(_ModuleSockets.begin()), last(_ModuleSockets.end());
		for (; first != last; ++first)
		{
			resultList.push_back(*first);
		}
	}

	/** Do a module operation invocation.
	 *	Caller MUST be in a module task to call this method.
	 *	The call is blocking until receptions of the operation
	 *	result message (or a module down)
	 */
	void CModuleBase::invokeModuleOperation(IModuleProxy *destModule, const NLNET::CMessage &opMsg, NLNET::CMessage &resultMsg) throw (EInvokeFailed)
	{
		H_AUTO(CModuleBase_invokeModuleOperation);

		nlassert(opMsg.getType() == CMessage::Request);

		// check that we are running in a coroutine task
		CModuleTask *task = dynamic_cast<CModuleTask *>(CCoTask::getCurrentTask());
		nlassert(task != NULL);
		// send the message to the module
		destModule->sendModuleMessage(this, opMsg);

		// fill the invoke stack
		_InvokeStack.push_back(destModule);

		for (;;)
		{
			// yield and wait for messages
			task->yield();

			if (task->mustFailInvoke())
			{
				nlassert(!_InvokeStack.empty());
				// empty the invoke stack
				_InvokeStack.pop_back();

				task->resetFailInvoke();

				throw EInvokeFailed();
			}

			while (!_SyncMessages.empty())
			{
				IModuleProxy *proxy = _SyncMessages.front().first;
				CMessage &msg = _SyncMessages.front().second;
				if (msg.getType() == CMessage::Response)
				{
					// we have the response message
					nlassert(proxy == destModule);
					resultMsg = msg;
					// remove this message form the queue
					_SyncMessages.pop_front();
					// empty the invoke stack
					_InvokeStack.pop_back();
					// stop reading received message now
					return;
				}
				else if (msg.getType() == CMessage::Except)
				{
					// the other side returned an exception !

					// empty the invoke stack
					_InvokeStack.pop_back();

					throw EInvokeFailed();
				}
				else
				{
					// another message, dispatch it normally
//					CMessage::TMessageType msgType = msg.getType();
//					try
//					{
						_onProcessModuleMessage(proxy, msg);
//					}
//					catch(...)
//					{
//						nlwarning("Some exception where throw will dispatching message '%s' from '%s' to '%s'",
//							msg.getName().c_str(),
//							proxy->getModuleName().c_str(),
//							this->getModuleName().c_str());
//
//						if (msgType == CMessage::Request)
//						{
//							// send back an exception message
//							CMessage except;
//							except.setType("EXCEPT", CMessage::Except);
//							proxy->sendModuleMessage(this, except);
//						}
//
//					}

					// remove this message form the queue
					_SyncMessages.pop_front();
				}
			}
		}
	}

	void CModuleBase::_onModuleUp(IModuleProxy *removedProxy)
	{
		H_AUTO(CModuleBase__onModuleUp);

		// call the normal callback in the interceptor list
		TInterceptors::iterator first(_ModuleInterceptors.begin()), last(_ModuleInterceptors.end());
		for (;first != last; ++first)
		{
			IModuleInterceptable *interceptor = *first;
			interceptor->onModuleUp(removedProxy);
		}
	}

	void CModuleBase::_onModuleDown(IModuleProxy *removedProxy)
	{
		H_AUTO(CModuleBase__onModuleDown);

		// remove any message from the message queue that come from this proxy
		{
			TMessageList::iterator first(_SyncMessages.begin()), last(_SyncMessages.end());
			for (; first != last; ++first)
			{
				if (first->first == removedProxy)
				{
					_SyncMessages.erase(first);
					first = _SyncMessages.begin();
				}
			}
		}
		// check the invocation stack also
		{
			TInvokeStack::iterator first(_InvokeStack.begin()), last(_InvokeStack.end());
			for (; first != last; ++first)
			{
				if (*first == removedProxy)
				{
					// at least, we need either a running task or the default dispatch task activated
					nlassert(!_ModuleTasks.empty() || _MessageDispatchTask != NULL);

					// gasp, we lost one of the module needed to managed the invocation stack!
					// make each call generate an exception
					while (first != _InvokeStack.end())
					{
						// The module task can be either the first in the module task list or
						// the co routine dispatching task if it is activated
						CModuleTask *task = !_ModuleTasks.empty() ? _ModuleTasks.front() : _MessageDispatchTask;
						task->failInvoke();
						// switch to task to unstack one level
						task->resume();
					}
					break;
				}
			}
		}

		// call the normal callback in the interceptor list
		TInterceptors::iterator first(_ModuleInterceptors.begin()), last(_ModuleInterceptors.end());
		for (;first != last; ++first)
		{
			(*first)->onModuleDown(removedProxy);
		}
	}

	bool CModuleBase::_onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
	{
		H_AUTO(CModuleBase__OnProcessModuleMessage);

		// try the call on each interceptor
		bool result;
		result = false;

		try
		{
			TInterceptors::iterator first(_ModuleInterceptors.begin()), last(_ModuleInterceptors.end());
			for (;first != last; ++first)
			{
				if ((*first)->onProcessModuleMessage(senderModuleProxy, message))
				{
					result = true;
					break;
				}
			}
		}
		catch(...)
		{
			nlwarning("Some exception where throw will dispatching message '%s' from '%s' to '%s'",
				message.getName().c_str(),
				senderModuleProxy->getModuleName().c_str(),
				this->getModuleName().c_str());

			if (message.getType() == CMessage::Request)
			{
				// send back an exception message
				CMessage except;
				except.setType("EXCEPT", CMessage::Except);
				senderModuleProxy->sendModuleMessage(this, except);
			}
			// here we return true because the message have been processed
			// (even if the processing have raised some exception !)
			result = true;
		}

		return result;
	}

	void CModuleBase::setFactory(IModuleFactory *factory)
	{
		nlassert(_ModuleFactory == NULL);

		_ModuleFactory = factory;
	}

	IModuleFactory *CModuleBase::getFactory()
	{
		return _ModuleFactory;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, plug)
	{
		nlunreferenced(human);
		nlunreferenced(quiet);
		nlunreferenced(rawCommandString);

		if (args.size() != 1)
			return false;

		IModuleSocket *socket = IModuleManager::getInstance().getModuleSocket(args[0]);

		if (socket == NULL)
		{
			log.displayNL("Unknown socket named '%s'", args[0].c_str());
			return true;
		}

		plugModule(socket);

		if (_ModuleSockets.find(socket) == _ModuleSockets.end())
		{
			log.displayNL("Failed to plug the module '%s' into the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());
		}
		else
			log.displayNL("Module '%s' plugged into the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, unplug)
	{
		nlunreferenced(human);
		nlunreferenced(quiet);
		nlunreferenced(rawCommandString);

		if (args.size() != 1)
			return false;

		IModuleSocket *socket = IModuleManager::getInstance().getModuleSocket(args[0]);

		if (socket == NULL)
		{
			log.displayNL("Unknown socket named '%s'", args[0].c_str());
			return true;
		}

		if (_ModuleSockets.find(socket) == _ModuleSockets.end())
		{
			log.displayNL("The module '%s' is not plugged in the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());
			return true;
		}

		unplugModule(socket);

		if (_ModuleSockets.find(socket) != _ModuleSockets.end())
			log.displayNL("Failed to unplug the module '%s' from the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());
		else
			log.displayNL("Module '%s' unplugged out of the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, sendPing)
	{
		nlunreferenced(human);
		nlunreferenced(quiet);
		nlunreferenced(rawCommandString);

		if (args.size() != 1)
			return false;

		string modName = args[0];

		// look in each socket
		vector<IModuleSocket*> sockets;
		this->getPluggedSocketList(sockets);

		for (uint i=0; i<sockets.size(); ++i)
		{
			IModuleSocket *socket = sockets[i];
			vector<IModuleProxy*> proxList;
			socket->getModuleList(proxList);

			for (uint i=0; i<proxList.size(); ++i)
			{
				if (proxList[i]->getModuleName() == modName)
				{
					// we found it !
					CMessage ping("DEBUG_MOD_PING");
					proxList[i]->sendModuleMessage(this, ping);
					log.displayNL("Ping debug message send to '%s'", modName.c_str());
					return true;
				}
			}
		}

		log.displayNL("Can't find a route to send message to module '%s'", modName.c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, dump)
	{
		nlunreferenced(human);
		nlunreferenced(quiet);
		nlunreferenced(rawCommandString);

		if (args.size() != 0)
			return false;

		log.displayNL("---------------------------");
		log.displayNL("Dumping base module state :");
		log.displayNL("---------------------------");
		log.displayNL("  Module name      : '%s'", getModuleName().c_str());
		log.displayNL("  Module full name : '%s'", getModuleFullyQualifiedName().c_str());
		log.displayNL("  Module class     : '%s'", _ModuleFactory->getModuleClassName().c_str());
		log.displayNL("  Module ID        : %u", _ModuleId);
		log.displayNL("  The module is plugged into %u sockets :", _ModuleSockets.size());
		{
			TModuleSockets::iterator first(_ModuleSockets.begin()), last(_ModuleSockets.end());
			for (; first != last; ++first)
			{
				IModuleSocket *ps = *first;
				vector<IModuleProxy*>	proxies;
				ps->getModuleList(proxies);

				log.displayNL("    Socket '%s', %u modules reachable :", ps->getSocketName().c_str(), proxies.size()-1);

				for (uint i=0; i<proxies.size(); ++i)
				{
					string name = proxies[i]->getModuleName();
					if (name.find('/') != string::npos)
						name = name.substr(name.find('/')+1);
					if (name != getModuleFullyQualifiedName())
					{
						log.displayNL("      Module '%s' (Module Proxy ID : %u, class : '%s')",
							proxies[i]->getModuleName().c_str(),
							proxies[i]->getModuleProxyId(),
							proxies[i]->getModuleClassName().c_str());
					}
				}
			}
		}

		return true;
	}


	/************************************************************************
	 * CModuleProxy impl
	 ************************************************************************/

	CModuleProxy::CModuleProxy(TModulePtr localModule, TModuleId localModuleId, const std::string &moduleClassName, const std::string &fullyQualifiedModuleName, const std::string &moduleManifest)
		: _ModuleProxyId(localModuleId),
		  _ForeignModuleId(INVALID_MODULE_ID),
		  _LocalModule(localModule),
		  _ModuleClassName(CStringMapper::map(moduleClassName)),
		  _FullyQualifiedModuleName(CStringMapper::map(fullyQualifiedModuleName)),
		  _Manifest(moduleManifest),
		  _SecurityData(NULL)
	{
	}

	TModuleId	CModuleProxy::getModuleProxyId() const
	{
		return _ModuleProxyId;
	}

	TModuleId	CModuleProxy::getForeignModuleId() const
	{
		return _ForeignModuleId;
	}

	uint32		CModuleProxy::getModuleDistance() const
	{
		return _Distance;
	}

	IModule				*CModuleProxy::getLocalModule() const
	{
		return _LocalModule;
	}

	CGatewayRoute		*CModuleProxy::getGatewayRoute() const
	{
		return _Route;
	}

	const std::string &CModuleProxy::getModuleName() const
	{
		return CStringMapper::unmap(_FullyQualifiedModuleName);
	}
	const std::string &CModuleProxy::getModuleClassName() const
	{
		return CStringMapper::unmap(_ModuleClassName);
	}

	const std::string &CModuleProxy::getModuleManifest() const
	{
		return _Manifest;
	}


	IModuleGateway *CModuleProxy::getModuleGateway() const
	{
		return _Gateway;
	}

	void		CModuleProxy::sendModuleMessage(IModule *senderModule, const NLNET::CMessage &message)
		throw (EModuleNotReachable)
	{
		H_AUTO(CModuleProxy_sendModuleMessage);

		if (_Gateway == NULL )
		{
			throw EModuleNotReachable();
		}

		// We need to find the proxy for the sender using the addressee gateway
		IModuleProxy *senderProx = _Gateway->getPluggedModuleProxy(senderModule);
		if (senderProx == NULL )
		{
			throw EModuleNotReachable();
		}

		_Gateway->sendModuleMessage(senderProx, this, message);
	}

	const TSecurityData *CModuleProxy::findSecurityData(uint8 dataTag) const
	{
		const TSecurityData *ms = _SecurityData;

		while (ms != NULL)
		{
			if (ms->DataTag == dataTag)
			{
				// this block match !
				return ms;
			}

			// try the next one
			ms = ms->NextItem;
		}

		// not found
		return NULL;
	}


} // namespace NLNET
