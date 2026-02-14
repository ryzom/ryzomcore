/** \file decal_manager.cpp
 * Batched decal rendering manager for NeL 3D.
 *
 * Implements the rendering pseudocode from the intern report (§.6.3).
 * Includes the vertex program for distance attenuation, Z blending,
 * and diffuse color (ported from the legacy CLegacyDecal system).
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std3d.h"
#include "nel/3d/decal_manager.h"
#include "nel/3d/scene.h"

#include <algorithm>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
// Comparator for sorting decals by priority (lower priority value = rendered first)
static bool decalPriorityCompare(const CDecal *a, const CDecal *b)
{
	return a->getPriority() < b->getPriority();
}


// ***************************************************************************
// Vertex program assembly code for decal attenuation (ported from legacy system)
// Constants:
//   c[0-3]: ModelViewProjection matrix
//   c[4]:   WorldToUV row 0 (→ tex U)
//   c[5]:   WorldToUV row 1 (→ tex V)
//   c[6]:   Camera position (relative to model origin)
//   c[7]:   DistScaleBias (x=scale, y=bias, z=0.0, w=1.0)
//   c[8]:   Diffuse color (RGB in [0,1])
//   c[11]:  BlendScale (x=bottomScale, y=bottomBias, z=topScale, w=topBias)
static const char *DecalAttenuationVertexProgramCode =
"!!VP1.0\n\
	DP4 o[HPOS].x, c[0], v[0];	          #transform vertex in view space\n\
	DP4 o[HPOS].y, c[1], v[0];\n\
	DP4 o[HPOS].z, c[2], v[0];\n\
	DP4 o[HPOS].w, c[3], v[0];\n\
	# transform texcoord 0\n\
	DP4 o[TEX0].x, c[4], v[0];\n\
	DP4 o[TEX0].y, c[5], v[0];\n\
	#compute distance from camera\n\
	ADD R0, v[0], -c[6];\n\
	DP3 R0.x, R0, R0;\n\
	RSQ R0.x, R0.x;\n\
	RCP R0.x, R0.x;\n\
	MUL o[COL0].xyz, c[8], v[3];\n\
	#compute attenuation with distance\n\
	MAD R0.w, R0.x, c[7].x, c[7].y;\n\
	# clamp in [0, 1]\n\
	MIN R0.w, R0.w, c[7].w;\n\
	MAX R0.w, R0.w, c[7].z;\n\
	#compute bottom blend\n\
	MAD R1.x, v[0].z, c[11].x, c[11].y;\n\
	MIN R1.x, R1.x, c[7].w;\n\
	MAX R1.x, R1.x, c[7].z;\n\
	MUL R0.w, R1.x, R0.w;\n\
	#compute top blend\n\
	MAD R1.x, v[0].z, c[11].z, c[11].w;\n\
	MIN R1.x, R1.x, c[7].w;\n\
	MAX R1.x, R1.x, c[7].z;\n\
	MUL R0.w, R1.x, R0.w;\n\
	#apply vertex alpha\n\
	MUL o[COL0].w, v[3].w, R0.w;\n\
	END \n";


// ***************************************************************************
CVertexProgramDecalAttenuation::CVertexProgramDecalAttenuation()
{
	CSource *source = new CSource();
	source->Profile = nelvp;
	source->DisplayName = "nelvp/DecalAttenuation";
	source->setSourcePtr(DecalAttenuationVertexProgramCode);
	source->ParamIndices["modelViewProjection"] = 0;
	source->ParamIndices["worldToUV0"] = 4;
	source->ParamIndices["worldToUV1"] = 5;
	source->ParamIndices["refCamDist"] = 6;
	source->ParamIndices["distScaleBias"] = 7;
	source->ParamIndices["diffuse"] = 8;
	source->ParamIndices["blendScale"] = 11;
	addSource(source);
}


// ***************************************************************************
CVertexProgramDecalAttenuation::~CVertexProgramDecalAttenuation()
{
}


// ***************************************************************************
void CVertexProgramDecalAttenuation::buildInfo()
{
	m_Idx.WorldToUV0 = getUniformIndex("worldToUV0");
	nlassert(m_Idx.WorldToUV0 != std::numeric_limits<uint>::max());
	m_Idx.WorldToUV1 = getUniformIndex("worldToUV1");
	nlassert(m_Idx.WorldToUV1 != std::numeric_limits<uint>::max());
	m_Idx.RefCamDist = getUniformIndex("refCamDist");
	nlassert(m_Idx.RefCamDist != std::numeric_limits<uint>::max());
	m_Idx.DistScaleBias = getUniformIndex("distScaleBias");
	nlassert(m_Idx.DistScaleBias != std::numeric_limits<uint>::max());
	m_Idx.Diffuse = getUniformIndex("diffuse");
	nlassert(m_Idx.Diffuse != std::numeric_limits<uint>::max());
	m_Idx.BlendScale = getUniformIndex("blendScale");
	nlassert(m_Idx.BlendScale != std::numeric_limits<uint>::max());
}


// ***************************************************************************
CDecalManager::CDecalManager() :
		_NextMaterialId(0),
		_UseVertexProgram(false),
		_DistScale(0.f),
		_DistBias(1.f)
{
	// Fixed-size AGP Volatile vertex buffer with Position + TexCoord0 + PrimaryColor (see PDF §4.5.4)
	_VB.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
	_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::PrimaryColorFlag);
	_VB.setNumVertices(NL3D_DECAL_VB_MAX_VERTICES);

	// Create the vertex program instance
	_VertexProgram = new CVertexProgramDecalAttenuation();
}


// ***************************************************************************
CDecalManager::~CDecalManager()
{
}


// ***************************************************************************
void CDecalManager::clearAllDecals()
{
	_Decals.clear();
}


// ***************************************************************************
void CDecalManager::addDecal(CDecal *decal, uint32 materialId)
{
	_Decals[materialId].push_back(decal);
}


// ***************************************************************************
uint32 CDecalManager::registerMaterial(const CMaterial &mat)
{
	CRegisteredMaterial rm;
	rm.Mat = mat;
	rm.Id = _NextMaterialId;
	_Materials.push_back(rm);
	return _NextMaterialId++;
}


// ***************************************************************************
// Rendering: implements the pseudocode from PDF §.6.3
//
// All UVs and per-vertex colors are precomputed on the CPU by CDecal::computeDecal().
// The manager batches decals that share the same texture into single draw calls,
// only flushing the VB when the texture pointer changes or the buffer overflows.
void CDecalManager::flush(CScene *sc)
{
	if (_Decals.empty())
		return;

	IDriver *drv = sc->getRenderTrav().getDriver();

	// No vertex program in the batched path — UVs and colors are CPU-precomputed.
	// This is the key to batching: VP constants are per-decal, which would force
	// a VB flush per decal, defeating the purpose. CPU precomputation lets us
	// batch freely across decals sharing the same texture.
	drv->activeVertexProgram(NULL);
	drv->activeVertexBuffer(_VB);
	drv->setupModelMatrix(CMatrix::Identity);

	// Iterate over each material group
	TDecalMap::iterator matIt = _Decals.begin();
	TDecalMap::iterator matEnd = _Decals.end();

	for (; matIt != matEnd; ++matIt)
	{
		std::vector<CDecal*> &decals = matIt->second;

		if (decals.empty())
			continue;

		// Sort decals by priority within this material group (lower priority first)
		std::sort(decals.begin(), decals.end(), decalPriorityCompare);

		uint32 count = 0; // current position in VB
		bool vbLocked = false;
		CVertexBufferReadWrite vba;
		CMaterial *currentMat = NULL; // track current material for batching

		for (uint d = 0; d < decals.size(); ++d)
		{
			CDecal *decal = decals[d];

			// Get this decal's precomputed vertices, UVs, and colors
			std::vector<CVector> &verts = decal->getVertices(false);
			const std::vector<CUV> &uvs = decal->getUVs();
			const std::vector<NLMISC::CRGBA> &colors = decal->getColors();

			if (verts.empty())
				continue;

			CMaterial *decalMat = &decal->getMaterial();

			// Flush when the material pointer changes (different texture)
			if (count > 0 && decalMat != currentMat)
			{
				if (vbLocked)
				{
					vba.unlock();
					vbLocked = false;
				}
				nlassert(count % 3 == 0);
				drv->renderRawTriangles(*currentMat, 0, count / 3);
				count = 0;
			}

			currentMat = decalMat;

			uint32 length = (uint32)verts.size();
			uint32 offset = 0;
			bool hasColors = (colors.size() == verts.size());

			// Ensure VB is locked before copying
			if (!vbLocked)
			{
				_VB.lock(vba);
				vbLocked = true;
			}

			while (offset < length)
			{
				uint32 remaining = length - offset;
				uint32 space = NL3D_DECAL_VB_MAX_VERTICES - count;

				if (remaining > space)
				{
					// Not enough space: fill what we can, render, reset
					for (uint32 i = 0; i < space; ++i)
					{
						*vba.getVertexCoordPointer(count + i) = verts[offset + i];
						if (offset + i < uvs.size())
							*vba.getTexCoordPointer(count + i, 0) = uvs[offset + i];
						*(CRGBA *)vba.getColorPointer(count + i) = hasColors ? colors[offset + i] : CRGBA::White;
					}
					offset += space;
					count += space;

					// Unlock, render, re-lock
					vba.unlock();
					vbLocked = false;
					nlassert(count % 3 == 0);
					drv->renderRawTriangles(*currentMat, 0, count / 3);
					count = 0;
					_VB.lock(vba);
					vbLocked = true;
				}
				else
				{
					// Enough space: copy all remaining
					for (uint32 i = 0; i < remaining; ++i)
					{
						*vba.getVertexCoordPointer(count + i) = verts[offset + i];
						if (offset + i < uvs.size())
							*vba.getTexCoordPointer(count + i, 0) = uvs[offset + i];
						*(CRGBA *)vba.getColorPointer(count + i) = hasColors ? colors[offset + i] : CRGBA::White;
					}
					count += remaining;
					offset = length; // done with this decal
				}
			}
		}

		// Unlock and render any remaining vertices
		if (vbLocked)
		{
			vba.unlock();
			vbLocked = false;
		}
		if (count > 0 && currentMat)
		{
			nlassert(count % 3 == 0);
			drv->renderRawTriangles(*currentMat, 0, count / 3);
		}
	}
}
