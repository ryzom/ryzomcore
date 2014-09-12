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

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/config_file.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/stereo_hmd.h>

#include "mouse_listener.h"
#include "camera.h"
#include "snowballs_client.h"
#include "entities.h"

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;

namespace SBCLIENT {

//
// Variables
//

static NL3D::UMaterial CompassMaterial = NULL;

// These variables are automatically set with the config file

static	float	CompassPosX, CompassPosY, CompassRadius;
static	CRGBA	CompassColor;

//
// Functions
//

void cbUpdateCompass (CConfigFile::CVar &var)
{
	if (var.Name == "CompassPosX") CompassPosX = var.asFloat ();
	else if (var.Name == "CompassPosY") CompassPosY = var.asFloat ();
	else if (var.Name == "CompassRadius") CompassRadius = var.asFloat ();
	else if (var.Name == "CompassColor") 
	{
		CompassColor.set(var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
		CompassMaterial.setColor(CompassColor);
	}
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initCompass ()
{
	CompassMaterial = Driver->createMaterial ();
	CompassMaterial.initUnlit ();
	CompassMaterial.setBlendFunc (UMaterial::srcalpha, UMaterial::invsrcalpha);
	CompassMaterial.setBlend(true);

	ConfigFile->setCallback ("CompassPosX", cbUpdateCompass);
	ConfigFile->setCallback ("CompassPosY", cbUpdateCompass);
	ConfigFile->setCallback ("CompassRadius", cbUpdateCompass);
	ConfigFile->setCallback ("CompassColor", cbUpdateCompass);

	cbUpdateCompass (ConfigFile->getVar ("CompassPosX"));
	cbUpdateCompass (ConfigFile->getVar ("CompassPosY"));
	cbUpdateCompass (ConfigFile->getVar ("CompassRadius"));
	cbUpdateCompass (ConfigFile->getVar ("CompassColor"));
}

void updateCompass ()
{
	float x = CompassPosX;
	float y = CompassPosY;
	float radius = CompassRadius;

	// tri
	CTriangle tri;
	tri.V0.set (-radius, 0, 0);
	tri.V1.set (radius, 0, 0);
	tri.V2.set (0, 3*radius, 0);

	CQuad quad;
	// quad
	quad.V0.set (-radius, -radius, 0);
	quad.V1.set ( radius, -radius, 0);
	quad.V2.set ( radius,  radius, 0);
	quad.V3.set (-radius,  radius, 0);

	Driver->setMatrixMode2D43();

	CMatrix mtx;

	// up
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateZ(MouseListener->getOrientation() - (float)Pi/2);
	mtx.translate(CVector(0,radius,0));
	Driver->setModelMatrix (mtx);
	Driver->drawTriangle (tri,  CompassMaterial);

	// down
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateZ(MouseListener->getOrientation() + (float)Pi/2);
	mtx.translate(CVector(0,radius,0));
	Driver->setModelMatrix (mtx);
	Driver->drawTriangle (tri,  CompassMaterial);

	// left
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateZ(MouseListener->getOrientation());
	mtx.translate(CVector(0,radius,0));
	Driver->setModelMatrix (mtx);
	Driver->drawTriangle (tri,  CompassMaterial);

	// right
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateZ(MouseListener->getOrientation() - (float)Pi);
	mtx.translate(CVector(0,radius,0));
	Driver->setModelMatrix (mtx);
	Driver->drawTriangle (tri,  CompassMaterial);

	// center
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateZ(MouseListener->getOrientation());
	Driver->setModelMatrix (mtx);
	Driver->drawQuad (quad,  CompassMaterial);

	x *= 3.0/4.0f;

	// Print position
	TextContext->setHotSpot(UTextContext::MiddleTop);
	TextContext->setColor(CompassColor);
	TextContext->setFontSize(14);
	if (Self != NULL)
		TextContext->printfAt(x, y-4.0f*radius, "%.2f %.2f %.2f", Self->Position.x, Self->Position.y, Self->Position.z);
	else
		TextContext->printfAt(x, y-4.0f*radius, "%.2f %.2f %.2f", MouseListener->getPosition().x, MouseListener->getPosition().y, MouseListener->getPosition().z);
}

void releaseCompass ()
{
	ConfigFile->setCallback("CompassPosX", NULL);
	ConfigFile->setCallback("CompassPosY", NULL);
	ConfigFile->setCallback("CompassRadius", NULL);
	ConfigFile->setCallback("CompassColor", NULL);

	Driver->deleteMaterial (CompassMaterial);
	CompassMaterial = NULL;
}

} /* namespace SBCLIENT */

/* end of file */
