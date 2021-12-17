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



#ifndef ITEM_CL_H
#define ITEM_CL_H

// Misc
#include "nel/misc/types_nl.h"

// Client
#include "entity_cl.h"


/**
 * CItemCL
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CItemCL : public CEntityCL
{
protected:
	/// Update the item position.
	virtual void updateVisualPropertyPos(const NLMISC::TGameCycle &gameCycle, const sint64 &prop, const NLMISC::TGameCycle &pI);

public:
	NLMISC_DECLARE_CLASS(CItemCL);

	/// Constructor
	CItemCL();

	/// Constructor
	CItemCL( const std::string &fileName );

	/// Destructor
	~CItemCL();

	/**
	 * Build the entity from a sheet.
	 */
	virtual bool build(const CEntitySheet *sheet);

	/// Initialize properties for an item.
	virtual void initProperties();

	/// Draw the selection Box
	virtual void drawBox();

private :

	/**
	 * Init the instance
	 */
	void initShape( const std::string &fileName );

};


#endif // ITEM_CL_H

/* End of item_cl.h */
