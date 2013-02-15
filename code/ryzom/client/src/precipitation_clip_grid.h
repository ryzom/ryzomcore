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


#ifndef CL_PRECIPITATION_CLIP_GRID_H
#define CL_PRECIPITATION_CLIP_GRID_H

#include "nel/3d/u_landscape.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/misc/traits_nl.h"
#include "nel/misc/polygon.h"


/*

// *********************************************************************************
template <class T> class CArray2D
{
public:
	typedef typename std::vector<T> TArrayContainer;
	typedef typename TArrayContainer::iterator iterator;
	typedef typename TArrayContainer::const_iterator const_iterator;
	CArray2D() : _Width(0), _Height(0) {}
	void init(uint width, uint height);
	void init(uint width, uint height, const T &defaultValue);
	bool empty() const { return _Array.empty(); }
	typename iterator begin() { return _Array.begin(); }
	typename iterator end() { return _Array.end(); }
	typename const_iterator begin() const { return _Array.begin(); }
	typename const_iterator end() const { return _Array.end(); }

	// access element by column/row
	T &operator()(uint x, uint y)
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array[x + y * _Width];
	}
	// access element by column/row (const version)
	const T &operator()(uint x, uint y) const
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array[x + y * _Width];
	}
	// Return width of array
	uint getWidth() const { return _Width; }
	// Return height of array
	uint getHeight() const { return _Height; }
	// Move array content of the given offset. No wrapping is applied
	// Example : move(1, 0) will move the array of one column to the left. The latest column is lost. The first column remains unchanged
	//
	void move(sint offsetX, sint offsetY);
	// Move a part of the array. Values are clamped as necessary
	void moveSubArray(sint dstX, sint dstY, sint srcX, sint srcY, sint width, sint height);
	// get an iterator to the start of a row
	iterator beginRow(uint row)
	{
		nlassert(row < _Height);
		return _Array.begin() + row * _Width;
	}
	const_iterator beginRow(uint row) const
	{
		nlassert(row < _Height);
		return _Array.begin() + row * _Width;
	}
	iterator endRow(uint row)
	{
		nlassert(row < _Height);
		return _Array.begin() + (row + 1) * _Width;
	}
	const_iterator endRow(uint row) const
	{
		nlassert(row < _Height);
		return _Array.begin() + (row + 1) * _Width;
	}
	// get an iterator at the given position
	iterator getIteratorAt(uint x, uint y)
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array.begin() + x + (y * _Width);
	}
	// Get a const iterator at the given position
	const iterator getIteratorAt(uint x, uint y) const
	{
		#ifdef NL_DEBUG
			nlassert(x < _Width);
			nlassert(y < _Height);
		#endif
		return _Array.begin() + x + (y * _Width);
	}
	// See which part of array should be updated after its content has been displaced by the given offset (by a call to move for example).
	// Example: getUpdateRects(0, 1, result) will result the first row as a result
	//
	void getUpdateRects(sint moveOffsetX, sint moveOffsetY, std::vector<CRect> &rectsToUpdate);
	// See which parts of array will be discarded if the array is displaced by the given offset
	void getDiscardRects(sint moveOffsetX, sint moveOffsetY, std::vector<CRect> &discardedRects);
private:
	TArrayContainer _Array;
	uint _Width;
	uint _Height;
private:
	inline void checkRect(const CRect &r) const
	{
		nlassert(r.X >= 0 && r.X < (sint32) _Width);
		nlassert(r.Y >= 0 && r.Y < (sint32) _Height);
		nlassert(r.X + r.Width >= 0 && r.X + (sint32)  r.Width <= (sint32) _Width);
		nlassert(r.Y + r.Height >= 0 && r.Y + (sint32) r.Height <= (sint32) _Height);
	}
};


// *********************************************************************************
template <class T>
void CArray2D<T>::getUpdateRects(sint moveOffsetX, sint moveOffsetY, std::vector<CRect> &rectsToUpdate)
{
	rectsToUpdate.clear();
	if (moveOffsetX < 0) // moved right ?
	{
		// the width to update
		uint width = std::min((uint) moveOffsetX, _Width);
		// the grid moved top or bottom, exclude this part
		sint height = _Height - abs(moveOffsetY);
		if (height > 0)
		{
			nlwarning("*1");
			// complete column on the right
			rectsToUpdate.push_back(CRect((sint32) (_Width - width), (sint32) (std::max(- moveOffsetY, 0)), (uint32) width, (uint32) height));
			#ifdef NL_DEBUG
				checkRect(rectsToUpdate.back());
			#endif
		}
	}
	else if (moveOffsetX > 0) // moved left ?
	{
		// the width to update
		uint width = std::min((uint) (- moveOffsetX), _Width);
		// the grid moved top or bottom.
		sint height = _Height - abs(moveOffsetY);
		if (height > 0)
		{
			//nlwarning("*2");
			// complete column on the right
			rectsToUpdate.push_back(CRect(0, (sint32) std::max(- moveOffsetY, 0), (uint32) width, (uint32) height));
			#ifdef NL_DEBUG
				checkRect(rectsToUpdate.back());
			#endif
		}
	}
	// update top or bottom part
	if (moveOffsetY < 0)
	{
		sint height = std::min((uint) moveOffsetY, _Height);
		//nlwarning("*3");
		rectsToUpdate.push_back(CRect(0, _Height - height, _Width, height));
		#ifdef NL_DEBUG
			checkRect(rectsToUpdate.back());
		#endif
	}
	else
	if (moveOffsetY > 0)
	{
		sint height = std::min((uint) (- moveOffsetY), _Height);
		//nlwarning("*4");
		rectsToUpdate.push_back(CRect(0, 0, _Width, height));
		#ifdef NL_DEBUG
			checkRect(rectsToUpdate.back());
		#endif
	}
}

// *********************************************************************************
template <class T>
void CArray2D<T>::getDiscardRects(sint moveOffsetX, sint moveOffsetY,std::vector<CRect> &discardedRects)
{
	getUpdateRects(- moveOffsetX, - moveOffsetY, discardedRects);
}

// *********************************************************************************
template <class T>
void CArray2D<T>::moveSubArray(sint dstX, sint dstY, sint srcX, sint srcY, sint width, sint height)
{
	if (srcX >= (sint) getWidth()) return;
	if (srcY >= (sint) getHeight()) return;
	if (dstX >= (sint) getWidth()) return;
	if (dstY >= (sint) getHeight()) return;
	if (srcX < 0)
	{
		width += srcX;
		if (width <= 0) return;
		srcX = 0;
	}
	if (srcY < 0)
	{
		height += srcY;
		if (height <= 0) return;
		srcY = 0;
	}
	if (srcX + width > (sint) getWidth())
	{
		width = getWidth() - srcX;
	}
	if (srcY + height > (sint) getHeight())
	{
		height = getHeight() - srcY;
	}
	if (dstX < 0)
	{
		width += dstX;
		if (width < 0) return;
		srcX -= dstX;
		dstX = 0;
	}
	if (dstY < 0)
	{
		height += dstY;
		if (height < 0) return;
		srcY -= dstY;
		dstY = 0;
	}
	if (dstX + width > (sint) getWidth())
	{
		width =  getWidth() - dstX;
	}
	if (dstY + height > (sint) getHeight())
	{
		height =  getHeight() - dstY;
	}
	#ifdef NL_DEBUG
		nlassert(width > 0);
		nlassert(height > 0);
		nlassert(srcX >= 0 && srcX < (sint) getWidth());
		nlassert(srcY >= 0 && srcY < (sint) getHeight());
	#endif
	if (dstY < srcY)
	{
		const_iterator src = getIteratorAt(srcX, srcY);
		iterator dst = getIteratorAt(dstX, dstY);
		do
		{
			if (CTraits<T>::SupportRawCopy)
			{
				// type support fast copy
				::memcpy(&(*dst), &(*src), sizeof(T) * width);
			}
			else
			{
				std::copy(src, src + width, dst);
			}
			src += _Width;
			dst += _Width;
		}
		while(--height);
	}
	else if (dstY > srcY)
	{
		// copy from top to bottom
		const_iterator src = getIteratorAt(srcX, srcY + height - 1);
		iterator dst = getIteratorAt(dstX, dstY + height - 1);
		do
		{
			if (CTraits<T>::SupportRawCopy)
			{
				// type support fast copy
				::memcpy(&(*dst), &(*src), sizeof(T) * width);
			}
			else
			{
				std::copy(src, src + width, dst);
			}
			src -= _Width;
			dst -= _Width;
		}
		while(--height);
	}
	else
	{
		const_iterator src = getIteratorAt(srcX, srcY);
		iterator dst = getIteratorAt(dstX, dstY);
		if (dstX < srcX)
		{
			do
			{
				if (CTraits<T>::SupportRawCopy)
				{
					// type support fast copy
					::memmove(&(*dst), &(*src), sizeof(T) * width);
				}
				else
				{
					std::reverse_copy(src, src + width, dst);
				}
				src += _Width;
				dst += _Width;
			}
			while(--height);
		}
		else
		{
			do
			{
				if (CTraits<T>::SupportRawCopy)
				{
					// type support fast copy
					::memcpy(&(*dst), &(*src), sizeof(T) * width);
				}
				else
				{
					std::copy(src, src + width, dst);
				}
				src += _Width;
				dst += _Width;
			}
			while(--height);
		}
	}
}


// *********************************************************************************
template <class T>
void CArray2D<T>::move(sint offsetX, sint offsetY)
{
	moveSubArray(offsetX, offsetY, 0, 0, _Width, _Height);
}

// *********************************************************************************
template <class T>
void CArray2D<T>::init(uint width, uint height)
{
	_Array.resize(width * height);
	_Width = width;
	_Height = height;
}

// *********************************************************************************
template <class T>
void CArray2D<T>::init(uint width,uint height, const T &defaultValue)
{
	_Array.resize(width * height, defaultValue);
	_Width = width;
	_Height = height;
}


// *********************************************************************************
// A height grid with wrapping

class CHeightGridWrapped
{
public:
	class CGridElem
	{
	public:
		sint X;
		sint Y;
		float Z;
	public:
		bool operator == (const CGridElem &other) const
		{
			return X == other.X &&
				   Y == other.Y &&
				   Z == other.Z;
		}
	};
	typedef std::list<CGridElem> TGridElemList;
	// ctor
	CHeightGridWrapped();
	// Init with the given size
    // The size is required to be a power of 2
	void init(uint size, float cellSize);
	inline void insert(const CGridElem &ge);
	inline void remove(const CGridElem &ge);
	const TGridElemList &getGridElemList(sint x, sint y) const { return _Grid(x & _SizeMask, y & _SizeMask); }
	uint getSize() const { return _Grid.getWidth(); }
	// tmp for debug
	uint getListMaxLength() const;
private:
	CArray2D<TGridElemList> _Grid;
	float _CellSize;
	float _InvCellSize;
	uint  _SizeMask;
};



// *********************************************************************************
inline void  CHeightGridWrapped::insert(const CGridElem &ge)
{
	_Grid(ge.X & _SizeMask, ge.Y & _SizeMask).push_back(ge);
}

// *********************************************************************************
inline void  CHeightGridWrapped::remove(const CGridElem &ge)
{
	TGridElemList &gel = _Grid(ge.X & _SizeMask, ge.Y & _SizeMask);
	for(TGridElemList::iterator &it = gel.begin(); it != gel.end(); ++it)
	{
		if (*it == ge)
		{
			gel.erase(it);
			return;
		}
	}
	nlassert(0);
}

const float HEIGHT_GRID_MIN_Z = -10000.f;

// A height grid that give approximate z of geometry near the viewer.
// Useful to avoid precipitations in interiors / tunnels
//
class CHeightGrid : public NL3D::ULandscapeTileCallback
{
public:
	// ctor
	CHeightGrid();
	// Init the grid before first use
	//   \param cellSize  size of a cell of the height grid (in world unit)
	//   \param cellSize  heightGridSize width/height of the height grid. It must be a power of 2
	//   \param cellSize  wrappedHeightGridSize width/height of the wrapped height grid (for incoming geometry).
	//                    It must be a power of 2.
	//   \param minZ Minimum possible Z
	//
	void init(float cellSize, uint heightGridSize, uint wrappedHeightGridSize, float minZ = HEIGHT_GRID_MIN_Z);
	void update(const CVector &newPos);
	// for debug, display the height grid on screen
	void  display(NL3D::UDriver &drv) const;
	inline void gridCoordToWorld(sint x, sint y, CVector &dest) const;
	inline CVector gridCoordToWorld(sint x, sint y) const;
	// Remove a collision mesh (no op if not already added)
	void	removeCollisionMesh(uint id);
///////////////////////////////////////////////////////////////////////
private:
	typedef std::map<uint, UVisualCollisionManager::CMeshInstanceColInfo> TColMeshMap;
	typedef std::vector<UVisualCollisionManager::CMeshInstanceColInfo> TColMeshVect;
	CHeightGridWrapped						_ZGridWrapped;
	CArray2D<float>							_ZGrid;
	float									_CellSize;
	float									_InvCellSize;
	float									_MinZ;
	uint									_GridSize;
	uint									_SizeMask;
	sint									_PosX;
	sint									_PosY;
	std::vector<CRect>						_UpdateRects;
	CHashMap<uint64, CTileAddedInfo>	_TileInfos;
	CPolygon2D								_Tri;
	CPolygon2D::TRasterVect					_Rasters;
	TColMeshMap								_Meshs; // meshs currenlty inserted in the wrapped grid
	TColMeshVect							_UpdateMeshs; // mesh to be removed / added to the wrapped grid during the update (keep there to avoid vector allocation)
	std::vector<CVector>					_CurrMeshVertices; // vertices of the mesh being inserted
	CAABBox									_BBox; // bbox containing current grid
private:
	void updateBBox();
	void updateRect(const CRect &rect);
	void discardRect(const CRect &rect);
	void updateCell(sint x, sint y);
	// from ULandscapeTileCallback
	virtual void tileAdded(const CTileAddedInfo &infos);
	virtual void tileRemoved(uint64 id);
	// add a tri in the height grid, and update height grid if necessary
	void addTri(const CVector2f corners[3], float z);
	void removeTri(const CVector2f corners[3], float z);
	// Add a collision mesh (doesn't test if already inserted)
	void	addCollisionMesh(const UVisualCollisionManager::CMeshInstanceColInfo &colMesh);
};

// ***********************************************************************************
inline void CHeightGrid::gridCoordToWorld(sint x, sint y, CVector &dest) const
{
	dest.set(x * _CellSize, y * _CellSize, 0.f);
}

// ***********************************************************************************
inline CVector CHeightGrid::gridCoordToWorld(sint x, sint y) const
{
	CVector dest;
	gridCoordToWorld(x, y, dest);
	return dest;
}




// TMP TMP TMP TMP
// TMP TMP TMP TMP
// TMP TMP TMP TMP
// TMP TMP TMP TMP
extern CHeightGrid HeightGrid;


*/

/**
  * Class to know where precipitation can't fall
  * This is a grid that is updated with the position of the player
  * It tells when the quad is in an interior mesh, and gives the
  * mean height for a point of the quad (for landscape)
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */

class CPrecipitationClipGrid
{
public:
	/** Point of the grid
	  */
	struct CGridPoint
	{
		float MeanHeight; // The mean height that precipitation must use at this point
		bool  Clipped;    // True if clipped
		CGridPoint(float meanHeight = 0.f, bool clipped = true) : MeanHeight(meanHeight), Clipped(clipped)
		{}
	};
public:
	// ctor
	CPrecipitationClipGrid();
	/** init the grid
	  * \param size width & height of the grid so the grid has (size + 1) ^2 elements. For example if size is 1, the grid is a simple square and it has four corners.
	  * \param gridEltSize width & height of a grid element.
	  */
	void initGrid(uint size, float gridEltSizeX, float gridEltSizeY);

	// Update the grid so that it match the position of the user. (this grid is centered at the user pos)
	void updateGrid(const NLMISC::CVector &userPos, NLPACS::UGlobalRetriever *globalRetriever);

	/** Get grid element point its grid coordinate in world
	  * \return NULL if the element is outside the grid
	  */
	 const CGridPoint *get(sint x, sint y) const;

	 // for debug, display the clip grid on screen
	 void  display(NL3D::UDriver &drv) const;

	 // Force the whole grid to be recomputed
	 void  touch() { _Touched = true; }

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
private:
	typedef std::vector<CGridPoint> TGrid;
private:
	bool                    _Touched;
	sint				    _XPos;
	sint				    _YPos;
	uint				    _Size;
	float				    _EltSizeX;
	float				    _EltSizeY;
	TGrid                   _Grid;
private:
	void clear();
	void updateGridPart(sint worldX, sint worldY, uint gridX, uint gridY, uint width, uint height, NLPACS::UGlobalRetriever *globalRetriever);
	void updateGridElt(CGridPoint &dest, uint worldX, uint worldY);
	void moveGrid(sint offsetX, sint offsetY);
	void printGridValues() const;
	static TGrid::iterator getGridIt(TGrid &grid, uint x, uint y, uint size)
	{
		nlassert(size > 0);
		nlassert(x <= size);
		nlassert(y <= size);
		return grid.begin() + (x + (y * (size + 1)));
	}
	static TGrid::const_iterator getGridIt(const TGrid &grid, uint x, uint y, uint size)
	{
		nlassert(size > 0);
		nlassert(x <= size);
		nlassert(y <= size);
		return grid.begin() + (x + (y * (size + 1)));
	}
};


#endif

