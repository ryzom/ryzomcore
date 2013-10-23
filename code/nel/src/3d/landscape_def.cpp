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


#include "nel/3d/landscape_def.h"
#include "nel/misc/common.h"


using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
sint		CLandscapeGlobals::CurrentDate=0;
sint		CLandscapeGlobals::CurrentRenderDate=0;
CVector		CLandscapeGlobals::RefineCenter= CVector::Null;
float		CLandscapeGlobals::RefineThreshold= 0.001f;
float		CLandscapeGlobals::OORefineThreshold= 1.0f / CLandscapeGlobals::RefineThreshold;

CVector		CLandscapeGlobals::PZBModelPosition= CVector::Null;

float		CLandscapeGlobals::TileDistNear= 50;
float		CLandscapeGlobals::TileDistFar= CLandscapeGlobals::TileDistNear+20;
float		CLandscapeGlobals::TileDistNearSqr= sqr(CLandscapeGlobals::TileDistNear);
float		CLandscapeGlobals::TileDistFarSqr= sqr(CLandscapeGlobals::TileDistFar);
float		CLandscapeGlobals::OOTileDistDeltaSqr= 1.0f / (CLandscapeGlobals::TileDistFarSqr - CLandscapeGlobals::TileDistNearSqr);
sint		CLandscapeGlobals::TileMaxSubdivision=0;
CBSphere	CLandscapeGlobals::TileFarSphere;
CBSphere	CLandscapeGlobals::TileNearSphere;
float		CLandscapeGlobals::TilePixelSize= 128;
float		CLandscapeGlobals::TilePixelBias128= 0.5f/CLandscapeGlobals::TilePixelSize;
float		CLandscapeGlobals::TilePixelScale128= 1-1/CLandscapeGlobals::TilePixelSize;
float		CLandscapeGlobals::TilePixelBias256= 0.5f/(CLandscapeGlobals::TilePixelSize*2);
float		CLandscapeGlobals::TilePixelScale256= 1-1/(CLandscapeGlobals::TilePixelSize*2);


float		CLandscapeGlobals::Far0Dist= 200;		// 200m.
float		CLandscapeGlobals::Far1Dist= 400;		// 400m.
float		CLandscapeGlobals::FarTransition= 10;	// Alpha transition= 10m.


bool					CLandscapeGlobals::VertexProgramEnabled= false;

CFarVertexBufferInfo	CLandscapeGlobals::CurrentFar0VBInfo;
CFarVertexBufferInfo	CLandscapeGlobals::CurrentFar1VBInfo;
CNearVertexBufferInfo	CLandscapeGlobals::CurrentTileVBInfo;

CLandscapeVBAllocator	*CLandscapeGlobals::CurrentFar0VBAllocator= NULL;
CLandscapeVBAllocator	*CLandscapeGlobals::CurrentFar1VBAllocator= NULL;
CLandscapeVBAllocator	*CLandscapeGlobals::CurrentTileVBAllocator= NULL;


IDriver					*CLandscapeGlobals::PatchCurrentDriver= NULL;
CIndexBuffer			CLandscapeGlobals::PassTriArray("CLandscapeGlobals::PassTriArray");
CIndexBufferReadWrite	CLandscapeGlobals::PassTriArrayIBA;
uint					NL3D_LandscapeGlobals_PassNTri= 0;
void					*NL3D_LandscapeGlobals_PassTriCurPtr= NULL;
CIndexBuffer::TFormat	NL3D_LandscapeGlobals_PassTriFormat= CIndexBuffer::IndicesUnknownFormat;


} // NL3D
