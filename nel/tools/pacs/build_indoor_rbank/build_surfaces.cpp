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

#include <map>
#include <vector>

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/exterior_mesh.h"
#include "nel/pacs/surface_quad.h"
#include "nel/pacs/chain.h"

#include "build_surfaces.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;


// how to generate connex surfaces
void	floodFillSurfaces(CCollisionMeshBuild &cmb, vector<CInteriorSurface> &surfaces)
{
	sint32	currentId = 0;

	uint	i;

	for (i=0; i<cmb.Faces.size(); ++i)
		cmb.Faces[i].InternalSurface = -1;

	for (i=0; i<cmb.Faces.size(); ++i)
	{
		CCollisionFace	&face = cmb.Faces[i];
		if (face.Surface == CCollisionFace::ExteriorSurface || face.InternalSurface != -1)
			continue;

		vector<uint>	stack;
		stack.push_back(i);
		face.InternalSurface = currentId;

		surfaces.resize(surfaces.size()+1);
		surfaces.back().Id = currentId;
		surfaces.back().CollisionMeshBuild = &cmb;
		surfaces.back().Material = face.Material;

		while (!stack.empty())
		{
			uint	pop = stack.back();
			stack.pop_back();

			surfaces.back().Faces.push_back(pop);
			CCollisionFace	&popFace = cmb.Faces[pop];

			int	edge, neighb;
			for (edge=0; edge<3; ++edge)
			{
				if ((neighb = popFace.Edge[edge]) != -1 && 
					cmb.Faces[neighb].InternalSurface == -1 &&
					cmb.Faces[neighb].Surface == popFace.Surface)
				{
					cmb.Faces[neighb].InternalSurface = currentId;
					stack.push_back(neighb);
				}
			}
		}

		++currentId;
	}
}


// reset the edge flags of the whole collision mesh build
void	resetEdgeFlags(CCollisionMeshBuild &cmb)
{
	uint	face, edge;

	for (face=0; face<cmb.Faces.size(); ++face)
		for (edge=0; edge<3; ++edge)
			cmb.Faces[face].EdgeFlags[edge] = false;
}



// how to generate the borders of a given surface

void	followBorder(CInteriorSurface &surface, uint first, uint edge, uint sens, vector<CVector> &vstore, bool &loop)
{
	CCollisionFace	*current = &surface.getFace(first);
	CCollisionFace	*next = (current->Edge[edge] == -1) ? NULL : &surface.CollisionMeshBuild->Faces[current->Edge[edge]];
	current->EdgeFlags[edge] = true;
	sint32			currentFace = surface.Faces[first];

	const sint32	currentSurfId = current->InternalSurface;
	const sint32	oppositeSurfId = (next != NULL) ? next->InternalSurface : (current->Visibility[edge] ? -1 : -2);
	sint			oedge;

	sint			pivot = (edge+sens)%3;
	sint			nextEdge = edge;

	bool			allowThis = true;

	// adds the pivot to the border and its normal
	vstore.push_back(surface.CollisionMeshBuild->Vertices[current->V[pivot]]);

	while (true)
	{
		loop = false;
		// -1 means no neighbor at all, -2 means a neighbor that is not available yet
		sint32	thisOpposite = (next != NULL) ? next->InternalSurface : (current->Visibility[nextEdge] ? -1 : -2);
		if ((thisOpposite != currentSurfId && thisOpposite != oppositeSurfId) ||
			(loop = (current->EdgeFlags[nextEdge] && !allowThis)))
		{
			// if reaches the end of the border, then quits.
			break;
		}
		else if (thisOpposite == oppositeSurfId)
		{
			// if the next edge belongs to the border, then go on the same element
			current->EdgeFlags[nextEdge] = true;
			if (oppositeSurfId >= 0)
			{
				for (oedge=0; oedge<3 && next->Edge[oedge]!=currentFace; ++oedge)
					;
				nlassert(oedge != 3);
				nlassert(allowThis || !next->EdgeFlags[oedge]);
				next->EdgeFlags[oedge] = true;
			}
			pivot = (pivot+sens)%3;
			nextEdge = (nextEdge+sens)%3;
			next = (current->Edge[nextEdge] == -1) ? NULL : &surface.CollisionMeshBuild->Faces[current->Edge[nextEdge]];
			vstore.push_back(surface.CollisionMeshBuild->Vertices[current->V[pivot]]);
		}
		else 
		{
			// if the next element is inside the surface, then go to the next element
			nlassert(next->InternalSurface == currentSurfId);

			for (oedge=0; oedge<3 && next->Edge[oedge]!=currentFace; ++oedge)
				;
			nlassert(oedge != 3);
			currentFace = current->Edge[nextEdge];
			current = next;
			pivot = (oedge+3-sens)%3;
			nextEdge = (oedge+sens)%3;
			next = (current->Edge[nextEdge] == -1) ? NULL : &surface.CollisionMeshBuild->Faces[current->Edge[nextEdge]];
		}

		allowThis = false;
	}
}


void	computeSurfaceBorders(CInteriorSurface &surface, vector<CInteriorBorder> &borders)
{
	uint	face, edge;

	for (face=0; face<surface.Faces.size(); ++face)
	{
		// for each element,
		// scan for a edge that points to a different surface
		CCollisionFace	&cf = surface.getFace(face);

		for (edge=0; edge<3; ++edge)
		{
			if ((cf.Edge[edge] == -1 || surface.getNeighbor(face, edge).InternalSurface != surface.Id) &&
				!cf.EdgeFlags[edge])
			{
				borders.resize(borders.size()+1);
				CInteriorBorder	&border = borders.back();

				border.Left = cf.InternalSurface;

				if (cf.Edge[edge] == -1)
				{
					// link on a neighbor retriever or not at all
					border.Right = cf.Visibility[edge] ? -1 : -2;
				}
				else
				{
					// link on the neighbor surface
					border.Right = surface.CollisionMeshBuild->Faces[cf.Edge[edge]].InternalSurface;
				}

				nldebug("generate border %d (%d-%d)", borders.size()-1, border.Left, border.Right);

				bool				loop;
				vector<CVector>		bwdVerts;
				vector<CVector>		&fwdVerts = border.Vertices;

				followBorder(surface, face, edge, 2, bwdVerts, loop);

				sint	i;

				fwdVerts.reserve(bwdVerts.size());
				fwdVerts.clear();

				for (i=(sint)(bwdVerts.size()-1); i>=0; --i)
				{
					fwdVerts.push_back(bwdVerts[i]);
				}

				if (loop)
				{
					fwdVerts.push_back(fwdVerts.front());
				}
				else
				{
					fwdVerts.resize(fwdVerts.size()-2);
					followBorder(surface, face, edge, 1, fwdVerts, loop);
				}
			}
		}
	}
}


void	computeSurfaceCenter(CInteriorSurface &surface)
{
	CCollisionMeshBuild	&cmb = *(surface.CollisionMeshBuild);

	CVector		center = CVector::Null;
	float		totalWeight = 0.0f;

	uint	i, j;

	for (i=0; i<surface.Faces.size(); ++i)
	{
		CCollisionFace	&face = surface.getFace(i);
		CVector			v[3];
		
		for (j=0; j<3; ++j)
			v[j] = cmb.Vertices[face.V[j]];

		float	weight = ((v[2]-v[0])^(v[1]-v[0])).norm();

		center += (v[0]+v[1]+v[2])*(weight/3.0f);
		totalWeight += weight;
	}

	surface.Center = center/totalWeight;
}


void	computeSurfaceQuadTree(CInteriorSurface &surface, CSurfaceQuadTree &quad)
{
	uint	i, j;

	CAABBox	box;
	bool	first = true;
	for (i=0; i<surface.Faces.size(); ++i)
	{
		for (j=0; j<3; ++j)
		{
			const CVector	&v = surface.CollisionMeshBuild->Vertices[surface.CollisionMeshBuild->Faces[surface.Faces[i]].V[j]];
			if (first)
				box.setCenter(v), first=false;
			else
				box.extend(v);
		}
	}

	quad.clear();
	quad.init(4.0f, 6, box.getCenter(), std::max(box.getHalfSize().x, box.getHalfSize().y));

	for (i=0; i<surface.Faces.size(); ++i)
	{
		for (j=0; j<3; ++j)
		{
			const CVector	&v = surface.CollisionMeshBuild->Vertices[surface.CollisionMeshBuild->Faces[surface.Faces[i]].V[j]];
			quad.addVertex(v);
		}
	}

	quad.compile();
}


void	buildSurfaces(CCollisionMeshBuild &cmb, CLocalRetriever &lr)
{
	vector<CInteriorSurface>	surfaces;
	vector<CInteriorBorder>		borders;

	floodFillSurfaces(cmb, surfaces);

	resetEdgeFlags(cmb);

	uint	surf, bord;

	/// \todo compute real surface center and quadtree
	for (surf=0; surf<surfaces.size(); ++surf)
	{
		CSurfaceQuadTree	quad;
		computeSurfaceBorders(surfaces[surf], borders);
		computeSurfaceCenter(surfaces[surf]);
		computeSurfaceQuadTree(surfaces[surf], quad);
		lr.addSurface(0, 0, (uint8)surfaces[surf].Material, 0, 0, false, 0.0f, false, surfaces[surf].Center, quad);
	}

	sint	numBorderChains = 0;

	for (bord=0; bord<borders.size(); ++bord)
	{
		sint32	left = borders[bord].Left;
		sint32	right = (borders[bord].Right == -2) ? CChain::convertBorderChainId(numBorderChains++) : borders[bord].Right;

		if (left<0 || left>=(sint)surfaces.size() ||
			right>(sint)surfaces.size())
			nlstop;

		lr.addChain(borders[bord].Vertices, left, right);
	}
}


//

void	buildSnapping(CCollisionMeshBuild &cmb, CLocalRetriever &lr)
{
	// copy the vertices
	lr.getInteriorVertices() = cmb.Vertices;

	// create the faces
	uint	i;
	vector<CLocalRetriever::CInteriorFace>	&faces = lr.getInteriorFaces();
	for (i=0; i<cmb.Faces.size(); ++i)
	{
		faces.push_back(CLocalRetriever::CInteriorFace());
		faces.back().Verts[0] = cmb.Faces[i].V[0];
		faces.back().Verts[1] = cmb.Faces[i].V[1];
		faces.back().Verts[2] = cmb.Faces[i].V[2];
		faces.back().Surface = cmb.Faces[i].InternalSurface;
	}

	// create the face grid
	lr.initFaceGrid();
}

