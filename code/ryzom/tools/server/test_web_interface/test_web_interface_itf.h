
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef TEST_WEB_INTERFACE_ITF
#define TEST_WEB_INTERFACE_ITF
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/callback_adaptor.h"

namespace TWI
{
	
	// Test interface for web to C++ messaging

	class CTestInterfaceWebItf 
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;	

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"BT",	CTestInterfaceWebItf::cb_beginTest	},
				{	"S32",	CTestInterfaceWebItf::cb_sendUInt32	},
				{	"S8",	CTestInterfaceWebItf::cb_sendUInt8	},
				{	"SS",	CTestInterfaceWebItf::cb_sendString	},
				{	"SC1",	CTestInterfaceWebItf::cb_composite1	},
				{	"SC2",	CTestInterfaceWebItf::cb_composite2	},
				{	"SC3",	CTestInterfaceWebItf::cb_composite3	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterfaceWeb__cbConnection);
			CTestInterfaceWebItf *_this = reinterpret_cast<CTestInterfaceWebItf *>(arg);

			_this->on_CTestInterfaceWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterfaceWeb__cbDisconnection);
			CTestInterfaceWebItf *_this = reinterpret_cast<CTestInterfaceWebItf *>(arg);

			_this->on_CTestInterfaceWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the 
		 *	interface).
		 */
		CTestInterfaceWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
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

		virtual ~CTestInterfaceWebItf()
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
			H_AUTO(CTestInterfaceWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CTestInterfaceWeb : Exception launch in callback server update");
			}
		}


		void returnUInt32(NLNET::TSockId dest, uint32 i32)
		{
			H_AUTO(returnUInt32_returnUInt32);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnUInt32 called");
#endif
			NLNET::CMessage message("R32");
			nlWrite(message, serial, i32);

			_CallbackServer->send(message, dest);
		}

		void returnUInt8(NLNET::TSockId dest, uint8 i8)
		{
			H_AUTO(returnUInt8_returnUInt8);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnUInt8 called");
#endif
			NLNET::CMessage message("R8");
			nlWrite(message, serial, i8);

			_CallbackServer->send(message, dest);
		}

		void returnString(NLNET::TSockId dest, const std::string &str)
		{
			H_AUTO(returnString_returnString);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnString called");
#endif
			NLNET::CMessage message("RS");
			nlWrite(message, serial, const_cast < std::string& > (str));

			_CallbackServer->send(message, dest);
		}

		void returnComposite1(NLNET::TSockId dest, uint32 i32, uint8 i8, const std::string &str)
		{
			H_AUTO(returnComposite1_returnComposite1);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnComposite1 called");
#endif
			NLNET::CMessage message("RC1");
			nlWrite(message, serial, i32);
			nlWrite(message, serial, i8);
			nlWrite(message, serial, const_cast < std::string& > (str));

			_CallbackServer->send(message, dest);
		}

		void returnComposite2(NLNET::TSockId dest, const std::string &str1, const std::string &str2, const std::string &str3, const std::string &str4)
		{
			H_AUTO(returnComposite2_returnComposite2);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnComposite2 called");
#endif
			NLNET::CMessage message("RC2");
			nlWrite(message, serial, const_cast < std::string& > (str1));
			nlWrite(message, serial, const_cast < std::string& > (str2));
			nlWrite(message, serial, const_cast < std::string& > (str3));
			nlWrite(message, serial, const_cast < std::string& > (str4));

			_CallbackServer->send(message, dest);
		}

		void returnComposite3(NLNET::TSockId dest, uint8 i81, uint32 i321, const std::string &str1, uint8 i82, uint32 i322, const std::string &str2, uint8 i83, uint32 i323, const std::string &str3, uint8 i84, uint32 i324, const std::string &str4, uint32 i325, uint8 i85)
		{
			H_AUTO(returnComposite3_returnComposite3);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::returnComposite3 called");
#endif
			NLNET::CMessage message("RC3");
			nlWrite(message, serial, i81);
			nlWrite(message, serial, i321);
			nlWrite(message, serial, const_cast < std::string& > (str1));
			nlWrite(message, serial, i82);
			nlWrite(message, serial, i322);
			nlWrite(message, serial, const_cast < std::string& > (str2));
			nlWrite(message, serial, i83);
			nlWrite(message, serial, i323);
			nlWrite(message, serial, const_cast < std::string& > (str3));
			nlWrite(message, serial, i84);
			nlWrite(message, serial, i324);
			nlWrite(message, serial, const_cast < std::string& > (str4));
			nlWrite(message, serial, i325);
			nlWrite(message, serial, i85);

			_CallbackServer->send(message, dest);
		}

		static void cb_beginTest (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(beginTest_on_beginTest);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_beginTest received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_beginTest : calling on_beginTest");
#endif


			callback->on_beginTest(from);

		}

		static void cb_sendUInt32 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendUInt32_on_sendUInt32);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendUInt32 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			nlRead(message, serial, i32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendUInt32 : calling on_sendUInt32");
#endif


			callback->on_sendUInt32(from, i32);

		}

		static void cb_sendUInt8 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendUInt8_on_sendUInt8);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendUInt8 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint8	i8;
			nlRead(message, serial, i8);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendUInt8 : calling on_sendUInt8");
#endif


			callback->on_sendUInt8(from, i8);

		}

		static void cb_sendString (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendString_on_sendString);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendString received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	str;
			nlRead(message, serial, str);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_sendString : calling on_sendString");
#endif


			callback->on_sendString(from, str);

		}

		static void cb_composite1 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(composite1_on_composite1);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite1 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			uint8	i8;
			std::string	str;
			nlRead(message, serial, i32);
			nlRead(message, serial, i8);
			nlRead(message, serial, str);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite1 : calling on_composite1");
#endif


			callback->on_composite1(from, i32, i8, str);

		}

		static void cb_composite2 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(composite2_on_composite2);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite2 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	str1;
			std::string	str2;
			std::string	str3;
			std::string	str4;
			nlRead(message, serial, str1);
			nlRead(message, serial, str2);
			nlRead(message, serial, str3);
			nlRead(message, serial, str4);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite2 : calling on_composite2");
#endif


			callback->on_composite2(from, str1, str2, str3, str4);

		}

		static void cb_composite3 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(composite3_on_composite3);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite3 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebItf *callback = (CTestInterfaceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint8	i81;
			uint32	i321;
			std::string	str1;
			uint8	i82;
			uint32	i322;
			std::string	str2;
			uint8	i83;
			uint32	i323;
			std::string	str3;
			uint8	i84;
			uint32	i324;
			std::string	str4;
			uint32	i325;
			uint8	i85;
			nlRead(message, serial, i81);
			nlRead(message, serial, i321);
			nlRead(message, serial, str1);
			nlRead(message, serial, i82);
			nlRead(message, serial, i322);
			nlRead(message, serial, str2);
			nlRead(message, serial, i83);
			nlRead(message, serial, i323);
			nlRead(message, serial, str3);
			nlRead(message, serial, i84);
			nlRead(message, serial, i324);
			nlRead(message, serial, str4);
			nlRead(message, serial, i325);
			nlRead(message, serial, i85);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb::cb_composite3 : calling on_composite3");
#endif


			callback->on_composite3(from, i81, i321, str1, i82, i322, str2, i83, i323, str3, i84, i324, str4, i325, i85);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CTestInterfaceWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CTestInterfaceWeb_Disconnection(NLNET::TSockId from) =0;


		virtual void on_beginTest(NLNET::TSockId from) =0;

		virtual void on_sendUInt32(NLNET::TSockId from, uint32 i32) =0;

		virtual void on_sendUInt8(NLNET::TSockId from, uint8 i8) =0;

		virtual void on_sendString(NLNET::TSockId from, const std::string &str) =0;

		virtual void on_composite1(NLNET::TSockId from, uint32 i32, uint8 i8, const std::string &str) =0;

		virtual void on_composite2(NLNET::TSockId from, const std::string &str1, const std::string &str2, const std::string &str3, const std::string &str4) =0;

		virtual void on_composite3(NLNET::TSockId from, uint8 i81, uint32 i321, const std::string &str1, uint8 i82, uint32 i322, const std::string &str2, uint8 i83, uint32 i323, const std::string &str3, uint8 i84, uint32 i324, const std::string &str4, uint32 i325, uint8 i85) =0;

	};
	
		// Test interface for web to C++ messaging

	/** This is the client side of the interface 
	 *	Derive from this class to invoke method on the callback server
	 */	

	class CTestInterfaceWebClientItf 
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"R32",	CTestInterfaceWebClientItf::cb_returnUInt32	},
				{	"R8",	CTestInterfaceWebClientItf::cb_returnUInt8	},
				{	"RS",	CTestInterfaceWebClientItf::cb_returnString	},
				{	"RC1",	CTestInterfaceWebClientItf::cb_returnComposite1	},
				{	"RC2",	CTestInterfaceWebClientItf::cb_returnComposite2	},
				{	"RC3",	CTestInterfaceWebClientItf::cb_returnComposite3	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CTestInterfaceWebClientItf *_this = reinterpret_cast<CTestInterfaceWebClientItf *>(arg);

			_this->on_CTestInterfaceWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_returnUInt32"), std::string("R32")));
			messageNames.insert(std::make_pair(std::string("on_returnUInt8"), std::string("R8")));
			messageNames.insert(std::make_pair(std::string("on_returnString"), std::string("RS")));
			messageNames.insert(std::make_pair(std::string("on_returnComposite1"), std::string("RC1")));
			messageNames.insert(std::make_pair(std::string("on_returnComposite2"), std::string("RC2")));
			messageNames.insert(std::make_pair(std::string("on_returnComposite3"), std::string("RC3")));

				initialized = true;
			}
			
			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;
			

			static std::string emptyString;
			
			return emptyString;

		}
		
		CTestInterfaceWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
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
			H_AUTO(CTestInterfaceWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CTestInterfaceWeb : Exception launch in callback client update");
			}
		}


		void beginTest()
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::beginTest called");
#endif
			NLNET::CMessage message("BT");

			_CallbackClient->send(message);
		}

		void sendUInt32(uint32 i32)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::sendUInt32 called");
#endif
			NLNET::CMessage message("S32");
			nlWrite(message, serial, i32);

			_CallbackClient->send(message);
		}

		void sendUInt8(uint8 i8)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::sendUInt8 called");
#endif
			NLNET::CMessage message("S8");
			nlWrite(message, serial, i8);

			_CallbackClient->send(message);
		}

		void sendString(const std::string &str)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::sendString called");
#endif
			NLNET::CMessage message("SS");
			nlWrite(message, serial, const_cast < std::string& > (str));

			_CallbackClient->send(message);
		}

		void composite1(uint32 i32, uint8 i8, const std::string &str)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::composite1 called");
#endif
			NLNET::CMessage message("SC1");
			nlWrite(message, serial, i32);
			nlWrite(message, serial, i8);
			nlWrite(message, serial, const_cast < std::string& > (str));

			_CallbackClient->send(message);
		}

		void composite2(const std::string &str1, const std::string &str2, const std::string &str3, const std::string &str4)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::composite2 called");
#endif
			NLNET::CMessage message("SC2");
			nlWrite(message, serial, const_cast < std::string& > (str1));
			nlWrite(message, serial, const_cast < std::string& > (str2));
			nlWrite(message, serial, const_cast < std::string& > (str3));
			nlWrite(message, serial, const_cast < std::string& > (str4));

			_CallbackClient->send(message);
		}

		void composite3(uint8 i81, uint32 i321, const std::string &str1, uint8 i82, uint32 i322, const std::string &str2, uint8 i83, uint32 i323, const std::string &str3, uint8 i84, uint32 i324, const std::string &str4, uint32 i325, uint8 i85)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::composite3 called");
#endif
			NLNET::CMessage message("SC3");
			nlWrite(message, serial, i81);
			nlWrite(message, serial, i321);
			nlWrite(message, serial, const_cast < std::string& > (str1));
			nlWrite(message, serial, i82);
			nlWrite(message, serial, i322);
			nlWrite(message, serial, const_cast < std::string& > (str2));
			nlWrite(message, serial, i83);
			nlWrite(message, serial, i323);
			nlWrite(message, serial, const_cast < std::string& > (str3));
			nlWrite(message, serial, i84);
			nlWrite(message, serial, i324);
			nlWrite(message, serial, const_cast < std::string& > (str4));
			nlWrite(message, serial, i325);
			nlWrite(message, serial, i85);

			_CallbackClient->send(message);
		}

		static void cb_returnUInt32 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnUInt32 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			nlRead(message, serial, i32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnUInt32 : calling on_returnUInt32");
#endif

			callback->on_returnUInt32(from, i32);
		}

		static void cb_returnUInt8 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnUInt8 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint8	i8;
			nlRead(message, serial, i8);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnUInt8 : calling on_returnUInt8");
#endif

			callback->on_returnUInt8(from, i8);
		}

		static void cb_returnString (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnString received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	str;
			nlRead(message, serial, str);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnString : calling on_returnString");
#endif

			callback->on_returnString(from, str);
		}

		static void cb_returnComposite1 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite1 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			uint8	i8;
			std::string	str;
			nlRead(message, serial, i32);
			nlRead(message, serial, i8);
			nlRead(message, serial, str);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite1 : calling on_returnComposite1");
#endif

			callback->on_returnComposite1(from, i32, i8, str);
		}

		static void cb_returnComposite2 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite2 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	str1;
			std::string	str2;
			std::string	str3;
			std::string	str4;
			nlRead(message, serial, str1);
			nlRead(message, serial, str2);
			nlRead(message, serial, str3);
			nlRead(message, serial, str4);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite2 : calling on_returnComposite2");
#endif

			callback->on_returnComposite2(from, str1, str2, str3, str4);
		}

		static void cb_returnComposite3 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite3 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWebClientItf *callback = (CTestInterfaceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint8	i81;
			uint32	i321;
			std::string	str1;
			uint8	i82;
			uint32	i322;
			std::string	str2;
			uint8	i83;
			uint32	i323;
			std::string	str3;
			uint8	i84;
			uint32	i324;
			std::string	str4;
			uint32	i325;
			uint8	i85;
			nlRead(message, serial, i81);
			nlRead(message, serial, i321);
			nlRead(message, serial, str1);
			nlRead(message, serial, i82);
			nlRead(message, serial, i322);
			nlRead(message, serial, str2);
			nlRead(message, serial, i83);
			nlRead(message, serial, i323);
			nlRead(message, serial, str3);
			nlRead(message, serial, i84);
			nlRead(message, serial, i324);
			nlRead(message, serial, str4);
			nlRead(message, serial, i325);
			nlRead(message, serial, i85);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWebClient::cb_returnComposite3 : calling on_returnComposite3");
#endif

			callback->on_returnComposite3(from, i81, i321, str1, i82, i322, str2, i83, i323, str3, i84, i324, str4, i325, i85);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CTestInterfaceWebClient_Disconnection(NLNET::TSockId from) =0;


		virtual void on_returnUInt32(NLNET::TSockId from, uint32 i32) =0;

		virtual void on_returnUInt8(NLNET::TSockId from, uint8 i8) =0;

		virtual void on_returnString(NLNET::TSockId from, const std::string &str) =0;

		virtual void on_returnComposite1(NLNET::TSockId from, uint32 i32, uint8 i8, const std::string &str) =0;

		virtual void on_returnComposite2(NLNET::TSockId from, const std::string &str1, const std::string &str2, const std::string &str3, const std::string &str4) =0;

		virtual void on_returnComposite3(NLNET::TSockId from, uint8 i81, uint32 i321, const std::string &str1, uint8 i82, uint32 i322, const std::string &str2, uint8 i83, uint32 i323, const std::string &str3, uint8 i84, uint32 i324, const std::string &str4, uint32 i325, uint8 i85) =0;

	};
	// a second interface to stress the generator

	class CTestInterface2WebItf 
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;	

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"S322",	CTestInterface2WebItf::cb_sendInt32_2	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterface2Web__cbConnection);
			CTestInterface2WebItf *_this = reinterpret_cast<CTestInterface2WebItf *>(arg);

			_this->on_CTestInterface2Web_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterface2Web__cbDisconnection);
			CTestInterface2WebItf *_this = reinterpret_cast<CTestInterface2WebItf *>(arg);

			_this->on_CTestInterface2Web_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the 
		 *	interface).
		 */
		CTestInterface2WebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
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

		virtual ~CTestInterface2WebItf()
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
			H_AUTO(CTestInterface2Web_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CTestInterface2Web : Exception launch in callback server update");
			}
		}


		void returnInt32_2(NLNET::TSockId dest, uint32 i32)
		{
			H_AUTO(returnInt32_2_returnInt32_2);
#ifdef NL_DEBUG
			nldebug("CTestInterface2Web::returnInt32_2 called");
#endif
			NLNET::CMessage message("R322");
			nlWrite(message, serial, i32);

			_CallbackServer->send(message, dest);
		}

		static void cb_sendInt32_2 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendInt32_2_on_sendInt32_2);
#ifdef NL_DEBUG
			nldebug("CTestInterface2Web::cb_sendInt32_2 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterface2WebItf *callback = (CTestInterface2WebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			nlRead(message, serial, i32);


#ifdef NL_DEBUG
			nldebug("CTestInterface2Web::cb_sendInt32_2 : calling on_sendInt32_2");
#endif


			callback->on_sendInt32_2(from, i32);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CTestInterface2Web_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CTestInterface2Web_Disconnection(NLNET::TSockId from) =0;


		virtual void on_sendInt32_2(NLNET::TSockId from, uint32 i32) =0;

	};
	
		// a second interface to stress the generator

	/** This is the client side of the interface 
	 *	Derive from this class to invoke method on the callback server
	 */	

	class CTestInterface2WebClientItf 
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"R322",	CTestInterface2WebClientItf::cb_returnInt32_2	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CTestInterface2WebClientItf *_this = reinterpret_cast<CTestInterface2WebClientItf *>(arg);

			_this->on_CTestInterface2WebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_returnInt32_2"), std::string("R322")));

				initialized = true;
			}
			
			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;
			

			static std::string emptyString;
			
			return emptyString;

		}
		
		CTestInterface2WebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
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
			H_AUTO(CTestInterface2Web_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CTestInterface2Web : Exception launch in callback client update");
			}
		}


		void sendInt32_2(uint32 i32)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterface2WebClient::sendInt32_2 called");
#endif
			NLNET::CMessage message("S322");
			nlWrite(message, serial, i32);

			_CallbackClient->send(message);
		}

		static void cb_returnInt32_2 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterface2WebClient::cb_returnInt32_2 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterface2WebClientItf *callback = (CTestInterface2WebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	i32;
			nlRead(message, serial, i32);


#ifdef NL_DEBUG
			nldebug("CTestInterface2WebClient::cb_returnInt32_2 : calling on_returnInt32_2");
#endif

			callback->on_returnInt32_2(from, i32);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CTestInterface2WebClient_Disconnection(NLNET::TSockId from) =0;


		virtual void on_returnInt32_2(NLNET::TSockId from, uint32 i32) =0;

	};


	struct TEnum
	{
		enum TValues
		{
			enum_a,
			enum_b,
			enum_c,
			
			invalid
		};

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(enum_a)
				NL_STRING_CONVERSION_TABLE_ENTRY(enum_b)
				NL_STRING_CONVERSION_TABLE_ENTRY(enum_c)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TEnum()
			: _Value(invalid)
		{
		}
		TEnum(TValues value)
			: _Value(value)
		{
		}

		TEnum(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TEnum &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TEnum &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TEnum &other) const
		{
			return _Value < other._Value;
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

		
	};
	
	class CTestInterfaceWeb2Itf 
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;	

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"SV32",	CTestInterfaceWeb2Itf::cb_sendVectorUInt32	},
				{	"SVS",	CTestInterfaceWeb2Itf::cb_sendVectorString	},
				{	"TWC",	CTestInterfaceWeb2Itf::cb_twoWayCall	},
				{	"TWCI",	CTestInterfaceWeb2Itf::cb_twoWayCallInt	},
				{	"TWCE",	CTestInterfaceWeb2Itf::cb_twoWayCallEnum	},
				{	"TWCV",	CTestInterfaceWeb2Itf::cb_twoWayCallVector	},
				{	"MVI",	CTestInterfaceWeb2Itf::cb_mixedVector	},
				{	"TWMV",	CTestInterfaceWeb2Itf::cb_mixedTwoWayVector	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterfaceWeb2__cbConnection);
			CTestInterfaceWeb2Itf *_this = reinterpret_cast<CTestInterfaceWeb2Itf *>(arg);

			_this->on_CTestInterfaceWeb2_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CTestInterfaceWeb2__cbDisconnection);
			CTestInterfaceWeb2Itf *_this = reinterpret_cast<CTestInterfaceWeb2Itf *>(arg);

			_this->on_CTestInterfaceWeb2_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the 
		 *	interface).
		 */
		CTestInterfaceWeb2Itf(ICallbackServerAdaptor *replacementAdaptor = NULL)
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

		virtual ~CTestInterfaceWeb2Itf()
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
			H_AUTO(CTestInterfaceWeb2_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CTestInterfaceWeb2 : Exception launch in callback server update");
			}
		}


		void returnVectorUInt32(NLNET::TSockId dest, const std::vector<uint32> &i32)
		{
			H_AUTO(returnVectorUInt32_returnVectorUInt32);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::returnVectorUInt32 called");
#endif
			NLNET::CMessage message("RV32");
			nlWrite(message, serialCont, i32);

			_CallbackServer->send(message, dest);
		}

		void returnVectorString(NLNET::TSockId dest, const std::vector<std::string> &vstr)
		{
			H_AUTO(returnVectorString_returnVectorString);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::returnVectorString called");
#endif
			NLNET::CMessage message("RVS");
			nlWrite(message, serialCont, vstr);

			_CallbackServer->send(message, dest);
		}

		void returnMixedVector(NLNET::TSockId dest, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32)
		{
			H_AUTO(returnMixedVector_returnMixedVector);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::returnMixedVector called");
#endif
			NLNET::CMessage message("RMVI");
			nlWrite(message, serial, param1);
			nlWrite(message, serialCont, vstr);
			nlWrite(message, serialCont, vi32);

			_CallbackServer->send(message, dest);
		}

		static void cb_sendVectorUInt32 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendVectorUInt32_on_sendVectorUInt32);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_sendVectorUInt32 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::vector<uint32>	vi32;
			nlRead(message, serialCont, vi32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_sendVectorUInt32 : calling on_sendVectorUInt32");
#endif


			callback->on_sendVectorUInt32(from, vi32);

		}

		static void cb_sendVectorString (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(sendVectorString_on_sendVectorString);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_sendVectorString received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::vector<std::string>	vstr;
			nlRead(message, serialCont, vstr);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_sendVectorString : calling on_sendVectorString");
#endif


			callback->on_sendVectorString(from, vstr);

		}

		static void cb_twoWayCall (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(twoWayCall_on_twoWayCall);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCall received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	param;
			nlRead(message, serial, param);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCall : calling on_twoWayCall");
#endif

			std::string retValue;

			retValue = callback->on_twoWayCall(from, param);
			
			NLNET::CMessage retMsg("R_TWC");

			nlWrite(retMsg, serial, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_twoWayCallInt (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(twoWayCallInt_on_twoWayCallInt);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallInt received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	param;
			nlRead(message, serial, param);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallInt : calling on_twoWayCallInt");
#endif

			uint32 retValue;

			retValue = callback->on_twoWayCallInt(from, param);
			
			NLNET::CMessage retMsg("R_TWCI");

			nlWrite(retMsg, serial, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_twoWayCallEnum (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(twoWayCallEnum_on_twoWayCallEnum);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallEnum received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			TEnum	param;
			nlRead(message, serial, param);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallEnum : calling on_twoWayCallEnum");
#endif

			TEnum retValue;

			retValue = callback->on_twoWayCallEnum(from, param);
			
			NLNET::CMessage retMsg("R_TWCE");

			nlWrite(retMsg, serial, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_twoWayCallVector (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(twoWayCallVector_on_twoWayCallVector);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallVector received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::vector<uint32>	param;
			nlRead(message, serialCont, param);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_twoWayCallVector : calling on_twoWayCallVector");
#endif

			std::vector<uint32> retValue;

			retValue = callback->on_twoWayCallVector(from, param);
			
			NLNET::CMessage retMsg("R_TWCV");

			nlWrite(retMsg, serialCont, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}

		static void cb_mixedVector (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(mixedVector_on_mixedVector);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_mixedVector received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	param1;
			std::vector<std::string>	vstr;
			std::vector<uint32>	vi32;
			nlRead(message, serial, param1);
			nlRead(message, serialCont, vstr);
			nlRead(message, serialCont, vi32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_mixedVector : calling on_mixedVector");
#endif


			callback->on_mixedVector(from, param1, vstr, vi32);

		}

		static void cb_mixedTwoWayVector (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(mixedTwoWayVector_on_mixedTwoWayVector);
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_mixedTwoWayVector received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2Itf *callback = (CTestInterfaceWeb2Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	param1;
			std::vector<std::string>	vstr;
			std::vector<uint32>	vi32;
			nlRead(message, serial, param1);
			nlRead(message, serialCont, vstr);
			nlRead(message, serialCont, vi32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2::cb_mixedTwoWayVector : calling on_mixedTwoWayVector");
#endif

			uint32 retValue;

			retValue = callback->on_mixedTwoWayVector(from, param1, vstr, vi32);
			
			NLNET::CMessage retMsg("R_TWMV");

			nlWrite(retMsg, serial, retValue);


			callback->_CallbackServer->send(retMsg, from);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CTestInterfaceWeb2_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CTestInterfaceWeb2_Disconnection(NLNET::TSockId from) =0;


		virtual void on_sendVectorUInt32(NLNET::TSockId from, const std::vector<uint32> &vi32) =0;

		virtual void on_sendVectorString(NLNET::TSockId from, const std::vector<std::string> &vstr) =0;

		virtual std::string on_twoWayCall(NLNET::TSockId from, const std::string &param) =0;

		virtual uint32 on_twoWayCallInt(NLNET::TSockId from, uint32 param) =0;

		virtual TEnum on_twoWayCallEnum(NLNET::TSockId from, TEnum param) =0;

		virtual std::vector<uint32> on_twoWayCallVector(NLNET::TSockId from, const std::vector<uint32> &param) =0;

		virtual void on_mixedVector(NLNET::TSockId from, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32) =0;

		virtual uint32 on_mixedTwoWayVector(NLNET::TSockId from, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32) =0;

	};
	
	
	/** This is the client side of the interface 
	 *	Derive from this class to invoke method on the callback server
	 */	

	class CTestInterfaceWeb2ClientItf 
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"RV32",	CTestInterfaceWeb2ClientItf::cb_returnVectorUInt32	},
				{	"RVS",	CTestInterfaceWeb2ClientItf::cb_returnVectorString	},
				{	"RMVI",	CTestInterfaceWeb2ClientItf::cb_returnMixedVector	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CTestInterfaceWeb2ClientItf *_this = reinterpret_cast<CTestInterfaceWeb2ClientItf *>(arg);

			_this->on_CTestInterfaceWeb2Client_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_returnVectorUInt32"), std::string("RV32")));
			messageNames.insert(std::make_pair(std::string("on_returnVectorString"), std::string("RVS")));
			messageNames.insert(std::make_pair(std::string("on_returnMixedVector"), std::string("RMVI")));

				initialized = true;
			}
			
			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;
			

			static std::string emptyString;
			
			return emptyString;

		}
		
		CTestInterfaceWeb2ClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
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
			H_AUTO(CTestInterfaceWeb2_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CTestInterfaceWeb2 : Exception launch in callback client update");
			}
		}


		void sendVectorUInt32(const std::vector<uint32> &vi32)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::sendVectorUInt32 called");
#endif
			NLNET::CMessage message("SV32");
			nlWrite(message, serialCont, vi32);

			_CallbackClient->send(message);
		}

		void sendVectorString(const std::vector<std::string> &vstr)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::sendVectorString called");
#endif
			NLNET::CMessage message("SVS");
			nlWrite(message, serialCont, vstr);

			_CallbackClient->send(message);
		}

		void twoWayCall(const std::string &param)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::twoWayCall called");
#endif
			NLNET::CMessage message("TWC");
			nlWrite(message, serial, const_cast < std::string& > (param));

			_CallbackClient->send(message);
		}

		void twoWayCallInt(uint32 param)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::twoWayCallInt called");
#endif
			NLNET::CMessage message("TWCI");
			nlWrite(message, serial, param);

			_CallbackClient->send(message);
		}

		void twoWayCallEnum(TEnum param)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::twoWayCallEnum called");
#endif
			NLNET::CMessage message("TWCE");
			nlWrite(message, serial, param);

			_CallbackClient->send(message);
		}

		void twoWayCallVector(const std::vector<uint32> &param)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::twoWayCallVector called");
#endif
			NLNET::CMessage message("TWCV");
			nlWrite(message, serialCont, param);

			_CallbackClient->send(message);
		}

		void mixedVector(uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::mixedVector called");
#endif
			NLNET::CMessage message("MVI");
			nlWrite(message, serial, param1);
			nlWrite(message, serialCont, vstr);
			nlWrite(message, serialCont, vi32);

			_CallbackClient->send(message);
		}

		void mixedTwoWayVector(uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::mixedTwoWayVector called");
#endif
			NLNET::CMessage message("TWMV");
			nlWrite(message, serial, param1);
			nlWrite(message, serialCont, vstr);
			nlWrite(message, serialCont, vi32);

			_CallbackClient->send(message);
		}

		static void cb_returnVectorUInt32 (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnVectorUInt32 received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2ClientItf *callback = (CTestInterfaceWeb2ClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::vector<uint32>	i32;
			nlRead(message, serialCont, i32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnVectorUInt32 : calling on_returnVectorUInt32");
#endif

			callback->on_returnVectorUInt32(from, i32);
		}

		static void cb_returnVectorString (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnVectorString received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2ClientItf *callback = (CTestInterfaceWeb2ClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::vector<std::string>	vstr;
			nlRead(message, serialCont, vstr);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnVectorString : calling on_returnVectorString");
#endif

			callback->on_returnVectorString(from, vstr);
		}

		static void cb_returnMixedVector (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnMixedVector received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CTestInterfaceWeb2ClientItf *callback = (CTestInterfaceWeb2ClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	param1;
			std::vector<std::string>	vstr;
			std::vector<uint32>	vi32;
			nlRead(message, serial, param1);
			nlRead(message, serialCont, vstr);
			nlRead(message, serialCont, vi32);


#ifdef NL_DEBUG
			nldebug("CTestInterfaceWeb2Client::cb_returnMixedVector : calling on_returnMixedVector");
#endif

			callback->on_returnMixedVector(from, param1, vstr, vi32);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CTestInterfaceWeb2Client_Disconnection(NLNET::TSockId from) =0;


		virtual void on_returnVectorUInt32(NLNET::TSockId from, const std::vector<uint32> &i32) =0;

		virtual void on_returnVectorString(NLNET::TSockId from, const std::vector<std::string> &vstr) =0;

		virtual void on_returnMixedVector(NLNET::TSockId from, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32) =0;

	};

}
	
#endif
