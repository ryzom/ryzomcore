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

#ifndef CL_MODULE_MESSAGES_H
#define CL_MODULE_MESSAGES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// class IMsgCL
//-----------------------------------------------------------------------------

class IMsgCL: public NLMISC::CRefCount
{
public:
	virtual	~IMsgCL() {}

	virtual const char* getName() const=0;
	virtual void serial(NLMISC::IStream& stream)=0;
};
typedef NLMISC::CSmartPtr<IMsgCL> TMsgCL;


//-----------------------------------------------------------------------------
// class CMsgCLText
//-----------------------------------------------------------------------------

class CMsgCLText: public IMsgCL
{
public:
	const char* getName() const { return "CL_LOG"; }

	CMsgCLText()
	{
	}

	
	CMsgCLText(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCLText(const NLMISC::CSString& lang,const NLMISC::CSString& speaker,const NLMISC::CSString& txt)
	{
		_Lang= lang;
		_Speaker= speaker;
		_Txt= txt;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_Lang);
		stream.serial(_Speaker);
		stream.serial(_Txt);
	}

	const NLMISC::CSString& getLang()		{ return _Lang; }
	const NLMISC::CSString& getSpeaker()	{ return _Speaker; }
	const NLMISC::CSString& getTxt()		{ return _Txt; }

private:
	NLMISC::CSString _Lang;
	NLMISC::CSString _Speaker;
	NLMISC::CSString _Txt;
};
typedef NLMISC::CSmartPtr<CMsgCLText> TMsgCLLog;


//-----------------------------------------------------------------------------
// class CMsgCLWinner
//-----------------------------------------------------------------------------

class CMsgCLWinner: public IMsgCL
{
public:
	const char* getName() const { return "CL_WINNER"; }

	CMsgCLWinner()
	{
	}
	
	CMsgCLWinner(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCLWinner(const NLMISC::CSString& winner)
	{
		_Winner= winner;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_Winner);
	}

	const NLMISC::CSString& getWinner()		{ return _Winner; }

private:
	NLMISC::CSString _Winner;
};
typedef NLMISC::CSmartPtr<CMsgCLWinner> TMsgCLWinner;


//-----------------------------------------------------------------------------
// class CMsgCLEnd
//-----------------------------------------------------------------------------

class CMsgCLEnd: public IMsgCL
{
public:
	const char* getName() const { return "CL_END"; }

	CMsgCLEnd()
	{
	}

	CMsgCLEnd(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	void serial(NLMISC::IStream& stream)
	{
	}

private:
};
typedef NLMISC::CSmartPtr<CMsgCLEnd> TMsgCLEnd;


//-----------------------------------------------------------------------------
#endif
