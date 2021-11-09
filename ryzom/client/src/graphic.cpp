// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"


//#include "nel/3d/material_user.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/debug.h"
#include "graphic.h"


// ***************************************************************************
void drawBitmapTiled ( NL3D::UDriver * /* driver */, float /* x */, float /* y */, float /* width */, float /* height */, uint32 /* windowWidth */, uint32 /* windowHeight */, NL3D::UTextureFile & /* texture */, uint32 /* textureWidth */, uint32 /* textureHeight */, bool /* blend */, NLMISC::CRGBA /* col */)
{

// Sorry for this. (Hulud) :)
/*	nlassert( driver );

	NL3D::CMaterialUser	material;
	material.setTexture(&texture);
	material.setColor(col);
	material.setBlend(blend);

	NLMISC::CQuadUV		quad;

	quad.V0.set(x,y,0);
	quad.V1.set(x+width,y,0);
	quad.V2.set(x+width,y+height,0);
	quad.V3.set(x,y+height,0);

	if ( textureWidth !=0 && textureHeight != 0 )
	{
		// compute values (control size in pixels / texture size in pixels)
		const float controlW = windowWidth * width;
		const float controlH = windowHeight * height;

		if (controlH > textureHeight)
		{
			quad.Uv0.U= 0.f;
			quad.Uv0.V= controlH / textureHeight + 1.0f;

			quad.Uv1.U= controlW / textureWidth;
			quad.Uv1.V= quad.Uv0.V;

			quad.Uv2.U= quad.Uv1.U ;
			quad.Uv2.V= quad.Uv0.V - float( (int)(quad.Uv0.V) );

			quad.Uv3.U= 0.f;
			quad.Uv3.V= quad.Uv2.V;
		}
		else
		{
			quad.Uv0.U= 0.0f;
			quad.Uv0.V= 1.0f;

			quad.Uv1.U= controlW / textureWidth;
			quad.Uv1.V= quad.Uv0.V;

			quad.Uv2.U= quad.Uv1.U ;
			quad.Uv2.V= 1.0f - controlH / textureHeight ;

			quad.Uv3.U= 0.f;
			quad.Uv3.V= quad.Uv2.V;
		}
	}
	else
	{
		quad.Uv0.U= 0.f;
		quad.Uv0.V= 1.f;
		quad.Uv1.U= 1.f;
		quad.Uv1.V= 1.f;
		quad.Uv2.U= 1.f;
		quad.Uv2.V= 0.f;
		quad.Uv3.U= 0.f;
		quad.Uv3.V= 0.f;
	}

	driver->drawQuad(quad, material);*/
}
