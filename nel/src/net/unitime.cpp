// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"
#include "nel/net/naming_client.h"
#include "nel/net/message.h"

#include "nel/net/unitime.h"

using namespace NLMISC;
using namespace std;

namespace NLNET
{

TTime _CUniTime::_SyncUniTime = 0;
TTime _CUniTime::_SyncLocalTime = 0;
bool _CUniTime::_Simulate = false;

bool _CUniTime::Sync = false;


void _CUniTime::setUniTime (NLMISC::TTime /* uTime */, NLMISC::TTime /* lTime */)
{
	nlstop;
/*	if (Sync)
	{
		TTime lt = getLocalTime ();
		TTime delta = uTime - lTime + _SyncLocalTime - _SyncUniTime;

		nlinfo ("_CUniTime::setUniTime(%" NL_I64 "d, %" NL_I64 "d): Resyncing delta %" NL_I64 "dms",uTime,lTime,delta);
	}
	else
	{
		nlinfo ("_CUniTime::setUniTime(%" NL_I64 "d, %" NL_I64 "d)",uTime,lTime);
		Sync = true;
	}
	_SyncUniTime = uTime;
	_SyncLocalTime = lTime;
*/}

void _CUniTime::setUniTime (NLMISC::TTime /* uTime */)
{
	nlstop;
//	setUniTime (uTime, getLocalTime ());
}



TTime _CUniTime::getUniTime ()
{
	nlstop;
	return 0;
/*	if (!Sync)
	{
		nlerror ("called getUniTime before calling syncUniTimeFromServer");
	}
	return getLocalTime () - (_SyncLocalTime - _SyncUniTime);
*/
}


const char *_CUniTime::getStringUniTime ()
{
	nlstop;
	return getStringUniTime(_CUniTime::getUniTime());
}


const char *_CUniTime::getStringUniTime (TTime ut)
{
	nlstop;
	static char str[512];

	uint32 ms = (uint32) (ut % 1000); // time in ms 1000ms dans 1s
	ut /= 1000;

	uint32 s = (uint32) (ut % 60); // time in seconds 60s dans 1mn
	ut /= 60;

	uint32 m = (uint32) (ut % 60); // time in minutes 60m dans 1h
	ut /= 60;

	uint32 h = (uint32) (ut % 9); // time in hours 9h dans 1j
	ut /= 9;

	uint32 day = (uint32) (ut % (8*4)); // time in days 8day dans 1month
	ut /= 8;

	uint32 week = (uint32) (ut % 4); // time in weeks 4week dans 1month
	ut /= 4;

	uint32 month = (uint32) (ut % 12); // time in months 12month dans 1year
	ut /= 12;

	uint  year =  (uint32) ut;	// time in years

	smprintf (str, 512, "%02d/%02d/%04d (week %d) %02d:%02d:%02d.%03d", day+1, month+1, year+1, week+1, h, m, s, ms);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// SYNCHRONISATION BETWEEN TIME SERVICE AND OTHER SERVICES ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

static bool GetUniversalTime;
static uint32 GetUniversalTimeSecondsSince1970;
static TTime GetUniversalTimeUniTime;

/*
static void cbGetUniversalTime (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;
	// get the association between a date and unitime
	msgin.serial (GetUniversalTimeSecondsSince1970);
	msgin.serial (GetUniversalTimeUniTime);
	GetUniversalTime = true;
}
*/

/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
static TCallbackItem UniTimeCallbackArray[] =
{
	{ "GUT", cbGetUniversalTime }
};
***************************************************************/

void _CUniTime::syncUniTimeFromService (CCallbackNetBase::TRecordingState /* rec */, const CInetAddress * /* addr */)
{
	nlstop;
/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
	TTime deltaAdjust, lt;
	uint32 firstsecond, nextsecond;
	TTime before, after, delta;

	// create a message with type in the full text format
	CMessage msgout ("AUT");
	CCallbackClient server( rec, "TS.nmr" );
	server.addCallbackArray (UniTimeCallbackArray, sizeof (UniTimeCallbackArray) / sizeof (UniTimeCallbackArray[0]));

	if (addr == NULL)
	{
		CNamingClient::lookupAndConnect ("TS", server);
	}
	else
	{
		server.connect (*addr);
	}

	if (!server.connected()) goto error;

	server.send (msgout);

	// before time
	before = CTime::getLocalTime ();

	// receive the answer
	GetUniversalTime = false;
	while (!GetUniversalTime)
	{
		if (!server.connected()) goto error;

		server.update ();

		nlSleep( 0 );
	}

	// after, before and delta is not used. It's only for information purpose.
	after = CTime::getLocalTime ();
	delta = after - before;

	nlinfo ("_CUniTime::syncUniTimeFromService(): ping:%" NL_I64 "dms, time:%ds, unitime:%" NL_I64 "dms", delta, GetUniversalTimeSecondsSince1970, GetUniversalTimeUniTime);

// <-- from here to the "-->" comment, the block must be executed in less than one second or an infinite loop occurs

	// get the second
	firstsecond = CTime::getSecondsSince1970 ();
	nextsecond = firstsecond+1;

	// wait the next start of the second (take 100% of CPU to be more accurate)
	while (nextsecond != CTime::getSecondsSince1970 ())
		nlassert (CTime::getSecondsSince1970 () <= nextsecond);

// -->

	// get the local time of the beginning of the next second
	lt = CTime::getLocalTime ();

	if ( ! _Simulate )
	{
		if (abs((sint32)((TTime)nextsecond - (TTime)GetUniversalTimeSecondsSince1970)) > 10)
		{
			nlerror ("the time delta (between me and the Time Service) is too big (more than 10s), servers aren't NTP synchronized");
			goto error;
		}

		// compute the delta between the other side and our side number of second since 1970
		deltaAdjust = ((TTime) nextsecond - (TTime) GetUniversalTimeSecondsSince1970) * 1000;

		// adjust the unitime to the current localtime
		GetUniversalTimeUniTime += deltaAdjust;

		nlinfo ("_CUniTime::syncUniTimeFromService(): rtime:%ds, runitime:%" NL_I64 "ds, rlocaltime:%" NL_I64 "d, deltaAjust:%" NL_I64 "dms", nextsecond, GetUniversalTimeUniTime, lt, deltaAdjust);
	}
	else
	{
		nlinfo ("_CUniTime::syncUniTimeFromService(): runitime:%" NL_I64 "ds, rlocaltime:%" NL_I64 "d", GetUniversalTimeUniTime, lt);
	}

	_CUniTime::setUniTime (GetUniversalTimeUniTime, lt);

	server.disconnect ();
	return;

error:
	nlerror ("Time Service is not found, lost or can't synchronize universal time");
***************************************************************/

}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// SYNCHRONISATION BETWEEN CLIENT AND SHARD ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// Server part

static void cbServerAskUniversalTime (CMessage& /* msgin */, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;
	TTime ut = _CUniTime::getUniTime ();

	// afficher l adresse de celui qui demande
	nlinfo("UT: Send the universal time %" NL_I64 "d to '%s'", ut, netbase.hostAddress(from).asString().c_str());

	CMessage msgout ("GUT");
	msgout.serial (ut);
	netbase.send (msgout, from);
}

TCallbackItem ServerTimeServiceCallbackArray[] =
{
	{ "AUT", cbServerAskUniversalTime },
};

void _CUniTime::installServer (CCallbackServer *server)
{
	nlstop;
	static bool alreadyAddedCallback = false;
	nlassert (server != NULL);
	nlassert (!alreadyAddedCallback);

	server->addCallbackArray (ServerTimeServiceCallbackArray, sizeof (ServerTimeServiceCallbackArray) / sizeof (ServerTimeServiceCallbackArray[0]));
	alreadyAddedCallback = true;
}

// Client part

static bool GetClientUniversalTime;
static TTime GetClientUniversalTimeUniTime;

/*
static void cbClientGetUniversalTime (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlstop;
	// get the association between a date and unitime
	msgin.serial (GetClientUniversalTimeUniTime);
	GetClientUniversalTime = true;
}
*/

/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
static TCallbackItem ClientUniTimeCallbackArray[] =
{
	{ "GUT", cbClientGetUniversalTime }
};
***************************************************************/


void _CUniTime::syncUniTimeFromServer (CCallbackClient * /* client */)
{
	nlstop;
/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************

	static bool alreadyAddedCallback = false;
	nlassert (client != NULL);

	if (!alreadyAddedCallback)
	{
		client->addCallbackArray (ClientUniTimeCallbackArray, sizeof (ClientUniTimeCallbackArray) / sizeof (ClientUniTimeCallbackArray[0]));
		alreadyAddedCallback = true;
	}

	sint attempt = 0;
	TTime bestdelta = 60000;	// 1 minute

	if (!client->connected ()) goto error;

	while (attempt < 10)
	{
		CMessage msgout ("AUT");

		if (!client->connected()) goto error;

		// send the message
		client->send (msgout);

		// before time
		TTime before = CTime::getLocalTime ();

		// receive the answer
		GetClientUniversalTime = false;
		while (!GetClientUniversalTime)
		{
			if (!client->connected()) goto error;

			client->update ();
		}

		TTime after = CTime::getLocalTime (), delta = after - before;

		if (delta < 10 || delta < bestdelta)
		{
			bestdelta = delta;

			_CUniTime::setUniTime (GetClientUniversalTimeUniTime, (before+after)/2);

			if (delta < 10) break;
		}
		attempt++;
	}
	client->disconnect ();
	nlinfo ("Universal time is %" NL_I64 "dms with a mean error of %" NL_I64 "dms", _CUniTime::getUniTime(), bestdelta/2);
	return;
error:
	nlwarning ("there's no connection or lost or can't synchronize universal time");
***************************************************************/
}


//
// Commands
//
/*
NLMISC_CATEGORISED_COMMAND(nel, time, "displays the universal time", "")
{
	if(args.size() != 0) return false;

	if ( _CUniTime::Sync )
	{
		log.displayNL ("CTime::getLocalTime(): %" NL_I64 "dms, _CUniTime::getUniTime(): %" NL_I64 "dms", CTime::getLocalTime (), _CUniTime::getUniTime ());
		log.displayNL ("_CUniTime::getStringUniTime(): '%s'", _CUniTime::getStringUniTime());
	}
	else
	{
		log.displayNL ("CTime::getLocalTime(): %" NL_I64 "dms <Universal time not sync>", CTime::getLocalTime ());
	}

	return true;
}
*/

} // NLNET
