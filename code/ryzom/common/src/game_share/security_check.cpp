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

#include "stdpch.h"

#include "security_check.h"

using namespace NLMISC;
using namespace NLNET;


//
CSecurityCheckForFastDisconnection::CSecurityCheckForFastDisconnection()
{
	memset(&Block, 0, sizeof(Block)); // ensure encode() will work the same of different builds
}

//
void CSecurityCheckForFastDisconnection::receiveSecurityCode(NLMISC::IStream& msgin)
{
	msgin.serial(Block.SessionId);
	SecurityCode.serial(msgin);
}

//
void CSecurityCheckForFastDisconnection::forwardSecurityCode(NLMISC::IStream& msgout, TSessionId sessionId, CSecurityCode& securityCode)
{
	msgout.serial(sessionId);
	securityCode.serial(msgout);
}

//
CSecurityCode CSecurityCheckForFastDisconnection::encode(const char *passPhrase)
{
	if (!passPhrase)
		throw Exception("Null passPhrase");
	strncpy(Block.PassPhrase, passPhrase, 10);
	CHashKeyMD5 md5 = getMD5((uint8*)&Block, sizeof(Block));
	CSecurityCode sc; // parts from NLMISC::CHashKeyMD5 (would CRC16 be better?)
	sc.Data[0] = md5.Data[0];
	sc.Data[1] = md5.Data[15];
	return sc;
}

//
void CSecurityCheckForFastDisconnection::check(const char *passPhrase)
{
	if (SecurityCode != encode(passPhrase))
		throw Exception("Check not passed");
}


/*
	// The following code helps ensure a sub portion of the md5 produces a wide range of different data
	for (uint i=1; i!=10000; ++i)
	{
		CSecurityCheckForFastDisconnection securityCheck;
		securityCheck.setSessionId(i);
		securityCheck.setCookie(cookie);
		CHashKeyMD5 md5 = securityCheck.encode(""); // replace by pwd
		uint8 Data[2];
		Data[0] = md5.Data[0];
		Data[1] = md5.Data[15];
		InfoLog->displayNL("%u\t%u", i, Data[0] + (Data[1] << 8));
	}
	for (uint i=1; i!=100; ++i)
	{
		CLoginCookie ck2;
		ck2.set(cookie.getUserAddr(), cookie.generateKey(), cookie.getUserId());
		CSecurityCheckForFastDisconnection securityCheck;
		securityCheck.setSessionId(sessionId);
		securityCheck.setCookie(ck2);
		CHashKeyMD5 md5 = securityCheck.encode(""); // replace by pwd
		uint8 Data[2];
		Data[0] = md5.Data[0];
		Data[1] = md5.Data[15];
		InfoLog->displayNL("%u\t%u", i, Data[0] + (Data[1] << 8));
	}
*/



