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

#ifndef NL_PATCH_RDR_PASS_H
#define NL_PATCH_RDR_PASS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/3d/driver.h"
#include "nel/3d/landscape_def.h"
#include "nel/3d/patch.h"
#include <vector>


namespace NL3D
{


// ***************************************************************************
/**
 * A render pass for a landscape material (tile or texture far).
 * Has list of Patch / TileId.
 */
class	CPatchRdrPass : public CRefCount
{
public:
	// The refcount to know how many tiles use it (init at 0).
	sint			RefCount;

	/// \name The Tiny material for this pass.
	// @{
	// The diffuse texture (for Far and tiles).
	NLMISC::CSmartPtr<ITexture>		TextureDiffuse;
	// The Alpha texture (for tiles only).
	NLMISC::CSmartPtr<ITexture>		TextureAlpha;
	// @}


	/// \name The Patch/Tile List for this pass.. updated at each render(), (in CPatch::preRender() and CLandscape::render()).
	/**
	 *	maxRenderedFaces is used to over-estimate the number of faces that will be rendered in this pass.
	 *	Since all Far0/Far1/Tiles are added, this is REALLY over-estimated because in CLandscape::render(),
	 *	Far0/Far1/Tiles render are split.
	 */
	// @{
	void		clearAllRenderList();
	void		appendRdrPatchFar0(CPatch *rdrPatch)
	{
		_MaxRenderedFaces+= rdrPatch->NumRenderableFaces;
		rdrPatch->_NextRdrFar0= _Far0ListRoot;
		_Far0ListRoot= rdrPatch;
	}
	void		appendRdrPatchFar1(CPatch *rdrPatch)
	{
		_MaxRenderedFaces+= rdrPatch->NumRenderableFaces;
		rdrPatch->_NextRdrFar1= _Far1ListRoot;
		_Far1ListRoot= rdrPatch;
	}
	void		appendRdrPatchTile(uint pass, CRdrTileId *rdrTile, uint maxRenderedFaces)
	{
		_MaxRenderedFaces+= maxRenderedFaces;
		rdrTile->_Next= _TileListRoot[pass];
		_TileListRoot[pass]= rdrTile;
	}
	CPatch		*getRdrPatchFar0() {return _Far0ListRoot;}
	CPatch		*getRdrPatchFar1() {return _Far1ListRoot;}
	CRdrTileId  *getRdrTileRoot(uint pass) {return _TileListRoot[pass];}
	uint		getMaxRenderedFaces() const {return _MaxRenderedFaces;}
	// @}


public:
	CPatchRdrPass();

	// The operator which compare the material.
	bool			operator<(const CPatchRdrPass &o) const
	{
		// Compare first the Alphatext, so minmum changes are made during render...
		if(TextureAlpha!=o.TextureAlpha)
			return (void*)TextureAlpha<(void*)o.TextureAlpha;
		else
			return (void*)TextureDiffuse<(void*)o.TextureDiffuse;
	}

private:
	/* NB: for faster Cache Access during preRender(), you must leave those variables packed like this
		_MaxRenderedFaces, _Far0ListRoot, _Far1ListRoot, _TileListRoot
	*/
	uint				_MaxRenderedFaces;
	// The list for Far0 and Far1 to render.
	CPatch				*_Far0ListRoot;
	CPatch				*_Far1ListRoot;
	// The list for each pass of Tiles to render.
	CRdrTileId			*_TileListRoot[NL3D_MAX_TILE_PASS];

};



} // NL3D


#endif // NL_PATCH_RDR_PASS_H

/* End of patch_rdr_pass.h */
