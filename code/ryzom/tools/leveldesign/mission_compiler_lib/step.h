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

#include "nel/misc/types_nl.h"
#include "nel/ligo/primitive.h"
#include <string>

#ifndef __STEP_H__
#define __STEP_H__


/** Action and objective base class */
class IStepContent
{
public:
	enum TStepContentType
	{
		objective,
		action
	};
protected:
	std::string				_ContentName;
	TStepContentType		_ContentType;
	/// post action list
	std::vector<IStepContent*>	_PostActions;

	// called by base class to retrieve the predefined parameters lists
	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef) = 0;
public:

	virtual void init(CMissionData &md, NLLIGO::IPrimitive *prim);

	/// Generate the mission script code for this step content.
	std::string genStepContentCode(CMissionData &md);

	/// Generate the phrase string.
	std::string genStepContentPhrase();

	static IStepContent *createStepContent(CMissionData &md, NLLIGO::IPrimitive *prim);

	virtual void fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints);

protected:
	/// Generate the mission script code for this step content.
	virtual std::string genCode(CMissionData &md) = 0;
	/// Generate the phrase string.
	virtual std::string genPhrase() { return std::string();}
};


/** Step base class */
class IStep
{
protected:
	/// The name of the step
	std::string		_StepName;
	/// The type of the step
	std::string		_StepType;
	/// Pointer on primitive that generate this step
	NLLIGO::IPrimitive	*_Primitive;

	/// The list of pre-action
	std::vector<IStepContent *>	_PreActions;
	/// The list of objectives
	std::vector<IStepContent *>	_Objectives;
	/// The list of post action
	std::vector<IStepContent *>	_PostActions;
	/// Substeps used for sequence interruption (like an if)
	std::vector<IStep*>			_SubSteps;
public:
	/// Flag this step a last of a branch, the compiler will generate a fail after it.
	bool	EndOfBranch;
	/// Flag this step as a jump point. This mean that there is a jump that jump on this step.
	bool	JumpPoint;
	
	IStep(CMissionData &md, NLLIGO::IPrimitive *prim);
	virtual ~IStep() {}

	virtual void init(CMissionData &md, NLLIGO::IPrimitive *prim) {}

	
	/// Get the step name
	const std::string &getStepName()
	{
		return _StepName;
	}

	NLLIGO::IPrimitive *getPrimitive()
	{
		return _Primitive;
	}
	
	/// Return true if the step is an action step.
//	virtual bool isAction() { return false; }

	virtual bool isAJump() { return false; }

	virtual bool isEnd() { return false; }
	
	/// Return a set of primitive pointer on the sub branch of this step (if any)
	virtual NLLIGO::TPrimitiveSet	getSubBranchs() {	return NLLIGO::TPrimitiveSet(); }

	/// Fill a vector of step name where this step eventualy jump (if any)
	void fillStepJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints);

	/// Generate the mission script code for this step.
	virtual std::string genCode(CMissionData &md);

	virtual std::string genCodePreActions(CMissionData &md);
	virtual std::string genCodeObjectives(CMissionData &md);
	virtual std::string genCodePostActions(CMissionData &md);

	void addSubStep(IStep *s) { _SubSteps.push_back(s); }


	/// Generate the string phrase text.
	virtual std::string genPhrase();
	
	/// Factory function to create a new step. The caller is responsible for deleting the allocated object.
	static IStep *createStep(CMissionData &md, NLLIGO::IPrimitive *prim);

protected:
	/// Fill a vector of step name where this step eventualy jump (if any)
	virtual void fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints);

};


/** StepContent FACTORY **/
class IStepContentFactory
{
public:
	virtual IStepContent *createStepContent(CMissionData &md, NLLIGO::IPrimitive *prim) =0;
};

template <class contentClass>
class CStepContentFactory : public IStepContentFactory
{
public:
	IStepContent *createStepContent(CMissionData &md, NLLIGO::IPrimitive *prim)
	{
		IStepContent *ret = new contentClass;
		ret->init(md, prim);
		return ret;
	}
};

#define REGISTER_STEP_CONTENT(className, key)	typedef CStepContentFactory<className> TStepContentFactory##className; \
												NLMISC_REGISTER_OBJECT_INDIRECT(IStepContentFactory, TStepContentFactory##className, string, key)



/** Step FACTORY **/
class IStepFactory
{
public:
	virtual IStep *createStep(CMissionData &md, NLLIGO::IPrimitive *prim) = 0;
};

template <class StepClass>
class CStepFactory : public IStepFactory
{
	IStep *createStep(CMissionData &md, NLLIGO::IPrimitive *prim)
	{
		return new StepClass(md, prim);
	}
};

#define REGISTER_STEP_INDIRECT(stepClass, key)	typedef CStepFactory<stepClass> TStepFactory##stepClass; \
												NLMISC_REGISTER_OBJECT_INDIRECT(IStepFactory, TStepFactory##stepClass, string, string(key));


/** objective base class **/
class CContentObjective : public IStepContent
{
protected:
	std::vector<std::string>	_OverloadObj;
	CPhrase						_OverloadPhrase;
	std::vector<std::string>	_RoleplayObj;
	CPhrase						_RoleplayPhrase;
	bool						_HideObj;

	// Option nb_guild_members_needed, available for each objective
	/*int _NbGuildMembersNeeded;

	std::string genNbGuildMembersNeededOption(CMissionData &md);*/
	
public:
	void init(CMissionData &md, NLLIGO::IPrimitive *prim);
	
	std::string genCode(CMissionData &md);
	
	std::string genPhrase();
};

/** special case for step if **/

class CStepIf : public IStep
{

	enum TIfType
	{
		it_mission_done,
		it_skills,
		it_bricks,
		it_sdb,
		it_race,
		it_cult,
		it_civ,
		it_faction_point,
		it_guild_cult,
		it_guild_civ,
		it_guild_fame,
		it_no_trial,
		it_item_in_inv,
	};

public:

	CStepIf(CMissionData &md, NLLIGO::IPrimitive *prim);
	
	NLLIGO::TPrimitiveSet getSubBranchs();

	void fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints);

	std::string genCode(CMissionData &md);
	
	/// type of test
	TIfType						_IfType;
	/// If parameters
	std::vector<std::string>	_IfParams;
	/// the list of sub branch for the dyn chat.
	NLLIGO::TPrimitiveSet		_SubBranchs;
};

/** special case for step player reconnect **/

class CStepPlayerReconnect : public IStep
{
	// Optional jump at end of failure
	std::string				_JumpTo;
	/// the list of sub branch
	NLLIGO::TPrimitiveSet	_SubBranchs;

public:
	CStepPlayerReconnect(CMissionData &md, NLLIGO::IPrimitive *prim);

	NLLIGO::TPrimitiveSet getSubBranchs();

	std::string genCode(CMissionData &md);

	void fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints);
};

#endif // __STEP_H__