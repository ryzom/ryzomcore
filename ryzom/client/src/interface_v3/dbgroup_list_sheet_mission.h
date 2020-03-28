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



#ifndef CL_DBGROUP_LIST_SHEET_MISSION_H
#define CL_DBGROUP_LIST_SHEET_MISSION_H


#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet_text.h"

// ***************************************************************************
/**
 * List of mission with a description. Selectable by line.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetMission : public CDBGroupListSheetText
{
public:
	// A child node
	struct	CSheetChildMission : public CDBGroupListSheetText::CSheetChild
	{
		virtual void updateText(CDBGroupListSheetText * /* pFather */, ucstring &/* text */) {}
		virtual CViewText *createViewText() const;
		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		// the sheet is valid if its text is not NULL
		virtual bool isSheetValid(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);
		virtual void init(CDBGroupListSheetText *pFather, uint index);

		CInterfaceProperty	CurrentPreReqState;
		uint8				CachePreReqState;
	};

	CDBGroupListSheetMission (const TCtorParam &param)
		: CDBGroupListSheetText(param)
	{}

	virtual CSheetChild *createSheetChild() { return new CSheetChildMission; }
};



#endif
