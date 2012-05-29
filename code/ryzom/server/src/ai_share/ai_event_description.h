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




#ifndef RYAI_AI_EVENT_ACTION_NODE_H
#define RYAI_AI_EVENT_ACTION_NODE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/smart_ptr.h"
#include "nel/net/message.h"

#include "ai_types.h"

#include <string>
#include <vector>

#include "ai_vector.h"

class	CTmpPropertyZone : public NLMISC::CRefCount
{
public:	
	enum TTarget { All, Fauna, Npc };
	CTmpPropertyZone()
		:verticalPos(AITYPES::vp_auto),
		 Target(All)
	{}
	typedef NLMISC::CSmartPtr<CTmpPropertyZone> TSmartPtr;

	AITYPES::TVerticalPos	verticalPos;
	std::vector <CAIVector> points;
	AITYPES::CPropertySet	properties;	
	TTarget Target;
};

class CAIEventActionNode : public NLMISC::CRefCount
{
public:
	typedef NLMISC::CSmartPtr<CAIEventActionNode> TSmartPtr;

	std::string Action;
	std::vector<std::string> Args;
	std::vector<CTmpPropertyZone::TSmartPtr>	_PropertyZones;
	std::vector<TSmartPtr> Children;
	uint32 Weight;
	uint32 Alias;

	void pushToPdr(CPersistentDataRecord& pdr) const
	{
		pdr.push(pdr.addString("type"),Action);

		if (Weight!=1) pdr.push(pdr.addString("weight"),Weight);
		if (Alias!=0) pdr.push(pdr.addString("alias"),Alias);

		for (uint32 i=0;i<Args.size();++i)
			pdr.push(pdr.addString("arg"),	Args[i]);

		// Note I haven't implemented property zones for now so make sure that we don't need them!
		nlassert(_PropertyZones.empty());

		for (uint32 i=0;i<Children.size();++i)
		{
			pdr.pushStructBegin(pdr.addString("child"));
			Children[i]->pushToPdr(pdr);
			pdr.pushStructEnd(pdr.addString("child"));
		}
	}
	static CAIEventActionNode* popFromPdr(CPersistentDataRecord& pdr)
	{
		CAIEventActionNode* result= new CAIEventActionNode;
		result->Weight= 1;
		result->Alias= 0;

		while (!pdr.isEndOfStruct())
		{
			uint16 token= pdr.peekNextToken();
			const std::string& tokenName= pdr.peekNextTokenName();
			if (tokenName=="type")		{ result->Action=			pdr.popNextArg(token).asString();	continue; }
			if (tokenName=="weight")	{ result->Weight= (uint32)	pdr.popNextArg(token).asUint();		continue; }
			if (tokenName=="alias")		{ result->Alias=  (uint32)	pdr.popNextArg(token).asUint();		continue; }
			if (tokenName=="arg")		{ result->Args.push_back(	pdr.popNextArg(token).asString() );	continue; }
			if (tokenName=="child")
			{
				pdr.popStructBegin(token);
				vectAppend(result->Children)= CAIEventActionNode::popFromPdr(pdr);
				pdr.popStructEnd(token);
				continue;
			}
			WARN("Unrecognised content found in pdr: "+tokenName);
			if (pdr.isStartOfStruct())
				pdr.skipStruct();
			else
				pdr.skipData();
		}

		return result;
	}
};


class CAIEventDescription : public NLMISC::CRefCount
{
public:
	typedef NLMISC::CSmartPtr<CAIEventDescription> TSmartPtr;

	std::string EventType;
	std::vector<std::string> StateKeywords;
	std::vector<std::string> NamedStates;
	std::vector<std::string> GroupKeywords;
	std::vector<std::string> NamedGroups;

	CAIEventActionNode::TSmartPtr Action;

	void pushToPdr(CPersistentDataRecord& pdr) const
	{
		pdr.push(pdr.addString("type"),EventType);
		
		for (uint32 i=0;i<StateKeywords.size();++i)
			pdr.push(pdr.addString("stateKeyword"),	StateKeywords[i]);

		for (uint32 i=0;i<NamedStates.size();++i)
			pdr.push(pdr.addString("state"),		NamedStates[i]);

		for (uint32 i=0;i<GroupKeywords.size();++i)
			pdr.push(pdr.addString("groupKeyword"),	GroupKeywords[i]);

		for (uint32 i=0;i<NamedGroups.size();++i)
			pdr.push(pdr.addString("group"),		NamedGroups[i]);

		if (Action!=NULL)
		{
			pdr.pushStructBegin(pdr.addString("action"));
			Action->pushToPdr(pdr);
			pdr.pushStructEnd(pdr.addString("action"));
		}
	}

	static CAIEventDescription* popFromPdr(CPersistentDataRecord& pdr)
	{
		CAIEventDescription* result= new CAIEventDescription;

		while (!pdr.isEndOfStruct())
		{
			uint16 token= pdr.peekNextToken();
			const std::string& tokenName= pdr.peekNextTokenName();
			if (tokenName=="type")			{ result->EventType=				pdr.popNextArg(token).asString();	continue; }
			if (tokenName=="stateKeyword")	{ result->StateKeywords.push_back(	pdr.popNextArg(token).asString() );	continue; }
			if (tokenName=="state")			{ result->NamedStates.push_back(	pdr.popNextArg(token).asString() );	continue; }
			if (tokenName=="groupKeyword")	{ result->GroupKeywords.push_back(	pdr.popNextArg(token).asString() );	continue; }
			if (tokenName=="group")			{ result->NamedGroups.push_back(	pdr.popNextArg(token).asString() );	continue; }

			if (tokenName=="action")
			{
				pdr.popStructBegin(token);
				result->Action= CAIEventActionNode::popFromPdr(pdr);
				pdr.popStructEnd(token);
				continue;
			}
			WARN("Unrecognised content found in pdr: "+tokenName);
			if (pdr.isStartOfStruct())
				pdr.skipStruct();
			else
				pdr.skipData();
		}
		return result;
	}

};


#endif
