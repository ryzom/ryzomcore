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

#ifndef NL_FACE_GRID_H
#define NL_FACE_GRID_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "vector_2s.h"
#include "surface_quad.h"
#include "chain.h"
#include "retrievable_surface.h"
#include "chain_quad.h"
#include "exterior_mesh.h"
#include "quad_grid.h"

#include "nel/pacs/u_global_position.h"



namespace NLPACS
{

/**
 * A selection grid for the interior faces. It contains the indexes of the faces, as uint32.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CFaceGrid
{
public:
	/**
	 * A temporary grid, used to create a static CFaceGrid object.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CFaceGridBuild
	{
	public:
		/// The grid.
		std::vector< std::vector<uint32> >	Grid;

		/// The width, in number of rows and lines of the grid.
		uint								Width;

		/// The size of each element.
		float								Size;

	public:
		/// Init an empty build object with width and size
		void								init(uint width, float elsize);

		/// Insert a value in the grid
		void								insert(const NLMISC::CVector &bmin, const NLMISC::CVector &bmax, uint32 value);
	};

protected:
	/// The width of the grid in rows and lines.
	uint16					_Width;
	/// The 2Log of the width of the grid.
	uint16					_Log2Width;

	/// The size (in meter) of each grid element.
	float					_ElSize;

	/// The grid of indexes to the data buffer.
	std::vector<uint32>		_Grid;
	/// The grid data buffer (packed datas).
	std::vector<uint32>		_GridData;

public:
	/// Constructor.
	CFaceGrid() : _Width(0), _Log2Width(0), _ElSize(0.0f) {}


	/// Clear the grid.
	void					clear();

	/// Create the face grid from a CFaceGridBuild.
	void					create(const CFaceGridBuild &fgb);

	/// Select faces indexes close to a given point
	void					select(const NLMISC::CVector &pos, std::vector<uint32> &selected) const;

	/// Serial the face grid
	void					serial(NLMISC::IStream &f);
};

// CFaceGrid inline

inline void CFaceGrid::clear()
{
	_Width = 0;
	_Log2Width = 0;
	_ElSize = 0.0f;
	NLMISC::contReset(_Grid);
	NLMISC::contReset(_GridData);
}

inline void CFaceGrid::create(const CFaceGrid::CFaceGridBuild &fgb)
{
	nlassert(fgb.Grid.size() == fgb.Width*fgb.Width);
	nlassert(fgb.Width < 32768);

	// clear first
	clear();

	// setup grid size
	_Width = (uint16)fgb.Width;
	_ElSize = fgb.Size;
	_Log2Width = uint16(NLMISC::getPowerOf2(_Width));

	// and store in packed format the data of the face grid build
	uint	i;
	for (i=0; i<fgb.Grid.size(); ++i)
	{
		_Grid.push_back((uint)_GridData.size());
		_GridData.insert(_GridData.end(), fgb.Grid[i].begin(), fgb.Grid[i].end());
	}
}

inline void	CFaceGrid::select(const NLMISC::CVector &pos, std::vector<uint32> &selected) const
{
	selected.clear();

	uint	start, stop, idx;
	uint	x, y;

	x = ((sint)(pos.x/_ElSize) & (_Width-1));
	y = ((sint)(pos.y/_ElSize) & (_Width-1));

	idx = x+(y<<_Log2Width);

	start = _Grid[idx++];
	stop = (idx == _Grid.size()) ? (uint)_GridData.size() : _Grid[idx];

	for (; start<stop; ++start)
		selected.push_back(_GridData[start]);
}

inline void	CFaceGrid::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version
	*/
	(void)f.serialVersion(0);

	f.serial(_Width, _Log2Width, _ElSize);
	f.serialCont(_Grid);
	f.serialCont(_GridData);
}



// CFaceGridBuild inline

inline void	CFaceGrid::CFaceGridBuild::init(uint width, float elsize)
{
	nlassert(NLMISC::isPowerOf2(width));
	Width = width;
	Size = elsize;
	Grid.clear();
	Grid.resize(Width*Width);
}

inline void	CFaceGrid::CFaceGridBuild::insert(const NLMISC::CVector &bmin,
											  const NLMISC::CVector &bmax,
											  uint32 value)
{
	sint	x0 = (sint)(bmin.x/Size),
			x1 = (sint)(bmax.x/Size),
			y0 = (sint)(bmin.y/Size),
			y1 = (sint)(bmax.y/Size);

	if (x1-x0 >= (sint)Width)
	{
		x0 = 0;
		x1 = Width-1;
	}
	else
	{
		x0 &= (Width-1);
		x1 &= (Width-1);
		if (x1 < x0)
			x1 += Width;
	}

	if (y1-y0 >= (sint)Width)
	{
		y0 = 0;
		y1 = Width-1;
	}
	else
	{
		y0 &= (Width-1);
		y1 &= (Width-1);
		if (y1 < y0)
			y1 += Width;
	}

	sint	x, y;

	for (y=y0; y<=y1; ++y)
		for (x=x0; x<=x1; ++x)
			Grid[(x&(Width-1))+(y&(Width-1))*Width].push_back(value);
}


}; // NLPACS

#endif // NL_FACE_GRID_H

/* End of face_grid.h */
