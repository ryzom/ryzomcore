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
#include "nel/3d/ps_light.h"
#include "nel/3d/point_light_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/ps_util.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"

namespace NL3D
{


// ***************************************************************************************************************
CPSLight::CPSLight() : _Color(CRGBA::White),
					   _ColorScheme(NULL),
					   _AttenStart(0.1f),
					   _AttenStartScheme(NULL),
					   _AttenEnd(1.f),
					   _AttenEndScheme(NULL)
{
	NL_PS_FUNC(CPSLight_CPSLight)
}

// ***************************************************************************************************************
CPSLight::~CPSLight()
{
	NL_PS_FUNC(CPSLight_CPSLight)
	if (_Owner && _Owner->getOwner())
	{
		// check that all lights have been deleted
		for(uint k = 0; k < _Lights.getSize(); ++k)
		{
			if (_Lights[k]) _Owner->getOwner()->getScene()->deleteModel(_Lights[k]);
		}
	}
	else
	{
		#ifdef NL_DEBUG
			// check that all lights have been deleted
			for(uint k = 0; k < _Lights.getSize(); ++k)
			{
				nlassert(_Lights[k] == NULL); // error there's	leak!
			}
		#endif
	}
	delete _ColorScheme;
	delete _AttenStartScheme;
	delete _AttenEndScheme;
}

// ***************************************************************************************************************
void CPSLight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSLight_serial)
	CPSLocatedBindable::serial(f);
	// version 1 : in version 0, scheme where not resized correctly; Fixed in this version
	// version 0 : color, start attenuation radius, end attenuation radius.
	sint ver = f.serialVersion(1);
	// color
	bool hasColorScheme = _ColorScheme != NULL;
	f.serial(hasColorScheme);
	if (hasColorScheme)
	{
		f.serialPolyPtr(_ColorScheme);
	}
	else
	{
		f.serial(_Color);
	}
	// Atten start
	bool hasAttenStartScheme = _AttenStartScheme != NULL;
	f.serial(hasAttenStartScheme);
	if (hasAttenStartScheme)
	{
		f.serialPolyPtr(_AttenStartScheme);
	}
	else
	{
		f.serial(_AttenStart);
	}
	// Atten end
	bool hasAttenEndScheme = _AttenEndScheme != NULL;
	f.serial(hasAttenEndScheme);
	if (hasAttenEndScheme)
	{
		f.serialPolyPtr(_AttenEndScheme);
	}
	else
	{
		f.serial(_AttenEnd);
	}

	// save # of lights
	if (ver == 0)
	{
		uint32 dummyNumLights; // from old buggy version
		f.serial(dummyNumLights);
	}
	if (f.isReading())
	{
		if (_Owner)
		{
			resize(_Owner->getMaxSize());
			for(uint k = 0; k < _Owner->getSize(); ++k)
			{
				CPSEmitterInfo ei;
				ei.setDefaults();
				newElement(ei);
			}
		}
		else
		{
			resize(0);
		}
	}
}

// ***************************************************************************************************************
uint32 CPSLight::getType(void) const
{
	NL_PS_FUNC(CPSLight_getType)
	return PSLight;
}

// ***************************************************************************************************************
void CPSLight::onShow(bool shown)
{
	for(uint k = 0; k < _Lights.getSize(); ++k)
	{
		if (_Lights[k])
		{
			if (shown)
			{
				_Lights[k]->show();
			}
			else
			{
				_Lights[k]->hide();
			}
		}
	}
}

// ***************************************************************************************************************
void CPSLight::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSLight_step)
	if (pass != PSMotion)
	{
		if (pass == PSToolRender)
		{
			show();
		}
		return;
	}
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	CScene *scene = _Owner->getOwner()->getScene();
	const uint32 BATCH_SIZE = 512;
	uint32 numLeftLights = _Lights.getSize();
	// avoid ctor call for color array
	uint8		   colorArray[BATCH_SIZE * sizeof(NLMISC::CRGBA)];
	NLMISC::CRGBA *colors = (NLMISC::CRGBA *) colorArray;
	float		  attenStart[BATCH_SIZE];
	float		  attenEnd[BATCH_SIZE];
	CPSAttrib<CPointLightModel *>::iterator lightIt = _Lights.begin();
	const CMatrix *convMat = &(getLocalToWorldMatrix());
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin();
	CRGBA globalColor = _Owner->getOwner()->getGlobalColor();
	while (numLeftLights)
	{
		uint32 toProcess = std::min(numLeftLights, BATCH_SIZE);
		// compute colors
		NLMISC::CRGBA *colPointer;
		uint   colStride;
		if (_ColorScheme)
		{
			colPointer = (CRGBA *) _ColorScheme->make(_Owner, _Lights.getSize() - numLeftLights, colors, sizeof(CRGBA), toProcess, true);
			colStride = 1;
		}
		else
		{
			colPointer = &_Color;
			colStride = 0;
		}
		// compute start attenuation
		float *attenStartPointer;
		uint   attenStartStride;
		if (_AttenStartScheme)
		{
			attenStartPointer = (float *) _AttenStartScheme->make(_Owner, _Lights.getSize() - numLeftLights, attenStart, sizeof(float), toProcess, true);
			attenStartStride = 1;
		}
		else
		{
			attenStartPointer = &_AttenStart;
			attenStartStride = 0;
		}
		// compute end attenuation
		float *attenEndPointer;
		uint   attenEndStride;
		if (_AttenEndScheme)
		{
			attenEndPointer = (float *) _AttenEndScheme->make(_Owner, _Lights.getSize() - numLeftLights, attenEnd, sizeof(float), toProcess, true);
			attenEndStride = 1;
		}
		else
		{
			attenEndPointer = &_AttenEnd;
			attenEndStride = 0;
		}
		numLeftLights -= toProcess;
		do
		{
			if (!*lightIt)
			{
				// light not created, create it from scene
				if (scene)
				{
					*lightIt = NLMISC::safe_cast<CPointLightModel *>(scene->createModel(PointLightModelId));
					if (*lightIt)
					{
						(*lightIt)->setTransformMode(CTransform::RotEuler);
					}
				}
			}
			if (*lightIt)
			{
				NLMISC::CVector pos = *convMat * *posIt;
				CPointLightModel *plm = *lightIt;
				if (pos != plm->getPos()) plm->setPos(pos);


				if (CParticleSystem::OwnerModel)
				{
					// make sure the visibility is the same
					if (CParticleSystem::OwnerModel->isHrcVisible())
					{
						plm->show();
					}
					else
					{
						plm->hide();
					}
					plm->setClusterSystem(CParticleSystem::OwnerModel->getClusterSystem());
				}

				CRGBA newCol = *colPointer;
				newCol.modulateFromColor(newCol, globalColor);
				if (newCol != plm->PointLight.getDiffuse())
				{
					plm->PointLight.setColor(newCol);
				}
				colPointer += colStride;
				if (*attenStartPointer != plm->PointLight.getAttenuationBegin()
				    || *attenEndPointer != plm->PointLight.getAttenuationEnd()
				   )
				{
					plm->PointLight.setupAttenuation(*attenStartPointer, *attenEndPointer);
				}
				attenStartPointer += attenStartStride;
				attenEndPointer += attenEndStride;
			}
			++ lightIt;
			++ posIt;
		}
		while(--toProcess);
	}
}

// ***************************************************************************************************************
void CPSLight::setColor(NLMISC::CRGBA color)
{
	NL_PS_FUNC(CPSLight_setColor)
	delete _ColorScheme;
	_ColorScheme = NULL;
	_Color = color;
}

// ***************************************************************************************************************
void CPSLight::setColorScheme(CPSAttribMaker<NLMISC::CRGBA> *scheme)
{
	NL_PS_FUNC(CPSLight_setColorScheme)
	delete _ColorScheme;
	_ColorScheme = scheme;
	if (_Owner)
	{
		if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

// ***************************************************************************************************************
void CPSLight::setAttenStart(float radius)
{
	NL_PS_FUNC(CPSLight_setAttenStart)
	nlassert(radius > 0.f);
	delete _AttenStartScheme;
	_AttenStartScheme =	NULL;
	_AttenStart = radius;
}

// ***************************************************************************************************************
void CPSLight::setAttenStartScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSLight_setAttenStartScheme)
	delete _AttenStartScheme;
	_AttenStartScheme = scheme;
	if (_Owner)
	{
		if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

// ***************************************************************************************************************
void CPSLight::setAttenEnd(float radius)
{
	NL_PS_FUNC(CPSLight_setAttenEnd)
	delete _AttenEndScheme;
	_AttenEndScheme = NULL;
	_AttenEnd = radius;
}

// ***************************************************************************************************************
void CPSLight::setAttenEndScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSLight_setAttenEndScheme)
	delete _AttenEndScheme;
	_AttenEndScheme = scheme;
	if (_Owner)
	{
		if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

// ***************************************************************************************************************
void CPSLight::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSLight_newElement)
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->newElement(info);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->newElement(info);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->newElement(info);
	_Lights.insert(NULL); // instance is created during step()
}

// ***************************************************************************************************************
void CPSLight::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSLight_deleteElement)
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->deleteElement(index);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->deleteElement(index);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->deleteElement(index);
	if (_Lights[index])
	{
		nlassert(_Owner && _Owner->getScene());
		_Owner->getScene()->deleteModel(_Lights[index]);
	}
	_Lights.remove(index);
}

// ***************************************************************************************************************
void CPSLight::resize(uint32 size)
{
	NL_PS_FUNC(CPSLight_resize)
	nlassert(size < (1 << 16));
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	_Lights.resize(size);
}

// ***************************************************************************************************************
void CPSLight::releaseAllRef()
{
	NL_PS_FUNC(CPSLight_releaseAllRef)
	CPSLocatedBindable::releaseAllRef();
	// delete all lights, because pointer to the scene is lost after detaching from a system.
	for(uint k = 0; k < _Lights.getSize(); ++k)
	{
		if (_Lights[k])
		{
			nlassert(_Owner && _Owner->getScene()); // if there's an instance there must be a scene from which it was created.
			_Owner->getScene()->deleteModel(_Lights[k]);
			_Lights[k] = NULL;
		}
	}
}

// ***************************************************************************************************************
void CPSLight::show()
{
	NL_PS_FUNC(CPSLight_show)
	uint32 index;
	CPSLocated *loc;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

	NLMISC::CMatrix xzMat;
	xzMat.setRot(CVector::I, CVector::K, CVector::Null);
	NLMISC::CMatrix xyMat;
	xyMat.setRot(CVector::I, CVector::J, CVector::Null);
	NLMISC::CMatrix yzMat;
	yzMat.setRot(CVector::J, CVector::K, CVector::Null);


	getDriver()->setupModelMatrix(NLMISC::CMatrix::Identity);
	const uint numSubdiv = 32;
	// for each element, see if it is the selected element, and if yes, display in red
	for (uint k = 0; k < _Lights.getSize(); ++k)
	{
		float radiusStart = _AttenStartScheme ? _AttenStartScheme->get(_Owner, k) : _AttenStart;
		float radiusEnd = _AttenEndScheme ? _AttenEndScheme->get(_Owner, k) : _AttenEnd;
		NLMISC::clamp(radiusStart, 0.f, radiusEnd);
		const NLMISC::CRGBA colStart = (((lb == NULL || this == lb) && loc == _Owner && index == k)  ? CRGBA::Blue : CRGBA(0, 0, 127));
		const NLMISC::CRGBA colEnd = (((lb == NULL || this == lb) && loc == _Owner && index == k)  ? CRGBA::Red : CRGBA(127, 0, 0));
		//
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], xzMat, numSubdiv, colStart);
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], xyMat, numSubdiv, colStart);
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], yzMat, numSubdiv, colStart);
		//
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], xzMat, numSubdiv, colEnd);
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], xyMat, numSubdiv, colEnd);
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], yzMat, numSubdiv, colEnd);
		//
	}
}

} // namespace NL3D
