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

#include "std3d.h"
#include "nel/3d/quad_grid.h"

namespace NL3D
{
	NLMISC::CPolygon2D CQuadGridBase::_ScaledPoly;
	NLMISC::CPolygon2D::TRasterVect CQuadGridBase::_PolyBorders;
	std::vector<uint> CQuadGridBase::_AlreadySelected;
	uint CQuadGridBase::_SelectStamp = 0;
} // NL3D
