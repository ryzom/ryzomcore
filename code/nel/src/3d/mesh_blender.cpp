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

#include "nel/3d/mesh_blender.h"
#include "nel/3d/driver.h"
#include "nel/misc/fast_floor.h"


namespace NL3D {


// ***************************************************************************
void		CMeshBlender::prepareRenderForGlobalAlpha(CMaterial &material, IDriver *drv, float globalAlpha, uint8 globalAlphaInt, bool gaDisableZWrite)
{
	// Disable ZWrite??
	if(gaDisableZWrite)
	{
		// Backup and set new the zwrite
		_BkZWrite=material.getZWrite ();
		material.setZWrite (false);
	}

	// Backup blend
	_BkBlend=material.getBlend ();
	material.setBlend (true);

	// Backup opacity and blend
	_BkOpacity= material.getOpacity ();
	_BkSrcBlend= material.getSrcBlend();
	_BkDstBlend= material.getDstBlend();

	// 2 ways: if Blend Constant Color is supported or not.
	if(drv->supportBlendConstantColor())
	{
		// Set opacity to 255 => If AlphaTest is used, AlphaTexture will be multiplied by 1.
		material.setOpacity ( 255 );

		// set the Blend constant Alpha.
		drv->setBlendConstantColor( CRGBA(0,0,0, globalAlphaInt) );

		// Active Blend with Blend Constant Alpha .
		material.setSrcBlend(CMaterial::blendConstantAlpha);

		// Don't set dest if we are in additive blend mode
		if ((_BkBlend == false) || (_BkDstBlend != CMaterial::one))
			material.setDstBlend(CMaterial::blendConstantInvAlpha);

		// if material is Alpha Test, no-op. Keep same threshold, since Use Alpha of the BlendConstantColor
		// to do the alpha-blending.
	}
	// not supported, ugly way: modify Opacity, and alpha threshold
	else
	{
		// New opacity
		material.setOpacity (globalAlphaInt);

		// must ensure Std Blend.
		material.setSrcBlend(CMaterial::srcalpha);

		// Don't set dest if we are in additive blend mode
		if ((_BkBlend == false) || (_BkDstBlend != CMaterial::one))
			material.setDstBlend(CMaterial::invsrcalpha);

		// if material is Alpha Test, must modulate AlphaTest limit to avoid Pop effects
		if(material.getAlphaTest())
		{
			_BkAlphaTestThreshold= material.getAlphaTestThreshold();
			material.setAlphaTestThreshold(_BkAlphaTestThreshold * globalAlpha);
		}
	}

}

// ***************************************************************************
void		CMeshBlender::restoreRender(CMaterial &material, IDriver *drv, bool gaDisableZWrite)
{
	// Resetup backuped zwrite
	if(gaDisableZWrite)
	{
		material.setZWrite (_BkZWrite);
	}

	// Resetup backuped blend
	material.setBlend (_BkBlend);

	// Resetup backuped opacity and blend factors
	material.setOpacity (_BkOpacity);
	material.setSrcBlend(_BkSrcBlend);
	material.setDstBlend(_BkDstBlend);

	// 2 ways: if Blend Constant Color is supported or not.
	if(drv->supportBlendConstantColor())
	{
		// nop
	}
	else
	{
		// Resetup backuped AlphaTest threshold
		if(material.getAlphaTest())
		{
			material.setAlphaTestThreshold(_BkAlphaTestThreshold);
		}
	}
}


// ***************************************************************************
void		CMeshBlender::prepareRenderForGlobalAlphaCoarseMesh(CMaterial &material, IDriver *drv, NLMISC::CRGBA color, float globalAlpha, bool gaDisableZWrite)
{
	// Don't need to bkup some values, because CoarseMesh.

	uint8	globalAlphaInt= (uint8)NLMISC::OptFastFloor(255 * globalAlpha);

	// Disable ZWrite??
	if(gaDisableZWrite)
		material.setZWrite (false);
	// Enable blend
	material.setBlend (true);

	// bkup color and blend factors.
	_BkupColor= material.getColor();
	_BkSrcBlend= material.getSrcBlend();
	_BkDstBlend= material.getDstBlend();

	// 2 ways: if Blend Constant Color is supported or not.
	if(drv->supportBlendConstantColor())
	{
		// Set opacity to 255 => AlphaTexture will be multiplied by 1.
		color.A= 255;
		// change color
		material.setColor ( color );

		// set the Blend constant Alpha.
		drv->setBlendConstantColor( CRGBA(0,0,0, globalAlphaInt) );

		// Active Blend with Blend Constant Alpha .
		material.setSrcBlend(CMaterial::blendConstantAlpha);
		material.setDstBlend(CMaterial::blendConstantInvAlpha);
	}
	else
	{
		// Set current Alpha blend transparency (in color becausematerial is unlit)
		color.A= globalAlphaInt;
		// change color
		material.setColor ( color );

		// Active Blend with Blend Constant Alpha .
		material.setSrcBlend(CMaterial::srcalpha);
		material.setDstBlend(CMaterial::invsrcalpha);

		// must modulate AlphaTest limit to avoid Pop effects
		material.setAlphaTestThreshold(0.5f * globalAlpha);
	}
}

// ***************************************************************************
void		CMeshBlender::restoreRenderCoarseMesh(CMaterial &material, IDriver *drv, bool gaDisableZWrite)
{
	// Resetup backuped color and blend factors
	material.setColor ( _BkupColor );
	material.setSrcBlend(_BkSrcBlend);
	material.setDstBlend(_BkDstBlend);

	// ReEnable ZWrite??
	if(gaDisableZWrite)
		material.setZWrite (true);
	// Reset blend
	material.setBlend (false);

	// 2 ways: if Blend Constant Color is supported or not.
	if(drv->supportBlendConstantColor())
	{
		// nop
	}
	else
	{
		// reset AlphaTest limit
		material.setAlphaTestThreshold(0.5f);
	}
}

} // NL3D
