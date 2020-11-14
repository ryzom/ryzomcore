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

#ifndef CC_MODULE_MESSAGES_H
#define CC_MODULE_MESSAGES_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// class CMsgCCTitle
//-----------------------------------------------------------------------------

class CMsgCCTitle: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_TITLE"; }

	CMsgCCTitle()
	{
	}

	
	CMsgCCTitle(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCCTitle(const NLMISC::CSString& lang,const NLMISC::CSString& txt)
	{
		_Lang= lang;
		_Txt= txt;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_Lang);
		stream.serial(_Txt);
	}

	const NLMISC::CSString& getLang()	{ return _Lang; }
	const NLMISC::CSString& getTxt()	{ return _Txt; }

private:
	NLMISC::CSString _Lang;
	NLMISC::CSString _Txt;
};
typedef NLMISC::CSmartPtr<CMsgCCTitle> TMsgCCTitlePtr;


//-----------------------------------------------------------------------------
// class CMsgCCText
//-----------------------------------------------------------------------------

class CMsgCCText: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_TEXT"; }

	CMsgCCText()
	{
	}

	CMsgCCText(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCCText(const NLMISC::CSString& lang,const NLMISC::CSString& speaker,const NLMISC::CSString& txt)
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
typedef NLMISC::CSmartPtr<CMsgCCText> TMsgCCTextPtr;


//-----------------------------------------------------------------------------
// class CMsgCCAnswer
//-----------------------------------------------------------------------------

class CMsgCCAnswer: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_ANSWER"; }

	CMsgCCAnswer()
	{
	}

	CMsgCCAnswer(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCCAnswer(const NLMISC::CSString& txt)
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
typedef NLMISC::CSmartPtr<CMsgCCAnswer> TMsgCCAnswerPtr;


//-----------------------------------------------------------------------------
// class CMsgCCBegin
//-----------------------------------------------------------------------------

class CMsgCCBegin: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_BEGIN"; }

	CMsgCCBegin()
	{
	}

	CMsgCCBegin(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCCBegin(const NLMISC::CSString& contestName)
	{
		_ContestName= contestName;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial(_ContestName);
	}

	const NLMISC::CSString& getContestName()		{ return _ContestName; }

private:
	NLMISC::CSString _ContestName;
};
typedef NLMISC::CSmartPtr<CMsgCCBegin> TMsgCCBeginPtr;


//-----------------------------------------------------------------------------
// class CMsgCCEndAnswers
//-----------------------------------------------------------------------------

class CMsgCCEndAnswers: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_END_ANSWERS"; }

	CMsgCCEndAnswers()
	{
	}

	CMsgCCEndAnswers(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	void serial(NLMISC::IStream& stream)
	{
	}

private:
};
typedef NLMISC::CSmartPtr<CMsgCCEndAnswers> TMsgCCEndAnswersPtr;


//-----------------------------------------------------------------------------
// class CMsgCCEnd
//-----------------------------------------------------------------------------

class CMsgCCEnd: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_END"; }

	CMsgCCEnd()
	{
	}

	CMsgCCEnd(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	void serial(NLMISC::IStream& stream)
	{
	}

private:
};
typedef NLMISC::CSmartPtr<CMsgCCEnd> TMsgCCEndPtr;


//-----------------------------------------------------------------------------
// class CMsgCCAckWinners
//-----------------------------------------------------------------------------

class CMsgCCAckWinners: public NLMISC::CRefCount
{
public:
	static const char* getName() { return "CC_ACK_WINNERS"; }

	CMsgCCAckWinners()
	{
	}

	CMsgCCAckWinners(GUSNET::TRawMsgBodyPtr msgBody)
	{
		msgBody->serial(*this);
	}

	CMsgCCAckWinners(std::vector<NLMISC::CSString>& winners)
	{
		_Winners= winners;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serialCont(_Winners);
	}

	const std::vector<NLMISC::CSString>& getWinners()		{ return _Winners; }

private:
	std::vector<NLMISC::CSString> _Winners;
};
typedef NLMISC::CSmartPtr<CMsgCCAckWinners> TMsgCCAckWinnersPtr;


//-----------------------------------------------------------------------------
#endif
