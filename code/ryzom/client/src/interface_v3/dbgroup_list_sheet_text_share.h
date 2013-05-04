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



#ifndef RZ_DBGROUP_LIST_SHEET_TEXT_SHARE_H
#define RZ_DBGROUP_LIST_SHEET_TEXT_SHARE_H

#include "nel/misc/types_nl.h"
#include "dbgroup_list_sheet_text.h"

namespace NLGUI
{
	class CViewBitmap;
}

// ***************************************************************************
/**
 * Special TextList for displaying sharing items & bricks
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date December 2003
 */
class CDBGroupListSheetTextShare : public CDBGroupListSheetText
{
public:

	/// Constructor
	CDBGroupListSheetTextShare(const TCtorParam &param);

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	// A child node
	class	CSheetChildShare : public CDBGroupListSheetText::CSheetChild
	{
	public:
		CSheetChildShare();
		virtual ~CSheetChildShare();
		virtual void init(CDBGroupListSheetText *pFather, uint index);
		virtual bool isInvalidated(CDBGroupListSheetText *pFather);
		virtual void update(CDBGroupListSheetText *pFather);
		virtual void updateViewText(CDBGroupListSheetText *pFather);
		virtual void hide(CDBGroupListSheetText *pFather);

		CInterfaceProperty	CurrentNbMember;
		CInterfaceProperty	CurrentChance;
		CInterfaceProperty	CurrentWanted;

		uint8	CacheNbMember;
		uint8	CacheChance;
		bool	CacheWanted;

		CViewText	*NbMember;	// Number of member that wants this item/phrase
		CViewText	*Chance;	// Chance the player has to obtain this item/phrase
		CViewBitmap	*Wanted;	// Does the player wants this item (validate icon)
	};

	virtual CSheetChild *createSheetChild() { return new CSheetChildShare; }

	sint32		getXWanted() const {return _XWanted;}
	sint32		getYWanted() const {return _YWanted;}
	const std::string &getWantedIcon() const {return _WantedIcon;}
	const std::string &getNotWantedIcon() const {return _NotWantedIcon;}
	sint32		getXNbMember() const {return _XNbMember;}
	sint32		getYNbMember() const {return _YNbMember;}
	sint32		getXChance() const {return _XChance;}
	sint32		getYChance() const {return _YChance;}

protected:
	friend class	CSheetChildShare;
	sint32		_XWanted;
	sint32		_YWanted;
	std::string _WantedIcon;
	std::string _NotWantedIcon;
	sint32		_XNbMember;
	sint32		_YNbMember;
	sint32		_XChance;
	sint32		_YChance;

	// get the list of the list_sheet_text
	CInterfaceGroup	*getList() const {return _List;}

	// get the textTemplate of the list_sheet_text
	CViewText		&getTextTemplate() {return _TextTemplate;}
};


#endif // NL_DBGROUP_LIST_SHEET_TEXT_SHARE_H

/* End of dbgroup_list_sheet_text_share.h */
