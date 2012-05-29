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

#include "yubo_chat.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


// ***************************************************************************
CYuboChat::CYuboChat()
{
	_Connected= false;
	_CommandState= None;
	_AutoLoginState= UserUnknown;
}

// ***************************************************************************
void	CYuboChat::connect(const string &url, const std::string &login, const std::string &pwd)
{
	if(_Connected)
		return;

	_URL= url;
	_Login= login;
	_Password= pwd;
	// mark as connected even if the connection failed (to allow display of error message)
	_Connected= true;


	// Connect the socket
	try
	{
		// connect in non blocking mode
		_Sock.connect(CInetAddress(_URL));
		_Sock.setNonBlockingMode(true);

		if (!_Sock.connected())
		{
			// Add an error string
			addStringReceived(toString("ERROR: Cannot connect to the url: %s", _URL.c_str()));
			return;
		}
	}
	catch(const Exception &e)
	{
		addStringReceived(toString("ERROR: exception with server %s: %s", _URL.c_str(), e.what()));
	}
}

// ***************************************************************************
void	CYuboChat::disconnect()
{
	if(!_Connected)
		return;

	// verify if the actual connection was not broken first
	if(_Sock.connected())
	{
		try
		{
			_Sock.disconnect();
		}
		catch(const Exception &e)
		{
			addStringReceived(toString("ERROR: exception with server %s: %s", _URL.c_str(), e.what()));
		}
	}

	// must reset the sock, else block at next connect (due to nonblocking stuff i think)
	contReset(_Sock);

	// flush
	_CurrentReceivedString.clear();
	_CommandState= None;
	_AutoLoginState= UserUnknown;
	_Connected= false;
}

// ***************************************************************************
void	CYuboChat::send(const ucstring &msg)
{
	if(!_Connected)
		return;

	// verify also that the socket was not disconnected by peer
	if(_Sock.connected())
	{
		// append to the queue of user string to send
		_StringsToSend.push_back(msg);

		// flush string only when the user is logued
		// Allow also the user to enter his login if autologin has failed
		if(_AutoLoginState!=UserUnknown)
		{
			while(!_StringsToSend.empty())
			{
				sendInternal(_StringsToSend.front());
				_StringsToSend.pop_front();
			}
		}
	}

}

// ***************************************************************************
void	CYuboChat::receive(std::list<ucstring> &msgList)
{
	// verify also that the socket was not disconnected by peer
	if(_Connected && _Sock.connected())
	{
		// **** Receive chars from chat
		bool	receiving= true;
		while(receiving)
		{
			uint32 size = 1;
			char	c;

			if (_Sock.receive((uint8*)&c, size, false) == CSock::Ok && size==1)
			{
				// the char is a command id?
				if(_CommandState==WaitCommandId)
				{
					// if the command is an option negociation
					if((uint8)c>=251 && (uint8)c<=254)
						// next char is the option id
						_CommandState= WaitOptionId;
					else
						// back to normal state
						_CommandState= None;
				}
				// the char is an option id?
				else if(_CommandState==WaitOptionId)
				{
					// just ignore and back to normal mode
					_CommandState= None;
				}
				// end of line?
				else if(c==0 || c=='\n')
				{
					if(!_CurrentReceivedString.empty())
					{
						addStringReceived(_CurrentReceivedString);
						// clear the received string
						_CurrentReceivedString.clear();
					}
				}
				// special telnet code?
				else if((uint8)c==255)
				{
					// next char is the command id
					_CommandState=WaitCommandId;
				}
				// normal char
				else
				{
					if(c=='\r')
					{
						if(_CurrentReceivedString.empty())
							_CurrentReceivedString=" ";
					}
					else
						_CurrentReceivedString += c;
				}

				// test if must send login/pwd
				if(toLower(_CurrentReceivedString)=="<sor> login: ")
				{
					// flush display
					addStringReceived(_CurrentReceivedString);
					_CurrentReceivedString.clear();

					// autolog
					if(_AutoLoginState==UserUnknown)
					{
						_AutoLoginState=LoginEntered;
						// if autologin valid
						if(!_Login.empty())
							sendInternal(_Login);
					}
				}
				else if(toLower(_CurrentReceivedString)=="<sor> password: ")
				{
					// flush display
					addStringReceived(_CurrentReceivedString);
					_CurrentReceivedString.clear();

					// autolog
					if(_AutoLoginState==LoginEntered)
					{
						_AutoLoginState=Logged;
						// if autologin valid
						if(!_Password.empty())
						{
							sendInternal(_Password);
							// cool stuff
							sendInternal(string(".emote is in game"));
							sendInternal(string(".set client Ryzom"));
						}
					}
				}

			}
			else
			{
				receiving= false;
			}
		}
	}

	// return the list of received string (NB: even if not connected, can contains conection errors)
	msgList= _ReceivedStrings;
	_ReceivedStrings.clear();
}

// ***************************************************************************
void CYuboChat::addStringReceived(const std::string &str)
{
	_ReceivedStrings.push_back(str);
}

// ***************************************************************************
void CYuboChat::sendInternal(const ucstring &msg)
{
	try
	{
		string	toSend= msg.toString();
		// replace any 255 char (eg: y trema) with '?'
		for(uint i=0;i<toSend.size();i++)
		{
			if((uint8)toSend[i]==255)
				toSend[i]= '?';
		}
		// append linefield
		toSend+= "\r\n";
		uint32	size= (uint32)toSend.size();
		uint32	off= 0;
		// until the whole message is sent
		while(size!=0)
		{
			if (_Sock.send((uint8 *)toSend.c_str()+off, size, false) != CSock::Ok)
			{
				addStringReceived(toString("ERROR: Can't send data to the server %s", _URL.c_str()));
				return;
			}
			// maybe send the rest of the message (very rare....)
			off+=size;
			size= (uint32)toSend.size()-off;
		}
	}
	catch(const Exception &e)
	{
		addStringReceived(toString("ERROR: exception with server %s: %s", _URL.c_str(), e.what()));
	}
}

