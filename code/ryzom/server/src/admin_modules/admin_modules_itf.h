// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef ADMIN_MODULES_ITF
#define ADMIN_MODULES_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/callback_adaptor.h"

#include "nel/misc/time_nl.h"

namespace ADMIN
{

	class TGraphData;

	class TGraphDatas;

	class THighRezData;

	class THighRezDatas;

	class TServiceStatus;

	// This is the interface used by PHP to call methods
	// on the Admin service module

	class CAdminServiceWebItf
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"GCMD",	CAdminServiceWebItf::cb_globalCmd	},
				{	"CCMD",	CAdminServiceWebItf::cb_controlCmd	},
				{	"SCMD",	CAdminServiceWebItf::cb_serviceCmd	},
				{	"GSO",	CAdminServiceWebItf::cb_getShardOrders	},
				{	"GS",	CAdminServiceWebItf::cb_getStates	},
				{	"GHRGI",	CAdminServiceWebItf::cb_getHighRezGraphInfo	},
				{	"GHRG",	CAdminServiceWebItf::cb_getHighRezGraph	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CAdminServiceWeb__cbConnection);
			CAdminServiceWebItf *_this = reinterpret_cast<CAdminServiceWebItf *>(arg);

			_this->on_CAdminServiceWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CAdminServiceWeb__cbDisconnection);
			CAdminServiceWebItf *_this = reinterpret_cast<CAdminServiceWebItf *>(arg);

			_this->on_CAdminServiceWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the
		 *	interface).
		 */
		CAdminServiceWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
		{
			if (replacementAdaptor == NULL)
			{
				// use default callback server
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(new CNelCallbackServerAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(replacementAdaptor);
			}
		}

		virtual ~CAdminServiceWebItf()
		{
		}

		/// Open the interface socket in the specified port
		void openItf(uint16 port)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;



			getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);

			_CallbackServer->setConnectionCallback (_cbConnection, this);
			_CallbackServer->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackServer->init(port);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch method invokation.
		 */
		void update()
		{
			H_AUTO(CAdminServiceWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CAdminServiceWeb : Exception launch in callback server update");
			}
		}


		void commandResult(NLNET::TSockId dest, const std::string &serviceAlias, const std::string &result)
		{
			H_AUTO(commandResult_commandResult);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::commandResult called");
#endif
			NLNET::CMessage message("CMDR");
			nlWrite(message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(message, serial, const_cast < std::string& > (result));

			_CallbackServer->send(message, dest);
		}

		static void cb_globalCmd (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(globalCmd_on_globalCmd);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_globalCmd received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	command;
			nlRead(message, serial, command);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_globalCmd : calling on_globalCmd");
#endif


			callback->on_globalCmd(from, command);

		}

		static void cb_controlCmd (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(controlCmd_on_controlCmd);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_controlCmd received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	serviceAlias;
			std::string	command;
			nlRead(message, serial, serviceAlias);
			nlRead(message, serial, command);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_controlCmd : calling on_controlCmd");
#endif


			callback->on_controlCmd(from, serviceAlias, command);

		}

		static void cb_serviceCmd (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(serviceCmd_on_serviceCmd);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_serviceCmd received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	serviceAlias;
			std::string	command;
			nlRead(message, serial, serviceAlias);
			nlRead(message, serial, command);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_serviceCmd : calling on_serviceCmd");
#endif


			callback->on_serviceCmd(from, serviceAlias, command);

		}

		static void cb_getShardOrders (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getShardOrders_on_getShardOrders);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getShardOrders received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getShardOrders : calling on_getShardOrders");
#endif

			std::vector<std::string> retValue;

			retValue = callback->on_getShardOrders(from);

			NLNET::CMessage retMsg("R_GSO");

			nlWrite(retMsg, serialCont, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_getStates (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getStates_on_getStates);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getStates received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getStates : calling on_getStates");
#endif

			std::vector<std::string> retValue;

			retValue = callback->on_getStates(from);

			NLNET::CMessage retMsg("R_GS");

			nlWrite(retMsg, serialCont, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_getHighRezGraphInfo (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getHighRezGraphInfo_on_getHighRezGraphInfo);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getHighRezGraphInfo received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	varAddr;
			nlRead(message, serial, varAddr);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getHighRezGraphInfo : calling on_getHighRezGraphInfo");
#endif

			std::vector<std::string> retValue;

			retValue = callback->on_getHighRezGraphInfo(from, varAddr);

			NLNET::CMessage retMsg("R_GHRGI");

			nlWrite(retMsg, serialCont, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_getHighRezGraph (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getHighRezGraph_on_getHighRezGraph);
#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getHighRezGraph received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CAdminServiceWebItf *callback = (CAdminServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	varAddr;
			uint32	startDate;
			uint32	endDate;
			uint32	milliStep;
			nlRead(message, serial, varAddr);
			nlRead(message, serial, startDate);
			nlRead(message, serial, endDate);
			nlRead(message, serial, milliStep);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWeb::cb_getHighRezGraph : calling on_getHighRezGraph");
#endif

			std::vector<std::string> retValue;

			retValue = callback->on_getHighRezGraph(from, varAddr, startDate, endDate, milliStep);

			NLNET::CMessage retMsg("R_GHRG");

			nlWrite(retMsg, serialCont, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CAdminServiceWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CAdminServiceWeb_Disconnection(NLNET::TSockId from) =0;


		// Send a command to the AS.
		// This is used to issue global commands like 'as.allStart' or 'as.allStop'.
		// The result is returned by the return message
		// serviceCmdResult.
		virtual void on_globalCmd(NLNET::TSockId from, const std::string &command) =0;

		// Send a service related command to the executor
		// (not to the controled service)
		// The result is returned by the return message
		// controlCmdResult.
		virtual void on_controlCmd(NLNET::TSockId from, const std::string &serviceAlias, const std::string &command) =0;

		// Send a command to a service.
		// The result is returned by the return message
		// serviceCmdResult.
		virtual void on_serviceCmd(NLNET::TSockId from, const std::string &serviceAlias, const std::string &command) =0;

		// Get the orders of each known shard.
		// The return value is a vector of string, one entry by shard
		virtual std::vector<std::string> on_getShardOrders(NLNET::TSockId from) =0;

		// Get the last known state of all services.
		// The return value is a vector of string, one entry by service
		virtual std::vector<std::string> on_getStates(NLNET::TSockId from) =0;

		// Get information about a high rez graph.
		// The return is a string array containing
		// the name of the var, the available sample
		// period as two unix date (start dans end)
		// and the number of samples available
		// If the var is not found, an empty array is returned
		virtual std::vector<std::string> on_getHighRezGraphInfo(NLNET::TSockId from, const std::string &varAddr) =0;

		// Get the data for a high resolution graph.
		// The return is a string array, each
		// string containing 'time:milliOffset:value
		// Set endDate to 0 to specify a start date relative
		// to the last sample date. In this case, start date
		// is interpreted as the number of second before
		// the last sample.
		virtual std::vector<std::string> on_getHighRezGraph(NLNET::TSockId from, const std::string &varAddr, uint32 startDate, uint32 endDate, uint32 milliStep) =0;

	};

		// This is the interface used by PHP to call methods
	// on the Admin service module

	/** This is the client side of the interface
	 *	Derive from this class to invoke method on the callback server
	 */

	class CAdminServiceWebClientItf
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"CMDR",	CAdminServiceWebClientItf::cb_commandResult	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CAdminServiceWebClientItf *_this = reinterpret_cast<CAdminServiceWebClientItf *>(arg);

			_this->on_CAdminServiceWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_commandResult"), std::string("CMDR")));

				initialized = true;
			}

			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;


			static std::string emptyString;

			return emptyString;

		}

		CAdminServiceWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
		{
			if (adaptorReplacement == NULL)
			{
				// use the default Nel adaptor
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(new CNelCallbackClientAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(adaptorReplacement);
			}
		}

		/// Connect the interface client to the callback server at the specified address and port
		virtual void connectItf(NLNET::CInetAddress address)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

			static bool callbackAdded = false;
			if (!callbackAdded)
			{

				getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
			}

			_CallbackClient->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackClient->connect(address);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch invokation returns.
		 */
		virtual void update()
		{
			H_AUTO(CAdminServiceWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CAdminServiceWeb : Exception launch in callback client update");
			}
		}

		// Send a command to the AS.
		// This is used to issue global commands like 'as.allStart' or 'as.allStop'.
		// The result is returned by the return message
		// serviceCmdResult.

		void globalCmd(const std::string &command)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::globalCmd called");
#endif
			NLNET::CMessage message("GCMD");
			nlWrite(message, serial, const_cast < std::string& > (command));

			_CallbackClient->send(message);
		}
		// Send a service related command to the executor
		// (not to the controled service)
		// The result is returned by the return message
		// controlCmdResult.

		void controlCmd(const std::string &serviceAlias, const std::string &command)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::controlCmd called");
#endif
			NLNET::CMessage message("CCMD");
			nlWrite(message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(message, serial, const_cast < std::string& > (command));

			_CallbackClient->send(message);
		}
		// Send a command to a service.
		// The result is returned by the return message
		// serviceCmdResult.

		void serviceCmd(const std::string &serviceAlias, const std::string &command)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::serviceCmd called");
#endif
			NLNET::CMessage message("SCMD");
			nlWrite(message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(message, serial, const_cast < std::string& > (command));

			_CallbackClient->send(message);
		}
		// Get the orders of each known shard.
		// The return value is a vector of string, one entry by shard

		void getShardOrders()
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::getShardOrders called");
#endif
			NLNET::CMessage message("GSO");

			_CallbackClient->send(message);
		}
		// Get the last known state of all services.
		// The return value is a vector of string, one entry by service

		void getStates()
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::getStates called");
#endif
			NLNET::CMessage message("GS");

			_CallbackClient->send(message);
		}
		// Get information about a high rez graph.
		// The return is a string array containing
		// the name of the var, the available sample
		// period as two unix date (start dans end)
		// and the number of samples available
		// If the var is not found, an empty array is returned

		void getHighRezGraphInfo(const std::string &varAddr)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::getHighRezGraphInfo called");
#endif
			NLNET::CMessage message("GHRGI");
			nlWrite(message, serial, const_cast < std::string& > (varAddr));

			_CallbackClient->send(message);
		}
		// Get the data for a high resolution graph.
		// The return is a string array, each
		// string containing 'time:milliOffset:value
		// Set endDate to 0 to specify a start date relative
		// to the last sample date. In this case, start date
		// is interpreted as the number of second before
		// the last sample.

		void getHighRezGraph(const std::string &varAddr, uint32 startDate, uint32 endDate, uint32 milliStep)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::getHighRezGraph called");
#endif
			NLNET::CMessage message("GHRG");
			nlWrite(message, serial, const_cast < std::string& > (varAddr));
			nlWrite(message, serial, startDate);
			nlWrite(message, serial, endDate);
			nlWrite(message, serial, milliStep);

			_CallbackClient->send(message);
		}

		static void cb_commandResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::cb_commandResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CAdminServiceWebClientItf *callback = (CAdminServiceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	serviceAlias;
			std::string	result;
			nlRead(message, serial, serviceAlias);
			nlRead(message, serial, result);


#ifdef NL_DEBUG
			nldebug("CAdminServiceWebClient::cb_commandResult : calling on_commandResult");
#endif

			callback->on_commandResult(from, serviceAlias, result);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CAdminServiceWebClient_Disconnection(NLNET::TSockId from) =0;


		virtual void on_commandResult(NLNET::TSockId from, const std::string &serviceAlias, const std::string &result) =0;

	};
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TGraphData
	{
	protected:
		//
		std::string	_ServiceAlias;
		//
		std::string	_VarName;
		//
		uint32	_SamplePeriod;
		//
		double	_Value;
	public:
		//
		const std::string &getServiceAlias() const
		{
			return _ServiceAlias;
		}

		std::string &getServiceAlias()
		{
			return _ServiceAlias;
		}


		void setServiceAlias(const std::string &value)
		{


				_ServiceAlias = value;


		}
			//
		const std::string &getVarName() const
		{
			return _VarName;
		}

		std::string &getVarName()
		{
			return _VarName;
		}


		void setVarName(const std::string &value)
		{


				_VarName = value;


		}
			//
		uint32 getSamplePeriod() const
		{
			return _SamplePeriod;
		}

		void setSamplePeriod(uint32 value)
		{

				_SamplePeriod = value;

		}
			//
		double getValue() const
		{
			return _Value;
		}

		void setValue(double value)
		{

				_Value = value;

		}

		bool operator == (const TGraphData &other) const
		{
			return _ServiceAlias == other._ServiceAlias
				&& _VarName == other._VarName
				&& _SamplePeriod == other._SamplePeriod
				&& _Value == other._Value;
		}


		// constructor
		TGraphData()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_ServiceAlias);
			s.serial(_VarName);
			s.serial(_SamplePeriod);
			s.serial(_Value);

		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TGraphDatas
	{
	protected:
		//
		uint32	_CurrentTime;
		//
		std::vector < TGraphData >	_Datas;
	public:
		//
		uint32 getCurrentTime() const
		{
			return _CurrentTime;
		}

		void setCurrentTime(uint32 value)
		{

				_CurrentTime = value;

		}
			//
		const std::vector < TGraphData > &getDatas() const
		{
			return _Datas;
		}

		std::vector < TGraphData > &getDatas()
		{
			return _Datas;
		}


		void setDatas(const std::vector < TGraphData > &value)
		{


				_Datas = value;


		}

		bool operator == (const TGraphDatas &other) const
		{
			return _CurrentTime == other._CurrentTime
				&& _Datas == other._Datas;
		}


		// constructor
		TGraphDatas()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CurrentTime);
			s.serialCont(_Datas);

		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class THighRezData
	{
	protected:
		//
		NLMISC::TTime	_SampleTick;
		//
		double	_Value;
	public:
		//
		NLMISC::TTime getSampleTick() const
		{
			return _SampleTick;
		}

		void setSampleTick(NLMISC::TTime value)
		{

				_SampleTick = value;

		}
			//
		double getValue() const
		{
			return _Value;
		}

		void setValue(double value)
		{

				_Value = value;

		}

		bool operator == (const THighRezData &other) const
		{
			return _SampleTick == other._SampleTick
				&& _Value == other._Value;
		}


		// constructor
		THighRezData()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_SampleTick);
			s.serial(_Value);

		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class THighRezDatas
	{
	protected:
		//
		std::string	_ServiceAlias;
		//
		std::string	_VarName;
		//
		uint32	_CurrentTime;
		//
		std::vector < THighRezData >	_Datas;
	public:
		//
		const std::string &getServiceAlias() const
		{
			return _ServiceAlias;
		}

		std::string &getServiceAlias()
		{
			return _ServiceAlias;
		}


		void setServiceAlias(const std::string &value)
		{


				_ServiceAlias = value;


		}
			//
		const std::string &getVarName() const
		{
			return _VarName;
		}

		std::string &getVarName()
		{
			return _VarName;
		}


		void setVarName(const std::string &value)
		{


				_VarName = value;


		}
			//
		uint32 getCurrentTime() const
		{
			return _CurrentTime;
		}

		void setCurrentTime(uint32 value)
		{

				_CurrentTime = value;

		}
			//
		const std::vector < THighRezData > &getDatas() const
		{
			return _Datas;
		}

		std::vector < THighRezData > &getDatas()
		{
			return _Datas;
		}


		void setDatas(const std::vector < THighRezData > &value)
		{


				_Datas = value;


		}

		bool operator == (const THighRezDatas &other) const
		{
			return _ServiceAlias == other._ServiceAlias
				&& _VarName == other._VarName
				&& _CurrentTime == other._CurrentTime
				&& _Datas == other._Datas;
		}


		// constructor
		THighRezDatas()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_ServiceAlias);
			s.serial(_VarName);
			s.serial(_CurrentTime);
			s.serialCont(_Datas);

		}


	private:


	};




	struct TShardOrders
	{
		enum TValues
		{
			so_autostart_on,
			so_autostart_off,
			/// the highest valid value in the enum
			last_enum_item = so_autostart_off,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(so_autostart_on, 0));
				indexTable.insert(std::make_pair(so_autostart_off, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_autostart_on)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_autostart_off)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TShardOrders()
			: _Value(invalid_val)
		{
		}
		TShardOrders(TValues value)
			: _Value(value)
		{
		}

		TShardOrders(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TShardOrders &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TShardOrders &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TShardOrders &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TShardOrders &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TShardOrders &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TShardOrders &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TRunningOrders
	{
		enum TValues
		{
			ro_deactivated,
			ro_activated,
			/// the highest valid value in the enum
			last_enum_item = ro_activated,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ro_deactivated, 0));
				indexTable.insert(std::make_pair(ro_activated, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ro_deactivated)
				NL_STRING_CONVERSION_TABLE_ENTRY(ro_activated)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRunningOrders()
			: _Value(invalid_val)
		{
		}
		TRunningOrders(TValues value)
			: _Value(value)
		{
		}

		TRunningOrders(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRunningOrders &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRunningOrders &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRunningOrders &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRunningOrders &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRunningOrders &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRunningOrders &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TRunningState
	{
		enum TValues
		{
			rs_stopped,
			rs_running,
			rs_online,
			/// the highest valid value in the enum
			last_enum_item = rs_online,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(rs_stopped, 0));
				indexTable.insert(std::make_pair(rs_running, 1));
				indexTable.insert(std::make_pair(rs_online, 2));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rs_stopped)
				NL_STRING_CONVERSION_TABLE_ENTRY(rs_running)
				NL_STRING_CONVERSION_TABLE_ENTRY(rs_online)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRunningState()
			: _Value(invalid_val)
		{
		}
		TRunningState(TValues value)
			: _Value(value)
		{
		}

		TRunningState(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRunningState &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRunningState &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRunningState &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRunningState &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRunningState &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRunningState &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TRunningTag
	{
		enum TValues
		{
			rt_chain_crashing,
			rt_locally_started,
			rt_locally_stopped,
			rt_globally_stopped,
			rt_stopped_for_patch,
			rt_externaly_started,
			rt_slow_to_stop,
			rt_slow_to_start,
			/// the highest valid value in the enum
			last_enum_item = rt_slow_to_start,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 8
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(rt_chain_crashing, 0));
				indexTable.insert(std::make_pair(rt_locally_started, 1));
				indexTable.insert(std::make_pair(rt_locally_stopped, 2));
				indexTable.insert(std::make_pair(rt_globally_stopped, 3));
				indexTable.insert(std::make_pair(rt_stopped_for_patch, 4));
				indexTable.insert(std::make_pair(rt_externaly_started, 5));
				indexTable.insert(std::make_pair(rt_slow_to_stop, 6));
				indexTable.insert(std::make_pair(rt_slow_to_start, 7));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_chain_crashing)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_locally_started)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_locally_stopped)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_globally_stopped)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_stopped_for_patch)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_externaly_started)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_slow_to_stop)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_slow_to_start)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRunningTag()
			: _Value(invalid_val)
		{
		}
		TRunningTag(TValues value)
			: _Value(value)
		{
		}

		TRunningTag(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRunningTag &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRunningTag &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRunningTag &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRunningTag &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRunningTag &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRunningTag &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TServiceStatus
	{
	protected:
		//
		std::string	_ShardName;
		//
		std::string	_ServiceLongName;
		//
		std::string	_ServiceShortName;
		//
		std::string	_ServiceAliasName;
		//
		TRunningState	_RunningState;
		//
		TRunningOrders	_RunningOrders;
		//
		std::set < TRunningTag >	_RunningTags;
		//
		std::string	_Status;
	public:
		//
		const std::string &getShardName() const
		{
			return _ShardName;
		}

		std::string &getShardName()
		{
			return _ShardName;
		}


		void setShardName(const std::string &value)
		{


				_ShardName = value;


		}
			//
		const std::string &getServiceLongName() const
		{
			return _ServiceLongName;
		}

		std::string &getServiceLongName()
		{
			return _ServiceLongName;
		}


		void setServiceLongName(const std::string &value)
		{


				_ServiceLongName = value;


		}
			//
		const std::string &getServiceShortName() const
		{
			return _ServiceShortName;
		}

		std::string &getServiceShortName()
		{
			return _ServiceShortName;
		}


		void setServiceShortName(const std::string &value)
		{


				_ServiceShortName = value;


		}
			//
		const std::string &getServiceAliasName() const
		{
			return _ServiceAliasName;
		}

		std::string &getServiceAliasName()
		{
			return _ServiceAliasName;
		}


		void setServiceAliasName(const std::string &value)
		{


				_ServiceAliasName = value;


		}
			//
		TRunningState getRunningState() const
		{
			return _RunningState;
		}

		void setRunningState(TRunningState value)
		{

				_RunningState = value;

		}
			//
		TRunningOrders getRunningOrders() const
		{
			return _RunningOrders;
		}

		void setRunningOrders(TRunningOrders value)
		{

				_RunningOrders = value;

		}
			//
		const std::set < TRunningTag > &getRunningTags() const
		{
			return _RunningTags;
		}

		std::set < TRunningTag > &getRunningTags()
		{
			return _RunningTags;
		}


		void setRunningTags(const std::set < TRunningTag > &value)
		{


				_RunningTags = value;


		}
			//
		const std::string &getStatus() const
		{
			return _Status;
		}

		std::string &getStatus()
		{
			return _Status;
		}


		void setStatus(const std::string &value)
		{


				_Status = value;


		}

		bool operator == (const TServiceStatus &other) const
		{
			return _ShardName == other._ShardName
				&& _ServiceLongName == other._ServiceLongName
				&& _ServiceShortName == other._ServiceShortName
				&& _ServiceAliasName == other._ServiceAliasName
				&& _RunningState == other._RunningState
				&& _RunningOrders == other._RunningOrders
				&& _RunningTags == other._RunningTags
				&& _Status == other._Status;
		}


		// constructor
		TServiceStatus()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_ShardName);
			s.serial(_ServiceLongName);
			s.serial(_ServiceShortName);
			s.serial(_ServiceAliasName);
			s.serial(_RunningState);
			s.serial(_RunningOrders);
			s.serialCont(_RunningTags);
			s.serial(_Status);

		}


	private:


	};



	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CAdminServiceSkel>	TInterceptor;
	protected:
		CAdminServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CAdminServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CAdminServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void upServiceUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void graphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void highRezGraphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void commandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CAdminServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// An AES send an update of the list of service up
		virtual void upServiceUpdate(NLNET::IModuleProxy *sender, const std::vector < TServiceStatus > &serviceStatus) =0;
		// An AES send graph data update
		virtual void graphUpdate(NLNET::IModuleProxy *sender, const TGraphDatas &graphDatas) =0;
		// An AES send high rez graph data update
		virtual void highRezGraphUpdate(NLNET::IModuleProxy *sender, const THighRezDatas &graphDatas) =0;
		// AES send back the result of execution of a command
		virtual void commandResult(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CAdminServiceSkel	*_LocalModuleSkel;


	public:
		CAdminServiceProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CAdminServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CAdminServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// An AES send an update of the list of service up
		void upServiceUpdate(NLNET::IModule *sender, const std::vector < TServiceStatus > &serviceStatus);
		// An AES send graph data update
		void graphUpdate(NLNET::IModule *sender, const TGraphDatas &graphDatas);
		// An AES send high rez graph data update
		void highRezGraphUpdate(NLNET::IModule *sender, const THighRezDatas &graphDatas);
		// AES send back the result of execution of a command
		void commandResult(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_upServiceUpdate(NLNET::CMessage &__message, const std::vector < TServiceStatus > &serviceStatus);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_graphUpdate(NLNET::CMessage &__message, const TGraphDatas &graphDatas);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_highRezGraphUpdate(NLNET::CMessage &__message, const THighRezDatas &graphDatas);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_commandResult(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &result);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminExecutorServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CAdminExecutorServiceSkel>	TInterceptor;
	protected:
		CAdminExecutorServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CAdminExecutorServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CAdminExecutorServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void setShardOrders_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void shutdownShard_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void controlCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void serviceCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void commandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void graphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void highRezGraphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void serviceStatusUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CAdminExecutorServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// AS send orders for a shard
		virtual void setShardOrders(NLNET::IModuleProxy *sender, const std::string &shardName, const TShardOrders &shardOrders) =0;
		// AS send a command to shutdown a shard with a delay
		virtual void shutdownShard(NLNET::IModuleProxy *sender, const std::string &shardName, uint32 delay) =0;
		// AS send a control command to this AES
		virtual void controlCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command) =0;
		// Send a command to a service.
		virtual void serviceCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command) =0;
		// AES client send back the result of execution of a command
		virtual void commandResult(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result) =0;
		// A service send graph data update
		virtual void graphUpdate(NLNET::IModuleProxy *sender, const TGraphDatas &graphDatas) =0;
		// A service high rez graph data update
		virtual void highRezGraphUpdate(NLNET::IModuleProxy *sender, const THighRezDatas &graphDatas) =0;
		// A service send an update of of it's status string
		virtual void serviceStatusUpdate(NLNET::IModuleProxy *sender, const std::string &status) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminExecutorServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CAdminExecutorServiceSkel	*_LocalModuleSkel;


	public:
		CAdminExecutorServiceProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CAdminExecutorServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CAdminExecutorServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// AS send orders for a shard
		void setShardOrders(NLNET::IModule *sender, const std::string &shardName, const TShardOrders &shardOrders);
		// AS send a command to shutdown a shard with a delay
		void shutdownShard(NLNET::IModule *sender, const std::string &shardName, uint32 delay);
		// AS send a control command to this AES
		void controlCmd(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command);
		// Send a command to a service.
		void serviceCmd(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command);
		// AES client send back the result of execution of a command
		void commandResult(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result);
		// A service send graph data update
		void graphUpdate(NLNET::IModule *sender, const TGraphDatas &graphDatas);
		// A service high rez graph data update
		void highRezGraphUpdate(NLNET::IModule *sender, const THighRezDatas &graphDatas);
		// A service send an update of of it's status string
		void serviceStatusUpdate(NLNET::IModule *sender, const std::string &status);
		// AS send orders for a shard

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_setShardOrders(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &shardName, const TShardOrders &shardOrders)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_setShardOrders(message , shardName, shardOrders);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// AS send a command to shutdown a shard with a delay

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_shutdownShard(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::string &shardName, uint32 delay)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_shutdownShard(message , shardName, delay);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// AS send a control command to this AES

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_controlCmd(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_controlCmd(message , commandId, serviceAlias, command);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setShardOrders(NLNET::CMessage &__message, const std::string &shardName, const TShardOrders &shardOrders);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_shutdownShard(NLNET::CMessage &__message, const std::string &shardName, uint32 delay);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_controlCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &command);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_serviceCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &command);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_commandResult(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_graphUpdate(NLNET::CMessage &__message, const TGraphDatas &graphDatas);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_highRezGraphUpdate(NLNET::CMessage &__message, const THighRezDatas &graphDatas);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_serviceStatusUpdate(NLNET::CMessage &__message, const std::string &status);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminExecutorServiceClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CAdminExecutorServiceClientSkel>	TInterceptor;
	protected:
		CAdminExecutorServiceClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CAdminExecutorServiceClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CAdminExecutorServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void serviceCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void serviceCmdNoReturn_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CAdminExecutorServiceClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// execute a command and return the result.
		virtual void serviceCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &command) =0;
		// Send a command to a service without waiting for the return value.
		virtual void serviceCmdNoReturn(NLNET::IModuleProxy *sender, const std::string &command) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdminExecutorServiceClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CAdminExecutorServiceClientSkel	*_LocalModuleSkel;


	public:
		CAdminExecutorServiceClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CAdminExecutorServiceClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CAdminExecutorServiceClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// execute a command and return the result.
		void serviceCmd(NLNET::IModule *sender, uint32 commandId, const std::string &command);
		// Send a command to a service without waiting for the return value.
		void serviceCmdNoReturn(NLNET::IModule *sender, const std::string &command);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_serviceCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &command);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_serviceCmdNoReturn(NLNET::CMessage &__message, const std::string &command);




	};

}

#endif
