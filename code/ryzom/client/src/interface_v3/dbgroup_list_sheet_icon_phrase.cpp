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
#include "dbgroup_list_sheet_icon_phrase.h"
#include "sphrase_manager.h"
#include "interface_manager.h"

using namespace	std;
using namespace	NLMISC;


// ***************************************************************************
// This is the template used for section
#define LSP_SECTION_TEMPLATE	"template_phrase_progression_section"


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetIconPhrase, std::string, "list_sheet_icon_phrase");

CDBGroupListSheetIconPhrase::CDBGroupListSheetIconPhrase(const TCtorParam &param)
: CDBGroupListSheet(param)
{
	// Split into Sections!
	_Sectionable= true;
	// allow empty sections to show there is nothing to display
	_SectionEmptyScheme= AllowEmptySectionWithNoSpace;
}


// ***************************************************************************
CDBGroupListSheetIconPhrase::CSheetChildPhrase::CSheetChildPhrase()
{
	LevelDB= NULL;
}

// ***************************************************************************
void CDBGroupListSheetIconPhrase::CSheetChildPhrase::init(CDBGroupListSheet *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	// try to get the Level DB
	CCDBNodeBranch *root = Ctrl->getRootBranch();
	if (root)
	{
		LevelDB= dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("LEVEL"), false));
	}
}


// ***************************************************************************
bool CDBGroupListSheetIconPhrase::CSheetChildPhrase::isInvalidated(CDBGroupListSheet * /* pFather */)
{
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SPhrase)
		return false;

	// no LEVEL db? no problem.
	if(!LevelDB)
		return false;

	// Verify that level cache has change or not
	if(LevelCache!=(uint32)LevelDB->getValue32())
		return true;
	else
		return false;
}

// ***************************************************************************
void CDBGroupListSheetIconPhrase::CSheetChildPhrase::update(CDBGroupListSheet * /* pFather */)
{
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SPhrase)
		return;

	// no LEVEL db? no problem.
	if(!LevelDB)
		return;

	// cache
	LevelCache= LevelDB->getValue32();
}

// ***************************************************************************
sint CDBGroupListSheetIconPhrase::CSheetChildPhrase::getSectionId() const
{
	// based on its required level
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	return pPM->getPhraseSectionFromLevel(LevelCache);
}

// ***************************************************************************
CInterfaceGroup		*CDBGroupListSheetIconPhrase::createSectionGroup(const std::string &igName)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	vector< pair<string, string> > tempVec(1);
	tempVec[0].first="id"; tempVec[0].second= igName;

	// make parent to me
	CInterfaceGroup	*pIG = CWidgetManager::getInstance()->getParser()->createGroupInstance(LSP_SECTION_TEMPLATE, getId(), tempVec);
	pIG->setParent (this);
	addGroup (pIG);

	return pIG;
}

// ***************************************************************************
void				CDBGroupListSheetIconPhrase::deleteSectionGroup(CInterfaceGroup	*group)
{
	delGroup(group);
}

// ***************************************************************************
void				CDBGroupListSheetIconPhrase::setSectionGroupId(CInterfaceGroup	*pIG, uint sectionId)
{
	nlassert(pIG);

	// Set Name
	CViewText *name = dynamic_cast<CViewText*>(pIG->getView("name"));
	if (name != NULL)
	{
		ucstring	sectionText= CI18N::get("uiPhraseSectionFmt");
		uint32	minLevel, maxLevel;
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		pPM->getPhraseLevelFromSection(sectionId, minLevel, maxLevel);
		strFindReplace(sectionText, "%min", toString(minLevel));
		strFindReplace(sectionText, "%max", toString(maxLevel));
		name->setText (sectionText);
	}
}

// ***************************************************************************
void				CDBGroupListSheetIconPhrase::getCurrentBoundSectionId(sint &minSectionId, sint &maxSectionId)
{
	// get the actual bound from book filter. NB: this work because the book filter is updated
	// at same time the database is modified, hence updateCoords is called, then this skill
	// filter is already up to date.
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	pPM->getPhraseSectionBoundFromSkillFilter(minSectionId, maxSectionId);
}

