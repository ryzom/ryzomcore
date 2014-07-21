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

//
// Includes
//

#include "nel/misc/types_nl.h"

#include <string>
#include <map>
#include <time.h>

#ifdef NL_OS_WINDOWS
#	include <direct.h>
#	define mkdir _mkdir
#else
#	include <sys/stat.h>
#	define mkdir(a) mkdir(a,0755)
#endif


#include "nel/misc/debug.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/path.h"

#include "nel/net/service.h"
#include "nel/net/udp_sock.h"

#include "receive_task.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

#ifndef UDP_DIR
#define UDP_DIR ""
#endif // UDP_DIR

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Structures
//

struct CClient
{
	CClient (TSockId from, uint32 session, const string &cn) : From(from), Session(session), NextPingNumber(0), LastPongReceived(0), ConnectionName(cn),
		BlockNumber(0), FullMeanPongTime(0), FullNbPong(0), NbPing(0), NbPong(0), MeanPongTime(0), NbDuplicated(0), FirstWrite(true)
	{ PongReceived.resize (1001); }

	CInetAddress	Address;	// udp address
	TSockId			From;		// used to find the TCP connection
	uint32			Session;	// used to find the link between UDP and TCP connection at startup

	vector<pair<uint8, uint16> >	PongReceived;	// contains the number of pong receive for each message number and the time

	uint32			NextPingNumber, LastPongReceived;
	string			ConnectionName;

	// this number is increase each time we filled the PongReceived array, the goal is to avoid a very old packet to use as a new one
	uint32			BlockNumber;

	uint32			FullMeanPongTime, FullNbPong;

	// used for stat, reset every stat update
	uint32			NbPing, NbPong, MeanPongTime, NbDuplicated;

	// true if the client just connect and we don't log stat
	bool			FirstWrite;

	void updatePong (sint64 pingTime, sint64 pongTime, uint32 pongNumber, uint32 blockNumber);
	void updateStat ();
	void updateFullStat ();
};

struct TInetAddressHash
{
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;

	inline bool operator() (const NLNET::CInetAddress &x1, const NLNET::CInetAddress &x2) const
	{
		return x1 == x2;
	}

	/// Hash function
	inline size_t operator() ( const NLNET::CInetAddress& x ) const
	{
		return x.port();
		//return x.internalIPAddress();
	}
};

//
// Types
//

typedef CHashMap<NLNET::CInetAddress,CClient*,TInetAddressHash> TClientMap;
#define GETCLIENTA(it) (*it).second

//
// Variables
//

// must be increase at each version and must be the same value as the client
uint32				Version = 2;

string				StatPathName = "stats/";

uint16				UDPPort = 45455;
uint16				TCPPort = 45456;
uint32				MaxUDPPacketSize = 1000;

CBufFIFO			Queue1, Queue2;

CBufFIFO			*CurrentReadQueue = NULL;

TReceivedMessage	*CurrentInMsg = NULL;

IThread				*ReceiveThread = NULL;
CReceiveTask		*ReceiveTask = NULL;

list<CClient>		Clients;	// contains all clients
TClientMap			ClientMap;	// contains a quick access to the client using the udp address

// TCP server for clients
CCallbackServer		*CallbackServer = NULL;

//
// Functions
//

string getDate()
{
	struct tm *newtime;
	time_t long_time;
	time( &long_time );
	newtime = localtime( &long_time );
	if (newtime)
	{
		string res = toString("%02d", newtime->tm_year-100) + "_";
		res += toString("%02d", newtime->tm_mon+1) + "_";
		res	+= toString("%02d", newtime->tm_mday);
		return res;
	}

	return "bad date "+toString( (uint32)long_time);
}

//
// Callbacks
//

void cbInit (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint64 session = (uint64)(uintptr_t) from;

	string connectionName;
	msgin.serial (connectionName);

	try
	{
		uint32 version;
		msgin.serial (version);
		if (version != Version)
		{
			// bad client version, disconnect it
			CallbackServer->disconnect (from);
			return;
		}
	}
	catch (const Exception &)
	{
		// bad client version, disconnect it
		CallbackServer->disconnect (from);
		return;
	}

	CMessage msgout ("INIT");
	msgout.serial (session);
	CallbackServer->send (msgout, from);

	Clients.push_back(CClient(from, (uint32)session, connectionName));
	nlinfo ("Added client TCP %s, session %x", from->asString().c_str(), session);
}

void cbDisconnect (TSockId from, void *arg)
{
	for (list<CClient>::iterator it = Clients.begin(); it != Clients.end(); it++)
	{
		if ((*it).From == from)
		{
			// clear struct
			(*it).updateFullStat();
			nlinfo( "Removing client %s", (*it).Address.asString().c_str() );
			ClientMap.erase ((*it).Address);
			Clients.erase (it);
			return;
		}
	}
}

//
// Callback Array
//

TCallbackItem CallbackArray[] =
{
	{ "INIT", cbInit },
};



void CClient::updatePong (sint64 pingTime, sint64 pongTime, uint32 pongNumber, uint32 blockNumber)
{
	// it means that it s a too old packet, discard it
	if (blockNumber != BlockNumber)
		return;

	// add the pong in the array to detect lost, duplication
	if (pongNumber >= PongReceived.size())
	{
		// if the array is too big, we flush actual data and restart all
		updateFullStat ();
		return;
	}

	PongReceived[pongNumber].first++;

	if (PongReceived[pongNumber].first > 1)
	{
		NbDuplicated++;
	}
	else
	{
		// increase only for new pong
		NbPong++;
		MeanPongTime += (uint32)(pongTime-pingTime);
	
		FullNbPong++;
		FullMeanPongTime += (uint32)(pongTime-pingTime);

		PongReceived[pongNumber].second = (uint16)(pongTime - pingTime);
	}

	if (pongNumber > LastPongReceived)
		LastPongReceived = pongNumber;

	// write each pong in a file
	string ha = Address.hostName();
	if (ha.empty())
	{
		ha = Address.ipAddress();
	}
	string fn = StatPathName + ConnectionName + "_" + ha + "_" + getDate() + ".pong";
	
	FILE *fp = fopen (fn.c_str(), "rt");
	if (fp == NULL)
	{
		// new file, add the header
		FILE *fp = fopen (fn.c_str(), "wt");
		if (fp != NULL)
		{
			fprintf (fp, "#%s\t%s\t%s\t%s\n", "PingTime", "PongTime", "Delta", "PingNumber");
			fclose (fp);
		}
	}
	else
	{
		fclose (fp);
	}

	fp = fopen (fn.c_str(), "at");
	if (fp == NULL)
	{
		nlwarning ("Can't open pong file name '%s'", fn.c_str());
	}
	else
	{
		fprintf (fp, "%"NL_I64"d\t%"NL_I64"d\t%"NL_I64"d\t%d\n", pongTime, pingTime, (pongTime-pingTime), pongNumber);
		fclose (fp);
	}
}

void CClient::updateFullStat ()
{
	uint32 NbLost = 0, NbDup = 0, NbPong = 0;

/*	if (Address.hostName().empty())
	{
		// don't log because we receive no pong at all
		return;
	}*/

	for (uint i = 0; i < LastPongReceived; i++)
	{
		if (PongReceived[i].first == 0) NbLost++;
		else 
		{
			NbPong++;
			NbDup += PongReceived[i].first - 1;
		}
	}

	{
		// write each pong in a file
		string ha = Address.hostName();
		if (ha.empty())
		{
			ha = Address.ipAddress();
		}
		string fn = StatPathName + ConnectionName + "_" + ha + "_" + getDate() + ".stat";
		
		string line = "Full Summary: ";
		line += "NbPing " + toString(LastPongReceived) + " ";
		line += "NbPong " + toString(NbPong) + " ";
		line += "NbLost " + toString(NbLost) + " ";
		if (LastPongReceived>0) line += "(" + toString((float)NbLost/LastPongReceived*100.0f) + "pc) ";
		line += "NbDuplicated " + toString(NbDup) + " ";
		if (LastPongReceived>0) line += "(" + toString((float)NbDup/LastPongReceived*100.0f) + "pc) ";

		if (FullNbPong == 0)
			line += "MeanPongTime <Undef> ";
		else
			line += "MeanPongTime " + toString(FullMeanPongTime/FullNbPong) + " ";

		FILE *fp = fopen (fn.c_str(), "at");
		if (fp == NULL)
		{
			nlwarning ("Can't open stat file name '%s'", fn.c_str());
		}
		else
		{
			fprintf (fp, "%s\n", line.c_str());
			fclose (fp);

			// send the full sumary to the client
			CMessage msgout("INFO");
			msgout.serial(line);
			CallbackServer->send (msgout, From);
		}

		nlinfo (line.c_str());
	}


	{
		// write each ping in a file
		string ha = Address.hostName();
		if (ha.empty())
		{
			ha = Address.ipAddress();
		}
		string fn = StatPathName + ConnectionName + "_" + ha + "_" + getDate() + ".ping";
		
		FILE *fp = fopen (fn.c_str(), "rt");
		if (fp == NULL)
		{
			// new file, add the header
			FILE *fp = fopen (fn.c_str(), "wt");
			if (fp != NULL)
			{
				fprintf (fp, "#%s\t%s\n", "NbPongRcv", "Delta");
				fclose (fp);
			}
		}
		else
		{
			fclose (fp);
		}

		fp = fopen (fn.c_str(), "at");
		if (fp == NULL)
		{
			nlwarning ("Can't open ping file name '%s'", fn.c_str());
		}
		else
		{
			// add a fake value to know that it s a different session
			fprintf (fp, "-1\t0\n");
			for (uint i = 0; i < LastPongReceived; i++)
			{
				fprintf (fp, "%d\t%d\n", PongReceived[i].first, PongReceived[i].second);
			}
			fclose (fp);
		}
	}

	// clear all structures

	PongReceived.clear ();
	PongReceived.resize (1001);

	BlockNumber++;

	NextPingNumber = LastPongReceived = 0;

	FullMeanPongTime = FullNbPong = 0;

//	NbPing = NbPong = MeanPongTime = NbDuplicated = 0;
}

void CClient::updateStat ()
{
	// write each pong in a file
	string ha = Address.hostName();
	if (ha.empty())
	{
		ha = Address.ipAddress();
	}
	string fn = StatPathName + ConnectionName + "_" + ha + "_" + getDate() + ".stat";
	
	string line;
	line += "NbPing " + toString(NbPing) + " ";
	line += "NbPong " + toString(NbPong) + " ";
	if (NbPong == 0)
		line += "MeanPongTime <Undef> ";
	else
		line += "MeanPongTime " + toString(MeanPongTime/NbPong) + " ";
	line += "NbDuplicated " + toString(NbDuplicated) + " ";

	FILE *fp = fopen (fn.c_str(), "at");
	if (fp == NULL)
	{
		nlwarning ("Can't open stat file name '%s'", fn.c_str());
	}
	else
	{
		if (FirstWrite)
		{
			//nlassert (!Address.hostName().empty())
			fprintf (fp, "HostAddress: %s\n", Address.asString().c_str());
			FirstWrite = false;
		}
		
		fprintf (fp, "%s\n", line.c_str());
		fclose (fp);
	}

	nlinfo (line.c_str());

	CMessage msgout("INFO");
	msgout.serial(line);
	CallbackServer->send (msgout, From);

	NbPing = NbPong = MeanPongTime = NbDuplicated = 0;
}

void updateStat ()
{
	static sint64 lastUpdate = CTime::getLocalTime ();

	if (CTime::getLocalTime() - lastUpdate < 2*1000)
		return;

	lastUpdate = CTime::getLocalTime();

	// update stat only at the linked UDP-TCP connection
	for (TClientMap::iterator it = ClientMap.begin (); it != ClientMap.end(); it++)
	{
		GETCLIENTA(it)->updateStat ();
	}
}


//
// Functions
//


void removeClientByAddr( TClientMap::iterator iclient )
{
	if ( iclient == ClientMap.end() )
	{
		// It may have already been removed on purpose
		return;
	}

	for (list<CClient>::iterator it = Clients.begin(); it != Clients.end(); it++)
	{
		if ((*it).Address == (*iclient).first)
		{
			(*it).updateFullStat();
			nlinfo( "Removing client %s", GETCLIENTA(iclient)->Address.asString().c_str() );
			Clients.erase(it);
			break;
		}
	}
	ClientMap.erase( iclient );
}

void handleReceivedPong (CClient *client, sint64 pongTime)
{
	// Preconditions
	nlassert( CurrentInMsg && (! CurrentInMsg->data().empty()) );

	// Prepare message to read
	CMemStream msgin( true );
	uint32 currentsize = CurrentInMsg->userSize();

	memcpy (msgin.bufferToFill (currentsize), CurrentInMsg->userDataR(), currentsize);

	// Read the header
	uint8 mode = 0;
	msgin.serial (mode);

	if (mode == 0)
	{
		// init the UDP connection
		if (client == NULL)
		{
			uint32 session = 0;
			msgin.serial (session);

			// Find a new udp connection, find the linked 
			list<CClient>::iterator it;
			for (it = Clients.begin(); it != Clients.end(); it++)
			{
				if ((*it).Session == session)
				{
					client = &(*it);
					// Found it, add in the map
					client->Address = CurrentInMsg->AddrFrom;
					ClientMap.insert (make_pair (client->Address, client));
					nlinfo ("TCP-UDP linked TCP is %s, UDP is %s", client->From->asString().c_str(), client->Address.asString().c_str());

					// Send a TCP message to the client to say that we can start
					CMessage msgout ("START");
					CallbackServer->send (msgout, client->From);
					break;
				}
			}
			if (it == Clients.end())
			{
				nlwarning ("Unknown TCP client, discard the UDP message (hacker?)");
				return;
			}
		}
		return;
	}
	else if (mode == 1)
	{
		if (client == NULL)
		{
			nlwarning ("Received a UDP packet from an old client (hacker?)");
			return;
		}

		// Read the message
		sint64 pingTime = 0;
		msgin.serial(pingTime);

		uint32 pongNumber = 0;
		msgin.serial(pongNumber);

		uint32 blockNumber = 0;
		msgin.serial(blockNumber);

//		nlinfo ("receive a pong from %s pongnb %d %"NL_I64"d", CurrentInMsg->AddrFrom.asString().c_str(), pongNumber, pongTime - pingTime);

		client->updatePong (pingTime, pongTime, pongNumber, blockNumber);
	}
}

void sendPing ()
{
	CMemStream msgout;
	for (TClientMap::iterator it = ClientMap.begin (); it != ClientMap.end(); it++)
	{
		msgout.clear();

		sint64 t = CTime::getLocalTime ();
		msgout.serial (t);

		uint32 p = GETCLIENTA(it)->NextPingNumber;
		msgout.serial (p);

		uint32 b = GETCLIENTA(it)->BlockNumber;
		msgout.serial (b);

		uint8 dummy=0;
		while (msgout.length() < 200)
			msgout.serial (dummy);

		uint32 size =  msgout.length();
		nlassert (size == 200);

		try
		{
			// send the new ping to the client
			ReceiveTask->DataSock->sendTo (msgout.buffer(), size, GETCLIENTA(it)->Address);
		}
		catch (const Exception &e)
		{
			nlwarning ("Can't send UDP packet to '%s' (%s)", GETCLIENTA(it)->Address.asString().c_str(), e.what());
		}

		GETCLIENTA(it)->NextPingNumber++;
		GETCLIENTA(it)->NbPing++;
	}
}

//
// Main Class
//

class CBenchService : public IService
{
public:
	
	void init()
	{
		nlassert( ReceiveTask==NULL && ReceiveThread==NULL );

		// Create stat folder if necessary

		if (!CFile::isExists (StatPathName))
		{
			mkdir (StatPathName.c_str());
		}

		// Create and start UDP server

		nlinfo( "Starting external UDP socket on port %d", UDPPort);
		ReceiveTask = new CReceiveTask (UDPPort, MaxUDPPacketSize);
		CurrentReadQueue = &Queue2;
		ReceiveTask->setWriteQueue( &Queue1 );
		nlassert( ReceiveTask != NULL );
		ReceiveThread = IThread::create( ReceiveTask );
		nlassert( ReceiveThread != NULL );
		ReceiveThread->start();

		// Setup current message placeholder
		CurrentInMsg = new TReceivedMessage();

		// Create the TCP server

		nlinfo( "Starting external TCP socket on port %d", TCPPort);
		CallbackServer = new CCallbackServer;
		CallbackServer->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
		CallbackServer->init (TCPPort);
		CallbackServer->setDisconnectionCallback (cbDisconnect, NULL);
	}

	bool update ()
	{
		try
		{
			// Send ping to every client
			sendPing();

			// Update and manage TCP connections
			CallbackServer->update ();


			// Swap queues
			if ( CurrentReadQueue == &Queue1 )
			{
				CurrentReadQueue = &Queue2;
				ReceiveTask->setWriteQueue( &Queue1 );
			}
			else
			{
				CurrentReadQueue = &Queue1;
				ReceiveTask->setWriteQueue( &Queue2 );
			}

			// Update and manage UDP connections
			while ( ! CurrentReadQueue->empty() )
			{
				sint64 pongTime;

				// Get a UDP message
				CurrentReadQueue->front( CurrentInMsg->data() );
				CurrentReadQueue->pop();
				nlassert( ! CurrentReadQueue->empty() );
				CurrentReadQueue->front( CurrentInMsg->VAddrFrom );
				CurrentReadQueue->pop();
				CurrentInMsg->vectorToAddress();
				pongTime = CurrentInMsg->getDate ();

				// Handle the UDP message

				// Retrieve client info or add one		
				TClientMap::iterator ihm = ClientMap.find( CurrentInMsg->AddrFrom );
				if ( ihm == ClientMap.end() )
				{
					if ( CurrentInMsg->eventType() == TReceivedMessage::User )
					{
						// Handle message for a new client
						handleReceivedPong( NULL, pongTime );
					}
					else
					{
						nlinfo( "Not removing already removed client" );
					}
				}
				else
				{
					// Already existing
					if ( CurrentInMsg->eventType() == TReceivedMessage::RemoveClient )
					{
						// Remove client
						removeClientByAddr( ihm );
					}
					else
					{
						// Handle message
						handleReceivedPong( GETCLIENTA(ihm), pongTime );
					}
				}

				updateStat ();
			}
		}
		catch (const Exception &e)
		{
			nlerrornoex ("Exception not catched: '%s'", e.what());
		}
		return true;
	}

	void release ()
	{
		nlassert( ReceiveTask != NULL );
		nlassert( ReceiveThread != NULL );

		ReceiveTask->requireExit();
		ReceiveTask->DataSock->close();
		ReceiveThread->wait();

		if (ReceiveThread != NULL)
		{
			delete ReceiveThread;
			ReceiveThread = NULL;
		}
	
		if (ReceiveTask != NULL)
		{
			delete ReceiveTask;
			ReceiveTask = NULL;
		}

		if (CurrentInMsg != NULL)
		{
			delete CurrentInMsg;
			CurrentInMsg = NULL;
		}

		if (CallbackServer != NULL)
		{
			delete CallbackServer;
			CallbackServer = NULL;
		}

	}
};

 
NLNET_SERVICE_MAIN (CBenchService, "BS", "bench_service", 45459, EmptyCallbackArray, UDP_DIR, "")
