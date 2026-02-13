/** \file decal_manager.cpp
 * Batched decal rendering manager for NeL 3D.
 *
 * Implements the rendering pseudocode from the intern report (§.6.3).
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

using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
CDecalManager::CDecalManager() :
		_NextMaterialId(0),
		_UseVertexProgram(false)
{
	// Fixed-size AGP Volatile vertex buffer with Position + TexCoord0 (see PDF §4.5.4)
	_VB.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
	_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	_VB.setNumVertices(NL3D_DECAL_VB_MAX_VERTICES);
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
// for each material {
//     count = 0; offset = 0
//     while there are remaining decals {
//         length = number of vertices for current decal
//         if count + length - offset > array buffer size {
//             copy partial, render full buffer, count = 0
//         } else {
//             copy remaining, increment decal
//         }
//     }
//     if count > 0 { render remaining }
// }
void CDecalManager::flush(CScene *sc)
{
	if (_Decals.empty())
		return;

	IDriver *drv = sc->getRenderTrav().getDriver();
	drv->activeVertexProgram(NULL);
	drv->activeVertexBuffer(_VB);
	drv->setupModelMatrix(CMatrix::Identity);

	// Iterate over each material group
	TDecalMap::iterator matIt = _Decals.begin();
	TDecalMap::iterator matEnd = _Decals.end();

	for (; matIt != matEnd; ++matIt)
	{
		uint32 materialId = matIt->first;
		std::vector<CDecal*> &decals = matIt->second;

		if (decals.empty())
			continue;

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

		for (uint d = 0; d < decals.size(); ++d)
		{
			CDecal *decal = decals[d];

			// Get this decal's computed vertices and UVs
			std::vector<CVector> &verts = decal->getVertices(_UseVertexProgram);
			const std::vector<CUV> &uvs = decal->getUVs();

			if (verts.empty())
				continue;

			uint32 length = (uint32)verts.size();
			uint32 offset = 0;

			while (offset < length)
			{
				uint32 remaining = length - offset;
				uint32 space = NL3D_DECAL_VB_MAX_VERTICES - count;

				if (remaining > space)
				{
					// Not enough space: fill what we can, render, reset
					{
						CVertexBufferReadWrite vba;
						_VB.lock(vba);
						for (uint32 i = 0; i < space; ++i)
						{
							*vba.getVertexCoordPointer(count + i) = verts[offset + i];
							if (offset + i < uvs.size())
								*vba.getTexCoordPointer(count + i, 0) = uvs[offset + i];
						}
					}
					offset += space;
					count += space;

					// Render the full buffer
					nlassert(count % 3 == 0);
					drv->renderRawTriangles(*mat, 0, count / 3);
					count = 0;
				}
				else
				{
					// Enough space: copy all remaining
					{
						CVertexBufferReadWrite vba;
						_VB.lock(vba);
						for (uint32 i = 0; i < remaining; ++i)
						{
							*vba.getVertexCoordPointer(count + i) = verts[offset + i];
							if (offset + i < uvs.size())
								*vba.getTexCoordPointer(count + i, 0) = uvs[offset + i];
						}
					}
					count += remaining;
					offset = length; // done with this decal
				}
			}
		}

		// Render any remaining vertices for this material
		if (count > 0)
		{
			nlassert(count % 3 == 0);
			drv->renderRawTriangles(*mat, 0, count / 3);
		}
	}
}
