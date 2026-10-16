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

#ifndef _SCRIPT_COMP_H_
#define _SCRIPT_COMP_H_

#include "ai_script_comp_base.h"

struct ReadFightActionException	:public	NLMISC::Exception
{
	ReadFightActionException(const	std::string	&reason):NLMISC::Exception(reason){}
};

class CFightScriptCompReader
	:public	NLMISC::CRefCount
{
public:
	CFightScriptCompReader()
	{}
	virtual ~CFightScriptCompReader()
	{}
	virtual	CFightScriptComp	*create	(const	std::string	&inStr) = 0;
	virtual	std::string	getName	()	const =0;

	static	CFightScriptCompReader	*getScriptReader	(const std::string &str);
	
	static	CFightScriptComp	*createScriptComp(const std::string &str);
};


class CFightScript
{
public:
	CFightScript();
	virtual ~CFightScript()
	{}

	void	add(CFightScriptCompReader	*reader);

	typedef	CHashMap<std::string, NLMISC::CSmartPtr<CFightScriptCompReader> >	TFightScriptMap;

	static	TFightScriptMap	_ScriptCompList;
};


//////////////////////////////////////////////////////////////////////////
//	Select Filter


class	CFightSelectFilter
		:public	CFightScriptComp
{
public:
	CFightSelectFilter(CFightScriptComp	*customComp, std::string param)
		:_CustomComp(customComp)
		,_Param(param)
	{}

	virtual ~CFightSelectFilter() NL_OVERRIDE
	{}
	bool	update	(CSpawnBot	&bot)	const NL_OVERRIDE;
	const	std::string	&getParam()	const
	{
		return	_Param;
	}

	std::string	toString() const NL_OVERRIDE;

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
	std::string	_Param;
};


class	CFightSelectFilterReader
		:public	CFightScriptCompReader
{
public:
	CFightSelectFilterReader()			{}
	virtual ~CFightSelectFilterReader() NL_OVERRIDE	{}

	CFightScriptComp	*create(const	std::string	&inStr) NL_OVERRIDE;
	std::string	getName	()	const NL_OVERRIDE
	{
		return	std::string("SELECT");
	}

};

#endif
