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



#ifndef NL_DBGROUP_LIST_SHEET_TEXT_BRICK_COMPOSITION_H
#define NL_DBGROUP_LIST_SHEET_TEXT_BRICK_COMPOSITION_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet_text.h"


// ***************************************************************************
/**
 * Special TextList for displaying brick for composition
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CDBGroupListSheetTextBrickComposition : public CDBGroupListSheetText
{
public:

	/// Constructor
	CDBGroupListSheetTextBrickComposition(const TCtorParam &param);

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	// A child node
	class	CSheetChildBrick : public CDBGroupListSheetText::CSheetChild
	{
	public:
		CSheetChildBrick();
		virtual ~CSheetChildBrick();
		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);
		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual void hide(CDBGroupListSheetText *pFather);
		virtual	sint getDeltaX(CDBGroupListSheetText *pFather) const;

		// The special costView for this child
		CViewText	*CostView;
	};

	virtual CSheetChild *createSheetChild() { return new CSheetChildBrick; }

	sint32		getXCost() const {return _XCost;}
	sint32		getYCost() const {return _YCost;}

protected:
	friend class	CSheetChildBrick;
	sint32		_XCost;
	sint32		_YCost;
	sint32		_BrickParameterDeltaX;

	// get the list of the list_sheet_text
	CInterfaceGroup	*getList() const {return _List;}

	// get the textTemplate of the list_sheet_text
	CViewText		&getTextTemplate() {return _TextTemplate;}
};


#endif // NL_DBGROUP_LIST_SHEET_TEXT_BRICK_COMPOSITION_H

/* End of dbgroup_list_sheet_text_brick_composition.h */
