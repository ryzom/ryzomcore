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

#ifndef NL_RETRIEVER_INSTANCE_H
#define NL_RETRIEVER_INSTANCE_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "local_retriever.h"
#include "edge_quad.h"

namespace NLPACS
{
class CCollisionSurfaceTemp;

/// Precision of Snap. 1/1024 meter. If you change this, CEdgeCollide::testPointMove() won't work.
const	float	SnapPrecision= 1024;


/**
 * An instance of a local retriever.
 * It defines a physical/geographic zone of landscape.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CRetrieverInstance
{
public:
	/**
	 * A neighbor link
	 */
	class CLink
	{
	public:
		uint16				Instance;
		uint16				BorderChainId;
		uint16				ChainId;
		uint16				SurfaceId;
		CLink() { reset(); }
		void	reset() { Instance = 0xFFFF; BorderChainId = 0xFFFF; ChainId = 0xFFFF; SurfaceId = 0xFFFF; }
		void	serial(NLMISC::IStream &f) { f.serial(Instance, BorderChainId, ChainId, SurfaceId); }
	};


protected:
	friend class CGlobalRetriever;

	/**
	 * The link to another node
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	struct CAStarNodeAccess
	{
		sint32	InstanceId;
		uint16	NodeId;
		uint16	ThroughChain;

		CAStarNodeAccess() :InstanceId(-1), NodeId(0xffff), ThroughChain(0xffff) {}

		bool	operator == (const CAStarNodeAccess &node) const { return InstanceId == node.InstanceId && NodeId == node.NodeId; }
		bool	operator != (const CAStarNodeAccess &node) const { return InstanceId != node.InstanceId || NodeId != node.NodeId; }
	};

	/**
	 * The information bound to the surfaces nodes.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	struct CAStarNodeInfo
	{
		/// The position of this node.
		NLMISC::CVector2f	Position;

		/// The cost to this node.
		float				Cost;
		float				F;

		/// The parent link.
		CAStarNodeAccess	Parent;
	};

	///
	std::vector<CAStarNodeInfo>			_NodesInformation;

protected:
	/// The id of this instance.
	sint32								_InstanceId;

	/// The id of the retrievable surface pattern.
	sint32								_RetrieverId;

	/// The type of the instance (see CLocalRetriever::_Type)
	CLocalRetriever::EType				_Type;

	/// @name Instance displacement.
	// @{
	uint8								_Orientation;
	NLMISC::CVector						_Origin;

	/// The BBox of the instance.
	NLMISC::CAABBox						_BBox;

	// @}

	/// @name Instance linkage.
	// @{

	/// The instance ids of the neighbors.
	std::vector<sint32>					_Neighbors;

	/// The neighbor chains on the border.
	std::vector<CLink>					_BorderChainLinks;
	// @}

	/// @name Interior instance coolision management.
	// @{

	/** The edges of the exterior collision mesh, stored in a quad grid.
		The edges are stored in the instance rather than in the local retriever.
	*/
	CEdgeQuad							_ExteriorEdgeQuad;

	// @}

public:
	/// Default constructor.
	CRetrieverInstance();

	/// Resets the instance. This doesn't affect any neighboring instances...
	void								reset();
	/// Resets links to the given instance. This doesn't affect any neighbor.
	void								resetLinks(uint32 id);
	/// Resets links of the instance. This doesn't affect any neighboring instances...
	void								resetLinks();

	/// Returns the id of this instance.
	sint32								getInstanceId() const { return _InstanceId; }
	/// Returns the id of the retriever associated to this instance.
	sint32								getRetrieverId() const { return _RetrieverId; }
	/// Returns the orientation of the instance in relation to the retriever.
	uint8								getOrientation() const { return _Orientation; }
	/// Returns the origin translation of this instance.
	NLMISC::CVector						getOrigin() const { return _Origin; }
	/// Returns the type of the instance
	CLocalRetriever::EType				getType() const { return _Type; }

	/// Gets the neighbors.
	std::vector<sint32>					getNeighbors() const { return _Neighbors; }
	/// Gets the id of the neighbor on the edge.
	sint32								getNeighbor(uint n) const { return _Neighbors[n]; }

	/// Gets the ids of the neighbor chains on the given edge.
	const std::vector<CLink>			&getBorderChainLinks() const { return _BorderChainLinks; }
	/// Gets the id of the nth neighbor chain on the given edge.
	CLink								getBorderChainLink(uint n) const { return _BorderChainLinks[n]; }
	/// Resets border chains links
	void								resetBorderChainLinks(const std::vector<uint> &links);
	/// Forces border chain link
	void								forceBorderChainLink(uint border, uint frontInstance, uint frontBorder, uint frontChain, uint frontSurface)
	{
		if (border >= _BorderChainLinks.size())
		{
			nlwarning("forceBorderChainLink(): couldn't force border %d on instance %d, doesn't exist!", border, _InstanceId);
			return;
		}

		if (_BorderChainLinks[border].Instance != 0xffff ||
			_BorderChainLinks[border].BorderChainId != 0xffff ||
			_BorderChainLinks[border].ChainId != 0xffff ||
			_BorderChainLinks[border].SurfaceId != 0xffff)
		{
			nlwarning("forceBorderChainLink(): forces border %d on instance %d whereas link is initialised yet, incoherences may appear!", border, _InstanceId);
		}

		_BorderChainLinks[border].Instance = uint16(frontInstance);
		_BorderChainLinks[border].BorderChainId = uint16(frontBorder);
		_BorderChainLinks[border].ChainId = uint16(frontChain);
		_BorderChainLinks[border].SurfaceId = uint16(frontSurface);
	}

	/// Returns the number of the edge on the instance corresponding to the edge on the retriever.
	uint8								getInstanceEdge(uint8 retrieverEdge) const { return (retrieverEdge+_Orientation)%4; }
	/// Returns the number of the edge on the retriever corresponding to the edge on the instance.
	uint8								getRetrieverEdge(uint8 instanceEdge) const { return (instanceEdge+4-_Orientation)%4; }

	/// Returns the bbox of the instance (using the translation and orientation of the retriever)
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }

	/// Inits the instance (after a serial for instance.)
	void								init(const CLocalRetriever &retriever);

	/** Inits the edgequad (only for Interior instances, and only after all the landscape
	 * instances have been built.
	 */
	void								initEdgeQuad(CGlobalRetriever &gr);
	/// link the edge quad of the interior with the landscape instances
	void								linkEdgeQuad(CGlobalRetriever &gr);

	/// Builds the instance.
	void								make(sint32 instanceId, sint32 retrieverId, const CLocalRetriever &retriever,
											 uint8 orientation, const NLMISC::CVector &origin);

	/// Links the instance to a given neighbor on the given edge.
	void								link(CRetrieverInstance &neighbor,
											 const std::vector<CLocalRetriever> &retrievers);

	/// Unlinks the instance. The neighbor instance are AFFECTED.
	void								unlink(std::vector<CRetrieverInstance> &instances);


	/**
	 * Retrieves the position in the instance from an estimated position.
	 * WARNING: the estimated position is a GLOBAL position, and the returned position
	 * is a LOCAL position (to the retriever).
	 */
//	CLocalRetriever::CLocalPosition		retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst) const;
	void								retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst, bool sortByDistance=true) const;
	/**
	 * Retrieves the position in the instance from an estimated position (double instead.)
	 * WARNING: the estimated position is a GLOBAL position, and the returned position
	 * is a LOCAL position (to the retriever).
	 */
	void								retrievePosition(const NLMISC::CVectorD &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst, bool sortByDistance=true) const;
	/**
	 * Retrieves the position in the instance from an estimated position (double instead.)
	 * WARNING: the estimated position is a GLOBAL position, and the returned position
	 * is a LOCAL position (to the retriever).
	 */
	//void								retrieveAccuratePosition(const NLMISC::CVectorD &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst, ) const;

	/// For the interior instances, snaps the point to the ground.
	void								snapToInteriorGround(ULocalPosition &position, const CLocalRetriever &retriever) const;

	///
	void								snap(ULocalPosition &position, const CLocalRetriever &retriever) const;

	/// Serialises this CRetrieverInstance.
	void								serial(NLMISC::IStream &f);

	/// Computes the position in the local axis of the retriever from a global position.
	NLMISC::CVector						getLocalPosition(const NLMISC::CVector &globalPosition) const;
	/// Computes the position in the local axis of the retriever from a global position.
	NLMISC::CVector						getLocalPosition(const NLMISC::CVectorD &globalPosition) const;
	/// Computes the position in the global axis from a local position (in the retriever axis).
	NLMISC::CVector						getGlobalPosition(const NLMISC::CVector &localPosition) const;
	/// Computes the position (as double) in the global axis from a local position (in the retriever axis).
	NLMISC::CVectorD					getDoubleGlobalPosition(const NLMISC::CVector &localPosition) const;

	/// Returns the bbox of the instance.
//	NLMISC::CAABBox						getBBox() { return _BBox; }


	/// \name Vector Snapping.
	// @{
	/** Snap a vector at 1mm (1/1024). v must be a local position (ie range from -80 to +240).
	 * Doing this, we are sure we have precision of 9+10 bits, which is enough for 24 bits float precision.
	 * NB: z is not snapped.
	 */
	static void	snapVector(CVector &v)
	{
		v.x= (float)floor(v.x*SnapPrecision)/SnapPrecision;
		v.y= (float)floor(v.y*SnapPrecision)/SnapPrecision;
	}
	/** Snap a vector at 1mm (1/1024). v must be a local position (ie range from -80 to +240).
	 * Doing this, we are sure we have precision of 9+10 bits, which is enough for 24 bits float precision.
	 */
	static void	snapVector(CVector2f &v)
	{
		v.x= (float)floor(v.x*SnapPrecision)/SnapPrecision;
		v.y= (float)floor(v.y*SnapPrecision)/SnapPrecision;
	}
	// @}

	/// \name Collision part
	// @{

	/** Test for collisions with the exterior mesh of an interior instance.
		This only works for interior instances !!
		see also testCollision() in CLocalRetriever
	*/
	void								testExteriorCollision(CCollisionSurfaceTemp &cst,
															  const NLMISC::CAABBox &bboxMoveLocal,
															  const NLMISC::CVector2f &transBase,
															  const CLocalRetriever &localRetriever) const;

	// @}
};

}; // NLPACS

#endif // NL_RETRIEVER_INSTANCE_H

/* End of retriever_instance.h */
