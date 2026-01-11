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

#ifndef NL_PARTICLE_SYSTEM_SHAPE_H
#define NL_PARTICLE_SYSTEM_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/shape.h"
#include "nel/3d/track.h"

namespace NLMISC
{
	class CContiguousBlockAllocator;
}


namespace NL3D {

///////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING CLASS ARE FOR INSERTION OF A PARTICLE SYSTEM IN A MOT MODEL //
///////////////////////////////////////////////////////////////////////////////

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		ParticleSystemModelId=NLMISC::CClassId(0x3a9b1dc3, 0x49627ff0);


class CParticleSystem;
class CParticleSystemModel;
class CParticleSystemDetailObs;



/** This class helps to instanciate a particle system
 * (the shape contains a particle system prototype stored as a memory stream)
 *  Use the createInstance method to insert the system in a scene
 *  To load the shape from a file, use a shape stream
 */
class CParticleSystemShape : public IShape
{
public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/


	/// Default ctor
	CParticleSystemShape();

	// release memory
	static void releaseInstance();

	/** build the shape from a 'raw' particle system. A prototype will be created by copying the system in a memory stream
	 *  NOTE : For now, prefer the instanciation from a file, which do not need reallocation
	 */
	void buildFromPS(const NL3D::CParticleSystem &ps);

	/// Dtor.
	virtual ~CParticleSystemShape() {}

	/** create a particle system instance
	 * \param scene the scene used to createModel().
	 * \return the specialized instance for this shape.
	 */
	virtual	CTransformShape		*createInstance(NL3D::CScene &scene);

	/// \name Inherited from IShape.
	// @{
	/** render() a particle system in a driver, with the specified TransformShape information.
	 * CTransfromShape call this method in the render traversal.
	 */
	virtual void				render(NL3D::IDriver *drv, CTransformShape *trans, bool passOpaque);
	// @}

	/// serial the shape
	virtual void	serial(NLMISC::IStream &f);
	NLMISC_DECLARE_CLASS(CParticleSystemShape);


	/// get a the user param default tracks
	CTrackDefaultFloat *getUserParamDefaultTrack(uint numTrack)
	{
		nlassert(numTrack < 4);
		return &_UserParamDefaultTrack[numTrack];
	}


	/// get a the trigger default track
	CTrackDefaultBool *getDefaultTriggerTrack(void)
	{

		return &_DefaultTriggerTrack;
	}


	/// Always return a unit bounding box, unless the system has a precomputed bbox.
	virtual	void	getAABBox(NLMISC::CAABBox &bbox) const;


	/** this method is meaningless here : the traverseLoadBalancing() for particle system
	  * compute the number of triangles from the Model, not the shape
	  */
	virtual float				getNumTriangles (float /* distance */) { return 0; }


	/// \name access default tracks.
	// @{
		CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
		CTrackDefaultVector*	getDefaultScale ()		{return &_DefaultScale;}
		CTrackDefaultQuat*		getDefaultRotQuat ()	{return &_DefaultRotQuat;}
	// @}

	// Test if the system is shared
	bool			isShared() const { return _Sharing; }

	// Get the number of cached textures
	uint			getNumCachedTextures() const { return (uint)_CachedTex.size(); }

	// Get a cached texture
	ITexture		*getCachedTexture(uint index) const { return _CachedTex[index]; }

protected:

	friend class CParticleSystemModel;
	friend class CParticleSystem;

public:
	/** Instanciate a particle system from this shape.
	  * A particle system may need to call this when a system is back in the frustum
	  * An contiguous block allocator may be provided for fast alloc, init will be called on such allocator with 0
	  * if num bytes is unknown of with the size needed otherwise.
	  */
	CParticleSystem *instanciatePS(CScene &scene, NLMISC::CContiguousBlockAllocator *blockAllocator = NULL);
public:
	/// inherited from ishape
	virtual void				flushTextures (IDriver &driver, uint selectedTexture);
protected:

	/** A memory stream containing a particle system. Each system is instanciated from this prototype
	  * Nevertheless, we store some more system infos which are needed for its lifecycle mgt.
	  */
	NLMISC::CMemStream  _ParticleSystemProto;
	float				_MaxViewDist;							// the max view distance of the system, mirror the PS value
	NLMISC::CAABBox     _PrecomputedBBox;						// mirror the ps value

	/// the default track for animation of user parameters
	CTrackDefaultFloat _UserParamDefaultTrack[4];

	/// Transform default tracks.
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultVector			_DefaultScale;
	CTrackDefaultQuat			_DefaultRotQuat;

	/// Trigger default track
	CTrackDefaultBool			_DefaultTriggerTrack;

	/// For sharing, this tells us if there's a system already instanciated that we could use for sharing
	NLMISC::CRefPtr<CParticleSystem> _SharedSystem;

	bool                _DestroyWhenOutOfFrustum;				// mirror the ps value
	bool				_DestroyModelWhenOutOfRange;			// mirror the ps value
	bool				_UsePrecomputedBBox;					// mirror the ps value
	bool				_Sharing;								// mirror the ps value

	// keep smart pointer on textures for caching, so that when flushTextures is called, subsequent
	std::vector<NLMISC::CSmartPtr<ITexture> > _CachedTex;

	// The amount of memory needed for instanciation or 0 if not known.
	// If the amount is known, a big block can be allocated for fast contiguous allocations
	// Given that a .ps can allocate numerous small block, this can be slow indeed..
	uint				_NumBytesWanted;

public:
	#ifdef PS_FAST_ALLOC
		// for fast allocation of ps resources. Used only if the system is shared
		// In this case, only one CParticleSystem instance is created, even if there are several models,
		// and because the allocator must remains until the instance is released, we must keep it in the shape
		//
		NLMISC::CContiguousBlockAllocator		Allocator;
	#endif
	// Order in which the element of the particle system must be processed during the sim loop.
	std::vector<uint>	_ProcessOrder;
};

} // NL3D


#endif // NL_PARTICLE_SYSTEM_SHAPE_H

/* End of particle_system_shape.h */
