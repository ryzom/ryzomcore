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

#ifndef NL_MESH_LIGHTMAPPER_PLANE_H
#define NL_MESH_LIGHTMAPPER_PLANE_H

// Private header of the CMeshLightmapper port (source: nel_mesh_lib calc_lm_plane.h).
// Faithful port -- see mesh_lightmapper.h for the porting rules.

#include "nel/misc/rgba.h"
#include "nel/misc/bitmap.h"
#include "nel/3d/mesh.h"

#include <vector>

namespace NL3D
{

// msk : this is the mask it has a size of w*h
// col : is all color planes continuous it has a size of w*h*nNbLayerUsed

// -------------------------------------------------------------------------------------------
struct SLMPlane
{

	sint32 x, y; // Pos in lightmap
	uint32 w, h; // Size
	std::vector<uint8> msk;	// 0 - No pixel
						// 1 - Pixel must be calculated
						// 2 - Pixel is interior and is calculated
						// 3 - Pixel is exterior in this plane but interior in another of the same smooth group
						// 4 - Pixel is exterior and is extrapolated
	std::vector<NLMISC::CRGBAF> col; // 32 bits value for each pixel of each layer.The layers are contiguous. (0.0f -> 1.0f)
	std::vector<uint8> ray;	// Raytrace composante
						// 0 - Ray passed
						// 1 - 254 - Ray blocked for one or more light
						// 255 - Ray passed through spot
	uint32 nNbLayerUsed;
	std::vector<CMesh::CFace*> faces;	

	// -----------------------------------------------------------------------
	// Interface
	// -----------------------------------------------------------------------

	SLMPlane ();

	void newLayer ();
	bool isAllBlack (uint8 nLayerNb);
	void copyColToBitmap8x2 (NLMISC::CBitmap* pImage, uint32 nLayerNb, NLMISC::CRGBAF refAmbient, NLMISC::CRGBAF refDiffuse);
	void copyColToBitmap32 (NLMISC::CBitmap* pImage, uint32 nLayerNb);
	void putIn (SLMPlane &Dst, bool bMaskOnly = false);
	bool testIn (SLMPlane &Dst);
	bool tryAllPosToPutIn (SLMPlane &Dst);
	void resize (uint32 nNewSizeX, uint32 nNewSizeY);
	void stretch (double osFactor);

	void createFromPlane (SLMPlane &Src);

	void copyFirstLayerTo (SLMPlane &Dst, uint8 nDstLayer);
	void contourDetect ();
	void andRayWidthMask ();

	void createFromFace (CMesh::CFace *pF);
	void createFromFaceGroup (std::vector<CMesh::CFace*>::iterator ItFace, uint32 nNbFace);

	NLMISC::CRGBAF getAverageColor (uint8 nLayerNb);
	bool isSameColorAs (uint8 nLayerNb, NLMISC::CRGBAF color, float precision);


	// -----------------------------------------------------------------------
	// Implementation
	// -----------------------------------------------------------------------

	bool isInTriangleOrEdge(double x, double y, double xt1, double yt1, 
							double xt2, double yt2, double xt3, double yt3);

	bool segmentIntersection(	double x1, double y1, double x2, double y2, 
								double x3, double y3, double x4, double y4);
};


} // NL3D

#endif // NL_MESH_LIGHTMAPPER_PLANE_H

/* End of mesh_lightmapper_plane.h */

