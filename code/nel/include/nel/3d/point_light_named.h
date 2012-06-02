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

#ifndef NL_POINT_LIGHT_NAMED_H
#define NL_POINT_LIGHT_NAMED_H

#include "nel/misc/types_nl.h"
#include "nel/3d/point_light.h"
#include "nel/misc/stream.h"


namespace NL3D {


class	CLightInfluenceInterpolator;


// ***************************************************************************
/**
 * A pointLight with a name, and a default color setup. The current setup is the setup in CPointLight.
 * A CPointLightNamed influence can also be interpolated with a CLightInfluenceInterpolator.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightNamed : public CPointLight
{
public:
	/// Animation used by this light
	std::string		AnimatedLight;

	/// Group of the light
	uint32			LightGroup;

public:

	/// Constructor
	CPointLightNamed();


	/// Set the Default ambient color of the light. Default to Black.
	void			setDefaultAmbient (NLMISC::CRGBA ambient)	{_DefaultAmbient=ambient;}
	/// Set the Default diffuse color of the light. Default to White
	void			setDefaultDiffuse (NLMISC::CRGBA diffuse)	{_DefaultDiffuse=diffuse;}
	/// Set the Default specular color of the light. Default to White
	void			setDefaultSpecular (NLMISC::CRGBA specular)	{_DefaultSpecular=specular;}

	/// Get the Default ambient color of the light.
	NLMISC::CRGBA	getDefaultAmbient () const	{return _DefaultAmbient;}
	/// Get the Default diffuse color of the light.
	NLMISC::CRGBA	getDefaultDiffuse () const	{return _DefaultDiffuse;}
	/// Get the Default specular color of the light.
	NLMISC::CRGBA	getDefaultSpecular () const	{return _DefaultSpecular;}

	/// get the unanimated diffuse (used for Landscape)
	NLMISC::CRGBA	getUnAnimatedDiffuse() const {return _UnAnimatedDiffuse;}

	/** modulate the default color setup with nFactor, and set to the current setup.
	 *	NB: getUnAnimatedDiffuse() == getDiffuse()
	 */
	void			setLightFactor(NLMISC::CRGBA nFactor);

	/** modulate the default color setup with nFactor, and set to the current setup.
	 *	give 2 params the second is the "not animated" factor. used for Landscape.
	 */
	void			setLightFactor(NLMISC::CRGBA nAnimatedFactor, NLMISC::CRGBA nUnAnimatedFactor);


	void			serial(NLMISC::IStream &f);


// ******************
private:

	// For CLightInfluenceInterpolator to work. Only used by CLightInfluenceInterpolator.
	friend class		CLightInfluenceInterpolator;
	sint				_IdInInfluenceList;

	// The Default light color.
	NLMISC::CRGBA		_DefaultAmbient;
	NLMISC::CRGBA		_DefaultDiffuse;
	NLMISC::CRGBA		_DefaultSpecular;

	// The unanimated diffuse
	NLMISC::CRGBA		_UnAnimatedDiffuse;

};


} // NL3D


#endif // NL_POINT_LIGHT_NAMED_H

/* End of point_light_named.h */
