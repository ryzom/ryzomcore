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

#ifndef NL_GLOBAL_RETRIEVER_H
#define NL_GLOBAL_RETRIEVER_H

#include <vector>
#include <list>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/thread.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/task_manager.h"

#include "local_retriever.h"
#include "retriever_instance.h"
#include "vector_2s.h"
#include "collision_surface_temp.h"
#include "retriever_bank.h"

#include "nel/pacs/u_global_retriever.h"


#include "quad_grid.h"


namespace NLPACS
{

/**
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CGlobalRetriever : public UGlobalRetriever
{
public:

	enum
	{
		MissingLr = -2,
		Failed = -1,
		Success = 0
	};

	/**
	 * The global position in the global retriever.
	 * Contains an instance id and a local position within the instance.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CGlobalPosition : public UGlobalPosition
	{
	public:
		/**
		 * Constuctor.
		 * Creates a CGlobalPosition from an instanceId and a local position.
		 */
		CGlobalPosition(sint32 instanceId=-1,
					   const CLocalRetriever::CLocalPosition &localPosition=CLocalRetriever::CLocalPosition())
		{
			InstanceId=instanceId;
			LocalPosition=localPosition;
		}

		/// Serialises the global position.
		//void							serial(NLMISC::IStream &f) { f.serial(InstanceId, LocalPosition); }
	};

	class CLocalPath
	{
	public:
		sint32								InstanceId;
		CLocalRetriever::CLocalPosition		Start;
		CLocalRetriever::CLocalPosition		End;
		std::vector<CVector2s>				Path;
	};

	typedef std::vector<CLocalPath>			CGlobalPath;

protected:
	friend class CLrLoader;

	// Lr async loader
	class CLrLoader : public NLMISC::IRunnablePos
	{
	public:
		/// Finished task
		volatile bool		Finished;
		/// Finished successfully
		volatile bool		Successful;
		/// Lr Id
		uint				LrId;
		/// Lr to load
		std::string			LoadFile;

		/// Loading buffer
		NLMISC::CMemStream					_Buffer;

		// valid states are:
		// Idle==true && Finished==true
		// Idle==false && Finished==false
		// Idle==false && Finished==true

		CLrLoader(const NLMISC::CVector &position) : Finished(true)
		{
			Position = position;
		}

		void	run();
		void	getName (std::string &result) const;
	};

	std::list<CLrLoader>					_LrLoaderList;

private:
	///
	mutable CCollisionSurfaceTemp			_InternalCST;

	/// Used to retrieve the surface. Internal use only, to avoid large amount of new/delete
	mutable std::vector<uint8>				_RetrieveTable;

protected:

	/// The CRetrieverBank where the commmon retrievers are stored.
	const CRetrieverBank					*_RetrieverBank;

	/// The instances of the global retriever.
	mutable std::vector<CRetrieverInstance>	_Instances;

	/// The grid of instances
	mutable CQuadGrid<uint32>				_InstanceGrid;

	/// The axis aligned bounding box of the global retriever.
	NLMISC::CAABBox							_BBox;

	/// Forbidden instance for retrieve position
	mutable std::vector<sint32>				_ForbiddenInstances;

public:
	/// @name Initialisation
	// @{

	/**
	 * Constructor.
	 * Creates a global retriever with given width, height and retriever bank.
	 */
	CGlobalRetriever(const CRetrieverBank *bank=NULL)
		: _RetrieverBank(bank)
	{ }
	virtual ~CGlobalRetriever();


	/// Setup an empty global retriever
	void							init();

	/// Fill the quadgrid with the instances
	void							initQuadGrid();

	/// Init the retrieve table
	void							initRetrieveTable();

	// @}


	/// @name Selectors
	//@{


	/// Gets the BBox of the global retriever.
	const NLMISC::CAABBox			&getBBox() const { return _BBox; }


	/// Gets the vector of retriever instances that compose the global retriever.
	const std::vector<CRetrieverInstance>	&getInstances() const { return _Instances; }

	/// Gets the retriever instance referred by its id.
	const CRetrieverInstance		&getInstance(uint id) const { return _Instances[id]; }


	/** Select the instances that are in contact with the given bbox.
	 * The selected instances are stored in CCollisionSurfaceTemp.CollisionInstances
	 */
	bool							selectInstances(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst, UGlobalPosition::TType type = UGlobalPosition::Unspecified) const;

	/// Get the retriever bank associated to this global retriever.
	const CRetrieverBank			*getRetrieverBank() const { return _RetrieverBank; }

	/// Get the local retriever
	const CLocalRetriever			&getRetriever(uint32 id) const { return _RetrieverBank->getRetriever(id); }


	/// Get the material at this position
	uint32							getMaterial(const UGlobalPosition &pos) const
	{
		if (pos.InstanceId < 0 || pos.InstanceId >= (sint)_Instances.size())
			return 0xFFFFFFFF;

		const CRetrieverInstance	&instance = _Instances[pos.InstanceId];
		const CLocalRetriever		&retriever = getRetriever(instance.getRetrieverId());

		if (!retriever.isLoaded() || pos.LocalPosition.Surface < 0 || pos.LocalPosition.Surface >= (sint)retriever.getSurfaces().size())
			return 0xFFFFFFFF;

		return retriever.getSurface(pos.LocalPosition.Surface).getMaterial();
	}

	/// Test if the position is an interior
	bool							isInterior(const UGlobalPosition &pos) const
	{
		if (pos.InstanceId < 0 || pos.InstanceId >= (sint)_Instances.size())
			return false;

		return (_Instances[pos.InstanceId].getType() == CLocalRetriever::Interior);
	}

	/// Test if the position is in water
	bool							isWaterPosition(const UGlobalPosition &pos, float &waterHeight) const
	{
		if (pos.InstanceId < 0 || pos.InstanceId >= (sint)_Instances.size())
			return false;

		const CRetrieverInstance	&instance = _Instances[pos.InstanceId];
		const CLocalRetriever		&retriever = getRetriever(instance.getRetrieverId());

		if (!retriever.isLoaded())
			return false;

		const CRetrievableSurface	&surface = retriever.getSurface(pos.LocalPosition.Surface);

		waterHeight = surface.getWaterHeight();

		return (surface.getFlags() & (1 << CRetrievableSurface::IsUnderWaterBit)) != 0;
	}

	//@}

	/// @name Position retrieving methods.
	//@{

	/// Retrieves the position of an estimated point in the global retriever.
	UGlobalPosition					retrievePosition(const NLMISC::CVector &estimated) const;

	/// Retrieves the position of an estimated point in the global retriever (double instead.)
	UGlobalPosition					retrievePosition(const NLMISC::CVectorD &estimated) const;

	/// Retrieves the position of an estimated point in the global retriever.
	UGlobalPosition					retrievePosition(const NLMISC::CVector &estimated, float threshold) const;

	/// Retrieves the position of an estimated point in the global retriever (double instead.)
	UGlobalPosition					retrievePosition(const NLMISC::CVectorD &estimated, double threshold) const;



	/// Retrieves the position of an estimated point in the global retriever (double instead.)
	UGlobalPosition					retrievePosition(const NLMISC::CVectorD &estimated, double threshold, UGlobalPosition::TType retrieveSpec) const;



	/// Retrieves the position of an estimated point in the global retriever (double instead.)
	UGlobalPosition					retrievePosition(const NLMISC::CVectorD &estimated, uint h, sint &result) const;


	/// Insure position inside surface
	bool							insurePosition(UGlobalPosition &pos) const
	{
		if (pos.InstanceId < 0 || pos.InstanceId >= (sint)_Instances.size())
			return false;

		const CLocalRetriever		&retriever = getRetriever(_Instances[pos.InstanceId].getRetrieverId());
		return retriever.insurePosition(pos.LocalPosition);
	}

	///
	bool							testPosition(UGlobalPosition &pos) const
	{
		if (pos.InstanceId < 0 || pos.InstanceId >= (sint)_Instances.size())
			return false;

		const CRetrieverInstance	&instance = _Instances[pos.InstanceId];

		if (!instance.getBBox().include(pos.LocalPosition.Estimation + instance.getOrigin()))
			return false;

		const CLocalRetriever		&retriever = getRetriever(instance.getRetrieverId());
		return retriever.testPosition(pos.LocalPosition, _InternalCST);
	}

	/// Return the retriever id from the string id
	sint32							getIdentifier(const std::string &id) const;

	/// Get the identifier of the global position.
	const std::string				&getIdentifier(const UGlobalPosition &position) const;

	/// Get the LocalRetrieverId of the global position.
	sint32							getLocalRetrieverId(const UGlobalPosition &position) const;

	/**
	  * Builds a instance of retriever, and link it on the ground (or wherever)
	  * \param id a valid retriever id to be instanciated
	  * \param a valid position where the retriever should be instanciated
	  * \return false if failed
	  */
	bool							buildInstance(const std::string &id, const NLMISC::CVectorD &position, sint32 &instanceId);

	/**
	  * Removes an instance of retriever (perform all unlinks necessary)
	  */
	void							removeInstance(sint32 instanceId);

	/// Snaps to interior ground.
//	void							snapToInteriorGround(UGlobalPosition &position) const;

	/// Converts a global position object into a 'human-readable' CVector.
	NLMISC::CVector					getGlobalPosition(const UGlobalPosition &global) const;

	/// Converts a global position object into a 'human-readable' CVector (double instead.)
	NLMISC::CVectorD				getDoubleGlobalPosition(const UGlobalPosition &global) const;

	/// Make a raytrace test. For the time, always return false.
	bool							testRaytrace (const NLMISC::CVectorD &v0, const NLMISC::CVectorD &v1);

	//@}


	/// @name Mutators
	//@{

	/// Creates an instance of local retriever at the origine position with the given orientation
	const CRetrieverInstance		&makeInstance(uint32 retriever, uint8 orientation, const NLMISC::CVector &origin);

	/// Gets the instance by its id, with full read/write access.
	CRetrieverInstance				&getInstanceFullAccess(uint id) { return _Instances[id]; }

	/// Sets the retriever bank.
	void							setRetrieverBank(const CRetrieverBank *bank) { _RetrieverBank = bank; }


	/// Resets all links within the global retriever.
	void							resetAllLinks();

	/// Inits all the instances inside the global retriever.
	void							initAll(bool initInstances = true);
	/// Links the instance referred by its id to its neighbors.
	void							makeLinks(uint n);
	/// Links all the instances inside the global retriever.
	void							makeAllLinks();

	/// Checks the retriever for errors.
	void							check() const;

	///
	float							distanceToBorder(const UGlobalPosition &pos) const;
	///
	void							getBorders(const UGlobalPosition &pos, std::vector<std::pair<NLMISC::CLine, uint8> > &edges);
	///
	void							getBorders(const NLMISC::CAABBox &sbox, std::vector<std::pair<NLMISC::CLine, uint8> > &edges);

	/// Serialises the global retriever.
	void							serial(NLMISC::IStream &f);

	//@}

	/// \name Dynamic loading part.
	// @{

	void							refreshLrAround(const NLMISC::CVector &position, float radius);

	void							refreshLrAroundNow(const NLMISC::CVector &position, float radius);

	// ensure all load tasks end. called at dtor
	void							waitEndOfAsyncLoading();

	// @}

	/// \name  Collisions part.
	// @{
	/** Test a movement of a cylinder against surface world.
	 * \param start is the start position of the movement.
	 * \param delta is the requested movement.
	 * \param radius is the radius of the vertical cylinder.
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface, ordered by increasing time. this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	*testCylinderMove(const UGlobalPosition &start, const NLMISC::CVector &delta,
		float radius, CCollisionSurfaceTemp &cst) const;
	/** Test a movement of a bbox against surface world.
	 * \param start is the start position of the movement.
	 * \param delta is the requested movement.
	 * \param locI is the oriented I vector of the BBox.  I.norm()== Width/2.
	 * \param locJ is the oriented J vector of the BBox.  J.norm()== Height/2.
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface, ordered by increasing time. this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	*testBBoxMove(const UGlobalPosition &start, const NLMISC::CVector &delta,
		const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const;
	/** apply a movement of a point against surface world. This should be called after test???Move().
	 * NB: It's up to you to give good t, relative to result of test???Move(). Else, undefined results...
	 * NB: if you don't give same start/delta as in preceding call to testMove(), and rebuildChains==false,
	 *	start is returned (nlstop in debug).
	 *
	 * \param start is the start position of the movement. (must be same as passed in test???Move()).
	 * \param delta is the requested movement (must be same as passed in test???Move()).
	 * \param t must be in [0,1]. t*delta is the actual requested movement.
	 * \param cst is the CCollisionSurfaceTemp object used as temp computing (one per thread). (must be same as passed in test???Move()).
	 * \param rebuildChains true if doMove() is not called just after the testMove(). Then CGlobalRetriever must recompute some part
	 *	of the data needed to performing his task.
	 * \return new position of the entity.
	 */
	UGlobalPosition		doMove(const UGlobalPosition &start, const NLMISC::CVector &delta, float t, CCollisionSurfaceTemp &cst, bool rebuildChains=false) const;
	/** retrieve a surface by its Id. NULL if not found or if -1.
	 */
	const CRetrievableSurface	*getSurfaceById(const CSurfaceIdent &surfId) const;
	/** Test a rotation of a BBox against the surfaces.
	 * NB: this function is not perfect because a ContactSurface may appears 2+ times in the returned array.
	 * \param start is the center of the bbox.
	 * \param locI is the new oriented I vector of the BBox.  I.norm()== Width/2.
	 * \param locJ is the new oriented J vector of the BBox.  J.norm()== Height/2.  NB : must have locI^locJ== aK (a>0)
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface (ContactTime and ContactNormal has no means). this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	&testBBoxRot(const CGlobalPosition &start,
		const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const;

	/** return the mean height of the surface under pos..
	 *
	 */
	float				getMeanHeight(const UGlobalPosition &pos) const;

	/// Upadates the height of the given global position
	void				updateHeight(UGlobalPosition &pos) const { pos.LocalPosition.Estimation.z = getMeanHeight(pos); }

	// @}


	/// \name  Pathfinding part.
	// @{

	/// Finds an A* path from a given global position to another.
	void							findAStarPath(const UGlobalPosition &begin, const UGlobalPosition &end, std::vector<CRetrieverInstance::CAStarNodeAccess> &path, uint32 forbidFlags) const;

	/// Finds a path from a given global position to another
	void							findPath(const UGlobalPosition &begin, const UGlobalPosition &end, CGlobalPath &path, uint32 forbidFlags=0) const;

	// @}

private:
	/// \name  Pathfinding part.
	// @{

	/// Gets the CAStarNodeInfo referred by its access.
	CRetrieverInstance::CAStarNodeInfo	&getNode(CRetrieverInstance::CAStarNodeAccess &access) const
	{
		return _Instances[access.InstanceId]._NodesInformation[access.NodeId];
	}

	// @}


	/// \name  Collisions part.
	// @{
	enum	TCollisionType { Circle, BBox };
	/** reset and fill cst.CollisionChains with possible collisions in bboxMove+origin.
	 * result: collisionChains, computed localy to origin.
	 */
	void			findCollisionChains(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector &origin) const;
	/** reset and fill cst.CollisionDescs with effective collisions against current cst.CollisionChains.
	 * result: new collisionDescs in cst.
	 */
	void			testCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, float radius, const CVector2f bbox[4], TCollisionType colType) const;
	/** reset and fill cst.MoveDescs with effective collisions of a point movement against current cst.CollisionChains.
	 * result: the surfaceIdent where we stop. -1 if we traverse a Wall, which should not happen because of collision test.
	 * NB: for precision pb, startCol and deltaCol should be snapped on a grid of 1/1024 meters, using snapVector().
	 * NB: for precision pb (stop on edge etc....), return a "Precision problem ident", ie (-2,-2).
	 * NB: when leaving an interior, return a surface (-3, -3) and restart is set to the real restart position
	 */
	CSurfaceIdent	testMovementWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, UGlobalPosition &restart) const;
	/** reset and fill cst.CollisionDescs with effective collisions against current cst.CollisionChains.
	 * result: new collisionDescs in cst.
	 */
	void			testRotCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, CSurfaceIdent startSurface, const CVector2f bbox[4])  const;
	/// test if a collisionChain separate 2 walls.
	bool			verticalChain(const CCollisionChain &colChain) const;
	/// see CLocalRetriever::getInteriorHeightAround()
	float			getInteriorHeightAround(const UGlobalPosition &position, float outsideTolerance) const;
	// @}

protected:
	friend class CRetrieverInstance;

	CCollisionSurfaceTemp	&getInternalCST() const { return _InternalCST; }
};

}; // NLPACS

#endif // NL_GLOBAL_RETRIEVER_H

/* End of global_retriever.h */
