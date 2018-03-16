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

#include "stdpacs.h"
#include "nel/pacs/build_indoor.h"

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/exterior_mesh.h"

using namespace std;
using namespace NLMISC;

namespace NLPACS
{

/**
 * The interior surface class. Intermediate to compute real retriever surfaces
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CInteriorSurface
{
public:
	/// The collision mesh root object
	CCollisionMeshBuild				*CollisionMeshBuild;

	/// The faces that compose the surface
	std::vector<uint32>				Faces;

	/// The Id of the surface
	sint32							Id;

	/// The center of the surface
	NLMISC::CVector					Center;

	/// The material of the surface
	sint32							Material;

public:
	CCollisionFace					&getFace(uint face) { return CollisionMeshBuild->Faces[Faces[face]]; }
	CCollisionFace					&getNeighbor(uint face, uint edge)
	{
		return CollisionMeshBuild->Faces[getFace(face).Edge[edge]];
	}
};


/**
 * The border of interior surfaces.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CInteriorBorder
{
public:
	/// The vertices that compose the border
	std::vector<NLMISC::CVector>	Vertices;

	/// The left and right surfaces
	sint32							Left, Right;

public:
};


// how to build interior snapping data
void	buildSnapping(CCollisionMeshBuild &cmb, CLocalRetriever &lr);


// how to build surfaces
void	buildSurfaces(CCollisionMeshBuild &cmb, CLocalRetriever &lr);



//
// functions to build interior surfaces and borders from mesh
//

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

		vector<sint32>	stack;
		stack.push_back(sint32(i));
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

			sint32	edge, neighb;
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

	for(;;)
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

	for (surf=0; surf<surfaces.size(); ++surf)
	{
		CSurfaceQuadTree	quad;
		computeSurfaceBorders(surfaces[surf], borders);
		computeSurfaceCenter(surfaces[surf]);
		computeSurfaceQuadTree(surfaces[surf], quad);
		lr.addSurface(0, 0, (uint8)surfaces[surf].Material, 0, 0, false, 0.0f, false, surfaces[surf].Center, quad);
		//lr.addSurface(0, 0, (uint8)surfaces[surf].Material, 0, 0, false, 0.0f, /*false,*/ surfaces[surf].Center, quad);
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


//
// functions to build local retrievers
//

void	buildExteriorMesh(CCollisionMeshBuild &cmb, CExteriorMesh &em)
{
	// find the first non interior face
	uint							i,
									edge = 0;

	vector<CExteriorMesh::CEdge>	edges;
	uint							numLink = 0;

	for (i=0; i<cmb.Faces.size(); ++i)
	{
		cmb.Faces[i].EdgeFlags[0] = false;
		cmb.Faces[i].EdgeFlags[1] = false;
		cmb.Faces[i].EdgeFlags[2] = false;
	}

	i = 0;

	for(;;)
	{
		bool	found = false;
		for (; i<cmb.Faces.size() && !found; ++i)
		{
			if (cmb.Faces[i].Surface != CCollisionFace::ExteriorSurface)
				continue;

			for (edge=0; edge<3 && !found; ++edge)
				if (cmb.Faces[i].Edge[edge] == -1 && !cmb.Faces[i].EdgeFlags[edge])
				{
					found = true;
					break;
				}

			if (found)
				break;
		}

		//
		if (!found)
			break;

		sint32			current = i;
		sint32			next = cmb.Faces[current].Edge[edge];

		sint			oedge;
		sint			pivot = (edge+1)%3;
		sint			nextEdge = edge;

		uint			firstExtEdge = (uint)edges.size();

		for(;;)
		{
			if (cmb.Faces[current].EdgeFlags[nextEdge])
			{
				// if reaches the end of the border, then quits.
				break;
			}
			else if (next == -1)
			{
				// if the next edge belongs to the border, then go on the same element
				cmb.Faces[current].EdgeFlags[nextEdge] = true;
				sint	link = (cmb.Faces[current].Visibility[nextEdge]) ? -1 : sint((numLink++));
				edges.push_back(CExteriorMesh::CEdge(cmb.Vertices[cmb.Faces[current].V[pivot]], link));
				nldebug("border: vertex=%d (%.2f,%.2f,%.2f) link=%d", cmb.Faces[current].V[pivot], cmb.Vertices[cmb.Faces[current].V[pivot]].x, cmb.Vertices[cmb.Faces[current].V[pivot]].y, cmb.Vertices[cmb.Faces[current].V[pivot]].z, link);
				pivot = (pivot+1)%3;
				nextEdge = (nextEdge+1)%3;
				next = cmb.Faces[current].Edge[nextEdge];
			}
			else
			{
				// if the next element is inside the surface, then go to the next element
				for (oedge=0; oedge<3 && cmb.Faces[next].Edge[oedge]!=current; ++oedge)
					;
				nlassert(oedge != 3);
				current = next;
				pivot = (oedge+2)%3;
				nextEdge = (oedge+1)%3;
				next = cmb.Faces[current].Edge[nextEdge];
			}
		}

		edges.push_back(edges[firstExtEdge]);
		edges.back().Link = -2;
	}

	em.setEdges(edges);
}


//
void	linkExteriorToInterior(CLocalRetriever &lr)
{
	CExteriorMesh					em = lr.getExteriorMesh();
	vector<CExteriorMesh::CEdge>	edges = em.getEdges();
	vector<CExteriorMesh::CLink>	links;
	const vector<CChain>			&chains = lr.getChains();
	const vector<COrderedChain3f>	&ochains = lr.getFullOrderedChains();
	const vector<uint16>			&bchains = lr.getBorderChains();

	{
		uint	i;

		for (i=0; i<bchains.size(); ++i)
		{
			static char	buf[512], w[256];
			const CChain	&chain = chains[bchains[i]];
			sprintf(buf, "chain=%d ", bchains[i]);
			uint	och;
			for (och=0; och<chain.getSubChains().size(); ++och)
			{
				const COrderedChain3f	&ochain = ochains[chain.getSubChain(och)];
				sprintf(w, "subchain=%d", chain.getSubChain(och));
				strcat(buf, w);
				uint	v;
				for (v=0; v<ochain.getVertices().size(); ++v)
				{
					sprintf(w, " (%.2f,%.2f)", ochain[v].x, ochain[v].y);
					strcat(buf, w);
				}
			}

			nlinfo("%s", buf);
		}
	}

	uint	edge, ch;
	for (edge=0; edge+1<edges.size(); ++edge)
	{
		if (edges[edge].Link == -1)
			continue;

		CVector	start = edges[edge].Start, stop = edges[edge+1].Start;
		bool	found = false;

		for (ch=0; ch<bchains.size() && !found; ++ch)
		{
			// get the border chain.
			//const CChain	&chain = chains[bchains[ch]];

			const CVector	&cstart = lr.getStartVector(bchains[ch]),
							&cstop = lr.getStopVector(bchains[ch]);

			float d = (start-cstart).norm()+(stop-cstop).norm();
			if (d < 1.0e-1f)
			{
				found = true;
				break;
			}
		}

		// create a link
		CExteriorMesh::CLink	link;

		if (!found)
		{
			nlwarning("in linkInteriorToExterior():");
			nlwarning("couldn't find any link to the exterior edge %d!!", edge);
		}
		else
		{
			// set it up to point on the chain and surface
			link.BorderChainId = uint16(ch);
			link.ChainId = bchains[ch];
			link.SurfaceId = (uint16)chains[link.ChainId].getLeft();
		}

		// enlarge the links
		if (edges[edge].Link >= (sint)links.size())
			links.resize(edges[edge].Link+1);

		// if the link already exists, warning
		if (links[edges[edge].Link].BorderChainId != 0xFFFF ||
			links[edges[edge].Link].ChainId != 0xFFFF ||
			links[edges[edge].Link].SurfaceId != 0xFFFF)
		{
			nlwarning("in linkInteriorToExterior():");
			nlwarning("link %d already set!!", edges[edge].Link);
		}

		// setup the link
		links[edges[edge].Link] = link;
	}

	em.setEdges(edges);
	em.setLinks(links);
	lr.setExteriorMesh(em);
}





//
bool	computeRetriever(CCollisionMeshBuild &cmb, CLocalRetriever &lr, CVector &translation, string &error, bool useCmbTrivialTranslation)
{
	// set the retriever
	lr.setType(CLocalRetriever::Interior);

	// if should use the own cmb bbox, then compute it
	if (useCmbTrivialTranslation)
	{
		translation = cmb.computeTrivialTranslation();
		// snap the translation vector to a meter wide grid
		translation.x = (float)ceil(translation.x);
		translation.y = (float)ceil(translation.y);
		translation.z = 0.0f;
	}

	vector<string>	errors;

	cmb.link(false, errors);
	cmb.link(true, errors);

	if (!errors.empty())
	{
		nlwarning("Edge issues reported !!");
		uint	i;
		error.clear();
		for (i=0; i<errors.size(); ++i)
			error += errors[i]+"\n";
		return false;
	}

	// translate the meshbuild to the local axis
	cmb.translate(translation);

	// find the exterior mesh border
	CExteriorMesh	extMesh;
	buildExteriorMesh(cmb, extMesh);
	lr.setExteriorMesh(extMesh);

	// build the surfaces in the local retriever
	buildSurfaces(cmb, lr);

	// create the snapping faces and vertices
	// after the build surfaces because the InternalSurfaceId is filled within buildSurfaces()...
	buildSnapping(cmb, lr);

	//
	lr.computeLoopsAndTips();

	lr.findBorderChains();
	lr.updateChainIds();
	lr.computeTopologies();

	lr.unify();

	lr.computeCollisionChainQuad();
/*
	//
	for (i=0; i<lr.getSurfaces().size(); ++i)
		lr.dumpSurface(i);
*/
	//
	linkExteriorToInterior(lr);

	// compute the bbox of the retriever
	uint	i, j;
	CAABBox	bbox;
	bool	first = true;

	for (i=0; i<extMesh.getEdges().size(); ++i)
		if (!first)
			bbox.extend(extMesh.getEdge(i).Start);
		else
			bbox.setCenter(extMesh.getEdge(i).Start), first=false;

	for (i=0; i<lr.getOrderedChains().size(); ++i)
		for (j=0; j<lr.getOrderedChain(i).getVertices().size(); ++j)
			if (!first)
				bbox.extend(lr.getOrderedChain(i)[j].unpack3f());
			else
				bbox.setCenter(lr.getOrderedChain(i)[j].unpack3f()), first=false;

	CVector	bboxhs = bbox.getHalfSize();
	bboxhs.z = 10000.0f;
	bbox.setHalfSize(bboxhs);

	lr.setBBox(bbox);

	return true;
}

} // NLPACS

