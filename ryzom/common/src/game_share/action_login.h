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



#ifndef NL_ACTION_LOGIN_H
#define NL_ACTION_LOGIN_H

#include "nel/misc/types_nl.h"

#include "nel/net/login_cookie.h"

#include "action.h"


namespace CLFECOMMON {

/**
 * This action means the entity Id has left the game.
 * Note: No more data or processing than in CAction.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CActionLogin: public CAction
{
public:

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void unpack (NLMISC::CBitMemStream &message)
	{
		uint32 ua, uk, ui;
		message.serial (ua);
		message.serial (uk);
		message.serial (ui);
		Cookie.set (ua, uk, ui);
	}

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is IN BITS, not in bytes (the actual size is this one plus the header size)
	 */
	virtual uint32	size () { return 3*32; }

	static CAction *create () { return new CActionLogin(); }

	void setCookie (NLNET::CLoginCookie &lc)
	{
		Cookie = lc;
	}

	const NLNET::CLoginCookie &getCookie () const { return Cookie; }

protected:

	NLNET::CLoginCookie Cookie;

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &message)
	{
		uint32 ua, uk, ui;
		if (Cookie.isValid ())
		{
			ua = Cookie.getUserAddr ();
			uk = Cookie.getUserKey ();
			ui = Cookie.getUserId ();
		}
		// if not valid, serialize dummy things
		message.serial (ua);
		message.serial (uk);
		message.serial (ui);
	}

	virtual void	reset()
	{
		Cookie.clear ();
	}

	friend class CActionFactory;

};

}

#endif // NL_ACTION_LOGIN_H

/* End of action_login.h */
