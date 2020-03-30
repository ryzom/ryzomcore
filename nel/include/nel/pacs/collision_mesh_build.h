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

#ifndef NL_COLLISION_MESH_BUILD_H
#define NL_COLLISION_MESH_BUILD_H

#include <vector>
#include <map>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/file.h"

#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"


namespace NLPACS
{

struct CCollisionFace
{
	/// \name Attributes to set
	// @{

	/// The number of the vertices of the face.
	uint32	V[3];

	/// The visibility of each edge.
	bool	Visibility[3];

	/// The number of the surface of which it is associated. -1 means exterior surface.
	sint32	Surface;

	///
	sint32	Material;

	// @}


	/// \name Internal attributes
	// @{

	/// The link to the neighbor faces -- don't fill
	sint32	Edge[3];

	/// The number of the connex surface associated -- don't fill
	sint32	InternalSurface;

	/// The flags for each edge -- don't fill
	bool	EdgeFlags[3];

	// @}


	/// The exterior/interior surfaces id
	enum
	{
		ExteriorSurface = -1,
		InteriorSurfaceFirst = 0
	};

	/// Serialise the face
	void	serial(NLMISC::IStream &f)
	{
		f.serial(V[0]);
		f.serial(V[1]);
		f.serial(V[2]);

		f.serial(Visibility[0]);
		f.serial(Visibility[1]);
		f.serial(Visibility[2]);

		f.serial(Surface, Material);
	}
};

/**
 * The collision intermediate mesh, used to build the real collision meshes.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CCollisionMeshBuild
{
private:
	// a reference on an edge
	struct CEdgeKey
	{
		uint32	V0;
		uint32	V1;

		CEdgeKey() {}
		CEdgeKey(uint32 v0, uint32 v1) : V0(v0), V1(v1) {}

		bool	operator() (const CEdgeKey &a, const CEdgeKey &b) const
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

	typedef	std::map<CEdgeKey, CEdgeInfo, CEdgeKey>	TLinkRelloc;
	typedef TLinkRelloc::iterator					ItTLinkRelloc;

public:


public:
	/// The vertices of the mesh
	std::vector<NLMISC::CVector>	Vertices;

	/// The faces of the mesh
	std::vector<CCollisionFace>		Faces;


public:
	void	serial(NLMISC::IStream &f)
	{
		f.serialCont(Vertices);
		f.serialCont(Faces);
	}

	void	translate(const NLMISC::CVector &translation)
	{
		uint	i;
		for (i=0; i<Vertices.size(); ++i)
			Vertices[i] += translation;
	}

	void	transform(const NLMISC::CMatrix &tranfo)
	{
		uint	i;
		for (i=0; i<Vertices.size(); ++i)
			Vertices[i] = tranfo * Vertices[i];
	}

	NLMISC::CVector	computeTrivialTranslation() const
	{
		uint	i;
		NLMISC::CAABBox	bbox;

		if (!Vertices.empty())
		{
			bbox.setCenter(Vertices[0]);
			for (i=1; i<Vertices.size(); ++i)
				bbox.extend(Vertices[i]);
		}

		return -bbox.getCenter();
	}

	//
	void	link(bool linkInterior,	std::vector<std::string> &errors)
	{
		uint			i, j;
		TLinkRelloc		relloc;


		// check each edge of each face
		for (i=0; i<Faces.size(); ++i)
		{
			if ((Faces[i].Surface == CCollisionFace::ExteriorSurface && linkInterior) ||
				(Faces[i].Surface >= CCollisionFace::InteriorSurfaceFirst && !linkInterior))
				continue;

			for (j=0; j<3; ++j)
			{
				Faces[i].Edge[j] = -1;

				uint	edge = (j+2)%3;
				uint32	va = Faces[i].V[j],
						vb = Faces[i].V[(j+1)%3];

				ItTLinkRelloc	it;
				if ((it = relloc.find(CEdgeKey(va, vb))) != relloc.end())
				{
					CCollisionFace	&left0 = Faces[(*it).second.Left];
					CCollisionFace	&left1 = Faces[i];
					// in this case, the left triangle of the edge has already been found.
					// should throw an error
					NLMISC::CVector	eva = Vertices[va], evb = Vertices[vb];
					static char	buf[512];
					sprintf(buf, "Edge issue: (%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f) [left face already found]\n"
								 "left.0:%d: v0:(%.2f,%.2f) v1:(%.2f,%.2f), v2:(%.2f,%.2f) surf=%d material=%d\n"
								 "left.1:%d: v0:(%.2f,%.2f) v1:(%.2f,%.2f), v2:(%.2f,%.2f) surf=%d material=%d",
									eva.x, eva.y, eva.z, evb.x, evb.y, evb.z,
									(*it).second.Left, Vertices[left0.V[0]].x, Vertices[left0.V[0]].y, Vertices[left0.V[1]].x, Vertices[left0.V[1]].y, Vertices[left0.V[2]].x, Vertices[left0.V[2]].y, Faces[(*it).second.Left].Surface, Faces[(*it).second.Left].Material,
									i,                 Vertices[left1.V[0]].x, Vertices[left1.V[0]].y, Vertices[left1.V[1]].x, Vertices[left1.V[1]].y, Vertices[left1.V[2]].x, Vertices[left1.V[2]].y, Faces[i].Surface, Faces[i].Material);
					errors.push_back(std::string(buf));
					continue;
/*					nlerror("On face %d, edge %d: left side of edge (%d,%d) already linked to face %d",
							i, edge, va, vb, (*it).second.Left);*/
				}
				else if ((it = relloc.find(CEdgeKey(vb, va))) != relloc.end())
				{
					// in this case, we must check the right face has been set yet
					if ((*it).second.Right != -1)
					{
						CCollisionFace	&right0 = Faces[(*it).second.Right];
						CCollisionFace	&right1 = Faces[i];
						NLMISC::CVector	eva = Vertices[va], evb = Vertices[vb];
						static char	buf[512];
						sprintf(buf, "Edge issue: (%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f) [right face already found]\n"
									 "right.0:%d: v0:(%.2f,%.2f) v1:(%.2f,%.2f), v2:(%.2f,%.2f)\n"
									 "right.1:%d: v0:(%.2f,%.2f) v1:(%.2f,%.2f), v2:(%.2f,%.2f)",
										eva.x, eva.y, eva.z, evb.x, evb.y, evb.z,
										(*it).second.Right, Vertices[right0.V[0]].x, Vertices[right0.V[0]].y, Vertices[right0.V[1]].x, Vertices[right0.V[1]].y, Vertices[right0.V[2]].x, Vertices[right0.V[2]].y,
										i,                  Vertices[right1.V[0]].x, Vertices[right1.V[0]].y, Vertices[right1.V[1]].x, Vertices[right1.V[1]].y, Vertices[right1.V[2]].x, Vertices[right1.V[2]].y);
						errors.push_back(std::string(buf));
						continue;
/*						nlerror("On face %d, edge %d: right side of edge (%d,%d) already linked to face %d",
								i, edge, vb, va, (*it).second.Right);*/
					}

					(*it).second.Right = i;
					(*it).second.RightEdge = edge;
				}
				else
				{
					// if the edge wasn't present yet, create it and set it up.
					relloc.insert(std::make_pair(CEdgeKey(va, vb), CEdgeInfo(i, edge, -1, -1)));
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
				Faces[left].Edge[leftEdge] = right;
			if (right != -1)
				Faces[right].Edge[rightEdge] = left;
		}
	}
};

}; // NLPACS

#endif // NL_COLLISION_MESH_BUILD_H

/* End of collision_mesh_build.h */
