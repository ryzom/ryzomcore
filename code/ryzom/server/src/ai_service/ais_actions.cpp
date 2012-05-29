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




//----------------------------------------------------------------------------

#include "stdpch.h"
#include "ais_actions.h"
#include "ai_instance.h"

#include "nel/misc/hierarchical_timer.h"

/*
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include "ai_entity_id.h"
#include "ai_mgr.h"
#include "ai_grp.h"
#include "ai_mgr_fauna.h"
#include "ai_grp_fauna.h"

#include "ai_place_xyr.h"

#include <string>
#include <map>
*/
//#include "ais_actions.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;


//----------------------------------------------------------------------------
// Singleton data instantiation
//----------------------------------------------------------------------------

CAISActions *CAISActions::Instance=NULL;

std::vector<TContext> CContextStack::Stack;

CAIInstance	*CWorkPtr::Instance=NULL;
CManager	*CWorkPtr::Mgr=NULL;
CGroup		*CWorkPtr::Grp=NULL;
CBot		*CWorkPtr::Bot=NULL;

CAIState			*CWorkPtr::State=NULL;
CAIStateProfile		*CWorkPtr::Profile=NULL;
CAIStateChat		*CWorkPtr::Chat=NULL;

CStateMachine	*CWorkPtr::EventContainer=NULL;

CContinent			*CWorkPtr::Continent=NULL;
CRegion				*CWorkPtr::Region=NULL;
CCellZone			*CWorkPtr::CellZone=NULL;
CCell				*CWorkPtr::Cell=NULL;
CFaunaZone			*CWorkPtr::FaunaZone=NULL;
CNpcZone			*CWorkPtr::NpcZone=NULL;
CRoad				*CWorkPtr::Road=NULL;
CRoadTrigger		*CWorkPtr::RoadTrigger=NULL;
//CGroupDesc<CGroupFamily>	*CWorkPtr::GroupDesc=NULL;
//CBotDesc<CGroupFamily>		*CWorkPtr::BotDesc=NULL;

COutpost			*CWorkPtr::Outpost = NULL;
//COutpostCharge		*CWorkPtr::OutpostCharge = NULL;
std::string			CWorkPtr::SquadVariantName;
//CGroupDesc<COutpostSquadFamily>	*CWorkPtr::OutpostGroupDesc = NULL;
//CBotDesc<COutpostSquadFamily>	*CWorkPtr::OutpostBotDesc = NULL;
IGroupDesc			*CWorkPtr::GroupDesc=NULL;
IBotDesc			*CWorkPtr::BotDesc=NULL;

CGroupFamily		*CWorkPtr::GroupFamily = NULL;

IAILogicAction					  *CWorkPtr::_LogicAction = NULL;
std::map<uint32, IAILogicAction *> CWorkPtr::_LogicActionMap;


//----------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------

void CAISActions::begin(uint32 contextAlias)
{
	// always repush the previous state.
	// if the stack is empty, push a global context
	if (CContextStack::size() == 0)
	{
		CContextStack::push(ContextGlobal);
	}
	else
	{
		CContextStack::push(CContextStack::topContext());
	}
}


void CAISActions::end(uint32 contextAlias) 
{ 
	CContextStack::pop();
}


void CAISActions::execute(uint64 action,const std::vector <CAIActions::CArg> &args)
{
	H_AUTO(CAISActions_execute);
	for (uint i=CContextStack::size();i--;)
	{
		TContext context=CContextStack::context(i);
		nlassert(context<ContextNumContexts);
		TActionMap::iterator it=ActionMap[context].find(action);
		while	(	it==ActionMap[context].end()
				&&	parentContext(context)!=ContextNumContexts)
		{
			context=parentContext(context);
			it=ActionMap[context].find(action);
		}
		if (it!=ActionMap[context].end())
		{
			// found an action handler so execute it
			// if we've navigated back up the handler stack then pop bypassed entries
//			while (i<CContextStack::size()-1)
//				CContextStack::pop();
			H_AUTO(CAISActions_execute_delegate);
			(*((*it).second))(args);

			return;
		}
	}

	std::string txt;
	uint64 id=action;
	for (uint j=0;j<8 && ((char*)&id)[j]!=0;++j)
		txt+=((char*)&id)[j];
	nlwarning("Failed to execute action: %16"NL_I64"x: %s",action,txt.c_str());
}


void CAISActions::openFile(const std::string &fileName)
{
	// need to tag all root alias tree for delete
	const	string	f=CFile::getFilename(CFile::getFilenameWithoutExtension(fileName));
	CAIS::instance().markTagForDelete(f);
}

void CAISActions::closeFile(const std::string &fileName)
{
	// need to delete all root alias that are still tagged to delete
	const	string	f=CFile::getFilename(CFile::getFilenameWithoutExtension(fileName));
	CAIS::instance().deleteTaggedAlias(f);

	// need to relink tribes and outpost
	// check for outpost modification to update tribe outpost
	/*
	if (!ChangedOutposts.empty())
	{
		set<CContinent*>::iterator first(ChangedOutposts.begin()), last(ChangedOutposts.end());
		for (; first != last; ++first)
		{
			CContinent *cont = *first;
			cont->relinkOutpost();
		}
		
	}
	ChangedOutposts.clear();
	*/
}
