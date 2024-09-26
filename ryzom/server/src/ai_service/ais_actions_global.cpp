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
#include "script_compiler.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;
using namespace	AITYPES;

extern CAIInstance *currentInstance;

//----------------------------------------------------------------------------
// Handy local variables and sub routines for GLOBAL context
//----------------------------------------------------------------------------

static	void	DoMgrAction(const std::vector <CAIActions::CArg> &args,
							enum TMgrType type,
							TContext context, 
							uint32 aIInstanceNumber=0)
{
//	if (!CAIS::initialised())
//		CAIS::initAI();

/*	CAIInstance	*aiInstance=NULL;
	
	if (aIInstanceNumber<CAIS::instance().AIList().size())
	{
		aiInstance=CAIS::instance().AIList()[aIInstanceNumber];
	}
	
	if	(!aiInstance)
	{
		aiInstance=CAIS::instance().AIList().addChild(new CAIInstance(*((CAIS*)NULL)),	aIInstanceNumber);	//	Arggghhh !!!
		if (!aiInstance)
		{
			return;
		}

	}
*/
	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
	CAIInstance	*aiInstance=currentInstance ;
	CWorkPtr::aiInstance(aiInstance);	// set the current AIInstance.

	// get hold of the manager's slot id - note that managers are identified by slot and not by alias!
	uint32 alias;	//,firstSlot,lastSlot;
	std::string name,mapName, filename;
	if	(!getArgs(args,"MANAGER",alias,name,mapName, filename))
		return;

	// see whether the manager is already loaded
	CManager*	mgr	=	aiInstance->managers().getChildByAlias(alias);
	
	// not found so look for a free slot
	if	(!mgr)
		aiInstance->newMgr(type, alias, name, mapName, filename);
	else
		mgr->registerForFile(filename);		

	mgr=aiInstance->managers().getChildByAlias(alias);

	// clear the delete flag (if present)
//	mgr->clearDeleteFlag();


	// setup the working manager pointer and exit
	CWorkPtr::mgr(mgr);
	if	(mgr)
		CWorkPtr::eventReactionContainer(mgr->getStateMachine());
	else
		CWorkPtr::eventReactionContainer(NULL);	
	
	// push the manager context onto the context stack
	CContextStack::setContext(context);
}


//----------------------------------------------------------------------------
// The GLOBAL context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextGlobal,MGRFAUNA)
{
	DoMgrAction(args,MgrTypeFauna,ContextFaunaMgr);
}

DEFINE_ACTION(ContextGlobal,MGRNPC)
{
	DoMgrAction(args,MgrTypeNpc,ContextNpcMgr);
}

//DEFINE_ACTION(ContextGlobal,MGRKAMI)
//{
//	DoMgrAction(args,MgrTypeKami,ContextNpcMgrKami);
//}

//DEFINE_ACTION(ContextGlobal,MGRTRIBE)
//{
//	DoMgrAction(args,MgrTypeTribe,ContextNpcMgrTribe);
//}

DEFINE_ACTION(ContextGlobal,MGRKARAV)
{
	DoMgrAction(args,MgrTypeKaravan,ContextNpcMgrKaravan);
}

DEFINE_ACTION(ContextGlobal,SCRIPT)
{
	// get hold of the parameters and check their validity
	string name;
	std::string code;
	if (!getArgs(args,"",name,code))
		return;
	
	AIVM::CLibrary::getInstance().addLib(name, code);
}

//----------------------------------------------------------------------------
// The base context for MGR contexts
//----------------------------------------------------------------------------

DEFINE_ACTION(BaseContextMgr,IDTREE)
{
	// set the id tree for the manager (results in creation or update of manager's object tree)
	// args: aliasTree

	if (!CWorkPtr::mgr())
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args,name(),aliasTree))
		return;

	// have the manager update it's structure from the id tree
	nlinfo("ACTION IDTREE: Applying new tree to manager[%u]: '%s'%s",
		CWorkPtr::mgr()->getChildIndex(),
		CWorkPtr::mgr()->getAliasTreeOwner()->getName().c_str(),
		CWorkPtr::mgr()->getAliasTreeOwner()->getAliasString().c_str());
	
	if (aliasTree)
		CWorkPtr::mgr()->updateAliasTree(*aliasTree);
}

DEFINE_ACTION(BaseContextMgr,BOUNDS)
{
	// set the bounding patat  for a manager
	// args: [x0, y0 [, x1, y1 [, x2,y2 [,...]]]]

	if (CWorkPtr::mgr()==NULL) return;

	// make sure argument count is even
	if (args.size()&1)
	{
		nlwarning("BOUNDS failed due to invalid arguments");
		return;
	}

	// build a vector of CAIVectot to hold points
	std::vector<CAIVector> points;
	for (uint i=0;i<args.size();i+=2)
	{
		double x,y;
		if (!(args[i].get(x)&&args[i+1].get(y)))
		{
			nlwarning("BOUNDS failed due to invalid arguments");
			return;
		}
		points.push_back(CAIVector(x,y));
	}

	// pass vector of points to the currently active manger
//	CWorkPtr::mgr()->setBounds(points);
}

//DEFINE_ACTION(BaseContextMgr,NOGO)
//{
//	// add a no-go zone to the manager's bounding patat
//	// args: [x0, y0 [, x1, y1 [, x2,y2 [,...]]]]
//
//	if (CWorkPtr::mgr()==NULL) return;
//
//	// make sure argument count is even	(plus alias)
//	uint32 alias;
//	if (args.size()<3 || !args[0].get(alias) || (args.size()&1)==0)
//	{
//		nlwarning("NOGO failed due to invalid arguments");
//		return;
//	}
//
//	// build a vector of CAIVectot to hold points
//	std::vector<CAIVector> points;
//	for (uint i=0;i<args.size();i+=2)
//	{
//		double x,y;
//		if (!(args[i].get(x)&&args[i+1].get(y)))
//		{
//			nlwarning("BOUNDS failed due to invalid arguments");
//			return;
//		}
//		points.push_back(CAIVector(x,y));
//	}
//
//	// pass vector of points to the currently active manger
//	//	CWorkPtr::mgr()->addNogo(alias,points);
//}

