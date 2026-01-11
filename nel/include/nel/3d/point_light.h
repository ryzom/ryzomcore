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

#ifndef NL_POINT_LIGHT_H
#define NL_POINT_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"
#include "nel/misc/stl_block_list.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CRGBA;


class	CLight;
class	CTransform;


// ***************************************************************************
// Size of a block for allocation of lighted models nodes.
#define	NL3D_LIGHTED_MODEL_ALLOC_BLOCKSIZE	1024


// ***************************************************************************
/**
 *	Description of a light. Owned by an IG, or a CPointLightModel.
 *	With the special sunLight, this is the only light which can interact with CTransform models in the
 *	standard lighting system.
 *
 *	Only Positionnal with or without attenuation are supported. no directionnal.
 *	This restriction is for faster rendering, especially if VertexProgram is used.
 *	New: Spot are managed but VertexProgrammed meshes won't use localAttenuation.
 *	Special Ambiant are provided too but they are considered like PointLight for dynamic light.
 *	They are used in a special way for static light in Igs.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 * \see CLightingManager
 */
class CPointLight
{
public:
	/// The list of model this light influence.
	//typedef	NLMISC::CSTLBlockList<CTransform*>	TTransformList;
	typedef	std::list<CTransform*>	TTransformList;
	typedef	TTransformList::iterator			ItTransformList;

	enum	TType
	{
		// The light is a point.
		PointLight= 0,

		// The light is a spotlight with a cone.
		SpotLight,

		// The light is an Ambient PointLight in an Ig.
		AmbientLight
	};


public:

	/** Constructor
	 *	Default type is PointLight.
	 *	Default ambient is Black, Diffuse and Specular are white.
	 *	Position is CVector::Null.
	 *	Attenuation is 10-30.
	 */
	CPointLight();
	/// call resetLightModels.
	~CPointLight();
	/// do not copy _LightedModels.
	CPointLight(const CPointLight &o);
	/// do not copy _LightedModels.
	CPointLight &operator=(const CPointLight &o);


	/// \name Light setup
	// @{

	/// set/get the type of the light.
	void			setType(TType type);
	TType			getType() const;

	/// Set the position in WorldSpace.
	void			setPosition(const CVector &v) {_Position= v;}
	/// Get the position in WorldSpace.
	const CVector	&getPosition() const {return _Position;}


	/// Set the ambient color of the light. Default to Black.
	void			setAmbient (NLMISC::CRGBA ambient)	{_Ambient=ambient;}
	/// Set the diffuse color of the light. Default to White
	void			setDiffuse (NLMISC::CRGBA diffuse)	{_Diffuse=diffuse;}
	/// Set the specular color of the light. Default to White
	void			setSpecular (NLMISC::CRGBA specular)	{_Specular=specular;}
	/// Set the diffuse and specular color of the light to the same value. don't modify _Ambient.
	void			setColor (NLMISC::CRGBA color)	{_Diffuse= _Specular= color;}

	/// Get the ambient color of the light.
	NLMISC::CRGBA	getAmbient () const	{return _Ambient;}
	/// Get the diffuse color of the light.
	NLMISC::CRGBA	getDiffuse () const	{return _Diffuse;}
	/// Get the specular color of the light.
	NLMISC::CRGBA	getSpecular () const	{return _Specular;}


	/** setup the attenuation of the light. if (0,0) attenuation is disabled.
	 *	clamp(attenuationBegin,0 , +oo) and calmp(attenuationEnd, attenuationBegin, +oo)
	 */
	void			setupAttenuation(float attenuationBegin, float attenuationEnd);
	/// get the begin radius of the attenuation.
	float			getAttenuationBegin() const {return _AttenuationBegin;}
	/// get the end radius of the attenuation.
	float			getAttenuationEnd() const {return _AttenuationEnd;}


	/** setup the spot AngleBegin and AngleEnd that define spot attenuation of the light. Useful only if SpotLight
	 *	NB: clamp(angleBegin, 0, PI); clamp(angleEnd, angleBegin, PI); Default is PI/4, PI/2
	 */
	void			setupSpotAngle(float spotAngleBegin, float spotAngleEnd);
	/// get the begin radius of the SpotAngles.
	float			getSpotAngleBegin() const {return _SpotAngleBegin;}
	/// get the end radius of the SpotAngles.
	float			getSpotAngleEnd() const {return _SpotAngleEnd;}


	/** setup the spot Direction. Useful only if SpotLight. Normalized internally
	 *	Default is (0, 1, 0)
	 */
	void			setupSpotDirection(const CVector &dir);
	/// get the spot Direction
	const CVector	&getSpotDirection() const {return _SpotDirection;}


	// serial
	void			serial(NLMISC::IStream &f);


	// @}



	/// \name Render tools.
	// @{

	/// Compute a linear attenuation from a point according to attenuation and spot setup. Return [0,1]
	float			computeLinearAttenuation(const CVector &pos) const;

	/** Compute a linear attenuation from a point and precomputed distance according to attenuation and spot setup. Return [0,1]
	 *	\param modelRadius if !0, suppose the point is a sphere, and compute the approximate Max attenuation from every point on
	 *	this sphere
	 */
	float			computeLinearAttenuation(const CVector &pos, float precomputedDist, float modelRadius=0) const;

	/// setup the CLight with current pointLight state. factor is used to modulate the colors.
	void			setupDriverLight(CLight &light, uint8 factor);

	/** setup the CLight with current pointLight state. Don't use driver Attenuation and use software one
	 *	setuped with an additional userAttenuation
	 *	\param factor is used to modulate the colors. Should also integrate light attenuation.
	 */
	void			setupDriverLightUserAttenuation(CLight &light, uint8 factor);

	/// Dirt all models this light influence
	void			resetLightedModels();

	/// append a model to the list. called by CLightingManager.
	ItTransformList	appendLightedModel(CTransform *model);
	/// remove a model from the list. called by CTransform.
	void			removeLightedModel(ItTransformList it);

	// @}

	// Purge static memory
	static void		purge ();

	// Specific For Ambient light
	bool			getAddAmbientWithSun() const {return _AddAmbientWithSun;}
	void			setAddAmbientWithSun(bool state);

// ******************
private:

	// Type of the light
	TType			_Type;

	// The position.
	CVector			_Position;

	// The light color.
	NLMISC::CRGBA	_Ambient;
	NLMISC::CRGBA	_Diffuse;
	NLMISC::CRGBA	_Specular;

	// Attenuation. setup / preComputed.
	float			_AttenuationBegin, _AttenuationEnd;
	float			_OODeltaAttenuation;
	float			_ConstantAttenuation;
	float			_LinearAttenuation;
	float			_QuadraticAttenuation;

	// Spot Setup
	CVector			_SpotDirection;
	float			_SpotAngleBegin;
	float			_SpotAngleEnd;
	float			_CosSpotAngleEnd;
	// 1 / (_CosSpotAngleBegin * _CosSpotAngleEnd)
	float			_OOCosSpotAngleDelta;
	float			_SpotExponent;

	// Ambient specific
	bool			_AddAmbientWithSun;

	// The memory for list of LightedModels
	//static	NLMISC::CBlockMemory<CTransform*, false>		_LightedModelListMemory;
	// LightedModels. NB: do not contains models that have this light in their FrozenStaticLightSetup
	TTransformList		_LightedModels;


	void		computeAttenuationFactors();
	void		computeSpotAttenuationFactors();

};


} // NL3D


#endif // NL_POINT_LIGHT_H

/* End of point_light.h */
