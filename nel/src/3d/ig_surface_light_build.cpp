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

#include "nel/3d/ig_surface_light_build.h"
#include "nel/3d/scene_group.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
void			CIGSurfaceLightBuild::buildSunDebugMesh(CMesh::CMeshBuild &meshBuild,
	CMeshBase::CMeshBaseBuild &meshBaseBuild, const CVector &deltaPos)
{
	contReset(meshBuild);
	contReset(meshBaseBuild);
	meshBaseBuild.Materials.resize(1);
	meshBaseBuild.Materials[0].initUnlit();
	meshBaseBuild.Materials[0].setBlend(true);
	meshBaseBuild.Materials[0].setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

	meshBuild.VertexFlags= CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag;

	// For all grids.
	ItRetrieverGridMap	it;
	for(it= RetrieverGridMap.begin(); it!= RetrieverGridMap.end(); it++)
	{
		for(uint iSurf= 0; iSurf<it->second.Grids.size(); iSurf++)
		{
			CSurface	&surface= it->second.Grids[iSurf];

			// Resize vector.
			uint	wVert= surface.Width;
			uint	hVert= surface.Height;
			uint	vId0= (uint)meshBuild.Vertices.size();
			// Allocate vertices / colors
			meshBuild.Vertices.resize(vId0 + wVert*hVert);
			vector<CRGBA>	colors;
			colors.resize(wVert*hVert);

			// Build vertices pos and colors.
			uint	x, y;
			for(y=0;y<hVert; y++)
			{
				for(x=0;x<wVert; x++)
				{
					uint	vId= y*wVert + x;
					// Copy Pos.
					meshBuild.Vertices[vId0 + vId]= surface.Cells[vId].CenterPos + deltaPos;
					// Copy Color.
					uint8	col= surface.Cells[vId].SunContribution;
					colors[vId].set(col, col, col, 128);
					// Force Blue color, to simulate ambiant.
					colors[vId].B= 128 + colors[vId].B/2;
					// OutSurface => green is 128.
					if(!surface.Cells[vId].InSurface)
						colors[vId].G= 128;

				}
			}

			// Build faces
			addDebugMeshFaces(meshBuild, surface, vId0, colors);

		}
	}

}


// ***************************************************************************
void			CIGSurfaceLightBuild::buildPLDebugMesh(CMesh::CMeshBuild &meshBuild, CMeshBase::CMeshBaseBuild &meshBaseBuild, const CVector &deltaPos, const CInstanceGroup &igOut)
{
	contReset(meshBuild);
	contReset(meshBaseBuild);
	meshBaseBuild.Materials.resize(1);
	meshBaseBuild.Materials[0].initUnlit();

	meshBuild.VertexFlags= CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag;

	// Get the number of lights in Ig.
	uint	numLight= (uint)igOut.getPointLightList().size();
	numLight= raiseToNextPowerOf2(numLight);
	uint	idMultiplier= 256/ numLight;

	// For all grids.
	ItRetrieverGridMap	it;
	for(it= RetrieverGridMap.begin(); it!= RetrieverGridMap.end(); it++)
	{
		// get the final surface
		CIGSurfaceLight::TRetrieverGridMap::const_iterator	itIg=
			igOut.getIGSurfaceLight().getRetrieverGridMap().find(it->first);

		// If not found, abort
		if( itIg== igOut.getIGSurfaceLight().getRetrieverGridMap().end() )
		{
			nlwarning("buildPLDebugMesh fails to find retriever '%d' in igOut", it->first);
			continue;
		}
		else if( it->second.Grids.size()!=itIg->second.Grids.size() )
		{
			nlwarning("buildPLDebugMesh find retriever '%d' in igOut, but with bad size: excepting: %d, get: %d",
				it->first, it->second.Grids.size(), itIg->second.Grids.size() );
			continue;
		}

		// For all surface of the retriever.
		for(uint iSurf= 0; iSurf<it->second.Grids.size(); iSurf++)
		{
			CSurface					&surface= it->second.Grids[iSurf];
			const CSurfaceLightGrid		&igSurface= itIg->second.Grids[iSurf];

			// Resize vector.
			uint	wVert= surface.Width;
			uint	hVert= surface.Height;
			uint	vId0= (uint)meshBuild.Vertices.size();
			// Allocate vertices / colors
			meshBuild.Vertices.resize(vId0 + wVert*hVert);
			vector<CRGBA>	colors;
			colors.resize(wVert*hVert);

			// Build vertices pos and colors.
			uint	x, y;
			for(y=0;y<hVert; y++)
			{
				for(x=0;x<wVert; x++)
				{
					uint	vId= y*wVert + x;
					// Copy Pos.
					meshBuild.Vertices[vId0 + vId]= surface.Cells[vId].CenterPos + deltaPos;
					// init Color with idMultiplier in Blue (info!).
					CRGBA	&col= colors[vId];
					col.set(0,0, idMultiplier, 255);
					// store the compressed id of the light found in igOut.
					nlassert( CSurfaceLightGrid::NumLightPerCorner>=2 );
					uint	idLight0= igSurface.Cells[vId].Light[0];
					uint	idLight1= igSurface.Cells[vId].Light[1];
					// 255 means no light. If at least one light
					if(idLight0<255)
					{
						uint	v= (idLight0+1)*idMultiplier;
						col.R= min(v, 255U);
						// if second light
						if(idLight1<255)
						{
							v= (idLight1+1)*idMultiplier;
							col.G= min(v, 255U);
						}
					}
				}
			}

			// Build faces
			addDebugMeshFaces(meshBuild, surface, vId0, colors);

		}
	}

}


// ***************************************************************************
void			CIGSurfaceLightBuild::addDebugMeshFaces(CMesh::CMeshBuild &meshBuild, CSurface &surface, uint vId0,
	const std::vector<CRGBA>	&colors)
{
	// Resize faces.
	uint	wVert= surface.Width;
	uint	hVert= surface.Height;
	uint	wCell= wVert-1;
	uint	hCell= hVert-1;
	// Allocate enough space for faces.
	meshBuild.Faces.reserve(meshBuild.Faces.size() + wCell*hCell *2);

	// Build faces
	uint	x,y;
	for(y=0;y<hCell; y++)
	{
		for(x=0;x<wCell; x++)
		{
			uint	v00= y*wVert + x;
			uint	v10= y*wVert + x+1;
			uint	v01= (y+1)*wVert + x;
			uint	v11= (y+1)*wVert + x+1;

			// Skip this cell??
			bool	skip= false;
			if(!surface.Cells[v00].InSurface && !surface.Cells[v00].Dilated)	skip= true;
			if(!surface.Cells[v10].InSurface && !surface.Cells[v10].Dilated)	skip= true;
			if(!surface.Cells[v01].InSurface && !surface.Cells[v01].Dilated)	skip= true;
			if(!surface.Cells[v11].InSurface && !surface.Cells[v11].Dilated)	skip= true;


			if(!skip)
			{
				// 1st triangle.
				CMesh::CFace	face0;
				face0.MaterialId= 0;
				face0.Corner[0].Vertex= vId0+ v00;
				face0.Corner[0].Color= colors[v00];
				face0.Corner[1].Vertex= vId0+ v10;
				face0.Corner[1].Color= colors[v10];
				face0.Corner[2].Vertex= vId0+ v01;
				face0.Corner[2].Color= colors[v01];

				// 2nd triangle.
				CMesh::CFace	face1;
				face1.MaterialId= 0;
				face1.Corner[0].Vertex= vId0+ v10;
				face1.Corner[0].Color= colors[v10];
				face1.Corner[1].Vertex= vId0+ v11;
				face1.Corner[1].Color= colors[v11];
				face1.Corner[2].Vertex= vId0+ v01;
				face1.Corner[2].Color= colors[v01];

				// Add 2 triangles
				meshBuild.Faces.push_back(face0);
				meshBuild.Faces.push_back(face1);
			}
		}
	}
}



} // NL3D
