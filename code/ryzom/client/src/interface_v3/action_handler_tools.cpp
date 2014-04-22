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

// Interface includes
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "../net_manager.h"


using namespace std;
using namespace NL3D;
using namespace NLMISC;




// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s sent", sMsg.c_str());
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d sent", sMsg.c_str(), u8);
		out.serial(u8);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8n1, uint8 u8n2)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), u8n1, u8n2);
		out.serial(u8n1);
		out.serial(u8n2);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d %d sent", sMsg.c_str(), u8n1, u8n2, u8n3);
		out.serial(u8n1);
		out.serial(u8n2);
		out.serial(u8n3);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3, uint8 u8n4)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d %d %d sent", sMsg.c_str(), u8n1, u8n2, u8n3, u8n4);
		out.serial(u8n1);
		out.serial(u8n2);
		out.serial(u8n3);
		out.serial(u8n4);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8n1, uint8 u8n2, uint8 u8n3, uint8 u8n4, uint8 u8n5)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d %d %d %d sent", sMsg.c_str(), u8n1, u8n2, u8n3, u8n4, u8n5);
		out.serial(u8n1);
		out.serial(u8n2);
		out.serial(u8n3);
		out.serial(u8n4);
		out.serial(u8n5);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint8 u8n1, uint32 u32n2)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), u8n1, u32n2);
		out.serial(u8n1);
		out.serial(u32n2);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
void sendMsgToServer(const std::string &sMsg, uint32 u32n1)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d sent", sMsg.c_str(), u32n1);
		out.serial(u32n1);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint32 u32n1, uint8 u8n2)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), u32n1, u8n2);
		out.serial(u32n1);
		out.serial(u8n2);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}

// ***************************************************************************
// sendMsgToServer Helper
void sendMsgToServer(const string &sMsg, uint32 u32n1, uint8 u8n2, uint8 u8n3)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d %d sent", sMsg.c_str(), u32n1, u8n2, u8n3);
		out.serial(u32n1);
		out.serial(u8n2);
		out.serial(u8n3);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}


// ***************************************************************************
void sendMsgToServer(const std::string &sMsg, uint32 u32n1, uint32 u32n2)
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), u32n1, u32n2);
		out.serial(u32n1);
		out.serial(u32n2);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("command : unknown message name : '%s'.", sMsg.c_str());
	}
}


