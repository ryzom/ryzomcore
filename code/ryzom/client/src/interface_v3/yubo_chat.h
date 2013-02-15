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

#ifndef NL_YUBO_CHAT_H
#define NL_YUBO_CHAT_H

#include "nel/misc/types_nl.h"
#include <vector>
#include "nel/net/tcp_sock.h"


// ***************************************************************************
/**
 * Yubo Chat (special telnet chat for Game Masters, same channel as the Yubo Klient)
 *	This is the NET part of the yubo chat only (no interface here)
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CYuboChat
{
public:

	CYuboChat();

	void	connect(const std::string &url, const std::string &login, const std::string &pwd);
	void	disconnect();

	// NB: return true even if was disconnected by peer, or if the connect() failed.
	bool	connected() const {return _Connected;}

	void	send(const ucstring &msg);
	void	receive(std::list<ucstring> &msgList);


private:
	bool					_Connected;
	std::string				_URL;
	std::string				_Login;
	std::string				_Password;		// problem if clearly sotred in ram?
	NLNET::CTcpSock			_Sock;
	std::list<ucstring>		_ReceivedStrings;
	std::list<ucstring>		_StringsToSend;
	std::string				_CurrentReceivedString;
	enum TLoginState
	{
		UserUnknown,
		LoginEntered,
		Logged
	};
	TLoginState				_AutoLoginState;
	enum TCommandState
	{
		None,
		WaitCommandId,
		WaitOptionId
	};
	TCommandState			_CommandState;

	void	addStringReceived(const std::string &str);
	void	sendInternal(const ucstring &msg);
};


#endif // NL_YUBO_CHAT_H

/* End of yubo_chat.h */
