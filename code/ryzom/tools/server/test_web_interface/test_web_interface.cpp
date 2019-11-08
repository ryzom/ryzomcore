
#include "test_web_interface.h"
#include "test_web_interface_itf.h"
#include "nel/net/service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace TWI;

class CTestWebInterfaceServiceImpl
	:	public TWI::CTestInterfaceWebItf,
		public TWI::CTestInterfaceWeb2Itf
{
	uint32 _TestState;
public:

	/////////////////////////////////////////
	/// Web interface callback imp
	/////////////////////////////////////////
	virtual void on_CTestInterfaceWeb_Connection(NLNET::TSockId from)
	{
		nlinfo("Connect from %s, %s", from->asString().c_str(), from->getTcpSock()->remoteAddr().asString().c_str());
	}

	virtual void on_CTestInterfaceWeb_Disconnection(NLNET::TSockId from)
	{
		nlinfo("Disconnect from %s, %s", from->asString().c_str(), from->getTcpSock()->remoteAddr().asString().c_str());
		
//		if (_TestState == 0
//			|| _TestState == 1
//			|| _TestState == 3
//			|| _TestState == 5
//			|| _TestState == 9
//			|| _TestState == 13
//			|| _TestState == 15
//			|| _TestState == 17
//			|| _TestState == 19)
//		{
//			// ok
//		}
//		else
//		{
//			nlwarning("Receive disconnect in state %u", _TestState);
////			nlstop;
//		}
//
//		if (_TestState > 0)
//			_TestState++;
	}

	virtual void on_beginTest(NLNET::TSockId from)\
	{
		_TestState = 0;
	}

	virtual void on_sendUInt32(NLNET::TSockId from, uint32 i32)
	{
		if (_TestState == 0)
		{
			nlassert(i32 == 0x12345678);
			returnUInt32(from, 0x12345678);
		}
		else if (_TestState == 3)
		{
			nlassert(i32 == 0xFEDCBA98);
			returnUInt32(from, 0x12345678);
		}
		else if (_TestState == 9)
		{
			nlassert(i32 == 0x12345678);
			returnUInt32(from, 0x12345678);
			returnUInt32(from, 0x87654321);
		}
		else if (_TestState == 10)
		{
			nlassert(i32 == 0x12345678);
			returnUInt8(from, 0x34);
			returnUInt32(from, 0x87654321);
		}
		else if (_TestState == 11)
		{
			nlassert(i32 == 0x12345678);

			// no response, live client alone and friendless
		}
		else
		{
			nlwarning("Received sendInt32 in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	virtual void on_sendUInt8(NLNET::TSockId from, uint8 i8)
	{
		if (_TestState == 1)
		{
			nlassert(i8 == 0xf1);
			returnUInt8(from, 0xf1);
		}
		else if (_TestState == 4)
		{
			nlassert(i8 == 0xcc);
			returnUInt8(from, 0x01);
		}
		else
		{
			nlwarning("Received sendInt8 in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	virtual void on_sendString(NLNET::TSockId from, const std::string &str)
	{
		if (_TestState == 2)
		{
			nlassert(str == "hello world");
			returnString(from, "hello world");
		}
		else if (_TestState == 5)
		{
			nlassert(str == "hello world");
			returnString(from, "ok");
		}
		else
		{
			nlwarning("Received sendString in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	virtual void on_composite1(NLNET::TSockId from, uint32 i32, uint8 i8, const std::string &str)
	{
		if (_TestState == 6)
		{
			nlassert(i32 == 0x12345678);
			nlassert(i8 == 0x9A);
			nlassert(str == "Ibm");
			returnComposite1(from, 0x12345678, 0x9a, "hal");
		}
		else
		{
			nlwarning("Received sendComposite1 in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	virtual void on_composite2(NLNET::TSockId from, const std::string &str1, const std::string &str2, const std::string &str3, const std::string &str4)
	{
		if (_TestState == 7)
		{
			nlassert(str1 == "ABCD");
			nlassert(str2 == "EFGH IJKL");
			nlassert(str3 == "MNO");
			nlassert(str4 == "PQ");
			returnComposite2(from, "ABCD", "EFGH IJKL", "MNO", "PQ");
		}
		else
		{
			nlwarning("Received sendComposite2 in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	virtual void on_composite3(NLNET::TSockId from, uint8 i81, uint32 i321, const std::string &str1, uint8 i82, uint32 i322, const std::string &str2, uint8 i83, uint32 i323, const std::string &str3, uint8 i84, uint32 i324, const std::string &str4, uint32 i325, uint8 i85)
	{
		if (_TestState == 8)
		{
			nlassert(i81 == 0x9a);
			nlassert(i321 == 0x12345678);
			nlassert(str1 == "ABC");
			nlassert(i322 == 0xbcdef012);
			nlassert(i82 == 0x34);
			nlassert(str2 == "DEFGH");
			nlassert(i323 == 0x567890ab);
			nlassert(i83 == 0xcd);
			nlassert(str3 == "I");
			nlassert(i324 == 0xef012345);
			nlassert(i84 == 0x67);
			nlassert(str4 == "");
			nlassert(i325 == 0x01234567);
			nlassert(i85 == 0x89);

			returnComposite3(from, 	0x9a, 0x12345678, "ABC",
							   0x34, 0xbcdef012, "DEFGH",
							   0xcd, 0x567890ab, "I",
							   0x67, 0xef012345, "",
							   0x01234567, 0x89);
		}
		else
		{
			nlwarning("Received sendComposite3 in state %u", _TestState);
//			nlstop;
		};

		_TestState++;
	}

	///////////////////////////////////
	// Wel interface 2 impl
	//////////////////////////////////
	/// Connection callback : a new interface client connect
	virtual void on_CTestInterfaceWeb2_Connection(NLNET::TSockId from)
	{
	}
	/// Disconnection callback : one of the interface client disconnect
	virtual void on_CTestInterfaceWeb2_Disconnection(NLNET::TSockId from)
	{
	}


	virtual void on_sendVectorUInt32(NLNET::TSockId from, const std::vector<uint32> &vi32)
	{
		returnVectorUInt32(from, vi32);
	}

	virtual void on_sendVectorString(NLNET::TSockId from, const std::vector<std::string> &vstr)
	{
		returnVectorString(from, vstr);
	}


	virtual std::string on_twoWayCall(NLNET::TSockId from, const std::string &param)
	{
		return param;
	}

	virtual uint32 on_twoWayCallInt(NLNET::TSockId from, uint32 param)
	{
		return param;
	}

	virtual TEnum on_twoWayCallEnum(NLNET::TSockId from, TEnum param)
	{
		return param;
	}
	
	virtual std::vector<uint32> on_twoWayCallVector(NLNET::TSockId from, const std::vector<uint32> &param)
	{
		return param;
	}

	virtual void on_mixedVector(NLNET::TSockId from, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32)
	{
		returnMixedVector(from, param1, vstr, vi32);
	}

	virtual uint32 on_mixedTwoWayVector(NLNET::TSockId from, uint32 param1, const std::vector<std::string> &vstr, const std::vector<uint32> &vi32)
	{
		return param1;
	}
	
};


class CTestWebInterfaceService
:	public IService
{

	CTestWebInterfaceServiceImpl *_WebItf;
public:
	void init () 
	{ 
		_WebItf = new CTestWebInterfaceServiceImpl;
		_WebItf->CTestInterfaceWebItf::openItf(8061);
		_WebItf->CTestInterfaceWeb2Itf::openItf(8062);
	}
	bool update () 
	{ 
		// update the web interface
		_WebItf->CTestInterfaceWebItf::update();
		_WebItf->CTestInterfaceWeb2Itf::update();
		
		return true; 
	}
	void release () 
	{ 
		delete _WebItf;
	}
};

NLNET::TUnifiedCallbackItem cbArray[]=
{ "", NULL};

NLNET_SERVICE_MAIN(CTestWebInterfaceService, "WTI", "test_web_interface", 8060, cbArray, ".", ".");

