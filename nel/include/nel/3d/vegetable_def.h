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

#ifndef NL_VEGETABLE_DEF_H
#define NL_VEGETABLE_DEF_H

#include "nel/misc/types_nl.h"


namespace NL3D
{

// ***************************************************************************
// RdrPass for Vegetables
#define	NL3D_VEGETABLE_NRDRPASS						5
#define	NL3D_VEGETABLE_RDRPASS_LIGHTED				0
#define	NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED		1
#define	NL3D_VEGETABLE_RDRPASS_UNLIT				2
#define	NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED			3
#define	NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT	4


// 64 LUT entries in constant of VertexProgram.
#define	NL3D_VEGETABLE_VP_LUT_SIZE			64


/// For Landscape Vegetable: Distance Types are: 10m, 20m, 30m, 40m, 50m.
#define	NL3D_VEGETABLE_BLOCK_NUMDIST	5
#define	NL3D_VEGETABLE_BLOCK_ELTDIST	10.0f
/// Blend apperance transition
#define	NL3D_VEGETABLE_BLOCK_BLEND_TRANSITION_DIST	10.0f


/// The number of Quadrant for vegetable sorting.
#define	NL3D_VEGETABLE_NUM_QUADRANT		8


/// The precision of frequency factor: 1/16.
#define	NL3D_VEGETABLE_FREQUENCY_FACTOR_PREC		16


} // NL3D


#endif // NL_VEGETABLE_DEF_H

/* End of vegetable_def.h */
