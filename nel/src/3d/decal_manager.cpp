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
// Helper: avoid division by zero for blend scale computation
static inline float decal_favoid0(float f)
{
	return NLMISC::favoid0(f);
}


// ***************************************************************************
// Rendering: implements the pseudocode from PDF §.6.3
//
// With vertex program path:
//   - VP handles MVP transform, UV generation, distance attenuation,
//     bottom/top Z blending, diffuse color modulation
//   - Per-decal VP constants set before each decal's vertices are flushed
//
// Without vertex program (CPU fallback):
//   - UVs and per-vertex colors precomputed by CDecal::computeDecal()
//   - Written directly into the VB alongside positions
void CDecalManager::flush(CScene *sc)
{
	if (_Decals.empty())
		return;

	IDriver *drv = sc->getRenderTrav().getDriver();

	// Try to compile and activate the vertex program
	bool vpActive = false;
	CVertexProgramDecalAttenuation *vp = _VertexProgram;
	if (_UseVertexProgram && vp && drv->compileVertexProgram(vp))
	{
		drv->activeVertexProgram(vp);
		// Set distance attenuation constants (shared across all decals)
		drv->setUniform4f(IDriver::VertexProgram, vp->idx().DistScaleBias, _DistScale, _DistBias, 0.f, 1.f);
		vpActive = true;
	}
	else
	{
		drv->activeVertexProgram(NULL);
	}

	drv->activeVertexBuffer(_VB);
	drv->setupModelMatrix(CMatrix::Identity);

	const CVector &camPos = sc->getCam()->getMatrix().getPos();

	// Iterate over each material group
	TDecalMap::iterator matIt = _Decals.begin();
	TDecalMap::iterator matEnd = _Decals.end();

	for (; matIt != matEnd; ++matIt)
	{
		uint32 materialId = matIt->first;
		std::vector<CDecal*> &decals = matIt->second;

		if (decals.empty())
			continue;

		// Sort decals by priority within this material group (lower priority first)
		std::sort(decals.begin(), decals.end(), decalPriorityCompare);

		// Find the registered material for this ID
		CMaterial *mat = NULL;
		for (uint m = 0; m < _Materials.size(); ++m)
		{
			if (_Materials[m].Id == materialId)
			{
				mat = &_Materials[m].Mat;
				break;
			}
		}

		// If no registered material found, use the first decal's material
		CMaterial fallbackMat;
		if (!mat)
		{
			if (!decals.empty())
			{
				fallbackMat = decals[0]->getMaterial();
			}
			else
			{
				fallbackMat.initUnlit();
				fallbackMat.setDoubleSided(true);
			}
			mat = &fallbackMat;
		}

		uint32 count = 0; // current position in VB
		bool vbLocked = false;
		CVertexBufferReadWrite vba;

		for (uint d = 0; d < decals.size(); ++d)
		{
			CDecal *decal = decals[d];

			// Get this decal's computed vertices and UVs
			std::vector<CVector> &verts = decal->getVertices(vpActive);
			const std::vector<CUV> &uvs = decal->getUVs();

			if (verts.empty())
				continue;

			// Each decal has its own material (with its own texture).
			// Flush when the material changes or when using VP (VP constants are per-decal).
			CMaterial *decalMat = &decal->getMaterial();
			if (count > 0 && (decalMat != mat || vpActive))
			{
				if (vbLocked)
				{
					vba.unlock();
					vbLocked = false;
				}
				nlassert(count % 3 == 0);
				drv->renderRawTriangles(*mat, 0, count / 3);
				count = 0;
			}

			// Use this decal's own material (which has its texture set)
			mat = decalMat;

			// If using VP, set per-decal constants before we start copying vertices
			if (vpActive)
			{
				// WorldToUV: rows of the inverse world matrix for UV generation
				const CMatrix &worldToUV = decal->getWorldToUVMatrix();
				float row0[4], row1[4];
				// Row 0: maps world position to U coordinate
				row0[0] = worldToUV.get()[0]; // column 0, row 0
				row0[1] = worldToUV.get()[4]; // column 1, row 0
				row0[2] = worldToUV.get()[8]; // column 2, row 0
				row0[3] = worldToUV.get()[12]; // column 3, row 0 (translation)
				// Row 1: maps world position to V coordinate (inverted Y for UV)
				row1[0] = -worldToUV.get()[1];
				row1[1] = -worldToUV.get()[5];
				row1[2] = -worldToUV.get()[9];
				row1[3] = 1.0f - (-worldToUV.get()[13]); // offset for V flip
				drv->setUniform4f(IDriver::VertexProgram, vp->idx().WorldToUV0, row0[0], row0[1], row0[2], row0[3]);
				drv->setUniform4f(IDriver::VertexProgram, vp->idx().WorldToUV1, row1[0], row1[1], row1[2], row1[3]);

				// Camera position (world space)
				drv->setUniform4f(IDriver::VertexProgram, vp->idx().RefCamDist, camPos.x, camPos.y, camPos.z, 1.f);

				// Diffuse color (normalized to [0,1])
				CRGBA diff = decal->getDiffuse();
				drv->setUniform4f(IDriver::VertexProgram, vp->idx().Diffuse, diff.R * (1.f / 255.f), diff.G * (1.f / 255.f), diff.B * (1.f / 255.f), 1.f);

				// Bottom & top blend scale/bias
				float bottomBlendScale = 1.f / decal_favoid0(decal->getBottomBlendZMax() - decal->getBottomBlendZMin());
				float topBlendScale = 1.f / decal_favoid0(decal->getTopBlendZMin() - decal->getTopBlendZMax());
				drv->setUniform4f(IDriver::VertexProgram, vp->idx().BlendScale,
					bottomBlendScale, -bottomBlendScale * decal->getBottomBlendZMin(),
					topBlendScale, -topBlendScale * decal->getTopBlendZMax());

				// MVP matrix
				drv->setUniformMatrix(IDriver::VertexProgram, vp->getUniformIndex(CProgramIndex::ModelViewProjection), IDriver::ModelViewProjection, IDriver::Identity);
			}

			uint32 length = (uint32)verts.size();
			uint32 offset = 0;
			const std::vector<CRGBA> &colors = decal->getColors();
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
						if (!vpActive && hasColors)
						{
							*(CRGBA *)vba.getColorPointer(count + i) = colors[offset + i];
						}
						else
						{
							*(CRGBA *)vba.getColorPointer(count + i) = CRGBA::White;
						}
					}
					offset += space;
					count += space;

					// Unlock, render, re-lock
					vba.unlock();
					vbLocked = false;
					nlassert(count % 3 == 0);
					drv->renderRawTriangles(*mat, 0, count / 3);
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
						if (!vpActive && hasColors)
						{
							*(CRGBA *)vba.getColorPointer(count + i) = colors[offset + i];
						}
						else
						{
							*(CRGBA *)vba.getColorPointer(count + i) = CRGBA::White;
						}
					}
					count += remaining;
					offset = length; // done with this decal
				}
			}
		}

		// Unlock and render any remaining vertices for the last decal
		if (vbLocked)
		{
			vba.unlock();
			vbLocked = false;
		}
		if (count > 0)
		{
			nlassert(count % 3 == 0);
			drv->renderRawTriangles(*mat, 0, count / 3);
		}
	}

	// Deactivate vertex program
	if (vpActive)
	{
		drv->activeVertexProgram(NULL);
	}
}
