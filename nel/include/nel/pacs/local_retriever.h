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

#ifndef NL_LOCAL_RETRIEVER_H
#define NL_LOCAL_RETRIEVER_H

#include <vector>
#include <string>
#include <list>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"
#include "nel/misc/polygon.h"

#include "vector_2s.h"
#include "surface_quad.h"
#include "chain.h"
#include "retrievable_surface.h"
#include "chain_quad.h"
#include "exterior_mesh.h"
#include "face_grid.h"

#include "nel/pacs/u_global_position.h"



namespace NLPACS
{

class CCollisionSurfaceTemp;

/**
 * A surface retriever, located by its bounding box.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CLocalRetriever
{
public:
	/**
	 * A tip of several chains. A CTip can contain more than 2 chains.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTip
	{
	public:
		/**
		 * A chain tip. Contains the id of the chain and whether this tip is the start of the chain.
		 * \author Benjamin Legros
		 * \author Nevrax France
		 * \date 2001
		 */
		struct CChainTip
		{
			/// The id of the chain.
			sint32	Chain;

			/// True if this tip is the beginning of the chain.
			bool	Start;

			/// Constructor.
			CChainTip(sint32 chainId = 0, bool start = 0) : Chain(chainId), Start(start) {}

			/// Serielaises the CChainTip
			void	serial(NLMISC::IStream &f) { f.serial(Chain, Start); }
		};

		CTip() : Point(NLMISC::CVector::Null) {}

		/// The position of the tip.x
		NLMISC::CVector					Point;

		// The chains linked to that tip.
		std::vector<CChainTip>			Chains;

	public:
		/// Serialises the CTip.
		void	serial(NLMISC::IStream &f)
		{
			f.serial(Point);
			f.serialCont(Chains);
		}

		/// Translates the CTip by the translation vector.
		void	translate(const NLMISC::CVector &translation)
		{
			Point += translation;
		}
	};


	/**
	 * A topology. It's a set of surfaces which are definning a connex master surface.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTopology : public std::vector<sint32>
	{
	public:
		void	serial(NLMISC::IStream &f)	{ f.serialCont(*this); }
	};


	/**
	 * An estimation of the position of a point on a specified surface.
	 * The estimated position is LOCAL reference to the retriever axis.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CLocalPosition : public ULocalPosition
	{
	public:
		/// Constructor.
		CLocalPosition(sint32 surface=-1, const NLMISC::CVector &estimation=NLMISC::CVector::Null)
		{
			Surface=surface;
			Estimation=estimation;
		}

		/// Serialises the CLocalPosition.
		//void							serial(NLMISC::IStream &f) { f.serial(Surface, Estimation); }
	};

	/**
	 * The different types of retriever (landscape or interior.)
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	enum EType
	{
		Landscape = 0,
		Interior
	};


	/**
	 * The faces used for snapping in interior retrievers.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CInteriorFace
	{
	public:
		uint32	Verts[3];
		uint32	Surface;
	public:
		void	serial(NLMISC::IStream &f) { f.serial(Verts[0], Verts[1], Verts[2], Surface); }
	};



	/**
	 * An iterator to go through chains without bothering about ordered chains and those kind of f*cking stuffs
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2003
	 */
	class CIterator
	{
		friend class CLocalRetriever;

	protected:
		/// The referent retriever
		const CLocalRetriever	*_Retriever;

		/// Chain Id
		uint16					_Chain;

		/// The current ochain in chain
		uint16					_OChainIndex;

		/// The current vertex in ochain
		sint16					_IndexInOChain;

		/// Max index in ochain
		sint16					_MaxIndexInOChain;

		/// Is ochain forward?
		bool					_OChainForward;

	public:
		/// Constructor
		CIterator(const CLocalRetriever *retriever, uint chain)
		{
			_Retriever = retriever;
			_Chain = uint16(chain);
			_OChainIndex = 0;
			_IndexInOChain = 0;
			_MaxIndexInOChain = 0;
			_OChainForward = 0;
			setupIndex();
		}

		CIterator	&operator ++ ()
		{
			const CChain	&chain = _Retriever->getChain(_Chain);
			if (_OChainIndex >= chain.getSubChains().size())
				return *this;

			if (_OChainForward)
			{
				++_IndexInOChain;
				if (((sint)_OChainIndex < (sint)chain.getSubChains().size()-1 && _IndexInOChain == _MaxIndexInOChain) || _IndexInOChain > _MaxIndexInOChain)
				{
					++_OChainIndex;
					setupIndex();
				}
			}
			else
			{
				--_IndexInOChain;
				if (((sint)_OChainIndex < (sint)chain.getSubChains().size()-1 && _IndexInOChain == 0) || _IndexInOChain < 0)
				{
					++_OChainIndex;
					setupIndex();
				}
			}
			return *this;
		}

		CVector2s	operator * () const
		{
			if (end())
				return CVector2s();
			return _Retriever->getOrderedChain(_Retriever->getChain(_Chain).getSubChain(_OChainIndex))[_IndexInOChain];
		}

		CVector		get3d() const
		{
			if (end())
				return CVector::Null;
			return _Retriever->getFullOrderedChain(_Retriever->getChain(_Chain).getSubChain(_OChainIndex))[_IndexInOChain];
		}

		/// Test end
		bool		end() const
		{
			return (_OChainIndex >= _Retriever->getChain(_Chain).getSubChains().size());
		}

	private:
		/// Setup index in ochain
		void	setupIndex()
		{
			const CChain			&chain = _Retriever->getChain(_Chain);
			if (_OChainIndex >= chain.getSubChains().size())
				return;
			const COrderedChain		&ochain = _Retriever->getOrderedChain(chain.getSubChain(_OChainIndex));
			_OChainForward = ochain.isForward();
			_MaxIndexInOChain = sint16(ochain.getVertices().size()-1);
			_IndexInOChain = (_OChainForward ? 0 : _MaxIndexInOChain);
		}
	};

protected:
	friend class	CRetrieverInstance;

	/// The chains insinde the zone.
	std::vector<COrderedChain>			_OrderedChains;
	std::vector<COrderedChain3f>		_FullOrderedChains;

	/// The chains insinde the zone.
	std::vector<CChain>					_Chains;

	/// The surfaces inside the zone.
	std::vector<CRetrievableSurface>	_Surfaces;

	/// The bbox of the local retriever
	NLMISC::CAABBox						_BBox;

	/// The tips making links between different chains.
	std::vector<CTip>					__Tips;

	/// The chains on the edges of the zone.
	std::vector<uint16>					_BorderChains;

	/// The topologies within the zone.
	std::vector<CTopology>				_Topologies[NumMaxCreatureModels];

	/// The tip recognition threshold
	static const float					_TipThreshold;

	/// The tip recognition threshold
	static const float					_EdgeTipThreshold;

	/// For collisions, the chainquad.
	CChainQuad							_ChainQuad;

	/// The type of the retriever
	EType								_Type;

	/// \name Internior retriever specific
	// @{
	/// The exterior mesh, for collisions
	CExteriorMesh						_ExteriorMesh;

	/// The vertices of the collision mesh
	std::vector<NLMISC::CVector>		_InteriorVertices;

	/// The faces of the collision mesh
	std::vector<CInteriorFace>			_InteriorFaces;

	/// The face selection grid
	CFaceGrid							_FaceGrid;

	/// An human readable identifier of the retriever.
	std::string							_Id;

	// @}

	/// Tells if retriever is loaded
	bool								_Loaded;

public:
	///
	bool								LoadCheckFlag;

private:
	/// The intersection between an ordered chain and the path.
	struct CIntersectionMarker
	{
		float	Position;
		uint16	OChain;
		uint16	Edge;
		bool	In;

		CIntersectionMarker() {}
		CIntersectionMarker(float position, uint16 ochain, uint16 edge, bool in) : Position(position), OChain(ochain), Edge(edge), In(in) {}

		bool	operator< (const CIntersectionMarker &comp) const { return Position < comp.Position; }
	};

public:
	/// @name Constructors
	// @{

	CLocalRetriever();

	// @}

	/// @name Selectors
	// @{

	/// Returns the chain tips inside the local retrievers.
	//const std::vector<CTip>				&getTips() const { return _Tips; }
	/// Returns the nth tip in the retriever.
	//const CTip							&getTip(uint n) const { return _Tips[n]; }

	/// Returns the ordered chains.
	const std::vector<COrderedChain>	&getOrderedChains() const { return _OrderedChains; }
	/// Returns the nth ordered chain.
	const COrderedChain					&getOrderedChain(uint n) const { return _OrderedChains[n]; }

	/// Returns the full ordered chains.
	const std::vector<COrderedChain3f>	&getFullOrderedChains() const { return _FullOrderedChains; }
	/// Returns the nth full ordered chain.
	const COrderedChain3f				&getFullOrderedChain(uint n) const { return _FullOrderedChains[n]; }

	/// Returns the chains.
	const std::vector<CChain>			&getChains() const { return _Chains; }
	/// retruns the nth chain.
	const CChain						&getChain(uint n) const { return _Chains[n]; }

	/// Returns the ids of the chains on the edges of the retriever.
	const std::vector<uint16>			&getBorderChains() const { return _BorderChains; }
	/// Returns the id of the nth chain on the edges of the retriever.
	uint16								getBorderChain(uint n) const { return _BorderChains[n]; }

	/// Returns the surfaces.
	const std::vector<CRetrievableSurface>	&getSurfaces() const { return _Surfaces; }
	/// Returns the nth surface.
	const CRetrievableSurface			&getSurface(uint n) const { return _Surfaces[n]; }

	/// Returns the type of the retriever
	EType								getType() const { return _Type; }

	/// Returns the bbox
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }

	/// Returns the exterior mesh of the retriever
	const CExteriorMesh					&getExteriorMesh() const { return _ExteriorMesh; }

	/// Returns the interior vertices
	const std::vector<NLMISC::CVector>	&getInteriorVertices() const { return _InteriorVertices; }

	/// Returns the interior faces
	const std::vector<CInteriorFace>	&getInteriorFaces() const { return _InteriorFaces; }

	/// Returns the identifier of the retriever.
	const std::string					&getIdentifier() const { return _Id; }

	/// Is loaded ?
	bool								isLoaded() const { return _Loaded; }

	/// Force Loaded State.
	void								forceLoaded(bool state) { _Loaded= state; }

	/// build BBoxes of interior surfaces in surfaceBBoxes (cleared first)
	void								buildInteriorSurfaceBBoxes(std::vector<NLMISC::CAABBox>	&surfaceBBoxes) const;

	/// Builds the polygons (loops) of a given surface
	void								buildSurfacePolygons(uint32 surface, std::list<NLMISC::CPolygon> &polygons) const;

	/// Builds the polygons (loops) of all surfaces in the retriever
	void								buildSurfacePolygons(std::list< std::list<NLMISC::CPolygon> > &polygons) const
	{
		uint	i;
		for (i=0; i<_Surfaces.size(); ++i)
		{
			polygons.push_back(std::list<NLMISC::CPolygon>());
			buildSurfacePolygons(i, polygons.back());
		}
	}

	/// Builds the polygons (loops) of a given surface
	void								build3dSurfacePolygons(uint32 surface, std::list<NLMISC::CPolygon> &polygons) const;

	/// Builds the polygons (loops) of all surfaces in the retriever
	void								build3dSurfacePolygons(std::list< std::list<NLMISC::CPolygon> > &polygons) const
	{
		uint	i;
		for (i=0; i<_Surfaces.size(); ++i)
		{
			polygons.push_back(std::list<NLMISC::CPolygon>());
			build3dSurfacePolygons(i, polygons.back());
		}
	}
	// @}


	/// @name Mutators
	//@{

	/// Adds a surface to the local retriever, using its features. Returns the id of the newly created surface.
	sint32								addSurface(uint8 normalq, uint8 orientationq,
												   uint8 mat, uint8 charact, uint8 level,
												   bool isUnderWater, float waterHeight,
												   bool clusterHint,
												   const NLMISC::CVector &center,
												   const CSurfaceQuadTree &quad,
												   sint8 quantHeight = 0);

	/**
	 * Adds a chain to the local retriever, using the vertices of the chain,
	 * the left and right surfaces id and the edge on which the chain is stuck
	 */
	sint32								addChain(const std::vector<NLMISC::CVector> &vertices,
												 sint32 left, sint32 right);

	/// Set the type of the retriever (see EType)
	void								setType(EType type) { _Type = type; }

	/// Sets the exterior mesh
	void								setExteriorMesh(const CExteriorMesh &em) { _ExteriorMesh = em; }

	/// Sets the bbox of the retriever
	void								setBBox(const NLMISC::CAABBox &bbox) { _BBox = bbox; }


	/// Returns the interior vertices
	std::vector<NLMISC::CVector>		&getInteriorVertices() { return _InteriorVertices; }

	/// Returns the interior faces
	std::vector<CInteriorFace>			&getInteriorFaces() { return _InteriorFaces; }

	/// Returns the identifier of the retriever.
	void								setIdentifier(const std::string &id) { _Id = id; }


	/// Returns the chain quad
	CChainQuad							&getChainQuad() { return _ChainQuad; }


	/// Inits the face grid
	void								initFaceGrid();


	/// Builds topologies tables.
	void								computeTopologies();

	/// Builds tips
	void								computeLoopsAndTips();

	/// Found chains on the edges of the retriever and fills _BorderChains tables.
	void								findBorderChains();

	/// Updates surfaces links from the links contained in the chains...
	void								updateChainIds();


	/// Sorts chains references inside the tips. NOT IMPLEMENTED YET.
	void								sortTips();


	/// Translates the local retriever by the translation vector.
	void								translate(const NLMISC::CVector &translation);


	///
	void								flushFullOrderedChains() { _FullOrderedChains.clear(); }

	///
	void								setFullOrderedChains(const std::vector<COrderedChain3f> &foc)	{ _FullOrderedChains = foc; }


	/// Serialises the CLocalRetriever.
	void								serial(NLMISC::IStream &f);

	/// Clear
	void								clear();

	// @}

	/// \name  Collisions part.
	// @{
	/// compute the chain quad, used for collisions. the ChainQuad is serialised in serial(). _OrderedChains must be OK.
	void			computeCollisionChainQuad();
	/** add possible collisions chains to the temp result.
	 * \param cst the temp result to store collision chains. they are appened to cst.CollisionChains.
	 * \param bboxMove the bbox which bounds the movement of the entity.
	 * \param transBase the vector we use to translate local position of edge.
	 */
	void			testCollision(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector2f &transBase) const;

	/**
	 * Check all surfaces integrity
	 */
	bool								checkSurfacesIntegrity(NLMISC::CVector translation = NLMISC::CVector::Null, bool verbose = false) const;

	/**
	 * Check surface integrity
	 */
	bool								checkSurfaceIntegrity(uint surf, NLMISC::CVector translation = NLMISC::CVector::Null, bool verbose = false) const;

	// @}

/*
protected:
*/

	/// Assures a position is really inside a surface (and independent from any accuracy issue) and returns true if the position was moved.
	bool								insurePosition(ULocalPosition &local) const;

	/// Retrieves a position inside the retriever (from the local position), returns true if the position is close to a border
	void								retrievePosition(NLMISC::CVector estimated, CCollisionSurfaceTemp &cst) const;

	/// Retrieves a position inside the retriever (from the local position), returns true if the position is close to a border
	void								retrieveAccuratePosition(CVector2s estimated, CCollisionSurfaceTemp &cst, bool &onBorder) const;

	/// Retrieves a position inside the retriever (from the local position), returns true if the position is close to a border
	bool								testPosition(ULocalPosition &local, CCollisionSurfaceTemp &cst) const;

	/// Snaps on the ground
	void								snapToInteriorGround(ULocalPosition &position, bool &snapped) const;

	/// Get the height of surface that matches the best this position
	float								getHeight(const ULocalPosition &position) const;

	/// Finds a path in a given surface, from the point A to the point B.
	void								findPath(const CLocalPosition &A, const CLocalPosition &B, std::vector<CVector2s> &path, NLPACS::CCollisionSurfaceTemp &cst) const;

	///
	void								unify();

	/** Get the height of interior surface that matches the best this position. Allows the position to be a bit outside the triangles.
	 *	return 0 if not loaded or not an interior surface.
	 *	\param outsideTolerance how much are we allowed to look outside the triangles
	 */
	float								getInteriorHeightAround(const ULocalPosition &position, float outsideTolerance) const;


public:

	// Last minute fix... Some issues appeared when linking (trying to link 2 chains with only 1 in the neighbourhood)

	class CChainReplacement
	{
	public:
		uint							Chain;
		sint							Left, Right;
		std::vector<NLMISC::CVector>	Vertices;
	};

	// temp object to store replaced ochain ids
	std::vector<uint>					FreeOChains;

	void	replaceChain(uint32 chainId, const std::vector<CChainReplacement> &replacement);

	void	forceBorderChainId(uint32 chainId, uint32 borderId)
	{
		if (chainId >= _Chains.size())
		{
			nlwarning("forceBorderChainId(): couldn't force border id %d for chain %d, doesn't exist", borderId, chainId);
			return;
		}

		_Chains[chainId].setBorderChainIndex(borderId);
	}

public:
	const NLMISC::CVector				&getStartVector(uint32 chain) const;
	const NLMISC::CVector				&getStopVector(uint32 chain) const;

	const NLMISC::CVector				&getStartVector(uint32 chain, sint32 surface) const;
	const NLMISC::CVector				&getStopVector(uint32 chain, sint32 surface) const;
/*
	uint16								getStartTip(uint32 chain, sint32 surface) const;
	uint16								getStopTip(uint32 chain, sint32 surface) const;

	void								setStartTip(uint32 chain, sint32 surface, uint16 startTip);
	void								setStopTip(uint32 chain, sint32 surface, uint16 stopTip);
*/
	uint32								getPreviousChain(uint32 chain, sint32 surface) const;
	uint32								getNextChain(uint32 chain, sint32 surface) const;

public:
	void								dumpSurface(uint surf, const NLMISC::CVector &vect = NLMISC::CVector::Null) const;

	float								distanceToBorder(const ULocalPosition &pos) const;
};

}; // NLPACS

#endif // NL_LOCAL_RETRIEVER_H

/* End of local_retriever.h */
