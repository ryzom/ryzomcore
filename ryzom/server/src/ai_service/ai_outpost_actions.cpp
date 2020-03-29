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
#include "ai_outpost.h"
#include "continent.h"
#include "continent_inline.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;


extern CAIInstance *currentInstance;

DEFINE_ACTION(ContextGlobal,SQD_TMPL)
{
	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
	CWorkPtr::aiInstance(currentInstance);
	CAIInstance *aii = CWorkPtr::aiInstance();
	
	CAIAliasDescriptionNode *aliasTree;
	string filename;
	if	(!getArgs(args, name(), aliasTree, filename))
		return;

	CContextStack::setContext(ContextSquadTemplate);
}

DEFINE_ACTION(ContextSquadTemplate,SQD_T_V)
{
	string templateName, variantTag;
	if ( !getArgs(args, name(), templateName, variantTag))
		return;

	CAIInstance *aii = CWorkPtr::aiInstance();
	CWorkPtr::squadVariantName(templateName + ":" + variantTag);

	CContextStack::setContext(ContextSquadTemplateVariant);
}

DEFINE_ACTION(ContextOutpostGroupDesc,IDTREE_F)
{
	// IDTREE "flat": instead of browsing nodes, we build tree from the argument vector

	if (!CWorkPtr::groupDesc())
		return;

	// read the bot sheet vector from the argument list
	vector<string> botSheets( args.size() );
	for ( uint i=0; i!=args.size(); ++i )
	{
		args[i].get( botSheets[i] );

		// Remove .creature suffix if present
		string::size_type p = botSheets[i].find( ".creature" );
		if ( p != string::npos )
			botSheets[i].erase( p );
	}
	if ( botSheets.empty() )
	{
		OUTPOST_WRN( "No bot sheets specified in %s => no squad", CWorkPtr::groupDesc()->getFullName().c_str() );
		return;
	}

	// build the bot nodes
	CAliasTreeOwner *owner = dynamic_cast<CAliasTreeOwner *>(CWorkPtr::groupDesc());
	if ( ! owner )
	{
		OUTPOST_WRN( "Invalid squad group desc %s", CWorkPtr::groupDesc()->getFullName().c_str() );
		return;
	}

	owner->pushCurrentOwnerList();
	for ( uint j=0; j!=botSheets.size(); ++j )
	{
		CAliasTreeOwner* childOwner = NULL;
		IAliasCont* cont = NULL;
			
		// we try to get a valid IAliasCont for this type
		if ( ! owner->getCont( childOwner, cont, AITYPES::AITypeSquadTemplateMember ) )
			break;
		
		// create a bot desc object (with alias 0)
		CAliasTreeOwner* child = NULL;
		CSmartPtr<CAIAliasDescriptionNode> fakeNode = new CAIAliasDescriptionNode( /*NLMISC::toString( "tm%u", j )*/botSheets[j], 0, AITYPES::AITypeSquadTemplateMember, NULL );
		child = childOwner->createChild( cont, fakeNode );
		if ( ! child )
		{
			OUTPOST_WRN( "Can't create child %s of squad template variant %s", botSheets[j].c_str(), owner->getName().c_str() );
		}
		else
		{
			CBotDesc<COutpostSquadFamily>* botDesc = dynamic_cast< CBotDesc<COutpostSquadFamily>* >(child);
			nlassert( botDesc );
			botDesc->setSheet( botSheets[j] );
			botDesc->setUseSheetBotName(true);
		}
	}
	owner->popCurrentOwnerList();
}

DEFINE_ACTION(ContextGlobal,OUTPOST)
{
	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
	CWorkPtr::aiInstance(currentInstance);
	CAIInstance *aii = CWorkPtr::aiInstance();
	
	CAIAliasDescriptionNode *aliasTree;
	string contName, mapName, filename, familyName;
	if	(!getArgs(args, name(), aliasTree, contName, filename, familyName))
		return;
	
	if (!CWorkPtr::continent())
		return;
		
	// see whether the region is already created
	COutpost *outpost = CWorkPtr::continent()->outposts().getChildByAlias(aliasTree->getAlias());
	
	// not found so create it
	if (!outpost)
	{
		outpost = new COutpost(CWorkPtr::continent(), aliasTree->getAlias(), aliasTree->getName(), filename);
		
		CWorkPtr::continent()->outposts().addAliasChild(outpost);
		CWorkPtr::outpost(outpost);
	}
	else
	{
		outpost->registerForFile(filename);
	}
	
	// set the owner tribe
	CWorkPtr::outpost()->setTribe(familyName);
	
	CContextStack::setContext(ContextOutpost);

}

DEFINE_ACTION(ContextOutpost,OUTP_SQD)
{
	CAIInstance *aii = CWorkPtr::aiInstance();
	COutpost *outpost = CWorkPtr::outpost();
	if (!outpost)
		return;

	set<string> squads;
	string variantTag;

	// read the variant tag (first element of vector)
	nlassert( ! args.empty() );
	args[0].get( variantTag );

	// read the squads (skip first element of vector; avoid duplicates)
	for ( uint i=1; i!=args.size(); ++i )
	{
		string s;
		args[i].get( s );
		squads.insert( s );
	}

	// link outpost to squads
	for ( set<string>::const_iterator its=squads.begin(); its!=squads.end(); ++its )
	{
		string variantName = (*its);
		if ( variantName.find( ':' ) == string::npos ) // the leveldesigner can specify which variant he wants
			variantName += ":" + variantTag;
		CGroupDesc<COutpostSquadFamily> *groupDesc = aii->getSquadByVariantName( variantName );
		if ( groupDesc )
			outpost->addSquadLink( groupDesc->getAlias(), groupDesc );
		else
			OUTPOST_WRN( "Can't find squad %s for outpost %s", variantName.c_str(), outpost->getName().c_str() );
	}
}

DEFINE_ACTION(ContextOutpost,IDTREE)
{
	// set the id tree for the region (results in creation or update of region's object tree)
	// args: aliasTree

	if (!CWorkPtr::outpost())
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args, name(),aliasTree))
		return;

		
	// have the manager update it's structure from the id tree
	nlinfo("ACTION IDTREE: Applying new tree to outpost[%u]: '%s'%s in continent '%s'", 
		CWorkPtr::outpost()->getChildIndex(), 
		CWorkPtr::outpost()->getName().c_str(),
		CWorkPtr::outpost()->getAliasString().c_str(), 
		CWorkPtr::outpost()->getOwner()->getName().c_str()
		);
	
	if (aliasTree && CWorkPtr::outpost())
		CWorkPtr::outpost()->updateAliasTree(*aliasTree);
}

DEFINE_ACTION(ContextOutpost,OUTPOGEO)
{
	COutpost* outpost = CWorkPtr::outpost();
	if (!outpost)
		return;
	
	std::vector <CAIVector> points;
	for (uint i=0; i<(args.size()-1); i+=2)
	{
		double x, y;
		args[i].get(x);
		args[i+1].get(y);
		points.push_back(CAIVector(x, y));
	}
	NLMISC::CSmartPtr<CAIPlaceOutpost> shape = NLMISC::CSmartPtr<CAIPlaceOutpost>(new CAIPlaceOutpost(outpost));
	shape->setPatat(AITYPES::vp_auto, points);
	shape->setOutpostAlias(outpost->getAlias());
	outpost->setShape(shape);
}

DEFINE_ACTION(ContextOutpost,SPWNZONE)
{
	COutpost* outpost = CWorkPtr::outpost();
	if (!outpost)
		return;
	
	float x, y, r;
	uint32 verticalPos;
	// read the alias tree from the argument list
	CAIAliasDescriptionNode* aliasTree;
	if (!getArgs(args, name(), aliasTree, x, y, r, verticalPos))
		return;
	
	// see whether the region is already loaded
	COutpostSpawnZone* spawnZone = outpost->spawnZones().getChildByAlias(aliasTree->getAlias());
	if (!spawnZone)
		return;
	
	spawnZone->setPosAndRadius((AITYPES::TVerticalPos)verticalPos, CAIPos(x, y, 0, 0.f), uint32(r*1000));
}

DEFINE_ACTION(ContextOutpost,BUILDING)
{
/****************************************************************************/
	/*
	COutpost* outpost = CWorkPtr::outpost();
	if (!outpost)
		return;
	
	float x, y, theta;
	uint32 verticalPos;
	// read the alias tree from the argument list
	CAIAliasDescriptionNode* aliasTree;
	if (!getArgs(args, name(), aliasTree, x, y, theta, verticalPos))
		return;
	
	// see whether the region is already loaded
	COutpostSpawnZone* spawnZone = outpost->spawnZones().getChildByAlias(aliasTree->getAlias());
	if (!spawnZone)
		return;
	
	spawnZone->setPosAndRadius((AITYPES::TVerticalPos)verticalPos, CAIPos(x, y, 0, 0.f), uint32(r*1000));
	*/
/****************************************************************************/
	COutpost* outpost = CWorkPtr::outpost();
	if (!outpost)
		return;
	CGroup* grp = outpost->getBuildingGroup();
	if (!grp) 
		return;
	
	uint32 alias;
	if (!getArgs(args, name(), alias))
		return;
	
//	LOG("Outpost Building: group: %s, bot: %u", grp->getFullName().c_str(), alias);
	
	// set workptr::bot to this bot
	CWorkPtr::bot(grp->bots().getChildByAlias(alias));	//lookupBotInGrpNpc(alias));
	if (!CWorkPtr::botNpc())
	{
		nlwarning("Failed to select bot %s as not found in group: %s",
			LigoConfig.aliasToString(alias).c_str(),
			CWorkPtr::grpNpc()->getName().c_str());
		return;
	}
	/*
	if (!(CWorkPtr::botNpc()->getChat().isNull()))
	{
		CWorkPtr::botNpc()->getChat()->clearMissions();
	}
	*/
	// set workptr state to this state
	CContextStack::setContext(ContextNpcBot);
}
/*
DEFINE_ACTION(ContextOutpost,PLACE)
{
	COutpost *outpost = CWorkPtr::outpost();
	if (!outpost)
		return;

	float x,y;

	if (!getArgs(args,name(), x, y))
		return;

	outpost->setPosition(CAIVector(x, y));
}
*/
/*
DEFINE_ACTION(ContextOutpost,CHARGE)
{
	COutpost *outpost = CWorkPtr::outpost();
	if (!outpost)
		return;

	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	string	civilisation;
	if (!getArgs(args, name(), aliasTree, civilisation))
		return;

	// see whether the outpost charge is already loaded
	COutpostCharge *charge = outpost->charges().getChildByAlias(aliasTree->getAlias());
	if (!charge)
		return;

	charge->setCivilisation(civilisation);

	CWorkPtr::outpostCharge(charge);
	CContextStack::setContext(ContextOutpostCharge);
}
*/
/*
DEFINE_ACTION(ContextOutpost,CHGPARM)
{
	COutpostCharge *charge = CWorkPtr::outpostCharge();
	if (!charge)
		return;

	vector<string>	params;
	for (uint i=0; args.size(); ++i)
	{
		string s;
		args[i].get(s);
		params.push_back(s);
	}

	charge->setParams(params);
}
*/
/*DEFINE_ACTION(ContextOutpost,SQUADFAM)
{
	COutpost *outpost = CWorkPtr::outpost();
	if (!outpost)
		return;
	
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args, name(), aliasTree))
		return;
	
	COutpostSquadFamily* squadFamily = outpost->squadFamilies().getChildByAlias(aliasTree->getAlias());
	if (!squadFamily)
		return;
	
	CWorkPtr::outpostSquadFamily(squadFamily);
	CContextStack::setContext(ContextOutpostSquadFamily);
}*/

static void DoMgrAction(
	std::vector <CAIActions::CArg> const& args,
	AITYPES::TMgrType type,
	CAISActionEnums::TContext context)
{
//	nlassertex(currentInstance != NULL, ("No AIInstance created !"));
//	CAIInstance* aiInstance = currentInstance;
//	CWorkPtr::aiInstance(aiInstance);	// set the current AIInstance.
	COutpost* outpost = CWorkPtr::outpost();
	
	// get hold of the manager's slot id - note that managers are identified by slot and not by alias!
	uint32 alias;
	std::string name, mapName, filename;
	bool manualSpawn = true;
	if (type==AITYPES::MgrTypeOutpost)
	{
		if (!getArgs(args, "MANAGER", alias, name, mapName, filename, manualSpawn))
			return;
	}
	else
	{
		if (!getArgs(args, "MANAGER", alias, name, mapName, filename))
			return;
	}
	
	// see whether the manager is already loaded
	COutpostManager* mgr = static_cast<COutpostManager*>(outpost->managers().getChildByAlias(alias));
	
	// not found so look for a free slot
	nlassert(mgr);
//	outpost->newMgr(type, alias, name, mapName, filename);
	mgr->registerForFile(filename);
	mgr->setAutoSpawn(!manualSpawn);
	
	mgr = outpost->managers().getChildByAlias(alias);
	
	// setup the working manager pointer and exit
	CWorkPtr::mgr(mgr);
	if (mgr)
		CWorkPtr::eventReactionContainer(mgr->getStateMachine());
	else
		CWorkPtr::eventReactionContainer(NULL);	
	
	// push the manager context onto the context stack
	CContextStack::setContext(context);
}


DEFINE_ACTION(ContextOutpost,MGRNPC)
{
	DoMgrAction(args, AITYPES::MgrTypeNpc, CAISActionEnums::ContextNpcMgr);
}

DEFINE_ACTION(ContextOutpost,MGROUTPO)
{
	DoMgrAction(args, AITYPES::MgrTypeOutpost, CAISActionEnums::ContextNpcMgr);
}

/*
DEFINE_ACTION(ContextOutpostSquadFamily,IDTREE)
{
	// set the id tree for the region (results in creation or update of region's object tree)
	// args: aliasTree
	
	if (!CWorkPtr::outpostSquadFamily())
		return;
	
	// read the alias tree from the argument list
	CAIAliasDescriptionNode *aliasTree;
	if (!getArgs(args, name(),aliasTree))
		return;
	
	
	// have the manager update it's structure from the id tree
	nlinfo("ACTION IDTREE: Applying new tree to outpost[%u]: '%s'%s in continent '%s'", 
		CWorkPtr::outpostSquadFamily()->getChildIndex(), 
		CWorkPtr::outpostSquadFamily()->getName().c_str(),
		CWorkPtr::outpostSquadFamily()->getAliasString().c_str(), 
		CWorkPtr::outpostSquadFamily()->getOwner()->getName().c_str()
		);
	
	if (aliasTree && CWorkPtr::outpostSquadFamily())
		CWorkPtr::outpostSquadFamily()->updateAliasTree(*aliasTree);
}
*/
DEFINE_ACTION(ContextSquadTemplateVariant,GRPTMPL)
{
	CAIInstance *aii = CWorkPtr::aiInstance();
	COutpostSquadFamily* const squadFamily = aii->getSquadFamily();
	if (!squadFamily)
		return;
	
	string grpFamily; // Ignored
	uint32 botCount;
	bool countMultipliedBySheet;
	bool multiLevel;
	
	// read the alias tree from the argument list
	CAIAliasDescriptionNode* aliasTree;
	if (!getArgs(args, name(), aliasTree, grpFamily, botCount, countMultipliedBySheet, multiLevel))
		return;
	
	IAliasCont *aliasCont = squadFamily->getAliasCont( AITYPES::AITypeGroupTemplate );
	CAliasTreeOwner *child = squadFamily->createChild( aliasCont, aliasTree );
	CGroupDesc<COutpostSquadFamily> *groupDesc = (dynamic_cast<CGroupDesc<COutpostSquadFamily>*>(child));
	if (!groupDesc)
		return;

	aii->registerSquadVariant( CWorkPtr::squadVariantName(), groupDesc );
	CWorkPtr::groupDesc( groupDesc );
	
	groupDesc->setBaseBotCount(botCount);
	groupDesc->setCountMultiplierFlag(countMultipliedBySheet);
	groupDesc->setMultiLevel(multiLevel);
	
	CContextStack::setContext(ContextOutpostGroupDesc);
}

//////////////////////////////////////////////////////////////////////////////
// ContextGroupDesc actions instances                                       //
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// ContextGroupDesc actions                                                 //
//////////////////////////////////////////////////////////////////////////////

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_SHEE,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	string lookSheet;
	
	if (!getArgs(args,name(), lookSheet))
		return;
	
	if (!groupDesc->setSheet(lookSheet))
	{
		groupDesc->getOwner()->groupDescs().removeChildByIndex(groupDesc->getChildIndex());
		CWorkPtr::groupDesc(NULL);
		return;
	}
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_LVLD,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	sint32 levelDelta;
	
	if (!getArgs(args,name(), levelDelta))
		return;
	
	groupDesc->setLevelDelta(levelDelta);
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_SEAS,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	bool seasons[4];
	
	if (!getArgs(args,name(), seasons[0], seasons[1], seasons[2], seasons[3]))
		return;
	
	groupDesc->setSeasonFlags(seasons);
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_ACT,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 spawnType;
	if (!getArgs(args, name(), spawnType))
		return;
	
	groupDesc->setSpawnType((AITYPES::TSpawnType)spawnType);
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_APRM,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;	
	
	for (size_t i=0; i<args.size(); ++i)
	{
		string property;
		args[i].get(property);
		groupDesc->properties().addProperty(AITYPES::CPropertyId::create(property));
	}
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_NRG,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 weight[4];
	
	if (!getArgs(args,name(), weight[0], weight[1], weight[2], weight[3]))
		return;
	
	groupDesc->setWeightLevels(weight);
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_EQUI,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	groupDesc->botEquipment().clear();
	
	for (size_t i=0; i<args.size(); ++i)
	{
		string equip;
		args[i].get(equip);
		groupDesc->botEquipment().push_back(equip);
	}
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_GPRM,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	for (size_t i=0; i<args.size(); ++i)
	{
		string param;
		args[i].get(param);
		
		param = NLMISC::toLower(param);
		
		if	(	param == "contact camp"
			||	param == "contact outpost"
			||	param == "contact city"
			||	param == "boss"	)
			groupDesc->properties().addProperty(param);
		else	// unreconized param, leace it for the group instance
			groupDesc->grpParameters().push_back(param);
	}
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,BOTTMPL,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	string lookSheet;
	bool multiLevel;
	
	// read the alias tree from the argument list
	CAIAliasDescriptionNode* aliasTree;
	if (!getArgs(args, name(), aliasTree, lookSheet, multiLevel))
		return;
	
	// see whether the region is already loaded
	CBotDesc<FamilyT>* botDesc = groupDesc->botDescs().getChildByAlias(aliasTree->getAlias());
	if (!botDesc)
		return;
	
	botDesc->setMultiLevel(multiLevel);
	botDesc->setSheet(lookSheet);
	
	CWorkPtr::botDesc(botDesc);
	CContextStack::setContext(ContextOutpostBotDesc);
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostBotDesc,BT_EQUI,FamilyT)
{
	CBotDesc<FamilyT>* botDesc = static_cast<CBotDesc<FamilyT>*>(CWorkPtr::botDesc());
	if (!botDesc)
		return;
	
	for (size_t i=0; i<args.size(); ++i)
	{
		string equip;
		args[i].get(equip);
		botDesc->equipement().push_back(equip);
	}
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostBotDesc,BT_LVLD,FamilyT)
{
	CBotDesc<FamilyT>* botDesc = static_cast<CBotDesc<FamilyT>*>(CWorkPtr::botDesc());
	if (!botDesc)
		return;
	
	sint32 levelDelta;
	
	if (!getArgs(args,name(), levelDelta))
		return;
	
	botDesc->setLevelDelta(levelDelta);
}


/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_GNRJ,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	uint32 energyValue;
	
	if (!getArgs(args,name(), energyValue))
		return;
}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,POPVER,FamilyT)
{
	// add a population version for a group
	// args: uint32 alias, string spawn_type, uint weight, (string sheet, uint32 count)+

	if(!CWorkPtr::groupDesc())
		return;
	
	const uint32 fixedArgsCount = 0;
	if (args.size()<fixedArgsCount+2 || ((args.size()-fixedArgsCount)&1)==1)
	{
		nlwarning("POPVER action FAILED due to bad number of arguments (%d)", args.size());
		return;
	}
	
	// get hold of the parameters and check their validity
	for (size_t i=fixedArgsCount; i+1<args.size(); i+=2)
	{
		std::string sheet;
		uint32 count;
		
		if	(	!args[i].get(sheet)
			||	!args[i+1].get(count))
		{
			nlwarning("POPVER Add Record FAILED due to bad arguments");
			continue;
		}
		
		CSheetId sheetId(sheet);
		if (sheetId==CSheetId::Unknown)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s", sheet.c_str());
			continue;
		}
		
		AISHEETS::ICreatureCPtr sheetPtr = AISHEETS::CSheets::getInstance()->lookup(sheetId);
		if (!sheetPtr)
		{
			nlwarning("POPVER Add Record Invalid sheet: %s", sheet.c_str());
			continue;
		}
		static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc())->populationRecords().push_back(CPopulationRecord(sheetPtr, count));
	}

}

/// :KLUDGE: This code is copied from continent_inline.h. Update both if you
/// make a modification.
// scales bot energy .. to match with group's one.
DEFINE_ACTION_TEMPLATE1(ContextOutpostGroupDesc,GT_END,FamilyT)
{
	CGroupDesc<FamilyT>* groupDesc = static_cast<CGroupDesc<FamilyT>*>(CWorkPtr::groupDesc());
	if (!groupDesc)
		return;
	
	if (!groupDesc->isMultiLevel())
	{
		uint32 totalEnergyValue = groupDesc->calcTotalEnergyValue();
		if (totalEnergyValue)
		{
			double coef = (double)groupDesc->groupEnergyValue()/(double)totalEnergyValue;
			groupDesc->setGroupEnergyCoef((float)coef);
		}
		else
		{
			nlwarning("exists some empty template groups");
		}
	}
}

// O for outpost
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_SHEE,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_LVLD,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_SEAS,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_ACT, O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_APRM,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_NRG, O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_EQUI,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_GPRM,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,BOTTMPL,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostBotDesc,BT_EQUI,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostBotDesc,BT_LVLD,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_GNRJ,O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,POPVER, O,COutpostSquadFamily);
DEFINE_ACTION_TEMPLATE1_INSTANCE(ContextOutpostGroupDesc,GT_END, O,COutpostSquadFamily);

