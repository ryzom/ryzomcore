// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_LOD_CHARACTER_SHAPE_BANK_H
#define NL_LOD_CHARACTER_SHAPE_BANK_H

#include "nel/misc/types_nl.h"

#include "nel/3d/lod_character_shape.h"

namespace NL3D {



// ***************************************************************************
/**
 * A Bank of CLodCharacterShape. Can contains at max 65536 shapes.
 *	You must call compile() after changing the shape bank.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterShapeBank
{
public:

	/// Constructor
	CLodCharacterShapeBank();

	/// \name build process
	// @{

	/// reset the shapeBank
	void						reset();

	/// Just add a shape to the bank. Use getShapeFullAcces() to fill it. return id of the shape
	uint32						addShape();

	/** Get a shapeId by its name. -1 if not found. Possible Values are in 0-65535
	 *	Call valid only if compile() has been correctly called
	 */
	sint32						getShapeIdByName(const std::string &name) const;

	/// Get a const ref on a shape. Ptr not valid after addShape(). NULL if not found
	const CLodCharacterShape	*getShape(uint32 shapeId) const;

	/** Get a ptr on a shape. Ptr not valid after addShape(). NULL if not found
	 *	May be used for example to replace the shape
	 */
	CLodCharacterShape			*getShapeFullAcces(uint32 shapeId);

	/** re-compile the shape Bank. This must be called after changing a shape, or after adding all your shapes.
	 *	It return false if same names have been found, but it is still correctly builded.
	 */
	bool						compile();

	/// get number of shape. This is also the MaxID+1.
	uint						getNumShapes() const;

	// @}


	/// serial
	void			serial(NLMISC::IStream &f);


// ******************************
private:
	/// Map name To Id.
	typedef	std::map<std::string, uint32>	TStrIdMap;
	typedef	TStrIdMap::iterator				ItStrIdMap;
	typedef	TStrIdMap::const_iterator		CstItStrIdMap;


private:

	/// Array of shape
	std::vector<CLodCharacterShape>	_ShapeArray;

	/// Map of shape id
	TStrIdMap						_ShapeMap;

};


} // NL3D


#endif // NL_LOD_CHARACTER_SHAPE_BANK_H

/* End of lod_character_shape_bank.h */
