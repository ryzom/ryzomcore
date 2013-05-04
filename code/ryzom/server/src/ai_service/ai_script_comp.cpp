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
#include "ai_generic_fight.h"
#include "ai_script_comp.h"
#include "server_share/msg_brick_service.h"

using	namespace	std;
using	namespace	NLMISC;

//	renvoie le bout de la string jusqu'au prochain ',' en respectant les priorités des parenthèses.
void	explodeSubStrings(const	std::string	&str, vector<std::string>	&strings, sint32 parenthesis=0)
{
	const	std::string	separators("(),");
	string::size_type	current=0;
	string::size_type	nextCurrent=current;
	strings.clear();
	
	nextCurrent=str.find_first_of(separators.c_str(), current);
	while (nextCurrent!=std::string::npos)
	{		
		switch(str.at(nextCurrent))
		{
		case '(':
			parenthesis++;
			if (parenthesis==0)
			{
				current=nextCurrent+1;
			}
			break;
		case ')':
			if (parenthesis==0)
			{
				strings.push_back(str.substr(current, nextCurrent-current));
				current=nextCurrent+1;
			}
			parenthesis--;
			break;
		case ',':
			if (parenthesis==0)
			{
				strings.push_back(str.substr(current, nextCurrent-current));
				current=nextCurrent+1;
			}
			break;
		default:
			break;
		}
		nextCurrent=str.find_first_of(separators.c_str(), nextCurrent+1);
	}

}



//////////////////////////////////////////////////////////////////////////
//	Select Filter

bool	CFightSelectFilter::update	(CSpawnBot	&bot)	const
{
	return	_CustomComp->update(bot);
//	nlassert("a filter is not designed to be updated");
}

std::string	CFightSelectFilter::toString() const
{
	return	"SELECT("+_Param+","+_CustomComp->toString()+")";
}



CFightScriptComp	*CFightSelectFilterReader::create	(const	std::string	&inStr)	throw	(ReadFightActionException)
{
	std::vector<std::string>	params;
	explodeSubStrings(inStr, params, -1);

	if	(params.size()!=2)
		throw	ReadFightActionException("SELECT Needs 2 Params: <Filter>,<ScriptComp>");

	CSmartPtr<CFightScriptComp>	scriptComp;
	try
	{
		scriptComp=createScriptComp(params[1]);
	}
	catch (const ReadFightActionException &ex)
	{
		throw	ReadFightActionException("cannot create sub ScriptComp : "+std::string(ex.what()));
	}
	return	new	CFightSelectFilter(scriptComp, params[0]);
}


//////////////////////////////////////////////////////////////////////////
//	Once


class	CFightOnce
		:public	CFightScriptComp
{
public:
	CFightOnce(CFightScriptComp	*customComp)
		:_CustomComp(customComp)
	{
		nlassert(customComp);	//	the creature is hitting, we cannot do anything ..
	}

	virtual ~CFightOnce()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		uint32	dummy;
		if (bot.getProp((size_t)this,dummy))	// check if we already go there (for once).
			return	true;

		if	(!_CustomComp->update(bot))
			return	false;

		bot.setProp((size_t)this,1);
		return	true;
	}

	string	toString() const
	{
		return	"ONCE("+_CustomComp->toString()+")";
	}

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
};


class	CFightOnceReader
		:public	CFightScriptCompReader
{
public:
	CFightOnceReader()			{}
	virtual ~CFightOnceReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);

		if	(params.size()!=1)
			throw	ReadFightActionException("ONCE Needs 1 Param: <ScriptComp>");

		CSmartPtr<CFightScriptComp>	scriptComp;
		try
		{
			scriptComp=createScriptComp(params[0]);
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightOnce(scriptComp);
	}
	std::string	getName	() const
	{
		return	std::string("ONCE");
	}

};



//////////////////////////////////////////////////////////////////////////
//	Timed Filter


class	CFightTimedFilter
		:public	CFightScriptComp
{
public:
	CFightTimedFilter(CFightScriptComp	*customComp, uint32	deltaTime)
		:_CustomComp(customComp)
		,_DeltaTime(deltaTime)
	{
		nlassert(customComp);	//	the creature is hitting, we cannot do anything ..
	}

	virtual ~CFightTimedFilter()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		uint32	decTime=0;
		if	(bot.getProp((size_t)this, decTime))
			if (CTimeInterface::gameCycle()<decTime)
				return	true;

		if	(!_CustomComp->update(bot))
			return	false;

		bot.setProp((size_t)this, CTimeInterface::gameCycle()+(uint32)_DeltaTime);
		return	true;
	}

	string	toString() const
	{
		return	"EVERY_SEC("+NLMISC::toString(_DeltaTime/10)+","+_CustomComp->toString()+")";
	}

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
	uint32	_DeltaTime;
};


class	CFightTimedFilterReader
		:public	CFightScriptCompReader
{
public:
	CFightTimedFilterReader()			{}
	virtual ~CFightTimedFilterReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);

		if	(params.size()!=2)
			throw	ReadFightActionException("EVERY_SEC Needs 2 Params: <time in seconds>,<ScriptComp>");

		sint time;
		NLMISC::fromString(params[0], time);
		time *= 10;

		CSmartPtr<CFightScriptComp>	scriptComp;
		try
		{
			scriptComp=createScriptComp(params[1]);
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightTimedFilter(scriptComp, time);
	}
	std::string	getName	() const
	{
		return	std::string("EVERY_SEC");
	}

};


//////////////////////////////////////////////////////////////////////////
//	HP Less Filter

class	CFightHPLessFilter
		:public	CFightScriptComp
{
public:
	CFightHPLessFilter(CFightScriptComp	*customComp, float	hpLimit)
		:_CustomComp(customComp)
		,_HPLimit(hpLimit)
	{
		nlassert(customComp);	//	comportment needed.
	}

	virtual ~CFightHPLessFilter()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		if	(bot.hpPercentage()>=_HPLimit)
			return	true;		
		return	_CustomComp->update(bot);
	}

	string	toString() const
	{
		return	"HP%LESS("+NLMISC::toString(_HPLimit)+","+_CustomComp->toString()+")";
	}

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
	float	_HPLimit;
};


class	CFightHPLessFilterReader
		:public	CFightScriptCompReader
{
public:
	CFightHPLessFilterReader()			{}
	virtual ~CFightHPLessFilterReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);

		if	(params.size()!=2)
			throw	ReadFightActionException("HP%LESS Needs 2 Params: <hp limit>,<ScriptComp>");

		float	hpLimit=(float)atof(params[0].c_str());

		CSmartPtr<CFightScriptComp>	scriptComp;
		try
		{
			scriptComp=createScriptComp(params[1]);
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightHPLessFilter(scriptComp, hpLimit);
	}
	std::string	getName	() const
	{
		return	std::string("HP%LESS");
	}

};

//////////////////////////////////////////////////////////////////////////
//	HP More Filter

class	CFightHPMoreFilter
		:public	CFightScriptComp
{
public:
	CFightHPMoreFilter(CFightScriptComp	*customComp, float	hpLimit)
		:_CustomComp(customComp)
		,_HPLimit(hpLimit)
	{
		nlassert(customComp);	//	comportment needed.
	}

	virtual ~CFightHPMoreFilter()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		if	(bot.hpPercentage()<=_HPLimit)
			return	true;
		return	_CustomComp->update(bot);
	}

	string	toString() const
	{
		return	"HP%MORE("+NLMISC::toString(_HPLimit)+","+_CustomComp->toString()+")";
	}

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
	float	_HPLimit;
};


class	CFightHPMoreFilterReader
		:public	CFightScriptCompReader
{
public:
	CFightHPMoreFilterReader()			{}
	virtual ~CFightHPMoreFilterReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);

		if	(params.size()!=2)
			throw	ReadFightActionException("HP%MORE Needs 2 Params: <hp limit>,<ScriptComp>");

		float	hpLimit=(float)atof(params[0].c_str());

		CSmartPtr<CFightScriptComp>	scriptComp;
		try
		{
			scriptComp=createScriptComp(params[1]);
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightHPMoreFilter(scriptComp, hpLimit);
	}
	std::string	getName	() const
	{
		return	std::string("HP%MORE");
	}

};


//////////////////////////////////////////////////////////////////////////
//	Random Filter

class	CFightRandomFilter
		:public	CFightScriptComp
{
public:
	CFightRandomFilter(CFightScriptComp	*customComp, float	random)
		:_CustomComp(customComp)
		,_Random(random)
	{
		nlassert(customComp);	//	comportment needed.
	}

	virtual ~CFightRandomFilter()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		if	(CAIS::rand16(32767)>=(_Random*32767))
			return	true;
		return	_CustomComp->update(bot);
	}

	string	toString() const
	{
		return	"RANDOM("+NLMISC::toString(_Random)+","+_CustomComp->toString()+")";
	}

protected:
private:
	NLMISC::CSmartPtr<CFightScriptComp>	_CustomComp;
	float	_Random;
};


class	CFightRandomFilterReader
		:public	CFightScriptCompReader
{
public:
	CFightRandomFilterReader()			{}
	virtual ~CFightRandomFilterReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);

		if	(params.size()!=2)
			throw	ReadFightActionException("RANDOM Needs 2 Params: <proba>,<ScriptComp>");

		float	random=(float)atof(params[0].c_str());

		CSmartPtr<CFightScriptComp>	scriptComp;
		try
		{
			scriptComp=createScriptComp(params[1]);
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightRandomFilter(scriptComp, random);
	}
	std::string	getName	() const
	{
		return	std::string("RANDOM");
	}

};

//////////////////////////////////////////////////////////////////////////
//	Send Action

class	CFightSendAction
		:public	CFightScriptComp
{
public:
	CFightSendAction(AISHEETS::IAIActionCPtr action, string const& actionName)
	: _Action(action)
	, _ActionName(actionName)
	{
	}

	virtual ~CFightSendAction() { }
	bool	update(CSpawnBot	&bot)	const
	{
		if	(!bot.getAIProfile())
			return	true;
		
		CBotProfileFight	*profile=dynamic_cast<CBotProfileFight*>(bot.getAIProfile());
		if	(!profile)
			return	true;

		if	(!profile->atAttackDist())
			return	false;

		TDataSetRow	dataSetRow;
		if	((CAIEntityPhysical*)bot.getTarget())
			dataSetRow=bot.getTarget()->dataSetRow();
		
		CEGSExecuteAiActionMsg	msg(bot.dataSetRow(), dataSetRow, _Action->SheetId(), bot._DamageCoef, bot._DamageSpeedCoef);
		msg.send(egsString);
		bot.setActionFlags(RYZOMACTIONFLAGS::Attacks);
		return	true;
	}
	string	toString() const
	{
		return	"SEND_ACTION("+_ActionName+")";
	}
protected:
private:
	string	_ActionName;
	AISHEETS::IAIActionCPtr _Action;
};

class	CFightSendActionReader
		:public	CFightScriptCompReader
{
public:
	CFightSendActionReader()			{}
	virtual ~CFightSendActionReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);

		if	(strings.size()!=1)
			throw	ReadFightActionException("SEND_ACTION Needs 1 param");

		NLMISC::CSheetId	sheetId(strings[0].c_str());
		if	(sheetId==NLMISC::CSheetId::Unknown)
			throw	ReadFightActionException("SheetId Unknown "+inStr);

		AISHEETS::IAIActionCPtr action = AISHEETS::CSheets::getInstance()->lookupAction(sheetId);
		if (action.isNull())
			throw	ReadFightActionException("SheetId Unknown "+inStr);

		return	new	CFightSendAction(action, strings[0]);
	}

	std::string	getName	() const
	{
		return	std::string("SEND_ACTION");
	}

};


//////////////////////////////////////////////////////////////////////////
//	Send Self Action

class	CFightSendSelfAction
:public	CFightScriptComp
{
public:
	CFightSendSelfAction(AISHEETS::IAIActionCPtr action, string const& actionName)
	: _Action(action)
	, _ActionName(actionName)
	{
	}
	
	virtual ~CFightSendSelfAction() { }
	bool	update(CSpawnBot	&bot)	const
	{
		CEGSExecuteAiActionMsg	msg(bot.dataSetRow(), bot.dataSetRow(), _Action->SheetId(), bot._DamageCoef, bot._DamageSpeedCoef);
		msg.send(egsString);
		bot.setActionFlags(RYZOMACTIONFLAGS::Attacks);
		return	true;
	}
	string	toString() const
	{
		return	"SEND_SELF_ACTION("+_ActionName+")";
	}
protected:
private:
	string	_ActionName;
	AISHEETS::IAIActionCPtr _Action;
};

class	CFightSendSelfActionReader
:public	CFightScriptCompReader
{
public:
	CFightSendSelfActionReader()			{}
	virtual ~CFightSendSelfActionReader()	{}
	
	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);
		
		if	(strings.size()!=1)
			throw	ReadFightActionException("SEND_SELF_ACTION Needs 1 param");
		
		NLMISC::CSheetId	sheetId(strings[0].c_str());
		if	(sheetId==NLMISC::CSheetId::Unknown)
			throw	ReadFightActionException("SheetId Unknown "+inStr);
		
		AISHEETS::IAIActionCPtr action = AISHEETS::CSheets::getInstance()->lookupAction(sheetId);
		if (action.isNull())
			throw	ReadFightActionException("SheetId Unknown "+inStr);
		
		return	new	CFightSendSelfAction(action, strings[0]);
	}
	
	std::string	getName	() const
	{
		return	std::string("SEND_SELF_ACTION");
	}
	
};

//////////////////////////////////////////////////////////////////////////
//	AggroBlock 

class	CFightAggroBlock
		:public	CFightScriptComp
{
public:
	CFightAggroBlock(const	uint32 time):_Time(time)
	{}

	virtual ~CFightAggroBlock()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		bot.blockAggro(_Time);
		return	true;
	}
	string	toString() const
	{
		return	"AGGRO_BLOCK("+NLMISC::toString(_Time/10)+")";
	}
protected:
private:
	uint32	_Time;
};

class	CFightAggroBlockReader
		:public	CFightScriptCompReader
{
public:
	CFightAggroBlockReader()			{}
	virtual ~CFightAggroBlockReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);

		if	(strings.size()!=1)
			throw	ReadFightActionException("AGGRO_BLOCK Needs 1 param");

		uint32	time;
		NLMISC::fromString(strings[0], time);
		time *= 10;
		return	new	CFightAggroBlock(time);
	}

	std::string	getName	() const
	{
		return	std::string("AGGRO_BLOCK");
	}

};


//////////////////////////////////////////////////////////////////////////
//	AggroChange 

class	CFightAggroChange
		:public	CFightScriptComp
{
public:
	CFightAggroChange()
	{}

	virtual ~CFightAggroChange()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		CAIEntityPhysical	*target=bot.getTarget();
		if	(!target)
			return	true;

		bot.minimizeAggroFor(target->dataSetRow());
		return	true;
	}
	string	toString() const
	{
		return	"AGGRO_CHANGE()";
	}
protected:
private:
};

class	CFightAggroChangeReader
		:public	CFightScriptCompReader
{
public:
	CFightAggroChangeReader()			{}
	virtual ~CFightAggroChangeReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		return	new	CFightAggroChange();
	}

	std::string	getName	() const
	{
		return	std::string("AGGRO_CHANGE");
	}

};


//////////////////////////////////////////////////////////////////////////
//	DamageCoef 

class	CFightDamageCoef
		:public	CFightScriptComp
{
public:
	CFightDamageCoef(const	float coef)	:_Coef(coef)
	{}

	virtual ~CFightDamageCoef()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		bot._DamageCoef=_Coef;
		return	true;
	}
	string	toString() const
	{
		return	"DAMAGE_COEF("+NLMISC::toString(_Coef)+")";
	}
protected:
private:
	float	_Coef;
};

class	CFightDamageCoefReader
		:public	CFightScriptCompReader
{
public:
	CFightDamageCoefReader()			{}
	virtual ~CFightDamageCoefReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);

		if	(strings.size()!=1)
			throw	ReadFightActionException("DAMAGE_COEF Needs 1 param");

		const	float	coef=(float)atof(strings[0].c_str());
		return	new	CFightDamageCoef(coef);
	}

	std::string	getName	() const
	{
		return	std::string("DAMAGE_COEF");
	}

};



//////////////////////////////////////////////////////////////////////////
//	GroupDamageCoef 

class	CFightGroupDamageCoef
:public	CFightScriptComp
{
public:
	CFightGroupDamageCoef(const	float coef)	:_Coef(coef)
	{}
	
	virtual ~CFightGroupDamageCoef()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		CSpawnGroup			&spawnGroup=bot.spawnGrp();
		CAliasCont<CBot>	&bots=spawnGroup.getPersistent().bots();

		float	damageCoef=1.f;
		for	(sint32 nbRec=bots.size()-(spawnGroup.nbSpawnedBot()+spawnGroup.nbBotToDespawn());nbRec>=0;nbRec--)
			damageCoef*=_Coef;
		
		for (CCont<CBot>::iterator	it=bots.begin(), itEnd=bots.end(); it!=itEnd; ++it)
		{
			CSpawnBot	*spawnBot=it->getSpawnObj();
			if	(spawnBot)
				spawnBot->_DamageCoef=damageCoef;
		}

		return	true;
	}
	string	toString() const
	{
		return	"UPDATE_GROUP_DAMAGE_COEF("+NLMISC::toString(_Coef)+")";
	}
protected:
private:
	float	_Coef;
};

class	CFightGroupDamageCoefReader
:public	CFightScriptCompReader
{
public:
	CFightGroupDamageCoefReader()			{}
	virtual ~CFightGroupDamageCoefReader()	{}
	
	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);
		
		if	(strings.size()!=1)
			throw	ReadFightActionException("UPDATE_GROUP_DAMAGE_COEF Needs 1 param");
		
		const	float	coef=(float)atof(strings[0].c_str());
		return	new	CFightGroupDamageCoef(coef);
	}
	
	std::string	getName	() const
	{
		return	std::string("UPDATE_GROUP_DAMAGE_COEF");
	}
	
};


//////////////////////////////////////////////////////////////////////////
//	DamageSpeedCoef 

class	CFightDamageSpeedCoef
		:public	CFightScriptComp
{
public:
	CFightDamageSpeedCoef(const	float coef)	:_Coef(coef)
	{}

	virtual ~CFightDamageSpeedCoef()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		bot._DamageSpeedCoef=_Coef;
		return	true;
	}
	string	toString() const
	{
		return	"DAMAGE_SPEED_COEF("+NLMISC::toString(_Coef)+")";
	}
protected:
private:
	float	_Coef;
};

class	CFightDamageSpeedCoefReader
		:public	CFightScriptCompReader
{
public:
	CFightDamageSpeedCoefReader()			{}
	virtual ~CFightDamageSpeedCoefReader()	{}

	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);

		if	(strings.size()!=1)
			throw	ReadFightActionException("DAMAGE_SPEED_COEF Needs 1 param");

		const	float	coef=(float)atof(strings[0].c_str());
		return	new	CFightDamageSpeedCoef(coef);
	}

	std::string	getName	() const
	{
		return	std::string("DAMAGE_SPEED_COEF");
	}

};

//////////////////////////////////////////////////////////////////////////
//	SetRandomTarget 

class	CFightSetRandomTarget
:public	CFightScriptComp
{
public:
	CFightSetRandomTarget(const	float coef)	:_Coef(coef)
	{}
	
	virtual ~CFightSetRandomTarget()
	{}
	bool	update(CSpawnBot	&bot)	const
	{
		const	CBotAggroOwner::TBotAggroList	&list=bot.getBotAggroList();
		size_t size = list.size();
		if (size>0)
		{
			CBotAggroOwner::TBotAggroList::const_iterator it = list.begin();
			for (size_t i=0; i<size; ++i)
				++it;
			if (it!=list.end())
				bot.maximizeAggroFor(it->first);
		}
		
		return	true;
	}
	string	toString() const
	{
		return	"SET_RANDOM_TARGET()";
	}
protected:
private:
	float	_Coef;
};

class	CFightSetRandomTargetReader
:public	CFightScriptCompReader
{
public:
	CFightSetRandomTargetReader()			{}
	virtual ~CFightSetRandomTargetReader()	{}
	
	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	strings;
		explodeSubStrings(inStr, strings, -1);
		
		if	(	strings.size()!=1
			||	strings[0]!="")
			throw	ReadFightActionException("SET_RANDOM_TARGET Needs 0 param");
		
		const	float	coef=(float)atof(strings[0].c_str());
		return	new	CFightSetRandomTarget(coef);
	}
	
	std::string	getName	() const
	{
		return	std::string("SET_RANDOM_TARGET");
	}
	
};


//////////////////////////////////////////////////////////////////////////
//	Once


class	CFightMult
:public	CFightScriptComp
{
public:
	CFightMult(const	std::vector<CSmartPtr<CFightScriptComp> >	&customComps)
		:_CustomComps(customComps)
	{
	}
	
	virtual ~CFightMult()
	{}
	bool	update	(CSpawnBot	&bot)	const
	{
		for	(uint32	i=0;i<_CustomComps.size();i++)
			_CustomComps[i]->update(bot);
		return	true;
	}
	
	string	toString() const
	{
		string	outputString="MULT(";

		for	(uint32	i=0;i<_CustomComps.size();i++)
		{
			if (i>0)
				outputString+=",";			// add a "," to separate this param from previous ..
			outputString+=_CustomComps[i]->toString();
		}
		
		return	outputString+")";
	}
	
protected:
private:
	const	std::vector<CSmartPtr<CFightScriptComp> >	_CustomComps;
};


class	CFightMultReader
:public	CFightScriptCompReader
{
public:
	CFightMultReader()			{}
	virtual ~CFightMultReader()	{}
	
	CFightScriptComp	*create	(const	std::string	&inStr)	throw	(ReadFightActionException)
	{
		vector<string>	params;
		explodeSubStrings(inStr, params, -1);
		
		const	uint32	nbSubScript=(uint32)params.size();
		
		std::vector<CSmartPtr<CFightScriptComp> >	scriptComps;
		try
		{
			for (uint32 i=0;i<nbSubScript;i++)
				scriptComps.push_back(createScriptComp(params[i]));
		}
		catch (const ReadFightActionException &ex)
		{
			throw	ReadFightActionException("cannot create sub ScriptComp : "+string(ex.what()));
		}
		return	new	CFightMult(scriptComps);
	}
	std::string	getName	() const
	{
		return	std::string("MULT");
	}
	
};

//////////////////////////////////////////////////////////////////////////
//	CFightScript


CFightScript::TFightScriptMap	CFightScript::_ScriptCompList;

CFightScript	justInstanciatedToRegisterReaders;

CFightScript::CFightScript()
{
	add(new	CFightSelectFilterReader());
	
	add(new CFightMultReader());
	add(new	CFightOnceReader());
	add(new CFightTimedFilterReader());
	add(new CFightRandomFilterReader());
	add(new CFightHPLessFilterReader());
	add(new CFightHPMoreFilterReader());	
	
	add(new	CFightAggroBlockReader());
	add(new	CFightAggroChangeReader());
	add(new	CFightDamageCoefReader());
	add(new	CFightGroupDamageCoefReader());
	add(new	CFightDamageSpeedCoefReader());
	add(new CFightSendActionReader());
	add(new CFightSendSelfActionReader());
	add(new	CFightSetRandomTargetReader());
}

void	CFightScript::add(CFightScriptCompReader	*reader)
{
	nlassert(reader!=NULL);
	nlassert(_ScriptCompList.find(reader->getName())==_ScriptCompList.end());
	_ScriptCompList.insert(std::make_pair(reader->getName(),reader));
}


CFightScriptCompReader	*CFightScriptCompReader::getScriptReader	(const string &str) throw (ReadFightActionException)
{
	CFightScript::TFightScriptMap::iterator	it=CFightScript::_ScriptCompList.find(str);
	if	(it==CFightScript::_ScriptCompList.end())
		throw	ReadFightActionException("Unknown ScriptComp "+str);

	return	&(*(it->second));
}

CFightScriptComp	*CFightScriptCompReader::createScriptComp	(const string &str)	throw	(ReadFightActionException)
{
	string	scriptCompName;
	{
		const	string::size_type	index=str.find_first_of("()", 0);
		if	(index==string::npos)
			throw	ReadFightActionException("ScriptComp Creation of :"+str+" Failed because of bad Syntax");
		scriptCompName=str.substr(0,index);
	}
	
	try
	{
		return	getScriptReader	(scriptCompName)->create(str);
	}
	catch (const ReadFightActionException &e)
	{
		throw	ReadFightActionException(string("ScriptComp creation failed : ")+string(e.what()));
	}

}
