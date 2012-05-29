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




#include "stdpch.h"

#include "precipitation_clip_grid.h"


using namespace NLMISC;
using namespace NL3D;


//CHeightGrid HeightGrid;


extern UVisualCollisionManager	*CollisionManager;
/////////////////
// CHeightGrid //
/////////////////


/*
// ***********************************************************************************
void	CHeightGrid::tileAdded(const CTileAddedInfo &infos)
{
	// insert in point grid
	_TileInfos[infos.TileID] = infos;
	CVector2f corners[3];
	corners[0].set(infos.Corners[0].x, infos.Corners[0].y);
	corners[1].set(infos.Corners[1].x, infos.Corners[1].y);
	corners[2].set(infos.Corners[2].x, infos.Corners[2].y);
	addTri(corners, infos.Center.z);
	corners[0].set(infos.Corners[0].x, infos.Corners[0].y);
	corners[1].set(infos.Corners[2].x, infos.Corners[2].y);
	corners[2].set(infos.Corners[3].x, infos.Corners[3].y);
	addTri(corners, infos.Center.z);
}

// ***********************************************************************************
void	CHeightGrid::tileRemoved(uint64 id)
{
	CHashMap<uint64, CTileAddedInfo>::iterator it = _TileInfos.find(id);
	nlassert(it != _TileInfos.end());
	CVector2f corners[3];
	const CTileAddedInfo &infos = it->second;
	corners[0].set(infos.Corners[0].x, infos.Corners[0].y);
	corners[1].set(infos.Corners[1].x, infos.Corners[1].y);
	corners[2].set(infos.Corners[2].x, infos.Corners[2].y);
	removeTri(corners, infos.Center.z);
	corners[0].set(infos.Corners[0].x, infos.Corners[0].y);
	corners[1].set(infos.Corners[2].x, infos.Corners[2].y);
	corners[2].set(infos.Corners[3].x, infos.Corners[3].y);
	removeTri(corners, infos.Center.z);
}

// ***********************************************************************************
void CHeightGrid::addTri(const CVector2f corners[3], float z)
{
	_Tri.Vertices.resize(3);
	_Tri.Vertices[0] = corners[0];
	_Tri.Vertices[1] = corners[1];
	_Tri.Vertices[2] = corners[2];
	sint minY;
	_Tri.computeBorders(_Rasters, minY);
	if (_Rasters.empty()) return;
	sint numRasters = _Rasters.size();
	CHeightGridWrapped::CGridElem ge;
	ge.Z = z;
	for(sint y = 0; y < numRasters; ++y)
	{
		ge.Y = y + minY;
		const CPolygon2D::TRaster &r = _Rasters[y];
		for (ge.X = r.first; ge.X <= r.second; ++ge.X)
		{
			_ZGridWrapped.insert(ge);
			if (ge.X >= _PosX && ge.X < _PosX + (sint) _GridSize &&
				ge.Y >= _PosY && ge.Y < _PosY + (sint) _GridSize
			   )
			{
				_ZGrid(ge.X - _PosX, ge.Y - _PosY) = std::max(z, _ZGrid(ge.X - _PosX, ge.Y - _PosY));
			}
		}
	}
}

// ***********************************************************************************
void CHeightGrid::removeTri(const CVector2f corners[3], float z)
{
	_Tri.Vertices.resize(3);
	_Tri.Vertices[0] = corners[0];
	_Tri.Vertices[1] = corners[1];
	_Tri.Vertices[2] = corners[2];
	sint minY;
	_Tri.computeBorders(_Rasters, minY);
	if (_Rasters.empty()) return;
	sint numRasters = _Rasters.size();
	CHeightGridWrapped::CGridElem ge;
	ge.Z = z;
	for(sint y = 0; y < numRasters; ++y)
	{
		ge.Y = y + minY;
		const CPolygon2D::TRaster &r = _Rasters[y];
		for (ge.X = r.first; ge.X <= r.second; ++ge.X)
		{
			_ZGridWrapped.remove(ge);
		}
	}
}

// ***********************************************************************************
CHeightGrid::CHeightGrid()
{
	_CellSize = 0.f;
	_GridSize = 0;
	_MinZ = HEIGHT_GRID_MIN_Z;
	_PosX = 0;
	_PosY = 0;
	_SizeMask = 0;
	_InvCellSize = 1.f;
	updateBBox();
}



// ***********************************************************************************
void CHeightGrid::init(float cellSize, uint heightGridSize, uint wrappedHeightGridSize, float minZ)
{
	nlassert(cellSize > 0.f);
	nlassert(heightGridSize > 0);
	nlassert(NLMISC::isPowerOf2(heightGridSize));
	nlassert(NLMISC::isPowerOf2(wrappedHeightGridSize));
	_ZGrid.init(heightGridSize, heightGridSize, minZ);
	_ZGridWrapped.init(wrappedHeightGridSize, cellSize);
	_MinZ = minZ;
	_CellSize = cellSize;
	_GridSize = heightGridSize;
	_InvCellSize = 1.f / _CellSize;
	updateBBox();
}

// ***********************************************************************************
void CHeightGrid::updateBBox()
{
	_BBox.setMinMax(CVector(_PosX * _CellSize, _PosY * _CellSize, _MinZ),
		            CVector((_PosX + _GridSize) * _CellSize, (_PosY + _GridSize) * _CellSize, -_MinZ));
}

// ***********************************************************************************
void CHeightGrid::update(const CVector &newPos)
{
	if (_CellSize == 0.f) return;
	sint newX = (sint) floorf(newPos.x / _CellSize) - (_GridSize >> 1);
	sint newY = (sint) floorf(newPos.y / _CellSize) - (_GridSize >> 1);
	if (newX == _PosX && newY == _PosY) return;
	// compute displacement of the grid
	sint offsetX = _PosX - newX;
	sint offsetY = _PosY - newY;
	// compute parts of the grid that have been discarded
	_ZGrid.getDiscardRects(offsetX, offsetY, _UpdateRects);
	for(uint k = 0; k < _UpdateRects.size(); ++k)
	{
		discardRect(_UpdateRects[k]);
	}
	_ZGrid.move(offsetX, offsetY);
	updateBBox();
	// compute parts of the grid that must be updated
	_ZGrid.getUpdateRects(offsetX, offsetY, _UpdateRects);
	_PosX = newX;
	_PosY = newY;
	for(uint k = 0; k < _UpdateRects.size(); ++k)
	{
		updateRect(_UpdateRects[k]);
	}

}

// ***********************************************************************************
void CHeightGrid::addCollisionMesh(const UVisualCollisionManager::CMeshInstanceColInfo &colMesh)
{
	const std::vector<CVector> &vertices = colMesh.Mesh.getVertices();
	_CurrMeshVertices.resize(vertices.size());
	std::vector<CVector>::const_iterator src, srcEnd;
	std::vector<CVector>::iterator dest;
	srcEnd = _CurrMeshVertices.end();
	dest = _CurrMeshVertices.begin();
	// transform vertices in world
	for (src = vertices.begin(); src != srcEnd; ++ src)
	{
		*dest = *colMesh.WorldMatrix * *src;
		++ dest;
	}
	std::vector<uint16> tris = colMesh.Mesh.getTriangles();
	uint triSize = tris.size();
	uint currVert = 0;
	// Insert tri in the grid
	// Because of low resolution, only a few tri will actually be inserted
	while (currVert != triSize)
	{
		const CVector &v0 = _CurrMeshVertices[tris[currVert]];
		const CVector &v1 = _CurrMeshVertices[tris[currVert + 1]];
		const CVector &v2 = _CurrMeshVertices[tris[currVert + 2]];
		// project corners in 2D
		CVector2f corners[3];
		corners[0].set(v0.x, v0.y);
		corners[1].set(v1.x, v1.y);
		corners[2].set(v2.x, v2.y);
		addTri(corners, maxof(v0.z, v1.z, v2.z));
		currVert += 3;
	}
	_Meshs[colMesh.ID] = colMesh;
}

// ***********************************************************************************
void CHeightGrid::removeCollisionMesh(uint id)
{
	TColMeshMap::iterator it = _Meshs.find(id);
	if (it == _Meshs.end()) return;
	const std::vector<CVector> &vertices = it->second.Mesh.getVertices();
	_CurrMeshVertices.resize(vertices.size());
	std::vector<CVector>::const_iterator src, srcEnd;
	std::vector<CVector>::iterator dest;
	srcEnd = vertices.end();
	dest = _CurrMeshVertices.begin();
	// transform vertices in world
	const CMatrix &worldMat = *(it->second.WorldMatrix);
	for (src = vertices.begin(); src != srcEnd; ++ src)
	{
		*dest = worldMat * *src;
		++ dest;
	}
	std::vector<uint16> tris = it->second.Mesh.getTriangles();
	uint triSize = tris.size();
	uint currVert = 0;
	while (currVert != triSize)
	{
		const CVector v0 = _CurrMeshVertices[tris[currVert]];
		const CVector v1 = _CurrMeshVertices[tris[currVert + 1]];
		const CVector v2 = _CurrMeshVertices[tris[currVert + 2]];
		// project corners in 2D
		CVector2f corners[3];
		corners[0].set(v0.x, v0.y);
		corners[1].set(v1.x, v1.y);
		corners[2].set(v2.x, v2.y);
		removeTri(corners, maxof(v0.z, v1.z, v2.z));
		currVert += 3;
	}
	_Meshs.erase(it);
}


// ***********************************************************************************
void CHeightGrid::updateRect(const NLMISC::CRect &rect)
{
	// add incoming meshs into the quad grid
	CAABBox bbox;
	bbox.setMinMax(CVector(rect.X * _CellSize, rect.Y * _CellSize, _MinZ), CVector((rect.X + rect.Width) * _CellSize, (rect.Y + rect.Height) * _CellSize, - _MinZ));
	CollisionManager->getMeshs(bbox, _UpdateMeshs);
	for(uint k = 0; k < _UpdateMeshs.size(); ++k)
	{
		if (_Meshs.count(_UpdateMeshs[k].ID) == 0)
		{
			// not already inserted
			addCollisionMesh(_UpdateMeshs[k]);
		}
	}
	// Update height grid from the values in the quad grid
	for (uint y = (uint) rect.Y; y < (uint) (rect.Y + rect.Height); ++y)
	{
		for (uint x = (uint) rect.X; x < (uint) (rect.X + rect.Width); ++x)
		{
			updateCell(x, y);
		}
	}
}

// ***********************************************************************************
void CHeightGrid::discardRect(const CRect &rect)
{
	// remove meshs that are totally out of the grid
	CAABBox bbox;
	bbox.setMinMax(CVector(rect.X * _CellSize, rect.Y * _CellSize, _MinZ), CVector((rect.X + rect.Width) * _CellSize, (rect.Y + rect.Width) * _CellSize, - _MinZ));
	CollisionManager->getMeshs(bbox, _UpdateMeshs);
	for(uint k = 0; k < _UpdateMeshs.size(); ++k)
	{
		if (!_UpdateMeshs[k].WorldBBox->intersect(_BBox))
		{
			removeCollisionMesh(_UpdateMeshs[k].ID); // remove mesh (no-op if already removed)
		}
	}
}

// ***********************************************************************************
void CHeightGrid::updateCell(sint px, sint py)
{
	sint worldX = px + _PosX;
	sint worldY = py + _PosY;
	float z = _MinZ;
	const CHeightGridWrapped::TGridElemList &pl = _ZGridWrapped.getGridElemList(worldX, worldY);
	for(CHeightGridWrapped::TGridElemList::const_iterator it = pl.begin(); it != pl.end(); ++it)
	{
		const CHeightGridWrapped::CGridElem &ge = *it;
		if (ge.X ==  worldX && ge.Y == worldY)
		{
			z = std::max(z, ge.Z);
		}
	}
	_ZGrid(px, py) = z;
}


// ***********************************************************************************
void	CHeightGrid::display(NL3D::UDriver &drv) const
{
	extern UCamera MainCam;
	drv.setModelMatrix(CMatrix::Identity);
	drv.setViewMatrix(MainCam.getMatrix().inverted());
	drv.setFrustum(MainCam.getFrustum());
	UMaterial m = drv.createMaterial();
	m.initUnlit();
	m.setColor(CRGBA::Red);
	for (sint y = _PosY; y < (sint)  (_PosY + _GridSize - 1); ++y)
	{
		for (sint x = _PosX; x < (sint) (_PosX + _GridSize - 1); ++x)
		{
			float z0 = _ZGrid(x - _PosX, y - _PosY);
			float z1 = _ZGrid(x + 1 - _PosX, y - _PosY);
			float z2 = _ZGrid(x + 1 - _PosX, y + 1 - _PosY);
			float z3 = _ZGrid(x - _PosX, y + 1 - _PosY);
			//
			CVector pos[4];
			CVector pos0 = gridCoordToWorld(x, y) + z0 * CVector::K;
			CVector pos1 = gridCoordToWorld(x + 1, y) + z1 * CVector::K;
			CVector pos2 = gridCoordToWorld(x + 1, y + 1) + z2 * CVector::K;
			CVector pos3 = gridCoordToWorld(x, y + 1) + z3 * CVector::K;
			drv.drawLine(CLine(pos0, pos1), m);
			drv.drawLine(CLine(pos1, pos2), m);
			drv.drawLine(CLine(pos2, pos3), m);
			drv.drawLine(CLine(pos3, pos0), m);
		}
	}
	m.setColor(CRGBA::Green);
	// draw dots of the point grid
	//for (sint y = 0; y < (sint) _PointGrid.getSize(); ++y)
	//{
	//	for (sint x = 0; x < (sint) _PointGrid.getSize(); ++x)
	//	{
	//		const CPointGrid::TPointList &pl = _PointGrid.getPointList(x ,y);
	//		for(CPointGrid::TPointList::const_iterator it = pl.begin(); it != pl.end(); ++it)
	//		{
	//			drv.drawLine(CLine(*it, *it + CVector::K), m);
	//		}
	//	}
	//}

	drv.deleteMaterial(m);
	// info : check max size of vectors
	nlwarning("Max list length = %d", (int) _ZGridWrapped.getListMaxLength());
}

////////////////
// CPointGrid //
////////////////

// *********************************************************************************
CHeightGridWrapped::CHeightGridWrapped()
{
	_CellSize = 0.f;
	_SizeMask = 0;
	_InvCellSize = 1.f;
}

// *********************************************************************************
uint CHeightGridWrapped::getListMaxLength() const
{
	uint sizeMax = 0;
	for(CArray2D<TGridElemList>::const_iterator it = _Grid.begin(); it != _Grid.end(); ++it)
	{
		sizeMax = std::max(sizeMax, (uint) it->size());
	}
	return sizeMax;
}

// *********************************************************************************
void CHeightGridWrapped::init(uint size, float cellSize)
{
	nlassert(cellSize > 0.f)
	nlassert(NLMISC::isPowerOf2(size));
	uint sizePower= NLMISC::getPowerOf2(size);
	_SizeMask = (1 << sizePower) - 1;
	_CellSize = cellSize;
	_InvCellSize = 1.f / _CellSize;
	_Grid.init(size, size);
}
*/

//=====================================================================
CPrecipitationClipGrid::CPrecipitationClipGrid()
{
	clear();
	touch();
}

//=====================================================================
void CPrecipitationClipGrid::initGrid(uint size, float gridEltSizeX, float gridEltSizeY)
{
	if (gridEltSizeX <= 0.f || gridEltSizeY <= 0.f || size == 0)
	{
		clear();
		return;
	}
	_Grid.clear();
	_Grid.resize((size + 1) * (size + 1));
	_XPos = 0;
	_YPos = 0;
	_EltSizeX = gridEltSizeX;
	_EltSizeY = gridEltSizeY;
	_Size = size;
}

//=====================================================================
void CPrecipitationClipGrid::updateGrid(const NLMISC::CVector &/* userPos */, NLPACS::UGlobalRetriever * /* gr */)
{
	// temp
	return;
	//
/*
	if (_EltSizeX == 0.f) return;
	sint newX = (sint) floorf(userPos.x / _EltSizeX) - (_Size >> 1);
	sint newY = (sint) floorf(userPos.y / _EltSizeY) - (_Size >> 1);

	if (_Touched)
	{
		#if defined(NL_CPU_INTEL) && defined(NL_DEBUG)
			CSimpleClock clock;
			CSimpleClock::init();
			clock.start();
		#endif
		// recompute the whole grid
		updateGridPart(newX, newY, 0, 0, _Size + 1, _Size + 1, gr);
		#if defined(NL_CPU_INTEL) && defined(NL_DEBUG)
			clock.stop();
			// display number of millisecond needed for the update
			double freq = (double) CSystemInfo::getProcessorFrequency();
			double msPerTick = 1000 / (double) freq;
			nldebug("Updated precipitation clip grid : time = %.2f ms", (float) (clock.getNumTicks() * msPerTick));
			// display the height in the clip grid
			printGridValues();
		#endif
		// update pos
		_XPos = newX;
		_YPos = newY;
		_Touched = false;
		return;
	}

	if (newX == _XPos && newY == _YPos)
	{
		return; // the grid hasn't changed its pos
	}

	// move the grid
	sint offsetX = newX - _XPos;
	sint offsetY = newY - _YPos;

	moveGrid(- offsetX, - offsetY);
	// complete new positions

	#if defined(NL_CPU_INTEL) && defined(NL_DEBUG)
		CSimpleClock clock;
		CSimpleClock::init();
		clock.start();
	#endif

	// Update right or left part.
	if (newX > _XPos) // moved right ?
	{
		// the width to update
		uint width = std::min((uint) offsetX, _Size + 1);
		// the grid moved top or bottom, exclude this part
		sint height = _Size + 1 - abs(offsetY);
		if (height > 0)
		{
			// complete column on the right
			updateGridPart(newX + _Size + 1 - width, newY - offsetY, _Size + 1 - width, std::max(-offsetY, 0), width, height, gr);
		}
	}
	else if (newX < _XPos) // moved left ?
	{
		// the width to update
		uint width = std::min((uint) (- offsetX), _Size + 1);
		// the grid moved top or bottom, exclude
		sint height = _Size + 1 - abs(offsetY);
		if (height > 0)
		{
			// complete column on the right
			updateGridPart(newX, newY - offsetY, 0, std::max(-offsetY, 0), width, height, gr);
		}
	}

	// update top or bottom part
	if (newY > _YPos)
	{
		sint height = std::min((uint) offsetY, _Size + 1);
		updateGridPart(newX, newY + _Size + 1 - height, 0, _Size + 1 - height, _Size + 1, height, gr);
	}
	else
	if (newY < _YPos)
	{
		sint height = std::min((uint) (- offsetY), _Size + 1);
		updateGridPart(newX, newY, 0, 0, _Size + 1, height, gr);
	}

	#if defined(NL_CPU_INTEL) && defined(NL_DEBUG)
		clock.stop();
		// display number of millisecond needed for the update
		double freq = (double) CSystemInfo::getProcessorFrequency();
		double msPerTick = 1000 / (double) freq;
		nldebug("Updated precipitation clip grid : time = %.2f ms", (float) (clock.getNumTicks() * msPerTick));
		// display the height in the clip grid
		printGridValues();
	#endif

	// update pos
	_XPos = newX;
	_YPos = newY;
*/
}

//=====================================================================
void CPrecipitationClipGrid::printGridValues() const
{
	for(uint k = 0; k <= _Size; ++k)
	{
		std::string outStr;
		for(uint l = 0; l <= _Size; ++l)
		{
			TGrid::const_iterator gridIt = getGridIt(_Grid, l, k, _Size);
			outStr += toString("%5d%c ", (uint) gridIt->MeanHeight, l == _Size ? ' ' : ',');
		}
		nlinfo(outStr.c_str());
	}
}

//=====================================================================
void CPrecipitationClipGrid::moveGrid(sint offsetX, sint offsetY)
{
	static TGrid otherGrid; // the grid is likely to be rather small so it's ok to keep it static
	otherGrid.resize(_Grid.size());
	// get start pos in the source
	uint srcX, srcY;
	sint width, height;
	// compute position in source & size
	if (offsetX > 0)
	{
		srcX = 0;
		width = (_Size + 1) - offsetX;
	}
	else
	{
		srcX = -offsetX;
		width = (_Size + 1) - srcX;
	}
	if (offsetY > 0)
	{
		srcY = 0;
		height = (_Size + 1) - offsetY;
	}
	else
	{
		srcY = -offsetY;
		height = (_Size + 1) - srcY;
	}
	if (width > 0 && height > 0)
	{
		TGrid::const_iterator srcIt = getGridIt(_Grid, srcX, srcY, _Size);
		TGrid::iterator       dstIt = getGridIt(otherGrid, srcX + offsetX, srcY + offsetY, _Size);
		do
		{
			// copy one row of the grid
			std::copy(srcIt, srcIt + width, dstIt);
			// go to next row for source & destination
			srcIt += (_Size + 1);
			dstIt += (_Size + 1);
		}
		while (--height);
		_Grid.swap(otherGrid); // replace previous grid
	}
}

//=====================================================================
const CPrecipitationClipGrid::CGridPoint *CPrecipitationClipGrid::get(sint x, sint y) const
{
	// pos in the grid
	sint gx = x - _XPos;
	sint gy = y - _YPos;
	if ((uint) gx > (_Size + 1) || (uint) gy > (_Size + 1)) return NULL;
	return &(*getGridIt(_Grid, gx, gy, _Size));
}

//=====================================================================
void CPrecipitationClipGrid::clear()
{
	 _XPos = 0;
	 _YPos = 0;
	 _Size = 0;
	 _EltSizeX = 0;
	 _EltSizeY = 0;
	 _Grid.clear();
}

//=====================================================================
void CPrecipitationClipGrid::updateGridPart(sint worldX, sint worldY, uint gridX, uint gridY, uint width, uint height, NLPACS::UGlobalRetriever *gr)
{
	nlassert(gridX <= _Size);
	nlassert(gridY <= _Size);
	nlassert(gridX + width >= gridX && gridX + width <= _Size + 1);
	nlassert(gridY + height >= gridY && gridY + height <= _Size + 1);
	NLMISC::CVector worldPos(worldX * _EltSizeX, worldY * _EltSizeY, 10000.f);
	TGrid::iterator dest = getGridIt(_Grid, gridX, gridY, _Size);
	// fill each row
	for (uint y = 0; y < height; ++y)
	{
		TGrid::iterator currDest = dest;
		NLMISC::CVector currWorldPos = worldPos;
		// fill a row
		for (uint x = 0; x < width; ++x)
		{
			if (!gr)
			{
				currDest->Clipped = true;
				currDest->MeanHeight = 0.f;
			}
			else
			{
				// build a global position
				NLPACS::UGlobalPosition gpos = gr->retrievePosition(currWorldPos, 20000.f);
				if (gpos.InstanceId != -1)
				{
					// is it an interior ?
					if (gr->isInterior(gpos))
					{
						currDest->Clipped = true;
						currDest->MeanHeight = 0.f;
					}
					else
					{
						// get mean height at this point
						currDest->Clipped = false;
						currDest->MeanHeight = gr->getMeanHeight(gpos);
					}
				}
				else
				{
					currDest->MeanHeight = 0.f;
				}
			}
			++ currDest;
			currWorldPos.x += _EltSizeX;
		}
		// move to next row
		dest += (_Size + 1);
		currWorldPos.y += _EltSizeY;
	}
}

//=====================================================================
void CPrecipitationClipGrid::display(NL3D::UDriver &drv) const
{
	// get render context
	CMatrix  modelMat = drv.getModelMatrix();
	CMatrix  viewMat  = drv.getViewMatrix();
	CFrustum f     = drv.getFrustum();

	drv.setMatrixMode2D(CFrustum(0, 800, 600, 0, 0, 1, false));

	const uint stepX = 16;
	const uint stepY = 16;
	for (uint y = 0; y < _Size + 1; ++y)
	{
		for (uint x = 0; x < _Size + 1; ++x)
		{
			TGrid::const_iterator gridPoint = getGridIt(_Grid, x, y, _Size);
			CRGBA col = gridPoint->Clipped ? CRGBA::Red : CRGBA::White;
			drv.drawLine((float) (x * stepX - 1), (float) (y * stepY), (float) (x * stepX + 1), (float) (y * stepY), col);
			drv.drawLine((float) (x * stepX), (float) (y * stepY - 1), (float) (x * stepX), (float) (y * stepY + 1), col);
		}
	}

	// restore render context
	drv.setModelMatrix(modelMat);
	drv.setViewMatrix(viewMat);
	drv.setFrustum(f);
}

























