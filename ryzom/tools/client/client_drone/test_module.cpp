

#include "nel/misc/random.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

/** A test module that transmit and receive message to test big message and message ordering.
 *	Must work in collaboration with CFEServerModuleTest.
 *
 *	The protocol is the following :
 *		- Client wait server module up
 *		- Client send a first 'hello' message
 *		-  -> server send back a 'start test' messages, then send 100 small message with serial number inside
 *		-		-> client receive the 100 small message
 *		-       -> client send 100 small messages with serial number inside
 *				(this test the packet ordering basics)
 *		- After server have received the 100 messages, it send 'part2' to the client
 *				-> server start sending 100 message with variable size (from 10B to 1MB), one mesage by tick
 *				-> client start	sending 100 message of variable size (from 10B to 1MB with a predefined rule), on message by tick
 *				(this test big message transfert, with ordering and bidirectionnal comm)
 *	For each message series, the receiver part validate the message ordering.
 */
class CFEClientModuleTest : public CEmptyModuleServiceBehav<CEmptySocketBehav<CModuleBase> >
{
public:
	TModuleProxyPtr		Server;

	enum TTestState 
	{
		ts_wait_start,
		ts_receive_part1,
		ts_wait_part2,
		ts_send_recv_part2,
		ts_receive_part2,
		ts_end
	};

	TTestState		TestState;

	// serial counter for reception
	uint32			ReceiveSerial;
	// send counter
	uint32			SendCounter;

	// random number generator for packet size
	CRandom			RandS;
	CRandom			RandR;

	CFEClientModuleTest() 
		: TestState(ts_end)
	{
	}

	virtual void				onModuleUp(IModuleProxy *moduleProxy)
	{
		if (moduleProxy->getModuleClassName() == "FEServerModuleTest")
		{
			nlassert(TestState == ts_end);
			nlassert(Server == NULL);
			// store the server
			Server = moduleProxy;

			nldebug("Test module is up, sending 'HELLO'");
			// start the comm by sending a 
			CMessage start("HELLO");

			Server->sendModuleMessage(this, start);

			TestState = ts_wait_start;
		}
	}
	virtual void				onModuleDown(IModuleProxy *moduleProxy)
	{
		if (moduleProxy == Server)
		{
			nldebug("Test module is down");
			TestState = ts_end;
			Server = NULL;
		}
	}
	virtual bool			onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
	{
		string msgName = message.getName();
		if (msgName == "START")
		{
			nlassert(TestState == ts_wait_start);

			nldebug("Received 'START', waiting for 100 message from server");
			// set in reception part 1
			TestState = ts_receive_part1;
			ReceiveSerial = 0;
		}
		else if (msgName == "MSG_PART1")
		{
			nlassert(TestState == ts_receive_part1);

			uint32 serial0;
			nlRead(message, serial, serial0);

			nlassert(serial0 == ReceiveSerial);
			++ReceiveSerial;

			nldebug("Received 'MSG_PART1' number %u / 100", serial0);
			if (ReceiveSerial == 100)
			{
				// send 100 message to the server
				for (uint32 i=0; i<100; ++i)
				{
					CMessage msg("MSG_PART1");
					nlRead(message, serial, i);

					Server->sendModuleMessage(this, msg);
				}
				TestState = ts_wait_part2;
			}
		}
		else if (msgName == "PART2")
		{
			nlassert(TestState == ts_wait_part2);

			nldebug("Received 'PART2', begin send/receive varaible message");

			TestState = ts_send_recv_part2;
			SendCounter = 0;
			ReceiveSerial = 0;

			RandS.srand(123456);
			RandR.srand(123456);
		}
		else if (msgName == "MSG_PART2")
		{
			nlassert(TestState == ts_send_recv_part2 
					|| TestState == ts_receive_part2 );

			nlassert(ReceiveSerial <= 100);

			uint32 serial0;
			nlRead(message, serial, serial0);
			nlassert(serial0 == ReceiveSerial);
			++ReceiveSerial;

			uint32 size = RandR.rand()>>4;

			for (uint i=0; i<size; ++i)
			{
				uint8 b;
				nlRead(message, serial, b);

				nlassert(b == uint8(i));
			}
			// check that all the buffer have been read
			nlassert(message.length() == (uint32)message.getPos());

			nldebug("Received 'MSG_PART2' number %u/100, size = %u", serial0, size);

			if (ReceiveSerial == 100)
			{
				nldebug("End of part2, test terminated");
				TestState = ts_end;
			}
		}
		else
			return false;
		return true;
	}

	virtual void	onModuleSecurityChange(IModuleProxy *moduleProxy)
	{
		// glop
	}


	virtual void	onModuleUpdate()
	{
		if (TestState == ts_send_recv_part2)
		{
			CMessage message("MSG_PART2");
			message.serial(SendCounter);
			++SendCounter;

			// select a packed size up to 1 MB
			uint32 size = RandS.rand()>>4;

			for (uint i=0; i<size; ++i)
			{
				uint8 b = uint8(i);
				message.serial(b);
			}

			nldebug("Sending 'MSG_PART2' number %u / 100, size = %u", SendCounter, size);
			// send
			Server->sendModuleMessage(this, message);

			if (SendCounter == 100)
			{
				// all packed send, wait for reception to end now
				if (ReceiveSerial < 100)
					TestState = ts_receive_part2;
				else
					TestState = ts_end;
			}
		}
	}

};

NLNET_REGISTER_MODULE_FACTORY(CFEClientModuleTest, string("FEClientModuleTest"));