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



#ifndef NL_DBGROUP_LIST_SHEET_TEXT_PHRASE_H
#define NL_DBGROUP_LIST_SHEET_TEXT_PHRASE_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet_text.h"


// ***************************************************************************
/**
 * Special TextList for displaying Phrase Sheet. Optionally display progression info
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetTextPhrase : public CDBGroupListSheetText
{
public:

	/// Constructor
	CDBGroupListSheetTextPhrase(const TCtorParam &param);

	// A child node
	struct	CSheetChildPhrase : public CDBGroupListSheetText::CSheetChild
	{
		CSheetChildPhrase();
		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);
		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual sint getSectionId() const;
		NLMISC::CCDBNodeLeaf	*LevelDB;
		uint			LevelCache;
	};

	virtual CSheetChild *createSheetChild() { return new CSheetChildPhrase; }

	// for section mgt
	virtual	void				getCurrentBoundSectionId(sint &minSectionId, sint &maxSectionId);
	virtual	CInterfaceGroup		*createSectionGroup(const std::string &igName);
	virtual	void				deleteSectionGroup(CInterfaceGroup	*group);
	virtual	void				setSectionGroupId(CInterfaceGroup	*, uint sectionId);

};


#endif // NL_DBGROUP_LIST_SHEET_TEXT_PHRASE_H

/* End of dbgroup_list_sheet_text_phrase.h */
