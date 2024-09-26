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

#include "nel/3d/scene_group.h"
#include "nel/3d/seg_remanence.h"
#include "nel/3d/seg_remanence_shape.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/dru.h"




#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


CVertexBuffer CSegRemanence::_VB;
CIndexBuffer CSegRemanence::_IB;


/// TODO : put this in a header (same code in ps_ribbon_base.cpp ..)
static inline void BuildHermiteVector(const NLMISC::CVector &P0,
							   const NLMISC::CVector &P1,
							   const NLMISC::CVector &T0,
							   const NLMISC::CVector &T1,
									 NLMISC::CVector &dest,
							   float lambda
							   )
{
	const float lambda2 = lambda * lambda;
	const float lambda3 = lambda2 * lambda;
	const float h1 = 2 * lambda3 - 3 * lambda2 + 1;
	const float h2 = - 2 * lambda3 + 3 * lambda2;
	const float h3 = lambda3 - 2 * lambda2 + lambda;
	const float h4 = lambda3 - lambda2;
	/// just avoid some ctor calls here...
	dest.set (h1 * P0.x + h2 * P1.x + h3 * T0.x + h4 * T1.x,
			  h1 * P0.y + h2 * P1.y + h3 * T0.y + h4 * T1.y,
			  h1 * P0.z + h2 * P1.z + h3 * T0.z + h4 * T1.z);

}


/// for test
static inline void BuildLinearVector(const NLMISC::CVector &P0,
									 const NLMISC::CVector &P1,
									 NLMISC::CVector &dest,
									 float lambda,
									 float oneMinusLambda
							        )
{
	NL_PS_FUNC(BuildLinearVector)
	dest.set (lambda * P1.x + oneMinusLambda * P0.x,
			  lambda * P1.y + oneMinusLambda * P0.y,
			  lambda * P1.z + oneMinusLambda * P0.z);
}



//===============================================================
CSegRemanence::CSegRemanence() : _NumSlices(0),
								 _Started(false),
								 _Stopping(false),
								 _Restarted(false),
								 _UnrollRatio(0),
								 _SliceTime(0.05f),
								 _AniMat(NULL),
								 _LastSampleFrame(0)
{
	IAnimatable::resize(AnimValueLast);

	// RenderFilter: We are a SegRemanece
	_RenderFilterType= UScene::FilterSegRemanence;
}

//===============================================================
CSegRemanence::~CSegRemanence()
{
	delete _AniMat;
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// hrc and clip hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}
}

//===============================================================
CSegRemanence::CSegRemanence(CSegRemanence &other)	: CTransformShape(other), _AniMat(NULL)
{
	copyFromOther(other);
}

//===============================================================
CSegRemanence &CSegRemanence::operator = (CSegRemanence &other)
{
	if (this != &other)
	{
		(CTransformShape &) *this = (CTransformShape &) other; // copy base
		copyFromOther(other);
	}
	return *this;
}

//===============================================================
void CSegRemanence::copyFromOther(CSegRemanence &other)
{
	if (this == &other) return;

	CAnimatedMaterial   *otherMat = other._AniMat != NULL ? new CAnimatedMaterial(*other._AniMat)
														  : NULL;
	delete _AniMat;
	_AniMat = otherMat;
	std::copy(other._Samples, other._Samples + 4, _Samples);
	_HeadSample   = other._HeadSample;
	_HeadProgress = other._HeadProgress;
	//
	_Pos	         = other._Pos; // sampled positions at each extremities of segment
	_NumSlices	     = other._NumSlices;
	_NumCorners      = other._NumCorners;
	_Started         = other._Started;
	_Stopping        = other._Stopping;
	_Restarted       = other._Restarted;
	_StartDate       = other._StartDate;
	_CurrDate        = other._CurrDate;
	_UnrollRatio     = other._UnrollRatio;
	_SliceTime       = other._SliceTime;
	_LastSampleFrame = other._LastSampleFrame;
}

//===============================================================
void CSegRemanence::registerBasic()
{
	CScene::registerModel(SegRemanenceShapeId, TransformShapeId, CSegRemanence::creator);
}



// helper functions to fill vb
static inline void vbPush(uint8 *&dest, const CVector &v)
{
	*(CVector *) dest = v;
	dest +=sizeof(CVector);
}

static inline void vbPush(uint8 *&dest, float f)
{
	*(float *) dest = f;
	dest +=sizeof(float);
}

//===============================================================
void CSegRemanence::render(IDriver *drv, CMaterial &mat)
{
	nlassert(_NumSlices >= 2);
	nlassert(_NumCorners >= 2);
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	// resize before locking because of volatile vb
	_VB.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
	_VB.setVertexFormat(CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag);
	_VB.setNumVertices(_NumCorners * (_NumSlices + 1));
	const uint vertexSize = _VB.getVertexSize();
	// Fill Vertex Buffer part
	{
		CVertexBufferReadWrite vba;
		_VB.lock (vba);
		uint8 *datas = (uint8 *) vba.getVertexCoordPointer();
		const uint8 *endDatas = datas + vertexSize *_VB.getNumVertices();
		//
		const CVector *src = &_Pos[0];

		float deltaV = 1.f / (_NumCorners - 1);
		// first slice
		for(uint k = 0; k < _NumCorners; ++k)
		{
			vbPush(datas, *src++);
			vbPush(datas, 0.f);  // U
			vbPush(datas, k * deltaV);  // V
			nlassert(datas <= endDatas);
		}

		float deltaU = 1.f / _NumSlices;
		float baseU = _HeadProgress * deltaU;

		for (uint l = 1; l < _NumSlices; ++l)
		{
			float currU  = baseU + (l - 1) * deltaU;
			for(uint k = 0; k < _NumCorners; ++k)
			{
				vbPush(datas, *src++);
				vbPush(datas, currU);       // U
				vbPush(datas, k * deltaV);  // V
				nlassert(datas <= endDatas);
			}
		}
		// last slice
		const CVector *prevRow = src - _NumCorners;
		for(uint k = 0; k < _NumCorners; ++k)
		{
			vbPush(datas, (1.f - _HeadProgress) * *src + _HeadProgress * *prevRow);
			++ src;
			++ prevRow;
			vbPush(datas, 1.f);         // U
			vbPush(datas, k * deltaV);  // V
			nlassert(datas <= endDatas);
		}
	}
	//
	uint numQuads = (_NumCorners - 1) * _NumSlices;
	// Fill Index Buffer part
	{
		_IB.setPreferredMemory(CIndexBuffer::RAMVolatile, false);
		_IB.setFormat(CIndexBuffer::Indices16);
		_IB.setNumIndexes(numQuads * 6);
		//
		CIndexBufferReadWrite iba;
		_IB.lock(iba);
		uint16 *indexPtr = (uint16 *) iba.getPtr();

		for (uint l = 0; l < _NumSlices; ++l)
		{
			for(uint k = 0; k < (_NumCorners - 1); ++k)
			{
				*indexPtr++ = l * _NumCorners + k;
				*indexPtr++ = l * _NumCorners + k + 1;
				*indexPtr++ = (l + 1) * _NumCorners + k;
				//
				*indexPtr++ = l * _NumCorners + k + 1;
				*indexPtr++ = (l + 1) * _NumCorners + k + 1;
				*indexPtr++ = (l + 1) * _NumCorners + k;
			}
		}
		nlassert(indexPtr == (uint16 *) iba.getPtr() + _IB.getNumIndexes());

	}

	// roll / unroll using texture matrix
	CMatrix texMat;
	texMat.setPos(NLMISC::CVector(1.f - _UnrollRatio, 0, 0));

	if (mat.getTexture(0) != NULL)
		mat.setUserTexMat(0, texMat);
	drv->setupModelMatrix(CMatrix::Identity);

	drv->activeVertexBuffer(_VB);
	drv->activeIndexBuffer(_IB);
	drv->renderTriangles(mat, 0, numQuads * 2);

	// draw wire frame version if needed
	#ifdef DEBUG_SEG_REMANENCE_DISPLAY
		static CMaterial unlitWF;
		unlitWF.initUnlit();
		unlitWF.setDoubleSided(true);
		IDriver::TPolygonMode oldPM = drv->getPolygonMode();
		drv->setPolygonMode(IDriver::Line);
		drv->renderTriangles(unlitWF, 0, numQuads * 2);
		drv->setPolygonMode(oldPM);
	#endif

	CScene *scene = getOwnerScene();
	// change unroll ratio
	if (!_Stopping)
	{
		if (_UnrollRatio != 1.f)
		_UnrollRatio = std::min(1.f, _UnrollRatio + scene->getEllapsedTime() / (srs->getNumSlices() * _SliceTime));
	}
	else
	{
		_UnrollRatio = std::max(0.f, _UnrollRatio - srs->getRollupRatio() * scene->getEllapsedTime() / (srs->getNumSlices() * _SliceTime));
		if (_UnrollRatio == 0.f)
		{
			_Stopping = false;
			_Started = false;
		}
	}
}

//===============================================================
void CSegRemanence::setupFromShape()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	if (srs->getNumCorners() != _NumCorners || srs->getNumSlices() != _NumSlices)
	{
		_NumCorners = srs->getNumCorners();
		_NumSlices  = srs->getNumSlices();
		_Pos.resize(_NumCorners * (_NumSlices + 1));
		for(uint k = 0; k < 4; ++k)
		{
			_Samples[k].Pos.resize(_NumSlices + 1);
		}
	}
	updateOpacityFromShape();
}

//===============================================================
void CSegRemanence::samplePos(double date)
{
	uint  newHeadSample = (uint) floor(date / _SliceTime);
	double sliceElapsedTime = date - (newHeadSample * _SliceTime);
	_HeadProgress = (float) (sliceElapsedTime / _SliceTime);
	NLMISC::clamp(_HeadProgress, 0.f, 1.f);
	uint offset = newHeadSample - _HeadSample; // number of samples to remove
	if(!_Restarted)
	{
		if (offset > 0)
		{
			offset = std::min(offset, (uint) _NumSlices);
			_Samples[0].swap(_Samples[1]);
			_Samples[1].swap(_Samples[2]);
			_Samples[2].swap(_Samples[3]);
			if (offset < _NumSlices + 1)
			{
				// make room for new position
				memmove(&_Pos[_NumCorners * offset], &_Pos[0], sizeof(_Pos[0]) * _NumCorners * (_NumSlices + 1 - offset));
			}
			// else, too much time ellapsed, are sampled pos are invalidated
			_HeadSample = newHeadSample;
		}
	}
	_Samples[3].Date = date;
	//
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	// update positions for sample head
	for(uint k = 0; k <_NumCorners;++k)
	{
		_Samples[3].Pos[k] = getWorldMatrix() * srs->getCorner(k);
	}
	if (_Restarted)
	{
		_HeadSample = newHeadSample;
		_Samples[0] = _Samples[1] = _Samples[2] = _Samples[3];
		CVector *head = &_Pos[0];
		for(uint l = 0; l < _NumSlices + 1; ++l)
		{
			for(uint k = 0; k < _NumCorners;++k)
			{
				*head++ = _Samples[0].Pos[k];
			}
		}
		_Restarted = false;
		return;
	}
	// update head pos
	CVector *head = &_Pos[0];
	CVector *endPtr = head + _NumCorners * (_NumSlices + 1);
	for(uint k = 0; k < _NumCorners;++k)
	{
		*head++ = _Samples[3].Pos[k];
	}
	// update current positions from sample pos
    double currDate = _Samples[3].Date - sliceElapsedTime;
	// interpolate linearly for 2 firstsamples
	while (currDate > _Samples[2].Date && head != endPtr)
	{
		double dt = _Samples[3].Date - _Samples[2].Date;
		float lambda = (float) (dt != 0 ? (currDate - _Samples[2].Date) / dt : 0);
		for(uint k = 0; k < _NumCorners;++k)
		{
			*head++ = lambda * (_Samples[3].Pos[k] - _Samples[2].Pos[k]) + _Samples[2].Pos[k];
		}
		currDate -= _SliceTime;
	}
	if (head != endPtr)
	{
		// interpolate smoothly for remaining samples
		while (currDate >= _Samples[1].Date)
		{
			double dt = _Samples[2].Date - _Samples[1].Date;
			if (dt == 0)
			{
				for(uint k = 0; k < _NumCorners;++k)
				{
					*head++ = _Samples[2].Pos[k];
				}
			}
			else
			{
				double lambda = (currDate - _Samples[1].Date) / dt;
				CVector T0, T1;
				for(uint k = 0; k < _NumCorners;++k)
				{
					if (_Samples[2].Date != _Samples[0].Date)
					{
						T0 = (float) dt * (_Samples[2].Pos[k] - _Samples[0].Pos[k]) / (float) (_Samples[2].Date - _Samples[0].Date);
					}
					else
					{
						T0= NLMISC::CVector::Null;
					}
					if (_Samples[3].Date != _Samples[1].Date)
					{
						T1 = (float) dt * (_Samples[3].Pos[k] - _Samples[1].Pos[k]) / (float) (_Samples[3].Date - _Samples[1].Date);
					}
					else
					{
						T1= NLMISC::CVector::Null;
					}
					BuildHermiteVector(_Samples[1].Pos[k], _Samples[2].Pos[k], T0, T1, *head, (float) lambda);
					++ head;
				}
			}
			if (head == endPtr) break;
			currDate -= _SliceTime;
		}
		/*
			// Version with no time correction
			while (currDate >= _Samples[1].Date)
			{
				float lambda = (float) ((currDate - _Samples[1].Date) / (_Samples[2].Date - _Samples[1].Date));
				for(uint k = 0; k < _NumCorners;++k)
				{
					CVector T1 = 0.5f * (_Samples[3].Pos[k] - _Samples[1].Pos[k]);
					CVector T0 = 0.5f * (_Samples[2].Pos[k] - _Samples[0].Pos[k]);
					BuildHermiteVector(_Samples[1].Pos[k], _Samples[2].Pos[k], T0, T1, *head, lambda);
					++ head;
				}
				if (head == endPtr) break;
				currDate -= _SliceTime;
			}
		*/
	}
}


//===============================================================
void CSegRemanence::start()
{
	if (_SliceTime == 0.f) return;
	if (_Started && !_Stopping) return;
	restart();
}

//===============================================================
void CSegRemanence::restart()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	if (!srs->getTextureShifting())
	{
		_UnrollRatio = 1.f;
	}
	else
	{
		if (!_Stopping)
			_UnrollRatio = 0.f;
	}
	_Started = _Restarted = true;
	_Stopping = false;
}

//===============================================================
void CSegRemanence::stop()
{
	_Stopping = true;
}

//===============================================================
void CSegRemanence::stopNoUnroll()
{
	_Started = _Restarted = _Stopping = false;
}

//===============================================================
void CSegRemanence::updateOpacityFromShape()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	bool transparent = srs->getMaterial().getBlend();
	setTransparency(transparent);
	setOpacity(!transparent);
}

//===============================================================
void CSegRemanence::setAnimatedMaterial(CAnimatedMaterial *mat)
{
	if (mat == _AniMat) return;
	delete _AniMat;
	_AniMat = mat;
	_AniMat->setFather(this, OwnerBit);
}

//===============================================================
void CSegRemanence::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);
	if (_AniMat)
	{
		_AniMat->registerToChannelMixer(chanMixer, prefix + _AniMat->getMaterialName() + ".")	;
	}
}

//===============================================================
ITrack *CSegRemanence::getDefaultTrack (uint valueId)
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	switch (valueId)
	{
		case PosValue:			return srs->getDefaultPos();
		case RotQuatValue:		return srs->getDefaultRotQuat();
		case ScaleValue:		return srs->getDefaultScale();
	}
	return CTransformShape::getDefaultTrack(valueId);
	return NULL;

}

//===============================================================
void CSegRemanence::traverseAnimDetail()
{
	CTransformShape::traverseAnimDetail();
	#ifndef DEBUG_SEG_REMANENCE_DISPLAY
		if (isStarted())
	#endif
	{
		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////

		CScene *scene = getOwnerScene();
		if (scene->getNumRender() != (_LastSampleFrame + 1))
		{
			if (!isStopping())
			{
				// if wasn't visible at previous frame, must invalidate position
				restart();
			}
			else
			{
				// ribbon started unrolling when it disapperaed from screen so simply remove it
				stopNoUnroll();
			}
		}
		_LastSampleFrame = scene->getNumRender();
		setupFromShape();
		samplePos(scene->getCurrentTime());

		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////


		// test if animated material must be updated.
		if(IAnimatable::isTouched(CSegRemanence::OwnerBit))
		{
			if (getAnimatedMaterial())
				getAnimatedMaterial()->update();
			clearAnimatedMatFlag();
		}
	}
}

//===============================================================
void CSegRemanence::setSliceTime(float duration)
{
	if ( duration != _SliceTime )
	{
		stopNoUnroll();
		_SliceTime = duration;
	}
}

}
