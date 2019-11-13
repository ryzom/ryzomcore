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

#include "nel/3d/landscape_profile.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************
// Yoyo: for profile only.
sint		ProfNTessFace= 0;
sint		ProfNRdrFar0= 0;
sint		ProfNRdrFar1= 0;
sint		ProfNRdrTile[NL3D_MAX_TILE_PASS];
sint		ProfNRefineFaces;
sint		ProfNRefineComputeFaces;
sint		ProfNRefineLeaves;
sint		ProfNSplits;
sint		ProfNMerges;
// New PriorityList vars.
sint		ProfNRefineInTileTransition;
sint		ProfNRefineWithLowDistance;
sint		ProfNSplitsPass;
// Material setup Profile
sint		ProfNTileSetupMaterial= 0;
sint		ProfNFar0SetupMaterial= 0;
sint		ProfNFar1SetupMaterial= 0;
// Patch render
sint		ProfNPatchRdrFar0=0;
sint		ProfNPatchRdrFar1=0;

} // NL3D
