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
#include "dbgroup_list_sheet_text_phrase_id.h"
#include "sphrase_manager.h"

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetTextPhraseId, std::string, "list_sheet_phraseid");

// ***************************************************************************
CDBGroupListSheetTextPhraseId::CDBGroupListSheetTextPhraseId(const TCtorParam &param)
:CDBGroupListSheetText(param)
{
	_CheckCoordAccelerated = false;
}



// ***************************************************************************
CDBGroupListSheetTextPhraseId::CSheetChildPhrase::CSheetChildPhrase()
{
	CacheVersion= 0;
}


// ***************************************************************************
void CDBGroupListSheetTextPhraseId::CSheetChildPhrase::updateViewText(CDBGroupListSheetText * /* pFather */)
{
	ucstring	text;
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SPhraseId)
		return;

	// Get the User Name of the phrase
	Ctrl->getContextHelp(text);

	Text->setText(text);
}

// ***************************************************************************
bool CDBGroupListSheetTextPhraseId::CSheetChildPhrase::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SPhraseId)
		return false;

	// Empty phrase? no problem.
	sint32	id= Ctrl->getSPhraseId();
	if(id==0)
		return false;

	// Verify that the id and the version of the Phrase is the same, else the name may have changed
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	sint32	version= pPM->getPhraseVersion(id);
	if(CacheVersion!=version)
		return true;
	else
		return false;
}

// ***************************************************************************
void CDBGroupListSheetTextPhraseId::CSheetChildPhrase::update(CDBGroupListSheetText * /* pFather */)
{
	if(Ctrl->getType()!=CCtrlSheetInfo::SheetType_SPhraseId)
		return;

	// Empty phrase? no op.
	sint32	id= Ctrl->getSPhraseId();
	if(id==0)
		return;

	// Verify that the id and the version of the Phrase is the same, else the name may have changed
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	sint32	version= pPM->getPhraseVersion(id);
	// update version
	CacheVersion= version;
}
