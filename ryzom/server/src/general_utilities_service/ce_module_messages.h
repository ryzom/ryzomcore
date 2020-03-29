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

#ifndef CE_MODULE_MESSAGES_H
#define CE_MODULE_MESSAGES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// class CMsgCERegister
//-----------------------------------------------------------------------------

class CMsgCERegister: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CE_REGISTER"; }

	CMsgCERegister()
	{
	}

	CMsgCERegister(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCERegister(const NLMISC::CSString& txt)
	{
		_Txt= txt;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_Txt);
	}

	const NLMISC::CSString& getTxt()		{ return _Txt; }

private:
	NLMISC::CSString _Txt;
};
typedef NLMISC::CSmartPtr<CMsgCERegister> TMsgCERegisterPtr;


//-----------------------------------------------------------------------------
// class CMsgCEWinner
//-----------------------------------------------------------------------------

class CMsgCEWinner
{
public:
	static const char* getName() { return "CE_WINNER"; }

	CMsgCEWinner()
	{
	}

	CMsgCEWinner(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCEWinner(const NLMISC::CSString& winnerName)
	{
		_WinnerName= winnerName;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_WinnerName);
	}

	const NLMISC::CSString& getWinnerName()		{ return _WinnerName; }

private:
	NLMISC::CSString _WinnerName;
};
typedef NLMISC::CSmartPtr<CMsgCEWinner> TMsgCEWinnerPtr;


//-----------------------------------------------------------------------------
#endif
