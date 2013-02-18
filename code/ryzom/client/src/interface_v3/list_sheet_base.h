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



#ifndef NL_LIST_SHEET_BASE_H
#define NL_LIST_SHEET_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"

namespace NLGUI
{
	class CCtrlScroll;
}

class	CDBCtrlSheet;

// ***************************************************************************
/**
 * Interface to access number of sheet elements, and elements in a ListSheet or ListSheetTrade
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class IListSheetBase : public CInterfaceGroup
{
public:

	IListSheetBase(const TCtorParam &param);

	// Get the index of a sheet inserted in this list. Returns -1 if it is not an index of that list
	virtual	sint			getIndexOf(const CDBCtrlSheet	*sheet) const =0;

	// get number of sheets
	virtual uint			getNbSheet() const = 0;

	// get a sheet by its index
	virtual	CDBCtrlSheet	*getSheet(uint index) const =0;

	// Get the number of active elements
	virtual	sint32			getNbElt () const =0;
	virtual	void			setNbElt (sint32 /* i */) { nlwarning("Must never be called"); }

	// Get the scroll bar
	virtual	CCtrlScroll		*getScrollBar() const =0;

	// get Db Branch name
	virtual const std::string &getDbBranchName() const =0;

	REFLECT_EXPORT_START(IListSheetBase, CInterfaceGroup)
		REFLECT_SINT32("nbelt", getNbElt, setNbElt);
	REFLECT_EXPORT_END

	// HELPERS
	static IListSheetBase *getListContaining(CInterfaceElement *pIE)
	{
		CInterfaceGroup *pIG = pIE->getParent();
		while (pIG != NULL)
		{
			IListSheetBase *pLSB = dynamic_cast<IListSheetBase*>(pIG);
			if (pLSB != NULL) return pLSB;
			pIG = pIG->getParent();
		}
		return NULL;
	}

protected:

	// If empty section encountered, what to do? display the section or not?
	enum	TSectionEmptyScheme
	{
		NoEmptySection=0,
		AllowEmptySectionWithNoSpace,
		AllowEmptySectionWithExtraSpace,
	};

	// Sectionable
	bool							_Sectionable;
	bool							_SectionableError;
	TSectionEmptyScheme				_SectionEmptyScheme;
	std::vector<CInterfaceGroup*>	_SectionGroups;
	bool		insertSectionGroup(uint sectionGroupIndex, uint sectionId, sint xGroup, sint yGroup);
	void		releaseSectionGroups(uint startSectionGroupIndex);
	// if sectionable, the deriver must implement those method
	// min and maxSectionId are exclusive (ie the encoutered sectionId should not be of those values)
	virtual	void				getCurrentBoundSectionId(sint &minSectionId, sint &maxSectionId) {minSectionId= 0;maxSectionId=0;}
	virtual	CInterfaceGroup		*createSectionGroup(const std::string &/* igName */) {return NULL;}
	virtual	void				deleteSectionGroup(CInterfaceGroup	*) {}
	virtual	void				setSectionGroupId(CInterfaceGroup	*, uint /* sectionId */) {}

};


#endif // NL_LIST_SHEET_BASE_H

/* End of list_sheet_base.h */
