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

#include "group_phrase_skill_filter.h"
#include "interface_manager.h"
#include "nel/gui/interface_expr.h"

#include "nel/gui/view_text.h"

#include "game_share/skills.h"
#include "game_share/brick_families.h"
#include "nel/misc/xml_auto_ptr.h"

#include "skill_manager.h"
#include "sbrick_manager.h"
#include "nel/misc/i_xml.h"

#include "../string_manager_client.h"


// ***************************************************************************
using namespace std;
using namespace NLMISC;


// ***************************************************************************
#define DB_BRICKS			"SERVER:BRICK_FAMILY"
const std::string	NodeIdSpecialPower = "bt_SPECIAL_POWER";
const std::string	NodeIdNoFilter = "ALL";


NLMISC_REGISTER_OBJECT(CViewBase, CGroupPhraseSkillFilter, std::string, "phrase_skill_filter");

// ***************************************************************************
CGroupPhraseSkillFilter::CGroupPhraseSkillFilter(const TCtorParam &param)
:CInterfaceGroup(param)
{
	_MustRebuild= false;
	_Tree= NULL;

	// By default no bricks are known.
	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
		_BrickSkillUsage[i]= false;

	// But for clearness, add by default Skill Fight, Magic, Craft, and Forage
	_BrickSkillUsage[SKILLS::SF]= true;
	_BrickSkillUsage[SKILLS::SM]= true;
	_BrickSkillUsage[SKILLS::SC]= true;
	_BrickSkillUsage[SKILLS::SH]= true;
}

// ***************************************************************************
bool CGroupPhraseSkillFilter::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!CInterfaceGroup::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr;

	// read _AHCtrlNode
	ptr = (char*) xmlGetProp (cur, (xmlChar*)"node_handler");
	if (ptr)
	{
		_AHCtrlNode = (const char*)ptr;
	}


	// Add observer on each Brick Families because if some brick is learned, a new skill may be displayed
	string sTmp;
	ICDBNode::CTextId textId;
	for (uint k = 0; k < BRICK_FAMILIES::NbFamilies; ++k)
	{
		_BrickFamilyObs[k].Owner= this;
		_BrickFamilyObs[k].BrickFamily= (BRICK_FAMILIES::TBrickFamily)k;
		sTmp = string(DB_BRICKS)+":"+NLMISC::toString((sint32)k)+":BRICKS";
		textId = ICDBNode::CTextId( sTmp );
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&(_BrickFamilyObs[k]), textId );
	}

	_MustRebuild = true;

	return true;
}

// ***************************************************************************
void CGroupPhraseSkillFilter::checkCoords()
{
	if (_MustRebuild)
		rebuild();
	CInterfaceGroup::checkCoords();
}

// ***************************************************************************
void CGroupPhraseSkillFilter::bkupSkillOpenedStateRecurs(bool *skillOpened, CGroupTree::SNode *node)
{
	if(!node)
		return;
	// the rootNode has an empty Id, dont test it
	// Also don't test SpecialPower Node.
	if(!node->Id.empty() &&
		node->Id!=NodeIdSpecialPower &&
		node->Id!=NodeIdNoFilter )
	{
		// get the open state
		uint	skillId;
		fromString(node->Id, skillId);
		if(skillId<SKILLS::NUM_SKILLS)
			skillOpened[skillId]= node->Opened;
	}

	// recurs to sons
	for(uint i=0;i<node->Children.size();i++)
	{
		bkupSkillOpenedStateRecurs(skillOpened, node->Children[i]);
	}
}


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
void	CGroupPhraseSkillFilter::rebuild()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSkillManager *pSM = CSkillManager::getInstance();

	// get the tree
	if (_Tree == NULL)
	{
		_Tree = dynamic_cast<CGroupTree*>(CWidgetManager::getInstance()->getElementFromId(getId(),"sbtree:tree_list"));

		if (_Tree == NULL)
		{
			nlwarning("cant find tree");
			return;
		}
	}

	// **** Bkup old tree state
	// get the old skill selected
	string oldSkillId= _Tree->getSelectedNodeId();

	// Backup for each skill the Opened State, to keep it after rebuild
	bool			skillOpened[SKILLS::NUM_SKILLS];
	memset(skillOpened, 0, SKILLS::NUM_SKILLS * sizeof(bool));

	bkupSkillOpenedStateRecurs(skillOpened, _Tree->getRootNode());


	// **** Build the new Skill Tree
	// clean all
	_Tree->removeAll();

	// Construct the snode hierarchy structure
	CGroupTree::SNode *pRoot = new CGroupTree::SNode;
	vector<CGroupTree::SNode*> allNodes;
	allNodes.resize(SKILLS::NUM_SKILLS, NULL);
	bool bQuit = false;
	uint nCounter = 0;

	// local variable (avoid realloc in loop)
	vector< pair<string, string> > tempVec(2);
	ucstring	sSkillName;
	string		sDBNameSkillValue;

	// Build the hierarchy
	while ((!bQuit) && (nCounter < 32)) // Counter is used to not infinitly loop
	{
		nCounter++;
		bQuit = true;
		// Try to create a skill
		for (uint32 i = 0; i < SKILLS::NUM_SKILLS; ++i)
		if (allNodes[i] == NULL) // not already created
		{
			if (pSM->isUnknown((SKILLS::ESkills)i)) continue;
			// if no bricks use this skill, skip
			if (!_BrickSkillUsage[i]) continue;

			// Can create if we can obtain its parent (if it get a parent)
			if (pSM->getParent((SKILLS::ESkills)i) != SKILLS::unknown)
			{
				if (allNodes[pSM->getParent((SKILLS::ESkills)i)] == NULL)
				{
					bQuit = false;
					continue;
				}
			}

			// Ok lets create it
			CGroupTree::SNode *pNode = new CGroupTree::SNode;
			pNode->Id = NLMISC::toString(i);

			// just text
			pNode->DisplayText = true;
			pNode->Template = NULL;
			pNode->Text= STRING_MANAGER::CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)i);;

			// Action handler?
			if(!_AHCtrlNode.empty())
			{
				pNode->AHName= _AHCtrlNode;
				pNode->AHParams= NLMISC::toString(i);
			}

			// Opened?
			pNode->Opened= skillOpened[i];

			// bkup
			allNodes[i] = pNode;

			// Attach to the good parent
			if (pSM->getParent((SKILLS::ESkills)i) == SKILLS::unknown)
				pRoot->addChild(pNode);
			else
				allNodes[pSM->getParent((SKILLS::ESkills)i)]->addChild(pNode);
		}
	}

	// Sort the First level in this order: Combat/Magic/Craft/Forage/Others.
	vector<CSortNode>	sortNodes;
	sortNodes.resize(pRoot->Children.size());
	uint	i;
	for(i=0;i<pRoot->Children.size();i++)
	{
		sortNodes[i].Node= pRoot->Children[i];
		// get the skill value of this node
		sint	skillValue;
		fromString(pRoot->Children[i]->Id, skillValue);
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
	for(i=0;i<pRoot->Children.size();i++)
	{
		pRoot->Children[i]= sortNodes[i].Node;
	}

	// Add a special Fitler at root for No filter (All)
	{
		// Ok lets create it
		CGroupTree::SNode *pNode = new CGroupTree::SNode;
		pNode->Id = NodeIdNoFilter;

		// just text
		pNode->DisplayText = true;
		pNode->Template = NULL;
		pNode->Text= CI18N::get("uiPhraseNoFilter");

		// Action handler?
		if(!_AHCtrlNode.empty())
		{
			pNode->AHName= _AHCtrlNode;
			pNode->AHParams= NLMISC::toString((uint32)SKILLS::unknown);
		}

		// No sons.
		pNode->Opened= false;

		// Append front
		pRoot->addChildFront(pNode);
	}

	// Add a special Fitler at root for Special Powers Phrases (because they don't use skill)
	{
		// Ok lets create it
		CGroupTree::SNode *pNode = new CGroupTree::SNode;
		pNode->Id = NodeIdSpecialPower;

		// just text
		pNode->DisplayText = true;
		pNode->Template = NULL;
		pNode->Text= CI18N::get("uiSpecialPowerFilter");

		// Action handler?
		if(!_AHCtrlNode.empty())
		{
			pNode->AHName= _AHCtrlNode;
			pNode->AHParams= "bt=" + BRICK_TYPE::toString(BRICK_TYPE::SPECIAL_POWER);
		}

		// No sons.
		pNode->Opened= false;

		pRoot->addChild(pNode);
	}

	// Setup the Ctrl Tree
	_Tree->setRootNode (pRoot);


	// **** Reset old Tree state.
	// Select node by its id setuped before.
	if(!_Tree->selectNodeById(oldSkillId))
		// if some error (none selected before...), select line 0: All.
		_Tree->selectLine(0);


	invalidateCoords();
	_MustRebuild = false;
}


// ***************************************************************************
void CGroupPhraseSkillFilter::CBrickFamilyObs::update (ICDBNode *node)
{
	CSBrickManager		*pBM= CSBrickManager::getInstance();
	CSkillManager		*pSM= CSkillManager::getInstance();

	CCDBNodeLeaf *leaf = NLMISC::safe_cast<CCDBNodeLeaf *>(node);

	// get only new bits set
	uint64	oldBf= leaf->getOldValue64();
	uint64	newBf= leaf->getValue64();
	uint64	newBitOn= newBf & (~oldBf);

	// For all those new bits, check if the brick free up a Skill
	bool	mustTouch= false;
	for(uint i=0;i<64;i++)
	{
		// if a new bit
		if(newBitOn & (SINT64_CONSTANT(1)<<i))
		{
			CSBrickSheet	*brick= pBM->getBrick(pBM->getBrickSheet(BrickFamily, i));
			// if a valid brick / valid skill
			if(brick && brick->getSkill()<SKILLS::NUM_SKILLS)
			{
				SKILLS::ESkills		skill= brick->getSkill();
				// if the skill was not set before
				if(!Owner->_BrickSkillUsage[skill])
				{
					// Ctrl Tree must be recomputed
					mustTouch= true;

					// Flag this skill and all his parents!
					while(skill!=SKILLS::unknown)
					{
						Owner->_BrickSkillUsage[skill]= true;
						skill= pSM->getParent(skill);
					}
				}
			}
		}
	}

	// New Skill unblocked?
	if(mustTouch)
		Owner->touch ();
}

