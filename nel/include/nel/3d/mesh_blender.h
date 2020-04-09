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

#ifndef NL_MESH_BLENDER_H
#define NL_MESH_BLENDER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/material.h"


namespace NL3D
{


class	IDriver;

// ***************************************************************************
/**
 * A tool class used for Alpha Blending of Meshes.
 *	Actually, it takes a material, modify it and modify driver so it will be correctly rendered.
 *	The backup method must be used after render, to restore the material.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshBlender
{
public:

	/// Constructor
	CMeshBlender() {}

	/// Modify the material and the driver for Global Alpha Use.
	void		prepareRenderForGlobalAlpha(CMaterial &material, IDriver *drv, float globalAlpha, uint8 globalAlphaInt, bool gaDisableZWrite);

	/// Restore the material and driver in their initial state.
	void		restoreRender(CMaterial &material, IDriver *drv, bool gaDisableZWrite);

	/// Same method, but special for CoarseMesh (used by CMeshMultiLod::renderMeshGeom).
	void		prepareRenderForGlobalAlphaCoarseMesh(CMaterial &material, IDriver *drv, NLMISC::CRGBA color, float globalAlpha, bool gaDisableZWrite);

	/// Same method, but special for CoarseMesh (used by CMeshMultiLod::renderMeshGeom).
	void		restoreRenderCoarseMesh(CMaterial &material, IDriver *drv, bool gaDisableZWrite);

private:
	uint8				_BkOpacity;
	bool				_BkZWrite;
	bool				_BkBlend;
	CMaterial::TBlend	_BkSrcBlend;
	CMaterial::TBlend	_BkDstBlend;
	float				_BkAlphaTestThreshold;
	NLMISC::CRGBA		_BkupColor;

};


} // NL3D


#endif // NL_MESH_BLENDER_H

/* End of mesh_blender.h */
