#ifndef UT_NET_LAYER3
#define UT_NET_LAYER3

#include <nel/net/callback_client.h>
#include <nel/net/callback_server.h>

uint16 TestPort1 = 56000;

uint NbTestReceived = 0;

CMessage msgoutExpectingAnswer0, msgoutSimple0, msgoutSimple50;

// Data structure for messages
struct TData
{
	string PayloadString;
	bool ExpectingAnswer;

	// Constructor
	TData() : ExpectingAnswer(false) {}

	// Serial
	void serial( NLMISC::IStream& s )
	{
		s.serial( PayloadString );
		s.serial( ExpectingAnswer );
	}
};

// This callback must not take more than 10 ms
void cbTest( CMessage &msgin, TSockId from, CCallbackNetBase &netbase )
{
	TTime before = CTime::getLocalTime();

	// Read data from the message
	TData data;
	msgin.serial( data );
	if ( data.PayloadString == "Payload" )
		++NbTestReceived;

	// Send the answer if required
	if ( data.ExpectingAnswer )
		netbase.send( msgoutSimple0, from );

	// Check that the duration is compatible with our timeout tests
	TTime maxDuration;
	if ( msgin.getName() == "TEST_50" )
	{
		while ( CTime::getLocalTime() - before < 49 ); // wait
		maxDuration = 70;
	}
	else
		maxDuration = 10;
	TTime actualDuration = CTime::getLocalTime() - before;
	if ( actualDuration > maxDuration )
		nlerror( "The callback cbTest takes too long (%u) for %s, please fix the test", (uint)actualDuration, msgin.getName().c_str() );
}


static TCallbackItem CallbackArray[] =
{
	{ "TEST_0", cbTest },
	{ "TEST_50", cbTest }
};


// Test suite for layer 3
class CUTNetLayer3: public Test::Suite
{
public:

	//
	CUTNetLayer3 ()
	{
		_Server = NULL;
		_Client = NULL;
		TEST_ADD(CUTNetLayer3::sendReceiveUpdate);

	}

	//
	~CUTNetLayer3 ()
	{
		if ( _Server )
			delete _Server;
		_Server = NULL;
		if ( _Client )
			delete _Client;
		_Client = NULL;
	}

	//
	void sendReceiveUpdate()
	{
		// Prepare messages for tests
		TData data;
		data.PayloadString = "Payload";
		data.ExpectingAnswer = true;
		msgoutExpectingAnswer0.clear(); // optional
		msgoutExpectingAnswer0.setType( "TEST_0" ); // could be passed to the constructor
		msgoutExpectingAnswer0.serial( data );
		data.ExpectingAnswer = false;
		msgoutSimple0.clear(); // optional
		msgoutSimple0.setType( "TEST_0" ); // could be passed to the constructor
		msgoutSimple0.serial( data );
		msgoutSimple50.clear(); // optional
		msgoutSimple50.setType( "TEST_50" ); // could be passed to the constructor
		msgoutSimple50.serial( data );

		// Init connections
		_Server = new CCallbackServer();
		_Server->init( TestPort1 );
		_Server->addCallbackArray( CallbackArray, sizeof(CallbackArray)/sizeof(TCallbackItem) );
		_Client = new CCallbackClient();
		_Client->connect( CInetAddress( "localhost", TestPort1 ) );
		_Client->addCallbackArray( CallbackArray, sizeof(CallbackArray)/sizeof(TCallbackItem) );

		// TEST: Simple message transmission
		NbTestReceived = 0;
		_Client->send( msgoutExpectingAnswer0 );
		for ( uint i=0; i!=10; ++i ) // give some time to receive
		{
			_Client->update();
			_Server->update(); // legacy version
			nlSleep( 50 );
		}
		TEST_ASSERT( NbTestReceived == 2 ); // answer and reply

		// TEST: ONE-SHOT update mode on the receiver
		NbTestReceived = 0;
		for ( uint i=0; i!=20; ++i ) // send 20 messages
			_Client->send( msgoutSimple0 );
		while ( NbTestReceived < 20 )
		{
			_Client->update2();
			uint prevNbTestReceived = NbTestReceived;
			_Server->update2( 0 ); // shortest time-out = ONE-SHOT mode
			TEST_ASSERT( (NbTestReceived == prevNbTestReceived) ||
						 (NbTestReceived == prevNbTestReceived + 1) );
			nlSleep( 10 );
		}
		
		// TEST: GREEDY update mode on the receiver
		NbTestReceived = 0;
		for ( uint i=0; i!=20; ++i ) // send 20 messages
			_Client->send( msgoutSimple0 );
		for ( uint i=0; i!=10; ++i ) // make sure all messages are flushed
		{
			_Client->update2();
			nlSleep( 10 );
		}
		_Server->update2( -1 ); // receive all
		TEST_ASSERT( NbTestReceived == 20 );

		// TEST: CONSTRAINED update mode on the receiver
		NbTestReceived = 0;
		for ( uint i=0; i!=20; ++i ) // send 20 messages that will trigger a time-consuming callback
			_Client->send( msgoutSimple50 );
		for ( uint i=0; i!=10; ++i ) // make sure all messages are flushed
		{
			_Client->update2();
			nlSleep( 10 );
		}
		while ( NbTestReceived < 20 )
		{
			uint prevNbTestReceived = NbTestReceived;
			_Server->update2( 80 ); // no more time than two callback executions
			TEST_ASSERT( NbTestReceived <= prevNbTestReceived + 2 );
		}

		// TEST: CONSTRAINED with minTime update mode on the receiver
		NbTestReceived = 0;
		while ( NbTestReceived < 20 )
		{
			_Client->send( msgoutSimple0 );
			_Client->send( msgoutSimple0 ); // send 2 messages at a time
			_Client->update2();
			TTime before = CTime::getLocalTime();
			_Server->update2( -1, 30 );
			TTime duration = CTime::getLocalTime() - before;
			TEST_ASSERT( duration >= 30 );
		}
	}

private:
	CCallbackServer *_Server;
	CCallbackClient *_Client;

};

#endif
