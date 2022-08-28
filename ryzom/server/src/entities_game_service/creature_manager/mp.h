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



#ifndef BS_MP_H
#define BS_MP_H

#include "egs_sheets/egs_static_raw_material.h"

/**
 * class for raw material
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CRawMaterial
{
public:
	/// Constructor
	CRawMaterial() : MinQuality(0), MaxQuality(0)
	{}

	/// copy Constructor
	CRawMaterial( const CRawMaterial &m)
	{
		Name				= m.Name;
		AssociatedItemName	= m.AssociatedItemName;
		MinQuality			= m.MinQuality;
		MaxQuality			= m.MaxQuality;
	}


	/// copy Constructor
	CRawMaterial( const CStaticRawMaterial &m)
	{
		Name				= m.Name;
		AssociatedItemName	= m.AssociatedItemName;
		MinQuality			= m.MinQuality;
		MaxQuality			= m.MaxQuality;
	}
	

	/// Destructor
	~CRawMaterial()
	{}

	/// serial
	void serial(class NLMISC::IStream &f)
	{
		f.serial( Name );
		f.serial( AssociatedItemName );
		f.serial( MinQuality );
		f.serial( MaxQuality );
	}

public:
	/// name of the mp
	std::string		Name;

	/// associated item name
	std::string		AssociatedItemName;

	/// min quality
	uint8			MinQuality;

	/// max quality
	uint8			MaxQuality;
};


#endif // BS_MP_H

/* End of mp.h */
