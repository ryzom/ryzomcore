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

#ifndef NL_NOISE_3D_H
#define NL_NOISE_3D_H

// ------------------------------------------------------------------------------------------------

#include "nel/3d/material.h"
#include "nel/3d/texture.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/geom_ext.h"

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
class CNoise3d
{
public:

	CNoise3d (NL3D::IDriver *pDriver);
	~CNoise3d ();

	void init (uint32 w=64, uint32 h=64, uint32 d=32);

	// Render to the screen by adding the noise*alpha at the given position (qc)
	// qc.uv - positions in the noise (x,y)
	// qc.V  - positions in destination texture (x,y)
	// wpos  - z position in the noise
	// alpha - intensity of noise
	void render (NLMISC::CQuadUV &qc, float wpos, float intensity);
	void render2passes (NLMISC::CQuadUV &qc, float wpos, float intensity);

	void renderGrid (uint32 nbw, uint32 nbh, uint32 w, uint32 h,
					float UStart, float VStart, float WStart, float dU, float dV, float dW, float intensity);
	void renderGrid2passes (uint32 nbw, uint32 nbh, uint32 w, uint32 h,
					float UStart, float VStart, float WStart, float dU, float dV, float dW, float intensity);

	void flush ();
	void flush2passes ();

	// Accessors
	uint32 getWidth ();
	uint32 getHeight ();
	uint32 getDepth ();

private:

	uint32 _Width, _Height, _Depth;
	uint32 _NbSliceW, _NbSliceH;

	uint8									*_Mem;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	_Tex;

	float		_Intensity;
	NLMISC::CUV *_OffS; // Decalage offset for all layer (depth)
	float		_ScaleW, _ScaleH;

	uint32				_NbVertices;
	NL3D::CVertexBuffer _VertexBuffer;
	NL3D::CMaterial *_Mat;

	NL3D::IDriver *_Driver;
	bool			_IsDriverSupportCloudSinglePass;
};
// ------------------------------------------------------------------------------------------------

} // namespace NL3D

#endif // NL_NOISE_3D_H

