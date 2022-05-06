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

#ifndef NL_U_SHAPE_BANK_H
#define NL_U_SHAPE_BANK_H

#include "nel/misc/types_nl.h"
#include "u_shape.h"


namespace NLMISC
{
	class IProgressCallback;
}

namespace NL3D
{

/**
 * Game interface for managing shape bank
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class UShapeBank
{
public:
	virtual ~UShapeBank() {}

	/**
	  * Add a new ShapeCache. If already exist do nothing.
	  */
	virtual void addShapeCache(const std::string &shapeCacheName) = 0;

	/**
	  * Remove a ShapeCache. All shapes in the shape cache are deleted. All links are redirected
	  * to the default ShapeCache.
	  */
	virtual void removeShapeCache(const std::string &shapeCacheName) = 0;

	/**
	  * Remove all ShapeCache and suppress all links (even the link to the default cache are removed)
	  */
	virtual void reset() = 0;

	/**
	  * Set the shapeCache shapeCacheName the new size.(delete shapes if maxsize<shapeCacheSize).
	  */
	virtual void setShapeCacheSize(const std::string &shapeCacheName, sint32 maxSize) = 0;

	/**
	  * Link a shape to a ShapeCache. The ShapeCache must exist and must not contains the shape.
	  */
	virtual void linkShapeToShapeCache(const std::string &shapeName, const std::string &shapeCacheName) = 0;


	/** PreLoad all shapes (.shape, .ps, .skel...) files from a directory into a shapeCache.
	 *	Shapes are Loaded if not present, assigned to the given cache, and fit in the cache Size as max possible.
	 *	NB: crash if you try to load a non shape file (eg: a .dds etc...)
	 *	\param shapeCacheName name of a shapeCache created with addShapeCache()/setShapeCacheSize(). no-op if don't exist
	 *	\param path a valid path (local or not) where to find shapes. NB: CPath is used to load the shapes.
	 *	\param wildcard a filter string like: "*.shape", "??_HOM*.shape". NB: toLower-ed internally
	 *	\param recurs true if want to recurs in sub directory
	 */
	virtual void	preLoadShapesFromDirectory(const std::string &shapeCacheName,
		const std::string &path, const std::string &wildCard, bool recurs= false, NLMISC::IProgressCallback *progress = NULL, bool flushTextures = false) =0;

	/** PreLoad all shapes (.shape, .ps, .skel...) files from a directory into a shapeCache.
	 *	same as preLoadShapesFromDirectory() but take a BNP name which must have been added with
	 *	CBigFile::add() or through CPath::addSearchBigFile()
	 *	\param bnpName eg: "characters.bnp" (NB: set the bigFileNAme without any path).
	 *  \param flushTex : true to flush the texture of each shape when it is loaded
	 */
	virtual void	preLoadShapesFromBNP(const std::string &shapeCacheName,
		const std::string &bnpName, const std::string &wildCard, NLMISC::IProgressCallback *progress = NULL, bool flushTextures = false) =0;

	/** Return a UShape proxy from this name. NB: if not found the mesh is not loaded, and the proxy
	 *	returned will be empty
	 */
	virtual UShape	getShape(const std::string &shapeName) = 0;

	/** Mark this shape name as needing a buildSystemGeometry() (typically used for selection)
	 *	NB: this will take effect only for shapes on a subsequent add(), load() or loadAsync()
	 */
	virtual void	buildSystemGeometryForshape(const std::string &shapeName) = 0;

};


} // NL3D


#endif // NL_U_SHAPE_BANK_H

/* End of u_shape_bank.h */
