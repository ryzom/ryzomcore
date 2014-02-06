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

#ifndef NL_SECURITY_CHECK_H
#define NL_SECURITY_CHECK_H

#include "nel/misc/types_nl.h"
#include "nel/net/login_cookie.h"
#include "nel/misc/md5.h"
#include "game_share/r2_basic_types.h"


/**
 * CSecurityCode
 */
class CSecurityCode
{
public:
	CSecurityCode() : AsNum(0) {}
	void serial( NLMISC::IStream& s ) { s.serial(AsNum); } // dependency on msg.xml:CLIENT_QUIT_REQUEST format
private:
	union
	{
		uint8 Data [2];
		uint16 AsNum;
	};

	friend class CSecurityCheckForFastDisconnection;
	friend bool operator!=( const CSecurityCode& sc1, const CSecurityCode& sc2 );
};

inline bool operator!=( const CSecurityCode& sc1, const CSecurityCode& sc2 )
{
	return sc1.AsNum != sc2.AsNum;
}


/**
 * CSecurityCheckForFastDisconnection
 */
class CSecurityCheckForFastDisconnection
{
public:
	/// Default constructor
	CSecurityCheckForFastDisconnection();
	/// Set session (or can be set by receiveSecurityCode())
	void setSessionId(TSessionId sessionId) { Block.SessionId = sessionId; }
	/// Set cookie
	void setCookie(const NLNET::CLoginCookie& cookie) { Block.Cookie.set(cookie.getUserAddr(), cookie.getUserKey(), cookie.getUserId()); } // don't use the default generated bitwise assignment operator, because of padding junk that would be copied
	/// Return the security code
	CSecurityCode encode(const char *passPhrase);
	/// Check  the security code
	void check(const char *passPhrase);

	/// Read some data from stream
	void receiveSecurityCode(NLMISC::IStream& msgin);
	/// Write some data to stream
	static void forwardSecurityCode(NLMISC::IStream& msgout, TSessionId sessionId, CSecurityCode& securityCode);

private:
	struct CBlock
	{
		TSessionId SessionId;
		NLNET::CLoginCookie Cookie;
		char PassPhrase [10];
	};
	CBlock Block;
	CSecurityCode SecurityCode;
};

#endif // NL_SECURITY_CHECK_H

/* End of security_check.h */
