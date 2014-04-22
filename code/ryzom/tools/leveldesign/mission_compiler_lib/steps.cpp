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


#include "mission_compiler.h"
#include "step.h"
#include "nel/misc/factory.h"
#include <memory>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


// *****
// IStep
// *****

IStep *IStep::createStep(CMissionData &md, IPrimitive *prim)
{
	string *c;
	if (!prim->getPropertyByName("class", c))
		throw EParseException(prim, "Can't find property 'class' on primitive");

	IStep *ret = CFactoryIndirect<IStepFactory, string>::instance().getFactory(*c)->createStep(md, prim);
	ret->init(md, prim);

	return ret;
};

IStep::IStep(CMissionData &md, NLLIGO::IPrimitive *prim)
	:	_Primitive(prim),
	EndOfBranch(false),
	JumpPoint(false)
{
	if (prim == NULL)
		return;

	prim->getPropertyByName("class", _StepType);
	prim->getPropertyByName("name", _StepName);

	// parse the sub prim to create action & objectives;
	IPrimitive *child;
	// parse the preactions
	{
		TPrimitiveClassAndNamePredicate pred("actions", "pre_actions");
		child = getPrimitiveChild(prim, pred);
	}

	if (child)
	{
		for (uint i=0; i<child->getNumChildren(); ++i)
		{
			IPrimitive *subChild;
			child->getChild(subChild, i);
			if (subChild)
			{
				// ok, we got one
				IStepContent *sc = IStepContent::createStepContent(md, subChild);
				if (sc)
					_PreActions.push_back(sc);
			}
		}
	}
	// parse the objectives
	{
		TPrimitiveClassAndNamePredicate pred("mission_objectives", "objectives");
		child = getPrimitiveChild(prim, pred);
	}
	if (child)
	{
		for (uint i=0; i<child->getNumChildren(); ++i)
		{
			IPrimitive *subChild;
			child->getChild(subChild, i);
			if (subChild)
			{
				// ok, we got one
				IStepContent *sc = IStepContent::createStepContent(md, subChild);
				if (sc)
					_Objectives.push_back(sc);
			}
		}
	}
	// parse the post actions
	{
		TPrimitiveClassAndNamePredicate pred("actions", "post_actions");
		child = getPrimitiveChild(prim, pred);
	}
	if (child)
	{
		for (uint i=0; i<child->getNumChildren(); ++i)
		{
			IPrimitive *subChild;
			child->getChild(subChild, i);
			if (subChild)
			{
				// ok, we got one
				IStepContent *sc = IStepContent::createStepContent(md, subChild);
				if (sc)
					_PostActions.push_back(sc);
			}
		}
	}
}

void IStep::fillStepJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints)
{
	uint i;
	// extract step content jump points
	for (i=0; i<_PreActions.size(); ++i)
	{
		_PreActions[i]->fillJump(md, jumpPoints);
	}
	for (i=0; i<_Objectives.size(); ++i)
	{
		_Objectives[i]->fillJump(md, jumpPoints);
	}
	for (i=0; i<_PostActions.size(); ++i)
	{
		_PostActions[i]->fillJump(md, jumpPoints);
	}

	fillJump(md, jumpPoints);

	for (i = 0; i < _SubSteps.size(); ++i)
		if (_SubSteps[i] != NULL)
			_SubSteps[i]->fillStepJump(md, jumpPoints);
}


void IStep::fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints)
{
	// jump to the next step
	IStep *next = md.getNextStep(this);

	if (!EndOfBranch && next)
	{
		TJumpInfo ji(next->getStepName());
		jumpPoints.insert(ji);
	}
	else
	{
		TJumpInfo ji("__end__");
		jumpPoints.insert(ji);
	}
}

std::string IStep::genCode(CMissionData &md)
{
	string ret;

	ret += genCodePreActions(md);
	ret += genCodeObjectives(md);
	ret += genCodePostActions(md);

	return ret;
}

std::string  IStep::genCodePreActions(CMissionData &md)
{
	string ret;
	// generate code for pre actions
	for (uint i=0; i<_PreActions.size(); ++i)
	{
		ret += _PreActions[i]->genStepContentCode(md);
	}
	return ret;
}

std::string  IStep::genCodeObjectives(CMissionData &md)
{
	string ret;
	// generate code for objectives
	for (uint i=0; i<_Objectives.size(); ++i)
	{
		ret += _Objectives[i]->genStepContentCode(md);
	}
	return ret;
}

std::string  IStep::genCodePostActions(CMissionData &md)
{
	string ret;
	//generate code for post actions
	for (uint i=0; i<_PostActions.size(); ++i)
	{
		ret += _PostActions[i]->genStepContentCode(md);
	}
	return ret;
}


std::string IStep::genPhrase()
{
	string ret;
	// generate code for pre actions
	for (uint i=0; i<_PreActions.size(); ++i)
	{
		ret += _PreActions[i]->genStepContentPhrase();
	}
	// generate code for objectives
	for (uint i=0; i<_Objectives.size(); ++i)
	{
		ret += _Objectives[i]->genStepContentPhrase();
	}
	//generate code for post actions
	for (uint i=0; i<_PostActions.size(); ++i)
	{
		ret += _PostActions[i]->genStepContentPhrase();
	}
	//generate code for sub steps
	for (uint i=0; i<_SubSteps.size(); ++i)
	{
		ret += _SubSteps[i]->genPhrase();
	}
	
	return ret;
}


class CStep : public IStep
{
public:
	CStep(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		//  nothing special to do
	}

};
REGISTER_STEP_INDIRECT(CStep, "step");

class CStepObjective : public CStep
{
public:
	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef) 
	{
		numEntry = 0;
		predef.clear();
	};

	CStepObjective(CMissionData &md, IPrimitive *prim, const std::string &prefix = "")
		: CStep(md, prim),
		_HideObj(false),
		_Prefix(prefix)
	{
	}

	void init(CMissionData &md, IPrimitive *prim)
	{
		_HideObj = md.getProperty(prim, _Prefix + "hide_obj", true, false) == "true";
		_OverloadObj = md.getPropertyArray(prim, _Prefix + "overload_objective", false, false);
		_RoleplayObj = md.getPropertyArray(prim, _Prefix + "roleplay_objective", false, false);
		uint32 numEntry;
		CPhrase::TPredefParams params;
		// ask derived class for predefined params
		getPredefParam(numEntry, params);
		_OverloadPhrase.initPhrase(md, prim, _OverloadObj, numEntry, params);
		_RoleplayPhrase.initPhrase(md, prim, _RoleplayObj, numEntry, params);
	}
	
	virtual bool isAction() { return true; }

	string genCode(CMissionData &md)
	{
		string ret;
		if (_HideObj)
			ret = "hide_obj" + NL;
		if (!_OverloadObj.empty())
		{
			ret += "set_obj : " + _OverloadPhrase.genScript(md) + NL;
		}
		if (!_RoleplayObj.empty())
		{
			ret += "set_obj_rp : " + _RoleplayPhrase.genScript(md) + NL;
		}

		return ret;
	}

	string genPhrase()
	{
		string ret;
		ret += _OverloadPhrase.genPhrase();
		ret += _RoleplayPhrase.genPhrase();

		ret += IStep::genPhrase();

		return ret;
	}
	
protected:
	string			_Prefix;
	bool			_HideObj;
	vector<string>	_OverloadObj;
	CPhrase			_OverloadPhrase;
	vector<string>	_RoleplayObj;
	CPhrase			_RoleplayPhrase;
};

class CStepDynChatTalkTo : public CStepObjective
{
//	string		_BotNameVar;
//	string		_BotName;
	TCompilerVarName	_BotName;
	CPhrase		_Phrase;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
		predef.resize(1);
		predef[0].resize(1);
		predef[0][0] = _BotName.getParamInfo();
//		if (_BotNameVar.empty())
//			predef[0][0] = CPhrase::TParamInfo("npc", STRING_MANAGER::bot);
//		else
//			predef[0][0] = CPhrase::TParamInfo(_BotNameVar, STRING_MANAGER::bot);
	}
public:
	CStepDynChatTalkTo(CMissionData &md, IPrimitive *prim, const std::string &prefix = "")
		: CStepObjective(md, prim, prefix)
	{
	}

	void init(CMissionData &md, IPrimitive *prim)
	{
		_BotName.init("npc", STRING_MANAGER::bot, md, prim, "npc_name");
//		_BotNameVar = md.getProperty(prim, "npc_name", false, false);
		// remove the variable tag if any
//		untagVar(_BotNameVar);

//		_BotName = md.getProperty(prim, "npc_name", true, false);
		vector<string> vs;
		vs = md.getPropertyArray(prim, "talk_to_menu", false, false);
		CPhrase::TPredefParams pp(1);
		pp[0].push_back(_BotName.getParamInfo());
//		if (_BotNameVar.empty())
//			pp[0].push_back(CPhrase::TParamInfo("npc", STRING_MANAGER::bot));
//		else
//			pp[0].push_back(CPhrase::TParamInfo(_BotNameVar, STRING_MANAGER::bot));
		_Phrase.initPhrase(md, prim, vs, 0, pp);
 
//		_Phrase.initPhrase(md, prim, vs);

//		if (_Phrase.asAdditionnalParams())
//		{
//			// we need to remove the default 'npc' parameter if add params are found
//			CPhrase temp;
//			temp.initPhrase(md, prim, vs);
//
//			_Phrase = temp;
//		}
		CStepObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CStepObjective::genCode(md);

		ret += "talk_to : "+_BotName._VarValue;
		
		if (!_Phrase.isEmpty())
			ret += " : "+_Phrase.genScript(md);
		ret += NL;
		
		return ret;
	}

	string genPhrase()
	{
		string ret;
		ret = CStepObjective::genPhrase();
		ret += _Phrase.genPhrase();

		return ret;
	}
};

class CStepJump : public IStep
{
public:
	CStepJump(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		_StepName = md.getProperty(prim, "target", true, false);
	}

	void fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
	{
		jumpPoints.insert(TJumpInfo(_StepName, "", false));
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret += "jump : " + _StepName + NL;

		return ret;
	}

	bool isAJump()
	{
		return true;
	}

	string		_StepName;
};
REGISTER_STEP_INDIRECT(CStepJump, "jump_to");

/// pseudo step used in step parsing 
class CStepEnd : public IStep
{
public:
	CStepEnd(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "end"+NL;

		return ret;
	}

	bool isEnd() { return true; }

};
REGISTER_STEP_INDIRECT(CStepEnd, "end");

class CStepFailure : public IStep
{
	// Optional jump at end of failure
	string		_JumpTo;
public:
	CStepFailure(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		// parse the sub prim to create action & objectives;
		IPrimitive *child;
		// parse the pre-actions
		{
			TPrimitiveClassAndNamePredicate pred("actions", "actions");
			child = getPrimitiveChild(prim, pred);
		}
		if (child)
		{
			for (uint i=0; i<child->getNumChildren(); ++i)
			{
				IPrimitive *subChild;
				child->getChild(subChild, i);
				if (subChild)
				{
					// ok, we got one
					IStepContent *sc = IStepContent::createStepContent(md, subChild);
					if (sc)
						_PreActions.push_back(sc);
				}
			}
		}
		// look for an optional jump
		{
			TPrimitiveClassPredicate pred("jump_to");
			child = getPrimitiveChild(prim, pred);
		}
		if (child)
		{
			// ok, we have a jump at end of fail step
			_JumpTo = md.getProperty(child, "target", true, false);
		}
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "failure" + NL;

		string tmp = IStep::genCode(md);
		tabulateLine(tmp, 1);
		ret += tmp;


		if (!_JumpTo.empty())
		{
			ret += "\tjump : "+_JumpTo + NL;
		}

		ret += "/failure"+NL;

		return ret;
	}

	void fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
	{
		IStep::fillJump(md, jumpPoints);
		if (!_JumpTo.empty())
			jumpPoints.insert(TJumpInfo(_JumpTo));
	}

};
REGISTER_STEP_INDIRECT(CStepFailure, "step_failure");

class CStepCrash : public IStep
{
	// Optional jump at end of failure
	string			_JumpTo;
	vector<string>	_AiInstances;
public:
	CStepCrash(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		// parse the sub prim to create action & objectives;
		IPrimitive *child;
		// parse the pre-actions
		{
			TPrimitiveClassAndNamePredicate pred("actions", "actions");
			child = getPrimitiveChild(prim, pred);
		}
		if (child)
		{
			for (uint i=0; i<child->getNumChildren(); ++i)
			{
				IPrimitive *subChild;
				child->getChild(subChild, i);
				if (subChild)
				{
					// ok, we got one
					IStepContent *sc = IStepContent::createStepContent(md, subChild);
					if (sc)
						_PreActions.push_back(sc);
				}
			}
		}

		// parse ai instance list
		vector<string> vs = md.getPropertyArray(prim, "ai_instances", true, false);
/*		if (vs.size() == 0)
		{
			string err = toString("crash block need at least one ai instance !");
			throw EParseException(prim, err.c_str());
		}
*/
		for (uint i=0; i<vs.size(); ++i)
		{
			_AiInstances.push_back(vs[i]);
		}

		// look for an optional jump
		{
			TPrimitiveClassPredicate pred("jump_to");
			child = getPrimitiveChild(prim, pred);
		}
		if (child)
		{
			// ok, we have a jump at end of fail step
			_JumpTo = md.getProperty(child, "target", true, false);
		}
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "crash";

		if (!_AiInstances.empty())
		{
			ret += " :";
			for (uint i=0; i<_AiInstances.size(); ++i)
			{
				ret += string(" ")+_AiInstances[i];
			}
		}
		
		ret += NL;

		string tmp = IStep::genCode(md);
		tabulateLine(tmp, 1);
		ret += tmp;


		if (!_JumpTo.empty())
		{
			ret += "\tjump : "+_JumpTo + NL;
		}

		ret += "/crash"+NL;

		return ret;
	}

	void fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
	{
		IStep::fillJump(md, jumpPoints);
		if (!_JumpTo.empty())
			jumpPoints.insert(TJumpInfo(_JumpTo));
	}

};
REGISTER_STEP_INDIRECT(CStepCrash, "step_crash");

// ///////////////////// //
// STEP PLAYER RECONNECT //
// ///////////////////// //

// ***************************************************************************
CStepPlayerReconnect::CStepPlayerReconnect(CMissionData &md, IPrimitive *prim) : IStep(md, prim)
{
	// parse the sub prim to create action & objectives;
	IPrimitive *child;

	TPrimitiveSet resp;
	{
		TPrimitivePropertyPredicate pred("step_tag", "true");
		filterPrimitiveChilds(prim, pred, resp);
	}
	for (uint i=0; i<resp.size(); ++i)
	{
		_SubBranchs.push_back(resp[i]);
	}

	// look for an optional jump
	{
		TPrimitiveClassPredicate pred("jump_to");
		child = getPrimitiveChild(prim, pred);
	}
	if (child)
	{
		// ok, we have a jump at end of fail step
		_JumpTo = md.getProperty(child, "target", true, false);
	}
}

// ***************************************************************************
TPrimitiveSet CStepPlayerReconnect::getSubBranchs()
{
	return _SubBranchs;
}

// ***************************************************************************
string CStepPlayerReconnect::genCode(CMissionData &md)
{
	string ret;

	ret = "player_reconnect"+NL;

	string tmp = IStep::genCode(md);
	tabulateLine(tmp, 1);
	ret += tmp;

	for (uint i = 0; i < _SubSteps.size(); ++i)
	{
		if ( md.isThereAJumpTo(_SubSteps[i]->getStepName()) )
		{
			// insert a jump point
			ret += "jump_point : " + _SubSteps[i]->getStepName() + NL;
		}

		ret += _SubSteps[i]->genCode(md);
	}

	if (!_JumpTo.empty())
	{
		ret += "\tjump : "+_JumpTo + NL;
	}

	ret += "/player_reconnect"+NL;

	return ret;
}

// ***************************************************************************
void CStepPlayerReconnect::fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
{
	IStep::fillJump(md, jumpPoints);
	if (!_JumpTo.empty())
		jumpPoints.insert(TJumpInfo(_JumpTo));
}

REGISTER_STEP_INDIRECT(CStepPlayerReconnect, "step_player_reconnect");

// //////////////////////////// //
// END OF STEP PLAYER RECONNECT //
// //////////////////////////// //

// ***************************************************************************
// get the jump point of a node. NB: if the node is itself a jump, then directly jump to its target
static std::string *getJumpTarget(IPrimitive *child)
{
	if(!child)
		return NULL;

	// default: jump to the node
	string *s= NULL;
	child->getPropertyByName("name", s);

	// if the node is a jump itself
	string *className= NULL;
	child->getPropertyByName("class", className);
	if (className && *className == "jump_to")
	{
		// generate jump to the step jump target
		child->getPropertyByName("target", s);
	}
	
	return s;
}


// ***************************************************************************
class CStepDynChat : public IStep
{
public:
	CStepDynChat(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		_BotName = md.getProperty(prim, "npc_name", true, false);
		vector<string> temp = md.getPropertyArray(prim, "phrase", false, false);
		_Phrase.initPhrase(md, prim, temp);
		// talk_to menu
		temp = md.getPropertyArray(prim, "talk_to_menu", false, false);
//		_TalkToMenu.initPhrase(md, prim, temp);

		_TalkToObjective = NULL;
		std::auto_ptr<CStepDynChatTalkTo> talkToObjective; // next calls could throw exceptions, so take care...
		if (!temp.empty())
		{
			talkToObjective.reset(new CStepDynChatTalkTo(md, prim, "talk_to_"));
			talkToObjective->init(md, prim);
		}

		// build the sub branch list
		IPrimitive *noResp;
		{
			TPrimitiveClassPredicate pred("no_answer");
			noResp = getPrimitiveChild(prim, pred);
		}
		nlassert(noResp);
		_SubBranchs.push_back(noResp);
		
		TPrimitiveSet resp;
		{
			TPrimitiveClassPredicate pred("dyn_answer");
			filterPrimitiveChilds(prim, pred, resp);
		}
		_Responses.resize(resp.size());
		for (uint i=0; i<resp.size(); ++i)
		{
			_SubBranchs.push_back(resp[i]);
			if (resp[i]->getNumChildren() == 0)
				throw EParseException(prim, "missing step under 'dyn_answer' node for response");

			vector<string> temp = md.getPropertyArray(resp[i], "phrase_answer", false, false);
			_Responses[i].initPhrase(md, resp[i], temp);
		}
		_TalkToObjective = talkToObjective.release(); // commit result
	}
	~CStepDynChat()
	{
		delete _TalkToObjective;
	}
	
	TPrimitiveSet	getSubBranchs() 
	{
		TPrimitiveSet vStepsToReturn;
		for (uint i = 0; i < _SubBranchs.size(); ++i)
		{
			TPrimitiveSet childs;
			{
				TPrimitivePropertyPredicate pred("step_tag", "true");
				filterPrimitiveChilds(_SubBranchs[i], pred, childs);
			}
			for (uint j = 0; j < childs.size(); ++j)
				vStepsToReturn.push_back(childs[j]);
		}
		return vStepsToReturn;
	}
	
	void fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
	{
		for (uint i=0; i<_SubBranchs.size(); ++i)
		{
			string *s;
			IPrimitive *firstStep;
			if (!_SubBranchs[i]->getChild(firstStep, 0))
				throw EParseException(_Primitive, toString("Can't find child in %uth sub branch", i+1).c_str());

			bool ret = firstStep->getPropertyByName("name", s);
			if (!ret)
				throw EParseException(_Primitive, toString("Can't find property 'name' in %uth sub branch", i+1).c_str());

			string label;
			_SubBranchs[i]->getPropertyByName("name", label);
			jumpPoints.insert(TJumpInfo(*s, label));
		}
	}

	string genCode(CMissionData &md)
	{
		string ret;

		// if there's a talk_to menu, add it
//		if (!_TalkToMenu.isEmpty())
		if (_TalkToObjective != NULL)
		{			
			ret += _TalkToObjective->genCode(md);
//			ret += "talk_to : "+_BotName;	
//			ret += " : " + _TalkToMenu.genScript(md);
//			ret += NL;
		}


		ret += "dyn_chat : " + _BotName + " : " + _Phrase.genScript(md);

		for (uint i=1; i<_SubBranchs.size(); ++i)
		{
			IPrimitive *child;
			_SubBranchs[i]->getChild(child, 0);

			// get the jump target
			string *s= getJumpTarget(child);

			ret += " : " + *s + " " + _Responses[i-1].genScript(md);
		}

		ret += NL;

		return ret;
	}

	string genPhrase()
	{
		string ret; /* = CStepObjective::genPhrase();*/

//		if (!_TalkToMenu.isEmpty())
		if (_TalkToObjective != NULL)
		{			
			ret += _TalkToObjective->genPhrase();
//			ret += _TalkToMenu.genPhrase();
		}

		ret += _Phrase.genPhrase();

		for (uint i=0; i<_Responses.size(); ++i)
		{
			ret += _Responses[i].genPhrase();
		}
		return ret;
	}

	/// the list of sub branch for the dyn chat.
	TPrimitiveSet	_SubBranchs;
	// phrase for the main text
	CPhrase			_Phrase;
	// list of phrase for each response
	vector<CPhrase>	_Responses;

	string	_BotName;

	// 'talk_to' part
//	CPhrase			_TalkToMenu;
	CStepDynChatTalkTo	*_TalkToObjective;	
	
};
REGISTER_STEP_INDIRECT(CStepDynChat, "step_dyn_chat");

class CLinearDynChat : public IStep
{
	enum TLinearMode
	{
		mode_fail,
		mode_retry,
		mode_continue,
//		mode_skipable
	};

	TLinearMode	_Mode;
	string		_BotName;
	CPhrase		_Phrase;
	CPhrase		_PhraseYes;

	
public:


	CLinearDynChat(CMissionData &md, IPrimitive *prim)
		: IStep(md, prim)
	{
		_BotName = md.getProperty(prim, "npc_name", true, false);
		vector<string> temp = md.getPropertyArray(prim, "phrase", false, false);
		_Phrase.initPhrase(md, prim, temp);

		// set the linear mode
		string className;
		prim->getPropertyByName("class", className);

		if (className == "linear_dyn_chat_fail")
			_Mode = mode_fail;
		else if (className == "linear_dyn_chat_retry")
			_Mode = mode_retry;
		else if (className == "linear_dyn_chat_continue")
			_Mode = mode_continue;
		else
		{
			string err = toString("Unknow class '%s' for dyn chat !", className.c_str());
			throw EParseException(prim, err.c_str());
		}

		// read the response phrase
		vector<string> phraseYes;
		phraseYes = md.getPropertyArray(prim, "phrase_yes", false, false);
		_PhraseYes.initPhrase(md, prim, phraseYes);
	}
	
	string genCode(CMissionData &md)
	{
		string ret;

		switch (_Mode)
		{
		case mode_fail:
			ret = "dyn_chat : "+_BotName+" : "+_Phrase.genScript(md)+" : "+_StepName+"_resp "+_PhraseYes.genScript(md)+NL;
			ret += "fail"+NL;
			ret += "jump_point : " +_StepName+"_resp"+NL;
			break;
		case mode_retry:
			ret += "jump_point : " +_StepName+"_retry"+NL;
			ret += "dyn_chat : "+_BotName+" : "+_Phrase.genScript(md)+" : "+_StepName+"_resp "+_PhraseYes.genScript(md)+NL;
			ret += "jump : "+_StepName+"_retry"+NL;
			ret += "jump_point : " +_StepName+"_resp"+NL;
			break;
		case mode_continue:
			ret = "dyn_chat : "+_BotName+" : "+_Phrase.genScript(md)+" : "+_StepName+"_resp "+_PhraseYes.genScript(md)+NL;
			ret += "jump_point : " +_StepName+"_resp"+NL;
			break;
		}

		return ret;
	}

	string genPhrase()
	{
		string ret;
//		ret = CStepObjective::genPhrase();
		ret += _Phrase.genPhrase();
		ret += _PhraseYes.genPhrase();

		return ret;
	}

};
typedef CLinearDynChat TLinearDynChatFail;
REGISTER_STEP_INDIRECT(TLinearDynChatFail, "linear_dyn_chat_fail");
typedef CLinearDynChat TLinearDynChatRetry;
REGISTER_STEP_INDIRECT(TLinearDynChatRetry, "linear_dyn_chat_retry");
typedef CLinearDynChat TLinearDynChatContinue;
REGISTER_STEP_INDIRECT(TLinearDynChatContinue, "linear_dyn_chat_continue");

// /////// //
// STEP IF //
// /////// //

CStepIf::CStepIf(CMissionData &md, IPrimitive *prim) : IStep(md, prim)
{
	if (prim->getNumChildren() != 2)
		throw EParseException(prim, toString("step_if need two child primitive for 'not_ok' and 'ok' clause, found %u child", prim->getNumChildren()).c_str());

	IPrimitive *notOk = const_cast<IPrimitive *>(prim->getPrimitive("result_no"));
	//prim->getChild(notOk, 0);
	IPrimitive *ok = const_cast<IPrimitive *>(prim->getPrimitive("result_yes"));
	//prim->getChild(ok, 1);
	
	if (notOk == NULL)
		throw EParseException(prim, "Can't find 'not ok' step branch");
	if (ok == NULL)
		throw EParseException(prim, "Can't find 'ok' step branch");

	string name;
	notOk->getPropertyByName("class", name);
	if (name != "result_no")
		throw EParseException(prim, toString("first child must be of class 'result_no', found '%s' instead", name.c_str()).c_str());
	ok->getPropertyByName("class", name);
	if (name != "result_yes")
		throw EParseException(prim, toString("second child must be of class 'result_yes', found '%s' instead", name.c_str()).c_str());
	
	// push in order : not ok, then ok
	_SubBranchs.push_back(notOk);
	_SubBranchs.push_back(ok);


	string s = md.getProperty(prim, "condition_type", true, false);
	if (s == "test_missions_done")
		_IfType = it_mission_done;
	else if (s == "test_skills_level")
		_IfType = it_skills;
	else if (s == "test_bricks_known")
		_IfType = it_bricks;
	else if (s == "test_sdb")
		_IfType = it_sdb;
	else if (s == "test_race")
		_IfType = it_race;
	else if (s == "test_cult")
		_IfType = it_cult;
	else if (s == "test_civ")
		_IfType = it_civ;
	else if (s == "test_faction_point")
		_IfType = it_faction_point;
	else if (s == "test_guild_civ")
		_IfType = it_guild_civ;
	else if (s == "test_guild_cult")
		_IfType = it_guild_cult;
	else if (s == "test_guild_fame")
		_IfType = it_guild_fame;
	else if (s == "test_no_trial")
		_IfType = it_no_trial;
	else if (s == "test_item_in_inv")
		_IfType = it_item_in_inv;
	else
		throw EParseException(prim, "Unknow test type !");

	_IfParams = md.getPropertyArray(prim, "test_parameters", true, false);
}

TPrimitiveSet CStepIf::getSubBranchs() 
{
	TPrimitiveSet vStepsToReturn;
	for (uint i = 0; i < _SubBranchs.size(); ++i)
	{
		TPrimitiveSet childs;
		{
			TPrimitivePropertyPredicate pred("step_tag", "true");
			filterPrimitiveChilds(_SubBranchs[i], pred, childs);
		}
		for (uint j = 0; j < childs.size(); ++j)
			vStepsToReturn.push_back(childs[j]);
	}
	return vStepsToReturn;
}

void CStepIf::fillJump(CMissionData &md, set<TJumpInfo> &jumpPoints)
{
	/*string *s;
	TPrimitiveSet childs;
	bool ret;
	
	filterPrimitiveChilds(_SubBranchs[1], TPrimitivePropertyPredicate("step_tag", "true"), childs);
	if (!childs.empty())
	{
		ret = childs[0]->getPropertyByName("name", s);
		if (!ret)
			throw EParseException(childs[0], "Can't find property 'name' in first step of 'ok' sub branch");

		jumpPoints.insert(TJumpInfo(*s, "yes"));
	}

	// insert link to 'no' step
	childs.clear();
	filterPrimitiveChilds(_SubBranchs[0], TPrimitivePropertyPredicate("step_tag", "true"), childs);
	if (!childs.empty())
	{
		ret = childs[0]->getPropertyByName("name", s);
		if (!ret)
			throw EParseException(childs[0], "Can't find property 'name' in first step of 'not ok' sub branch");

		jumpPoints.insert(TJumpInfo(*s, "no"));
	}*/
}

string CStepIf::genCode(CMissionData &md)
{
	string ret;

	uint32 i;

	TPrimitiveSet childs;
	vector<IStep*> yesSteps;
	vector<IStep*> noSteps;

	// Get the 'yes branch' jump point
	{
		TPrimitivePropertyPredicate pred("step_tag", "true");
		filterPrimitiveChilds(_SubBranchs[1], pred, childs);
	}
	if (!childs.empty())
	{
		for (i = 0; i < _SubSteps.size(); ++i)
			if (std::find(childs.begin(), childs.end(), _SubSteps[i]->getPrimitive()) != childs.end())
				yesSteps.push_back(_SubSteps[i]);

		if (yesSteps.empty())
		{
			string err = toString("In step '%s', can't find sub step in 'yes' sub branch", getStepName().c_str());
			throw EParseException(_SubBranchs[1]->getParent(), err.c_str());
		}
	}

	// Get the 'no branch' jump point
	childs.clear();
	{
		TPrimitivePropertyPredicate pred("step_tag", "true");
		filterPrimitiveChilds(_SubBranchs[0], pred, childs);
	}
	if (!childs.empty())
	{
		for (i = 0; i < _SubSteps.size(); ++i)
			if (std::find(childs.begin(), childs.end(), _SubSteps[i]->getPrimitive()) != childs.end())
				noSteps.push_back(_SubSteps[i]);

		if (noSteps.empty())
		{
			string err = toString("In step '%s', can't find sub step in 'no' sub branch", getStepName().c_str());
			throw EParseException(_SubBranchs[0]->getParent(), err.c_str());
		}
	}

	if ((yesSteps.empty()) && (noSteps.empty()))
	{
		string err = toString("In step '%s', no yes-branch and no no-branch", getStepName().c_str());
		throw EParseException(_SubBranchs[0]->getParent(), err.c_str());
	}

	string jumpToYes;
	if (!yesSteps.empty())
		jumpToYes = yesSteps[0]->getStepName();
	else
		jumpToYes = getStepName() + "_endif";


	// type of IF:
	switch(_IfType)
	{
	case it_mission_done:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_mission_done need at least one mission to test");
		for (uint i=0; i<_IfParams.size(); ++i)
			ret += "if_mission_done : " + _IfParams[i] + " : " + jumpToYes + NL;
		break;
	case it_skills:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_skills need at least one <skill level> to test");
		ret = "if_skills : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += "; ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_bricks:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_bricks need at least one sbrick to test");
		ret = "if_bricks : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += "; ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_sdb:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_sdb need an expression to test");
		ret = "if_sdb : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_race:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_race need an expression to test");
		ret = "if_race : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_cult:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_cult need an expression to test");
		ret = "if_cult : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_civ:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_civ need an expression to test");
		ret = "if_civ : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_faction_point:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_faction_point need an expression to test");
		ret = "if_faction_point : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_guild_civ:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "it_guild_civ need an expression to test");
		ret = "if_guild_civ : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_guild_cult:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "it_guild_cult need an expression to test");
		ret = "if_guild_cult : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_guild_fame:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "it_guild_fame need an expression to test");
		ret = "if_guild_fame : ";
		for (uint i=0; i<_IfParams.size(); ++i)
		{
			ret += _IfParams[i];
			if (i < _IfParams.size()-1)
				ret += " ";
		}
		ret += " : " + jumpToYes + NL;
		break;
	case it_no_trial:
		ret = "if_no_trial : " + jumpToYes + NL;
		break;
	case it_item_in_inv:
		if (_IfParams.empty())
			throw EParseException(_Primitive, "if_item_in_inv need at least one mission to test");
		for (uint i=0; i<_IfParams.size(); ++i)
			ret += "if_item_in_inv : " + _IfParams[i] + " : " + jumpToYes + NL;
		break;
	}

	// Generate the 'no branch'

	if (!noSteps.empty())
	{
		ret += "# no: " + noSteps[0]->getStepName() + NL;
		for (uint i = 0; i < noSteps.size(); ++i)
		{
			if ( md.isThereAJumpTo(noSteps[i]->getStepName()) )
			{
				// insert a jump point
				ret += "jump_point : " + noSteps[i]->getStepName() + NL;
			}

			ret += noSteps[i]->genCode(md);
		}
	}

	// Generate the 'yes branch'

	if (!yesSteps.empty())
	{
		ret += "jump : " + getStepName() + "_endif" + NL;
		ret += "# yes: " + yesSteps[0]->getStepName() + NL;

		for (uint i = 0; i < yesSteps.size(); ++i)
		{
			if ((i == 0) || md.isThereAJumpTo(yesSteps[i]->getStepName()) )
			{
				// insert a jump point
				ret += "jump_point : " + yesSteps[i]->getStepName() + NL;
			}

			ret += yesSteps[i]->genCode(md);
		}
	}

	// Generate ending jump point

	ret += "jump_point : " + getStepName() + "_endif" + NL;

	return ret;
}

REGISTER_STEP_INDIRECT(CStepIf, "step_if");

// ////////////// //
// END OF STEP IF //
// ////////////// //

class CStepOOO : public CStepObjective
{
public:
	CStepOOO(CMissionData &md, IPrimitive *prim)
		: CStepObjective(md, prim)
	{
	}


	string genCode(CMissionData &md)
	{
		string ret;

		ret += CStepObjective::genCode(md);

		ret += IStep::genCodePreActions(md);

		ret += "ooo" + NL;

		string tmp = IStep::genCodeObjectives(md);
		tabulateLine(tmp, 1);
		ret += tmp;

		ret += "/ooo"+NL;

		ret += genCodePostActions(md);

		return ret;
	}

};
REGISTER_STEP_INDIRECT(CStepOOO, "step_ooo");

class CStepAny : public CStepObjective
{
public:
	CStepAny(CMissionData &md, IPrimitive *prim)
		: CStepObjective(md, prim)
	{
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret += CStepObjective::genCode(md);

		ret += IStep::genCodePreActions(md);

		ret += "any" + NL;

		string tmp = IStep::genCodeObjectives(md);
		tabulateLine(tmp, 1);

		ret += tmp;

		ret += "/any"+NL;

		ret += genCodePostActions(md);

		return ret;
	}

};
REGISTER_STEP_INDIRECT(CStepAny, "step_any");

