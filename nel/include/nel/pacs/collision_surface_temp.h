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

#ifndef NL_COLLISION_SURFACE_TEMP_H
#define NL_COLLISION_SURFACE_TEMP_H

#include "nel/misc/types_nl.h"
#include "edge_collide.h"
#include "collision_desc.h"


namespace NLPACS
{


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CEdgeCollideNode : public CEdgeCollide
{
public:
	/// Next edgeCollideNode in the CCollisionSurfaceTemp allocator. 0xFFFFFFFF if none.
	uint32			Next;

public:
	CEdgeCollideNode()
	{
		Next= 0xFFFFFFFF;
	}
};


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionChain
{
public:
	/// First edgeCollideNode in the CCollisionSurfaceTemp allocator. 0xFFFFFFFF if none. This is a List of edgeCollide.
	uint32			FirstEdgeCollide;
	/// The Left/Right surface next this chain.
	CSurfaceIdent	LeftSurface, RightSurface;
	/// the id in the local retriever which generate this chain (temp).
	uint16			ChainId;
	/// In the algorithm, this chain has been tested???
	bool			Tested;
	/// If the chain is an exterior edge
	bool			ExteriorEdge;

public:
	CCollisionChain()
	{
		FirstEdgeCollide= 0xFFFFFFFF;
		Tested= false;
		ExteriorEdge = false;
	}


	/// test if 2 CCollisionChain have same surface neighbors.
	bool		sameSurfacesThan(const CCollisionChain &o) const
	{
		return (LeftSurface==o.LeftSurface  && RightSurface==o.RightSurface)
			|| (LeftSurface==o.RightSurface && RightSurface==o.LeftSurface);
	}

	/// test if Left or Right == surf.
	bool		hasSurface(const CSurfaceIdent &surf) const
	{
		return LeftSurface==surf || RightSurface==surf;
	}

	/// Return Left if surf==Right, else return Right.
	const CSurfaceIdent		&getOtherSurface(const CSurfaceIdent &surf) const
	{
		if(RightSurface==surf)
			return LeftSurface;
		else
			return RightSurface;
	}
};


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CEdgeChainEntry
{
public:
	/// The id of the ordered chain.
	uint16		OChainId;
	/// the first edge of the ordered chain, found in this quad.
	uint16		EdgeStart;
	/// the end edge of the ordered chain, found in this quad. "end edge" is lastEdge+1: numEdges= end-start.
	uint16		EdgeEnd;
};


// ***************************************************************************
/**
 * Temp collision data used for exterior mesh collision detection.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CExteriorEdgeEntry
{
public:
	/// The id of the edge.
	uint16			EdgeId;
	/// The id of the interior chain id
	uint16			ChainId;
	/// The interior and exterior surfaces along this edge
	CSurfaceIdent	Interior, Exterior;

	void			serial(NLMISC::IStream &f)	{ f.serial(EdgeId, ChainId, Interior, Exterior); }
};



// ***************************************************************************
/**
 * Description of the contact of a collision against a chain.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMoveSurfaceDesc
{
public:
	/// This is the 128 bits rational float, when the movement reach this surface.
	CRational64			ContactTime;

	/// To which chain we have collided.
	CSurfaceIdent		LeftSurface, RightSurface;

	/// Is it an interface between exterior and interior
	bool				ExteriorEdge;

	/// true if Dot product between EdgeCollide Norm and Movement is >=0.
	bool				MovementSens;

	/// Chain Id of the exterior edge
	uint16				ChainId;

public:
	CMoveSurfaceDesc() : ExteriorEdge(false) {}
	CMoveSurfaceDesc(CRational64 t, CSurfaceIdent left, CSurfaceIdent right) : ContactTime(t), LeftSurface(left), RightSurface(right), ExteriorEdge(false) {}
	bool	operator<(const CMoveSurfaceDesc &o) const
	{
		return ContactTime<o.ContactTime;
	}

	/// test if Left or Right == surf.
	bool		hasSurface(const CSurfaceIdent &surf)
	{
		return LeftSurface==surf || RightSurface==surf;
	}

	/// Return Left if surf==Right, else return Right.
	const CSurfaceIdent		&getOtherSurface(const CSurfaceIdent &surf)
	{
		if(RightSurface==surf)
			return LeftSurface;
		else
			return RightSurface;
	}
};


// ***************************************************************************
/**
 * Description of the contact of a rot collision against a chain.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CRotSurfaceDesc
{
public:
	/// This tells if this chain (arc of the graph) has been inserted.
	bool				Tested;

	/// To which chain we have collided.
	CSurfaceIdent		LeftSurface, RightSurface;

public:
	CRotSurfaceDesc()  : Tested(false) {}
	CRotSurfaceDesc(CSurfaceIdent left, CSurfaceIdent right) : Tested(false), LeftSurface(left), RightSurface(right) {}

	/// test if Left or Right == surf.
	bool		hasSurface(const CSurfaceIdent &surf)
	{
		return LeftSurface==surf || RightSurface==surf;
	}

	/// Return Left if surf==Right, else return Right.
	const CSurfaceIdent		&getOtherSurface(const CSurfaceIdent &surf)
	{
		if(RightSurface==surf)
			return LeftSurface;
		else
			return RightSurface;
	}
};


// ***************************************************************************
/**
 * Temp collision data used during resolution of collision within surfaces. There should be one CCollisionSurfaceTemp
 * per thread. This is a private class, in essence.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionSurfaceTemp
{
public:
	typedef std::pair<uint16, uint16>	TExteriorEdgeIndex;
//	typedef std::pair<bool, uint8>		TSurfaceLUTEntry;

	class CDistanceSurface
	{
	public:
		uint16							Surface;
		uint16							Instance;
		float							Distance;
		bool							FoundCloseEdge;
		CDistanceSurface() {}
		CDistanceSurface(float distance, uint16 surface, uint16 instance, bool foundCloseEdge) : Surface(surface), Instance(instance), Distance(distance), FoundCloseEdge(foundCloseEdge) {}

		bool		operator () (const CDistanceSurface &a, const CDistanceSurface &b) const
		{
			return a.Distance < b.Distance;
		}
	};

	class CSurfaceLUTEntry
	{
	public:
		bool						IsPossible;
		bool						FoundCloseEdge;
		bool						OnVerticalEdge;
		uint8						Counter;

		void						reset() { IsPossible = false; FoundCloseEdge = false; Counter = 0; }
	};

public:
	/// For CChainQuad::selectEdges().
	uint16							OChainLUT[65536];
	std::vector<CEdgeChainEntry>	EdgeChainEntries;
	std::vector<uint16>				ExteriorEdgeIndexes;

	/// Array of possible near surfaces
//	TSurfaceLUTEntry				SurfaceLUT[65536];
	CSurfaceLUTEntry				SurfaceLUT[65536];
	uint8							OutCounter;
	std::vector<uint16>				PossibleSurfaces;
	std::vector<CDistanceSurface>	SortedSurfaces;


	/// Array of near Collision Chains.
	std::vector<CCollisionChain>	CollisionChains;


	/// result of testMovementWithCollisionChains().
	std::vector<CMoveSurfaceDesc>	MoveDescs;


	/// result of testRotWithCollisionChains().
	std::vector<CRotSurfaceDesc>	RotDescs;


	/// Result of collision testMove().
	TCollisionSurfaceDescVector		CollisionDescs;


	/// CGlobalRetriever instance possibly colliding movement.
	std::vector<sint32>				CollisionInstances;


	/// For testMove/doMove, prec settings.
	CSurfaceIdent					PrecStartSurface;
	NLMISC::CVector					PrecStartPos;
	NLMISC::CVector					PrecDeltaPos;
	bool							PrecValid;

public:

	/// Constructor
	CCollisionSurfaceTemp();

	/// \name Access to EdgeCollideNode
	// @{
	void				resetEdgeCollideNodes();
	/// return first Id.
	uint32				allocEdgeCollideNode(uint32 size=1);
	CEdgeCollideNode	&getEdgeCollideNode(uint32 id);
	// @}

	//
	void				incSurface(sint32 surf)
	{
		if (surf >= 0)
		{
			if (!SurfaceLUT[surf].IsPossible)
				PossibleSurfaces.push_back((uint16)surf);
			SurfaceLUT[surf].IsPossible = true;
			++SurfaceLUT[surf].Counter;
		}
		else
		{
			++OutCounter;
		}
	}

	//
	void				decSurface(sint32 surf)
	{
		if (surf >= 0)
		{
			if (!SurfaceLUT[surf].IsPossible)
				PossibleSurfaces.push_back((uint16)surf);
			SurfaceLUT[surf].IsPossible = true;
			--SurfaceLUT[surf].Counter;
		}
		else
		{
			--OutCounter;
		}
	}

private:
	/// Allocator of EdgeCollideNode.
	std::vector<CEdgeCollideNode>		_EdgeCollideNodes;

};


} // NLPACS


#endif // NL_COLLISION_SURFACE_TEMP_H

/* End of collision_surface_temp.h */
