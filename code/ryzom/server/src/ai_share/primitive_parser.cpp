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

#include <errno.h>

#include "nel/misc/config_file.h"
#include "nel/misc/command.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/i_xml.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"
#include "nel/ligo/ligo_config.h"
#include "nel/net/service.h"
#include "ai_types.h"

#include "ai_actions.h"
#include "ai_actions_dr.h"
#include "ai_alias_description_node.h"
#include "ai_share.h"
#include "../server_share/primitive_cfg.h"
#include "../server_share/used_continent.h"

using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;
using namespace std;
using namespace AITYPES;


namespace AI_SHARE 
{

// debug
static bool s_WriteScenarioDebugDataToFile = false;

static	void	parsePrimGroupFamilyProfileFaunaContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim);
static	void	parsePrimGroupFamilyProfileTribeContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim);
static	void	parsePrimGroupFamilyProfileNpcContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim);

//---------------------------------------------------------------------------------------
// THIS LINE EXISTS TO MAKE SURE THE LINKER DOESN'T THROW OUT THIS MODULE AT LINK TIME!!!

bool LinkWithPrimitiveParser=false;

// The ligo config, if NULL, you don't have called AI_SHARE::init()
NLLIGO::CLigoConfig *LigoConfig = NULL;

//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo


//---------------------------------------------------------------------------------------
// some handy utilities	for extracting common fields from prims
//---------------------------------------------------------------------------------------

static std::string nodeName(const IPrimitive *prim)
{
	std::string result;
	prim->getPropertyByName("name",result);
	return result;
}

static TAIType nodeType(const IPrimitive *prim)
{
	std::string result;
	prim->getPropertyByName("ai_type",result);
	return	getType<TAIType>(result.c_str());
}

static TAITypeSpec nodeTypeSpec(const IPrimitive *prim)
{
	std::string result;
	prim->getPropertyByName("ai_type",result);
	return	getType<TAITypeSpec>(result.c_str());
}

static std::string nodeClass(const IPrimitive *prim)
{
	std::string result;
	prim->getPropertyByName("class",result);
	return result;
}

std::map <const IPrimitive *,uint32> MapPrimToAlias;
std::map <uint32,const IPrimitive *> MapAliasToPrim;

uint32 nodeAlias(const IPrimitive *prim, bool canFail = false)
{
	uint32 alias = 0;
	// see if we've already got an alias for this prim node
	if (MapPrimToAlias.find(prim)!=MapPrimToAlias.end())
	{
		alias=MapPrimToAlias[prim];
		return alias;
	}

	TPrimitiveClassPredicate pred("alias");
	IPrimitive *aliasNode = getPrimitiveChild(const_cast<IPrimitive*>(prim), pred);
	if (aliasNode)
	{
		CPrimAlias *pa = dynamic_cast<CPrimAlias*>(aliasNode);
		alias = pa->getFullAlias();
	}

	if (!canFail)
		nlassertex(alias != 0, ("in primitive '%s'", buildPrimPath(prim).c_str()));


//	std::string s;
//	prim->getPropertyByName("alias",s);

#ifdef NL_DEBUG
//	nlassert(!s.empty());
#endif

//	// for legacy reasons the field may be called 'unique_id' instead of 'alias'
//	if (s.empty())
//		prim->getPropertyByName("unique_id",s);
//	alias=NLMISC::fromString(s.c_str());

//	// if we haven't found a sensible alias value use the prim node address
//	if (alias==0 && s!="0")
//	{
//		alias=(sint32)prim;		
//		if(		nodeType(prim)!=AITypeBadType
//			&&	nodeType(prim)!=AITypeEventAction
//			&&	nodeType(prim)!=AITypeActionZone
//			&&	nodeType(prim)!=AITypeFaunaSpawnAtom)	//	legacy reasons .. again (bad).
//			nlwarning("Failed to find alias for prim node: '%s': '%s' (using generated alias: %u)",
//				buildPrimPath(prim).c_str(),
////				getName(nodeType(prim)),
//				nodeName(prim).c_str(),alias);
//	}

	// if we haven a valid alias, ask one to the container
	if (alias == 0)
	{
		CPrimitiveContext &ctx = CPrimitiveContext::instance();
		nlassert(ctx.CurrentPrimitive);
		alias = ctx.CurrentPrimitive->genAlias(const_cast<IPrimitive*>(prim), 0);
		alias = ctx.CurrentPrimitive->buildFullAlias(alias);
	}

	// make sure the alias is unique
	if (MapAliasToPrim.find(alias)!=MapAliasToPrim.end())
//		&&	nodeClass(prim) != "npc_group_parameters" )  // <= for legacy reason
	{
		nlassert(false);
//		uint32 oldAlias=alias;
//		while (MapAliasToPrim.find(alias)!=MapAliasToPrim.end())
//			++alias;
//#if !FINAL_VERSION
//		nlwarning("Alias %u not unique - remaping to %u",oldAlias,alias);
//#endif
	}

	// add alias to maps...
	MapAliasToPrim[alias]=prim;
	MapPrimToAlias[prim]=alias;

	return alias;
}

//---------------------------------------------------------------------------------------
// handy routine for reading vertical pos
//---------------------------------------------------------------------------------------

static bool parseVerticalPos(const IPrimitive *prim, uint32 &verticalPos, const char *propertyName = "vertical_pos")
{
	string s;

	if (prim->getPropertyByName(propertyName, s))
	{
		verticalPos = verticalPosFromString(s);
		return true;
	}
	else
	{
		verticalPos = vp_auto;
		return false;
	}
}

//---------------------------------------------------------------------------------------
// handy routine for reading the family flags
//---------------------------------------------------------------------------------------

//	TODO

//static void parseFamilyFlag(const IPrimitive *prim, set<string> &result)
//{
//	static bool inited;					//	todo, change this as we can need to reload definitions .. (like new tribes or other ..)
//	static vector<string>	familyNames;
//	static vector<string>	tribeNames;
//
//	if (!inited)
//	{
//		// build the list of family and tribes
//		TPopulationFamily::getFamilyNames(familyNames);
//		const std::vector<std::pair<std::string, NLMISC::TStringId> > &names = TPopulationFamily::getTribesNames();
//
//		for (uint i=0; i<names.size(); ++i)
//		{
//			tribeNames.push_back(names[i].first);
//		}
//		
//		inited = true;
//	}
//
//	result.clear();
//	// read the family flags
//	for (uint i=0; i<familyNames.size(); ++i)
//	{
//		if (familyNames[i] == "tribe")
//		{
//			// special case for tribe
//			for (uint j=0; j<tribeNames.size(); ++j)
//			{
//				string flagName = tribeNames[j];
//				string s;
//				if (prim->getPropertyByName(flagName.c_str(), s) && s=="true")
//					result.insert(flagName);
//			}
//		}
//		else
//		{
//			// standard case for all other
//			string s;
//			if (prim->getPropertyByName(familyNames[i].c_str(), s) && s=="true")
//				result.insert(familyNames[i]);
//		}
//
//	}
//
//}

//---------------------------------------------------------------------------------------
// handy routine for scanning a node for mission sub-nodes
//---------------------------------------------------------------------------------------

static void lookForMissions(const IPrimitive *prim, std::vector<int> &missions, std::vector<std::string> &missionsNames)
{
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			if (NLMISC::nlstricmp(nodeClass(child),"mission")==0)
			{
				LOG("Found mission: %s: %s", 
					LigoConfig->aliasToString(nodeAlias(child)).c_str(), 
					nodeName(child).c_str());
				missions.push_back(nodeAlias(child));
				missionsNames.push_back(nodeName(child));
			}
		}
	}
}


//---------------------------------------------------------------------------------------
// handy structures for dealing with folders and their sub-trees
//---------------------------------------------------------------------------------------
struct SFolderRef
{
	SFolderRef() { }
	SFolderRef(const CAIAliasDescriptionNode *node,const IPrimitive *prim) : Prim(prim), Node(node) { }

	const IPrimitive *Prim;
 	NLMISC::CSmartPtr<const CAIAliasDescriptionNode> Node;
};



// the following variables are setup by parsePrimGrpNpc() and referenced by parsePrimNpcBot 
//oldlevel static std::string DefaultBotLevel;
static std::string DefaultBotLook;
static uint32	DefaultBotVerticalPos = vp_auto;
//static std::string DefaultBotStats;
static std::vector<std::string> EmptyStringVector;
static const std::vector<std::string> *DefaultBotKeywords=&EmptyStringVector;
static const std::vector<std::string> *DefaultBotEquipment=&EmptyStringVector;
static const std::vector<std::string> *DefaultBotChat=&EmptyStringVector;
static std::vector<std::string> DefaultMissionNames;
static std::vector<int> DefaultMissions;
static const std::vector<std::string> *DefaultGrpParameters=&EmptyStringVector;
static string	CurrentGroupFamily;



//---------------------------------------------------------------------------------------
// some handy utilities	for manageing the treeNode trees
//---------------------------------------------------------------------------------------

// the following routine looks though the children of treeNode for one that matches prim
// if no child is found then treeNode is returned
static const CAIAliasDescriptionNode *nextTreeNode(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name, type and alias
	std::string name=nodeName(prim);		
	TAIType type=nodeType(prim);
	uint32 uniqueId=0;
	if (type != AITYPES::AITypeBadType)
		uniqueId = nodeAlias(prim);

	// see if one of the children of treeNode corresponds to the primitive
	for (uint i=0;i<treeNode->getChildCount();++i)
	{
		CAIAliasDescriptionNode *childNode=treeNode->getChild(i);
		if	(	childNode->getAlias()==uniqueId	)
		{
			if	(	childNode->getType()!=type
				||	childNode->getName()!=name	)
			{
				nlwarning("nextTreeNode(): Unique ID conflict in node: (%s, %u, %s): looking for (%s, %u, %s) but found (%s, %u, %s)",
				getName(treeNode->getType()),treeNode->getAlias(),treeNode->fullName().c_str(),
				getName(childNode->getType()),childNode->getAlias(),childNode->fullName().c_str(),
				getName(type),uniqueId,name.c_str());
				continue;
			}
			return childNode;
		}

	}
	return treeNode;
}

// the following routine recursively scans 'prim' and its children, constructing a CAIAliasDescriptionNode tree
// to represent ai type tree entries
static	void	buildAliasTree(CAIAliasDescriptionNode *treeNode,CAIAliasDescriptionNode *rootNode,const IPrimitive *prim,std::vector<SFolderRef> &folders)
{
	// run through the node children looking for nodes with types that we recognize
	for	(uint i=0;i<prim->getNumChildren();++i)
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child,i))
		{
			// try to get a type, alias and name for the child
			TAIType type=nodeType(child);

			if	(type==AITypeBadType)
			{
				buildAliasTree(treeNode,rootNode,child,folders);
				continue;
			}

//			CAIAliasDescriptionNode *parentNode= (type==AITypeFolder)?	rootNode: treeNode;
			CAIAliasDescriptionNode *parentNode= treeNode;

			uint32		uniqueId=nodeAlias(child, true);
			std::string	name=nodeName(child);

			// make sure the name is unique
			if	(parentNode->findNodeChildByNameAndType(name,type)!=NULL)
			{
				LOG("Name not unique: %s: '%s:%s' @ '%s'",
					getName(type),parentNode->fullName().c_str(),
					name.c_str(),
					treeNode->fullName().c_str());
			}

			// make sure the unique id is unique (SLOW!, must be replace with a fast hash_map access!)
			if	(rootNode->lookupAlias(uniqueId)!=NULL)
			{
				nlwarning("WARNING - Alias clash for '%s'%s and '%s:%s' @ '%s'",
					rootNode->lookupAlias(uniqueId)->fullName().c_str(),
					LigoConfig->aliasToString(uniqueId).c_str(),
					parentNode->fullName().c_str(), name.c_str(),
					treeNode->fullName().c_str());
			}
			  
			// create a new tree node as a child of treeNode
			NLMISC::CSmartPtr<CAIAliasDescriptionNode>	node=new CAIAliasDescriptionNode(name,uniqueId,type,parentNode);

			// parse this branch as a sub-tree of our new node
			buildAliasTree(node,rootNode,child,folders);

			// if this was a folder then add to the folder vector
			if	(type==AITypeFolder)
				folders.push_back(SFolderRef(node,child));
		}

	}

}

//---------------------------------------------------------------------------------------
// routines used by the primitive file parser
//---------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//	States Methods
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//	predecl.
static void parsePrimGrpNpc(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,const std::string initialState);
static void parsePrimGrpFauna(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim);

static void parsePrimStateChat(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// read the main body of the parameters
	const std::vector<std::string> *botKeywords=&EmptyStringVector;
	const std::vector<std::string> *botNames=&EmptyStringVector;
	const std::vector<std::string> *chat=&EmptyStringVector;
	
	prim->getPropertyByName("bot_keyword_filter",botKeywords);
	prim->getPropertyByName("bots_by_name",botNames);
	prim->getPropertyByName("chat_parameters",chat);
	
	// register the profile
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("CHAT",treeNode->getAlias());
	if (!botKeywords->empty())	CAIActions::execute("BOTKEYS",*botKeywords);
	if (!botNames->empty())		CAIActions::execute("BOTNAMES",*botNames);
	if (!chat->empty())			CAIActions::execute("CHAT",*chat);
	CAIActions::end(treeNode->getAlias());
}


static void parsePrimStateProfile(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// read the main body of the parameters
	string			movingProfile;
	string			activityProfile;
	vector<string>	*profileParams = &EmptyStringVector;
	const std::vector<std::string> *grpKeywords=&EmptyStringVector;
	const std::vector<std::string> *grpNames=&EmptyStringVector;
	
	prim->getPropertyByName("ai_movement", movingProfile);
	prim->getPropertyByName("ai_activity", activityProfile);
	prim->getPropertyByName("ai_profile_params", profileParams);
	prim->getPropertyByName("grp_keyword_filter", grpKeywords);
	prim->getPropertyByName("grps_by_name", grpNames);
	
	// register the profile
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("PROFILE",treeNode->getAlias());
	CAIActions::exec("MOVEPROF", movingProfile);
	CAIActions::exec("ACTPROF", activityProfile);
	CAIActions::execute("PROFPARM", *profileParams);
	
	if (!grpKeywords->empty())	CAIActions::execute("GRPKEYS",*grpKeywords);
	if (!grpNames->empty())		CAIActions::execute("GRPNAMES",*grpNames);
	CAIActions::end(treeNode->getAlias());
}

static	CTmpPropertyZone::TSmartPtr	parseMachineStatePropertyZone(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	CTmpPropertyZone::TSmartPtr	propertyZone=new	CTmpPropertyZone();

//	void setPatat (AITYPES::TVerticalPos verticalPos, const std::vector <CAIVector> &points)

	uint numPoints=prim->getNumVector();
	if (numPoints!=0)
	{
		const CPrimVector *const pointArray=prim->getPrimVector();
		for (uint i=0;i<numPoints;++i)
		{
			propertyZone->points.push_back(CAIVector(pointArray[i].x,pointArray[i].y));
		}

	}

	vector<string>	*params = &EmptyStringVector;
	prim->getPropertyByName("params", params);
	for (uint i=0;i<params->size();i++)
	{
//		const	string	str=(*params)[i];
//		CPropertyId	activity=CPropertyId::create(str);
//		propertyZone->properties.addActivity(activity);

		propertyZone->properties.addProperty(CPropertyId::create((*params)[i]));
	}
	return	propertyZone;
}


static CAIEventActionNode::TSmartPtr parsePrimEventAction(const CAIAliasDescriptionNode *treeNode, const IPrimitive *prim)
{
	CAIEventActionNode::TSmartPtr result=new CAIEventActionNode;

	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=0;
	
	result->Alias = treeNode?treeNode->getAlias():uniqueId;
	
	LOG("Parsing npc event action: %s",name.c_str());
	
	// read the main body of the parameters
	std::string weightStr;
	const std::vector<std::string> *parameters=&EmptyStringVector;
	
	prim->getPropertyByName("action",result->Action);
	prim->getPropertyByName("weight",weightStr);
	prim->getPropertyByName("parameters",parameters);
	NLMISC::fromString(weightStr, result->Weight);
	result->Args=*parameters;
	
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(!prim->getChild(child,i))
			continue;

		// try to get a type for the child
		const	TAIType type=nodeType(child);

		// it's one of ours! - so match against the types we recognise
		switch(type)
		{
			case AITypeEventAction:
			{
				CAIEventActionNode::TSmartPtr childAction;
				childAction=parsePrimEventAction(treeNode?nextTreeNode(treeNode,child):NULL,child);
				if	(childAction)
					result->Children.push_back(childAction);
			}
			break;
			case AITypeActionZone:
			{
				result->_PropertyZones.push_back(parseMachineStatePropertyZone(treeNode?nextTreeNode(treeNode,child):NULL,child));
				result->_PropertyZones.back()->Target = CTmpPropertyZone::All;
			}
			break;
			case AITypeFaunaActionZone:
			{
				result->_PropertyZones.push_back(parseMachineStatePropertyZone(treeNode?nextTreeNode(treeNode,child):NULL,child));
				result->_PropertyZones.back()->Target = CTmpPropertyZone::Fauna;
			}
			break;
			case AITypeNpcActionZone:
			{
				result->_PropertyZones.push_back(parseMachineStatePropertyZone(treeNode?nextTreeNode(treeNode,child):treeNode,child));
				result->_PropertyZones.back()->Target = CTmpPropertyZone::Npc;
			}
			break;
			case AITypeBadType:
			case AITypeFolder:
			// not handled there, but by caller		
			break;
			default:
				nlwarning("Don't know how to treat ai_type '%s'",getName(type));
			break;
		}
		
	}
	return result;
}

/*
<PRIMITIVE CLASS_NAME="fauna_action_zone" TYPE="zone" R="128" G="128" B="128" A="128" AUTO_INIT="true" DELETABLE="true" NUMBERIZE="false">
<PARAMETER NAME="name" TYPE="string" VISIBLE="true"/>
<PARAMETER NAME="ai_type" TYPE="string" VISIBLE="false">
<DEFAULT_VALUE VALUE="FAUNA_ACTION_ZONE"/>
</PARAMETER>
<PARAMETER NAME="params" TYPE="string_array" VISIBLE="true" WIDGET_HEIGHT="100"/>
</PRIMITIVE>
 */


static void parsePrimGroupDescriptionsForAction(const CAIAliasDescriptionNode *aliasNode,
												const IPrimitive *prim,
												uint32 logicActionAlias
											   )
{	

	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child, i))
		{
			if (nodeType(child) == AITypeBadType) continue; // skip alias node
			const CAIAliasDescriptionNode *childTreeNode = nextTreeNode(aliasNode, child);			
			string	familyTag;
			child->getPropertyByName("family", familyTag);
			CAIActions::exec("GRPFAM", childTreeNode, familyTag, name, logicActionAlias);
			TAIType type = nodeType(child);
			switch(type)
			{
				case	AITypeGroupFamily:
					{
						nlwarning("Parsing a group_family, primitive is outdated. Please report to jvuarand.");
					//	parsePrimGroupFamily(nextTreeNode(aliasNode,child),child);
					}
					break;
					
				case AITypeGroupFamilyProfileFauna:
					parsePrimGroupFamilyProfileFaunaContent(childTreeNode, child);
	//				parsePrimGroupFamilyProfileGeneric(nextTreeNode(aliasNode,child),child, GroupFamilyFauna);
					break;
					
				case AITypeGroupFamilyProfileTribe:
					parsePrimGroupFamilyProfileTribeContent(childTreeNode, child);
					break;
				case AITypeGroupFamilyProfileNpc:
					parsePrimGroupFamilyProfileNpcContent(childTreeNode, child);
					break;			
					
	//			case AITypeGroupFamilyProfileGeneric:
	//				parsePrimGroupFamilyProfileGeneric(nextTreeNode(aliasNode,child),child, GroupFamilyTribe);
	//				break;	
				default:
					CAIActions::end(childTreeNode->getAlias());
				break;
			}
			
		}
	}	
}


// add group description to already parsed event actions
static void addGroupDescriptionToEventAction(const CAIAliasDescriptionNode *treeNode, const IPrimitive *prim, uint depth)
{		
	uint32 uniqueId=0;
	CAIActions::begin(treeNode?treeNode->getAlias():uniqueId);	
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure it's valid
		const IPrimitive *child;
		if	(!prim->getChild(child,i))
			continue;						
		//
		const	TAIType type=nodeType(child);

		// it's one of ours! - so match against the types we recognise
		switch(type)
		{
			case AITypeEventAction:
			{				
				addGroupDescriptionToEventAction(treeNode?nextTreeNode(treeNode,child):NULL,child, depth + 1);
			}
			break;
			case AITypeActionZone:
			case AITypeFaunaActionZone:
			case AITypeNpcActionZone:
				// no-op, already parsed
			break;				
			case AITypeFolder:
			{
				string cname = nodeClass(child);
				// parse optional group descriptions			
				if (cname == "group_descriptions")
				{										
					CAIActions::exec("SETACTN", treeNode?treeNode->getAlias():uniqueId);
					parsePrimGroupDescriptionsForAction(treeNode?nextTreeNode(treeNode,child):NULL, child, treeNode?treeNode->getAlias():uniqueId);
					CAIActions::exec("CLRACTN");
				}
			}
			break;
			case AITypeBadType:
			default:
				nlwarning("Don't know how to treat ai_type '%s'",getName(type));
			break;
		}		
	}	
	CAIActions::end(treeNode?treeNode->getAlias():uniqueId);
}


static void parsePrimEvent(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	CAIEventDescription::TSmartPtr result=new CAIEventDescription;
	
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	LOG("Parsing npc event: %s",name.c_str());
	
	// read the main body of the parameters
	const std::vector<std::string> *stateKeywords=&EmptyStringVector;
	const std::vector<std::string> *namedStates=&EmptyStringVector;
	const std::vector<std::string> *groupKeywords=&EmptyStringVector;
	const std::vector<std::string> *namedGroups=&EmptyStringVector;
	
	prim->getPropertyByName("event",result->EventType);

	
	// the event should have two children, the alias and the action
	const IPrimitive *child;
	std::string type;
	if (	prim->getNumChildren()!=2
		||	!prim->getChild(child,1)
		||	!child->getPropertyByName("ai_type",type)
		||	getType<TAIType>(type.c_str())!=AITypeEventAction)
	{
		nlwarning("Failed to find the action associated with event: %s (in %s)",name.c_str(), treeNode?treeNode->fullName().c_str():"");
		return;
	}
	
	result->Action=parsePrimEventAction(treeNode?nextTreeNode(treeNode,child):NULL,child);
	
	prim->getPropertyByName("state_keyword_filter",stateKeywords);
	prim->getPropertyByName("states_by_name",namedStates);
	prim->getPropertyByName("group_keyword_filter",groupKeywords);
	prim->getPropertyByName("groups_by_name",namedGroups);
	
	result->StateKeywords=	*stateKeywords;
	result->NamedStates=	*namedStates;
	result->GroupKeywords=	*groupKeywords;
	result->NamedGroups=	*namedGroups;	
	
	// register the event and call the parser for the associated action
	CAIActions::begin(treeNode?treeNode->getAlias():uniqueId);
	CAIActions::exec("EVENT",uniqueId,result);
	CAIActions::end(treeNode?treeNode->getAlias():uniqueId);

	// Each 'event actions' may have a group description attached to it,
	// parse it here, because we don't want to embed those descriptions in the
    // CAIEventDescription class				
	addGroupDescriptionToEventAction(treeNode?nextTreeNode(treeNode,child):NULL, child, 1);		
	CAIActions::exec("ENDEVENT"); // this will clear the logic actions map
}


//---------------------------------------------------------------------------------------
// kami routines

static void parsePrimBotKami(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	nlassert(false);
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	// lookup the kami type
	std::string kamiType;
	if (!prim->getPropertyByName("ai_kami_type",kamiType) || (kamiType!="PREACHER" && kamiType!="GUARDIAN"))
	{
		nlwarning("ai_kami_type property not found in kami record: %s",treeNode->fullName().c_str());
		return;
	}
	
	// lookup the creature sheet name
	std::string sheet;
	if (!prim->getPropertyByName("sheet",sheet))
	{
		nlwarning("'sheet' property not found in kami record: %s",treeNode->fullName().c_str());
		return;
	}
	
	// lookup x,y,theta
	const CPrimPoint *point=dynamic_cast<const CPrimPoint *>(prim);
	if (point==NULL)
	{
		nlwarning("Failed to cast to CPrimPoin kami: %s",name.c_str());
		CAIActions::end(treeNode->getAlias());
		return;
	}
	sint x=(uint32)(point->Point.x*1000);
	sint y=(uint32)(point->Point.y*1000);
	float theta=(float)point->Angle;
	
	
	// do the business
	LOG("Adding kami npc bot: %s: %s  pos: (%d,%d)  orientation: %.2f",kamiType.c_str(),name.c_str(),x,y,theta);
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("BOTNPC",treeNode->getAlias());
	CAIActions::exec("LOOK",sheet);
	CAIActions::exec("STATS",25);
	CAIActions::exec("KEYWORDS",kamiType);
	CAIActions::exec("STARTPOS",x,y,theta);
	
	if (kamiType=="PREACHER")
	{
		// todo: get rid of this code :o)
		// add teleport stuff to the Kami ... this is temporary
		std::vector<std::string> chat;
		chat.push_back(std::string("shop: KAMI_TP_FOREST"));
		CAIActions::execute("CHAT",chat);
	}
	
	CAIActions::end(treeNode->getAlias());
}

static void parsePrimGrpKami(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	LOG("Parsing group kami: %s",name.c_str());
	
	// extract x and y coords of points from patat (if there is one)
//	std::vector <CAIActions::CArg> points;
//	uint numPoints=prim->getNumVector();
//	if (numPoints!=0)
//	{
//		const CPrimVector *pointArray=prim->getPrimVector();
//		for (uint i=0;i<numPoints;++i)
//		{
//			points.push_back(CAIActions::CArg(pointArray[i].x));
//			points.push_back(CAIActions::CArg(pointArray[i].y));
//		}
//	}
	
	// setup the grp context
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("GRPNPC",uniqueId);
	
	// commit the zone points
//	if (!points.empty())
//		CAIActions::execute("PATAT",points);
	
	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			// try to get a type for the child
			// it's one of ours! - so match against the types we recognise
			switch(nodeType(child))
			{
			case AITypeBot:
				parsePrimBotKami(nextTreeNode(treeNode,child),child);
				break;
			case AITypeBadType:
			case AITypeFolder:
				break;
			default:
					nlwarning("Didn't found ai_type when expecting 'BOT_NPC' in parsePrimGrpKami");
				break;
			}
			
		}
		
	}
	CAIActions::end(treeNode->getAlias());
}



//---------------------------------------------------------------------------------------
// karavan routines

static void parsePrimBotKaravan(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	nlassert(false);
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	// lookup the karavan type
	std::string karavanType;
	if (!prim->getPropertyByName("ai_karavan_type",karavanType))
	{
		nlwarning("ai_karavan_type property not found in karavan record: %s",treeNode->fullName().c_str());
		return;
	}
	
	// lookup the creature sheet name
	std::string sheet;
	if (!prim->getPropertyByName("sheet",sheet))
	{
		nlwarning("'sheet' property not found in karavan record: %s",treeNode->fullName().c_str());
		return;
	}
	
	// add the bot to the current group
	const CPrimPoint *point=dynamic_cast<const CPrimPoint *>(prim);
	if (!point)
	{
		nlwarning("Failed to cast to CPrimPoin karavan: %s",name.c_str());
		CAIActions::end(treeNode->getAlias());
		return;
	}
	
	sint x=(uint32)(point->Point.x*1000);
	sint y=(uint32)(point->Point.y*1000);
	float theta=(float)point->Angle;
	
	LOG("Adding karavan npc bot: %s: %s  pos: (%d,%d)  orientation: %.2f",karavanType.c_str(),name.c_str(),x,y,theta);
	
	// do the business
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("BOTNPC",treeNode->getAlias());
	CAIActions::exec("LOOK",sheet);
	CAIActions::exec("STATS",25);
	CAIActions::exec("KEYWORDS",karavanType);
	CAIActions::exec("STARTPOS",x,y,theta);
	//	if (kamiType=="")
	{
		// todo: get rid of this code :o)
		// add teleport stuff to the Kami ... this is temporary
		std::vector<std::string> chat;
		chat.push_back(std::string("shop: KARAVAN_TP_FOREST"));
		CAIActions::execute("CHAT",chat);
	}
	CAIActions::end(treeNode->getAlias());
}

static	void	parsePrimGrpKaravan(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	LOG("Parsing group karavan: %s",name.c_str());
	
	// setup the grp context
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("GRPNPC",treeNode->getAlias());
	
	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			TAIType type=nodeType(child);
			
			switch(type)
			{
			case AITypeBot:
				parsePrimBotKaravan(nextTreeNode(treeNode,child),child);
				break;
			case AITypeFolder:
			case AITypeBadType:
				break;
			default:
				nlwarning("Unsupported ai_type in parsePrimGrpKaravan");
				break;
			}
			
		}
		
	}
	CAIActions::end(treeNode->getAlias());
}


static void parsePrimState(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,const char *pointsType)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	LOG("Parsing npc state with %s: %s",pointsType,name.c_str());
	
	// look for keywords
	std::string		moveProfile;
	std::string		activityProfile;
	vector<string>	*profileParams = &EmptyStringVector;
	const std::vector<std::string> *keywords=&EmptyStringVector;
	prim->getPropertyByName("grp_keywords",keywords);
	prim->getPropertyByName("ai_movement",moveProfile);
	prim->getPropertyByName("ai_activity",activityProfile);
	prim->getPropertyByName("ai_profile_params", profileParams);

	uint32	verticalPos;
	parseVerticalPos(prim, verticalPos);
	
	// extract x and y coords of points from spline or patat
	std::vector <CAIActions::CArg> points;
	uint numPoints=prim->getNumVector();
	if (numPoints!=0)
	{
		const CPrimVector *pointArray=prim->getPrimVector();
		for (uint i=0;i<numPoints;++i)
		{
			points.push_back(CAIActions::CArg(pointArray[i].x));
			points.push_back(CAIActions::CArg(pointArray[i].y));
		}
	}
	else
		LOG("State has no geometry: %s: %s",pointsType,name.c_str());
	
	// create the state
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("STATE",uniqueId);
	CAIActions::exec("MOVEPROF",moveProfile);
	CAIActions::exec("ACTPROF", activityProfile);
	CAIActions::execute("PROFPARM", *profileParams);
	CAIActions::exec("VERTPOS", verticalPos);
	if (!keywords->empty())
		CAIActions::execute("KEYWORDS",*keywords);
	if (!points.empty())	
		CAIActions::execute(pointsType,points);
	
	// run through the group children looking for nodes with types that we recognise
	uint j;
	for (j=0;j<prim->getNumChildren();++j)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,j))
		{
			// try to get a type for the child
			switch	(nodeType(child))
			{
			case AITypeNpcStateProfile:
			case AITypeNpcStateChat:
				// this kind of node is read in the next loop
				// this is because any mission in a group MUST be read before
				// the parsing of the state profile/
//				parsePrimStateProfile(nextTreeNode(treeNode,child),child);
				break;
			case AITypeGrp:
				{
					switch	(nodeTypeSpec(child))
					{
					case AITypeSpecNpc:
						parsePrimGrpNpc(nextTreeNode(treeNode,child),child,name);
						break;
					case AITypeSpecFauna:						
						parsePrimGrpFauna(nextTreeNode(treeNode,child),child);
						break;
					case AITypeSpecKami:						// a non-deposit kami group
						parsePrimGrpKami(nextTreeNode(treeNode,child),child);
						break;						
					case AITypeSpecKaravan:
						parsePrimGrpKaravan(nextTreeNode(treeNode,child),child);
						break;						
						
					default:
						nlwarning("Don't know how to treat ai_group of type '%s' in primState",getName(nodeTypeSpec(child)));
						break;
					}
					
				}
				break;
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode,child),child);
				break;
			case AITypeState:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
				break;				
			case	AITypeBadType:
			case	AITypeFolder:
				break;
			default:
				nlwarning("Don't know how to treat ai_type '%s'",getName(nodeType(child)));
				break;
			}
			
		}
		
	}
	// a second loop for state profile node.
	for (j=0;j<prim->getNumChildren();++j)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,j))
		{
			switch	(nodeType(child))
			{				
			case AITypeNpcStateProfile:
				parsePrimStateProfile(nextTreeNode(treeNode,child),child);
				break;
			case AITypeNpcStateChat:
				parsePrimStateChat(nextTreeNode(treeNode,child),child);
				break;
			default:	// we don't care unknown objets.
				break;
			}

		}
		
	}
	CAIActions::end(treeNode->getAlias());
}

//////////////////////////////////////////////////////////////////////////
//	End States Methods
//////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------------------
// shared routines

static void parsePrimPlaces(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			// hack because the place type
			std::string placeName;
			child->getPropertyByName("ai_place_type",placeName);

			// try to get a type for the child
			TAIType type=nodeType(child);
			if (	type!=AITypeBadType
				&&	type!=AITypeFolder)
			{
				// get hold of the unique id
				uint32 uniqueId=nodeAlias(child);

				// it's one of ours! - so match against the types we recognise
				if (type == AITypePlace || type == AITypeOutpost || type == AITypePlaceFauna)
				{
					// read the radius
					std::string radiusString;
					child->getPropertyByName("radius",radiusString);
					uint radius;
					NLMISC::fromString(radiusString, radius);
					if (radius == 0)
					{
						nlwarning("Ignoring place '%s' because bad radius: '%s' (converted to int as %u)",
							placeName.c_str(),
							radiusString.c_str(),
							radius);
						continue;
					}
//					if (radiusString!=toString(radius))
//					{
//						nlwarning("Ignoring place '%s' because bad radius: '%s'",placeName.c_str(),radiusString.c_str());
//						continue;
//					}
//
					// read the xy coordinates
					if (child->getNumVector()!=1)
					{
						nlwarning("Ignoring place '%s' because num points not 1 (%d)",placeName.c_str(),child->getNumVector());
						continue;
					}
					float x=(float)(child->getPrimVector()->x);
					float y=(float)(child->getPrimVector()->y);

					uint32	verticalPos;
					parseVerticalPos(child, verticalPos);

					if (type == AITypePlaceFauna)
					{
						std::string tmpStr;
						uint32 stayTimeMin = 1000;
						uint32 stayTimeMax = 1000;
						uint32 index = 0;
						std::string indexNext;
						bool flagSpawn = false;
						bool flagFood = false;
						bool flagRest = false;
						//
						bool        active = true;
						bool        timeDriven = false;
						std::string timeInterval;
						std::string dayInterval;
				
						//
						if (child->getPropertyByName("visit_time_min", tmpStr))
						{
							NLMISC::fromString(tmpStr, stayTimeMin);
						}
						if (child->getPropertyByName("visit_time_max", tmpStr))
						{
							NLMISC::fromString(tmpStr, stayTimeMax);
						}
						if (child->getPropertyByName("index", tmpStr))
						{
							NLMISC::fromString(tmpStr, index);
						}
						child->getPropertyByName("index_next", indexNext);
						if (child->getPropertyByName("flag_spawn", tmpStr))
						{
							flagSpawn = nlstricmp(tmpStr, "true") == 0;
						}
						if (child->getPropertyByName("flag_rest", tmpStr))
						{
							flagRest = nlstricmp(tmpStr, "true") == 0;
						}
						if (child->getPropertyByName("flag_food", tmpStr))
						{
							flagFood = nlstricmp(tmpStr, "true") == 0;
						}
						if (child->getPropertyByName("active", tmpStr))
						{
							active = nlstricmp(tmpStr, "true") == 0;
						}
						if (child->getPropertyByName("time_driven", tmpStr))
						{
							timeDriven = nlstricmp(tmpStr, "true") == 0;
						}
						child->getPropertyByName("time_interval", timeInterval);
						child->getPropertyByName("day_interval", dayInterval);
						LOG("Adding place (type XYRFauna): %s at: (%.0f,%.0f) x %d (%s)",
							placeName.c_str(),
							child->getPrimVector()->x,
							child->getPrimVector()->y,
							radius,
							verticalPosToString((TVerticalPos)verticalPos).c_str());

						// add the place to the current context
						CAIActions::begin(uniqueId);
						CAIActions::exec("PLACEXYR",uniqueId,x,y,radius*1000, verticalPos);
						CAIActions::exec("PLXYRFAF", uniqueId, flagSpawn, flagRest, flagFood); // set flags
						CAIActions::exec("PLXYRFAS", uniqueId, stayTimeMin, stayTimeMax);     // set stay times
						CAIActions::exec("PLXYRFAI", uniqueId, index, indexNext);             // set indices
						CAIActions::exec("PLXYRFAA", uniqueId, active, timeDriven, timeInterval, dayInterval);             // set indices
						CAIActions::end(uniqueId);
					}
					else
					{
						LOG("Adding place (type XYR): %s at: (%.0f,%.0f) x %d (%s)",
							placeName.c_str(),
							child->getPrimVector()->x,
							child->getPrimVector()->y,
							radius,
							verticalPosToString((TVerticalPos)verticalPos).c_str());

						// add the place to the current context
						CAIActions::begin(uniqueId);
						CAIActions::exec("PLACEXYR",uniqueId,x,y,radius*1000, verticalPos);
						CAIActions::end(uniqueId);
					}
				}
			} 
		}
	}
}

struct parsePopException : public Exception
{
	parsePopException	(const	std::string &Reason) : Exception(Reason)
	{}
};

static void	parsePopulation(const IPrimitive *prim, std::string &sheet, uint &count)
{
	// try to get a type for the child
	std::string type;
	if (!prim->getPropertyByName("ai_type",type))
	{
		throw	parsePopException(std::string("ai_type not found"));
	}

	// it's one of ours! - so match against the types we recognise
	if (NLMISC::nlstricmp(type,"FAUNA_SPAWN_ATOM")!=0)
	{
		throw	parsePopException(std::string("Expected ai_type FAUNA_SPAWN_ATOM but found")+type);
	}
		
	std::string	countStr;

	if	(!prim->getPropertyByName("count",countStr))
	{
		throw	parsePopException(std::string("FAUNA_SPAWN_ATOM failed to find property ''count'"));
	}

	{
		string s;
		if (prim->getPropertyByName("creature_code", s) && !s.empty())
			sheet = s+".creature";
	}
		
	NLMISC::fromString(countStr, count);
	if (count<=0)
	{
		throw	parsePopException(std::string("FAUNA_SPAWN_ATOM property 'count' invalid: ")+countStr);
	}

}

//---------------------------------------------------------------------------------------
// fauna routines

static void parsePrimGrpFaunaSpawn(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	LOG("Parsing group fauna spawn: %s",name.c_str());

	// we need a vector of arguments for the call to execute() at the end
	std::vector <CAIActions::CArg> executeArgs;
	executeArgs.push_back(CAIActions::CArg(uniqueId));
	
	// lookup the spawn type for the manager
	std::string spawnType("ALWAYS");
	prim->getPropertyByName("spawn_type",spawnType);
	executeArgs.push_back(CAIActions::CArg(spawnType));
	
	// deal with the weight
	std::string s;
	uint32 weight = 0;
	if (prim->getPropertyByName("weight",s))
	{
		NLMISC::fromString(s, weight);
		if	(toString(weight)!=s)
		{
		nlwarning("weight invalid value: %s");
		weight=100;
		}
	}
	executeArgs.push_back(CAIActions::CArg(weight));	

	// run through the group children looking for nodes with types that we recognise
	for (uint i=0; i<prim->getNumChildren(); ++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (!prim->getChild(child,i))
			continue;

		// not interested to parse alias nodes !
		if (nodeClass(child) == "alias")
			continue;

		try
		{
			std::string		theSheet;
			uint			count;
			parsePopulation	(child, theSheet, count);
			if	(theSheet.empty())
				continue;
			executeArgs.push_back(CAIActions::CArg(theSheet));
			executeArgs.push_back(count);
		}
		catch (const parsePopException &e)
		{
			nlwarning("FaunaGroup: %s of %s : %s", nodeName(child).c_str(), treeNode->fullName().c_str(), e.what());
		}
		
	}

	// open the parse context
	CAIActions::begin(treeNode->getAlias());

	// call the 'execution' system
	if (	!executeArgs.empty()
		&&	executeArgs.size()>=5)
		CAIActions::execute("POPVER",executeArgs);
	else
		nlwarning("FAUNA_SPAWN failed because population is empty, '%s'%s",
			name.c_str(), 
			LigoConfig->aliasToString(uniqueId).c_str());

	// close the parse context
	CAIActions::end(treeNode->getAlias());
}



static void parsePrimGrpFauna(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// lookup the fauna type for the manager
/*	std::string faunaTypeStr;
	prim->getPropertyByName("fauna_type",faunaTypeStr);

	if	(	getType<TFaunaType>(faunaTypeStr.c_str())==FaunaTypeBadType	)
	{
		nlwarning("Ignoring group %s due to unknown type: '%s'",name.c_str(),faunaTypeStr.c_str());
		return;
	}
*/
	// open the parse context
	LOG("Parsing group fauna: %s",name.c_str());
	CAIActions::begin(treeNode->getAlias());


//	Don't know what for .. ???
//	// get hold of the ids of the places
//	uint32 feedPlace=~0u, restPlace=~0u, spawnPlace=~0u;
//	for (uint i=0;i<treeNode->getChildCount();++i)
//	{
//		if (treeNode->getChild(i)->getType()==AITypePlace)
//		{
//			if (name=="FAUNA_FOOD")		feedPlace=treeNode->getChild(i)->getAlias();
//			if (name=="FAUNA_REST")		restPlace=treeNode->getChild(i)->getAlias();
//			if (name=="FAUNA_SPAWN")	spawnPlace=treeNode->getChild(i)->getAlias();
//		}
//		
//	}

	// setup the grp context
	CAIActions::exec("GRPFAUNA",uniqueId/*,faunaTypeStr*/);

	// create any places that we're going to reffer to later
	parsePrimPlaces(nextTreeNode(treeNode,prim),prim);
	
	// time variables used for determining time spent eating, rate of progress of hunger, etc
	std::string s;
	if (prim->getPropertyByName("times",s) && !s.empty())
	{
		float time1=0, time2=0;
		sscanf(s.c_str(),"%f %f",&time1,&time2);
		CAIActions::exec("SETTIMES",time1,time2);
	}
	else	
	{
		LOG("No 'times' record found: using default value: 10 10");
		float time1=30.0f, time2=120.0f;
		CAIActions::exec("SETTIMES",time1,time2);
	}
	s.clear();
	prim->getPropertyByName("autoSpawn", s);
	CAIActions::exec("AUTOSPWN", uint32(nlstricmp( s.c_str(), "false") != 0));

	// time variables used for determining time corpses stay on ground and time before creatures respawn
	s.clear();
	prim->getPropertyByName("spawn_times",s);		// This code is a nasty hack to provide legacy support
	if (s.empty())									// for .primitive files generated with a defective
		prim->getPropertyByName("Spawn_times",s);	// world_editor.xml (case problem in 'spawn_times')
	if (!s.empty())
	{
		float time1=0, time2=0, time3=-1;
		sscanf(s.c_str(),"%f %f %f",&time1,&time2,&time3);
		if(time3==-1)
			CAIActions::exec("SPAWTIME",time1,time2);
		else
			CAIActions::exec("SPAWTIME",time1,time2,time3);
	}
	else
	{
		LOG("No 'spawn times' record found: using default value: 30 120");
		float time1=30.0f, time2=120.0f;
		CAIActions::exec("SPAWTIME",time1,time2);
	}

	//	ring cycles.
	{
		std::string cycles("");
		prim->getPropertyByName("cycles",cycles);
		CAIActions::exec("STCYCLES",cycles);
	}

	// solidarity used 
	s.clear();
	prim->getPropertyByName("solidarity",s);
	CAIActions::exec("ASSIST",uint32(nlstricmp( s.c_str(), "disabled")!=0));

	// run through the group children looking for nodes with types that we recognise
	for (uint j=0;j<prim->getNumChildren();++j)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (!prim->getChild(child,j))
			continue;

		// try to get a type for the child
		TAIType type=nodeType(child);
		switch(type)
		{
		case AITypeGrpFaunaPop:
			parsePrimGrpFaunaSpawn(nextTreeNode(nextTreeNode(treeNode,prim),prim),child);
			break;
		case AITypeEvent:
			parsePrimEvent(nextTreeNode(treeNode,child),child);
			break;
		case AITypeBadType:
		case AITypeFolder:
			break;

		default:
			if (type!=AITypePlace && type!=AITypePlaceFauna)
				nlwarning("Don't know how to treat ai_type '%s'",getName(type));
			break;
		}				

	}
	CAIActions::end(treeNode->getAlias());
}

static void parsePrimMgrFauna(const std::string &mapName,const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,std::vector<SFolderRef> &folders, const std::string &filename, bool firstTime=true)
{
	// get hold of the unique id
	uint32 uniqueId=nodeAlias(prim, true);

	// setup the mgr context
	if (firstTime)
	{
		CAIActions::begin(treeNode->getAlias());
		CAIActions::exec("MGRFAUNA",treeNode->getAlias(),treeNode->getName(),mapName, filename);
		CAIActions::exec("IDTREE",treeNode);
	}

	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (!prim->getChild(child,i))
			continue;

		// try to get a type for the child
		TAIType type=nodeType(child);
		
		switch(type)
		{
		case AITypeGrp:
			parsePrimGrpFauna(nextTreeNode(treeNode,child),child);
			break;
		case AITypeState:
			parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
			break;
		case AITypeEvent:
			parsePrimEvent(nextTreeNode(treeNode,child),child);
			break;			
			// this isn't an ai block so check its children
		case AITypeBadType:
			parsePrimMgrFauna(mapName,nextTreeNode(treeNode,child),child,folders,filename,false);
			break;
		case AITypeFolder:
			break;
		default:
			nlwarning("Found ai_type: '%s' when expecting 'GROUP_FAUNA'",getName(type));
			break;
		}

	}
	if (firstTime)
		CAIActions::end(treeNode->getAlias());
}

static void parsePrimNPCPunctualState(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	LOG("Parsing npc punctual state: %s",name.c_str());

	// look for keywords
	const std::vector<std::string> *keywords=&EmptyStringVector;
	prim->getPropertyByName("grp_keywords",keywords);

	// create the state
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("PUNCTUAL",uniqueId);
	if (!keywords->empty())	CAIActions::execute("KEYWORDS",*keywords);

	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			switch	(nodeType(child))
			{
			case AITypeNpcStateProfile:
				parsePrimStateProfile(nextTreeNode(treeNode,child),child);
				break;
			case AITypeNpcStateChat:
				parsePrimStateChat(nextTreeNode(treeNode,child),child);
				break;
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode,child),child);
				break;
			case AITypeBadType:
			case AITypeFolder:
				break;
			default:	// we don't care unknown objets.
				nlwarning("Don't know how to treat ai_type '%s'",getName(nodeType(child)));
				break;
			}
			
		}

	}
	CAIActions::end(treeNode->getAlias());
}

static void parsePrimMgrKami(const std::string &mapName,const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,std::vector<SFolderRef> &folders, const std::string &filename,bool firstTime=true)
{
	// get hold of the unique id
	uint32 uniqueId=nodeAlias(prim);

	// setup the mgr context
	if (firstTime)
	{
		CAIActions::begin(treeNode->getAlias());
		CAIActions::exec("MGRNPC",treeNode->getAlias(),treeNode->getName(),mapName, filename);
		CAIActions::exec("IDTREE",treeNode);
	}

	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (!prim->getChild(child,i))
			continue;
		
		// try to get a type for the child
		TAIType type=nodeType(child);
		
		switch(type)
		{
		case AITypeNpcStateRoute:
			parsePrimState(nextTreeNode(treeNode,child),child,"PATH");
			break;
		case AITypeNpcStateZone:
			parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
			break;
		case AITypePunctualState:
			parsePrimNPCPunctualState(nextTreeNode(treeNode,child),child);
			break;
		case AITypeEvent:
			parsePrimEvent(nextTreeNode(treeNode,child),child);
			break;
			
		case AITypeKamiDeposit:			// a deposit
			parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
			break;
			
		case AITypeBadType:
			parsePrimMgrKami(mapName,nextTreeNode(treeNode,child),child,folders,filename, false);
			break;
		case AITypeFolder:
			break;
		default:
			nlwarning("Unsupported ai_type in parsePrimMgrKami");
			break;
		}

	}
	if (firstTime)
		CAIActions::end(treeNode->getAlias());
}

static void parsePrimMgrKaravan(const std::string &mapName,const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,std::vector<SFolderRef> &folders, const std::string &filename,bool firstTime=true)
{
	// get hold of the unique id
	uint32 uniqueId=nodeAlias(prim);

	// setup the mgr context
	if (firstTime)
	{
		CAIActions::begin(treeNode->getAlias());
		CAIActions::exec("MGRNPC",treeNode->getAlias(),treeNode->getName(),mapName, filename);
		CAIActions::exec("IDTREE",treeNode);
	}

	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			// try to get a type for the child
			TAIType type=nodeType(child);
			
			switch(type)
			{
			case AITypeKaravanState:
				parsePrimState(nextTreeNode(treeNode,child),child,"");
				break;
			case AITypeBadType:
				parsePrimMgrKaravan(mapName,nextTreeNode(treeNode,child),child,folders,filename, false);
				break;
			case AITypeFolder:
				break;
			default:
				nlwarning("Unsupported ai_type in parsePrimMgrKaravan");
				break;
			}

		}

	}
	if (firstTime)
		CAIActions::end(treeNode->getAlias());
}

//---------------------------------------------------------------------------------------
// npc routines


void mergeEquipement(const std::vector<std::string> &grpEquip, const std::vector<std::string> &botEquip, std::vector<std::string> &result)
{
	uint i;
	map<string, string> equip;
	string key, tail;

	for (i=0; i<grpEquip.size(); ++i)
	{
		if (stringToKeywordAndTail(grpEquip[i], key, tail))
			equip[key] = tail;
	}
	for (i=0; i<botEquip.size(); ++i)
	{
		if (stringToKeywordAndTail(botEquip[i], key, tail))
			equip[key] = tail;
	}

	result.clear();
	// rebuild the final equipement
	map<string, string>::iterator first(equip.begin()), last(equip.end());
	for (; first != last; ++first)
	{
		result.push_back(first->first + " : " + first->second);
	}
}

static void parsePrimBotNpc(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// setup the set of parameters that are initialised by default in parsePrimGrpNpc()
//oldlevel 	std::string						levelStr; /*	= DefaultBotLevel;	*/
	std::string						look; //		= DefaultBotLook;    	
//	std::string						fight;
//	std::string						stats; //		= DefaultBotStats;    
	const std::vector<std::string> *keywords	= NULL; //DefaultBotKeywords;	
	const std::vector<std::string> *equipment	= NULL; //&EmptyStringVector;//	= DefaultBotEquipment;
	const std::vector<std::string> *chat		= NULL; //DefaultBotChat;
	static std::vector<int>			missions;
	static std::vector<std::string>	missionsNames;
	missions = DefaultMissions;
	missionsNames = DefaultMissionNames;
	std::string						isStuck;
	uint32							verticalPos;
	
	// try to read the above parameters from prim (on failure default value persists)
//oldlevel 	if (!prim->getPropertyByName("level", levelStr) || levelStr.empty())
//oldlevel 		levelStr = DefaultBotLevel;
	if (!prim->getPropertyByName("keywords", keywords) || keywords == NULL)
		keywords = DefaultBotKeywords;
	if (!prim->getPropertyByName("equipment", equipment) || equipment == NULL)
		equipment = &EmptyStringVector;
	if (!prim->getPropertyByName("chat_parameters",	chat) || chat == NULL)
		chat = DefaultBotChat;
	if (!prim->getPropertyByName("sheet_client", look) || look.empty())
		look = DefaultBotLook;
	prim->getPropertyByName("is_stuck",			isStuck);
	lookForMissions(prim, missions, missionsNames);

	if (!parseVerticalPos(prim, verticalPos))
		verticalPos = DefaultBotVerticalPos;

	// build the equipement
	std::vector<std::string> equipmentMerged;
	mergeEquipement(*DefaultBotEquipment, *equipment, equipmentMerged);

	// lookup coordinate and orientation for the bot
	const CPrimPoint *point=dynamic_cast<const CPrimPoint *>(prim);
	if (point==NULL)
	{
		nlwarning("Failed to cast to CPrimPoin bot: %s",name.c_str());
		return;
	}
	sint x=(uint32)(point->Point.x*1000);
	sint y=(uint32)(point->Point.y*1000);
	float theta=(float)point->Angle;

	// setup the bots
	LOG("Adding npc bot: %s  pos: (%d,%d)  orientation: %.2f",name.c_str(),x,y,theta);

	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("BOTNPC",uniqueId);
	CAIActions::exec("STARTPOS",x,y,theta, verticalPos);
	CAIActions::exec("LOOK",look);
//	CAIActions::exec("STATS",stats+NLMISC::toString("_lvl_%02d",level),level);
	CAIActions::exec("STATS");	//,level);
	CAIActions::exec("ISSTUCK", uint32(nlstricmp( isStuck.c_str(), "true") == 0));
	CAIActions::exec("BLDNGBOT", uint32(false));
//	if ( !fight.empty() ) 
//		CAIActions::exec("FIGHTBRK",fight);
	CAIActions::execute("CHAT", *chat);
	CAIActions::execute("EQUIP", equipmentMerged);
	CAIActions::execute("KEYWORDS", *keywords);
	for (uint i=0; i<missions.size(); ++i)
		CAIActions::exec("MISSIONS", missions[i], missionsNames[i]);
//		CAIActions::execute("MISSIONS", missions, missionsNames);
	CAIActions::end(treeNode->getAlias());
}

static void parsePrimGrpParameters(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	vector<string>	*profileParams = &EmptyStringVector;
 	DefaultGrpParameters	=&EmptyStringVector;	
	
	prim->getPropertyByName("grp_parameters",DefaultGrpParameters);
	prim->getPropertyByName("ai_profile_params",profileParams);

	CAIActions::execute("PARAMETR",*DefaultGrpParameters);
	CAIActions::execute("PROFPARM",*profileParams);
}

static void parsePrimGrpNpc(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim, const std::string initialState)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	LOG("Parsing npc group: %s",name.c_str());

	// setup the grp context
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("GRPNPC",treeNode->getAlias());

	// group parameters
	const std::vector<std::string> *keywords=&EmptyStringVector;
	prim->getPropertyByName("grp_keywords",keywords);
	CAIActions::execute("KEYWORDS",*keywords);

	string s;
	prim->getPropertyByName("autoSpawn", s);
	CAIActions::exec("AUTOSPWN", uint32(nlstricmp( s.c_str(), "false") != 0));

	// bot groups contain a set of default parameters for their bot children
//oldlevel 	DefaultBotLevel.clear();				prim->getPropertyByName("bot_level",DefaultBotLevel);
	
	DefaultBotLook.clear();					prim->getPropertyByName("bot_sheet_client",DefaultBotLook);
//	DefaultBotStats.clear();				prim->getPropertyByName("bot_sheet_server",DefaultBotStats);

	DefaultBotKeywords=&EmptyStringVector;	prim->getPropertyByName("bot_keywords",DefaultBotKeywords);
	DefaultBotEquipment=&EmptyStringVector;	prim->getPropertyByName("bot_equipment",DefaultBotEquipment);
	DefaultBotChat=&EmptyStringVector;		prim->getPropertyByName("bot_chat_parameters",DefaultBotChat);
	DefaultMissionNames.clear();
	DefaultMissions.clear();				lookForMissions(prim, DefaultMissions, DefaultMissionNames);


	parseVerticalPos(prim, DefaultBotVerticalPos, "bot_vertical_pos");
	
	DefaultGrpParameters	=&EmptyStringVector;
	CAIActions::execute("PARAMETR",*DefaultGrpParameters);

	// count the number of bots so that we can allocate the correct space for them
	bool foundBots=false;
	for (uint j=0;!foundBots && j<prim->getNumChildren();++j)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,j))
		{
			// try to get a type for the child - we're looking for children of type 'NPC_BOT'
			TAIType type=nodeType(child);
			if	(	type!=AITypeBadType
				&&	type==AITypeBot	)
				foundBots=true;
		}
	}

	// parse the group parameter
	for (uint i=0; i<prim->getNumChildren(); ++i)
	{
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			if (nodeClass(child) == "npc_group_parameters")
			{
				parsePrimGrpParameters(nextTreeNode(treeNode,child),child);
				break;
			}
		}
	}

	prim->getPropertyByName("count",s);
	uint botCount;
	NLMISC::fromString(s, botCount);

	if (foundBots && botCount != 0)
	{
		nlwarning("Mixing of automatic and explicit bot is not supported, only explicit bot will be spawned ! (in '%s')", treeNode->fullName().c_str());
	}
	// if we didn't find any explicit bots then generate a set of 'default' bots
	if (!foundBots)
	{
		if (botCount>200 || s!=NLMISC::toString(botCount))
		{
			nlwarning("Invalid 'count' (value: '%s') parameter in NPC bot group: %s",s.c_str(),treeNode->fullName().c_str());
			return;
		}
		if (botCount==0)
		{
			if (!DefaultBotLook.empty())
			{
				nlwarning("No bots found in NPC group: %s",treeNode->fullName().c_str());
				return;
			}
		}

		CAIActions::exec("BOTCOUNT",botCount);
		for (uint i=0;i<botCount;++i)
		{
			CAIActions::begin(i);
			CAIActions::exec("BOTNPC",i);
			CAIActions::exec("STARTPOS",0,0,0.0f, DefaultBotVerticalPos);
			CAIActions::exec("LOOK",	DefaultBotLook);
			CAIActions::exec("STATS");	//,	level	);	//	+NLMISC::toString("_lvl_%02d",level)
			CAIActions::execute("CHAT",*DefaultBotChat);
			CAIActions::execute("EQUIP",*DefaultBotEquipment);
			CAIActions::execute("KEYWORDS",*DefaultBotKeywords);
			CAIActions::end(i);
		}
	}
	else
		CAIActions::exec("BOTCOUNT",0);	// tell the system that we are not using auto generated bots but named bots

	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			// try to get a type for the child
				// it's one of ours! - so match against the types we recognise
			switch(nodeType(child))
			{
			case AITypeBot:
				parsePrimBotNpc(nextTreeNode(treeNode,child),child);
				break;
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode,child),child);
				break;
			case AITypeBadType:
			case AITypeFolder:
//			case AITypeGrpParameters:
				break;
			default:
				nlwarning("Don't know how to treat ai_type '%s'",getName(nodeType(child)));
				break;
			}

		}

	}
	CAIActions::end(treeNode->getAlias());
}

/*static void parsePrimNPCPunctualState(const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	LOG("Parsing npc punctual state: %s",name.c_str());

	// look for keywords
	const std::vector<std::string> *keywords=&EmptyStringVector;
	prim->getPropertyByName("grp_keywords",keywords);

	// create the state
	CAIActions::begin(treeNode->getAlias());
	CAIActions::exec("PUNCTUAL",uniqueId);
	if (!keywords->empty())	CAIActions::execute("KEYWORDS",*keywords);

	// run through the group children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			// try to get a type for the child
			TAIType type=nodeType(child);
			if (	type!=AITypeBadType
				&&	type!=AITypeFolder)
			{
				if (type==AITypeNpcStateProfile)
					parsePrimStateProfile(nextTreeNode(treeNode,child),child);
				else if (type==AITypeEvent)
					parsePrimEvent(nextTreeNode(treeNode,child),child);
				else
					nlwarning("Don't know how to treat ai_type '%s'",getName(type));
			} 
		}
	}

	CAIActions::end(treeNode->getAlias());
}
*/
static void parsePrimMgrNpc(const std::string &mapName,const CAIAliasDescriptionNode *treeNode,const IPrimitive *prim,std::vector<SFolderRef> &folders, const std::string &filename,bool firstTime=true)
{
	// get hold of the unique id
	uint32 uniqueId=nodeAlias(prim);

	// setup the mgr context
	if (firstTime)
	{
		CAIActions::begin(treeNode->getAlias());
		CAIActions::exec("MGRNPC",treeNode->getAlias(),treeNode->getName(),mapName, filename);
		CAIActions::exec("IDTREE",treeNode);
	}

	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			switch(nodeType(child))
			{
			case AITypeNpcStateRoute:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATH");
				break;
			case AITypeNpcStateZone:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
				break;
			case AITypeState:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
				break;
			case AITypePunctualState:
				parsePrimNPCPunctualState(nextTreeNode(treeNode,child),child);
				break;
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode,child),child);
				break;
			case AITypeGrp:
				parsePrimGrpNpc(nextTreeNode(treeNode,child),child,std::string());
				break;
			// uinknown so pass it..
			case AITypeBadType:
				if (nodeClass(child) != "alias")
					parsePrimMgrNpc(mapName,nextTreeNode(treeNode,child),child,folders,filename, false);
				break;
			case AITypeFolder:
				break;
			default:
				nlwarning("unrecognised ai_type in NPC manager %s: '%s'",treeNode->fullName().c_str(),getName(nodeType(child)));
				break;
			}
		}
	}

	if (firstTime)
	{
		// now add the folders to the manager
		for (uint i=0;i<folders.size();++i)
			parsePrimMgrNpc(mapName,folders[i].Node,folders[i].Prim,folders, filename,false);
		// close the manger
		CAIActions::end(treeNode->getAlias());
	}
}

static void parsePrimMgrOutpost(std::string const& mapName, CAIAliasDescriptionNode const* treeNode, IPrimitive const* prim, std::vector<SFolderRef>& folders, std::string const& filename,bool firstTime=true)
{
	// get hold of the unique id
	uint32 uniqueId = nodeAlias(prim);
	
	// setup the mgr context
	if (firstTime)
	{
		std::string str;
		prim->getPropertyByName("manual_spawn", str);
		bool manualSpawn = (str == "true");
		CAIActions::begin(treeNode->getAlias());
		CAIActions::exec("MGROUTPO",treeNode->getAlias(), treeNode->getName(), mapName, filename, manualSpawn);
		CAIActions::exec("IDTREE",treeNode);
	}
	
	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child,i))
		{
			switch(nodeType(child))
			{
			case AITypeNpcStateRoute:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATH");
				break;
			case AITypeNpcStateZone:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
				break;
			case AITypeState:
				parsePrimState(nextTreeNode(treeNode,child),child,"PATAT");
				break;
			case AITypePunctualState:
				parsePrimNPCPunctualState(nextTreeNode(treeNode,child),child);
				break;
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode,child),child);
				break;
			case AITypeGrp:
				parsePrimGrpNpc(nextTreeNode(treeNode,child),child,std::string());
				break;
				// uinknown so pass it..
			case AITypeBadType:
				if (nodeClass(child) != "alias")
					parsePrimMgrOutpost(mapName, nextTreeNode(treeNode, child), child, folders, filename, false);
				break;
			case AITypeFolder:
				break;
			default:
				nlwarning("unrecognised ai_type in outpost manager %s: '%s'",treeNode->fullName().c_str(),getName(nodeType(child)));
				break;
			}
		}
	}
	
	if (firstTime)
	{
		// now add the folders to the manager
		for (uint i=0;i<folders.size();++i)
			parsePrimMgrOutpost(mapName, folders[i].Node, folders[i].Prim, folders, filename, false);
		// close the manger
		CAIActions::end(treeNode->getAlias());
	}
}

//---------------------------------------------------------------------------------------
// parsing route nodes for managers (of all types)

static void parsePrimMgr(const IPrimitive *prim,const std::string &mapName, const std::string &filename)
{
	H_AUTO( parsePrim_Mgr );
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	LOG("Parsing manager: %s",name.c_str());

	// make sure the manager type is specified and is one of ours
	std::string mgrTypeName;
	TMgrType mgrType=MgrTypeBadType;
	prim->getPropertyByName("ai_manager_type",mgrTypeName);
	mgrType=getTypeAs(mgrType,mgrTypeName.c_str());
	if (mgrType==MgrTypeBadType)
	{
		nlwarning("Ignoring manager due to unknown type: '%s'",mgrTypeName.c_str());
		return;
	}

//	TAITypeSpec	typeSpec;
//	switch (mgrType)
//	{
//		case MgrTypeFauna:		typeSpec = AITypeSpecFauna;		break;
//		case MgrTypeKaravan:	typeSpec = AITypeSpecKaravan;	break;
//		case MgrTypeKami:		typeSpec = AITypeSpecKami;		break;
////		case MgrTypeTribe:		typeSpec = AITypeSpecTribe;		break;
//		case MgrTypeNpc:		typeSpec = AITypeSpecNpc;		break;
//			
//		// TODO: case MgrTypePet:		 
//		// TODO: case MgrTypeGuildNpc:	
//			
//		default: nlwarning("Unhandled ai manager type: '%s'",mgrTypeName.c_str());
//	}

		// build the tree of aliases and the vector of folders
		std::vector<SFolderRef> folders;
		NLMISC::CSmartPtr<CAIAliasDescriptionNode>	aliasTree;
	{
		H_AUTO( parsePrim_aliasTree );
		aliasTree=new	CAIAliasDescriptionNode	(name, uniqueId, AITypeManager, NULL);
		buildAliasTree(aliasTree,aliasTree,prim,folders);
	}

	switch	(mgrType)
	{
		case MgrTypeFauna:		parsePrimMgrFauna(mapName,aliasTree,prim,folders, filename);	break;
		case MgrTypeKaravan:	parsePrimMgrKaravan(mapName,aliasTree,prim,folders, filename);	break;
		case MgrTypeKami:		parsePrimMgrKami(mapName,aliasTree,prim,folders, filename);		break;
//		case MgrTypeTribe:		parsePrimMgrTribe(mapName,aliasTree,prim,folders);				break;
		case MgrTypeNpc:		parsePrimMgrNpc(mapName,aliasTree,prim,folders, filename);		break;
		case MgrTypeOutpost:	parsePrimMgrOutpost(mapName,aliasTree,prim,folders, filename);	break;
		
		// TODO: case MgrTypePet:
		// TODO: case MgrTypeGuildNpc:
		
		default: nlwarning("Unhandled ai manager type: '%s'",mgrTypeName.c_str());
	}

	CAIAliasDescriptionNode::flushUnusedAliasDescription	();
}

//---------------------------------------------------------------------------------------
// parsing spires
/*
static void parsePrimMgrSpire(std::string const& mapName, CAIAliasDescriptionNode const* treeNode, IPrimitive const* prim, std::vector<SFolderRef>& folders, std::string const& filename, bool firstTime=true)
{
	uint32 uniqueId = nodeAlias(prim);
	
	// setup the mgr context
	if (firstTime)
	{
	}
	
	parsePrimGrpSpire(treeNode, prim, "");
	
	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0; i<prim->getNumChildren(); ++i)
	{
		// get a pointer to the child and make sure its valid
		IPrimitive const* child;
		if (prim->getChild(child, i))
		{
			switch (nodeType(child))
			{
			case AITypeEvent:
				parsePrimEvent(nextTreeNode(treeNode, child), child);
				break;
			default:
				nlwarning("unrecognised ai_type in NPC manager %s: '%s'",treeNode->fullName().c_str(),getName(nodeType(child)));
				break;
			}
		}
	}
	
	CAIActions::end(treeNode->getAlias());
}
*/
static void parsePrimSpire(IPrimitive const* prim, std::string const& mapName, std::string const& filename)
{
	// get hold of the node name and unique id
	std::string name = nodeName(prim);
	uint32 uniqueId = nodeAlias(prim);
	
	LOG("Parsing spire: %s", name.c_str());
	
	CPrimPoint const* point = dynamic_cast<CPrimPoint const*>(prim);
	if (!point)
	{
		nlwarning("Failed to cast spire to CPrimPoint: %s", name.c_str());
		return;
	}
	sint x = (uint32)(point->Point.x*1000);
	sint y = (uint32)(point->Point.y*1000);
	float theta = (float)point->Angle;
	uint32 verticalPos;
	if (!parseVerticalPos(prim, verticalPos))
		verticalPos = DefaultBotVerticalPos;
	std::string effect, sheet_socle;
	std::vector<std::string>* sheet_spire;
//	if (!prim->getPropertyByName("effect", effect) || effect.empty())
//	{
//		nlwarning("No effect defined in spire %s", name.c_str());
//		return;
//	}
	if (!prim->getPropertyByName("sheet_socle", sheet_socle) || sheet_socle.empty())
		sheet_socle = DefaultBotLook;
	if (!prim->getPropertyByName("sheet_spire", sheet_spire) || !sheet_spire || sheet_spire->empty())
		sheet_spire = &EmptyStringVector;
	vector<string> sheets;
	sheets.insert(sheets.end(), "socle:"+sheet_socle);
	sheets.insert(sheets.end(), sheet_spire->begin(), sheet_spire->end());
	
	uint32 mgrAlias = 0;
	uint32 stateAlias = 0;
	uint32 grpAlias = uniqueId;
	uint32 botAlias = 0;
	
	// build the tree of aliases and the vector of folders
	std::vector<SFolderRef> folders;
	NLMISC::CSmartPtr<CAIAliasDescriptionNode> aliasTree = new CAIAliasDescriptionNode(name, uniqueId, AITypeManager, NULL);
	buildAliasTree(aliasTree, aliasTree, prim, folders);
	
	CAIActions::begin(mgrAlias); // mgr
	CAIActions::exec("SPIREMGR", mgrAlias, name, mapName, filename);
	CAIActions::exec("IDTREE", aliasTree);
	
	CAIActions::begin(stateAlias);
	CAIActions::exec("SPIRSTAT", stateAlias, name);
	
	CAIActions::begin(grpAlias); // grp
	CAIActions::exec("SPIREGRP", grpAlias, name);
	
	CAIActions::begin(botAlias); // bot
	CAIActions::exec("SPIREBOT", botAlias, name);
	CAIActions::exec("LOOK", sheet_socle);
	CAIActions::execute("SPIRSHTS", sheets);
	CAIActions::exec("STARTPOS", x, y, theta, verticalPos);
	CAIActions::end(botAlias); // bot
	
	// run through the mgr children looking for nodes with types that we recognise
	for (uint i=0; i<prim->getNumChildren(); ++i)
	{
		// get a pointer to the child and make sure its valid
		IPrimitive const* child;
		if (prim->getChild(child, i))
		{
			switch (nodeType(child))
			{
			case AITypeEvent:
				parsePrimEvent(NULL, child);
				break;
			default:
				nlwarning("unrecognised ai_type in spire %s: '%s'", name.c_str(), getName(nodeType(child)));
				break;
			}
		}
	}
	CAIActions::end(grpAlias); // grp
	CAIActions::end(stateAlias); // state
	CAIActions::end(mgrAlias); // mgr
	
	CAIAliasDescriptionNode::flushUnusedAliasDescription();
}

/////////////////////////////////////////////////////////
/////////////////// Dynamic system parsing //////////////
/////////////////////////////////////////////////////////

static void parsePrimDynFaunaZone(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	float x, y, r;
	x = prim->getPrimVector()->x;
	y = prim->getPrimVector()->y;
	string s;
	prim->getPropertyByName("radius", s);
	NLMISC::fromString(s, r);

	vector<string>	*params = &EmptyStringVector;	
	prim->getPropertyByName("properties", params);
	
	uint32 verticalPos;
	parseVerticalPos(prim, verticalPos);

	std::string concatStr;
	for (uint i = 0; i < params->size(); ++ i)
	{
		concatStr += (*params)[i] + " ";
	}
	
	//nlinfo("creating cell zone with flags : %s", concatStr.c_str());

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("DYNFZ", aliasNode, x, y, r, /*activities,*/ verticalPos);
	CAIActions::execute("ACT_PARM", *params);

	CAIActions::end(aliasNode->getAlias());
}
static void parsePrimDynNpcZonePlace(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	float x, y, r;
	x = prim->getPrimVector()->x;
	y = prim->getPrimVector()->y;
	string s;
	prim->getPropertyByName("radius", s);
	NLMISC::fromString(s, r);

	vector<string>	*params=&EmptyStringVector;
	prim->getPropertyByName("properties", params);
		
	uint32 verticalPos;
	parseVerticalPos(prim, verticalPos);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("DYNNZ", aliasNode, x, y, r, verticalPos);

	CAIActions::execute("DYNNZPRM", *params);
	
	CAIActions::end(aliasNode->getAlias());
}
static void parsePrimDynNpcZoneShape(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	uint32 verticalPos;
	parseVerticalPos(prim, verticalPos);
	
	// extract x and y coords of points from patate
	std::vector <CAIActions::CArg> args;
	args.push_back(aliasNode);
	args.push_back(verticalPos);
	uint numPoints=prim->getNumVector();
	if (numPoints!=0)
	{
		const CPrimVector *pointArray=prim->getPrimVector();
		for (uint i=0;i<numPoints;++i)
		{
			args.push_back(CAIActions::CArg(pointArray[i].x));
			args.push_back(CAIActions::CArg(pointArray[i].y));
		}
	}
	else
		LOG("Zone has no geometry"/*": %s: %s",pointsType,name.c_str()*/);
	
	vector<string>	*params=&EmptyStringVector;
	prim->getPropertyByName("properties", params);
	
	CAIActions::begin(aliasNode->getAlias());
	CAIActions::execute("DYNNZSHP", args);

	CAIActions::execute("DYNNZPRM", *params);
	
	CAIActions::end(aliasNode->getAlias());
}
static void parsePrimRoadTrigger(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	CPrimVector t1, t2, sp;
	float t1r = 0.f;
	float t2r = 0.f;
	float spr = 0.f;
	string s;

	for (uint i=0; i<prim->getNumChildren(); ++i)
	{
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			if (nodeName(child) == "trigger 1")
			{
				t1 = *child->getPrimVector();
				child->getPropertyByName("radius", s);
				NLMISC::fromString(s, t1r);
			}
			else if (nodeName(child) == "trigger 2")
			{
				t2 = *child->getPrimVector();
				child->getPropertyByName("radius", s);
				NLMISC::fromString(s, t2r);
			}
			else if (nodeName(child) == "spawn")
			{
				sp = *child->getPrimVector();
				child->getPropertyByName("radius", s);
				NLMISC::fromString(s, spr);
			}
		}
	}


	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("TRIGGER", aliasNode);

	CAIActions::exec("TRIGT1", t1.x, t1.y, t1r);
	CAIActions::exec("TRIGT2", t2.x, t2.y, t2r);
	CAIActions::exec("TRIGSP", sp.x, sp.y, spr);

//	TODO
//	set<string>	flags;
//	parseFamilyFlag(prim, flags);
//	vector<string> v(flags.begin(), flags.end());
//	CAIActions::execute("TRIGFLG", v);

	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimDynRoad(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// road dificulty
	string s;
	prim->getPropertyByName("difficulty", s);
	float difficulty;
	NLMISC::fromString(s, difficulty);

	uint32	verticalPos;
	parseVerticalPos(prim, verticalPos);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("DYNROAD", aliasNode, difficulty, verticalPos);

	// build polygon data
	vector<double>	poly;
	const CPrimVector *v = prim->getPrimVector();
	for (uint i=0; i<prim->getNumVector(); ++i)
	{
		poly.push_back(v[i].x);
		poly.push_back(v[i].y);
	}
	CAIActions::execute("ROADGEO", poly);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeRoadTrigger:
				parsePrimRoadTrigger(nextTreeNode(aliasNode,child),child);
				break;
			default:
				break;
			}
		}
	}
	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimGeomItems(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeDynFaunaZone:
				parsePrimDynFaunaZone(nextTreeNode(aliasNode,child),child);
				break;
			case AITypeDynNpcZonePlace:
				parsePrimDynNpcZonePlace(nextTreeNode(aliasNode,child),child);
				break;
			case AITypeDynNpcZoneShape:
				parsePrimDynNpcZoneShape(nextTreeNode(aliasNode,child),child);
				break;
			case AITypeDynRoad:
				parsePrimDynRoad(nextTreeNode(aliasNode,child),child);
				break;
			default:
				break;
			}
		}
	}
}

static void parsePrimCell(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

//	set<string>		flags;

	// read the family flags
//	TODO
//	parseFamilyFlag(prim, flags);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("CELL", aliasNode);
	// build the resulting vector of family/tribe string/
//	vector<string> vs(flags.begin(), flags.end());
//	CAIActions::execute("CELLFLG", vs);

	// build polygon data
	vector<double>	poly;
	const CPrimVector *v = prim->getPrimVector();
	for (uint i=0; i<prim->getNumVector(); ++i)
	{
		poly.push_back(v[i].x);
		poly.push_back(v[i].y);
	}
	CAIActions::execute("CELLGEO", poly);


	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			string cname = nodeClass(child);
			if (cname == "geom_items")
				parsePrimGeomItems(nextTreeNode(aliasNode,child),child);
		}
	}
	

	CAIActions::end(aliasNode->getAlias());
}


//void	parsePrimCellZoneEnergy(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
//{
//	std::string	family;
//	std::string	energy;
//	std::string	energy2;
//	std::string	energy3;
//	std::string	energy4;
//	prim->getPropertyByName("name", family);
//	prim->getPropertyByName("energy_0_25", energy);
//	prim->getPropertyByName("energy_25_50", energy2);
//	prim->getPropertyByName("energy_50_75", energy3);
//	prim->getPropertyByName("energy_75_100", energy4);
//	CAIActions::exec("CZ_NRJ", family, energy, energy2, energy3, energy4);
//}

static void parsePrimCellZone(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("CELLZNE", aliasNode);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeCell:
				parsePrimCell(nextTreeNode(aliasNode,child),child);
				break;
			default:
				break;
			}

		}

	}
	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimCellZones(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeCellZone:
				parsePrimCellZone(nextTreeNode(aliasNode,child),child);
				break;
			default:
				break;
			}
		}
	}
}

static void parsePrimBotTemplate(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim, std::string const& familyType)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	string s;
	vector<string>	*botEquip = &EmptyStringVector;
	string			lookSheet;
	bool			multiLevel = false;
	sint32			levelDelta = 0;
	
	if (nodeClass(prim) == "bot_template_npc")
	{
		prim->getPropertyByName("equipment", botEquip);
		prim->getPropertyByName("sheet_look", lookSheet);
	}
	else if (nodeClass(prim) == "bot_template_npc_ml")
	{
		prim->getPropertyByName("equipment", botEquip);
		prim->getPropertyByName("sheet_look", lookSheet);
		multiLevel = true;
		prim->getPropertyByName("level_delta", s);
		NLMISC::fromString(s, levelDelta);
	}
	else
	{
		prim->getPropertyByName("creature_code", lookSheet);
		if (!lookSheet.empty())
		{
			// the new code system replace the old sheet_carac or creature_type
//			lookSheet = lookSheet+".creature";
		}
		else
		{
			// read the old think
#ifdef NL_DEBUG
			nlassert(false);
#endif
			prim->getPropertyByName("creature_type", lookSheet);
//oldlevel 			lookSheet = lookSheet+"_lvl_"+toString("%02u", level)+".creature";
		}
	}

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("BOTTMPL"+familyType, aliasNode, lookSheet, multiLevel);	//, caracSheet);
	CAIActions::execute("BT_EQUI"+familyType, *botEquip);
	CAIActions::exec("BT_LVLD"+familyType,	levelDelta);

	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimGroupTemplate(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim, std::string const& familyType)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// read template parameters
	string s;
	uint32	botCount;
	vector<string>	*botEquip = &EmptyStringVector;
	string			lookSheet;
	string			caracSheet;
	vector<string>	*grpParam = &EmptyStringVector;
	bool			countMultipliedBySheet;
	bool			seasons[4];
	uint32			weights[4];
	uint32			grpEnergyValue;
	bool			multiLevel = false;
	sint32			levelDelta = 0;

	prim->getPropertyByName("count", s);
	NLMISC::fromString(s, botCount);

	prim->getPropertyByName("count_multiplied_by_sheet", s);
	countMultipliedBySheet = (s == "true");
	

	if (nodeClass(prim) == "group_template_npc")
	{
		// properties only for npc bots
		prim->getPropertyByName("bot_equipment", botEquip);
		prim->getPropertyByName("bot_sheet_look", lookSheet);
	}
	else if (nodeClass(prim) == "group_template_npc_ml")
	{
		// properties only for npc bots
		prim->getPropertyByName("bot_equipment", botEquip);
		prim->getPropertyByName("bot_sheet_look", lookSheet);
		multiLevel = true;
		prim->getPropertyByName("level_delta", s);
		NLMISC::fromString(s, levelDelta);
	}
	else
	{
		prim->getPropertyByName("creature_code", lookSheet);	//caracSheet);
		if (!lookSheet.empty())
		{
		}
		else
		{
			// read the old think
			prim->getPropertyByName("creature_type", lookSheet);
		}
	}
	prim->getPropertyByName("grp_parameters", grpParam);
	prim->getPropertyByName("exist_in_spring", s);
	seasons[0] = (s == "true");
	prim->getPropertyByName("exist_in_summer", s);
	seasons[1] = (s == "true");
	prim->getPropertyByName("exist_in_autumn", s);
	seasons[2] = (s == "true");
	prim->getPropertyByName("exist_in_winter", s);
	seasons[3] = (s == "true");
	
	prim->getPropertyByName("weight_0_25", s);
	NLMISC::fromString(s, weights[0]);
	prim->getPropertyByName("weight_25_50", s);
	NLMISC::fromString(s, weights[1]);
	prim->getPropertyByName("weight_50_75", s);
	NLMISC::fromString(s, weights[2]);
	prim->getPropertyByName("weight_75_100", s);
	NLMISC::fromString(s, weights[3]);

	vector<string>	*actParams = &EmptyStringVector;
	
	prim->getPropertyByName("properties", actParams);
	
	prim->getPropertyByName("total_energy_value", s);
	grpEnergyValue = (uint32)(ENERGY_SCALE*atof(s.c_str()));

	s = "ALWAYS";
	prim->getPropertyByName("spawn_type",s);
	TSpawnType st;
	getType(st, s.c_str());
	uint32 spawnType = st;

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("GRPTMPL", aliasNode, 
		CurrentGroupFamily, 
		botCount,
		countMultipliedBySheet,
		multiLevel
		);
	if ( nodeClass(prim) == "squad_template_variant" )
	{
		vector<string> *botSheets;
		if ( ! (prim->getPropertyByName("bot_sheets", botSheets) && botSheets) )
			nlerror( "Missing property bot_sheets in squad %s", name.c_str() );

		CAIActions::execute("IDTREE_F", *botSheets); // create the bot descs as well (group descs created in exec("GRPTMPL"))
	}
	CAIActions::exec("GT_SHEE"+familyType,	lookSheet);
	CAIActions::exec("GT_LVLD"+familyType,	levelDelta);
	CAIActions::exec("GT_SEAS"+familyType, seasons[0], seasons[1], seasons[2], seasons[3]);
	CAIActions::exec("GT_NRG"+familyType, weights[0], weights[1], weights[2], weights[3]);
	
	if (grpEnergyValue!=0)	// means not initialized.
		CAIActions::exec("GT_GNRJ"+familyType, grpEnergyValue);
	CAIActions::exec("GT_ACT"+familyType, /*activity,*/ spawnType);
	CAIActions::execute("GT_APRM"+familyType, *actParams);

	CAIActions::execute("GT_EQUI"+familyType, *botEquip);
	CAIActions::execute("GT_GPRM"+familyType, *grpParam);

	std::vector <CAIActions::CArg> executeArgs;

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeBotTemplate:
			case AITypeBotTemplateMultiLevel:
				parsePrimBotTemplate(nextTreeNode(aliasNode,child), child, familyType);
				break;
			case AITypeFaunaSpawnAtom:
				{

					try
					{
						std::string		theSheet;
						uint			count;
						parsePopulation	(child, theSheet, count);
						executeArgs.push_back(CAIActions::CArg(theSheet));
						executeArgs.push_back(count);
					}
					catch (const parsePopException &e)
					{
						nlwarning("FaunaGroup: %s of %s : %s", nodeName(child).c_str(), aliasNode->fullName().c_str(), e.what());
					}
					
				}
				break;
			default:
				break;
			}

		}

	}

	if (!executeArgs.empty())
		CAIActions::execute("POPVER"+familyType,executeArgs);

	CAIActions::execute("GT_END"+familyType);

	CAIActions::end(aliasNode->getAlias());
}


static	void	parsePrimGroupFamilyProfileFaunaContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	CAIActions::execute("TMPPRFF");
	
	const vector<string>	*foodParams = &EmptyStringVector;
	prim->getPropertyByName("food", foodParams);
	CAIActions::execute("TMPPRFFF", *foodParams);

	const vector<string>	*restParams = &EmptyStringVector;
	prim->getPropertyByName("rest", restParams);
	CAIActions::execute("TMPPRFFR", *restParams);

	std::string	energy;
	std::string	energy2;
	std::string	energy3;
	std::string	energy4;
	prim->getPropertyByName("energy_0_25", energy);
	prim->getPropertyByName("energy_25_50", energy2);
	prim->getPropertyByName("energy_50_75", energy3);
	prim->getPropertyByName("energy_75_100", energy4);
	CAIActions::exec("CZ_NRJ", energy, energy2, energy3, energy4);
	
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeGroupTemplateFauna:
				parsePrimGroupTemplate(nextTreeNode(aliasNode,child),child,"C");
				break;
			default:
				break;
			}
			
		}
		
	}
}

static	void	parsePrimGroupFamilyProfileFauna(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);

	string	familyTag;
	CAIActions::exec("GRPFAM", aliasNode, familyTag);
	parsePrimGroupFamilyProfileFaunaContent(aliasNode, prim);	
}


static	void	parsePrimGroupFamilyProfileTribeContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	std::string	aggro_groups;
	prim->getPropertyByName("aggro_groups", aggro_groups);
	
	CAIActions::exec("TMPPRFT", aggro_groups);

	std::string	energy;
	std::string	energy2;
	std::string	energy3;
	std::string	energy4;
	prim->getPropertyByName("energy_0_25", energy);
	prim->getPropertyByName("energy_25_50", energy2);
	prim->getPropertyByName("energy_50_75", energy3);
	prim->getPropertyByName("energy_75_100", energy4);
	CAIActions::exec("CZ_NRJ", energy, energy2, energy3, energy4);

	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeGroupTemplate:
			case AITypeGroupTemplateMultiLevel:
				parsePrimGroupTemplate(nextTreeNode(aliasNode,child),child,"C");
				break;
			default:
				break;
			}
			
		}
		
	}
}

static	void	parsePrimGroupFamilyProfileNpcContent(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{	
	CAIActions::execute("TMPPRFN");


	const vector<string>	*flagList = &EmptyStringVector;
	prim->getPropertyByName("flags", flagList);	
	CAIActions::execute("TMPPRFNF", *flagList);

	std::string	energy;
	std::string	energy2;
	std::string	energy3;
	std::string	energy4;
	prim->getPropertyByName("energy_0_25", energy);
	prim->getPropertyByName("energy_25_50", energy2);
	prim->getPropertyByName("energy_50_75", energy3);
	prim->getPropertyByName("energy_75_100", energy4);
	CAIActions::exec("CZ_NRJ", energy, energy2, energy3, energy4);

	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeGroupTemplate:
			case AITypeGroupTemplateMultiLevel:
				parsePrimGroupTemplate(nextTreeNode(aliasNode,child),child,"C");
				break;
			default:
				break;
			}
			
		}
		
	}
}

static	void	parsePrimGroupFamilyProfileTribe(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);

	string	familyTag;
	prim->getPropertyByName("family", familyTag);
	
	CAIActions::exec("GRPFAM", aliasNode, familyTag);
	parsePrimGroupFamilyProfileTribeContent(aliasNode, prim);
}

static	void	parsePrimGroupFamilyProfileNpc(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	string	familyTag;	
	
	CAIActions::exec("GRPFAM", aliasNode, familyTag);
	parsePrimGroupFamilyProfileNpcContent(aliasNode, prim);
}

static void parsePrimGroupDescriptions(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if	(prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case	AITypeGroupFamily:
				{
					nlwarning("Parsing a group_family, primitive is outdated. Please report to jvuarand.");
				//	parsePrimGroupFamily(nextTreeNode(aliasNode,child),child);
				}
				break;
				
			case AITypeGroupFamilyProfileFauna:
				parsePrimGroupFamilyProfileFauna(nextTreeNode(aliasNode,child),child);
//				parsePrimGroupFamilyProfileGeneric(nextTreeNode(aliasNode,child),child, GroupFamilyFauna);
				break;
				
			case AITypeGroupFamilyProfileTribe:
				parsePrimGroupFamilyProfileTribe(nextTreeNode(aliasNode,child),child);
				break;

			case AITypeGroupFamilyProfileNpc:
				parsePrimGroupFamilyProfileNpc(nextTreeNode(aliasNode,child),child);
				break;
				
//			case AITypeGroupFamilyProfileGeneric:
//				parsePrimGroupFamilyProfileGeneric(nextTreeNode(aliasNode,child),child, GroupFamilyTribe);
//				break;			
			default:
				break;
			}
		}
	}
}

static void parsePrimDynRegion(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim, const std::string &filename)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("DYNREG",aliasNode, filename);
	CAIActions::exec("IDTREE",aliasNode);

	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			string cname = nodeClass(child);

			if (cname == "cell_zones")
				parsePrimCellZones(nextTreeNode(aliasNode,child),child);
			else if (cname == "group_descriptions")
				parsePrimGroupDescriptions(nextTreeNode(aliasNode,child),child);
		}
	}

	CAIActions::end(aliasNode->getAlias());
}
/*
static void parsePrimOutpostCharge(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	string civilisation;
	const vector<string>	*params = &EmptyStringVector;

	prim->getPropertyByName("civilisation", civilisation);
	prim->getPropertyByName("parameters", params);


	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("CHARGE", aliasNode, civilisation);

	CAIActions::execute("CHGPARM", *params);

	CAIActions::end(aliasNode->getAlias());
}
*/
//static void parsePrimOutpostSquadFamily(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
//{
//	// get hold of the node name and unique id
//	std::string name = nodeName(prim);
////	uint32 uniqueId = nodeAlias(prim);
//	/*
//	string civilisation;
//	const vector<string>	*params = &EmptyStringVector;
//	
//	prim->getPropertyByName("civilisation", civilisation);
//	prim->getPropertyByName("parameters", params);
//	*/
//	CAIActions::begin(aliasNode->getAlias());
//	CAIActions::exec("SQUADFAM", aliasNode);
////	CAIActions::exec("IDTREE",aliasNode);
//	/*
//	
//	CAIActions::execute("CHGPARM", *params);
//	
//	*/
//	for (uint i=0;i<prim->getNumChildren();++i)	
//	{
//		// get a pointer to the child and make sure its valid
//		const IPrimitive *child;
//		if	(prim->getChild(child, i))
//		{
//			switch(nodeType(child))
//			{
//			case AITypeGroupTemplate:
//			case AITypeGroupTemplateMultiLevel:
//				parsePrimGroupTemplate(nextTreeNode(aliasNode,child),child,"O");
//				break;
//			}
//			
//		}
//		
//	}
//	
//	CAIActions::end(aliasNode->getAlias());
//}

static void parsePrimOutpostSpawnZone(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim)
{
	float x, y, r;
	x = prim->getPrimVector()->x;
	y = prim->getPrimVector()->y;
	string s;
	prim->getPropertyByName("radius", s);
	NLMISC::fromString(s, r);
	
	uint32 verticalPos;
	parseVerticalPos(prim, verticalPos);
	
	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("SPWNZONE", aliasNode, x, y, r, verticalPos);
	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimOutpostBuilding(CAIAliasDescriptionNode const* aliasNode, IPrimitive const* prim)
{
	string name = nodeName(prim);
	uint32 uniqueId = nodeAlias(prim);
	float x, y, theta;
	const CPrimPoint *point=dynamic_cast<CPrimPoint const*>(prim);
	if (point==NULL)
	{
		nlwarning("Failed to cast to CPrimPoin bot: %s",name.c_str());
		return;
	}
	x = point->Point.x;
	y = point->Point.y;
	theta = point->Angle;
//	x = prim->getPrimVector()->x;
//	y = prim->getPrimVector()->y;
//	string s;
//	prim->getPropertyByName("radius", s);
//	r = float(atof(s.c_str()));
	
	uint32 verticalPos;
	parseVerticalPos(prim, verticalPos);
	bool isStuck = true;
	
	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("BUILDING", uniqueId);
	CAIActions::exec("STARTPOS", (sint32)(x*1000.f), (sint32)(y*1000.f), theta, verticalPos);
//	CAIActions::exec("LOOK", "");
	CAIActions::exec("ISSTUCK", uint32(isStuck));
	CAIActions::exec("BLDNGBOT", uint32(true));
	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimBotTemplate(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim, std::string const& familyType);

static void parsePrimSquadTemplateVariant(const CAIAliasDescriptionNode *aliasNode, const IPrimitive *prim, const std::string& templateName)
{
	// get hold of the node name
	std::string name = nodeName(prim);

	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("SQD_T_V",templateName,name);
	parsePrimGroupTemplate(aliasNode,prim,"O");

	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimSquadTemplate(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{
	// get hold of the node name and unique id
	string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);

	// build the tree of aliases and the vector of folders
	std::vector<SFolderRef> folders;
	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	aliasNode = new CAIAliasDescriptionNode	(name, uniqueId, AITypeOutpost, NULL);
	buildAliasTree(aliasNode, aliasNode, prim, folders);
	
	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("SQD_TMPL", aliasNode, filename);

	// run through the children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeSquadTemplateVariant:
				parsePrimSquadTemplateVariant(nextTreeNode(aliasNode,child), child, name);
				break;
			default:
				break;
			}
		}
	}

	CAIActions::end(aliasNode->getAlias());	
}

static void parsePrimOutpost(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{
	// get hold of the node name and unique id
	string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	string familyName;
	string continent, s;
	stringToKeywordAndTail(mapName, continent, s);
	
	prim->getPropertyByName("owner_tribe", familyName);

	// build the tree of aliases and the vector of folders
	std::vector<SFolderRef> folders;
	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	aliasNode = new CAIAliasDescriptionNode	(name, uniqueId, AITypeOutpost, NULL);
	buildAliasTree(aliasNode, aliasNode, prim, folders);
	
	CAIActions::begin(aliasNode->getAlias());
	
	// outpost general properties
	CAIActions::exec("OUTPOST", aliasNode, continent, filename, familyName);

	// link squads
	const char* props[] = { "tribe_squads", "tribe_squads2", "default_squads", "buyable_squads" };
	size_t nprops = sizeof(props)/sizeof(props[0]);
	for (size_t i=0; i!=nprops; ++i)
	{
		vector<string>* propSquads;
		if (prim->getPropertyByName(props[i], propSquads) && propSquads)
		{
			vector<string> squadsToLink;
			// Insert default variant at beginning of vector
			squadsToLink.push_back(continent);
			squadsToLink.insert( squadsToLink.end(), (*propSquads).begin(), (*propSquads).end() );
			CAIActions::execute("OUTP_SQD", squadsToLink );
		}
		else
			nlerror( "Missing property %s in %s in %s", props[i], name.c_str(), filename.c_str() );
	}
	
	CAIActions::exec("IDTREE",aliasNode);
	
	// build polygon data
	vector<double>	poly;
	const CPrimVector *v = prim->getPrimVector();
	for (uint i=0; i<prim->getNumVector(); ++i)
	{
		poly.push_back(v[i].x);
		poly.push_back(v[i].y);
	}
	CAIActions::execute("OUTPOGEO", poly);
	
	// run through the children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
//			case AITypeOutpostSquadFamily:
//				parsePrimOutpostSquadFamily(nextTreeNode(aliasNode,child), child);
//				break;
			case AITypeOutpostSpawnZone:
				parsePrimOutpostSpawnZone(nextTreeNode(aliasNode,child), child);
				break;
			case AITypeOutpostBuilding:
				parsePrimOutpostBuilding(nextTreeNode(aliasNode,child), child);
				break;
			case AITypeManager:
				parsePrimMgr(child, mapName, filename);
				break;
			default:
				break;
			}
		}
	}


	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimDynSystem(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{
	// get hold of the node name and unique id
	std::string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	
	string contName;
	prim->getPropertyByName("continent_name", contName);
	
	nlassertex(!contName.empty(), ("Error while loading dynamic system from '%s', the continent name is empty !", filename.c_str()));
	// build the tree of aliases and the vector of folders
	std::vector<SFolderRef> folders;
	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	aliasNode = new CAIAliasDescriptionNode	(name, uniqueId, AITypeDynamicSystem, NULL);
	buildAliasTree(aliasNode, aliasNode, prim, folders);
	
	CAIActions::begin(aliasNode->getAlias());
	CAIActions::exec("DYNSYS", contName, mapName);
	
	// run through the dynsystem children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch(nodeType(child))
			{
			case AITypeDynamicRegion:
				parsePrimDynRegion(nextTreeNode(aliasNode,child), child, filename);
				break;
			case AITypeOutpost:
				parsePrimOutpost(child, mapName, filename);
				break;
			default:
				break;
			}
		}
	}
	
	CAIActions::exec("DYN_END");
	CAIActions::end(aliasNode->getAlias());
}


static void parsePrimNogoPointList(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{
	// get hold of the node name and unique id
	string name=nodeName(prim);
	uint32 uniqueId=nodeAlias(prim);
	NLMISC::CSmartPtr<CAIAliasDescriptionNode>	aliasNode = new CAIAliasDescriptionNode	(name, uniqueId, AITypeOutpost, NULL);
//
	CAIActions::begin(aliasNode->getAlias());

	// run through the charge children looking for nodes with types that we recognise
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		// get a pointer to the child and make sure its valid
		const IPrimitive *child;
		if (prim->getChild(child, i))
		{
			switch	(nodeType(child))
			{
			case AITypeNogoPoint: // OutpostCharge:
				{
					float x=(float)(child->getPrimVector()->x);
					float y=(float)(child->getPrimVector()->y);
					CAIActions::exec("SETNOGO", x, y);
				}
				break;
			default:
				break;
			}

		}

	}
	CAIActions::end(aliasNode->getAlias());
}

static void parsePrimSafeZone(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{	
	float x=(float)(prim->getPrimVector()->x);
	float y=(float)(prim->getPrimVector()->y);
	string	radiusString;
	prim->getPropertyByName("radius",radiusString);
	float radius;
	NLMISC::fromString(radiusString, radius);
	
	CAIActions::exec("SAFEZONE", x, y, radius);
}


static void parsePrimScript(const IPrimitive *prim, const std::string &mapName, const std::string &filename)
{
	string	primName;
	const std::vector<std::string>* pcode = NULL;
	prim->getPropertyByName("name", primName);
	prim->getPropertyByName("code", pcode);
	std::string code;
	// Concat the strings inside a single one...
	if (pcode!=NULL && !pcode->empty())
	{
		std::vector<std::string>::const_iterator it = pcode->begin(), itEnd = pcode->end();
		code = *it;
		++it;
		for(; it!=itEnd; ++it)
		{
			code += "\n";
			code += *it;
		}
	}
	// ... coz CAIActions::exec cannot handle a vector<string>
	CAIActions::exec("SCRIPT", primName, code);
}


/**
* parse a user model
* a user model contains an id, a base SheetId (used later to fill unmodified attributes of the dynamic sheet)
* and a script defining all attributes to be modified along with the new values
*/
static void parseUserModelListRec(const IPrimitive				*prim, 
								  const std::string				&mapName, 
								  const std::string				&filename, 
								  std::vector<CAIActions::CArg> &args)
{
	AITYPES::TAIType type = nodeType(prim);
	if (type == AITypeUserModelList)
	{
		// run through the list of children 
		for (uint j = 0; j<prim->getNumChildren(); ++j)	
		{
			// get a pointer to the child and make sure its valid
			const IPrimitive *child;
			if (!prim->getChild(child,j))
				continue;
			
			// try to get a type for the child
			TAIType type=nodeType(child);
			
			if (type == AITypeUserModel)
			{
				std::string userModelId;
				std::string script = "";
				std::string baseSheet;
				const std::vector<std::string> *pcode = NULL;
				child->getPropertyByName("name", userModelId);
				child->getPropertyByName("script", pcode);
				child->getPropertyByName("sheet_client", baseSheet);
				if (pcode != NULL && !pcode->empty())
				{
					std::vector<std::string>::const_iterator start = pcode->begin();
					for(std::vector<std::string>::const_iterator it = start; it != pcode->end(); ++it)
					{
						if (it != start)
						{
							script += "\n";
						}
						script += *it;
					}
				}
				args.push_back(CAIActions::CArg(userModelId));
				args.push_back(CAIActions::CArg(baseSheet));
				args.push_back(CAIActions::CArg(script));
				nldebug("<ParsePrimUserModelList> Add user model '%s'", userModelId.c_str());
			}
		}
	}
	else
	{
		for (uint i=0;i<prim->getNumChildren();++i)	
		{
			const IPrimitive *child;
			if (prim->getChild(child,i))
				parseUserModelListRec(child, mapName, filename, args);
		}
	}
	
}

/**
* Parse method called on the main user model node, which is a list of user models
* builds a CArgs vector and executes an AiAction USR_MDL
* This aiAction is defined in the CAIUserModelManager class
*/
static void parsePrimUserModelList(const IPrimitive		*prim,
								   const std::string	&mapName,
								   const std::string	&filename,
								   uint32				primAlias)
{
	std::vector<CAIActions::CArg> args;
	args.push_back(CAIActions::CArg(primAlias));
	parseUserModelListRec(prim, mapName, filename, args);
	if (args.size() == 0)
	{
		nlinfo("<ParsePrimUserModelList> No User Model List found in primitive '%s'", filename.c_str());
		return;
	}
	CAIActions::execute("USR_MDL", args);
}

/**
* Method called to parse a custom loot table.
* A custom loot table contains an id and money values, and a list of custom loot sets.
* A custom loot set contains a drop probability and a script defining what item it will drop
* (item name, quantity, quality)
*/
static void parseCustomLootTableRec(const IPrimitive			*prim, 
								  const std::string				&mapName, 
								  const std::string				&filename, 
								  std::vector<CAIActions::CArg> &args)
{
	AITYPES::TAIType type = nodeType(prim);
	if (type == AITypeCustomLootTable)
	{
		//first push number of loot sets in the current loot table
		uint size = prim->getNumChildren();
		args.push_back(CAIActions::CArg(size));

		//then push loot table info
		std::string customTableId;
		std::string strMoneyBase;
		std::string strMoneyFactor;
		std::string strMoneyProba;

		prim->getPropertyByName("name", customTableId);
		prim->getPropertyByName("money_base", strMoneyBase);
		prim->getPropertyByName("money_factor", strMoneyFactor);
		prim->getPropertyByName("money_proba", strMoneyProba);
		
		args.push_back(CAIActions::CArg(customTableId));

		char *ptr = NULL;

		float moneyProba = static_cast<float>(strtod(strMoneyProba.c_str(), &ptr));
		if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
		{
			args.push_back(CAIActions::CArg(moneyProba));
		}

		float moneyFactor = static_cast<float>(strtod(strMoneyFactor.c_str(), &ptr));
		if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
		{
			args.push_back(CAIActions::CArg(moneyFactor));
		}

		uint32 moneyBase = static_cast<uint32>(strtol(strMoneyBase.c_str(), &ptr, 10));
		if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
		{
			args.push_back(CAIActions::CArg(moneyBase));
		}
	
		// run through the list of children 
		for (uint j = 0; j<prim->getNumChildren(); ++j)	
		{
			// get a pointer to the child and make sure its valid
			const IPrimitive *child;
			if (!prim->getChild(child,j))
				continue;
			
			// try to get a type for the child
			TAIType type=nodeType(child);
			
			if (type == AITypeCustomLootSet)
			{
				std::string dropProba;
				std::string script = "";

				const std::vector<std::string> *pcode = NULL;
				child->getPropertyByName("drop_proba", dropProba);
				child->getPropertyByName("script", pcode);
				if (pcode != NULL && !pcode->empty())
				{
					std::vector<std::string>::const_iterator start = pcode->begin();
					for(std::vector<std::string>::const_iterator it = start; it != pcode->end(); ++it)
					{
						if (it != start)
						{
							script += "\n";
						}
						script += *it;
					}
				}
				args.push_back(CAIActions::CArg(dropProba));
				args.push_back(CAIActions::CArg(script));
			
			}
		}
		nldebug("<ParseCustomLootTable> Add custom loot table'%s'", customTableId.c_str());
	}
	else
	{
		for (uint i=0;i<prim->getNumChildren();++i)	
		{
			const IPrimitive *child;
			if (prim->getChild(child,i))
				parseCustomLootTableRec(child, mapName, filename, args);
		}
	}
}

/**
* Main parsing method used to build the CArgs vector containing the custom loot tables data defined in the primitive.
* Called on the main node which is a list of custom loot tables
* Once the parsing is done and the CArgs vector is built, an AiAction CUSTOMLT (CustomLootTables) is executed.
* This aiAction is defined in the CAIUserModelManager class
*/
static void parsePrimCustomLootTable(const IPrimitive	*prim,
									 const std::string	&mapName,
									 const std::string	&filename,
									 uint32				primAlias)
{
	AITYPES::TAIType type = nodeType(prim);
	if (type == AITypeCustomLootTables)
	{
		uint size = prim->getNumChildren();

		if (size == 0)
		{
			nlinfo("<ParsePrimCustomLootTable> Custom loot tables folder declared but empty in primitive '%s'", filename.c_str());
			return;
		}
		
		std::vector<CAIActions::CArg> args;

		args.push_back(CAIActions::CArg(size));
		args.push_back(CAIActions::CArg(primAlias));
	
		//TODO: add the primitive aliasStaticPart in the vector in order to remove all userModels/custom loot table
		//from AIS and EGS managers when a primitive is unloaded
		
		
		//args.push_back(CAIActions::CArg(nodeAlias(prim)));

		parseCustomLootTableRec(prim, mapName, filename, args);

		CAIActions::execute("CUSTOMLT", args);
	}
	else
	{
		for (uint i=0;i<prim->getNumChildren();++i)	
		{
			const IPrimitive *child;
			if (prim->getChild(child,i))
				parsePrimCustomLootTable(child, mapName, filename, primAlias);
		}
	}
}

static void parsePrim(const IPrimitive *prim,const std::string &mapName, const std::string &filename)
{
	AITYPES::TAIType type = nodeType(prim);

	switch(type)
	{
	case AITypeManager:
		{
			// we're clear to parse so go ahead!
			parsePrimMgr(prim, mapName, filename);
		}
		break;
	case AITypeDynamicSystem:
		{
			// we're clear to parse so go ahead!
			parsePrimDynSystem(prim, mapName, filename);
		}
		break;
	case AITypeSquadTemplate:
		{
			parsePrimSquadTemplate(prim, mapName, filename);
		}
		break;
	case AITypeNogoPointList:
		{
			parsePrimNogoPointList(prim, mapName, filename);
		}
		break;
	case AITypeSafeZone:
		{
			parsePrimSafeZone(prim, mapName, filename);
		}
		break;
	case AITypeScript:
		{
			parsePrimScript(prim, mapName, filename);
		}
		break;
	case AITypeSpire:
		{
			parsePrimSpire(prim, mapName, filename);
		}
		break;
	default:
		{
			// this node's not a manager so checkout children
			for (uint i=0;i<prim->getNumChildren();++i)	
			{
				const IPrimitive *child;
				if (prim->getChild(child,i))
					parsePrim(child, mapName, filename);
			}
		}
	}
}


//---------------------------------------------------------------------------------------
// utility routines for primitive file list management

struct TFileInfo
{
	std::string FileName;
	std::string MapName;
//	uint32 FirstSlot;
};

const uint32 HighestSlotId=1023;

//void getFileSlotVector(std::vector <TFileInfo> &vect)
//{
//	CPrimitiveCfg::readPrimitiveCfg();
//
//	const std::vector<std::string> &mapNames = CPrimitiveCfg::getMapNames();
//	std::vector<std::string>::const_iterator first(mapNames.begin()), last(mapNames.end());
//	for (; first != last; ++first)
//	{
//		const std::vector<std::string> &primitives = CPrimitiveCfg::getMap(*first);
//		std::vector<std::string>::const_iterator first2(primitives.begin()), last2(primitives.end());
//		for (; first2 != last2; ++first2)
//		{
//			TFileInfo fi;
//			fi.MapName = *first;
//			fi.FileName = *first2;
//			vect.push_back(fi);
//		}
//	}
//
//}


void parsePrimStream(NLMISC::IStream & stream, const std::string & streamName)
{
	H_AUTO( parsePrimStream );

	std::string filename = streamName;
	nlinfo("begin parsing prim stream: %s", filename.c_str());

	// normalise the file name
//	std::string fileName=lookupFileName(std::string(filename));
	if (filename.empty())
	{
		nlwarning( "parse primitive stream called with empty name" );
		return;
	}

	CPrimitives* primDoc = new CPrimitives();
	try
	{
		{
			//stream to PrimDoc
			H_AUTO( streamToPrimDoc );
			CPrimitiveContext::instance().CurrentPrimitive = primDoc;
			primDoc->serial(stream);
			CPrimitiveContext::instance().CurrentPrimitive = NULL;
		}
	}
	catch(...)
	{
		nlwarning("stream error");
		return;
	}

	parsePrimNoStream( primDoc, streamName );

	delete primDoc;
}

void parsePrimNoStream( CPrimitives* primDoc, const std::string & streamName )
{
	H_AUTO( parsePrimNoStream );
	using namespace AI_SHARE;
	using namespace NLMISC;

	// clear out the alias maps
	MapPrimToAlias.clear();
	MapAliasToPrim.clear();

	if( s_WriteScenarioDebugDataToFile )	// debug
	{
		nldebug( "writing test2.primitive" );
		saveXmlPrimitiveFile(*primDoc, "test2.primitive");
	}

	// Call init() !!
	nlassert (LigoConfig != NULL);

	CPrimitiveContext::instance().CurrentPrimitive = primDoc;

	//prim to AIAction	
	// initialise the action executor
	CAIActions::openFile(streamName);	// AJM: note this does not open a file or stream

	// do the real parsing work		
	{
		uint32 primAlias = primDoc->getAliasStaticPart();
		H_AUTO(parsePrimUserModelList);
		
		//first parse for user models
		parsePrimUserModelList((IPrimitive *)primDoc->RootNode,
			CPrimitiveCfg::getContinentNameOf(streamName)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(streamName)),
			CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(streamName))),
			primAlias);
		
		//then parse for custom loot table
		H_AUTO(parsePrimCustomLootTable);
		parsePrimCustomLootTable((IPrimitive *)primDoc->RootNode,
			CPrimitiveCfg::getContinentNameOf(streamName)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(streamName)), 
			CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(streamName))),
			primAlias);
		H_AUTO(parsePrim);
		//then do the real parsing work
		parsePrim((IPrimitive *)primDoc->RootNode,CPrimitiveCfg::getContinentNameOf(streamName)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(streamName)), CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(streamName))));
	}

	CPrimitiveContext::instance().CurrentPrimitive = NULL;

	// allow for action executor housekeeping
	CAIActions::closeFile(streamName);	// AJM: note this does not close a file or a stream
}

//---------------------------------------------------------------------------------------
// the main primitive file parser routine
void parsePrimFile(const std::string &filename)
{
	H_AUTO(parsePrimFile);
	
	nldebug("PARSING PRIM FILE %s", filename.c_str());
	// Call init() !!
	nlassert (LigoConfig != NULL);

	// clear out the alias maps
	MapPrimToAlias.clear();
	MapAliasToPrim.clear();

	// normalise the file name
//	std::string fileName=lookupFileName(std::string(filename));
	if (filename.empty())
		return;

	// read the file into memory and parse to generate 'prims' data tree
	CPrimitives prims;

	string sFilename;
	bool opened;
	CIFile fileIn;
	if (fileIn.open (filename))
	{
		nlinfo("Opening %s in %s", filename.c_str(), CPath::getCurrentPath().c_str());
		sFilename = filename;
		opened = true;
	}
	else
	{
		// If file not found, search in path
		sFilename = CPath::lookup( filename, false);
		nlinfo("Opening in %s", filename.c_str());
		opened = (fileIn.open (sFilename.c_str()));
	}
	
	nlinfo("Load&parse primitive file '%s' (in '%s')", 
		CFile::getFilename(filename).c_str(), 
		sFilename.c_str());

	// Test if binary caching is wanted
	bool cachePrims = true;
	CConfigFile::CVar *cachePrimsVar = IService::getInstance()->ConfigFile.getVarPtr("CachePrims");
	if (cachePrimsVar)
	{
		cachePrims = cachePrimsVar->asInt() != 0;
	}	
	bool cachePrimsLog = false;
	CConfigFile::CVar *cachePrimsLogVar = IService::getInstance()->ConfigFile.getVarPtr("CachePrimsLog");
	if (cachePrimsLogVar)
	{
		cachePrimsLog = cachePrimsLogVar->asInt() != 0;
	}

	if ( opened )
	{
		// lookup in binary cache before reading in XML
		bool	readXml = true;
		string binFileName = NLNET::IService::getInstance()->WriteFilesDirectory.toString() +"primitive_cache/"+CFile::getFilename(filename)+".binprim";
		if (cachePrims
			&& CFile::fileExists(binFileName)
			&& CFile::getFileModificationDate(binFileName) > CFile::getFileModificationDate(sFilename))
		{
			if (cachePrimsLog)
			{
				// ok, the cache is here and up to date !
				nlinfo("Loading '%s' from binary file '%s'",
					sFilename.c_str(),
					binFileName.c_str());
			}
			try
			{
				CIFile binFile(binFileName);
				CPrimitiveContext::instance().CurrentPrimitive = &prims;
				prims.serial(binFile);
				CPrimitiveContext::instance().CurrentPrimitive = NULL;
				// ok, all was fine, don't read in xml !
				readXml = false;
			}
			catch(...)
			{}
		}
		if (readXml)
		{
			// Xml stream
			CIXml xmlIn;
			xmlIn.init (fileIn);

			// set the primitive context
			CPrimitiveContext::instance().CurrentPrimitive = &prims;
			// Read it
			if (!prims.read (xmlIn.getRootNode (), sFilename.c_str(), *LigoConfig))
			{
				nlwarning ("Error reading file %s", sFilename.c_str());
				return;
			}
			// clean the context
			CPrimitiveContext::instance().CurrentPrimitive = NULL;

			if (cachePrims)
			{
				// save a binary version
				CFile::createDirectory(IService::getInstance()->WriteFilesDirectory.toString()+"primitive_cache");
				COFile saveBin(binFileName);
				prims.serial(saveBin);
			}
		}
	}
	else
	{
		// if file not found sFilename is the result of CPath::lookup so if not in path sFilename is an empty string
		nlwarning ("Failed to open file '%s'  for reading.", sFilename.empty() ? filename.c_str():sFilename.c_str());

		return;
	}

	// initialise the action executor
	CAIActions::openFile(sFilename);

	CPrimitiveContext::instance().CurrentPrimitive = &prims;
	// do the real parsing work
	{
		uint32 primAlias = prims.getAliasStaticPart();
		H_AUTO(parsePrimUserModelList);
		
		//first parse for user models
		parsePrimUserModelList((IPrimitive *)prims.RootNode,
			CPrimitiveCfg::getContinentNameOf(filename)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename)),
			CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename))),
			primAlias);
		
		//then parse for custom loot table
		H_AUTO(parsePrimCustomLootTable);
		parsePrimCustomLootTable((IPrimitive *)prims.RootNode,
			CPrimitiveCfg::getContinentNameOf(filename)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename)), 
			CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename))),
			primAlias);
	H_AUTO(parsePrim);
	parsePrim((IPrimitive *)prims.RootNode,CPrimitiveCfg::getContinentNameOf(filename)+":"+CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename)), CFile::getFilename(CFile::getFilenameWithoutExtension(CFile::getFilename(sFilename))));
	}

	CPrimitiveContext::instance().CurrentPrimitive = NULL;

	// allow for action executor housekeeping
	CAIActions::closeFile(sFilename);
}


//---------------------------------------------------------------------------------------

NLMISC_COMMAND(loadPdrFile,"load a primitive file. don't forget to call BuildPrimitiveDependencies","<file name>")
{
	if	(args.size()!=1)
		return	false;
	AI_SHARE::CAIActionsDataRecord pdr;
	pdr.readFile(args[0]);
	CAIActions::IExecutor* executer;
	executer = CAIActions::getExecuter();
	if (!executer)
	{
		nlwarning("no executer");
		return false;
	}
	pdr.applyToExecutor(*executer);
	return true;

}


NLMISC_COMMAND(loadPrimitiveFile,"load a primitive file. don't forget to call BuildPrimitiveDependencies","<file name>")
{
	if	(args.size()!=1)
		return	false;

	if (NLMISC::CFile::getExtension(args[0]).empty())
		parsePrimFile(args[0]+".primitive");
	else
		parsePrimFile(args[0]);

	return true;
}

//	have to do this on managers coz there no more implicit correspondence ..

NLMISC_COMMAND(loadMapsFromCommon,"load all primitive defined in usedPrimitives in common.cfg for the given map","<map name>")
{
	if	(args.size()!=1)
		return	false;

	CPrimitiveCfg::readPrimitiveCfg();

	// We create a static set of 'loaded primitive file' names via this method in order to avoid loading the same primitive file more than once at AIS startup
	static std::set<std::string> loadedPrimitives;

	std::vector<std::string>	mapConfigNames;
	// load only active primitive maps...
	{
		CConfigFile::CVar& usedPrimitives = IService::getInstance()->ConfigFile.getVar("UsedPrimitives");

		const vector<string>	&basePrim = CPrimitiveCfg::getMap(args[0]);
		set<string> filter(basePrim.begin(), basePrim.end());
		for ( uint i = 0; i < usedPrimitives.size(); ++i)
		{
			const vector<string> &prims = CPrimitiveCfg::getMap(usedPrimitives.asString(i));
			for (uint j=0; j<prims.size(); ++j)
			{
				if (filter.find(prims[j]) != filter.end())
				{
					// check for primitive filter
					CConfigFile::CVar *pvar = IService::getInstance()->ConfigFile.getVarPtr("PrimitiveFilter");
					if (pvar)
					{
						const	std::string filename = CPath::lookup(prims[j], false);
						bool load = true;
						for (uint k=0; k<pvar->size(); ++k)
						{
							string nameFilter = pvar->asString(k);
							if (filename.find(nameFilter) != string::npos)
							{
								log.displayNL("loadMap : loading of primitive '%s' canceled by PrimitiveFilter '%s'",
									filename.c_str(),
									nameFilter.c_str());
								load = false;
								break;
							}
						}

						if (!load)
							continue;
					}

					// ensure that each primitive file is only loaded once at AIS startup
					if (loadedPrimitives.find(prims[j])!=loadedPrimitives.end())
						continue;
					loadedPrimitives.insert(prims[j]);

					// this one can be loaded
					ICommand::execute(toString("loadPrimitiveFile %s", prims[j].c_str()), log);
				}
			}
		}
	}
	return true;
}


typedef	map<string,set<string> >	TLoadedPrimitiveMapSet;
static	TLoadedPrimitiveMapSet	loadedPrimitives;

NLMISC_COMMAND(loadMap,"load a complete set of primitive files","<map name>")
{
	if(args.size() !=1)
		return false;

	CPrimitiveCfg::readPrimitiveCfg();
	const vector<string> &map = CPrimitiveCfg::getMap(args[0]);

	const	string	continentName=CPrimitiveCfg::getContinentNameOf(args[0]);
	nlassert(continentName.size()>0);
	
	vector<string>::const_iterator first(map.begin()), last(map.end());
	for (; first != last; ++first)
	{
		const	std::string filename = CPath::lookup(*first, false);

		// check for primitive filter
		CConfigFile::CVar *pvar = IService::getInstance()->ConfigFile.getVarPtr("PrimitiveFilter");
		if (pvar)
		{
			bool load = true;
			for (uint i=0; i<pvar->size(); ++i)
			{
				string filter = pvar->asString(i);
				if (filename.find(filter) != string::npos)
				{
					log.displayNL("loadMap : loading of primitive '%s' canceled by PrimitiveFilter '%s'",
						filename.c_str(),
						filter.c_str());
					load = false;
					break;
				}
			}

			if (!load)
				continue;
		}

		// check primitive already loaded		
		if	(filename.empty())
			continue;

		TLoadedPrimitiveMapSet::iterator	it=loadedPrimitives.find(continentName);
		if	(	it!=loadedPrimitives.end()
			&&	it->second.find(filename)!=it->second.end())
			continue;

		// check that the continent is active
		CUsedContinent &uc = CUsedContinent::instance();
		uint32 in = uc.getInstanceForContinent(continentName);
		if (in == INVALID_AI_INSTANCE)
		{
			log.displayNL("loadMap : while loading map '%s', can't load primitive '%s' coz continent '%s' is not active",
				args[0].c_str(),
				filename.c_str(),
				continentName.c_str());
		}
		else
		{
			ICommand::execute(toString("createStaticAIInstance %s", continentName.c_str()), log);

			loadedPrimitives[continentName].insert(filename);
			parsePrimFile(filename);
		}
	}
	return true;
}

NLMISC_COMMAND(unloadMap,"unload a complete set of primitive files","<map name>")
{
	if(args.size() !=1)
		return false;

	CPrimitiveCfg::readPrimitiveCfg();
	const vector<string> &map = CPrimitiveCfg::getMap(args[0]);

	const	string	continentName=CPrimitiveCfg::getContinentNameOf(args[0]);
	if (!continentName.empty())
	{
		vector<string>::const_iterator first(map.begin()), last(map.end());
		for (; first != last; ++first)
		{
			const	std::string filename = CPath::lookup(*first, false);

			if (filename.empty())
				continue;

			// check that the continent is active
			CUsedContinent &uc = CUsedContinent::instance();
			uint32 in = uc.getInstanceForContinent(continentName);
			if (in == INVALID_AI_INSTANCE)
			{
				log.displayNL("unloadMap : while loading map '%s', can't load primitive '%s' coz continent '%s' is not active",
					args[0].c_str(),
					filename.c_str(),
					continentName.c_str());
			}
			else
				ICommand::execute(toString("unloadPrimitiveFile %s", filename.c_str()), log);
		}
	}
	else
		log.displayNL("unloadMap failed : no map named %s found", args[0].c_str());

	// Remove this file
	loadedPrimitives.erase (continentName);

	return true;
}

NLMISC_COMMAND(verbosePrimitiveParserLog,"Turn on or off or check the state of verbose .primitive parser logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);

	nlinfo("verbose Logging is %s",VerboseLog?"ON":"OFF");
	return true;
}

} // end of namespace

