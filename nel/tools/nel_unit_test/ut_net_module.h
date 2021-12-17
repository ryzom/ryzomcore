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

#ifndef UT_NET_MODULE
#define UT_NET_MODULE

#include <nel/misc/dynloadlib.h>
#include <nel/misc/command.h>
#include <nel/misc/path.h>
#include <nel/net/module_common.h>
#include <nel/net/module_manager.h>
#include <nel/net/module.h>
#include <nel/net/inet_address.h>
#include <nel/net/module_socket.h>
#include <nel/net/module_gateway.h>
#include <nel/net/service.h>

class CModuleType0 : public NLNET::CModuleBase
{
public:

	uint	PingCount;
	uint	ResponseReceived;

	set<NLNET::TModuleProxyPtr>	ModuleType0;

	uint32	ModuleUpCalled;
	uint32	ModuleDownCalled;
	uint32	ProcessMessageCalled;
	uint32	SecurityUpdateCalled;


	CModuleType0()
		: PingCount(0),
		ResponseReceived(0)
	{
		ModuleUpCalled = 0;
		ModuleDownCalled = 0;
		ProcessMessageCalled = 0;
		SecurityUpdateCalled = 0;
	}

	std::string					buildModuleManifest() const
	{
		return "CModuleType0";
	}

	bool initModule(const NLNET::TParsedCommandLine &param)
	{
		bool ret = CModuleBase::initModule(param);
		if (param.getParam("FAIL") != NULL)
			return false;

		return ret;
	}

	void				onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId)
	{
	}
	/// A nel layer 5 service has stopped.
	void				onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId) 
	{
	}
	void				onModuleUpdate()
	{
	}
	/** The service main loop is terminating it job', all module will be
	 *	disconnected and removed after this callback.
	 */
	void				onApplicationExit() 
	{
	}

	void				onModuleUp(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleUpCalled++;

		if (moduleProxy->getModuleClassName() == getModuleClassName())
			ModuleType0.insert(moduleProxy);
	}
	/** Called by a socket to inform this module that another
	 *	module has been deleted OR has been no more accessible (due to
	 *	some gateway disconnection).
	 */
	void				onModuleDown(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleDownCalled++;

		if (moduleProxy->getModuleClassName() == getModuleClassName())
			ModuleType0.erase(moduleProxy);
	}

	bool				onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message)
	{
		ProcessMessageCalled++;

		if (message.getName() == "DEBUG_MOD_PING")
		{
			PingCount++;
			return true;
		}
		else if (message.getName() == "HELLO")
		{
			NLNET::CMessage ping("DEBUG_MOD_PING");
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			{
				NLNET::CMessage resp;
				resp.setType("HELLO_RESP", NLNET::CMessage::Response);

				senderModuleProxy->sendModuleMessage(this, resp);
			}
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			return true;
		}
		else if (message.getName() == "HELLO2")
		{
			// the response for the life, the universe and all other things...
			throw 42;

			return true;
		}

		return false;
	}

	void				onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy)
	{
		SecurityUpdateCalled++;
	}

	void				onModuleSocketEvent(NLNET::IModuleSocket *moduleSocket, IModule::TModuleSocketEvent eventType)
	{
	}

	void startTaskA()
	{
		// start a task on module 
		NLNET_START_MODULE_TASK(CModuleType0, taskA);
	}

	// test task A 
	void		taskA()
	{
		// use the first like me in the list
		nlassert(!ModuleType0.empty());

		NLNET::TModuleProxyPtr proxy = *ModuleType0.begin();

		NLNET::CMessage msg;
		msg.setType("HELLO", NLNET::CMessage::Request);
		NLNET::CMessage resp;
		invokeModuleOperation(proxy, msg, resp);

		nlassert(resp.getName() == "HELLO_RESP");

		ResponseReceived++;
	}

	void startTaskB()
	{
		// start a task on module
		NLNET_START_MODULE_TASK(CModuleType0, taskB);
	}

	// test task B 
	void		taskB()
	{
		// use the first like me in the list
		nlassert(!ModuleType0.empty());

		NLNET::TModuleProxyPtr proxy = *ModuleType0.begin();

		NLNET::CMessage msg;
		msg.setType("HELLO2", NLNET::CMessage::Request);
		NLNET::CMessage resp;

		try
		{
			invokeModuleOperation(proxy, msg, resp);
		}
		catch(const IModule::EInvokeFailed &)
		{
			ResponseReceived++;
		}
	}

};

NLNET_REGISTER_MODULE_FACTORY(CModuleType0, "ModuleType0");

// A module that doesn't support immediate dispatching
class CModuleAsync : public CModuleType0
{
public:

	bool isImmediateDispatchingSupported() const 
	{ 
		return false; 
	}
};

NLNET_REGISTER_MODULE_FACTORY(CModuleAsync, "ModuleAsync");

enum TTestSecurityTypes
{
	tst_type1,
	tst_type2,
	tst_type3,
	tst_type4,
};

// security type 1 data : contains host gateway name
struct TSecurityType1 : public NLNET::TSecurityData
{
	string	SecurityGatewayName;

	TSecurityType1(const TCtorParam &param)
		: NLNET::TSecurityData(param)
	{
	}

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(SecurityGatewayName);
	}

};

NLMISC_REGISTER_OBJECT(NLNET::TSecurityData, TSecurityType1, uint8, tst_type1);


// security type 2 data : contains host gateway name and a predefined integer value
struct TSecurityType2 : public NLNET::TSecurityData
{
	string	SecurityGatewayName;
	uint32	IntegerValue;

	TSecurityType2(const TCtorParam &param)
		: NLNET::TSecurityData(param)
	{
		IntegerValue = 0x12345678;
	}

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(SecurityGatewayName);
		s.serial(IntegerValue);
	}
};

NLMISC_REGISTER_OBJECT(NLNET::TSecurityData, TSecurityType2, uint8, tst_type2);


// security type 3 data, same as type 1
struct TSecurityType3 : public NLNET::TSecurityData
{
	string	SecurityGatewayName;

	TSecurityType3(const TCtorParam &param)
		: NLNET::TSecurityData(param)
	{
	}

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(SecurityGatewayName);
	}

};
NLMISC_REGISTER_OBJECT(NLNET::TSecurityData, TSecurityType3, uint8, tst_type3);

// security type 4 data, same as type 1 but not registered
struct TSecurityType4 : public NLNET::TSecurityData
{
	string	SecurityGatewayName;

	TSecurityType4(const TCtorParam &param)
		: NLNET::TSecurityData(param)
	{
	}

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(SecurityGatewayName);
	}
};

/** a sample security plug-in that add type 1 security data to local modules,
 *	type 2 security to foreign module,
 *	and that remove received type 1 security from foreign module
 */
class CTestSecurity1 : public NLNET::CGatewaySecurity
{
public:
	CTestSecurity1(const TCtorParam &params)
		: NLNET::CGatewaySecurity(params)
	{}

	virtual void onNewProxy(NLNET::IModuleProxy *proxy)
	{
		if (proxy->getGatewayRoute() == NULL)
		{
			// add a type 1 security
			TSecurityType1 *st1 = new TSecurityType1(NLNET::TSecurityData::TCtorParam(tst_type1));
			st1->SecurityGatewayName = _Gateway->getFullyQualifiedGatewayName();

			setSecurityData(proxy, st1);
		}
		else
		{
			// remove any type 1 data and set a type 2 data
			removeSecurityData(proxy, tst_type1);
			TSecurityType2 *st2 = new TSecurityType2(NLNET::TSecurityData::TCtorParam(tst_type2));
			st2->SecurityGatewayName = _Gateway->getFullyQualifiedGatewayName();

			setSecurityData(proxy, st2);
		}

		forceSecurityUpdate(proxy);
	}

	void onNewSecurityData(NLNET::CGatewayRoute *from, NLNET::IModuleProxy *proxy, NLNET::TSecurityData *firstSecurityData)
	{
		// replace the complete security set
		replaceAllSecurityDatas(proxy, firstSecurityData);
		// remove any type 1 data and set a type 2 data
		removeSecurityData(proxy, tst_type1);
		TSecurityType2 *st2 = new TSecurityType2(NLNET::TSecurityData::TCtorParam(tst_type2));
		st2->SecurityGatewayName = _Gateway->getFullyQualifiedGatewayName();

		setSecurityData(proxy, st2);

		// we don't need to update in this case  (update is always done by gateway)
	}

	virtual void onDelete()
	{
		vector<NLNET::IModuleProxy*>	proxies;
		_Gateway->getModuleProxyList(proxies);

		// remove any security data managed by this plug-in
		for (uint i=0; i<proxies.size(); ++i)
		{
			NLNET::IModuleProxy *proxy = proxies[i];
			if (proxy->getFirstSecurityData() != NULL)
			{
				bool update = false;
				update |= removeSecurityData(proxy, tst_type1);
				update |= removeSecurityData(proxy, tst_type2);
				if (update)
					forceSecurityUpdate(proxy);
			}
		}
	}

};
NLMISC_REGISTER_OBJECT(NLNET::CGatewaySecurity, CTestSecurity1, std::string, "TestSecurity1");

/** a sample security plug-in that add type 3 and type 4 security data to local modules,
 */
class CTestSecurity2 : public NLNET::CGatewaySecurity
{
public:
	CTestSecurity2(const TCtorParam &params)
		: NLNET::CGatewaySecurity(params)
	{}

	virtual void onNewProxy(NLNET::IModuleProxy *proxy)
	{
		if (proxy->getGatewayRoute() == NULL)
		{
			// add a type 3 security
			TSecurityType3 *st3 = new TSecurityType3(NLNET::TSecurityData::TCtorParam(tst_type3));
			st3->SecurityGatewayName = _Gateway->getFullyQualifiedGatewayName();
			setSecurityData(proxy, st3);
			// add a type 4 security
			TSecurityType4 *st4 = new TSecurityType4(NLNET::TSecurityData::TCtorParam(tst_type4));
			st4->SecurityGatewayName = _Gateway->getFullyQualifiedGatewayName();
			setSecurityData(proxy, st4);
			forceSecurityUpdate(proxy);
		}
	}

	void onNewSecurityData(NLNET::CGatewayRoute *from, NLNET::IModuleProxy *proxy, NLNET::TSecurityData *firstSecurityData)
	{
		// replace the complete security set
		replaceAllSecurityDatas(proxy, firstSecurityData);
	}

	virtual void onDelete()
	{
		vector<NLNET::IModuleProxy*>	proxies;
		_Gateway->getModuleProxyList(proxies);

		// remove any security data managed by this plug-in
		for (uint i=0; i<proxies.size(); ++i)
		{
			NLNET::IModuleProxy *proxy = proxies[i];
			if (proxy->getGatewayRoute() == NULL)
			{
				removeSecurityData(proxy, tst_type3);
				removeSecurityData(proxy, tst_type4);
				forceSecurityUpdate(proxy);
			}
		}
	}
};
NLMISC_REGISTER_OBJECT(NLNET::CGatewaySecurity, CTestSecurity2, std::string, "TestSecurity2");


// A module interceptor
class CInterceptor : public NLNET::IModuleInterceptable
{
public:
	string	Name;
	uint32	ModuleUpCalled;
	uint32	ModuleDownCalled;
	uint32	ProcessMessageCalled;
	uint32	SecurityUpdateCalled;

	CInterceptor(NLNET::IInterceptorRegistrar *registrar, const string &name)
		:	Name(name)
	{
		NLNET::IModuleInterceptable::registerInterceptor(registrar),
		ModuleUpCalled = 0;
		ModuleDownCalled = 0;
		ProcessMessageCalled = 0;
		SecurityUpdateCalled = 0;
	}

	virtual std::string			buildModuleManifest() const
	{
		return Name;
	}

	virtual void				onModuleUp(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleUpCalled++;
	}

	virtual void				onModuleDown(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleDownCalled++;
	}

	virtual bool				onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message)
	{
		ProcessMessageCalled++;
		return false;
	}

	virtual void				onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy)
	{
		SecurityUpdateCalled++;
	}
};

// Test suite for Modules class
class CUTNetModule : public Test::Suite
{
	string	_WorkingPath;
	string	_RestorePath;

public:
	// utility to look for a specified proxy in a vector of proxy
	// return true if the proxy if found
	bool lookForModuleProxy(const vector<NLNET::IModuleProxy*> proxList, const std::string &modName)
	{
		for (uint i=0; i<proxList.size(); ++i)
		{
			if (proxList[i]->getModuleName().find(modName) == (proxList[i]->getModuleName().size() - modName.size()))
				return true;
		}

		return false;
	}

	NLNET::IModuleProxy *retrieveModuleProxy(NLNET::IModuleGateway *gw, const std::string &modName)
	{
		vector<NLNET::IModuleProxy*> proxList;
		gw->getModuleProxyList(proxList);

		for (uint i=0; i<proxList.size(); ++i)
		{
			if (proxList[i]->getModuleName().find(modName) == (proxList[i]->getModuleName().size() - modName.size()))
				return proxList[i];
		}

		return NULL;
	}

	void setup()
	{
		_RestorePath = NLMISC::CPath::getCurrentPath();

		NLMISC::CPath::setCurrentPath(_WorkingPath.c_str());
	}

	void tear_down()
	{
		NLMISC::CPath::setCurrentPath(_RestorePath.c_str());
	}

	CUTNetModule ()
	{
		TEST_ADD(CUTNetModule::testModuleInitInfoParsing);
		TEST_ADD(CUTNetModule::testModuleInitInfoQuering);
		TEST_ADD(CUTNetModule::testModuleInitInfoBadParsing);
		TEST_ADD(CUTNetModule::localModuleFactory);
		//TEST_ADD(CUTNetModule::loadModuleLib);
		//TEST_ADD(CUTNetModule::createModule);
		//TEST_ADD(CUTNetModule::deleteModule);
		TEST_ADD(CUTNetModule::failedInit);
		//TEST_ADD(CUTNetModule::unloadModuleLib);
		TEST_ADD(CUTNetModule::createLocalGateway);
		TEST_ADD(CUTNetModule::plugLocalGateway);
		//TEST_ADD(CUTNetModule::moduleManagerCommands);
		TEST_ADD(CUTNetModule::gatewayTransportManagement);
		TEST_ADD(CUTNetModule::connectGateways);
		TEST_ADD(CUTNetModule::moduleDisclosure);
		TEST_ADD(CUTNetModule::moduleMessaging);
		TEST_ADD(CUTNetModule::localMessageQueing);
		TEST_ADD(CUTNetModule::uniqueNameGenerator);
		TEST_ADD(CUTNetModule::gwPlugUnplug);
		TEST_ADD(CUTNetModule::peerInvisible);
		TEST_ADD(CUTNetModule::firewalling);
		TEST_ADD(CUTNetModule::distanceAndConnectionLoop);
		TEST_ADD(CUTNetModule::securityPlugin);
		TEST_ADD(CUTNetModule::synchronousMessaging);
		TEST_ADD(CUTNetModule::layer3Autoconnect);
		TEST_ADD(CUTNetModule::interceptorTest);
	}

	void interceptorTest()
	{
		// Check that the interceptor system.

		// TODO : right now, there is no test of the security update call

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		// create the modules
		NLNET::IModule *gw = mm.createModule("StandardGateway", "gw", "");
		NLNET::IModule *mod = mm.createModule("ModuleType0", "mod", "");
		NLNET::IModuleGateway *gGw = dynamic_cast<NLNET::IModuleGateway *>(gw);
		CModuleType0 *mod0 = dynamic_cast<CModuleType0*>(mod);

		TEST_ASSERT(gGw != NULL);
		TEST_ASSERT(mod0 != NULL);

		// create the interceptors and attach it to the mod0
		CInterceptor *inter0 = new CInterceptor(mod, "Inter0");
		CInterceptor *inter1 = new CInterceptor(mod, "Inter1");

		// plug the modules
		cr.execute("gw.plug gw", NLMISC::InfoLog());
		cr.execute("mod.plug gw", NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// send a message to the module fro; the gateway
		NLNET::CMessage msg("foo");
		NLNET::IModuleProxy *modProx = retrieveModuleProxy(gGw, "mod");
		TEST_ASSERT(modProx != NULL);
		modProx->sendModuleMessage(gw, msg);

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// check the module manifest
		TEST_ASSERT(modProx->getModuleManifest() == "CModuleType0 Inter0 Inter1");

		// unplug the modules
		cr.execute("gw.unplug gw", NLMISC::InfoLog());
		cr.execute("mod.unplug gw", NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// now check that all methods have been called in that
		// module and in the two interceptors,
		// also check the manifest string content.
		TEST_ASSERT(mod0->ModuleUpCalled == 1);
		TEST_ASSERT(mod0->ModuleDownCalled == 1);
		TEST_ASSERT(mod0->ProcessMessageCalled == 1);
//		TEST_ASSERT(mod0->SecurityUpdateCalled);
		
		TEST_ASSERT(inter0->ModuleUpCalled == 1);
		TEST_ASSERT(inter0->ModuleDownCalled == 1);
		TEST_ASSERT(inter0->ProcessMessageCalled == 1);
//		TEST_ASSERT(inter0->SecurityUpdateCalled);
		
		TEST_ASSERT(inter1->ModuleUpCalled == 1);
		TEST_ASSERT(inter1->ModuleDownCalled == 1);
		TEST_ASSERT(inter1->ProcessMessageCalled == 1);
//		TEST_ASSERT(inter1->SecurityUpdateCalled);


		// delete the modules
		mm.deleteModule(gw);
		mm.deleteModule(mod);

		// delete the interceptors
		delete inter0;
		delete inter1;
	}
	
	void layer3Autoconnect()
	{
		// Check that layer 3 client can automatically reconnect in case of server 
		// down/up
		//
		//	We create two gateway, gw1 and gw2, plugged in themselves, then we create
		//	a layer 3 client on gw1, update the network, then we create the layer 3 server
		//	on gw2, update the network then check that module are connected.
		//
		//	Then we close the L3 server on gw2, update the network, check that module
		//	are diconnected and reopen the L3 server and recheck that module are connected.
		//
		//


		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		// create the modules
		NLNET::IModule *gw1 = mm.createModule("StandardGateway", "gw1", "");
		NLNET::IModule *gw2 = mm.createModule("StandardGateway", "gw2", "");
		NLNET::IModuleGateway *gGw1 = dynamic_cast<NLNET::IModuleGateway *>(gw1);
		NLNET::IModuleGateway *gGw2 = dynamic_cast<NLNET::IModuleGateway *>(gw2);

		// plug gateway in themselves
		cr.execute("gw1.plug gw1", NLMISC::InfoLog());
		cr.execute("gw2.plug gw2", NLMISC::InfoLog());

		// create the client transport
		cr.execute("gw1.transportAdd L3Client l3c", NLMISC::InfoLog());
		cr.execute("gw1.transportCmd l3c(retryInterval=1)", NLMISC::InfoLog());
		cr.execute("gw1.transportCmd l3c(connect addr=localhost:8062)", NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") == NULL);
		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") == NULL);

		// open the server
		cr.execute("gw2.transportAdd L3Server l3s", NLMISC::InfoLog());
		cr.execute("gw2.transportCmd l3s(open port=8062)", NLMISC::InfoLog());
		
		// update the network (give more time because we must cover the Layer3 client reconnection timer)
		for (uint i=0; i<40; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(50);
		}

		// check module connectivity
		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") != NULL);
		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") != NULL);

		// exchange some message
		cr.execute("gw1.sendPing "+gw2->getModuleFullyQualifiedName(), NLMISC::InfoLog());
		cr.execute("gw2.sendPing "+gw1->getModuleFullyQualifiedName(), NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// check the ping counter
		TEST_ASSERT(gGw1->getReceivedPingCount() == 1);
		TEST_ASSERT(gGw2->getReceivedPingCount() == 1);

		// flood a little with ping
		for (uint i=0; i<100; ++i)
			cr.execute("gw1.sendPing "+gw2->getModuleFullyQualifiedName(), NLMISC::InfoLog());

		// close the server
		cr.execute("gw2.transportCmd l3s(close)", NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// test no connectivity
		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") == NULL);
		TEST_ASSERT(retrieveModuleProxy(gGw2, "gw1") == NULL);

		// re-open the server
		cr.execute("gw2.transportCmd l3s(open port=8062)", NLMISC::InfoLog());

		// update the network (give more time because we must cover the Layer3 client reconnection timer)
		for (uint i=0; i<40; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(50);
		}

		// check module connectivity
		TEST_ASSERT(retrieveModuleProxy(gGw1, "gw2") != NULL);
		TEST_ASSERT(retrieveModuleProxy(gGw2, "gw1") != NULL);

		// exchange some message
		cr.execute("gw1.sendPing "+gw2->getModuleFullyQualifiedName(), NLMISC::InfoLog());
		cr.execute("gw2.sendPing "+gw1->getModuleFullyQualifiedName(), NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// check the ping counter
		TEST_ASSERT(gGw1->getReceivedPingCount() == 2);
		TEST_ASSERT(gGw2->getReceivedPingCount() == 2);

		// cleanup modules
		mm.deleteModule(gw1);
		mm.deleteModule(gw2);
	}

	void synchronousMessaging()
	{
		// check that the synchronous messaging is working
		// by using module task

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();


		// create the modules
		NLNET::IModule *gw = mm.createModule("StandardGateway", "gw", "");
		NLNET::IModule *m1 = mm.createModule("ModuleType0", "m1", "");
		NLNET::IModule *m2 = mm.createModule("ModuleType0", "m2", "");

		// plug the two modules in the gateway
		cr.execute("m1.plug gw", NLMISC::InfoLog());
		cr.execute("m2.plug gw", NLMISC::InfoLog());

		// update the network
		for (uint i=0; i<15; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		CModuleType0 *mod1 = dynamic_cast<CModuleType0 *>(m1);

		// start a task on module 1
		mod1->startTaskA();

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}


		TEST_ASSERT(mod1->PingCount == 4);
		TEST_ASSERT(mod1->ResponseReceived == 1);

		// start a task on module 1
		mod1->startTaskB();

		// update the network
		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}


		TEST_ASSERT(mod1->PingCount == 4);
		TEST_ASSERT(mod1->ResponseReceived == 2);
		mm.deleteModule(m1);
		mm.deleteModule(m2);
		mm.deleteModule(gw);
	}

	void securityPlugin()
	{
		// Check that security plug-in work well.
		//
		//	We connect three gateway in series with the central gateway
		//	using a security module that adds security data to
		//	proxies :
		//		For local proxies, it adds type 1 security data
		//		For foreign proxies, it adds type 2 security data
		//		for foreign proxies, it removes any type 1 security data found
		//
		//         gw1 (l3c) -------- (l3s) gw2 (l3c) ------ (l3s) gw3
		//          ^                        ^
		//          |                        |
		//    SecurityPlugin2          SecurityPlugin1
		//
		//	After connecting and plugging-in each gateway into themselves, 
		//	we check the presence and content of the security datas.
		//  then we remove the securityPlugin1 and check that all 
		//	security data have been removed,
		//	Then, we re create the securityPlugin1 and recheck
		//	then one again, we remove it and recheck.
		//
		//	For the second part of the check, we create a security
		//	plug-in 2 on gw1 that add 'type3' security data on
		//	local plug-in.
		//	We also plug the security plug-in 1 and then we check that 
		//	we have the correct security data

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *gw1, *gw2, *gw3;

		// create the modules
		gw1 = mm.createModule("StandardGateway", "gw1", "");
		gw2 = mm.createModule("StandardGateway", "gw2", "");
		gw3 = mm.createModule("StandardGateway", "gw3", "");

		TEST_ASSERT(gw1 != NULL);
		TEST_ASSERT(gw2 != NULL);
		TEST_ASSERT(gw3 != NULL);

		// plug gateway in themselves
		NLNET::IModuleSocket *sGw1, *sGw2, *sGw3;
		sGw1 = mm.getModuleSocket("gw1");
		sGw2 = mm.getModuleSocket("gw2");
		sGw3 = mm.getModuleSocket("gw3");

		TEST_ASSERT(sGw1 != NULL);
		TEST_ASSERT(sGw2 != NULL);
		TEST_ASSERT(sGw3 != NULL);

		gw1->plugModule(sGw1);
		gw2->plugModule(sGw2);
		gw3->plugModule(sGw3);

		string cmd;
		// create security plug-in
		cmd = "gw2.securityCreate TestSecurity1";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// create the transports
		cmd = "gw1.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// connect transport
		cmd = "gw2.transportCmd l3s(open port=8062)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportCmd l3s(open port=8063)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw1.transportCmd l3c(connect addr=localhost:8062)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportCmd l3c(connect addr=localhost:8063)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		
		for (uint retry = 0; retry < 2; ++retry)
		{
			if (retry > 0)
			{
				// recreate the security plug-in
				cmd = "gw2.securityCreate TestSecurity1";
				TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
			}
			// update the network
			for (uint i=0; i<15; ++i)
			{
				mm.updateModules();
				NLMISC::nlSleep(40);
			}
			NLNET::IModuleGateway *gGw1, *gGw2, *gGw3; 
			gGw1 = dynamic_cast<NLNET::IModuleGateway *>(gw1);
			TEST_ASSERT(gGw1 != NULL);
			gGw2 = dynamic_cast<NLNET::IModuleGateway *>(gw2);
			TEST_ASSERT(gGw2 != NULL);
			gGw3 = dynamic_cast<NLNET::IModuleGateway *>(gw3);
			TEST_ASSERT(gGw3 != NULL);
			

			// check security data
			NLNET::IModuleProxy *proxGw1_1, *proxGw2_1, *proxGw3_1;
			NLNET::IModuleProxy *proxGw1_2, *proxGw2_2, *proxGw3_2;
			NLNET::IModuleProxy *proxGw1_3, *proxGw2_3, *proxGw3_3;

			proxGw1_1 = retrieveModuleProxy(gGw1, "gw1");
			proxGw2_1 = retrieveModuleProxy(gGw1, "gw2");
			proxGw3_1 = retrieveModuleProxy(gGw1, "gw3");
			proxGw1_2 = retrieveModuleProxy(gGw2, "gw1");
			proxGw2_2 = retrieveModuleProxy(gGw2, "gw2");
			proxGw3_2 = retrieveModuleProxy(gGw2, "gw3");
			proxGw1_3 = retrieveModuleProxy(gGw3, "gw1");
			proxGw2_3 = retrieveModuleProxy(gGw3, "gw2");
			proxGw3_3 = retrieveModuleProxy(gGw3, "gw3");

			TEST_ASSERT(proxGw1_1 != NULL);
			TEST_ASSERT(proxGw2_1 != NULL);
			TEST_ASSERT(proxGw3_1 != NULL);
			TEST_ASSERT(proxGw1_2 != NULL);
			TEST_ASSERT(proxGw2_2 != NULL);
			TEST_ASSERT(proxGw3_2 != NULL);
			TEST_ASSERT(proxGw1_3 != NULL);
			TEST_ASSERT(proxGw2_3 != NULL);
			TEST_ASSERT(proxGw3_3 != NULL);

			const NLNET::TSecurityData *ms;
			const TSecurityType1 *st1;
			const TSecurityType2 *st2;

			ms = proxGw1_1->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);

			ms = proxGw1_2->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type2);
			st2 = dynamic_cast<const TSecurityType2 *>(ms);
			TEST_ASSERT(st2 != NULL);
			TEST_ASSERT(st2->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st2->IntegerValue == 0x12345678);
			TEST_ASSERT(st2->NextItem == NULL);

			ms = proxGw1_3->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type2);
			st2 = dynamic_cast<const TSecurityType2 *>(ms);
			TEST_ASSERT(st2 != NULL);
			TEST_ASSERT(st2->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st2->IntegerValue == 0x12345678);
			TEST_ASSERT(st2->NextItem == NULL);

			ms = proxGw2_1->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type1);
			st1 = dynamic_cast<const TSecurityType1 *>(ms);
			TEST_ASSERT(st1 != NULL);
			TEST_ASSERT(st1->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st1->NextItem == NULL);

			ms = proxGw2_2->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type1);
			st1 = dynamic_cast<const TSecurityType1 *>(ms);
			TEST_ASSERT(st1 != NULL);
			TEST_ASSERT(st1->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st1->NextItem == NULL);

			ms = proxGw2_3->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type1);
			st1 = dynamic_cast<const TSecurityType1 *>(ms);
			TEST_ASSERT(st1 != NULL);
			TEST_ASSERT(st1->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st1->NextItem == NULL);

			ms = proxGw3_1->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type2);
			st2 = dynamic_cast<const TSecurityType2 *>(ms);
			TEST_ASSERT(st2 != NULL);
			TEST_ASSERT(st2->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st2->IntegerValue == 0x12345678);
			TEST_ASSERT(st2->NextItem == NULL);

			ms = proxGw3_2->getFirstSecurityData();
			TEST_ASSERT(ms != NULL);
			TEST_ASSERT(ms->DataTag == tst_type2);
			st2 = dynamic_cast<const TSecurityType2 *>(ms);
			TEST_ASSERT(st2 != NULL);
			TEST_ASSERT(st2->SecurityGatewayName == gw2->getModuleFullyQualifiedName());
			TEST_ASSERT(st2->IntegerValue == 0x12345678);
			TEST_ASSERT(st2->NextItem == NULL);

			ms = proxGw3_3->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);

			// remove the security plug-in
			// create security plug-in
			cmd = "gw2.securityRemove";
			TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

			// update the network
			for (uint i=0; i<15; ++i)
			{
				mm.updateModules();
				NLMISC::nlSleep(40);
			}

			ms = proxGw1_1->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw1_2->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw1_3->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw2_1->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw2_2->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw2_3->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw3_1->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw3_2->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
			ms = proxGw3_3->getFirstSecurityData();
			TEST_ASSERT(ms == NULL);
		}

		// part 2
		// create the security plug-in
		cmd = "gw2.securityCreate TestSecurity1";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw1.securityCreate TestSecurity2";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<15; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		NLNET::IModuleGateway *gGw1, *gGw2, *gGw3; 
		gGw1 = dynamic_cast<NLNET::IModuleGateway *>(gw1);
		TEST_ASSERT(gGw1 != NULL);
		gGw2 = dynamic_cast<NLNET::IModuleGateway *>(gw2);
		TEST_ASSERT(gGw2 != NULL);
		gGw3 = dynamic_cast<NLNET::IModuleGateway *>(gw3);
		TEST_ASSERT(gGw3 != NULL);
		

		// check security data
		NLNET::IModuleProxy *proxGw1_1, *proxGw2_1, *proxGw3_1;
		NLNET::IModuleProxy *proxGw1_2, *proxGw2_2, *proxGw3_2;
		NLNET::IModuleProxy *proxGw1_3, *proxGw2_3, *proxGw3_3;

		proxGw1_1 = retrieveModuleProxy(gGw1, "gw1");
		proxGw2_1 = retrieveModuleProxy(gGw1, "gw2");
		proxGw3_1 = retrieveModuleProxy(gGw1, "gw3");
		proxGw1_2 = retrieveModuleProxy(gGw2, "gw1");
		proxGw2_2 = retrieveModuleProxy(gGw2, "gw2");
		proxGw3_2 = retrieveModuleProxy(gGw2, "gw3");
		proxGw1_3 = retrieveModuleProxy(gGw3, "gw1");
		proxGw2_3 = retrieveModuleProxy(gGw3, "gw2");
		proxGw3_3 = retrieveModuleProxy(gGw3, "gw3");

		TEST_ASSERT(proxGw1_1 != NULL);
		TEST_ASSERT(proxGw2_1 != NULL);
		TEST_ASSERT(proxGw3_1 != NULL);
		TEST_ASSERT(proxGw1_2 != NULL);
		TEST_ASSERT(proxGw2_2 != NULL);
		TEST_ASSERT(proxGw3_2 != NULL);
		TEST_ASSERT(proxGw1_3 != NULL);
		TEST_ASSERT(proxGw2_3 != NULL);
		TEST_ASSERT(proxGw3_3 != NULL);

		const NLNET::TSecurityData *ms;
//		const TSecurityType1 *st1;
//		const TSecurityType2 *st2;

		ms = proxGw1_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms != NULL);
		TEST_ASSERT(dynamic_cast<const TSecurityType4*>(ms) != NULL);

		ms = proxGw1_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_2->findSecurityData(0xff);
		TEST_ASSERT(ms != NULL);
		TEST_ASSERT(dynamic_cast<const NLNET::TUnknownSecurityData*>(ms) != NULL);

		ms = proxGw1_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_3->findSecurityData(0xff);
		TEST_ASSERT(ms != NULL);
		TEST_ASSERT(dynamic_cast<const NLNET::TUnknownSecurityData*>(ms) != NULL);


		ms = proxGw2_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw2_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_2->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw2_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_3->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);


		ms = proxGw3_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw3_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw3_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw3_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_2->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw3_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		// remove the security plug-in
		// create security plug-in
		cmd = "gw1.securityRemove";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<15; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		ms = proxGw1_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw1_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_2->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw1_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw1_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw1_3->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);


		ms = proxGw2_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw2_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_2->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw2_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms != NULL);
		ms = proxGw2_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw2_3->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);


		ms = proxGw3_1->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_1->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw3_1->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_1->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw3_2->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_2->findSecurityData(tst_type2);
		TEST_ASSERT(ms != NULL);
		ms = proxGw3_2->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_2->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		ms = proxGw3_3->findSecurityData(tst_type1);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type2);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type3);
		TEST_ASSERT(ms == NULL);
		ms = proxGw3_3->findSecurityData(tst_type4);
		TEST_ASSERT(ms == NULL);

		// cleanup
		mm.deleteModule(gw1);
		mm.deleteModule(gw2);
		mm.deleteModule(gw3);
	}

	void distanceAndConnectionLoop()
	{
		// Check that we support a closed loop or multi connection
		// of gateway and that the gateway chooses the best 
		// route to reach a module when more than one is possible
		// and that the distance is updated
		//
		// For this test, we use the following context:
		//	three gateway (gw1, gw2, gw3), each having a layer 3
		//	server and client transport.
		//	one gateway (gw4) having just a layer3 server
		//	gw1 connects on gw2, gw2 connects on gw3, gw3 connects on gw4.
		//	we check the module list and distance, then gw3 connects to gw1, closing
		//	the loop.
		//	we recheck module list and distance.
		//	We then disconnect gw3 from gw1 and recheck module list and distance.
		//
		//	Finally, we create a second connection from gw1 to gw2 and
		//	recheck module list and distances
		//	
		//                     /---<optional>---\
		//      (l3s) gw1 (l3c)                  (l3s) gw2 (l3c) ------ (l3s) gw3 (l3c) ------ (l3s) gw4
		//	      |            \----------------/                                   |
		//	      |                                                                 |
		//	      |                                                                 |
		//         \------------------<optional>-----------------------------------/
		//

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *gw1, *gw2, *gw3, *gw4;

		// create the modules
		gw1 = mm.createModule("StandardGateway", "gw1", "");
		gw2 = mm.createModule("StandardGateway", "gw2", "");
		gw3 = mm.createModule("StandardGateway", "gw3", "");
		gw4 = mm.createModule("StandardGateway", "gw4", "");

		TEST_ASSERT(gw1 != NULL);
		TEST_ASSERT(gw2 != NULL);
		TEST_ASSERT(gw3 != NULL);
		TEST_ASSERT(gw4 != NULL);

		// plug gateway into themselves
		NLNET::IModuleSocket *sGw1, *sGw2, *sGw3, *sGw4;
		sGw1 = mm.getModuleSocket("gw1");
		sGw2 = mm.getModuleSocket("gw2");
		sGw3 = mm.getModuleSocket("gw3");
		sGw4 = mm.getModuleSocket("gw4");

		TEST_ASSERT(sGw1 != NULL);
		TEST_ASSERT(sGw2 != NULL);
		TEST_ASSERT(sGw3 != NULL);
		TEST_ASSERT(sGw4 != NULL);

		gw1->plugModule(sGw1);
		gw2->plugModule(sGw2);
		gw3->plugModule(sGw3);
		gw4->plugModule(sGw4);

		string cmd;
		// create the transports
		cmd = "gw1.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw1.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw4.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// connect transport
		cmd = "gw1.transportCmd l3s(open port=8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportCmd l3s(open port=8062)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportCmd l3s(open port=8063)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw4.transportCmd l3s(open port=8064)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw1.transportCmd l3c(connect addr=localhost:8062)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw2.transportCmd l3c(connect addr=localhost:8063)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gw3.transportCmd l3c(connect addr=localhost:8064)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<15; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(40);
		}

		// check the modules list
		// ok, now, check that each gateways know the gateway it must know
		NLNET::IModuleGateway *gGw1, *gGw2, *gGw3, *gGw4;
		gGw1 = dynamic_cast<NLNET::IModuleGateway *>(gw1);
		TEST_ASSERT(gGw1 != NULL);
		gGw2 = dynamic_cast<NLNET::IModuleGateway *>(gw2);
		TEST_ASSERT(gGw2 != NULL);
		gGw3 = dynamic_cast<NLNET::IModuleGateway *>(gw3);
		TEST_ASSERT(gGw3 != NULL);
		gGw4 = dynamic_cast<NLNET::IModuleGateway *>(gw4);
		TEST_ASSERT(gGw4 != NULL);

		vector<NLNET::IModuleProxy*> proxList;
		gGw1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw3->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw4->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);

		// check the distance
		NLNET::IModuleProxy *gw1_1Prox, *gw1_2Prox, *gw1_3Prox, *gw1_4Prox; 
		NLNET::IModuleProxy *gw2_1Prox, *gw2_2Prox, *gw2_3Prox, *gw2_4Prox; 
		NLNET::IModuleProxy *gw3_1Prox, *gw3_2Prox, *gw3_3Prox, *gw3_4Prox; 
		NLNET::IModuleProxy *gw4_1Prox, *gw4_2Prox, *gw4_3Prox, *gw4_4Prox; 

		gw1_1Prox = retrieveModuleProxy(gGw1, "gw1");
		TEST_ASSERT(gw1_1Prox != NULL);
		TEST_ASSERT(gw1_1Prox->getModuleDistance() == 0);
		gw1_2Prox = retrieveModuleProxy(gGw2, "gw1");
		TEST_ASSERT(gw1_2Prox != NULL);
		TEST_ASSERT(gw1_2Prox->getModuleDistance() == 1);
		gw1_3Prox = retrieveModuleProxy(gGw3, "gw1");
		TEST_ASSERT(gw1_3Prox != NULL);
		TEST_ASSERT(gw1_3Prox->getModuleDistance() == 2);
		gw1_4Prox = retrieveModuleProxy(gGw4, "gw1");
		TEST_ASSERT(gw1_4Prox != NULL);
		TEST_ASSERT(gw1_4Prox->getModuleDistance() == 3);

		gw2_1Prox = retrieveModuleProxy(gGw1, "gw2");
		TEST_ASSERT(gw2_1Prox != NULL);
		TEST_ASSERT(gw2_1Prox->getModuleDistance() == 1);
		gw2_2Prox = retrieveModuleProxy(gGw2, "gw2");
		TEST_ASSERT(gw2_2Prox != NULL);
		TEST_ASSERT(gw2_2Prox->getModuleDistance() == 0);
		gw2_3Prox = retrieveModuleProxy(gGw3, "gw2");
		TEST_ASSERT(gw2_3Prox != NULL);
		TEST_ASSERT(gw2_3Prox->getModuleDistance() == 1);
		gw2_4Prox = retrieveModuleProxy(gGw4, "gw2");
		TEST_ASSERT(gw2_4Prox != NULL);
		TEST_ASSERT(gw2_4Prox->getModuleDistance() == 2);

		gw3_1Prox = retrieveModuleProxy(gGw1, "gw3");
		TEST_ASSERT(gw3_1Prox != NULL);
		TEST_ASSERT(gw3_1Prox->getModuleDistance() == 2);
		gw3_2Prox = retrieveModuleProxy(gGw2, "gw3");
		TEST_ASSERT(gw3_2Prox != NULL);
		TEST_ASSERT(gw3_2Prox->getModuleDistance() == 1);
		gw3_3Prox = retrieveModuleProxy(gGw3, "gw3");
		TEST_ASSERT(gw3_3Prox != NULL);
		TEST_ASSERT(gw3_3Prox->getModuleDistance() == 0);
		gw3_4Prox = retrieveModuleProxy(gGw4, "gw3");
		TEST_ASSERT(gw3_4Prox != NULL);
		TEST_ASSERT(gw3_4Prox->getModuleDistance() == 1);

		gw4_1Prox = retrieveModuleProxy(gGw1, "gw4");
		TEST_ASSERT(gw4_1Prox != NULL);
		TEST_ASSERT(gw4_1Prox->getModuleDistance() == 3);
		gw4_2Prox = retrieveModuleProxy(gGw2, "gw4");
		TEST_ASSERT(gw4_2Prox != NULL);
		TEST_ASSERT(gw4_2Prox->getModuleDistance() == 2);
		gw4_3Prox = retrieveModuleProxy(gGw3, "gw4");
		TEST_ASSERT(gw4_3Prox != NULL);
		TEST_ASSERT(gw4_3Prox->getModuleDistance() == 1);
		gw4_4Prox = retrieveModuleProxy(gGw4, "gw4");
		TEST_ASSERT(gw4_4Prox != NULL);
		TEST_ASSERT(gw4_4Prox->getModuleDistance() == 0);

		// now, connect gw3 to gw1
		cmd = "gw3.transportCmd l3c(connect addr=localhost:8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check module list
		proxList.clear();
		gGw1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw3->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw4->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);

		// check the distances
		gw1_1Prox = retrieveModuleProxy(gGw1, "gw1");
		TEST_ASSERT(gw1_1Prox != NULL);
		TEST_ASSERT(gw1_1Prox->getModuleDistance() == 0);
		gw1_2Prox = retrieveModuleProxy(gGw2, "gw1");
		TEST_ASSERT(gw1_2Prox != NULL);
		TEST_ASSERT(gw1_2Prox->getModuleDistance() == 1);
		gw1_3Prox = retrieveModuleProxy(gGw3, "gw1");
		TEST_ASSERT(gw1_3Prox != NULL);
		TEST_ASSERT(gw1_3Prox->getModuleDistance() == 1);
		gw1_4Prox = retrieveModuleProxy(gGw4, "gw1");
		TEST_ASSERT(gw1_4Prox != NULL);
		TEST_ASSERT(gw1_4Prox->getModuleDistance() == 2);

		gw2_1Prox = retrieveModuleProxy(gGw1, "gw2");
		TEST_ASSERT(gw2_1Prox != NULL);
		TEST_ASSERT(gw2_1Prox->getModuleDistance() == 1);
		gw2_2Prox = retrieveModuleProxy(gGw2, "gw2");
		TEST_ASSERT(gw2_2Prox != NULL);
		TEST_ASSERT(gw2_2Prox->getModuleDistance() == 0);
		gw2_3Prox = retrieveModuleProxy(gGw3, "gw2");
		TEST_ASSERT(gw2_3Prox != NULL);
		TEST_ASSERT(gw2_3Prox->getModuleDistance() == 1);
		gw2_4Prox = retrieveModuleProxy(gGw4, "gw2");
		TEST_ASSERT(gw2_4Prox != NULL);
		TEST_ASSERT(gw2_4Prox->getModuleDistance() == 2);

		gw3_1Prox = retrieveModuleProxy(gGw1, "gw3");
		TEST_ASSERT(gw3_1Prox != NULL);
		TEST_ASSERT(gw3_1Prox->getModuleDistance() == 1);
		gw3_2Prox = retrieveModuleProxy(gGw2, "gw3");
		TEST_ASSERT(gw3_2Prox != NULL);
		TEST_ASSERT(gw3_2Prox->getModuleDistance() == 1);
		gw3_3Prox = retrieveModuleProxy(gGw3, "gw3");
		TEST_ASSERT(gw3_3Prox != NULL);
		TEST_ASSERT(gw3_3Prox->getModuleDistance() == 0);
		gw3_4Prox = retrieveModuleProxy(gGw4, "gw3");
		TEST_ASSERT(gw3_4Prox != NULL);
		TEST_ASSERT(gw3_4Prox->getModuleDistance() == 1);

		gw4_1Prox = retrieveModuleProxy(gGw1, "gw4");
		TEST_ASSERT(gw4_1Prox != NULL);
		TEST_ASSERT(gw4_1Prox->getModuleDistance() == 2);
		gw4_2Prox = retrieveModuleProxy(gGw2, "gw4");
		TEST_ASSERT(gw4_2Prox != NULL);
		TEST_ASSERT(gw4_2Prox->getModuleDistance() == 2);
		gw4_3Prox = retrieveModuleProxy(gGw3, "gw4");
		TEST_ASSERT(gw4_3Prox != NULL);
		TEST_ASSERT(gw4_3Prox->getModuleDistance() == 1);
		gw4_4Prox = retrieveModuleProxy(gGw4, "gw4");
		TEST_ASSERT(gw4_4Prox != NULL);
		TEST_ASSERT(gw4_4Prox->getModuleDistance() == 0);

		// close gw3 to gw1
		cmd = "gw3.transportCmd l3c(close connId=1)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check module list
		proxList.clear();
		gGw1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw3->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw4->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);

		// check the distances
		gw1_1Prox = retrieveModuleProxy(gGw1, "gw1");
		TEST_ASSERT(gw1_1Prox != NULL);
		TEST_ASSERT(gw1_1Prox->getModuleDistance() == 0);
		gw1_2Prox = retrieveModuleProxy(gGw2, "gw1");
		TEST_ASSERT(gw1_2Prox != NULL);
		TEST_ASSERT(gw1_2Prox->getModuleDistance() == 1);
		gw1_3Prox = retrieveModuleProxy(gGw3, "gw1");
		TEST_ASSERT(gw1_3Prox != NULL);
		TEST_ASSERT(gw1_3Prox->getModuleDistance() == 2);
		gw1_4Prox = retrieveModuleProxy(gGw4, "gw1");
		TEST_ASSERT(gw1_4Prox != NULL);
		TEST_ASSERT(gw1_4Prox->getModuleDistance() == 3);

		gw2_1Prox = retrieveModuleProxy(gGw1, "gw2");
		TEST_ASSERT(gw2_1Prox != NULL);
		TEST_ASSERT(gw2_1Prox->getModuleDistance() == 1);
		gw2_2Prox = retrieveModuleProxy(gGw2, "gw2");
		TEST_ASSERT(gw2_2Prox != NULL);
		TEST_ASSERT(gw2_2Prox->getModuleDistance() == 0);
		gw2_3Prox = retrieveModuleProxy(gGw3, "gw2");
		TEST_ASSERT(gw2_3Prox != NULL);
		TEST_ASSERT(gw2_3Prox->getModuleDistance() == 1);
		gw2_4Prox = retrieveModuleProxy(gGw4, "gw2");
		TEST_ASSERT(gw2_4Prox != NULL);
		TEST_ASSERT(gw2_4Prox->getModuleDistance() == 2);

		gw3_1Prox = retrieveModuleProxy(gGw1, "gw3");
		TEST_ASSERT(gw3_1Prox != NULL);
		TEST_ASSERT(gw3_1Prox->getModuleDistance() == 2);
		gw3_2Prox = retrieveModuleProxy(gGw2, "gw3");
		TEST_ASSERT(gw3_2Prox != NULL);
		TEST_ASSERT(gw3_2Prox->getModuleDistance() == 1);
		gw3_3Prox = retrieveModuleProxy(gGw3, "gw3");
		TEST_ASSERT(gw3_3Prox != NULL);
		TEST_ASSERT(gw3_3Prox->getModuleDistance() == 0);
		gw3_4Prox = retrieveModuleProxy(gGw4, "gw3");
		TEST_ASSERT(gw3_4Prox != NULL);
		TEST_ASSERT(gw3_4Prox->getModuleDistance() == 1);

		gw4_1Prox = retrieveModuleProxy(gGw1, "gw4");
		TEST_ASSERT(gw4_1Prox != NULL);
		TEST_ASSERT(gw4_1Prox->getModuleDistance() == 3);
		gw4_2Prox = retrieveModuleProxy(gGw2, "gw4");
		TEST_ASSERT(gw4_2Prox != NULL);
		TEST_ASSERT(gw4_2Prox->getModuleDistance() == 2);
		gw4_3Prox = retrieveModuleProxy(gGw3, "gw4");
		TEST_ASSERT(gw4_3Prox != NULL);
		TEST_ASSERT(gw4_3Prox->getModuleDistance() == 1);
		gw4_4Prox = retrieveModuleProxy(gGw4, "gw4");
		TEST_ASSERT(gw4_4Prox != NULL);
		TEST_ASSERT(gw4_4Prox->getModuleDistance() == 0);

		// make a double connection from gw1 to gw2
		cmd = "gw1.transportCmd l3c(connect addr=localhost:8062)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check module list
		proxList.clear();
		gGw1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw3->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		proxList.clear();
		gGw4->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);

		// check the distances
		gw1_1Prox = retrieveModuleProxy(gGw1, "gw1");
		TEST_ASSERT(gw1_1Prox != NULL);
		TEST_ASSERT(gw1_1Prox->getModuleDistance() == 0);
		gw1_2Prox = retrieveModuleProxy(gGw2, "gw1");
		TEST_ASSERT(gw1_2Prox != NULL);
		TEST_ASSERT(gw1_2Prox->getModuleDistance() == 1);
		gw1_3Prox = retrieveModuleProxy(gGw3, "gw1");
		TEST_ASSERT(gw1_3Prox != NULL);
		TEST_ASSERT(gw1_3Prox->getModuleDistance() == 2);
		gw1_4Prox = retrieveModuleProxy(gGw4, "gw1");
		TEST_ASSERT(gw1_4Prox != NULL);
		TEST_ASSERT(gw1_4Prox->getModuleDistance() == 3);

		gw2_1Prox = retrieveModuleProxy(gGw1, "gw2");
		TEST_ASSERT(gw2_1Prox != NULL);
		TEST_ASSERT(gw2_1Prox->getModuleDistance() == 1);
		gw2_2Prox = retrieveModuleProxy(gGw2, "gw2");
		TEST_ASSERT(gw2_2Prox != NULL);
		TEST_ASSERT(gw2_2Prox->getModuleDistance() == 0);
		gw2_3Prox = retrieveModuleProxy(gGw3, "gw2");
		TEST_ASSERT(gw2_3Prox != NULL);
		TEST_ASSERT(gw2_3Prox->getModuleDistance() == 1);
		gw2_4Prox = retrieveModuleProxy(gGw4, "gw2");
		TEST_ASSERT(gw2_4Prox != NULL);
		TEST_ASSERT(gw2_4Prox->getModuleDistance() == 2);

		gw3_1Prox = retrieveModuleProxy(gGw1, "gw3");
		TEST_ASSERT(gw3_1Prox != NULL);
		TEST_ASSERT(gw3_1Prox->getModuleDistance() == 2);
		gw3_2Prox = retrieveModuleProxy(gGw2, "gw3");
		TEST_ASSERT(gw3_2Prox != NULL);
		TEST_ASSERT(gw3_2Prox->getModuleDistance() == 1);
		gw3_3Prox = retrieveModuleProxy(gGw3, "gw3");
		TEST_ASSERT(gw3_3Prox != NULL);
		TEST_ASSERT(gw3_3Prox->getModuleDistance() == 0);
		gw3_4Prox = retrieveModuleProxy(gGw4, "gw3");
		TEST_ASSERT(gw3_4Prox != NULL);
		TEST_ASSERT(gw3_4Prox->getModuleDistance() == 1);

		gw4_1Prox = retrieveModuleProxy(gGw1, "gw4");
		TEST_ASSERT(gw4_1Prox != NULL);
		TEST_ASSERT(gw4_1Prox->getModuleDistance() == 3);
		gw4_2Prox = retrieveModuleProxy(gGw2, "gw4");
		TEST_ASSERT(gw4_2Prox != NULL);
		TEST_ASSERT(gw4_2Prox->getModuleDistance() == 2);
		gw4_3Prox = retrieveModuleProxy(gGw3, "gw4");
		TEST_ASSERT(gw4_3Prox != NULL);
		TEST_ASSERT(gw4_3Prox->getModuleDistance() == 1);
		gw4_4Prox = retrieveModuleProxy(gGw4, "gw4");
		TEST_ASSERT(gw4_4Prox != NULL);
		TEST_ASSERT(gw4_4Prox->getModuleDistance() == 0);

		// release modules
		mm.deleteModule(gw1);
		mm.deleteModule(gw2);
		mm.deleteModule(gw3);
		mm.deleteModule(gw4);
	}

	void firewalling()
	{
		// check that, with firewall mode enabled, unsafe root can only see protected
		// modules if they initiate the dialog first (i.e only a protected module can send
		// a message to an unsafe module).
		//
		// for this test, we have the following context :
		// 'master' : a gateway that accesses connection on two transports, one 'firewalled', the other one normal
		// 'peer1' : gateway connected to gateway 'master' on a firewalled transport
		// 'peer2' : gateway connected to gateway 'master' on a firewalled transport
		// 'other' : gateway connected to gateway 'master' on a classic transport
		//
		//	peer1 (l3c)-----\
		//			         >-|<- (l3s1/Firewalled) master (l3s2) ----- (l3c) other
		//	peer2 (l3c)-----/
		//
		//  'peer1' and 'peer2' must not see any module except modules that try to communicate with them
		//	'master' and 'other' must see 'peer1', 'peer2', 'master' and 'other'
		//
		//	Switching OFF the firewall should disclose all modules,
		//	switching ON then must throw an exception

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *peer1, *peer2, *master, *other;

		// create the modules
		peer1 = mm.createModule("StandardGateway", "peer1", "");
		peer2 = mm.createModule("StandardGateway", "peer2", "");
		master = mm.createModule("StandardGateway", "master", "");
		other = mm.createModule("StandardGateway", "other", "");

		TEST_ASSERT(peer1 != NULL);
		TEST_ASSERT(peer2 != NULL);
		TEST_ASSERT(master != NULL);
		TEST_ASSERT(other != NULL);

		// plug gateway in themselves
		NLNET::IModuleSocket *sPeer1, *sPeer2, *sMaster, *sOther;
		sPeer1 = mm.getModuleSocket("peer1");
		sPeer2 = mm.getModuleSocket("peer2");
		sMaster = mm.getModuleSocket("master");
		sOther = mm.getModuleSocket("other");

		TEST_ASSERT(sPeer1 != NULL);
		TEST_ASSERT(sPeer2 != NULL);
		TEST_ASSERT(sMaster != NULL);
		TEST_ASSERT(sOther != NULL);

		peer1->plugModule(sPeer1);
		peer2->plugModule(sPeer2);
		master->plugModule(sMaster);
		other->plugModule(sOther);

		string cmd;
		// create the transports
		cmd = "peer1.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer2.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportAdd L3Server l3s1";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportAdd L3Server l3s2";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "other.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// Set option and connect transport
		cmd = "master.transportOptions l3s1(Firewalled)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportCmd l3s1(open port=8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportCmd l3s2(open port=8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer1.transportCmd l3c(connect addr=localhost:8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer2.transportCmd l3c(connect addr=localhost:8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "other.transportCmd l3c(connect addr=localhost:8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// d'ho ! all done, now let's run some loop of update
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// ok, now, check that each gateway only knows the gateway it must know
		NLNET::IModuleGateway *gPeer1, *gPeer2, *gMaster, *gOther;
		gPeer1 = dynamic_cast<NLNET::IModuleGateway *>(peer1);
		TEST_ASSERT(gPeer1 != NULL);
		gPeer2 = dynamic_cast<NLNET::IModuleGateway *>(peer2);
		TEST_ASSERT(gPeer2 != NULL);
		gMaster = dynamic_cast<NLNET::IModuleGateway *>(master);
		TEST_ASSERT(gMaster != NULL);
		gOther = dynamic_cast<NLNET::IModuleGateway *>(other);
		TEST_ASSERT(gOther != NULL);

		vector<NLNET::IModuleProxy*> proxList;
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 1);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));

		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 1);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		
		// now send the debug 'PING' message from 'other' to 'peer1', and a message from 'master' to 'peer2'
		{
			NLNET::CMessage ping("DEBUG_MOD_PING");

			// retrieve peer1 proxy from other
			NLNET::IModuleProxy *peer1Prox = retrieveModuleProxy(gMaster, "peer1");
			TEST_ASSERT(peer1Prox != NULL);
			peer1Prox->sendModuleMessage(master, ping);
		}
		{
			NLNET::CMessage ping("DEBUG_MOD_PING");

			// retrieve peer1 proxy from other
			NLNET::IModuleProxy *peer2Prox = retrieveModuleProxy(gOther, "peer2");
			TEST_ASSERT(peer2Prox != NULL);
			peer2Prox->sendModuleMessage(other, ping);
		}

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check new proxy table and ping counter
		TEST_ASSERT(gPeer1->getReceivedPingCount() == 1);
		proxList.clear();
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));

		TEST_ASSERT(gPeer2->getReceivedPingCount() == 1);
		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		// now, remove firewall mode
		cmd = "master.transportOptions l3s1()";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check new proxy table and ping counter
		proxList.clear();
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		TEST_ASSERT(gPeer2->getReceivedPingCount() == 1);
		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		// no try reactivate firewall mode with active route
		cmd = "master.transportOptions l3s1(Firewalled)";
		TEST_THROWS(cr.execute(cmd, NLMISC::InfoLog()), NLNET::IModuleGateway::EGatewayFirewallBreak);


		// cleanup
		mm.deleteModule(peer1);
		mm.deleteModule(peer2);
		mm.deleteModule(master);
		mm.deleteModule(other);

	}

	void peerInvisible()
	{
		// check that, with peer invisible enable, the peer modules are effectively invisible,
		// and, also, check that other modules, on other route are visible.
		// for this test, we have the following context :
		// 'master' : a gateway that acces connection on to transport, on 'peer invisible', the other normal
		// 'peer1' : gateway connected to gateway 'master' on a peer invisible transport
		// 'peer2' : gateway connected to gateway 'master' on a peer invisible transport
		// 'other' : gateway connected to gateway 'master' on a classic transport
		//
		//	peer1 (l3c)-----\
		//			         >-- (l3s1/PeerInvisible) master (l3s2) ----- (l3c) other
		//	peer2 (l3c)-----/
		//
		//  'peer1' must see 'master' and 'other'
		//	'peer2' must see 'master' and 'other'
		//	'master' must see 'peer1', 'peer2' and 'other'
		//	'other' must see 'peer1', 'peer2' and 'master'
		//
		//	When switching the PeerInvisible option to OFF, peer1 and peer2 must see each other

		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *peer1, *peer2, *master, *other;

		// create the modules
		peer1 = mm.createModule("StandardGateway", "peer1", "");
		peer2 = mm.createModule("StandardGateway", "peer2", "");
		master = mm.createModule("StandardGateway", "master", "");
		other = mm.createModule("StandardGateway", "other", "");

		TEST_ASSERT(peer1 != NULL);
		TEST_ASSERT(peer2 != NULL);
		TEST_ASSERT(master != NULL);
		TEST_ASSERT(other != NULL);

		// plug gateway in themselves
		NLNET::IModuleSocket *sPeer1, *sPeer2, *sMaster, *sOther;
		sPeer1 = mm.getModuleSocket("peer1");
		sPeer2 = mm.getModuleSocket("peer2");
		sMaster = mm.getModuleSocket("master");
		sOther = mm.getModuleSocket("other");

		TEST_ASSERT(sPeer1 != NULL);
		TEST_ASSERT(sPeer2 != NULL);
		TEST_ASSERT(sMaster != NULL);
		TEST_ASSERT(sOther != NULL);

		peer1->plugModule(sPeer1);
		peer2->plugModule(sPeer2);
		master->plugModule(sMaster);
		other->plugModule(sOther);

		string cmd;
		// create the transports
		cmd = "peer1.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer2.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportAdd L3Server l3s1";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportAdd L3Server l3s2";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "other.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// Set option and connect transport
		cmd = "master.transportOptions l3s1(PeerInvisible)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportCmd l3s1(open port=8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "master.transportCmd l3s2(open port=8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer1.transportCmd l3c(connect addr=localhost:8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "peer2.transportCmd l3c(connect addr=localhost:8060)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "other.transportCmd l3c(connect addr=localhost:8061)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// d'ho ! all done, now let's run some loop of update
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// ok, now, check that each gateway only knows the gateway it must know
		NLNET::IModuleGateway *gPeer1, *gPeer2, *gMaster, *gOther;
		gPeer1 = dynamic_cast<NLNET::IModuleGateway *>(peer1);
		TEST_ASSERT(gPeer1 != NULL);
		gPeer2 = dynamic_cast<NLNET::IModuleGateway *>(peer2);
		TEST_ASSERT(gPeer2 != NULL);
		gMaster = dynamic_cast<NLNET::IModuleGateway *>(master);
		TEST_ASSERT(gMaster != NULL);
		gOther = dynamic_cast<NLNET::IModuleGateway *>(other);
		TEST_ASSERT(gOther != NULL);

		vector<NLNET::IModuleProxy*> proxList;
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 3);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 3);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		// now, remove the 'PeerInvisible' options
		cmd = "master.transportOptions l3s1()";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check new proxy table
		proxList.clear();
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));

		// now, re set the 'PeerInvisible' options
		cmd = "master.transportOptions l3s1(PeerInvisible)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// update the network
		for (uint i=0; i<7; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check new proxy table
		proxList.clear();
		gPeer1->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 3);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gPeer2->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 3);
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gMaster->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));

		proxList.clear();
		gOther->getModuleProxyList(proxList);
		TEST_ASSERT(proxList.size() == 4);
		TEST_ASSERT(lookForModuleProxy(proxList, "other"));
		TEST_ASSERT(lookForModuleProxy(proxList, "master"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer1"));
		TEST_ASSERT(lookForModuleProxy(proxList, "peer2"));
		// cleanup
		mm.deleteModule(peer1);
		mm.deleteModule(peer2);
		mm.deleteModule(master);
		mm.deleteModule(other);

	}

	void gwPlugUnplug()
	{
		// check that multiple plug/unplug operations work well
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *mod = mm.createModule("StandardGateway", "gw", "");
		TEST_ASSERT(mod != NULL);

		NLNET::IModuleSocket *socket = mm.getModuleSocket("gw");
		TEST_ASSERT(socket != NULL);
		mod->plugModule(socket);
		mod->unplugModule(socket);
		mod->plugModule(socket);
		mod->unplugModule(socket);
		mod->plugModule(socket);

		std::vector<NLNET::IModuleProxy*> result;
		socket->getModuleList(result);
		TEST_ASSERT(result.size() == 1);

		mod->unplugModule(socket);

		mm.deleteModule(mod);
	}

	void uniqueNameGenerator()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		mm.setUniqueNameRoot("foo");

		// create a simple module
		NLNET::IModule *mod = mm.createModule("ModuleType0", "mod", "");
		TEST_ASSERT(mod != NULL);
		TEST_ASSERT(mod->getModuleFullyQualifiedName() == "foo:mod");
		mm.deleteModule(mod);

		// reset the unique name to normal value
		mm.setUniqueNameRoot(string());

		mod = mm.createModule("ModuleType0", "mod", "");
		TEST_ASSERT(mod != NULL);
		TEST_ASSERT(mod->getModuleFullyQualifiedName() != "foo:mod");

		mm.deleteModule(mod);
	}

	void localMessageQueing()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		NLNET::IModuleGateway *gws = dynamic_cast<NLNET::IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);

		// get the socket interface of the gateway
		NLNET::IModuleSocket *socketGws = mm.getModuleSocket("gws");
		TEST_ASSERT(socketGws != NULL);

		// create two modules that will communicate localy
		NLNET::IModule *m1= mm.createModule("ModuleType0", "m1", "");
		TEST_ASSERT(m1!= NULL);
		NLNET::IModule *m2= mm.createModule("ModuleAsync", "m2", "");
		TEST_ASSERT(m2!= NULL);

		m1->plugModule(socketGws);
		m2->plugModule(socketGws);

		// update the networks
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(50);
		}

		// retrieve module proxy and send one ping to each other
		vector<NLNET::IModuleProxy*>	proxiesC;
		gws->getModuleProxyList(proxiesC);
		TEST_ASSERT(proxiesC.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxiesC, "m2"));
		NLNET::IModuleProxy *pm2 = retrieveModuleProxy(gws, "m2");
		TEST_ASSERT(pm2 != NULL);
		NLNET::CMessage aMessage("DEBUG_MOD_PING");
		pm2->sendModuleMessage(m1, aMessage);

		proxiesC.clear();
		gws->getModuleProxyList(proxiesC);
		TEST_ASSERT(proxiesC.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxiesC, "m1"));
		NLNET::IModuleProxy *pm1 = retrieveModuleProxy(gws, "m1");
		TEST_ASSERT(pm1 != NULL);
		aMessage = NLNET::CMessage("DEBUG_MOD_PING");
		pm1->sendModuleMessage(m2, aMessage);

		// check received ping count
		CModuleType0 *mod1 = dynamic_cast<CModuleType0*>(m1);
		TEST_ASSERT(mod1 != NULL);
		TEST_ASSERT(mod1->PingCount == 1);
		CModuleType0 *mod2 = dynamic_cast<CModuleType0*>(m2);
		TEST_ASSERT(mod2 != NULL);
		TEST_ASSERT(mod2->PingCount == 0);
	
		// update the networks
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(50);
		}

		// check received ping count
		TEST_ASSERT(mod1->PingCount == 1);
		TEST_ASSERT(mod2->PingCount == 1);

		// update the networks
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(50);
		}

		// check received ping count
		TEST_ASSERT(mod1->PingCount == 1);
		TEST_ASSERT(mod2->PingCount == 1);


		// cleanup
		mm.deleteModule(m1);
		mm.deleteModule(m2);
		mm.deleteModule(mods);
	}

	void moduleMessaging()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		// create two gateway an connect them, plug the gateway on themselves and send a message
		NLNET::IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		NLNET::IModuleGateway *gws = dynamic_cast<NLNET::IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);

		// plug the module in itself before opening connection
		NLNET::IModuleSocket *socketGws = mm.getModuleSocket("gws");
		TEST_ASSERT(socketGws != NULL);
		mods->plugModule(socketGws);

		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		NLNET::IModule *modc = mm.createModule("StandardGateway", "gwc", "");
		TEST_ASSERT(modc != NULL);
		NLNET::IModuleGateway *gwc = dynamic_cast<NLNET::IModuleGateway*>(modc);
		TEST_ASSERT(gwc != NULL);
		// add transport for client mode
		cmd = "gwc.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// plug the module in itself before opening connection
		NLNET::IModuleSocket *socketGwc = mm.getModuleSocket("gwc");
		TEST_ASSERT(socketGwc != NULL);
		modc->plugModule(socketGwc);

		// update the gateways...
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// send a message from gws to gwc using the proxy
		// First, get the proxy for the client (must be the second one)
		vector<NLNET::IModuleProxy*>	proxiesS;
		gws->getModuleProxyList(proxiesS);
		TEST_ASSERT(proxiesS.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxiesS, "gwc"));
		NLNET::CMessage aMessage("DEBUG_MOD_PING");
		proxiesS[1]->sendModuleMessage(mods, aMessage);

		// update the gateways...
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 1);

		// send two crossing messages simultaneously
		vector<NLNET::IModuleProxy*>	proxiesC;
		gwc->getModuleProxyList(proxiesC);
		TEST_ASSERT(proxiesC.size() == 2);
		TEST_ASSERT(lookForModuleProxy(proxiesC, "gws"));
		proxiesS[1]->sendModuleMessage(mods, aMessage);
		proxiesC[1]->sendModuleMessage(modc, aMessage);

		// update the gateways...
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}
		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 2);
		TEST_ASSERT(gws->getReceivedPingCount() == 1);
		

		// send with ISocket
		socketGws->sendModuleMessage(mods, proxiesS[1]->getModuleProxyId(), aMessage);
		// update the gateways...
		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}
		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 3);
		TEST_ASSERT(gws->getReceivedPingCount() == 1);

		// cleanup modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc);
		TEST_ASSERT(mm.getLocalModule("gwc") == NULL);
	}

	void moduleDisclosure()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
		NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

		NLNET::IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		NLNET::IModuleGateway *gws = dynamic_cast<NLNET::IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);

		TEST_ASSERT(gws->getProxyCount() == 0);

		// plug the module in itself before opening connection
		NLNET::IModuleSocket *socketGws = mm.getModuleSocket("gws");
		TEST_ASSERT(socketGws != NULL);
		mods->plugModule(socketGws);

		// now, there must be one proxy in the gateway
		TEST_ASSERT(gws->getProxyCount() == 1);
		vector<NLNET::IModuleProxy*>	proxies;
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == mods->getModuleId());

		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		NLNET::IModule *modc = mm.createModule("StandardGateway", "gwc", "");
		TEST_ASSERT(modc != NULL);
		NLNET::IModuleGateway *gwc = dynamic_cast<NLNET::IModuleGateway*>(modc);
		TEST_ASSERT(gwc != NULL);
		// add transport for client mode
		cmd = "gwc.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		for (uint i=0; i<5; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// The server must have not changed
		TEST_ASSERT(gws->getProxyCount() == 1);

		// The client must have one proxy
		TEST_ASSERT(gwc->getProxyCount() == 1);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gws") == proxies[0]->getModuleName().size() - 3);

		// plug the client module in itself after opening connection
		NLNET::IModuleSocket *socketGwc = mm.getModuleSocket("gwc");
		TEST_ASSERT(socketGwc != NULL);
		modc->plugModule(socketGwc);


		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// The server must have now the two modules
		TEST_ASSERT(gws->getProxyCount() == 2);
		proxies.clear();
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 2);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == mods->getModuleId());
		TEST_ASSERT(proxies[1]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[1]->getModuleName().find("gwc") == proxies[1]->getModuleName().size() - 3);

		// The client must have two module also
		TEST_ASSERT(gwc->getProxyCount() == 2);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 2);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gws") == proxies[1]->getModuleName().size() - 3);
		TEST_ASSERT(proxies[1]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[1]->getForeignModuleId() == modc->getModuleId());


		// unplug the client module in itself after opening connection
		mods->unplugModule(socketGws);

		for (uint i=0; i<4; ++i)
		{
			NLMISC::nlSleep(100);
			mm.updateModules();
		}

		// The server must have one module left
		TEST_ASSERT(gws->getProxyCount() == 1);
		proxies.clear();
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gwc") == proxies[0]->getModuleName().size() - 3);

		// The client must have one module left
		TEST_ASSERT(gwc->getProxyCount() == 1);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == modc->getModuleId());
		
		// Dump the module state
		cmd = "gws.dump";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc.dump";
		TEST_ASSERT(cr.execute(cmd, NLMISC::InfoLog()));

		// cleanup modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc);
		TEST_ASSERT(mm.getLocalModule("gwc") == NULL);
	}

	void connectGateways()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		NLNET::IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		NLNET::IModuleGateway *gws = dynamic_cast<NLNET::IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);
		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
			
		NLNET::IModule *modc1 = mm.createModule("StandardGateway", "gwc1", "");
		TEST_ASSERT(modc1 != NULL);
		NLNET::IModuleGateway *gwc1 = dynamic_cast<NLNET::IModuleGateway*>(modc1);
		TEST_ASSERT(gwc1 != NULL);
		// add transport for client mode
		cmd = "gwc1.transportAdd L3Client l3c";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc1.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		for (uint i=0; i<4; ++i)
		{
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		TEST_ASSERT(gws->getRouteCount() == 1);
		TEST_ASSERT(gwc1->getRouteCount() == 1);
			
		// do a second connect to the server for stress
		// add transport for client mode
		cmd = "gwc1.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// create third gateway
		NLNET::IModule *modc2 = mm.createModule("StandardGateway", "gwc2", "");
		TEST_ASSERT(modc2 != NULL);
		NLNET::IModuleGateway *gwc2 = dynamic_cast<NLNET::IModuleGateway*>(modc2);
		TEST_ASSERT(gwc2 != NULL);
		// add transport for client mode
		cmd = "gwc2.transportAdd L3Client l3c";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc2.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<4; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			NLMISC::nlSleep(100);
			mm.updateModules();
		}

		TEST_ASSERT(gws->getRouteCount() == 3);
		TEST_ASSERT(gwc1->getRouteCount() == 2);
		TEST_ASSERT(gwc2->getRouteCount() == 1);

		// dump the gateways state
		cmd = "gws.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc1.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gwc2.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// cleanup the modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc1);
		TEST_ASSERT(mm.getLocalModule("gwc1") == NULL);
		mm.deleteModule(modc2);
		TEST_ASSERT(mm.getLocalModule("gwc2") == NULL);
	}

	void gatewayTransportManagement()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		// create a gateway module
		NLNET::IModule *mod = mm.createModule("StandardGateway", "gw", "");
		TEST_ASSERT(mod != NULL);
		NLNET::IModuleGateway *gw = dynamic_cast<NLNET::IModuleGateway*>(mod);
		TEST_ASSERT(gw != NULL);

		// Create a layer 3 server transport
		// send a transport creation command
		string cmd = "gw.transportAdd L3Server l3s";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		NLNET::IGatewayTransport *transportL3s = gw->getGatewayTransport("l3s");
		TEST_ASSERT(transportL3s != NULL);

		// send a transport command
		cmd = "gw.transportCmd l3s(open port=6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// Create a layer 3 client transport
		// send a transport creation command
		cmd = "gw.transportAdd L3Client l3c";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		NLNET::IGatewayTransport *transportL3c = gw->getGatewayTransport("l3c");
		TEST_ASSERT(transportL3c != NULL);

		// send a transport command
		cmd = "gw.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<4; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		TEST_ASSERT(transportL3s->getRouteCount() == 1);	
		TEST_ASSERT(transportL3c->getRouteCount() == 1);
		TEST_ASSERT(gw->getRouteCount() == 2);
		
		// dump the gateways state
		cmd = "gw.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		
		// close all connections
		cmd = "gw.transportCmd l3s(close)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		
		cmd = "gw.transportCmd l3c(close connId=0)";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<4; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		TEST_ASSERT(transportL3s->getRouteCount() == 0);
		TEST_ASSERT(transportL3c->getRouteCount() == 0);
		TEST_ASSERT(gw->getRouteCount() == 0);

		// Remove transports
		cmd = "gw.transportRemove l3s";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
		cmd = "gw.transportRemove l3c";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		TEST_ASSERT(gw->getGatewayTransport("l3c") == NULL);
		TEST_ASSERT(gw->getGatewayTransport("l3s") == NULL);
		TEST_ASSERT(gw->getTransportCount() == 0);

		// update the module to update the network callback client and server
		for (uint i=0; i<4; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			mm.updateModules();
			NLMISC::nlSleep(100);
		}

		// cleanup the modules
		mm.deleteModule(mod);
		TEST_ASSERT(mm.getLocalModule("gw") == NULL);
	}

/*	void moduleManagerCommands()
	{
		string cmd;
		// load a library
		cmd = "moduleManager.loadLibrary net_module_lib_test/net_module_lib_test";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// dump the module state
		cmd = "moduleManager.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// create a module
		cmd = "moduleManager.createModule ModuleType1 AModuleName";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// dump the module state
		cmd = "moduleManager.dump";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));

		// delete the module
		cmd = "moduleManager.deleteModule AModuleName";
		TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute(cmd, NLMISC::InfoLog()));
	}*/

	void plugLocalGateway()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		NLNET::IModule *gateway1 = mm.createModule("LocalGateway", "g1", "");
		TEST_ASSERT(gateway1 != NULL);
		NLNET::IModule *gateway2 = mm.createModule("LocalGateway", "g2", "");
		TEST_ASSERT(gateway2 != NULL);

		NLNET::IModuleSocket *socket1 = mm.getModuleSocket("g1");
		TEST_ASSERT(socket1 != NULL);
		NLNET::IModuleSocket *socket2 = mm.getModuleSocket("g2");
		TEST_ASSERT(socket2 != NULL);
		gateway1->plugModule(socket1);
		gateway1->plugModule(socket2);
		gateway2->plugModule(socket1);
		gateway2->plugModule(socket2);

		mm.deleteModule(gateway1);
		TEST_ASSERT(mm.getLocalModule("g1") == NULL);
		mm.deleteModule(gateway2);
		TEST_ASSERT(mm.getLocalModule("g2") == NULL);
	}

	void createLocalGateway()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		NLNET::IModule *gateway = mm.createModule("LocalGateway", "localGateway", "");
		TEST_ASSERT(gateway != NULL);

		NLNET::IModule *mod1 = mm.createModule("ModuleType0", "plugged1", "");
		TEST_ASSERT(mod1 != NULL);
		NLNET::IModule *mod2 = mm.createModule("ModuleType0", "plugged2", "");
		TEST_ASSERT(mod2 != NULL);

		NLNET::IModuleSocket *socket = mm.getModuleSocket("localGateway");
		TEST_ASSERT(socket != NULL);
		mod1->plugModule(socket);
		mod2->plugModule(socket);

		mm.deleteModule(mod1);
		TEST_ASSERT(mm.getLocalModule("plugged1") == NULL);
		mm.deleteModule(mod2);
		TEST_ASSERT(mm.getLocalModule("plugged2") == NULL);

		mm.deleteModule(gateway);
		TEST_ASSERT(mm.getLocalModule("localGateway") == NULL);
	}

/*	void unloadModuleLib()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		CRefPtr<IModule> module1 = mm.createModule("ModuleType1", "TheModule2", "the args");
		TEST_ASSERT(module1 != NULL);

		TEST_ASSERT(mm.unloadModuleLibrary("net_module_lib_test"));

		// the module must have been deleted
		TEST_ASSERT(module1 == NULL);

		TModulePtr module2 = mm.createModule("ModuleType1", "TheModuleThatCantBeCreated", "the args");
		TEST_ASSERT(module2 == NULL);
	}*/

	void failedInit()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		NLNET::IModule *module = mm.createModule("ModuleType0", "FailingInit", "FAIL");
		TEST_ASSERT(module == NULL);
	}

/*	void deleteModule()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		IModule *module = mm.createModule("ModuleType1", "TheModuleToDelete", "the args");
		TEST_ASSERT(module != NULL);

		CRefPtr<IModule> checkPtr(module);

		mm.deleteModule(module);
		TEST_ASSERT(checkPtr == NULL);
	}*/

/*	void createModule()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		TModulePtr module = mm.createModule("ModuleType1", "TheModule", "the args");
		TEST_ASSERT(module != NULL);

		TEST_ASSERT(module->getModuleClassName() == "ModuleType1");
		TEST_ASSERT(module->getModuleName() == "TheModule");

		string lh;
		if (IService::isServiceInitialized())
			lh = IService::getInstance()->getHostName();
		else
			lh = ::NLNET::CInetAddress::localHost().hostName();
		string fqmn = lh+":"+toString(getpid())+":TheModule";

		TEST_ASSERT(module->getModuleFullyQualifiedName() == fqmn);
	}*/

/*	void loadModuleLib()
	{
		string moduleLibName = "net_module_lib_test/net_module_lib_test";

		IModuleManager &mm = IModuleManager::getInstance();
		TEST_ASSERT(mm.loadModuleLibrary(moduleLibName));

		vector<string>	moduleList;
		mm.getAvailableModuleClassList(moduleList);

		TEST_ASSERT(moduleList.size() == 6);
		TEST_ASSERT(moduleList[0] == "LocalGateway");
		TEST_ASSERT(moduleList[1] == "ModuleAsync");
		TEST_ASSERT(moduleList[2] == "ModuleType0");
		TEST_ASSERT(moduleList[3] == "ModuleType1");
		TEST_ASSERT(moduleList[4] == "ModuleType2");
		TEST_ASSERT(moduleList[5] == "StandardGateway");
	}*/

	void localModuleFactory()
	{
		NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();

		vector<string>	moduleList;
		mm.getAvailableModuleClassList(moduleList);

		TEST_ASSERT(moduleList.size() == 4);
		TEST_ASSERT(moduleList[0] == "LocalGateway");
		TEST_ASSERT(moduleList[1] == "ModuleAsync");
		TEST_ASSERT(moduleList[2] == "ModuleType0");
		TEST_ASSERT(moduleList[3] == "StandardGateway");
	}

	void testModuleInitInfoBadParsing()
	{
		NLNET::TParsedCommandLine	mif;

		string	paramString = " a=1   b=2   ( b=1) ";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = " lswkd ,fpqoj(( cruq fzemfwijf ujr wmozejifp_zujf woijpc_u ' ";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a ( b=2";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a  b=2)";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a  b=2\"toto\"";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "=a";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a(=b)";
		TEST_ASSERT(!mif.parseParamList(paramString));
	}

	void testModuleInitInfoQuering()
	{
		NLNET::TParsedCommandLine	mif;

		string	paramString = " a=1   b=2   sub   ( y=22 zzzz=12 subsub (g=\"bean in box\" z=2) ) "; 

		TEST_ASSERT(mif.parseParamList(paramString));

		TEST_ASSERT(mif.getParam("a") != NULL);
		TEST_ASSERT(mif.getParam("a") == mif.SubParams[0]);

		TEST_ASSERT(mif.getParam("sub") != NULL);
		TEST_ASSERT(mif.getParam("sub") == mif.SubParams[2]);

		TEST_ASSERT(mif.getParam("foo") == NULL);

		TEST_ASSERT(mif.getParam("sub.subsub.g") != NULL);
		TEST_ASSERT(mif.getParam("sub.subsub.g") == mif.SubParams[2]->SubParams[2]->SubParams[0]);
	}

	void testModuleInitInfoParsing()
	{
		NLNET::TParsedCommandLine	mif;

		string	paramString = "a"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a=1"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a(b=1)"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a a a a"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		TEST_ASSERT(mif.SubParams.size() == 4);
		paramString = " a ( b=1 )"; 
		TEST_ASSERT(mif.parseParamList(paramString));

		paramString = " a=1   b=2   sub   ( y=22 zzzz=12 subsub (g=\"bean in box\" z=2) ) "; 

		TEST_ASSERT(mif.parseParamList(paramString));

		TEST_ASSERT(mif.SubParams.size() == 3);

		TEST_ASSERT(mif.SubParams[0]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[0]->ParamName == "a");
		TEST_ASSERT(mif.SubParams[0]->ParamValue == "1");

		TEST_ASSERT(mif.SubParams[1]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[1]->ParamName == "b");
		TEST_ASSERT(mif.SubParams[1]->ParamValue == "2");

		TEST_ASSERT(mif.SubParams[2]->SubParams.size() == 3);
		TEST_ASSERT(mif.SubParams[2]->ParamName == "sub");
		TEST_ASSERT(mif.SubParams[2]->ParamValue.empty());

		TEST_ASSERT(mif.SubParams[2]->SubParams[0]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2]->SubParams[0]->ParamName == "y");
		TEST_ASSERT(mif.SubParams[2]->SubParams[0]->ParamValue == "22");
	
		TEST_ASSERT(mif.SubParams[2]->SubParams[1]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2]->SubParams[1]->ParamName == "zzzz");
		TEST_ASSERT(mif.SubParams[2]->SubParams[1]->ParamValue == "12");
	
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams.size() == 2);
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->ParamName == "subsub");
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->ParamValue.empty());
	
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[0]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[0]->ParamName == "g");
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[0]->ParamValue == "bean in box");
	
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[1]->SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[1]->ParamName == "z");
		TEST_ASSERT(mif.SubParams[2]->SubParams[2]->SubParams[1]->ParamValue == "2");
	}
};

#endif
