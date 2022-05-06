// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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



#ifndef NL_DBGROUP_LIST_SHEET_ICON_PHRASE_H
#define NL_DBGROUP_LIST_SHEET_ICON_PHRASE_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet.h"


// ***************************************************************************
/**
 * Special IconList for displaying Phrase Sheet. Optionally display progression info
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CDBGroupListSheetIconPhrase : public CDBGroupListSheet
{
public:

	/// Constructor
	CDBGroupListSheetIconPhrase(const TCtorParam &param);

	// A child node
	struct	CSheetChildPhrase : public CDBGroupListSheet::CSheetChild
	{
		CSheetChildPhrase();
		virtual void init(CDBGroupListSheet *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheet *pFather);
		virtual void update(CDBGroupListSheet *pFather);
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


#endif // NL_DBGROUP_LIST_SHEET_ICON_PHRASE_H

/* End of dbgroup_list_sheet_icon_phrase.h */
