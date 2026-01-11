// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/point_light.h"
#include "nel/3d/light.h"
#include "nel/3d/transform.h"
#include "nel/misc/common.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
//NLMISC::CBlockMemory<CTransform*, false>		CPointLight::_LightedModelListMemory(NL3D_LIGHTED_MODEL_ALLOC_BLOCKSIZE);


// ***************************************************************************
CPointLight::CPointLight() : _LightedModels(/*&_LightedModelListMemory*/)
{
	_Position= CVector::Null;
	_Ambient= CRGBA::Black;
	_Diffuse= _Specular= CRGBA::White;

	// Default setup. this is arbitrary
	_Type= PointLight;
	_AttenuationBegin= 10;
	_AttenuationEnd= 30;

	// Default spot setup. this is arbitrary
	_SpotDirection.set(0,1,0);
	_SpotAngleBegin= float(NLMISC::Pi/4);
	_SpotAngleEnd= float(NLMISC::Pi/2);

	// compute AttenuationFactors only one time.
	static	bool	done= false;
	static	float	cAtt, lAtt, qAtt;
	static	float	spotCOOD, spotCAE, spotEXP;
	if(!done)
	{
		done= true;
		computeAttenuationFactors();
		computeSpotAttenuationFactors();
		// bkup setup.
		cAtt= _ConstantAttenuation;
		lAtt= _LinearAttenuation;
		qAtt= _QuadraticAttenuation;
		spotCAE= _CosSpotAngleEnd;
		spotCOOD= _OOCosSpotAngleDelta;
		spotEXP= _SpotExponent;
	}
	else
	{
		// just copy bkuped setup.
		_ConstantAttenuation= cAtt;
		_LinearAttenuation= lAtt;
		_QuadraticAttenuation= qAtt;
		_CosSpotAngleEnd= spotCAE;
		_OOCosSpotAngleDelta= spotCOOD;
		_SpotExponent= spotEXP;
	}

	_AddAmbientWithSun= false;
}


// ***************************************************************************
CPointLight::CPointLight(const CPointLight &o) : _LightedModels(/*&_LightedModelListMemory*/)
{
	// copy (no need to init)
	operator=(o);
}

// ***************************************************************************
CPointLight &CPointLight::operator=(const CPointLight &o)
{
	/// copy all but _LightedModels !!
	_Type= o._Type;
	_Position= o._Position;
	_Ambient= o._Ambient;
	_Diffuse= o._Diffuse;
	_Specular= o._Specular;

	_AttenuationBegin= o._AttenuationBegin;
	_AttenuationEnd= o._AttenuationEnd;
	_OODeltaAttenuation= o._OODeltaAttenuation;
	_ConstantAttenuation= o._ConstantAttenuation;
	_LinearAttenuation= o._LinearAttenuation;
	_QuadraticAttenuation= o._QuadraticAttenuation;

	_SpotDirection= o._SpotDirection;
	_SpotAngleBegin= o._SpotAngleBegin;
	_SpotAngleEnd= o._SpotAngleEnd;
	_CosSpotAngleEnd= o._CosSpotAngleEnd;
	_OOCosSpotAngleDelta= o._OOCosSpotAngleDelta;
	_SpotExponent= o._SpotExponent;

	_AddAmbientWithSun= o._AddAmbientWithSun;

	return *this;
}


// ***************************************************************************
CPointLight::~CPointLight()
{
	resetLightedModels();
}


// ***************************************************************************
void			CPointLight::setType(TType type)
{
	_Type= type;
}
CPointLight::TType		CPointLight::getType() const
{
	return _Type;
}

// ***************************************************************************
void			CPointLight::setupAttenuation(float attenuationBegin, float attenuationEnd)
{
	// set values.
	attenuationBegin= max(attenuationBegin, 0.f);
	attenuationEnd= max(attenuationEnd, attenuationBegin);
	_AttenuationBegin= attenuationBegin;
	_AttenuationEnd= attenuationEnd;

	// update factors.
	computeAttenuationFactors();

}


// ***************************************************************************
void			CPointLight::setupSpotAngle(float spotAngleBegin, float spotAngleEnd)
{
	clamp(spotAngleBegin, 0.f, float(Pi));
	clamp(spotAngleEnd, spotAngleBegin, float(Pi));
	_SpotAngleBegin= spotAngleBegin;
	_SpotAngleEnd= spotAngleEnd;

	// update factors.
	computeSpotAttenuationFactors();
}


// ***************************************************************************
void			CPointLight::setupSpotDirection(const CVector &dir)
{
	_SpotDirection= dir;
	_SpotDirection.normalize();
}


// ***************************************************************************
void			CPointLight::computeAttenuationFactors()
{
	// disable attenuation?
	if(_AttenuationBegin==0 && _AttenuationEnd==0)
	{
		// setup for attenuation disabled.
		_ConstantAttenuation= 1;
		_LinearAttenuation= 0;
		_QuadraticAttenuation= 0;
	}
	else
	{
		// precompute attenuation values, with help of CLight formula!!
		CLight	dummyLight;
		dummyLight.setupAttenuation(_AttenuationBegin, _AttenuationEnd);
		_ConstantAttenuation= dummyLight.getConstantAttenuation();
		_LinearAttenuation= dummyLight.getLinearAttenuation();
		_QuadraticAttenuation= dummyLight.getQuadraticAttenuation();

		// setup _OODeltaAttenuation
		_OODeltaAttenuation= _AttenuationEnd - _AttenuationBegin;
		if(_OODeltaAttenuation <=0 )
			_OODeltaAttenuation= 0;
		else
			_OODeltaAttenuation= 1.0f / _OODeltaAttenuation;
	}
}


// ***************************************************************************
void			CPointLight::computeSpotAttenuationFactors()
{
	// Factors for linear Attenuation.
	float	cosSpotAngleBegin= (float)cosf(_SpotAngleBegin);
	_CosSpotAngleEnd= (float)cos(_SpotAngleEnd);
	if(cosSpotAngleBegin - _CosSpotAngleEnd > 0)
		_OOCosSpotAngleDelta= 1.0f / (cosSpotAngleBegin - _CosSpotAngleEnd);
	else
		_OOCosSpotAngleDelta= 1e10f;

	// compute an exponent such that at middleAngle, att is 0.5f.
	float	caMiddle= (cosSpotAngleBegin + _CosSpotAngleEnd) /2;
	float divid=(float)log (caMiddle);
	if (divid==0.f)
		divid=0.0001f;
	_SpotExponent= (float)(log (0.5)/divid);
}


// ***************************************************************************
void			CPointLight::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(2);

	if(ver>=2)
		f.serial(_AddAmbientWithSun);
	else
		_AddAmbientWithSun= false;

	if(ver>=1)
	{
		f.serialEnum(_Type);
		f.serial(_SpotDirection);
		f.serial(_SpotAngleBegin);
		f.serial(_SpotAngleEnd);
	}
	else if(f.isReading())
	{
		_Type= PointLight;
		_SpotDirection.set(0,1,0);
		_SpotAngleBegin= float(NLMISC::Pi/4);
		_SpotAngleEnd= float(NLMISC::Pi/2);
	}

	f.serial(_Position);
	f.serial(_Ambient);
	f.serial(_Diffuse);
	f.serial(_Specular);
	f.serial(_AttenuationBegin);
	f.serial(_AttenuationEnd);

	// precompute.
	if(f.isReading())
	{
		computeAttenuationFactors();
		computeSpotAttenuationFactors();
	}

}


// ***************************************************************************
float			CPointLight::computeLinearAttenuation(const CVector &pos) const
{
	return computeLinearAttenuation(pos, (pos - _Position).norm() );
}

// ***************************************************************************
float			CPointLight::computeLinearAttenuation(const CVector &pos, float dist, float modelRadius) const
{
	float	gAtt;

	// Attenuation Distance
	if(_AttenuationEnd==0)
		gAtt= 1;
	else
	{
		float	distMinusRadius= dist - modelRadius;
		if(distMinusRadius<_AttenuationBegin)
			gAtt= 1;
		else if(distMinusRadius<_AttenuationEnd)
		{
			gAtt= (_AttenuationEnd - distMinusRadius) * _OODeltaAttenuation;
		}
		else
			gAtt= 0;
	}

	// Spot Attenuation
	if(_Type== SpotLight)
	{
		float	spotAtt;

		// Compute unnormalized direction
		CVector	dir= pos - _Position;
		// get cosAngle(dir, SpotDirection):
		float	cosAngleDirSpot= (dir*_SpotDirection) / dist;

		// Modify with modelRadius. NB: made Only for big models.
		if(modelRadius>0)
		{
			// If the pointLight is in the model, consider no spotAtt
			if(modelRadius > dist)
				spotAtt= 1;
			else
			{
				// compute the angle of the cone made by the model sphere and the pointLightCenter.
				float	cosAngleSphere= modelRadius / sqrtf( sqr(dist) + sqr(modelRadius) );
				/* If this one is smaller than cosAngleDirSpot, it's mean that the angle of this cone is greater than the
					angleDirSpot, hence a part of the sphere "ps" exist such that _SportDirection*(ps-_Position).normed() == 1
					=> no spotAttenuation
				*/
				if(cosAngleSphere < cosAngleDirSpot)
					spotAtt= 1;
				else
				{
					// Must compute cos( AngleDirSpot-AngleSphere )
					float	sinAngleSphere= sqrtf(1 - sqr(cosAngleSphere));
					float	sinAngleDirSpot= sqrtf(1 - sqr(cosAngleDirSpot));
					float	cosDelta= cosAngleSphere * cosAngleDirSpot + sinAngleSphere * sinAngleDirSpot;

					// spot attenuation on the exterior of the sphere
					spotAtt= (cosDelta - _CosSpotAngleEnd) * _OOCosSpotAngleDelta;
				}
			}
		}
		else
		{
			// spot attenuation
			spotAtt= (cosAngleDirSpot - _CosSpotAngleEnd) * _OOCosSpotAngleDelta;
		}

		// modulate
		clamp(spotAtt, 0.f, 1.f);
		gAtt*= spotAtt;
	}

	return gAtt;
}

// ***************************************************************************
void			CPointLight::setupDriverLight(CLight &light, uint8 factor)
{
	// expand 0..255 to 0..256, to avoid loss of precision.
	uint	ufactor= factor + (factor>>7);	// add 0 or 1.

	// modulate with factor
	CRGBA	ambient, diffuse, specular;
	ambient.modulateFromuiRGBOnly(_Ambient, ufactor);
	diffuse.modulateFromuiRGBOnly(_Diffuse, ufactor);
	specular.modulateFromuiRGBOnly(_Specular, ufactor);

	// setup the pointLight
	if(_Type == SpotLight )
	{
		light.setupSpotLight(ambient, diffuse, specular, _Position, _SpotDirection,
			_SpotExponent, float(NLMISC::Pi/2) ,
			_ConstantAttenuation, _LinearAttenuation, _QuadraticAttenuation);
	}
	// PointLight or AmbientLight
	else
	{
		light.setupPointLight(ambient, diffuse, specular, _Position, CVector::Null,
			_ConstantAttenuation, _LinearAttenuation, _QuadraticAttenuation);
	}
}


// ***************************************************************************
void			CPointLight::setupDriverLightUserAttenuation(CLight &light, uint8 factor)
{
	// expand 0..255 to 0..256, to avoid loss of precision.
	uint	ufactor= factor + (factor>>7);	// add 0 or 1.

	// modulate with factor
	CRGBA	ambient, diffuse, specular;
	ambient.modulateFromuiRGBOnly(_Ambient, ufactor);
	diffuse.modulateFromuiRGBOnly(_Diffuse, ufactor);
	specular.modulateFromuiRGBOnly(_Specular, ufactor);

	// setup the pointLight, disabling attenuation.
	// NB: setup a pointLight even if it is a SpotLight because already attenuated
	light.setupPointLight(ambient, diffuse, specular, _Position, CVector::Null,
		1, 0, 0);
}


// ***************************************************************************
void			CPointLight::resetLightedModels()
{
	// For each transform, resetLighting him.
	while(_LightedModels.begin() != _LightedModels.end() )
	{
		CTransform	*model= *_LightedModels.begin();
		// reset lighting
		model->resetLighting();

		// NB: the transform must erase him from this list.
		nlassert( _LightedModels.begin() == _LightedModels.end() || *_LightedModels.begin() != model );
	}
}


// ***************************************************************************
CPointLight::ItTransformList	CPointLight::appendLightedModel(CTransform *model)
{
	// append the entry in the list
	_LightedModels.push_back(model);
	ItTransformList	it= _LightedModels.end();
	it--;
	return it;
}
// ***************************************************************************
void			CPointLight::removeLightedModel(ItTransformList it)
{
	// delete the entry in the list.
	_LightedModels.erase(it);
}
// ***************************************************************************
void			CPointLight::purge ()
{
	//_LightedModelListMemory.purge();
}

// ***************************************************************************
void			CPointLight::setAddAmbientWithSun(bool state)
{
	_AddAmbientWithSun= state;
}


} // NL3D
