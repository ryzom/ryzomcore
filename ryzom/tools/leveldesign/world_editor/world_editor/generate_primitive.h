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

#ifndef NL_GENERATE_PRIMITIVE_H
#define NL_GENERATE_PRIMITIVE_H


// Element to insert in the quad grid
class CQuadGridElement
{
public:
	NLMISC::CVector		Position;
	float		Radius;
};

// ***************************************************************************

/**
  * Generate primitives
  */
class CGeneratePrimitive
{
public:

	// Constructor
	CGeneratePrimitive ();

	// Destructor
	~CGeneratePrimitive ();

	/**
	  * Generate
	  *
	  * Select the root primitive
	  * Load the landscape
	  */
	bool		generate (std::vector< std::vector<NLLIGO::IPrimitive*> > &dest, const std::vector<const NLLIGO::IPrimitive*> &source, 
							NLMISC::IProgressCallback &callback, const NLLIGO::CLigoConfig &config, const char *dataDirectory,
							const char *primClassToGenerate);

private:

	/**
	  * Clear all the container
	  */
	void		clear ();

	/**
	  * Load landscape
	  */
	bool		loadLandscape (const std::vector<const NLLIGO::IPrimitive*> &source, NLMISC::IProgressCallback &callback, 
								const NLLIGO::CLigoConfig &config, const char *dataDirectory);

	/**
	  * Insert primitive in a quad grid
	  */
	static void	insertPrimitive (NL3D::CQuadGrid<CQuadGridElement> &quadGrid, const NLLIGO::IPrimitive &primitive, NLMISC::IProgressCallback *callback);

	/**
	  * Add a primitive in a primitive pointer vector
	  */
	bool addPrimitive (std::vector<NLLIGO::IPrimitive*> &dest, const NLMISC::CVector &pos, 
						const char *primClassToGenerate, const NLLIGO::IPrimitive &parent, 
						const class CFlora &flora, const class CFloraPlant &plant,
						NL3D::CQuadGrid<CQuadGridElement> &quadGrid, const NLLIGO::CLigoConfig &config);

	/**
	  * Test a primitive position with others primitives and landscape
	  */
	bool testPosition (const NLMISC::CVector &pos, const class CFloraPlant &thePlant, const class CFlora &flora, float scale, uint layer, 
										   NL3D::CQuadGrid<CQuadGridElement> &quadGrid, const NLLIGO::CLigoConfig &config);
									   
	/**
	  * Get z on the landscape
	  */
	bool getZFromXY (float &result, float x, float y, float zMin, float zMax, uint layer, 
		const NLLIGO::CLigoConfig &config, const std::vector<NL3D::CTrianglePatch> &triangles);

	/// *** Data

	// The landscape
	NL3D::CLandscape			*_Landscape;

	// Visual collision manager
	/* todo COL_USING_VISUAL_COLLISION_MANAGER
	NL3D::CVisualCollisionManager	*_VCM;
	NL3D::CVisualCollisionEntity	*_VCE; */

	// *** Errors report
	std::vector<std::string>	_FileNotFound;
};


#endif // NL_GENERATE_PRIMITIVE_H

/* End of generate_primitive.h */
