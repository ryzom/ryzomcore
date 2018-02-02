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

#include "nel/3d/ps_face.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"
#include "nel/3d/ps_iterator.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/quat.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

using NLMISC::CQuat;

////////////////////////////
// CPSFace implementation //
////////////////////////////

/** Well, we could have put a method template in CPSFace, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSFaceHelper
{
public:
	template <class T, class U>
	static void drawFaces(T posIt, U indexIt, CPSFace &f, uint size, uint32 srcStep)
	{
		NL_PS_FUNC(CPSFaceHelper_drawFaces)
		PARTICLES_CHECK_MEM;
		nlassert(f._Owner);
		IDriver *driver = f.getDriver();

		CVertexBuffer &vb = f.getNeededVB(*driver);
		f.updateMatBeforeRendering(driver, vb);

		uint8 *currVertex;

		// number of left faces to draw, number of faces to process at once
		uint32 leftFaces = size, toProcess;
		f._Owner->incrementNbDrawnParticles(size); // for benchmark purpose
		f.setupDriverModelMatrix();
		float sizeBuf[CPSQuad::quadBufSize];
		float *ptSize;
		T endPosIt;

		// if constant size is used, the pointer points always the same float
		uint32 ptSizeIncrement = f._SizeScheme ? 1 : 0;

		if (f._ColorScheme)
		{
			f._ColorScheme->setColorType(driver->getVertexColorFormat());
		}

		if (f._PrecompBasis.size()) // do we use precomputed basis ?
		{
			do
			{
				{
					toProcess = leftFaces > (uint32) CPSQuad::quadBufSize ? (uint32) CPSQuad::quadBufSize : leftFaces;
					vb.setNumVertices(4 * toProcess);
					CVertexBufferReadWrite vba;
					vb.lock (vba);
					currVertex = (uint8 *) vba.getVertexCoordPointer() ;
					if (f._SizeScheme)
					{
						ptSize = (float *) (f._SizeScheme->make(f._Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true, srcStep));
					}
					else
					{
						ptSize = &f._ParticleSize;
					}
					f.updateVbColNUVForRender(vb, size - leftFaces, toProcess, srcStep, *driver);
					const uint32 stride = vb.getVertexSize();
					endPosIt = posIt + toProcess;
					do
					{
						const CPlaneBasis &currBasis = f._PrecompBasis[*indexIt].Basis;
						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis.X.x;
						((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis.X.y;
						((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis.X.z;
						currVertex += stride;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis.Y.x;
						((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis.Y.y;
						((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis.Y.z;
						currVertex += stride;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis.X.x;
						((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis.X.y;
						((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis.X.z;
						currVertex += stride;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis.Y.x;
						((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis.Y.y;
						((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis.Y.z;
						currVertex += stride;
						ptSize += ptSizeIncrement;
						++indexIt;
						++posIt;
					}
					while (posIt != endPosIt);
				}
				driver->activeVertexBuffer(vb),
				driver->renderRawQuads(f._Mat, 0, toProcess);
				leftFaces -= toProcess;
			}
			while (leftFaces);
		}
		else
		{
			// must compute each particle basis at each time
			static CPlaneBasis planeBasis[CPSQuad::quadBufSize]; // buffer to compute each particle basis
			CPlaneBasis *currBasis;
			uint32    ptPlaneBasisIncrement = f._PlaneBasisScheme ? 1 : 0;
			const uint32 vSize = vb.getVertexSize();
			do
			{
				{
					toProcess = leftFaces > (uint32) CPSQuad::quadBufSize ? (uint32) CPSQuad::quadBufSize : leftFaces;
					vb.setNumVertices(4 * toProcess);
					CVertexBufferReadWrite vba;
					vb.lock (vba);
					currVertex = (uint8 *) vba.getVertexCoordPointer() ;
					if (f._SizeScheme)
					{
						ptSize  = (float *) (f._SizeScheme->make(f._Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true, srcStep));
					}
					else
					{
						ptSize = &f._ParticleSize;
					}

					if (f._PlaneBasisScheme)
					{
						currBasis = (CPlaneBasis *) (f._PlaneBasisScheme->make(f._Owner, size - leftFaces, planeBasis, sizeof(CPlaneBasis), toProcess, true, srcStep));
					}
					else
					{
						currBasis = &f._PlaneBasis;
					}
					f.updateVbColNUVForRender(vb, size - leftFaces, toProcess, srcStep, *driver);
					endPosIt = posIt + toProcess;
					do
					{
						// we use this instead of the + operator, because we avoid 4 constructor calls this way
						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis->X.x;
						((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis->X.y;
						((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis->X.z;
						currVertex += vSize;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis->Y.x;
						((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis->Y.y;
						((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis->Y.z;
						currVertex += vSize;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis->X.x;
						((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis->X.y;
						((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis->X.z;
						currVertex += vSize;

						CHECK_VERTEX_BUFFER(vb, currVertex);
						((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis->Y.x;
						((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis->Y.y;
						((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis->Y.z;
						currVertex += vSize;
						ptSize += ptSizeIncrement;
						++posIt;
						currBasis += ptPlaneBasisIncrement;
					}
					while (posIt != endPosIt);
				}
				driver->activeVertexBuffer(vb);
				driver->renderRawQuads(f._Mat, 0, toProcess);
				leftFaces -= toProcess;
			}
			while (leftFaces);
		}
		PARTICLES_CHECK_MEM;
	}
};

///======================================================================================
CPSFace::CPSFace(CSmartPtr<ITexture> tex) : CPSQuad(tex)
{
	NL_PS_FUNC(CPSFace_CPSFace)
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("Face");
}

///======================================================================================
void CPSFace::step(TPSProcessPass pass)
{
//	if (!FilterPS[1]) return;
	NL_PS_FUNC(CPSFace_step)
	if (pass == PSToolRender) // edition mode only
	{
		showTool();
		return;
	}
	else if (pass == PSMotion)
	{

		if (!_PrecompBasis.empty()) // do we use precomputed basis ?
		{
			// rotate all precomputed basis
			for (CPSVector< CPlaneBasisPair >::V::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
			{
				// not optimized at all, but this will apply to very few elements anyway...
				CMatrix mat;
				mat.rotate(CQuat(it->Axis, CParticleSystem::EllapsedTime * it->AngularVelocity));
				CVector n = mat * it->Basis.getNormal();
				it->Basis = CPlaneBasis(n);
			}
		}
		return;
	}
	else	// check this is the right pass
	if (!
		(	(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	   )
	{
		return;
	}



	if (!_Owner->getSize()) return;
	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);
	if (!numToProcess) return;


	if (step == (1 << 16))
	{
		/// build index iterator
		CPSVector<uint32>::V::const_iterator indexIt = _IndexInPrecompBasis.begin();

		/// draw the faces
		CPSFaceHelper::drawFaces(_Owner->getPos().begin(),
								 indexIt,
								 *this,
								 numToProcess,
								 step
								);
	}
	else
	{
		/// build index iterator
		CAdvance1616Iterator<CPSVector<uint32>::V::const_iterator, const uint32>
			indexIt(_IndexInPrecompBasis.begin(), 0, step);
		CPSFaceHelper::drawFaces(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
								 indexIt,
								 *this,
								 numToProcess,
								 step
								);
	}

}


///======================================================================================
void CPSFace::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSFace_IStream )
	f.serialVersion(1);
	CPSQuad::serial(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);

		init();
	}
	else
	{
		uint32 nbConfigurations = (uint32)_PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);
		}
	}
}


///======================================================================================
/// this produce a random unit vector
static CVector MakeRandomUnitVect(void)
{
	NL_PS_FUNC(MakeRandomUnitVect)
	CVector v((float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  );
	v.normalize();
	return v;
}

///======================================================================================
void CPSFace::hintRotateTheSame(uint32 nbConfiguration
						, float minAngularVelocity
						, float maxAngularVelocity
					  )
{
	NL_PS_FUNC(CPSFace_hintRotateTheSame)
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;
	_PrecompBasis.resize(nbConfiguration);
	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}
		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}
}

///======================================================================================
void CPSFace::fillIndexesInPrecompBasis(void)
{
	NL_PS_FUNC(CPSFace_fillIndexesInPrecompBasis)
	const uint32 nbConf = (uint32)_PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}
	for (CPSVector<uint32>::V::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}

///======================================================================================
void CPSFace::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSFace_newElement)
	CPSQuad::newElement(info);
	newPlaneBasisElement(info);
	const uint32 nbConf = (uint32)_PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}
}

///======================================================================================
void CPSFace::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSFace_deleteElement)
	CPSQuad::deleteElement(index);
	deletePlaneBasisElement(index);
	if (!_PrecompBasis.empty()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}
}

///======================================================================================
void CPSFace::resize(uint32 size)
{
	NL_PS_FUNC(CPSFace_resize)
	nlassert(size < (1 << 16));
	resizePlaneBasis(size);
	if (!_PrecompBasis.empty()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
	CPSQuad::resize(size);
}

} // NL3D
