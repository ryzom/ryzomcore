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

#include "stdmisc.h"

#include "nel/misc/polygon.h"
#include "nel/misc/plane.h"
#include "nel/misc/triangle.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


//==================================//
//		CPolygon implementation     //
//==================================//

// ***************************************************************************
CPolygon::CPolygon(const CVector &a, const CVector &b, const CVector &c)
{
	Vertices.reserve(3);
	Vertices.push_back(a);
	Vertices.push_back(b);
	Vertices.push_back(c);
}

// ***************************************************************************
void CPolygon::toTriFan(std::vector<NLMISC::CTriangle> &dest) const
{
	sint count = (sint) Vertices.size() - 2;
	for(sint k = 0; k < count; ++k)
	{
		dest.push_back(CTriangle(Vertices[0], Vertices[k + 1], Vertices[k + 2]));
	}
}

// ***************************************************************************
float CPolygon::computeArea() const
{
	float area = 0.f;
	sint numVerts = (sint) Vertices.size();
	for(sint k = 0; k < numVerts - 2; ++k)
	{
		CVector v0 = Vertices[k + 1] - Vertices[0];
		CVector v1 = Vertices[k + 2] - Vertices[0];
		area += (v0 ^ v1).norm();
	}
	return 0.5f * fabsf(area);
}

// ***************************************************************************
void			CPolygon::clip(const CPlane	 *planes, uint nPlanes)
{
	if(nPlanes==0 || getNumVertices()==0)
		return;

	// The final polygon has at maximum currentVertices+number of clipping planes.
	// For performance, the vectors are static, so reallocation rarely occurs.
	static	vector<CVector>		tab0, tab1;
	tab0.resize(getNumVertices()+nPlanes);
	tab1.resize(getNumVertices()+nPlanes);
	// Init tab0 with Vertices.
	copy(Vertices.begin(), Vertices.end(), tab0.begin());
	CVector				*in=&(*tab0.begin()), *out= &(*tab1.begin());
	sint				nin= getNumVertices(), nout;
	for(sint i=0;i<(sint)nPlanes;i++)
	{
		nout= planes[i].clipPolygonBack(in, out, nin);
		swap(in, out);
		nin= nout;
		if(nin==0)
			break;
	}

	// Final result in "in".
	Vertices.resize(nin);
	if(nin>0)
	{
		memcpy(&(*Vertices.begin()), in, nin*sizeof(CVector));
	}
}


// ***************************************************************************
void			CPolygon::clip(const std::vector<CPlane> &planes)
{
	if(planes.empty())
		return;
	clip(&(*planes.begin()), (uint)planes.size());
}



// ***************************************************************************
void CPolygon::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serialCont(Vertices);
}

// ***************************************************************************
void CPolygon::getBestTriplet(uint &index0,uint &index1,uint &index2)
{
	nlassert(Vertices.size() >= 3);
	uint i, j, k;
	float bestArea = 0.f;
	const uint numVerts = (uint)Vertices.size();
	for (i = 0; i < numVerts; ++i)
	{
		for (j = 0; j < numVerts; ++j)
		{
			if (i != j)
			{
				for (k = 0; k < numVerts; ++k)
				{
					if (k != i && k != j)
					{
						CVector v0 = Vertices[j] - Vertices[i];
						CVector v1 = Vertices[k] - Vertices[i];
						float area = (v0 ^ v1).norm();
						if (area > bestArea)
						{
							bestArea = area;
							index0 = i;
							index1 = j;
							index2 = k;
						}
					}
				}
			}
		}
	}

}

// ***************************************************************************
void CPolygon::buildBasis(CMatrix &dest)
{
	nlassert(Vertices.size() > 3);
	uint i1, i2, i3;
	getBestTriplet(i1, i2, i3);
	CVector v1 = (Vertices[i2] - Vertices[i1]).normed();
	CVector v2 = (Vertices[i3] - Vertices[i1]).normed();
	CVector K = v2 ^ v1;
	CVector I = v1 - (v1 * K) * v1;
	CVector J = K ^ I;
	dest.setRot(I, J, K);
	dest.setPos(Vertices[i1]);
}

// ***************************************************************************

class CConcavePolygonsVertexDesc
{
public:

	CConcavePolygonsVertexDesc (float length, uint index)
	{
		Length = length;
		Index = index;
	}

	// Length > 0
	float	Length;

	// First vertex index
	uint	Index;
};
typedef std::map<float, CConcavePolygonsVertexDesc> TCConcavePolygonsVertexMap;


// ***************************************************************************
bool CPolygon::toConvexPolygonsEdgeIntersect (const CVector2f& a0, const CVector2f& a1, const CVector2f& b0, const CVector2f& b1)
{
	// both vertical?
	if( a0.x-a1.x==0 && b0.x-b1.x==0 )
		return false;

	// compute intersection of both lines
	CVector2f intersection;

	// first edge vertical?
	if(a0.x - a1.x==0)
	{
		float Ab = (b0.y - b1.y) / (b0.x - b1.x);

		// Intersection
		intersection.x = a0.x;
		intersection.y = b0.y + (a0.x-b0.x) * Ab;
	}
	// second edge vertical?
	else if(b0.x - b1.x==0)
	{
		float Aa = (a0.y - a1.y) / (a0.x - a1.x);

		// Intersection
		intersection.x = b0.x;
		intersection.y = a0.y + (b0.x-a0.x) * Aa;
	}
	// standard case
	else
	{
		float Aa = (a0.y - a1.y) / (a0.x - a1.x);
		float Ba = a0.y - a0.x * Aa;
		float Ab = (b0.y - b1.y) / (b0.x - b1.x);
		float Bb = b0.y - b0.x * Ab;

		// colinear?
		if(Aa==Ab)
			return false;

		// Intersection
		intersection.x = (Bb - Ba) / (Aa - Ab);
		intersection.y = Aa * intersection.x + Ba;
	}

	// In it ?
	return ( ( (a0-intersection)*(a1-intersection) < 0 ) && ( (b0-intersection)*(b1-intersection) < 0 ) );
}

// ***************************************************************************

class CBSPNode2v
{
public:
	CBSPNode2v ()
	{
		Back = NULL;
		Front = NULL;
	}
	CBSPNode2v ( const CPlane &plane, CVector p0, CVector p1, uint v0, uint v1 ) : Plane (plane), P0 (p0), P1 (p1)
	{
		Back = NULL;
		Front = NULL;
		Parent = NULL;
		V0 = v0;
		V1 = v1;
	}
	~CBSPNode2v ()
	{
		if (Front)
			delete Front;
		if (Back)
			delete Back;
	}

	void insert (CBSPNode2v *node)
	{
		// Front ?
		bool p0Front = (Plane * node->P0) > 0;
		bool p1Front = (Plane * node->P1) > 0;
		if (p0Front && p1Front)
		{
			// Front child ?
			if (Front)
				Front->insert (node);
			else
			{
				// Link left
				Front = node;
				node->Parent = this;
			}
		}
		else if ((!p0Front) && (!p1Front))
		{
			// Back child ?
			if (Back)
				Back->insert (node);
			else
			{
				// Link left
				Back = node;
				node->Parent = this;
			}
		}
		else
		{
			// Split vertex
			CVector newVertex = Plane.intersect (node->P0, node->P1);

			// New node
			CBSPNode2v *newNode = new CBSPNode2v (node->Plane, node->P0, newVertex, node->V0, node->V1);

			// Old node
			node->P0 = newVertex;

			// Insert child
			CBSPNode2v **p0Parent;
			CBSPNode2v **p1Parent;

			// Get insertion pointer
			if (p0Front)
			{
				p0Parent = &Front;
				p1Parent = &Back;
			}
			else
			{
				p0Parent = &Back;
				p1Parent = &Front;
			}

			// Insert children
			if (*p0Parent)
			{
				(*p0Parent)->insert (newNode);
			}
			else
			{
				*p0Parent = newNode;
				newNode->Parent = this;
			}

			// Insert children
			if (*p1Parent)
			{
				(*p1Parent)->insert (node);
			}
			else
			{
				*p1Parent = node;
				node->Parent = this;
			}
		}
	}

	bool intersect (const CVector &p0, const CVector &p1, uint v0, uint v1) const
	{
		// Front ?
		bool p0Front = (Plane * p0) > 0;
		bool p1Front = (Plane * p1) > 0;

		if (p0Front != p1Front)
			if ( (v0 != V0) && (v0 != V1) && (v1 != V0) && (v1 != V1) )
				if (CPolygon::toConvexPolygonsEdgeIntersect ((CVector2f) P0, (CVector2f) P1, (CVector2f) p0, (CVector2f) p1))
					return true;

		if (p0Front || p1Front)
		{
			if (Front)
				if (Front->intersect (p0, p1, v0, v1))
					return true;
		}

		if ((!p0Front) || (!p1Front))
		{
			if (Back)
				if (Back->intersect (p0, p1, v0, v1))
					return true;
		}

		return false;
	}

	CBSPNode2v	*Back, *Front, *Parent;
	CPlane		Plane;
	CVector		P0;
	CVector		P1;
	uint		V0;
	uint		V1;
};

// ***************************************************************************

bool CPolygon::toConvexPolygonsLeft (const std::vector<CVector> &vertex, uint a, uint b, uint c)
{
	return ( (vertex[b].x - vertex[a].x) * (vertex[c].y - vertex[a].y) - (vertex[c].x - vertex[a].x) * (vertex[b].y - vertex[a].y) ) < 0;
}

// ***************************************************************************

bool CPolygon::toConvexPolygonsLeftOn (const std::vector<CVector> &vertex, uint a, uint b, uint c)
{
	return ( (vertex[b].x - vertex[a].x) * (vertex[c].y - vertex[a].y) - (vertex[c].x - vertex[a].x) * (vertex[b].y - vertex[a].y) ) <= 0;
}

// ***************************************************************************

bool CPolygon::toConvexPolygonsInCone (const std::vector<CVector> &vertex, uint a, uint b)
{
	// Prev and next
	uint a0 = a+1;
	if (a0==vertex.size())
		a0=0;
	uint a1;
	if (a==0)
		a1= (uint)vertex.size()-1;
	else
		a1= a-1;

	if (toConvexPolygonsLeftOn (vertex, a, a1, a0) )
	{
		return toConvexPolygonsLeft ( vertex, a, b, a0) && toConvexPolygonsLeft ( vertex, b, a, a1);
	}
	else
	{
		return !( toConvexPolygonsLeft ( vertex, a, b, a1) && toConvexPolygonsLeft ( vertex, b, a, a0) );
	}
}

// ***************************************************************************

bool CPolygon::toConvexPolygonsDiagonal (const std::vector<CVector> &vertex, const CBSPNode2v &bsp, uint a, uint b)
{
	// Check it is a border
	if ( ( (b - a) == 1) || ( (a - b) == 1) || ( (a==0) && (b ==(vertex.size()-1))) || ( (b==0) && (a ==(vertex.size()-1))) )
		return true;

	// Check visibility
	if (toConvexPolygonsInCone (vertex, a, b) && toConvexPolygonsInCone (vertex, b, a))
	{
		// Intersection ?
		return !bsp.intersect (vertex[a], vertex[b], a, b);
	}
	return false;
}

// ***************************************************************************

void CPolygon::toConvexPolygonsLocalAndBSP (std::vector<CVector> &localVertices, CBSPNode2v &root, const CMatrix &basis) const
{
	// Invert matrix
	CMatrix invert = basis;
	invert.invert ();

	// Insert vertices in an ordered table
	uint vertexCount = (uint)Vertices.size();
	TCConcavePolygonsVertexMap vertexMap;
	localVertices.resize (vertexCount);
	uint i, j;

	// Transform the vertex
	for (i=0; i<vertexCount; i++)
	{
		CVector local = invert*Vertices[i];
		localVertices[i] = CVector (local.x, local.y, 0);
	}

	// Plane direction
	i=0;
	j=(uint)Vertices.size()-1;
	CVector normal = localVertices[i] - localVertices[j];
	normal = normal ^ CVector::K;
	CPlane clipPlane;
	clipPlane.make(normal, localVertices[i]);

	// Build the BSP root
	root = CBSPNode2v (clipPlane, localVertices[i], localVertices[j], i, j);

	// Insert all others edges
	j=i++;
	for (; i<Vertices.size(); i++)
	{
		// Plane direction
		normal = localVertices[i] - localVertices[j];
		normal = normal ^ CVector::K;
		clipPlane.make(normal, localVertices[i]);

		// Build the BSP root
		root.insert ( new CBSPNode2v (clipPlane, localVertices[i], localVertices[j], i, j) );

		j=i;
	}
}


// ***************************************************************************
bool CPolygon::toConvexPolygons (std::list<CPolygon>& outputPolygons, const CMatrix& basis) const
{
	// Some vertices ?
	if (Vertices.size()>2)
	{
		// Local vertices
		std::vector<CVector>	localVertices;

		// Build the BSP root
		CBSPNode2v root;

		// Build the local array and the BSP
		toConvexPolygonsLocalAndBSP (localVertices, root, basis);

		// Build a vertex list
		std::list<uint> vertexList;
		uint i;
		for (i=0; i<Vertices.size(); i++)
			vertexList.push_back (i);

		// Clip ears while there is some polygons
		std::list<uint>::iterator current=vertexList.begin();
		std::list<uint>::iterator begin=vertexList.begin();
		do
		{
again:;
			// Search for a diagonal
			bool found = false;

			// Get next vertex
			std::list<uint>::iterator first = current;
			std::list<uint>::iterator lastPreviousPrevious=current;
			std::list<uint>::iterator lastPrevious=current;
			lastPrevious++;
			if (lastPrevious==vertexList.end())
				lastPrevious = vertexList.begin();
			std::list<uint>::iterator currentNext = lastPrevious;
			std::list<uint>::iterator last = lastPrevious;
			last++;
			if (last==vertexList.end())
				last = vertexList.begin();
			while (last != current)
			{
				// Is a diagonal ?
				if (
					(toConvexPolygonsDiagonal (localVertices, root, *lastPreviousPrevious, *last)) &&
					(toConvexPolygonsDiagonal (localVertices, root, *currentNext, *last)) &&
					(toConvexPolygonsDiagonal (localVertices, root, *last, *current))
					)
				{
					// Find one
					found = true;
				}
				else
				{
					// Come back
					last = lastPrevious;
					lastPrevious = lastPreviousPrevious;
					break;
				}

				// Next vertex
				lastPreviousPrevious = lastPrevious;
				lastPrevious = last++;
				if (last==vertexList.end())
					last = vertexList.begin();
			}

			// Last polygon ?
			if (last==current)
			{
				// Add a polygon
				outputPolygons.push_back (CPolygon());
				CPolygon &back = outputPolygons.back ();
				back.Vertices.reserve (vertexList.size());

				// Add each vertex in the new polygon
				current=vertexList.begin();
				while (current!=vertexList.end())
				{
					back.Vertices.push_back (Vertices[*current]);
					current++;
				}

				// Exit
				return true;
			}
			else
			{
				std::list<uint>::iterator firstNext = current;
				std::list<uint>::iterator firstNextNext = currentNext;
				if (first != vertexList.begin())
					first--;
				else
				{
					first = vertexList.end();
					first--;
				}

				while (current != first)
				{
					// Is a diagonal ?
					if (
						(toConvexPolygonsDiagonal (localVertices, root, *firstNextNext, *first)) &&
						(toConvexPolygonsDiagonal (localVertices, root, *lastPrevious, *first)) &&
						(toConvexPolygonsDiagonal (localVertices, root, *last, *first))
						)
					{
						// Find one
						found = true;
					}
					else
					{
						// Come back
						first = firstNext;
						break;
					}

					// Next vertex
					firstNextNext = firstNext;
					firstNext = first;
					if (first==vertexList.begin())
					{
						first = vertexList.end();
						first--;
					}
					else
						first--;
				}
			}

			// Found ?
			if (found)
			{
				// Count vertex
				outputPolygons.push_back (CPolygon());
				CPolygon &back = outputPolygons.back ();

				// Vertex count
				uint vertexCount = 1;
				current = first;
				while (current != last)
				{
					vertexCount++;
					current++;
					if (current == vertexList.end())
						current = vertexList.begin();
				}

				// Alloc vertices
				back.Vertices.reserve (vertexCount);

				// Copy and remove vertices
				back.Vertices.push_back (Vertices[*first]);
				first++;
				if (first == vertexList.end())
					first = vertexList.begin();
				while (first != last)
				{
					back.Vertices.push_back (Vertices[*first]);

					// Remove from list
					first = vertexList.erase (first);
					if (first == vertexList.end())
						first = vertexList.begin();
					nlassert (first != vertexList.end());
				}
				back.Vertices.push_back (Vertices[*first]);
				current = begin = last;
				goto again;
			}

			// Next current
			current++;
			if (current == vertexList.end())
				current = vertexList.begin ();
		}
		while (current != begin);
	}
	return false;
}

// ***************************************************************************

bool CPolygon::chain (const std::vector<CPolygon> &other, const CMatrix& basis)
{
	// Local vertices
	std::vector<CVector>	localVertices;

	// Build the BSP root
	CBSPNode2v root;

	// Build the local array and the BSP
	toConvexPolygonsLocalAndBSP (localVertices, root, basis);

	// Local vertices
	std::vector<std::vector<CVector> >	localVerticesOther (other.size());

	// Build the BSP root
	std::vector<CBSPNode2v> rootOther (other.size());

	// Build a copy of the polygons
	std::vector<CPolygon> copy = other;

	// Main copy
	CPolygon mainCopy = *this;

	// For each other polygons
	uint o;
	for (o=0; o<other.size(); o++)
	{
		// Build the local array and the BSP
		other[o].toConvexPolygonsLocalAndBSP (localVerticesOther[o], rootOther[o], basis);
	}

	// Look for a couple..
	uint thisCount = (uint)Vertices.size();
	uint i, j;
	for (o=0; o<other.size(); o++)
	{
		uint otherCount = (uint)other[o].Vertices.size();

		// Try to link in the main polygon
		for (i=0; i<thisCount; i++)
		{
			for (j=0; j<otherCount; j++)
			{
				// Test this segement
				if (!root.intersect (localVertices[i], localVerticesOther[o][j], i, 0xffffffff))
				{
					// Test each other polygons
					uint otherO;
					for (otherO=0; otherO<other.size(); otherO++)
					{
						// Intersect ?
						if (rootOther[otherO].intersect (localVertices[i], localVerticesOther[o][j], 0xffffffff, (otherO == o)?j:0xffffffff))
							break;
					}

					// Continue ?
					if (otherO==other.size())
					{
						// Insert new vertices
						mainCopy.Vertices.insert (mainCopy.Vertices.begin()+i, 2+otherCount, CVector());

						// Copy the first vertex
						mainCopy.Vertices[i] = mainCopy.Vertices[i+otherCount+2];

						// Copy the new vertices
						uint k;
						for (k=0; k<otherCount; k++)
						{
							uint index = j+k;
							if (index>=otherCount)
								index -= otherCount;
							mainCopy.Vertices[i+k+1] = copy[o].Vertices[index];
						}

						// Copy the last one
						mainCopy.Vertices[i+otherCount+1] = copy[o].Vertices[j];
						break;
					}
				}
			}
			if (j!=otherCount)
				break;
		}

		// Not found ?
		if (i==thisCount)
		{
			// Try to link in the sub polygons
			uint otherToCheck;
			for (otherToCheck=o+1; otherToCheck<other.size(); otherToCheck++)
			{
				uint otherToCheckCount = (uint)other[otherToCheck].Vertices.size();
				for (i=0; i<otherToCheckCount; i++)
				{
					for (j=0; j<otherCount; j++)
					{
						// Test this segement
						if (!rootOther[otherToCheck].intersect (localVerticesOther[otherToCheck][i], localVerticesOther[o][j], i, 0xffffffff))
						{
							// Test each other polygons
							uint otherO;
							for (otherO=0; otherO<other.size(); otherO++)
							{
								// Intersect ?
								if (rootOther[otherO].intersect (localVerticesOther[otherToCheck][i], localVerticesOther[o][j],  (otherToCheck == otherO)?i:0xffffffff,  (otherO == o)?j:0xffffffff))
									break;
							}

							// Continue ?
							if (otherO==other.size())
							{
								// Insert new vertices
								copy[otherToCheck].Vertices.insert (copy[otherToCheck].Vertices.begin()+i, 2+otherCount, CVector());

								// Copy the first vertex
								copy[otherToCheck].Vertices[i] = copy[otherToCheck].Vertices[i+otherCount+2];

								// Copy the new vertices
								uint k;
								for (k=0; k<otherCount; k++)
								{
									uint index = j+k;
									if (index>=otherCount)
										index -= otherCount;
									copy[otherToCheck].Vertices[i+k+1] = copy[otherO].Vertices[index];
								}

								// Copy the last one
								copy[otherToCheck].Vertices[i+otherCount+1] = copy[otherO].Vertices[j];
								break;
							}
						}
					}
					if (j!=otherCount)
						break;
				}
				if (i!=otherToCheckCount)
					break;
			}
			if (otherToCheck==other.size())
			{
				// Not ok
				return false;
			}
		}
	}

	// Ok
	*this = mainCopy;
	return true;
}

// ***************************************************************************


//====================================//
//		CPolygon2d implementation     //
//====================================//



// ***************************************************************************
CPolygon2D::CPolygon2D(const CPolygon &src, const CMatrix &projMat)
{
	fromPolygon(src, projMat);
}

// ***************************************************************************
void CPolygon2D::fromPolygon(const CPolygon &src, const CMatrix &projMat /*=CMatrix::Identity*/)
{
	uint size = (uint)src.Vertices.size();
	Vertices.resize(size);
	for (uint k = 0; k < size; ++k)
	{
		CVector proj = projMat * src.Vertices[k];
		Vertices[k].set(proj.x, proj.y);
	}
}

// ***************************************************************************
bool		CPolygon2D::isConvex()
{
	bool Front  = true, Back = false;
	// we apply a dummy algo for now : check whether every vertex is in the same side
	// of every plane defined by a segment of this poly
	uint numVerts = (uint)Vertices.size();
	if (numVerts < 3) return true;
	CVector		segStart, segEnd;
	CPlane		clipPlane;
	for (TVec2fVect::const_iterator it = Vertices.begin(); it != Vertices.end(); ++it)
	{
		segStart.set(it->x, it->y, 0);	          // segment start
		segEnd.set((it + 1)->x, (it + 1)->y, 0);  // segment end
		float n = (segStart - segEnd).norm();     // segment norm
		if (n != 0)
		{
			clipPlane.make(segStart, segEnd, (n > 10 ? n : 10) * CVector::K + segStart); // make a plane, with this segment and the poly normal
			// check each other vertices against this plane
			for (TVec2fVect::const_iterator it2 = Vertices.begin(); it2 != Vertices.end(); ++it2)
			{
				if (it2 != it && it2 != (it + 1)) // the vertices must not be part of the test plane (because of imprecision)
				{

					float dist  = clipPlane * CVector(it2->x, it2-> y, 0);
					if (dist != 0) // midlle pos
					{
						if (dist > 0) Front = true; else Back = true;
						if (Front && Back) return false; // there are both front end back vertices -> failure
					}
				}
			}
		}
	}
	return true;
}

// ***************************************************************************

void		CPolygon2D::buildConvexHull(CPolygon2D &dest) const
{
	nlassert(&dest != this);

	if (this->Vertices.size() == 3) // with 3 points it is always convex
	{
		dest = *this;
		return;
	}
	uint k, l;
	uint numVerts = (uint)Vertices.size();
	CVector2f p, curr, prev;
	uint      pIndex, p1Index, p2Index, pCurr, pPrev;
	// this is not optimized, but not used in realtime.. =)
	nlassert(numVerts >= 3);
	dest.Vertices.clear();

	typedef std::set<uint> TIndexSet;
	TIndexSet leftIndex;
	for (k = 0; k < Vertices.size(); ++k)
	{
		leftIndex.insert(k);
	}


	// 1) find the highest point p of the set. We are sure it belongs to the hull
	pIndex = 0;
	p = Vertices[0];
	for (k = 1; k < numVerts; ++k)
	{
		if (Vertices[k].y < p.y)
		{
			pIndex = k;
			p = Vertices[k];
		}
	}

	leftIndex.erase(pIndex);


	float bestCP = 1.1f;
	p1Index = p2Index = pIndex;

	for (k = 0; k < numVerts; ++k)
	{
		if (k != pIndex)
		{
			for (l = 0; l < numVerts; ++l)
			{
				if (l != pIndex && l != k)
				{
					CVector2f seg1 = (Vertices[l] - p).normed();
					CVector2f seg2 = (Vertices[k] - p).normed();

					//CVector cp = CVector(seg1.x, seg1.y, 0) ^ CVector(seg2.x, seg2.y, 0);
					//float n = fabsf(cp.z);
					float n = seg1 * seg2;
					if (n < bestCP)
					{
						p1Index = l;
						p2Index = k;
						bestCP  = n;
					}
				}
			}
		}
	}


	leftIndex.erase(p2Index);



	// start from the given triplet, and complete the poly until we reach the first point
	pCurr = p2Index;
	pPrev = pIndex;

	curr = Vertices[pCurr];
	prev = Vertices[pPrev];

	// create the first triplet vertices
	dest.Vertices.push_back(Vertices[p1Index]);
	dest.Vertices.push_back(prev);
	dest.Vertices.push_back(curr);

	uint step = 0;

	for(;;)
	{
		bestCP = 1.1f;
		CVector2f seg2 = (prev - curr).normed();
		TIndexSet::iterator bestIt = leftIndex.end();
		for (TIndexSet::iterator it =  leftIndex.begin(); it != leftIndex.end(); ++it)
		{
			if (step == 0 && *it == p1Index) continue;
			CVector2f seg1 = (Vertices[*it] - curr).normed();
			float n = seg1 * seg2;
			if (n < bestCP)
			{
				bestCP = n;
				bestIt = it;
			}
		}

		nlassert(bestIt != leftIndex.end());
		if (*bestIt == p1Index)
		{
			return; // if we reach the start point we have finished
		}
		prev = curr;
		curr = Vertices[*bestIt];
		pPrev = pCurr;
		pCurr = *bestIt;
		// add new point to the destination
		dest.Vertices.push_back(curr);
		++step;
		leftIndex.erase(bestIt);
	}
}

// ***************************************************************************


void CPolygon2D::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);
	f.serialCont(Vertices);
}

// ***************************************************************************
/// get the best triplet of vector. e.g the triplet that has the best surface
void		CPolygon2D::getBestTriplet(uint &index0, uint &index1, uint &index2)
{
	nlassert(Vertices.size() >= 3);
	uint i, j, k;
	float bestArea = 0.f;
	const uint numVerts = (uint)Vertices.size();
	for (i = 0; i < numVerts; ++i)
	{
		for (j = 0; j < numVerts; ++j)
		{
			if (i != j)
			{
				for (k = 0; k < numVerts; ++k)
				{
					if (k != i && k != j)
					{
						CVector2f v0 = Vertices[j] - Vertices[i];
						CVector2f v1 = Vertices[k] - Vertices[i];
						float area = fabsf((CVector(v0.x, v0.y, 0) ^ CVector(v1.x, v1.y, 0)).norm());
						if (area > bestArea)
						{
							bestArea = area;
							index0 = i;
							index1 = j;
							index2 = k;
						}
					}
				}
			}
		}
	}
}


/// ***************************************************************************************
// scan a an edge of a poly and write it into a table
static void ScanEdge(CPolygon2D::TRasterVect &outputVect, sint topY, const CVector2f &v1, const CVector2f &v2, bool rightEdge = true)
{
	 const uint rol16 = 65536;
	 sint ceilY1 = (sint) ceilf(v1.y);
	 sint height;
	 float deltaX, deltaY;
	 float fInverseSlope;
	 sint  iInverseSlope, iPosX;

	 // check whether this segment gives a contribution to the final poly
	 height = (sint) (ceilf(v2.y) - ceilY1);
	 if (height <= 0) return;

	 // compute slope
	 deltaY = v2.y - v1.y;
	 deltaX = v2.x - v1.x;
	 fInverseSlope = deltaX / deltaY;


	 CPolygon2D::TRasterVect::iterator  outputIt = outputVect.begin() + (ceilY1 - topY);

	 // slope with ints
	 iInverseSlope = (sint) (rol16 * fInverseSlope);

	 // sub-pixel accuracy
	 iPosX = (int) (rol16 * (v1.x + fInverseSlope * (ceilY1 - v1.y)));

	 const CPolygon2D::TRasterVect::iterator endIt = outputIt + height;
	 if (rightEdge)
	 {
		 do
		 {
		   outputIt->second =  iPosX >> 16;
		   iPosX += iInverseSlope;
		   ++outputIt;
		 }
		 while (outputIt != endIt);
	 }
	 else
	 {
		 iPosX += (rol16 - 1);
		 do
		 {
		   outputIt->first =  iPosX >> 16;
		   iPosX += iInverseSlope;
		   ++outputIt;
		 }
		 while (outputIt != endIt);
	 }
}


// *******************************************************************************
// This function alow to cycle forward through a vertex vector like if it was a circular list
static inline CPolygon2D::TVec2fVect::const_iterator Next(const CPolygon2D::TVec2fVect::const_iterator &it, const CPolygon2D::TVec2fVect &cont)
{
	nlassert(cont.size() != 0);
	if ((it + 1) == cont.end()) return cont.begin();
	return (it + 1);
}


// *******************************************************************************
// This function alow to cycle backward through a (non null) vertex vector like if it was a circular list
static inline CPolygon2D::TVec2fVect::const_iterator Prev(const CPolygon2D::TVec2fVect::const_iterator &it, const CPolygon2D::TVec2fVect &cont)
{
	nlassert(cont.size() != 0);
	if (it == cont.begin()) return cont.end() - 1;
	return (it - 1);
}


// *******************************************************************************
bool CPolygon2D::isCCWOriented() const
{
	const TVec2fVect &V = Vertices;
	nlassert(Vertices.size() >= 3);
	// compute highest and lowest pos of the poly
	float fHighest = V[0].y;
	float fLowest = fHighest;
	// iterators to the highest and lowest vertex
	TVec2fVect::const_iterator it = V.begin() ;
	const TVec2fVect::const_iterator endIt = V.end();
	TVec2fVect::const_iterator pHighest = V.begin();
	do
	{
		if (it->y < fHighest)
		{
			fHighest = it->y;
			pHighest = it;
		}
		fLowest = std::max(fLowest, it->y);
		++it;
	}
	while (it != endIt);
	// we seek this vertex
	TVec2fVect::const_iterator pHighestRight = pHighest;
	if (fLowest == fHighest)
	{
		// special case : degenerate poly
		while (pHighestRight->x == pHighest->x)
		{
			pHighestRight = Next(pHighestRight, V);
			if (pHighestRight == pHighest) return false; // the poly is reduced to a point, returns an abritrary value
		}
		return pHighest->x <= pHighestRight->x;
	}
	// iterator to the first vertex that has an y different from the top vertex
	while (Next(pHighestRight, V)->y == fHighest)
	{
		pHighestRight = Next(pHighestRight, V);
	}

	// iterator to the first vertex after pHighestRight, that has the same y than the highest vertex
	TVec2fVect::const_iterator pHighestLeft = Next(pHighestRight, V);
	// seek the vertex
	while (pHighestLeft->y != fHighest)
	{
		pHighestLeft = Next(pHighestLeft, V);
	}
	TVec2fVect::const_iterator pPrevHighestLeft = Prev(pHighestLeft, V);
	// we need to get the orientation of the polygon
	// There are 2 case : flat, and non-flat top
	// check for flat top
	if (pHighestLeft->x != pHighestRight->x)
	{
		// compare right and left side
		return pHighestLeft->x <= pHighestRight->x;
	}
	// The top of the poly is sharp
	// We perform a cross product of the 2 highest vect to get its orientation
	 float deltaXN = Next(pHighestRight, V)->x - pHighestRight->x;
	 float deltaYN = Next(pHighestRight, V)->y - pHighestRight->y;
	 float deltaXP = pPrevHighestLeft->x - pHighestLeft->x;
	 float deltaYP = pPrevHighestLeft->y - pHighestLeft->y;
	 return (deltaXN * deltaYP - deltaYN * deltaXP) >= 0;
}

// *******************************************************************************
void	CPolygon2D::computeBorders(TRasterVect &borders, sint &highestY) const
{
	#ifdef NL_DEBUG
		checkValidBorders();
	#endif
	// an 'alias' to the vertices
	const TVec2fVect &V = Vertices;
	if (Vertices.size() < 3)
	{
		borders.clear();
		return;
	}
	bool    ccw;  // set to true when it has a counter clock wise orientation

	// compute highest and lowest pos of the poly
	float fHighest = V[0].y;
	float fLowest  = fHighest;

	// iterators to the thighest and lowest vertex
	TVec2fVect::const_iterator pLowest = V.begin(), pHighest = V.begin();
	TVec2fVect::const_iterator it = V.begin() ;
	const TVec2fVect::const_iterator endIt = V.end();
	do
	{
		if (it->y > fLowest)
		{
			fLowest = it->y;
			pLowest = it;
		}
		else
		if (it->y < fHighest)
		{
			fHighest = it->y;
			pHighest = it;
		}
		++it;
	}
	while (it != endIt);


	sint iHighest = (sint) ceilf(fHighest) ;
	sint iLowest  = (sint) ceilf(fLowest) ;

	highestY = iHighest;


	/// check poly height, and discard null height
	uint polyHeight = iLowest - iHighest;
	if (polyHeight <= 0)
	{
		borders.clear();
		return;
	}

	borders.resize(polyHeight);

	// iterator to the first vertex that has an y different from the top vertex
	TVec2fVect::const_iterator pHighestRight = pHighest;
	// we seek this vertex
	while (Next(pHighestRight, V)->y == fHighest)
	{
		pHighestRight = Next(pHighestRight, V);
	}

	// iterator to the first vertex after pHighestRight, that has the same y than the highest vertex
	TVec2fVect::const_iterator pHighestLeft = Next(pHighestRight, V);
	// seek the vertex
	while (pHighestLeft->y != fHighest)
	{
		pHighestLeft = Next(pHighestLeft, V);
	}

	TVec2fVect::const_iterator pPrevHighestLeft = Prev(pHighestLeft, V);

	// we need to get the orientation of the polygon
	// There are 2 case : flat, and non-flat top

	// check for flat top
	if (pHighestLeft->x != pHighestRight->x)
	{
		// compare right and left side
		if (pHighestLeft->x > pHighestRight->x)
		{
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);
		}
		else
		{
			ccw = false; // the list is CW oriented
		}
	}
	else
	{
		// The top of the poly is sharp
		// We perform a cross product of the 2 highest vect to get its orientation

		 const float deltaXN = Next(pHighestRight, V)->x - pHighestRight->x;
		 const float deltaYN = Next(pHighestRight, V)->y - pHighestRight->y;
		 const float deltaXP = pPrevHighestLeft->x - pHighestLeft->x;
		 const float deltaYP = pPrevHighestLeft->y - pHighestLeft->y;
		 if ((deltaXN * deltaYP - deltaYN * deltaXP) < 0)
		 {
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);
		 }
		 else
		 {
			ccw = false; // the list is CW oriented
		 }
	}


	// compute borders
	TVec2fVect::const_iterator currV, nextV; // current and next vertex
	if (!ccw) // clock wise order ?
	{
		currV = pHighestRight ;
		// compute right edge from top to bottom
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *currV, *nextV, true);
            currV = nextV;
		}
		while (currV != pLowest); // repeat until we reach the bottom vertex

		// compute left edge from bottom to top
		do
		{
   			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *nextV, *currV, false);
			currV = nextV;
		}
		while (currV != pHighestLeft);
	}
	else // ccw order
	{
		currV = pHighestLeft;
		// compute left edge from top to bottom
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *currV, *nextV, false) ;
			currV = nextV;
		}
		while (currV != pLowest) ;

		// compute right edge from bottom to top
		do
		{
			nextV = Next(currV, V);
			ScanEdge(borders, iHighest, *nextV, *currV, true);
			currV = nextV;
		}
		while (currV != pHighestRight)  ;
	}
}

//=========================================================================
// scan outer right edge of a poly
static void ScanOuterEdgeRight(CPolygon2D::TRaster *r, float x1, float y1, float x2, float y2, sint minY)
{
	CPolygon2D::TRaster *currRaster;
	float deltaX, deltaY;
	float inverseSlope;
	sint32  iInverseSlope, iposx;
	sint  height;
	deltaX = x2 - x1;
	height = (sint) (ceilf(y2) - floorf(y1)) ;
	if (height <= 0) return;
	if (deltaX >= 0.f)
	{
		if (height == 1)
		{
			currRaster = r + ((sint) floorf(y1) - minY);
			currRaster->second = std::max((sint) floorf(x2), currRaster->second);
		}
		else
		{
			deltaY = y2 - y1;
			if(deltaY)
				inverseSlope = deltaX / deltaY;
			else
				inverseSlope = 0;
			iInverseSlope = (sint32) (65536.0 * inverseSlope);
			currRaster = r + ((sint) floorf(y1) - minY);
			iposx = (sint32) (65536.0 * (x1 + inverseSlope * (ceilf(y1) - y1))); // sub-pixel accuracy
			if (ceilf(y1) == y1)
			{
				iposx += iInverseSlope;
			}
			do
			{
				currRaster->second = std::max((sint) (iposx >> 16), currRaster->second);
				iposx += iInverseSlope;
				++ currRaster;
				-- height;
			}
			while (height != 1);
			// correction for last line
			currRaster->second = std::max((sint) floorf(x2), currRaster->second);
		}
	}
	else
	{
		deltaY = y2 - y1;
		if(deltaY)
			inverseSlope = deltaX / deltaY;
		else
			inverseSlope = 0;
		iInverseSlope = (sint32) (65536.0 * inverseSlope);
		currRaster = r + ((sint) floorf(y1) - minY);
		currRaster->second = std::max((sint) floorf(x1), currRaster->second);
		++ currRaster;
		iposx = (sint32) (65536.0 * (x1 + inverseSlope * (ceilf(y1) - y1))); // sub-pixel accuracy
		if (ceilf(y1) == y1)
		{
			iposx += iInverseSlope;
		}
		while (--height)
		{
			currRaster->second = std::max((sint) (iposx >> 16), currRaster->second);
			iposx += iInverseSlope;
			++ currRaster;
		}
	}
}

//=========================================================================
// scan outer left edge of a poly
static void ScanOuterEdgeLeft(CPolygon2D::TRaster *r, float x1, float y1, float x2, float y2, sint minY)
{
	CPolygon2D::TRaster *currRaster;
	float deltaX, deltaY;
	float inverseSlope;
	sint32   iInverseSlope, iposx;
	sint  height;
	deltaX = x2 - x1;
	height = (sint) (ceilf(y2) - floorf(y1)) ;
	if (height <= 0) return;
	if (deltaX < 0.f)
	{
		if (height == 1)
		{
			currRaster = r + ((sint) floorf(y1) - minY);
			currRaster->first = std::min((sint) floorf(x2), currRaster->first);
		}
		else
		{
			deltaY = y2 - y1;
			if(deltaY)
				inverseSlope = deltaX / deltaY;
			else
				inverseSlope = 0;
			iInverseSlope = (sint32) (65536.0 * inverseSlope);
			currRaster = r + ((sint) floorf(y1) - minY);
			iposx = (sint32) (65536.0 * (x1 + inverseSlope * (ceilf(y1) - y1))); // sub-pixel accuracy
			if (ceilf(y1) == y1)
			{
				iposx += iInverseSlope;
			}
			do
			{
				currRaster->first = std::min((sint) (iposx >> 16), currRaster->first);
				iposx += iInverseSlope;
				++ currRaster;
				-- height;
			}
			while (height != 1);
			// correction for last line
			currRaster->first = std::min((sint) floorf(x2), currRaster->first);
		}
	}
	else
	{
		deltaY = y2 - y1;
		if(deltaY)
			inverseSlope = deltaX / deltaY;
		else
			inverseSlope = 0;
		iInverseSlope = (sint32) (65536.0 * inverseSlope);
		currRaster = r + ((sint) floorf(y1) - minY);
		currRaster->first = std::min((sint) floorf(x1), currRaster->first);
		++ currRaster;
		iposx = (sint32) (65536.0 * (x1 + inverseSlope * (ceilf(y1) - y1))); // sub-pixel accuracy
		if (ceilf(y1) == y1)
		{
			iposx += iInverseSlope;
		}
		while (--height)
		{
			currRaster->first = std::min((sint) (iposx >> 16), currRaster->first);
			iposx += iInverseSlope;
			++ currRaster;
		}
	}
}


// *******************************************************************************
void CPolygon2D::computeOuterBorders(TRasterVect &borders, sint &minimumY) const
{
	#ifdef NL_DEBUG
		checkValidBorders();
	#endif
	borders.clear();
	// NB : this version is not much optimized, because of the min/max test
	// during rasterization.
	// TODO : optimize if needed ...

	if (Vertices.empty())
	{
		minimumY = -1;
		return;
	}
	const CVector2f *first = &Vertices[0];
	const CVector2f *last  = first + Vertices.size();

	const CVector2f *curr = first, *next, *plowest ,*phighest;
	const CVector2f *pHighestRight, *pHighestRightNext, *pHighestLeft;
	const CVector2f *pPrevHighestLeft;
	double		    deltaXN, deltaYN, deltaXP, deltaYP;
	bool		    ccw;  // true if CCW oriented
	sint            polyHeight;
	sint            highest, lowest;

	float fright   = curr->x;
	float fleft    = curr->x;
	float fhighest = curr->y;
	float flowest  = curr->y;
	plowest = phighest = curr;

	// compute highest and lowest pos of the poly
	do
	{
		fright = std::max(fright, curr->x);
		fleft  = std::min(fleft, curr->x);
		if (curr->y > flowest)
		{
			flowest = curr->y;
			plowest = curr;
		}
		if (curr->y < fhighest)
		{
			fhighest = curr->y;
			phighest = curr;
		}
		++curr;
	}
	while (curr != last);


	highest = (sint) floorf(fhighest);
	lowest = (sint) floorf(flowest);

	polyHeight = lowest - highest + 1;
	nlassert(polyHeight > 0);

	// make room for rasters
	borders.resize(polyHeight);
	// fill with xmin / xman
	sint ileft = (sint) floorf(fleft);
	sint iright = (sint) ceilf(fright);
	minimumY = highest;
	if (flowest == fhighest) // special case : degenerate poly
	{

		borders.resize(1);
		borders.front().first =  ileft;
		borders.front().second =  ileft;
		return;
	}
	//
	for(TRasterVect::iterator it = borders.begin(); it != borders.end(); ++it)
	{
		it->second = ileft;
		it->first = iright;
	}



	pHighestRight = phighest;
	for (;;)
	{
		pHighestRightNext  = pHighestRight + 1;
		if (pHighestRightNext == last) pHighestRightNext = first;
		if (pHighestRightNext->y != pHighestRight->y) break;
		pHighestRight = pHighestRightNext;
	}

	pPrevHighestLeft = pHighestRight;
	pHighestLeft = pHighestRight;
	++pHighestLeft;
	if (pHighestLeft == last) pHighestLeft = first;

	while (pHighestLeft->y != fhighest)
	{
		pPrevHighestLeft = pHighestLeft;
		++pHighestLeft;
		if (pHighestLeft == last) pHighestLeft = first;
	}


	// we need to get the orientation of the polygon
	// There are 2 case : flat, and non-flat top

	// check for flat top
	if (pHighestLeft->x != pHighestRight->x)
	{
		// compare right and left side
		if (pHighestLeft->x > pHighestRight->x)
		{
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);
		}
		else
		{
			ccw = false; // the list is CW oriented
		}
	}
	else
	{
		pHighestRightNext = pHighestRight + 1;
		if (pHighestRightNext == last) pHighestRightNext = first;
		 deltaXN = pHighestRightNext->x - pHighestRight->x;
		 deltaYN = pHighestRightNext->y - pHighestRight->y;
		 deltaXP = pPrevHighestLeft->x - pHighestLeft->x;
		 deltaYP = pPrevHighestLeft->y - pHighestLeft->y;
		 if ((deltaXN * deltaYP - deltaYN * deltaXP) < 0)
		 {
			ccw = true;
			std::swap(pHighestLeft, pHighestRight);
		 }
		 else
		 {
			ccw = false;
		 }
	}

	if (!ccw)
	{
		// clock wise oriented list
		curr = pHighestRight;
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanOuterEdgeRight(&borders[0], curr->x, curr->y, next->x, next->y, minimumY);
			curr = next;
		}
		while (curr != plowest);
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanOuterEdgeLeft(&borders[0], next->x, next->y, curr->x, curr->y, minimumY);
			curr = next;
		}
		while (curr != pHighestLeft);
	}
	else
	{
		// ccw oriented
		curr = pHighestLeft;
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanOuterEdgeLeft(&borders[0], curr->x, curr->y, next->x, next->y, minimumY);
			curr = next;
		}
		while (curr != plowest);
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanOuterEdgeRight(&borders[0], next->x, next->y, curr->x, curr->y, minimumY);
			curr = next;
		}
		while (curr != pHighestRight);
	}
}


//=========================================================================
// scan inner right edge of a poly
static void ScanInnerEdge(CPolygon2D::TRaster *r, float x1, float y1, float x2, float y2, sint minY, bool rightEdge)
{
	const uint rol16 = 65536;
	CPolygon2D::TRaster *currRaster;
	float deltaX, deltaY;
	float inverseSlope;
	sint32  iInverseSlope, iposx;
	sint  height;
	deltaX = x2 - x1;
	height = (sint) (ceilf(y2) - floorf(y1));
	if (height <= 0) return;
	deltaY = y2 - y1;
	if(deltaY)
		inverseSlope = deltaX / deltaY;
	else
		inverseSlope = 0;
	iInverseSlope = (sint32) (rol16 * inverseSlope);
	currRaster = r + ((sint) floorf(y1) - minY);
	//
	iposx = (sint32) (rol16 * (x1 + inverseSlope * (ceilf(y1) - y1))); // sub-pixel accuracy
	if (rightEdge)
	{
		iposx -= rol16 - 1;
		if (deltaX >= 0.f)
		{
			// start of segment
			if (floorf(y1) != y1)
			{
				currRaster->second = std::min((sint) floorf(x1) - 1, currRaster->second);
				++ currRaster;
				-- height;
				if (height == 0) return;
			}
			do
			{
				currRaster->second = std::min((sint) (iposx >> 16), currRaster->second);
				iposx += iInverseSlope;
				++ currRaster;
			}
			while (--height);
		}
		else
		{
			// start of segment
			if (floorf(y1) != y1)
			{
				currRaster->second = std::min((sint) (iposx >> 16), currRaster->second);
				++ currRaster;
				-- height;
				if (height == 0) return;
			}
			while (--height)
			{
				iposx += iInverseSlope;
				currRaster->second = std::min((sint) (iposx >> 16), currRaster->second);
				++ currRaster;
			}
			// fill bottom of segment
			currRaster->second = std::min((sint) floorf(x2) - 1, currRaster->second);
		}
	}
	else
	{
		iposx += rol16 - 1;
		if (deltaX < 0.f)
		{
			// start of segment
			if (floorf(y1) != y1)
			{
				currRaster->first = std::max((sint) ceilf(x1), currRaster->first);
				++ currRaster;
				-- height;
				if (height == 0) return;
			}
			do
			{
				currRaster->first = std::max((sint) (iposx >> 16), currRaster->first);
				iposx += iInverseSlope;
				++ currRaster;
			}
			while (--height);
		}
		else
		{
			// start of segment
			if (floorf(y1) != y1)
			{
				currRaster->first = std::max((sint) (iposx >> 16), currRaster->first);
				++ currRaster;
				-- height;
				if (height == 0) return;
			}
			while (--height)
			{
				iposx += iInverseSlope;
				currRaster->first = std::max((sint) (iposx >> 16), currRaster->first);
				++ currRaster;
			}
			// fill bottom of segment
			currRaster->first = std::max((sint) ceilf(x2), currRaster->first);
		}
	}
}

// *******************************************************************************
void CPolygon2D::computeInnerBorders(TRasterVect &borders, sint &minimumY) const
{
	#ifdef NL_DEBUG
		checkValidBorders();
	#endif
	borders.clear();
	if (Vertices.empty())
	{
		minimumY = -1;
		return;
	}
	const CVector2f *first = &Vertices[0];
	const CVector2f *last  = first + Vertices.size();

	const CVector2f *curr = first, *next, *plowest ,*phighest;
	const CVector2f *pHighestRight, *pHighestRightNext, *pHighestLeft;
	const CVector2f *pPrevHighestLeft;
	double		    deltaXN, deltaYN, deltaXP, deltaYP;
	bool		    ccw;  // true if CCW oriented
	sint            polyHeight;
	sint            highest, lowest;

	float fright   = curr->x;
	float fleft    = curr->x;
	float fhighest = curr->y;
	float flowest  = curr->y;
	plowest = phighest = curr;

	// compute highest (with lowest y) and lowest (with highest y) points of the poly
	do
	{
		fright = std::max(fright, curr->x);
		fleft  = std::min(fleft, curr->x);
		if (curr->y > flowest)
		{
			flowest = curr->y;
			plowest = curr;
		}
		if (curr->y < fhighest)
		{
			fhighest = curr->y;
			phighest = curr;
		}
		++curr;
	}
	while (curr != last);
	if (flowest == fhighest)
	{
		minimumY = -1;
		return;
	}
	highest = (sint) floorf(fhighest);
	lowest = (sint) ceilf(flowest);

	polyHeight = lowest - highest;
	minimumY = highest;
	if (polyHeight == 0)
	{
		minimumY = -1;
		return;
	}
	// make room for rasters
	borders.resize(polyHeight);
	// fill with xmin / xman
	sint ileft = (sint) floorf(fleft) - 1;
	sint iright = (sint) ceilf(fright);
	for(TRasterVect::iterator it = borders.begin(); it != borders.end(); ++it)
	{
		it->second = iright;
		it->first = ileft;
	}
	pHighestRight = phighest;
	for (;;)
	{
		pHighestRightNext  = pHighestRight + 1;
		if (pHighestRightNext == last) pHighestRightNext = first;
		if (pHighestRightNext->y != pHighestRight->y) break;
		pHighestRight = pHighestRightNext;
	}

	pPrevHighestLeft = pHighestRight;
	pHighestLeft = pHighestRight;
	++pHighestLeft;
	if (pHighestLeft == last) pHighestLeft = first;

	while (pHighestLeft->y != fhighest)
	{
		pPrevHighestLeft = pHighestLeft;
		++pHighestLeft;
		if (pHighestLeft == last) pHighestLeft = first;
	}


	// we need to get the orientation of the polygon
	// There are 2 case : flat, and non-flat top

	// check for flat top
	if (pHighestLeft->x != pHighestRight->x)
	{
		// compare right and left side
		if (pHighestLeft->x > pHighestRight->x)
		{
			ccw = true;  // the list is CCW oriented
			std::swap(pHighestLeft, pHighestRight);
		}
		else
		{
			ccw = false; // the list is CW oriented
		}
	}
	else
	{
		pHighestRightNext = pHighestRight + 1;
		if (pHighestRightNext == last) pHighestRightNext = first;
		 deltaXN = pHighestRightNext->x - pHighestRight->x;
		 deltaYN = pHighestRightNext->y - pHighestRight->y;
		 deltaXP = pPrevHighestLeft->x - pHighestLeft->x;
		 deltaYP = pPrevHighestLeft->y - pHighestLeft->y;
		 if ((deltaXN * deltaYP - deltaYN * deltaXP) < 0)
		 {
			ccw = true;
			std::swap(pHighestLeft, pHighestRight);
		 }
		 else
		 {
			ccw = false;
		 }
	}

	if (!ccw)
	{
		// cw oriented
		curr = pHighestRight;
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanInnerEdge(&borders[0], curr->x, curr->y, next->x, next->y, minimumY, true);
			curr = next;
		}
		while (curr != plowest);
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanInnerEdge(&borders[0], next->x, next->y, curr->x, curr->y, minimumY, false);
			curr = next;
		}
		while (curr != pHighestLeft);
	}
	else
	{
		// ccw oriented
		curr = pHighestLeft;
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanInnerEdge(&borders[0], curr->x, curr->y, next->x, next->y, minimumY, false);
			curr = next;
		}
		while (curr != plowest);
		do
		{
			next = curr + 1;
			if (next == last) next = first;
			ScanInnerEdge(&borders[0], next->x, next->y, curr->x, curr->y, minimumY, true);
			curr = next;
		}
		while (curr != pHighestRight);
	}
	// fix for top
	if (floorf(fhighest) != fhighest)
	{
		borders[0].first = 1;
		borders[0].second = 0;
	}
	// fix for bottom
	if (floorf(flowest) != flowest)
	{
		borders.back().first = 1;
		borders.back().second = 0;
	}
}

// *******************************************************************************
void CPolygon2D::checkValidBorders() const
{
	for (uint k = 0; k < Vertices.size(); ++k)
	{
		nlassert(Vertices[k].x >= -32000.f); // coordinate too big !
		nlassert(Vertices[k].x < 32000.f);   // coordinate too big !
		nlassert(Vertices[k].y >= -32000.f); // coordinate too big !
		nlassert(Vertices[k].y < 32000.f);   // coordinate too big !
	}
}

// *******************************************************************************
/// Sum the dot product of this poly vertices against a plane
float		CPolygon2D::sumDPAgainstLine(float a, float b, float c) const
{
	float sum = 0.f;
	for (uint k = 0; k < Vertices.size(); ++k)
	{
		const CVector2f &p = Vertices[k];
		sum += a * p.x + b * p.y + c;
	}
	return sum;
}


// *******************************************************************************
bool  CPolygon2D::getNonNullSeg(uint &index) const
{
	nlassert(!Vertices.empty());
	float bestLength = 0.f;
	sint  bestIndex = -1;
	for (uint k = 0; k < Vertices.size() - 1; ++k)
	{
		float norm2 = (Vertices[k + 1] - Vertices[k]).sqrnorm();
		if ( norm2 > bestLength)
		{
			bestLength = norm2;
			bestIndex = (int) k;
		}
	}
	float norm2 = (Vertices[Vertices.size() - 1] - Vertices[0]).sqrnorm();
	if ( norm2 > bestLength)
	{
		index = (uint)Vertices.size() - 1;
		return true;
	}

	if (bestIndex != -1)
	{
		index = bestIndex;
		return true;
	}
	else
	{
		return false;
	}
}


// *******************************************************************************
void  CPolygon2D::getLineEquation(uint index, float &a, float &b, float &c) const
{
	nlassert(index < Vertices.size());
	const CVector2f &v0 = getSegRef0(index);
	const CVector2f &v1 = getSegRef1(index);

	NLMISC::CVector2f seg = v0 - v1;
	a = seg.y;
	b = - seg.x;
	c = - v0.x * a - v0.y * b;
}

// *******************************************************************************
bool        CPolygon2D::intersect(const CPolygon2D &other) const
{
	nlassert(!other.Vertices.empty());
	uint nonNullSegIndex;
	/// get the orientation of this poly
	if (getNonNullSeg(nonNullSegIndex))
	{
		float a0, b0, c0; /// contains the seg 2d equation
		getLineEquation(nonNullSegIndex, a0, b0, c0);
		float orient = sumDPAgainstLine(a0, b0, c0);

		for (uint k = 0; k < Vertices.size(); ++k)
		{
			/// don't check against a null segment
		    if ( (getSegRef0(k) - getSegRef1(k)).sqrnorm() == 0.f) continue;

			/// get the line equation of the current segment
			float a, b, c; /// contains the seg 2d equation
			getLineEquation(k, a, b, c);
			uint l;
			for (l = 0; l < other.Vertices.size(); ++l)
			{
				const CVector2f &ov = other.Vertices[l];
				if ( orient * (ov.x * a + ov.y * b +c) > 0.f) break;
			}
			if (l == other.Vertices.size()) // all point on the outside
			{
				return false; // outside
			}
		}
		return true;
	}
	else // this poly is just a point
	{
		return other.contains(Vertices[0]);
	}
}

// *******************************************************************************
bool		CPolygon2D::contains(const CVector2f &p, bool hintIsConvex /*= true*/) const
{
	if (hintIsConvex)
	{
		uint numVerts = (uint)Vertices.size();
		nlassert(numVerts >= 0.f);
		for (uint k = 0; k < numVerts; ++k)
		{
			if (getSegRef0(k) != getSegRef1(k))
			{
				float a, b, c; /// contains the seg 2d equation
				getLineEquation(k, a, b, c);
				float orient = a * p.x + b * p.y + c;
				for(uint l = k + 1; l < numVerts; ++l)
				{
					getLineEquation(l, a, b, c);
					float newOrient = a * p.x + b * p.y + c;
					if (newOrient * orient < 0.f) return false;
				}
				return true;
			}
		}
		// the poly reduces to a point
		return p == Vertices[0];
	}
	else
	{
		// concave case
		static std::vector<float> xInter;
		xInter.clear();
		for(uint k = 0; k < Vertices.size(); ++k)
		{
			const CVector2f &p0 = getSegRef0(k);
			const CVector2f &p1 = getSegRef1(k);
			if (p0.y == p1.y)
			{
				if (p.y == p0.y)
				{
					if ((p.x >= p0.x && p.x <= p1.x)
						|| (p.x >= p1.x && p.x <= p0.x))
					{
						return true;
					}
				}
			}
			if ((p.y >= p0.y && p.y < p1.y) ||
				(p.y >= p1.y && p.y < p0.y)
			   )
			{

				float inter = p0.x + (p.y - p0.y) * (p1.x - p0.x) / (p1.y- p0.y);
				xInter.push_back(inter);
			}
		}
		if (xInter.size() < 2) return false;
		std::sort(xInter.begin(), xInter.end());
		for(uint k = 0; k < xInter.size() - 1; ++k)
		{
			if (p.x >= xInter[k] && p.x <= xInter[k + 1])
			{
				return (k & 1) == 0;
			}
		}
		return false;
	}
}


// *******************************************************************************
CPolygon2D::CPolygon2D(const CTriangle &tri, const CMatrix &projMat)
{
	Vertices.resize(3);
	NLMISC::CVector proj[3] = { projMat * tri.V0, projMat * tri.V1, projMat * tri.V2 };
	Vertices[0].set(proj[0].x, proj[0].y);
	Vertices[1].set(proj[1].x, proj[1].y);
	Vertices[2].set(proj[2].x, proj[2].y);
}

// *******************************************************************************
void CPolygon2D::getBoundingRect(CVector2f &minCorner, CVector2f &maxCorner) const
{
	nlassert(!Vertices.empty());
	minCorner = maxCorner = Vertices[0];
	uint numVertices = (uint)Vertices.size();
	for(uint k = 0; k < numVertices; ++k)
	{
		minCorner.minof(minCorner, Vertices[k]);
		maxCorner.maxof(minCorner, Vertices[k]);
	}
}

// *******************************************************************************
bool operator ==(const CPolygon2D &lhs,const CPolygon2D &rhs)
{
	if (lhs.Vertices.size() != rhs.Vertices.size()) return false;
	return std::equal(lhs.Vertices.begin(), lhs.Vertices.end(), rhs.Vertices.begin());
}

// *******************************************************************************
bool operator < (const CPolygon2D &lhs, const CPolygon2D &rhs)
{
	if (lhs.Vertices.size() != rhs.Vertices.size()) return lhs.Vertices.size() < rhs.Vertices.size();
	for(uint k = 0; k < lhs.Vertices.size(); ++k)
	{
		if (lhs.Vertices[k] != rhs.Vertices[k]) return lhs.Vertices[k] < rhs.Vertices[k];
	}
	return false;
}


// *******************************************************************************
static inline bool testSegmentIntersection(const CVector2f &a, const CVector2f &b,
										   const CVector2f &c, const CVector2f &d)
{
	double denom = a.x * double(d.y - c.y) +
			b.x * double(c.y - d.y) +
			d.x * double(b.y - a.y) +
			c.x * double(a.y - b.y);

	if (denom == 0) return false;
	//
	double num = a.x * double(d.y - c.y) +
				 c.x * double(a.y - d.y) +
				 d.x * double(c.y - a.y);

	if (num == 0 || (num == denom)) return false;
	double lambda = num / denom;
	if (lambda <= 0 || lambda >= 1) return false;
	//
	num = - (a.x * double(c.y - b.y) +
		  b.x * double(a.y - c.y) +
		  c.x * double(b.y - a.y));

	if (num == 0 || (num == denom)) return false;
	lambda = num / denom;
	if (lambda <= 0 || lambda >= 1) return false;
	return true;
}


// *******************************************************************************
bool CPolygon2D::selfIntersect() const
{
	if (Vertices.size() < 3) return false;
	uint numEdges = (uint)Vertices.size();
	for(uint k = 0; k < numEdges; ++k)
	{
		// test intersection with all other edges that don't share a vertex with this one
		const CVector2f &p0 = getSegRef0(k);
		const CVector2f &p1 = getSegRef1(k);
		for(uint l = 0; l < k; ++l)
		{
			const CVector2f &v0 = getSegRef0(l);
			const CVector2f &v1 = getSegRef1(l);
			if (v0 == p0 || v0 == p1 || v1 == p0 || v1 == p1) continue;
			//
			if (testSegmentIntersection(p0, p1, v0, v1)) return true;
		}
	}
	return false;
}



} // NLMISC





















