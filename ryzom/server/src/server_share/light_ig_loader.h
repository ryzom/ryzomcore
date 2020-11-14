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



#ifndef NL_LIGHT_IG_LOADER_H
#define NL_LIGHT_IG_LOADER_H

// misc
#include "nel/misc/types_nl.h"
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/stream.h>
#include <nel/misc/matrix.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>

// STL
#include <string>
#include <vector>


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CLightIGLoader
{
public:

	/// Constructor
	CLightIGLoader();


	/// Load IG
	void				loadIG(const std::string &filename);


	/// Num instances
	uint				getNumInstance()
	{
		return (uint)_ShapeNames.size();
	}

	/// get shape name
	const std::string	&getShapeName(uint i)
	{
		return _ShapeNames[i];
	}

	/// get shape name
	const std::string	&getInstanceName(uint i)
	{
		return _InstanceNames[i];
	}

	/// get instance matrix
	void				getInstanceMatrix(uint i, NLMISC::CMatrix &matrix)
	{
		matrix.identity();	
		matrix.translate(_ShapePos[i]);
		matrix.rotate(_ShapeRots[i]);
		matrix.scale(_ShapeScales[i]);	
	}


protected:

	/// File
	NLMISC::CIFile		_File;



	/// Shape names array
	std::vector<std::string>		_ShapeNames;

	/// Shape names array
	std::vector<std::string>		_InstanceNames;

	/// Shape pos array
	std::vector<NLMISC::CVector>	_ShapePos;

	/// Shape Rots array
	std::vector<NLMISC::CQuat>		_ShapeRots;

	/// Shape Scales array
	std::vector<NLMISC::CVector>	_ShapeScales;

};


#endif // NL_LIGHT_IG_LOADER_H

/* End of light_ig_loader.h */
