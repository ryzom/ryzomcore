// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include <sstream>

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/exterior_mesh.h"

#include "mouline.h"
#include "build_surfaces.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;

/*
// a reference on an edge
struct CEdgeKey
{
	uint32	V0;
	uint32	V1;

	CEdgeKey() {}
	CEdgeKey(uint32 v0, uint32 v1) : V0(v0), V1(v1) {}

	bool	operator() (const CEdgeKey &a, const CEdgeKey &b)
	{
		return a.V0 < b.V0 || (a.V0 == b.V0 && a.V1 < b.V1);
	}
};

// the info on an edge
struct CEdgeInfo
{
	sint32	Left, LeftEdge;
	sint32	Right, RightEdge;

	CEdgeInfo(sint32 left=-1, sint32 leftEdge=-1, sint32 right=-1, sint32 rightEdge=-1) : Left(left), LeftEdge(leftEdge), Right(right), RightEdge(rightEdge) {}
};

typedef	map<CEdgeKey, CEdgeInfo, CEdgeKey>	TLinkRelloc;
typedef TLinkRelloc::iterator				ItTLinkRelloc;


//
void	linkMesh(CCollisionMeshBuild &cmb, bool linkInterior)
{
	uint			i, j;
	TLinkRelloc		relloc;

	// check each edge of each face
	for (i=0; i<cmb.Faces.size(); ++i)
	{
		if (cmb.Faces[i].Surface == CCollisionFace::ExteriorSurface && linkInterior ||
			cmb.Faces[i].Surface >= CCollisionFace::InteriorSurfaceFirst && !linkInterior)
			continue;

		for (j=0; j<3; ++j)
		{
			cmb.Faces[i].Edge[j] = -1;

			uint	edge = (j+2)%3;
			uint32	va = cmb.Faces[i].V[j],
					vb = cmb.Faces[i].V[(j+1)%3];

			ItTLinkRelloc	it;
			if ((it = relloc.find(CEdgeKey(va, vb))) != relloc.end())
			{
				// in this case, the left triangle of the edge has already been found.
				// should throw an error
				nlerror("On face %d, edge %d: left side of edge (%d,%d) already linked to face %d",
						i, edge, va, vb, (*it).second.Left);
			}
			else if ((it = relloc.find(CEdgeKey(vb, va))) != relloc.end())
			{
				// in this case, we must check the right face has been set yet
				if ((*it).second.Right != -1)
				{
					nlerror("On face %d, edge %d: right side of edge (%d,%d) already linked to face %d",
							i, edge, vb, va, (*it).second.Right);
				}

				(*it).second.Right = i;
				(*it).second.RightEdge = edge;
			}
			else
			{
				// if the edge wasn't present yet, create it and set it up.
				relloc.insert(make_pair(CEdgeKey(va, vb), CEdgeInfo(i, edge, -1, -1)));
			}
		}
	}

	// for each checked edge, update the edge info inside the faces
	ItTLinkRelloc	it;
	for (it=relloc.begin(); it!=relloc.end(); ++it)
	{
		sint32	left, leftEdge;
		sint32	right, rightEdge;

		// get the link info on the edge
		left = (*it).second.Left;
		leftEdge = (*it).second.LeftEdge;
		right = (*it).second.Right;
		rightEdge = (*it).second.RightEdge;

		// update both faces
		if (left != -1)
			cmb.Faces[left].Edge[leftEdge] = right;
		if (right != -1)
			cmb.Faces[right].Edge[rightEdge] = left;
	}
}
*/



void	buildExteriorMesh(CCollisionMeshBuild &cmb, CExteriorMesh &em)
{
	uint							startFace = 0;
	vector<CExteriorMesh::CEdge>	edges;

	uint	i;
	for (i=0; i<cmb.Faces.size(); ++i)
	{
		cmb.Faces[i].EdgeFlags[0] = false;
		cmb.Faces[i].EdgeFlags[1] = false;
		cmb.Faces[i].EdgeFlags[2] = false;
	}
	
	while (true)
	{
		// find the first non interior face
		uint	i, edge;
		bool	found = false;
		for (i=startFace; i<cmb.Faces.size() && !found; ++i)
		{
			if (cmb.Faces[i].Surface != CCollisionFace::ExteriorSurface)
				continue;

			for (edge=0; edge<3 && !found; ++edge)
				if (cmb.Faces[i].Edge[edge] == -1 && !cmb.Faces[i].EdgeFlags[edge])
				{
//					nlassert(cmb.Faces[i].Material != 0xdeadbeef);
					found = true;
					break;
				}

			if (found)
				break;
		}

		//
		if (!found)
			break;

//		cmb.Faces[i].Material = 0xdeadbeef;

		startFace = i+1;

		sint32		current = i;
		sint32		next = cmb.Faces[current].Edge[edge];

		sint		oedge;
		sint		pivot = (edge+1)%3;
		sint		nextEdge = edge;
		bool		allowThis = true;

		uint		numLink = 0;
		uint		firstEdge = (uint)edges.size();

		vector<CExteriorMesh::CEdge>	loop;

		while (true)
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
				/// \todo get the real edge link
				sint	link = (cmb.Faces[current].Visibility[nextEdge]) ? -1 : 0;	//(numLink++);

				//edges.push_back(CExteriorMesh::CEdge(cmb.Vertices[cmb.Faces[current].V[pivot]], link));
				loop.push_back(CExteriorMesh::CEdge(cmb.Vertices[cmb.Faces[current].V[pivot]], link));

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

		// mark the end of a ext mesh block
		// this way, collisions won't be checked in the pacs engine
		if (loop.size() >= 3)
		{
			uint	n = (uint)loop.size();
			while (loop.front().Link >= 0 && loop.back().Link >= 0 && n > 0)
			{
				loop.push_back(loop.front());
				loop.erase(loop.begin());
				--n;
			}
		}
		loop.push_back(loop.front());
		loop.back().Link = -2;
		edges.insert(edges.end(), loop.begin(), loop.end());
		//edges.push_back(edges[firstEdge]);
		//edges.back().Link = -2;
	}

	bool	previousWasLink = false;
	sint	previousLink = -1;
	for (i=0; i<edges.size(); ++i)
	{
//		nldebug("ext-mesh: vertex=%d (%.2f,%.2f,%.2f) link=%d", i, edges[i].Start.x, edges[i].Start.y, edges[i].Start.z, edges[i].Link);
		if (edges[i].Link >= 0)
		{
			if (!previousWasLink)
				++previousLink;
			edges[i].Link = previousLink;
			previousWasLink = true;
		}
		else
		{
			previousWasLink = false;
		}
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

		nlinfo("Border chains (to be linked) for this retriever:");

		for (i=0; i<bchains.size(); ++i)
		{
			char w[256];
			std::stringstream ss;
			const CChain	&chain = chains[bchains[i]];
			sprintf(w, "Border chain %d: chain=%d ", i, bchains[i]);
			ss << w;
			uint	och;
			for (och=0; och<chain.getSubChains().size(); ++och)
			{
				const COrderedChain3f	&ochain = ochains[chain.getSubChain(och)];
				sprintf(w, "subchain=%d", chain.getSubChain(och));
				ss << w;
				uint	v;
				for (v=0; v<ochain.getVertices().size(); ++v)
				{
					sprintf(w, " (%.2f,%.2f)", ochain[v].x, ochain[v].y);
					ss << w;
				}
			}

			nlinfo("%s", ss.str().c_str());
		}
	}

	uint	edge, ch;
	for (edge=0; edge+1<edges.size(); )
	{
		if (edges[edge].Link == -1 || edges[edge].Link == -2)
		{
			++edge;
			continue;
		}

		uint	startedge = edge;
		uint	stopedge;

		for (stopedge=edge; stopedge+1<edges.size() && edges[stopedge+1].Link == edges[startedge].Link; ++stopedge)
			;

		edge = stopedge+1;

		CVector	start = edges[startedge].Start, stop = edges[stopedge+1].Start;
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
			nlwarning("couldn't find any link to the exterior edge %d-%d!!", startedge, stopedge);
		}
		else
		{
			// set it up to point on the chain and surface
			link.BorderChainId = ch;
			link.ChainId = bchains[ch];
			link.SurfaceId = (uint16)chains[link.ChainId].getLeft();
		}

		// enlarge the links
		if (edges[startedge].Link >= (sint)links.size())
			links.resize(edges[startedge].Link+1);

		// if the link already exists, warning
		if (links[edges[startedge].Link].BorderChainId != 0xFFFF ||
			links[edges[startedge].Link].ChainId != 0xFFFF ||
			links[edges[startedge].Link].SurfaceId != 0xFFFF)
		{
			nlwarning("in linkInteriorToExterior():");
			nlwarning("link %d already set!!", edges[startedge].Link);
		}

		// setup the link
		links[edges[startedge].Link] = link;
	}

//	em.setEdges(edges);
	em.setLinks(links);
	lr.setExteriorMesh(em);
}





//
void	computeRetriever(CCollisionMeshBuild &cmb, CLocalRetriever &lr, CVector &translation, bool useCmbTrivialTranslation)
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

	uint	i, j;

	for (i=0; i<cmb.Faces.size(); ++i)
	{
		CVector		normal = ((cmb.Vertices[cmb.Faces[i].V[1]]-cmb.Vertices[cmb.Faces[i].V[0]])^(cmb.Vertices[cmb.Faces[i].V[2]]-cmb.Vertices[cmb.Faces[i].V[0]])).normed();

		if (normal.z < 0.0f)
		{
			nlwarning("Face %d in cmb (%s) has negative normal! -- face is flipped", i, cmb.Faces[i].Surface == CCollisionFace::InteriorSurfaceFirst ? "interior" : "exterior");
/*
			std::swap(cmb.Faces[i].V[1], cmb.Faces[i].V[2]);
			std::swap(cmb.Faces[i].Visibility[1], cmb.Faces[i].Visibility[2]);
*/
		}
	}

	// first link faces
/*
	linkMesh(cmb, false);
	linkMesh(cmb, true);
*/
	vector<string>	errors;
	
	cmb.link(false, errors);
	cmb.link(true, errors);

	if (!errors.empty())
	{
		nlwarning("Edge issues reported !!");
		uint	i;
		for (i=0; i<errors.size(); ++i)
			nlwarning("%s", errors[i].c_str());
		nlerror("Can't continue.");
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
}
