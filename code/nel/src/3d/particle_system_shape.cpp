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

#include "std3d.h"

#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/ps_allocator.h"
#include "nel/misc/file.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/contiguous_block_allocator.h"

// tmp
#include "nel/3d/ps_face_look_at.h"
#include "nel/3d/ps_force.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


using NLMISC::CIFile;


namespace {

NLMISC::CMutex s_PSSMutex;

} /* anonymous namespace */

// private usage : macro to check the memory integrity
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	// #define PARTICLES_CHECK_MEM NLMEMORY::CheckHeap(true);
	#define PARTICLES_CHECK_MEM
#else
	#define PARTICLES_CHECK_MEM
#endif

// ***************************************************************************
// A singleton to define the TextureCategory of Particle system
class CPSTextureCategory
{
public:
	static NLMISC::CSmartPtr<ITexture::CTextureCategory>	&get()
	{
		if(!_Instance)
			_Instance= new CPSTextureCategory();
		return _Instance->_TextureCategory;
	}
	// release memory
	static void releaseInstance()
	{
		if( _Instance )
			delete _Instance;
		_Instance = NULL;
	}

private:
	NLMISC::CSmartPtr<ITexture::CTextureCategory>	_TextureCategory;
	CPSTextureCategory()
	{
		_TextureCategory= new ITexture::CTextureCategory("PARTICLE SYSTEM");
	}
	static CPSTextureCategory	*_Instance;
};
CPSTextureCategory	*CPSTextureCategory::_Instance= NULL;

///===========================================================================
void CParticleSystemShape::releaseInstance()
{
	CPSTextureCategory::releaseInstance();
}

///===========================================================================
CParticleSystemShape::CParticleSystemShape() : _MaxViewDist(100.f),
											   _DestroyWhenOutOfFrustum(false),
											   _DestroyModelWhenOutOfRange(false),
											   _UsePrecomputedBBox(false),
											   _Sharing(false),
											   _NumBytesWanted(0)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	for (uint k = 0; k < 4; ++k)
	{
		_UserParamDefaultTrack[k].setDefaultValue(0);
	}
	_DefaultPos.setDefaultValue(CVector::Null);
	_DefaultScale.setDefaultValue( CVector(1, 1, 1) );
	_DefaultRotQuat.setDefaultValue(CQuat());
	_DefaultTriggerTrack.setDefaultValue(true); // by default, system start as soon as they are instanciated

}

///===========================================================================
void	CParticleSystemShape::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	sint ver = f.serialVersion(6);
	/// version 6 : added sharing flag
	//NLMISC::CVector8 &buf = _ParticleSystemProto.bufferAsVector();
	//f.serialCont(buf);

	if (f.isReading ())
	{
		std::vector<uint8> buf;
		f.serialCont(buf);
		_ParticleSystemProto.fill(&buf[0], (uint32)buf.size());
	}
	else
	{
		f.serialBufferWithSize ((uint8*)_ParticleSystemProto.buffer(), _ParticleSystemProto.length());
	}

	if (ver > 1)
	{
		// serial default tracks
		for (uint k = 0; k < 4; ++k)
		{
			f.serial(_UserParamDefaultTrack[k]);
		}
	}
	if ( ver > 2)
	{
		f.serial (_DefaultPos);
		f.serial (_DefaultScale);
		f.serial (_DefaultRotQuat);
	}
	if ( ver > 3)
	{
		f.serial(_MaxViewDist);
		f.serial(_DestroyWhenOutOfFrustum);
		f.serial(_DestroyModelWhenOutOfRange);
	}
	if ( ver > 4)
	{
		f.serial(_UsePrecomputedBBox);
		if (_UsePrecomputedBBox)
		{
			f.serial(_PrecomputedBBox);
		}
	}
	if ( ver > 5)
	{
		f.serial(_Sharing);
	}
}

///===========================================================================
void CParticleSystemShape::buildFromPS(const CParticleSystem &ps)
{
	// must be sure that we are writing in the stream
	if (_ParticleSystemProto.isReading())
	{
		_ParticleSystemProto.invert();
	}

	// to have const correctness in the prototype, we must do this...
	CParticleSystem *myPs = const_cast<CParticleSystem *>(&ps);
	nlassert(myPs);
	// build the prototype
	_ParticleSystemProto.serialPtr(myPs);

	// mirror some system values
	_MaxViewDist = myPs->getMaxViewDist();
	_DestroyWhenOutOfFrustum = myPs->doesDestroyWhenOutOfFrustum();
	_DestroyModelWhenOutOfRange = myPs->getDestroyModelWhenOutOfRange();
	if (!myPs->getAutoComputeBBox())
	{
		_UsePrecomputedBBox = true;
		myPs->computeBBox(_PrecomputedBBox);
	}
	else
	{
		_UsePrecomputedBBox = false;
	}
	_Sharing = myPs->isSharingEnabled();
}

///===========================================================================
void	CParticleSystemShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	if (!_UsePrecomputedBBox)
	{
		bbox.setCenter(NLMISC::CVector::Null);
		bbox.setHalfSize(NLMISC::CVector(1, 1, 1));
	}
	else
	{
		bbox = _PrecomputedBBox;
	}
}


///===========================================================================
CParticleSystem *CParticleSystemShape::instanciatePS(CScene &scene, NLMISC::CContiguousBlockAllocator *blockAllocator /*= NULL*/)
{
	if (_Sharing && _SharedSystem != NULL) // is sharing enabled, and is a system already instanciated
	{
		return _SharedSystem;
	}

	// avoid prb with concurrent thread (may happen if an instance group containing ps is loaded in background)
	s_PSSMutex.enter();


	#ifdef PS_FAST_ALLOC
		nlassert(PSBlockAllocator == NULL);
		if (blockAllocator)
		{
			// set new allocator for particle system memory
			PSBlockAllocator = blockAllocator;
			blockAllocator->init(_NumBytesWanted); // if size wanted is already known, set it
		}
	#endif

	//NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
	// copy the datas
	CParticleSystem *myInstance = NULL;

	// serialize from the memory stream
	if (!_ParticleSystemProto.isReading()) // we must be sure that we are reading the stream
	{
		_ParticleSystemProto.invert();
	}

	_ParticleSystemProto.resetPtrTable();
	_ParticleSystemProto.seek(0, NLMISC::IStream::begin);

//	NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
	_ParticleSystemProto.serialPtr(myInstance); // instanciate the system
/*	NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
	nlinfo("instanciation time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));
*/

	myInstance->setScene(&scene);

	if (_CachedTex.empty() && scene.getDriver())
	{
		//nlinfo("flushing texs");
		// load && cache textures
		myInstance->enumTexs(_CachedTex, *scene.getDriver());
		for(uint k = 0; k < _CachedTex.size(); ++k)
		{
			if (_CachedTex[k])
			{
				_CachedTex[k]->setTextureCategory(CPSTextureCategory::get());
				scene.getDriver()->setupTexture (*(ITexture *)_CachedTex[k]);
			}
		}
	}
	else
	{
		/*
		for(uint k = 0; k < _CachedTex.size(); ++k)
		{
			nlinfo(_CachedTex[k]->getShareName().c_str());
		}
		*/
	}

	// tmp

	if (_Sharing)
	{
		_SharedSystem = myInstance; // set this as the first shared instance
	}

	#ifdef PS_FAST_ALLOC
		if (blockAllocator)
		{
			_NumBytesWanted = blockAllocator->getNumAllocatedBytes(); // now we know the number of wanted bytes, subsequent alloc can be much faster
			PSBlockAllocator = NULL;
		}
	#endif

	s_PSSMutex.leave();

	/*NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
	nlinfo("instanciation time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));	*/
	return myInstance;
}

///===========================================================================
CTransformShape		*CParticleSystemShape::createInstance(CScene &scene)
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(scene.createModel(NL3D::ParticleSystemModelId) );
	psm->Shape = this;
	psm->_Scene = &scene; // the model needs the scene to recreate the particle system he holds
	// by default, we don't instanciate the system. It will be instanciated only if visible and triggered
	// psm->_ParticleSystem = instanciatePS(scene);

	// Setup position with the default value
	psm->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	psm->ITransformable::setRotQuat( _DefaultRotQuat.getDefaultValue() );
	psm->ITransformable::setScale( _DefaultScale.getDefaultValue() );

	// ParticleSystems are added to the "Fx" Load Balancing Group.
	psm->setLoadBalancingGroup("Fx");

	return psm;
}

///===========================================================================
void	CParticleSystemShape::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	H_AUTO ( NL3D_Particles_Render );
	nlassert(drv);
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(trans);
	if (psm->_Invalidated) return;
	CParticleSystem *ps = psm->getPS();
	/// has the system been triggered yet ?
	if (!ps) return;

	TAnimationTime delay = psm->getEllapsedTime();
	nlassert(ps->getScene());

	///////////////////////
	// render particles  //
	///////////////////////

	/// if sharing is enabled,  we should resetup the system matrix
	if (ps->isSharingEnabled())
	{
		ps->setSysMat(&(psm->getWorldMatrix()));
		ps->setUserMatrix(&(psm->getUserMatrix()));
	}

	// Setup the matrix.
	/// drv->setupModelMatrix(trans->getWorldMatrix());

	ps->setDriver(drv);
	// draw particle
	PARTICLES_CHECK_MEM;
	if (passOpaque)
	{
		PSLookAtRenderTime = 0;
		//NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
		ps->step(CParticleSystem::SolidRender, delay, *this, *psm);
		/*NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
		nlinfo("Solid render time time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));
		nlinfo("LookAt Render time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(PSLookAtRenderTime)));	*/
	}
	else
	{
		//PSLookAtRenderTime = 0;
		//NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
		ps->step(CParticleSystem::BlendRender, delay, *this, *psm);
		/*NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
		nlinfo("Blend render time time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));
		nlinfo("LookAt Render time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(PSLookAtRenderTime)));	*/
	}
	PARTICLES_CHECK_MEM;
	if (psm->isToolDisplayEnabled())
	{
		ps->step(CParticleSystem::ToolRender, delay, *this, *psm);
		PARTICLES_CHECK_MEM;
	}
}

///===========================================================================
void CParticleSystemShape::flushTextures(IDriver &driver, uint selectedTexture)
{
	// if textures are already flushed, no-op
	if (!_CachedTex.empty()) return;
	if (_SharedSystem)
	{
		_SharedSystem->enumTexs(_CachedTex, driver);
	}
	else
	{
		s_PSSMutex.enter();

		// must create an instance just to flush the textures
		CParticleSystem *myInstance = NULL;

		#ifdef PS_FAST_ALLOC
			nlassert(PSBlockAllocator == NULL);
			NLMISC::CContiguousBlockAllocator blockAllocator;
			PSBlockAllocator = &blockAllocator;
			blockAllocator.init(300000); // we release memory just after, and we don't want to fragment the memory, so provide large enough mem
		#endif
		// serialize from the memory stream
		if (!_ParticleSystemProto.isReading()) // we must be sure that we are reading the stream
		{
			_ParticleSystemProto.invert();
		}
		_ParticleSystemProto.resetPtrTable();
		_ParticleSystemProto.seek(0, NLMISC::IStream::begin);
		_ParticleSystemProto.serialPtr(myInstance); // instanciate the system
		#ifdef PS_FAST_ALLOC
			_NumBytesWanted = blockAllocator.getNumAllocatedBytes(); // next allocation will be fast because we know how much memory to allocate
		#endif
		myInstance->enumTexs(_CachedTex, driver);
		// tmp
		/*
		#ifdef NL_DEBUG
			for(uint k = 0; k < myInstance->getNbProcess(); ++k)
			{
				CPSLocated *loc = (CPSLocated *) myInstance->getProcess(k);
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (dynamic_cast<CPSCentralGravity *>(loc->getBoundObject(l)))
					{
						nlwarning("PS %s uses central gravity", myInstance->getName().c_str());
						break;
					}
				}
			}
		#endif */
		// sort the process inside the fx
		myInstance->getSortingByEmitterPrecedence(_ProcessOrder);
		delete myInstance;
		#ifdef PS_FAST_ALLOC
			PSBlockAllocator = NULL;
		#endif
		s_PSSMutex.leave();
	}
	for(uint k = 0; k < _CachedTex.size(); ++k)
	{
		//nlinfo(_CachedTex[k]->getShareName().c_str());
		if (_CachedTex[k])
		{
			_CachedTex[k]->setTextureCategory(CPSTextureCategory::get());
			driver.setupTexture(*_CachedTex[k]);
		}
	}
}


} // NL3D
