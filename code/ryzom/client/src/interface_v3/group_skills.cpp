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



// ***************************************************************************
#include "stdpch.h"

#include "group_skills.h"
#include "interface_manager.h"
#include "nel/gui/interface_expr.h"

#include "nel/gui/view_text.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/dbview_number.h"
#include "nel/gui/dbview_bar.h"

#include "game_share/skills.h"
#include "nel/misc/xml_auto_ptr.h"

#include "skill_manager.h"
#include "nel/misc/i_xml.h"

#include "../string_manager_client.h"

#include "../user_entity.h"

// ***************************************************************************
using namespace std;
using namespace NLMISC;

// ***************************************************************************
#define DB_SKILLS		"SERVER:CHARACTER_INFO:SKILLS"
#define WIN_TREE_LIST	"sbtree:tree_list"

extern CUserEntity	*UserEntity;

// Context help
extern void contextHelp (const std::string &help);

NLMISC_REGISTER_OBJECT(CViewBase, CGroupSkills, std::string, "skills_displayer");


bool CGroupSkills::InhibitSkillUpFX = true;


// ***************************************************************************
bool CGroupSkills::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!CInterfaceGroup::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr;

	// read _TemplateSkill
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"template_skill");
	if (ptr)
	{
		_TemplateSkill = (const char*)ptr;
	}

	// read _AHCtrlNode
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"node_handler");
	if (ptr)
	{
		_AHCtrlNode = (const char*)ptr;
	}


	// Add observer on the skill parameter because if the skill is coming to be not-zero, a new skill must be displayed
	_SkillsObs.Owner = this;

	string sTmp;
	ICDBNode::CTextId textId;

	for (uint k = 0; k < SKILLS::NUM_SKILLS; ++k)
	{
		sTmp = string(DB_SKILLS)+":"+NLMISC::toString((sint32)k)+":BaseSKILL";
		textId = ICDBNode::CTextId( sTmp );
		NLGUI::CDBManager::getInstance()->getDB()->addObserver (&_SkillsObs, textId );
	}

	_MustRebuild = true;

	// create all the Tree Nodes now
	createAllTreeNodes();

	return true;
}

// ***************************************************************************
void CGroupSkills::clearGroups()
{
	CInterfaceGroup::clearGroups();
}

// ***************************************************************************
void CGroupSkills::checkCoords ()
{
	if (_MustRebuild)
		rebuild();
	CInterfaceGroup::checkCoords();
}


// ***************************************************************************
void CGroupSkills::rebuild()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSkillManager *pSM = CSkillManager::getInstance();

	// **** first time bind?
	if(!_Tree)
	{
		_Tree = dynamic_cast<CGroupTree*>(CWidgetManager::getInstance()->getElementFromId(getId(),WIN_TREE_LIST));
		if (_Tree == NULL)
		{
			nlwarning("cant find tree");
			return;
		}

		// setup
		_Tree->setRootNode (_TreeRoot);
		_Tree->selectLine(0);
	}

	// **** Update the Show flag of each node
	// for each skill
	for (uint32 i = 0; i < SKILLS::NUM_SKILLS; ++i)
	{
		bool	show= false;
		if (!pSM->isUnknown((SKILLS::ESkills)i))
		{
			// Show level 0 and level 1 skills + skills that have a PARENT with value > 0
			SKILLS::ESkills		parentSkill= pSM->getParent((SKILLS::ESkills)i);

			// if no grand parent, or if grand parent trained
			if (pSM->getParent(parentSkill) == SKILLS::unknown ||
				pSM->getBaseSkillValue(parentSkill) > 0)
				show= true;
		}

		// set shown?
		if(_AllNodes[i])
			_AllNodes[i]->Show= show;
	}

	// refresh
	_Tree->forceRebuild();

	invalidateCoords();
	_MustRebuild = false;
}


// ***************************************************************************

void CGroupSkills::CSkillsObs::update (ICDBNode *node)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	CCDBNodeLeaf *leaf = NLMISC::safe_cast<CCDBNodeLeaf *>(node);

	// Rebuild all only if new skill (previously level 0)
	if (leaf->getOldValue32() == 0)
		Owner->_MustRebuild= true;

	// Popup a message if previous was not 0
	if ((leaf->getOldValue32() != 0) &&
		(leaf->getValue32() != 0)) // prevent displaying FX when resetData() is called during a Far TP
	{
		CGroupSkills::InhibitSkillUpFX = false;

		ICDBNode *skill = node->getParent();
		if (skill)
		{
			ICDBNode *skillParent = skill->getParent();
			if (skillParent)
			{
				uint skillId;
				if (skillParent->getNodeIndex (skill, skillId))
				{
					CAHManager::getInstance()->runActionHandler("skill_popup", NULL, "skillId="+toString(skillId)+"|delta="+toString(leaf->getValue32()-leaf->getOldValue32()));

					// Context help
					contextHelp ("skill");
				}
			}
		}
	}
	else
	{
		if( !CGroupSkills::InhibitSkillUpFX ) // TODO: couldn't this be replaced by IngameDbMngr.initInProgress()?
		{
			UserEntity->skillUp();
		}
	}

	// Check if this skill canunblock title
	CSkillManager *pSM = CSkillManager::getInstance();
	string sTmp = leaf->getFullName();
	sTmp = sTmp.substr(0, sTmp.rfind(':'));
	sTmp = sTmp.substr(sTmp.rfind(':')+1,sTmp.size());
	sint32 eSkills;
	fromString(sTmp, eSkills);
	pSM->tryToUnblockTitleFromSkill((SKILLS::ESkills)eSkills, leaf->getValue32());
}


// ***************************************************************************
// Get the skill buf text
static DECLARE_INTERFACE_USER_FCT(getSkillBaseText)
{
	if (args.size() != 2)
	{
		nlwarning("<getSkillBaseText> Expecting 2 arg.");
		return false;
	}
	if (!args[0].toInteger() && !args[1].toInteger())
	{
		nlwarning("<getSkillBaseText> Can't convert arg 0/1 to a int value.");
		return false;
	}

	sint64	skillValue= args[0].getInteger();
	sint64	skillBase= args[1].getInteger();

	if(skillValue!=skillBase)
	{
		result.setUCString( toString("(%d)", skillBase) );
	}
	else
	{
		result.setUCString( ucstring() );
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("getSkillBaseText", getSkillBaseText);


// ***************************************************************************
class CSortNode
{
public:
	CGroupTree::SNode	*Node;
	sint				Value;

	bool	operator<(const CSortNode &o) const
	{
		return Value<o.Value;
	}
};
// constructor
CGroupSkills::CGroupSkills( const TCtorParam &param ) :
	CInterfaceGroup(param)
{
	_MustRebuild = false;
	_Tree= NULL;
	_TreeRoot= NULL;
}

// destructor
CGroupSkills::~CGroupSkills()
{
	// remove observers
	_SkillsObs.Owner = NULL;

	CInterfaceManager *pIM= CInterfaceManager::getInstance();
	string sTmp;
	ICDBNode::CTextId textId;
	for (uint k = 0; k < SKILLS::NUM_SKILLS; ++k)
	{
		sTmp = string(DB_SKILLS)+":"+NLMISC::toString((sint32)k)+":BaseSKILL";
		textId = ICDBNode::CTextId( sTmp );
		NLGUI::CDBManager::getInstance()->getDB()->removeObserver(&_SkillsObs, textId );
	}

	// first remove any nodes from the tree group
	if( _Tree )
	{
		// reset now the node hierarchy. NB: the node hierarchy is also deleted
		_Tree->setRootNode(NULL);
	}
	_Tree= NULL;

	// template nodes not linked to hierarchy will memory leak, we must remove them also
	for (sint i = 0; i<SKILLS::NUM_SKILLS; i++)
	{
		// is the refptr still allocated?
		if(_AllNodes[i])
		{
			// NB: delete call makeOrphan, and delete children that may still be in the array
			// but it's OK, because tested with refptr
			delete _AllNodes[i];
			_AllNodes[i]= NULL;
		}
	}

	// still allocated (refptr) ?
	if(_TreeRoot)
	{
		delete _TreeRoot;
		_TreeRoot = NULL;
	}
}
void CGroupSkills::createAllTreeNodes()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSkillManager *pSM = CSkillManager::getInstance();

	// Construct the snode hierarchy structure
	_TreeRoot = new CGroupTree::SNode;
	_AllNodes.resize(SKILLS::NUM_SKILLS, NULL);
	bool bQuit = false;
	uint nCounter = 0;

	// local variable (avoid realloc in loop)
	vector< pair<string, string> > tempVec(2);
	ucstring	sSkillName;

	while ((!bQuit) && (nCounter < 32)) // Counter is used to not infinitly loop
	{
		nCounter++;
		bQuit = true;
		// Try to create a skill
		for (uint32 i = 0; i < SKILLS::NUM_SKILLS; ++i)
		if (_AllNodes[i] == NULL) // not already created
		{
			if (pSM->isUnknown((SKILLS::ESkills)i)) continue;

			// Create all skills
			SKILLS::ESkills		parentSkill= pSM->getParent((SKILLS::ESkills)i);

			// if parent, the parent node must be created
			if (parentSkill != SKILLS::unknown)
			{
				if (_AllNodes[parentSkill] == NULL)
				{
					bQuit = false;
					continue;
				}
			}

			// Ok lets create it
			CGroupTree::SNode *pNode = new CGroupTree::SNode;
			pNode->Id = NLMISC::toString(i);

			// get Skill Name
			sSkillName = STRING_MANAGER::CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)i);

			// just text or template?
			if(_TemplateSkill.empty())
			{
				pNode->DisplayText = true;
				pNode->Template = NULL;
				pNode->Text= sSkillName;
			}
			else
			{
				pNode->DisplayText = false;

				// create the template
				tempVec[0].first="id"; tempVec[0].second= pNode->Id;
				tempVec[1].first="skillid"; tempVec[1].second= NLMISC::toString(i);
				CInterfaceGroup	*pIG = CWidgetManager::getInstance()->getParser()->createGroupInstance(_TemplateSkill, getId() + ":" + WIN_TREE_LIST, tempVec);
				if (pIG == NULL)
					nlwarning("error");
				// Set Skill Name
				CViewText *pViewSkillName = dynamic_cast<CViewText*>(pIG->getView("name"));
				if (pViewSkillName != NULL)
					pViewSkillName->setText (sSkillName);
				// Set Skill Max Value
				CViewText *pViewSkillMax = dynamic_cast<CViewText*>(pIG->getView("max"));
				if (pViewSkillMax != NULL)
					pViewSkillMax->setText (toString(pSM->getMaxSkillValue((SKILLS::ESkills)i)));
				pNode->Template = pIG;
			}


			// Action handler?
			if(!_AHCtrlNode.empty())
			{
				pNode->AHName= _AHCtrlNode;
				pNode->AHParams= NLMISC::toString(i);
			}

			// bkup
			_AllNodes[i] = pNode;

			// not opened by default
			pNode->Opened= false;

			// Attach to the good parent
			if (parentSkill == SKILLS::unknown)
				_TreeRoot->addChild(pNode);
			else
				_AllNodes[parentSkill]->addChild(pNode);
		}
	}

	// Sort the First level in this order: Combat/Magic/Craft/Forage/Others.
	vector<CSortNode>	sortNodes;
	sortNodes.resize(_TreeRoot->Children.size());
	uint	i;
	for(i=0;i<_TreeRoot->Children.size();i++)
	{
		sortNodes[i].Node= _TreeRoot->Children[i];
		// get the skill value of this node
		sint	skillValue;
		fromString(_TreeRoot->Children[i]->Id, skillValue);
		// Special sort:
		if(skillValue==SKILLS::SF)
			skillValue= -4;
		if(skillValue==SKILLS::SM)
			skillValue= -3;
		if(skillValue==SKILLS::SC)
			skillValue= -2;
		if(skillValue==SKILLS::SH)
			skillValue= -1;
		// prepare tri
		sortNodes[i].Value= skillValue;
	}
	sort(sortNodes.begin(), sortNodes.end());
	// store sorted values
	for(i=0;i<_TreeRoot->Children.size();i++)
	{
		_TreeRoot->Children[i]= sortNodes[i].Node;
	}
}



