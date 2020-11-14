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

#ifndef NL_MRM_INTERNAL_H
#define NL_MRM_INTERNAL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mrm_mesh.h"


namespace NL3D
{

// ***************************************************************************
struct CLinearEquation
{
	struct Element
	{
		uint32	index;
		float	factor;

		Element(uint32 i, float f)
		{
			index = i;
			factor = f;
		}
	};

	std::vector<Element> Elts;

	void init(uint32 ii)
	{
		clear();
		Elts.push_back (Element(ii, 1.0f));
	}

	// this = this + eq * factor
	void add(CLinearEquation& eq, float factor)
	{
		Element tmp(0, 0.0f);

		for (uint32 i = 0; i < eq.Elts.size(); ++i)
		{
			tmp.index = eq.Elts[i].index;
			tmp.factor = factor * eq.Elts[i].factor;
			Elts.push_back (tmp);
		}
	}

	// this = this * factor
	void mul (float factor)
	{
		for (uint32 i = 0; i < Elts.size(); ++i)
			Elts[i].factor *= factor;
	}

	void clear()
	{
		NLMISC::contReset (Elts);
	}
};

// ***************************************************************************
/**
 * An internal mesh vertex representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMVertex
{
public:
	// Original / Dest position.
	CVector					Current,Original;
	std::vector<CVector>	BSCurrent;
	// For Skinning.
	CMesh::CSkinWeight		CurrentSW, OriginalSW;
	std::vector<sint>		SharedFaces;
	sint					CollapsedTo;
	// Final index in the coarser mesh.
	sint					CoarserIndex;

	// The link to the current meshInterface vertex.
	CMesh::CInterfaceLink	InterfaceLink;

public:
	CMRMVertex() {CollapsedTo=-1;}
};


// ***************************************************************************
/**
 * An internal mesh vertex attribute (UV, color, normal...) representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMAttribute
{
public:
	CVectorH		Current,Original;
	std::vector<CVectorH>		BSCurrent;
	sint			CollapsedTo;		// -2 <=> "must interpolate from Current to Original".
	// Final index in the coarser mesh.
	sint			CoarserIndex;

public:
	// temporary data in construction:
	sint			InterpolatedFace;
	sint			NbSharedFaces;
	bool			Shared;
	// A wedge is said "shared" if after edge is collapsed, he lost corners.
	// If NbSharedFaces==0, this wedge has been entirely destroyed.

public:
	CMRMAttribute() {CollapsedTo=-1;}
};


// ***************************************************************************
/**
 * An internal mesh edge Index representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMEdge
{
	sint	v0,v1;
	CMRMEdge() {}
	CMRMEdge(sint a, sint b) {v0= a; v1=b;}
	bool operator==(const CMRMEdge &o) const
	{
		// Order means nothing  ( (v0,v1) == (v1,v0) ).... Kick it.
		return (v0==o.v0 && v1==o.v1) || (v0==o.v1 && v1==o.v0);
	}
	bool operator<(const CMRMEdge &o) const
	{
		// Order means nothing  ( (v0,v1) == (v1,v0) ).... Kick it.
		sint max0= std::max(v0,v1);
		sint min0= std::min(v0,v1);
		sint max1= std::max(o.v0,o.v1);
		sint min1= std::min(o.v0,o.v1);
		if(max0!=max1)
			return max0<max1;
		else
			return min0<min1;
	}
};


// ***************************************************************************
struct	CMRMFaceBuild;
/**
 * A tuple Edge/Face.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMEdgeFace : public CMRMEdge
{
	CMRMFaceBuild		*Face;
	CMRMEdgeFace();
	CMRMEdgeFace(sint a, sint b, CMRMFaceBuild *f)
	{
		v0=a; v1=b;
		Face= f;
	}
	CMRMEdgeFace(const CMRMEdge &e, CMRMFaceBuild *f) : CMRMEdge(e)
	{
		Face= f;
	}

};


// ***************************************************************************
/**
 * The map of edge collapses.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
typedef	std::multimap<float, CMRMEdgeFace>	TEdgeMap;
typedef	TEdgeMap::iterator					ItEdgeMap;


// ***************************************************************************
/**
 * An internal mesh extended face representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMFaceBuild : public CMRMFace
{
public:
	// temporary data in construction:
	// The interpolated attrbute of the face.
	CVectorH		InterpolatedAttribute;
	std::vector<CVectorH>	BSInterpolated;

	// Is this face deleted in the current MRM collapse?
	bool			Deleted;
	// The iterator of the edges in the EdgeCollapse list.
	ItEdgeMap		It0, It1, It2;
	// The mirror value of iterator: are they valid???
	bool			ValidIt0, ValidIt1, ValidIt2;


public:
	CMRMFaceBuild()
	{
		Deleted=false;
		ValidIt0= ValidIt1= ValidIt2= false;
	}
	CMRMFaceBuild &operator=(const CMRMFace &f)
	{
		(CMRMFace &)(*this)=f;
		return *this;
	}

	// Edges.
	//=======
	sint	getAssociatedEdge(const CMRMEdge &edge) const
	{
		sint v0= edge.v0;
		sint v1= edge.v1;
		if(Corner[0].Vertex==v0 && Corner[1].Vertex==v1)	return 0;
		if(Corner[0].Vertex==v1 && Corner[1].Vertex==v0)	return 0;
		if(Corner[1].Vertex==v0 && Corner[2].Vertex==v1)	return 1;
		if(Corner[1].Vertex==v1 && Corner[2].Vertex==v0)	return 1;
		if(Corner[0].Vertex==v0 && Corner[2].Vertex==v1)	return 2;
		if(Corner[0].Vertex==v1 && Corner[2].Vertex==v0)	return 2;
		return -1;
	}
	bool	hasEdge(const CMRMEdge &edge) const
	{
		return getAssociatedEdge(edge)!=-1;
	}
	CMRMEdge	getEdge(sint eId) const
	{
		if(eId==0) return CMRMEdge(Corner[0].Vertex, Corner[1].Vertex);
		if(eId==1) return CMRMEdge(Corner[1].Vertex, Corner[2].Vertex);
		if(eId==2) return CMRMEdge(Corner[2].Vertex, Corner[0].Vertex);
		nlstop;
		return CMRMEdge(-1,-1);
	}
	void	invalidAllIts(TEdgeMap &edgeMap)
	{
		ValidIt0= ValidIt1= ValidIt2= false;
		It0= edgeMap.end();
		It1= edgeMap.end();
		It2= edgeMap.end();
	}
	void	invalidEdgeIt(const CMRMEdge &e, TEdgeMap &edgeMap)
	{
		if(e== getEdge(0))
			It0= edgeMap.end(), ValidIt0= false;
		else if(e== getEdge(1))
			It1= edgeMap.end(), ValidIt1= false;
		else if(e== getEdge(2))
			It2= edgeMap.end(), ValidIt2= false;
		else nlstop;
	}
	bool	validEdgeIt(const CMRMEdge &e)
	{
		if(e== getEdge(0)) return ValidIt0;
		if(e== getEdge(1)) return ValidIt1;
		if(e== getEdge(2)) return ValidIt2;
		nlstop;
		return false;
	}

	// Vertices.
	//==========
	bool	hasVertex(sint	numvertex)
	{
		return Corner[0].Vertex==numvertex || Corner[1].Vertex==numvertex || Corner[2].Vertex==numvertex;
	}


	// Wedges.
	//==========
	bool	hasWedge(sint attribId, sint numwedge)
	{
		return Corner[0].Attributes[attribId]==numwedge ||
			Corner[1].Attributes[attribId]==numwedge ||
			Corner[2].Attributes[attribId]==numwedge;
	}
	sint	getAssociatedWedge(sint attribId, sint numvertex)
	{
		if(Corner[0].Vertex==numvertex)	return Corner[0].Attributes[attribId];
		if(Corner[1].Vertex==numvertex)	return Corner[1].Attributes[attribId];
		if(Corner[2].Vertex==numvertex)	return Corner[2].Attributes[attribId];
		return -1;
	}
};


// ***************************************************************************
/**
 * An internal polygon with LOD information for Interface system
 */
class	CMRMSewingMesh
{
	struct	CLod
	{
		// A list of edge that must be collapsed for this Lod. NB: sorted from first to collapse to last to collapse
		std::vector<CMRMEdge>	EdgeToCollapse;
	};

	// A list of Lods.
	std::vector<CLod>	_Lods;

public:

	/** Build a MRM sewing mesh from a CMeshBuild interface.
	 */
	void	build(const CMesh::CInterface &meshInt, uint nWantedLods, uint divisor);

	/** >=0 if the lod has this edge to collapse. -1 else. NB: order of collapse is returned.
	 *	\param vertToCollapse is the vertex id which must be collapsed to the other (ie the one which moves/dissapear)
	 */
	sint	mustCollapseEdge(uint lod, const CMRMEdge &edge, uint &vertToCollapse) const;

	/// get the number of edge to collapse for a lod
	sint	getNumCollapseEdge(uint lod) const;

};




} // NL3D


#endif // NL_MRM_INTERNAL_H

/* End of mrm_internal.h */
