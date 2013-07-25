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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/light.h"

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************
uint	CDriverGL::getMaxLight () const
{
	H_AUTO_OGL(CDriverGL_getMaxLight )
	// return min(maxLight supported by openGL, MaxLight=8).
	return _MaxDriverLight;
}


// ***************************************************************************
void	CDriverGL::setLight (uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL_setLight )
	// bkup real light, for lightmap dynamic lighting purpose
	if(num==0)
	{
		_UserLight0= light;
		// because the GL setup change, must dirt lightmap rendering
		_LightMapDynamicLightDirty= true;
	}

	setLightInternal(num, light);
}


// ***************************************************************************
void	CDriverGL::setLightInternal(uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL_setLightInternal)
	// Check light count is good
//	nlassert (num<_MaxDriverLight);

	// Set the light
	if (num<_MaxDriverLight)
	{
		// GL light number
		GLenum lightNum=(GLenum)(GL_LIGHT0+num);

		// Get light mode
		CLight::TLightMode mode=light.getMode ();

		// Copy the mode
		_LightMode[num]=mode;

		// Set the ambiant color
		GLfloat colorGL[4];
		CRGBA colorNeL=light.getAmbiant ();
		colorGL[0]=(float)colorNeL.R/255.f;
		colorGL[1]=(float)colorNeL.G/255.f;
		colorGL[2]=(float)colorNeL.B/255.f;
		colorGL[3]=1.f;
		glLightfv (lightNum, GL_AMBIENT, colorGL);

		// Set the diffuse color
		colorNeL=light.getDiffuse ();
		colorGL[0]=(float)colorNeL.R/255.f;
		colorGL[1]=(float)colorNeL.G/255.f;
		colorGL[2]=(float)colorNeL.B/255.f;
		colorGL[3]=1.f;
		glLightfv (lightNum, GL_DIFFUSE, colorGL);

		// Set the specular color
		colorNeL=light.getSpecular ();
		// don't know why, but with ATI cards, specular of 0 causes incorrect rendering (random specular is added)
		if (_Extensions.ATITextureEnvCombine3)
		{
			// special case for ATI (there will be some specular, but there's a bug otherwise)
			colorGL[0]=std::max(1.f / 1024.f, (float)colorNeL.R/255.f);
			colorGL[1]=std::max(1.f / 1024.f, (float)colorNeL.G/255.f);
			colorGL[2]=std::max(1.f / 1024.f, (float)colorNeL.B/255.f);
			colorGL[3]=1.f;
		}
		else
		{
			colorGL[0]=(float)colorNeL.R/255.f;
			colorGL[1]=(float)colorNeL.G/255.f;
			colorGL[2]=(float)colorNeL.B/255.f;
			colorGL[3]=1.f;
		}
		glLightfv (lightNum, GL_SPECULAR, colorGL);

		// Set light attenuation
		glLightf (lightNum, GL_CONSTANT_ATTENUATION, light.getConstantAttenuation());
		glLightf (lightNum, GL_LINEAR_ATTENUATION, light.getLinearAttenuation());
		glLightf (lightNum, GL_QUADRATIC_ATTENUATION, light.getQuadraticAttenuation());

		// Set the position
		if ((mode==CLight::DirectionalLight)||(mode==CLight::SpotLight))
		{
			// Get the direction of the light
			_WorldLightDirection[num]=light.getDirection ();
		}

		if (mode!=CLight::DirectionalLight)
		{
			// Get the position of the light
			_WorldLightPos[num]=light.getPosition ();
		}

		if (mode==CLight::SpotLight)
		{
			// Get the exponent of the spot
			float exponent=light.getExponent ();

			// Set it
			glLightf (lightNum, GL_SPOT_EXPONENT, exponent);

			// Get the cutoff of the spot
			float cutoff=180.f*(light.getCutoff ()/(float)NLMISC::Pi);

			// Set it
			glLightf (lightNum, GL_SPOT_CUTOFF, cutoff);
		}
		else
		{
			// Disable spot properties
#ifdef USE_OPENGLES
			glLightf (lightNum, GL_SPOT_CUTOFF, 180.f);
			glLightf (lightNum, GL_SPOT_EXPONENT, 0.f);
#else
			glLighti (lightNum, GL_SPOT_CUTOFF, 180);
			glLighti (lightNum, GL_SPOT_EXPONENT, 0);
#endif
		}

		// Flag this light as dirt.
		_LightDirty[num]= true;

		// dirt the lightSetup and hence the render setup
		_LightSetupDirty= true;
		_RenderSetupDirty=true;
	}
}

// ***************************************************************************
void	CDriverGL::enableLight (uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL_enableLight )
	// User call => set the User flag
	if(num<_MaxDriverLight)
	{
		_UserLightEnable[num]= enable;
	}

	// enable the light in GL
	enableLightInternal(num, enable);

	// because the GL setup has changed, must dirt lightmap rendering
	_LightMapDynamicLightDirty= true;
}


// ***************************************************************************
void	CDriverGL::enableLightInternal(uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL_enableLightInternal)
	// Check light count is good
	//	nlassert (num<_MaxDriverLight);

	// Enable glLight
	if (num<_MaxDriverLight)
	{
		_DriverGLStates.enableLight(num, enable);

		// If this light is dirty, and reenabled, then it must be refresh at next render => set the global flag.
		if (enable && _LightDirty[num])
		{
			_LightSetupDirty= true;
			_RenderSetupDirty= true;
		}
	}
}


// ***************************************************************************

void	CDriverGL::setAmbientColor (CRGBA color)
{
	H_AUTO_OGL(CDriverGL_setAmbientColor )
	// Gl array
	GLfloat array[4];
	array[0]=(float)color.R/255.f;
	array[1]=(float)color.G/255.f;
	array[2]=(float)color.B/255.f;
	array[3]=1.f;

	// Set the color
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, array);
}


// ***************************************************************************
void				CDriverGL::cleanLightSetup ()
{
	H_AUTO_OGL(CDriverGL_cleanLightSetup )
	// Should be dirty
	nlassert (_LightSetupDirty);

	// First light
	bool first=true;

	// For each lights
	for (uint i=0; i<_MaxDriverLight; i++)
	{
		// Is this light enabled and dirty?
		if (_DriverGLStates.isLightEnabled(i) && _LightDirty[i])
		{
			// If first light
			if (first)
			{
				first=false;

				// Push the matrix
				glPushMatrix ();

				// Load the view matrix
				glLoadMatrixf (_ViewMtx.get());
			}

			// Light is directionnal ?
			if (_LightMode[i]==(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=-_WorldLightDirection[i].x;
				vectorGL[1]=-_WorldLightDirection[i].y;
				vectorGL[2]=-_WorldLightDirection[i].z;
				vectorGL[3]=0.f;

				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_POSITION, vectorGL);
			}

			// Spotlight ?
			if (_LightMode[i]==(uint)CLight::SpotLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				vectorGL[0]=_WorldLightDirection[i].x;
				vectorGL[1]=_WorldLightDirection[i].y;
				vectorGL[2]=_WorldLightDirection[i].z;

				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_SPOT_DIRECTION, vectorGL);
			}

			// Position
			if (_LightMode[i]!=(uint)CLight::DirectionalLight)
			{
				// GL vector
				GLfloat vectorGL[4];

				// Set the GL array
				// Must Substract CameraPos, because ViewMtx may not be the exact view.
				vectorGL[0]=_WorldLightPos[i].x - _PZBCameraPos.x;
				vectorGL[1]=_WorldLightPos[i].y - _PZBCameraPos.y;
				vectorGL[2]=_WorldLightPos[i].z - _PZBCameraPos.z;
				vectorGL[3]=1.f;

				// Set it
				glLightfv ((GLenum)(GL_LIGHT0+i), (GLenum)GL_POSITION, vectorGL);
			}

			// Cleaned!
			_LightDirty[i]= false;
		}
	}

	// Pop old matrix
	if (!first)
		glPopMatrix ();

	// Clean flag
	_LightSetupDirty=false;
}


// ***************************************************************************
void			CDriverGL::setLightMapDynamicLight (bool enable, const CLight& light)
{
	H_AUTO_OGL(CDriverGL_setLightMapDynamicLight )
	// just store, for future setup in lightmap material rendering
	_LightMapDynamicLightEnabled= enable;
	_LightMapDynamicLight= light;
	_LightMapDynamicLightDirty= true;
}


// ***************************************************************************
void			CDriverGL::setupLightMapDynamicLighting(bool enable)
{
	H_AUTO_OGL(CDriverGL_setupLightMapDynamicLighting)
	// start lightmap dynamic lighting
	if(enable)
	{
		// disable all lights but the 0th.
		for(uint i=1;i<_MaxDriverLight;i++)
			enableLightInternal(i, false);

		// if the dynamic light is really enabled
		if(_LightMapDynamicLightEnabled)
		{
			// then setup and enable
			setLightInternal(0, _LightMapDynamicLight);
			enableLightInternal(0, true);
		}
		// else just disable also the light 0
		else
		{
			enableLightInternal(0, false);
		}

		// ok it has been setup
		_LightMapDynamicLightDirty= false;
	}
	// restore old lighting
	else
	{
		// restore the light 0
		setLightInternal(0, _UserLight0);

		// restore all standard light enable states
		for(uint i=0;i<_MaxDriverLight;i++)
			enableLightInternal(i, _UserLightEnable[i]);
	}

	// in all case, must refresh render setup, cause lighting may be modified
	refreshRenderSetup();
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
