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

#include <vector>

#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/command.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>

#include "camera.h"
#include "snowballs_client.h"
#include "commands.h"
#include "mouse_listener.h"
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

// These variables are automatically set with the config file

static	float	RadarPosX, RadarPosY, RadarWidth, RadarHeight;
static	uint	RadarMinDistance, RadarMaxDistance;
static	CRGBA	RadarBackColor, RadarFrontColor, RadarSelfColor, RadarOtherColor, RadarPlaceColor;
static	float	RadarEntitySize;
static	uint	RadarFontSize;
static	float	RadarLittlePosX, RadarLittlePosY, RadarLittleRadius;
static NL3D::UMaterial RadarMaterial = NULL;

uint RadarState;
uint RadarDistance;

//
// Functions
//

// Structure for particular places
struct RadarParticularPlace
{
	RadarParticularPlace (float X, float Y, string Name) : x(X), y(Y), name(Name) { }
	float x, y;
	string name;
};

// Container for the particular places (automatically filled by the config file)
vector<RadarParticularPlace> RadarParticularPlaces;


/*********************************************************\
					displayRadar()
\*********************************************************/
void displayRadar ()
{
	float xLeft = RadarPosX;
	float xRight = RadarPosX + RadarWidth;
	float yTop = RadarPosY + RadarHeight;
	float yBottom = RadarPosY;
	float xCenter = RadarPosX + RadarWidth/2.0f;
	float yCenter = RadarPosY + RadarHeight/2.0f;

	Driver->setMatrixMode2D11 ();

	// Background
	RadarMaterial.setColor(RadarBackColor);
	CQuad quad;
	quad.V0.set(xLeft,yBottom,0);
	quad.V1.set(xRight,yBottom,0);
	quad.V2.set(xRight,yTop,0);
	quad.V3.set(xLeft,yTop,0);
	Driver->drawQuad (quad, RadarMaterial);

	// Print radar's range
	TextContext->setHotSpot(UTextContext::TopRight);
	TextContext->setColor(RadarFrontColor);
	TextContext->setFontSize(RadarFontSize);
	TextContext->printfAt(xRight-0.01f,yTop-0.01f,"%d m",RadarDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = 0.8f*stepV/RadarDistance;
	stepH = 0.8f*stepH/RadarDistance;

	// Drawing radar's lines
	float gapV = stepV/2;
	float gapH = stepH/2;
	
	CLine line;
	RadarMaterial.setColor(RadarFrontColor);
	while(gapH<=RadarWidth/2.0)
	{
		// v lines
	        line.V0.set(xCenter+gapH, yTop, 0);
	        line.V1.set(xCenter+gapH, yBottom, 0);
	        Driver->drawLine(line, RadarMaterial);
		//Driver->drawLine (xCenter+gapH,yTop,xCenter+gapH,yBottom,RadarFrontColor);
		line.V0.set(xCenter-gapH, yTop, 0);
		line.V1.set(xCenter-gapH, yBottom, 0);
		Driver->drawLine(line, RadarMaterial);
		//Driver->drawLine (xCenter-gapH,yTop,xCenter-gapH,yBottom,RadarFrontColor);

		gapH += stepH;
	}

	while(gapV<=RadarHeight/2.0)
	{
		// h lines
	        line.V0.set(xLeft, yCenter+gapV, 0);
	        line.V1.set(xRight, yCenter+gapV, 0);
	        Driver->drawLine(line, RadarMaterial);
		//Driver->drawLine (xLeft,yCenter+gapV,xRight,yCenter+gapV,RadarFrontColor);
		line.V0.set(xLeft, yCenter-gapV, 0);
		line.V1.set(xRight, yCenter-gapV, 0);
		Driver->drawLine(line, RadarMaterial);
		//Driver->drawLine (xLeft,yCenter-gapV,xRight,yCenter-gapV,RadarFrontColor);

		gapV += stepV;
	}

	float scale = 1.0f;

	float xscreen = xCenter;
	float yscreen = yCenter;

	Driver->setFrustum (CFrustum(0.f, 1.0f, 0.f, 1.f, -1.f, 1.f, false));
	
	// distance between user and neighbour player
	float myPosx = Self->Position.x;
	float myPosy = Self->Position.y;

	// Quads size
	float radius = RadarEntitySize;

	// Arrow in center (user)
	RadarMaterial.setColor(RadarSelfColor);
	CTriangle triangle;
	triangle.V0.set(xscreen-2*radius, yscreen-2*radius, 0);
	triangle.V1.set(xscreen, yscreen-radius, 0);
	triangle.V2.set(xscreen, yscreen+2*radius, 0);
	Driver->drawTriangle(triangle, RadarMaterial);
	//Driver->drawTriangle(xscreen-2*radius,yscreen-2*radius, xscreen,yscreen-radius, xscreen,yscreen+2*radius, RadarSelfColor);
	triangle.V0.set(xscreen, yscreen-radius, 0);
	triangle.V1.set(xscreen+2*radius, yscreen-2*radius, 0);
	triangle.V2.set(xscreen, yscreen+2*radius, 0);
	Driver->drawTriangle(triangle, RadarMaterial);
	//Driver->drawTriangle(xscreen,yscreen-radius, xscreen+2*radius,yscreen-2*radius, xscreen,yscreen+2*radius, RadarSelfColor);

	TextContext->setColor(RadarOtherColor);
	RadarMaterial.setColor(RadarOtherColor);

	for(EIT eit=Entities.begin(); eit!=Entities.end(); eit++)
	{
		if((*eit).second.Type == CEntity::Other)
		{
			CVector playerPos = (*eit).second.Position;

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*0.4f/RadarDistance;
			posy = (posy-myPosy)*0.4f/RadarDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = MouseListener->getOrientation ();
			if(posx==0)
			{
				if(posy==0)
				{
					x = xscreen;
					y = yscreen;
				}
				else
				{
					if(posy>0)
					{
						x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(Pi-az));
					}
					else
					{
						x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(-az));
					}
				}
			}
			else
			{
				an = (float) atan(posy/posx);
				if(posx<0) an = an + (float)Pi;
				x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
				y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
			}

			// Players out of range are not displayed
			if(x<xLeft || x>xRight || y>yTop || y<yBottom) continue;

			TextContext->setColor(RadarOtherColor);

			quad.V0.set(x-radius, y+radius, 0);
			quad.V1.set(x+radius, y+radius, 0);
			quad.V2.set(x+radius, y-radius, 0);
			quad.V3.set(x-radius, y-radius, 0);
			Driver->drawQuad(quad, RadarMaterial);
			//Driver->drawQuad (x-radius,y-radius, x+radius,y+radius,RadarOtherColor);

			// Print his name
			TextContext->setFontSize(RadarFontSize);
			if(x>=xCenter)
			{
				if(y>=yCenter)
				{
					TextContext->setHotSpot(UTextContext::BottomLeft);
					TextContext->printfAt(x+2*radius, y+2*radius, (*eit).second.Name.c_str());
				}
				else
				{
					TextContext->setHotSpot(UTextContext::TopLeft);
					TextContext->printfAt(x+2*radius, y-2*radius, (*eit).second.Name.c_str());
				}
			}
			else
			{
				if(y>=yCenter)
				{
					TextContext->setHotSpot(UTextContext::BottomRight);
					TextContext->printfAt(x-2*radius, y+2*radius, (*eit).second.Name.c_str());
				}
				else
				{
					TextContext->setHotSpot(UTextContext::TopRight);
					TextContext->printfAt(x-2*radius, y-2*radius, (*eit).second.Name.c_str());
				}
			}
		}
	}

	// display particular places
	RadarMaterial.setColor(RadarPlaceColor);
	for(uint i = 0; i < RadarParticularPlaces.size(); i++)
	{
		// relative position
		float posx = (RadarParticularPlaces[i].x-myPosx)*0.4f/RadarDistance;
		float posy = (RadarParticularPlaces[i].y-myPosy)*0.4f/RadarDistance;
		
		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = MouseListener->getOrientation ();
		if(posx==0)
		{
			if(posy==0)
			{
				x = xscreen;
				y = yscreen;
			}
			else
			{
				if(posy>0)
				{
					x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(Pi-az));
				}
				else
				{
					x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(-az));
				}
			}
		}
		else
		{
			an = (float) atan(posy/posx);
			if(posx<0) an = an + (float)Pi;
			x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
			y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
		}


		if(x<xLeft || x>xRight || y>yTop || y<yBottom) 
		{
			continue;
		}

		triangle.V0.set(x-radius, y-radius, 0);
		triangle.V1.set(x+radius, y-radius, 0);
		triangle.V2.set(x, y+radius, 0);
		Driver->drawTriangle(triangle, RadarMaterial);
		//Driver->drawTriangle(x-radius,y-radius, x+radius,y-radius, x,y+radius, RadarPlaceColor);

		TextContext->setFontSize(RadarFontSize);
		TextContext->setColor(RadarPlaceColor);

		if(x>=xCenter)
		{
			if(y>=yCenter)
			{
				TextContext->setHotSpot(UTextContext::BottomLeft);
				TextContext->printfAt(x+2*radius, y+2*radius, RadarParticularPlaces[i].name.c_str());
			}
			else
			{
				TextContext->setHotSpot(UTextContext::TopLeft);
				TextContext->printfAt(x+2*radius, y-2*radius, RadarParticularPlaces[i].name.c_str());
			}
		}
		else
		{
			if(y>=yCenter)
			{
				TextContext->setHotSpot(UTextContext::BottomRight);
				TextContext->printfAt(x-2*radius, y+2*radius, RadarParticularPlaces[i].name.c_str());
			}
			else
			{
				TextContext->setHotSpot(UTextContext::TopRight);
				TextContext->printfAt(x-2*radius, y-2*radius, RadarParticularPlaces[i].name.c_str());
			}
		}
	}
}

/*********************************************************\
					displayLittleRadar()
\*********************************************************/
void displayLittleRadar()
{
	float radius = RadarLittleRadius;

	float xLeft = RadarLittlePosX - radius*3.f/4.f;
	float yBottom = RadarLittlePosY - radius;

	float xRight = RadarLittlePosX + radius*3.f/4.f;
	float yTop = RadarLittlePosY + radius;

	Driver->setMatrixMode2D11 ();
	
	// Background
	RadarMaterial.setColor(RadarBackColor);
	CQuad quad;
	quad.V0.set(xLeft,yBottom,0);
	quad.V1.set(xRight,yBottom,0);
	quad.V2.set(xRight,yTop,0);
	quad.V3.set(xLeft,yTop,0);

	//Driver->drawQuad (xLeft,yBottom,xRight,yTop,RadarBackColor);
	Driver->drawQuad(quad, RadarMaterial);

	// Print radar's range
	TextContext->setHotSpot(UTextContext::MiddleBottom);
	TextContext->setColor(RadarFrontColor);
	TextContext->setFontSize(RadarFontSize);
	TextContext->printfAt(xLeft+radius*3.f/4.f,yTop+0.01f,"%d m",RadarDistance);
		
	// Radar unit
	float stepV = 50.0f;
	float stepH = stepV*3.f/4.f;
	// Changing scale
	stepV = radius*stepV/RadarDistance;
	stepH = radius*stepH/RadarDistance;

	// Drawing radar's lines
	// h lines
	RadarMaterial.setColor(RadarFrontColor);
	CLine line;
	line.V0.set(xLeft, yTop, 0);
	line.V1.set(xRight, yTop, 0);
	Driver->drawLine(line, RadarMaterial);
	line.V0.set(xLeft, yBottom+radius, 0);
	line.V1.set(xRight, yBottom+radius, 0);
	Driver->drawLine(line, RadarMaterial);
	line.V0.set(xLeft, yBottom, 0);
	line.V1.set(xRight, yBottom, 0);
	Driver->drawLine(line, RadarMaterial);
	//Driver->drawLine (xLeft, yTop,         xRight, yTop,          RadarFrontColor);
	//Driver->drawLine (xLeft, yBottom+radius,xRight, yBottom+radius, RadarFrontColor);
	//Driver->drawLine (xLeft, yBottom,      xRight, yBottom,       RadarFrontColor);
	
	// v lines (assuming 4:3 screen)
	line.V0.set(xLeft, yTop, 0);
	line.V1.set(xLeft, yBottom, 0);
	Driver->drawLine(line, RadarMaterial);
	line.V0.set(xLeft + radius*0.75f, yTop, 0);
	line.V1.set(xLeft + radius*0.75f, yBottom, 0);
	Driver->drawLine(line, RadarMaterial);
	line.V0.set(xRight, yTop, 0);
	line.V1.set(xRight, yBottom, 0);
	Driver->drawLine(line, RadarMaterial);
	//Driver->drawLine (xLeft,               yTop, xLeft,               yBottom, RadarFrontColor);
	//Driver->drawLine (xLeft+radius*3.f/4.f, yTop, xLeft+radius*3.f/4.f, yBottom, RadarFrontColor);
	//Driver->drawLine (xRight,              yTop, xRight,              yBottom, RadarFrontColor);

	float scale = 1.0f;

	float xscreen = xLeft + radius*3.f/4.f;
	float yscreen = yBottom + radius;

	Driver->setFrustum (CFrustum(0.f, 1.0f, 0.f, 1.f, -1.f, 1.f, false));
	
	// distance between user and neighbour player
	float myPosx = Self->Position.x;
	float myPosy = Self->Position.y;

	// Quads size
	float entitySize = RadarEntitySize/2.0f;

	TextContext->setColor(RadarOtherColor);

	for(EIT eit=Entities.begin(); eit!=Entities.end(); eit++)
	{
		if((*eit).second.Type == CEntity::Other)
		{
			CVector playerPos = (*eit).second.Position;

			// position of neighbour
			float posx = playerPos.x;
			float posy = playerPos.y;

			// relative position
			posx = (posx-myPosx)*radius/RadarDistance;
			posy = (posy-myPosy)*radius/RadarDistance;

			float dist = (float) sqrt((posx*posx)+(posy*posy));

			// Display a quad to show a player
			float an;
			float az;
			float x;
			float y;
			az = MouseListener->getOrientation ();
			if(posx==0)
			{
				if(posy==0)
				{
					x = xscreen;
					y = yscreen;
				}
				else
				{
					if(posy>0)
					{
						x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(Pi-az));
					}
					else
					{
						x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
						y = (float) (yscreen - dist*sin(-az));
					}
				}
			}
			else
			{
				an = (float) atan(posy/posx);
				if(posx<0) an = an + (float)Pi;
				x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
				y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
			}

			// Players out of range are not displayed
			if(x<xLeft || x>xRight || y>yTop || y<yBottom) continue;

			TextContext->setColor(RadarOtherColor);

			quad.V0.set(x-entitySize, y+entitySize, 0);
			quad.V1.set(x+entitySize, y+entitySize, 0);
			quad.V2.set(x+entitySize, y-entitySize, 0);
			quad.V3.set(x-entitySize, y-entitySize, 0);
			RadarMaterial.setColor(RadarOtherColor);
			Driver->drawQuad(quad, RadarMaterial);
			//Driver->drawQuad (x-entitySize,y-entitySize,x+entitySize,y+entitySize,RadarOtherColor);
		}
	}


	// display particular places
	CTriangle triangle;
	for(uint i = 0; i < RadarParticularPlaces.size(); i++)
	{
		// relative position
		float posx = (RadarParticularPlaces[i].x-myPosx)*radius/RadarDistance;
		float posy = (RadarParticularPlaces[i].y-myPosy)*radius/RadarDistance;

		float dist = (float) sqrt((posx*posx)+(posy*posy));

		// Display a quad to show a player
		float an;
		float az;
		float x;
		float y;
		az = MouseListener->getOrientation ();
		if(posx==0)
		{
			if(posy==0)
			{
				x = xscreen;
				y = yscreen;
			}
			else
			{
				if(posy>0)
				{
					x = (float) (xscreen - dist*cos(Pi-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(Pi-az));
				}
				else
				{
					x = (float) (xscreen - dist*cos(-az)*3.f/4.f);
					y = (float) (yscreen - dist*sin(-az));
				}
			}
		}
		else
		{
			an = (float) atan(posy/posx);
			if(posx<0) an = an + (float)Pi;
			x = (float) (xscreen - dist*cos(-Pi/2 + an-az)*3.f/4.f); 
			y = (float) (yscreen - dist*sin(-Pi/2 + an-az));
		}


		if(x<xLeft || x>xRight || y>yTop || y<yBottom) 
		{
			continue;
		}

		triangle.V0.set(x-entitySize, y-entitySize, 0);
		triangle.V1.set(x+entitySize, y-entitySize, 0);
		triangle.V2.set(x, y+entitySize, 0);
		RadarMaterial.setColor(RadarPlaceColor);
		Driver->drawTriangle(triangle, RadarMaterial);
		//Driver->drawTriangle(x-entitySize,y-entitySize, x+entitySize,y-entitySize, x,y+entitySize, RadarPlaceColor);
	}
}

void cbUpdateRadar (CConfigFile::CVar &var)
{
	if (var.Name == "RadarPosX") RadarPosX = var.asFloat ();
	else if (var.Name == "RadarPosY") RadarPosY = var.asFloat ();
	else if (var.Name == "RadarWidth") RadarWidth = var.asFloat ();
	else if (var.Name == "RadarHeight") RadarHeight = var.asFloat ();
	else if (var.Name == "RadarBackColor") RadarBackColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarFrontColor") RadarFrontColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarSelfColor") RadarSelfColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarOtherColor") RadarOtherColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarPlaceColor") RadarPlaceColor.set (var.asInt(0), var.asInt(1), var.asInt(2), var.asInt(3));
	else if (var.Name == "RadarEntitySize") RadarEntitySize = var.asFloat ();
	else if (var.Name == "RadarState") RadarState = var.asInt ();
	else if (var.Name == "RadarDistance") RadarDistance = var.asInt ();
	else if (var.Name == "RadarMinDistance") RadarMinDistance = var.asInt ();
	else if (var.Name == "RadarMaxDistance") RadarMaxDistance = var.asInt ();
	else if (var.Name == "RadarParticularPlaces")
	{
		RadarParticularPlaces.clear ();
		for (uint i = 0; i < var.size(); i += 3)
		{
			RadarParticularPlaces.push_back (RadarParticularPlace(var.asFloat(i), var.asFloat(i+1), var.asString(i+2)));
		}
	}
	else if (var.Name == "RadarFontSize") RadarFontSize = var.asInt ();
	else if (var.Name == "RadarLittlePosX") RadarLittlePosX = var.asFloat ();
	else if (var.Name == "RadarLittlePosY") RadarLittlePosY = var.asFloat ();
	else if (var.Name == "RadarLittleRadius") RadarLittleRadius = var.asFloat ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void initRadar ()
{
	ConfigFile->setCallback ("RadarPosX", cbUpdateRadar);
	ConfigFile->setCallback ("RadarPosY", cbUpdateRadar);
	ConfigFile->setCallback ("RadarWidth", cbUpdateRadar);
	ConfigFile->setCallback ("RadarHeight", cbUpdateRadar);
	ConfigFile->setCallback ("RadarBackColor", cbUpdateRadar);
	ConfigFile->setCallback ("RadarFrontColor", cbUpdateRadar);
	ConfigFile->setCallback ("RadarSelfColor", cbUpdateRadar);
	ConfigFile->setCallback ("RadarOtherColor", cbUpdateRadar);
	ConfigFile->setCallback ("RadarPlaceColor", cbUpdateRadar);
	ConfigFile->setCallback ("RadarEntitySize", cbUpdateRadar);
	ConfigFile->setCallback ("RadarState", cbUpdateRadar);
	ConfigFile->setCallback ("RadarDistance", cbUpdateRadar);
	ConfigFile->setCallback ("RadarMinDistance", cbUpdateRadar);
	ConfigFile->setCallback ("RadarMaxDistance", cbUpdateRadar);
	ConfigFile->setCallback ("RadarParticularPlaces", cbUpdateRadar);
	ConfigFile->setCallback ("RadarFontSize", cbUpdateRadar);
	ConfigFile->setCallback ("RadarLittlePosX", cbUpdateRadar);
	ConfigFile->setCallback ("RadarLittlePosY", cbUpdateRadar);
	ConfigFile->setCallback ("RadarLittleRadius", cbUpdateRadar);

	cbUpdateRadar (ConfigFile->getVar ("RadarPosX"));
	cbUpdateRadar (ConfigFile->getVar ("RadarPosY"));
	cbUpdateRadar (ConfigFile->getVar ("RadarWidth"));
	cbUpdateRadar (ConfigFile->getVar ("RadarHeight"));
	cbUpdateRadar (ConfigFile->getVar ("RadarFrontColor"));
	cbUpdateRadar (ConfigFile->getVar ("RadarBackColor"));
	cbUpdateRadar (ConfigFile->getVar ("RadarSelfColor"));
	cbUpdateRadar (ConfigFile->getVar ("RadarOtherColor"));
	cbUpdateRadar (ConfigFile->getVar ("RadarPlaceColor"));
	cbUpdateRadar (ConfigFile->getVar ("RadarEntitySize"));
	cbUpdateRadar (ConfigFile->getVar ("RadarState"));
	cbUpdateRadar (ConfigFile->getVar ("RadarDistance"));
	cbUpdateRadar (ConfigFile->getVar ("RadarMinDistance"));
	cbUpdateRadar (ConfigFile->getVar ("RadarMaxDistance"));
	cbUpdateRadar (ConfigFile->getVar ("RadarParticularPlaces"));
	cbUpdateRadar (ConfigFile->getVar ("RadarFontSize"));
	cbUpdateRadar (ConfigFile->getVar ("RadarLittlePosX"));
	cbUpdateRadar (ConfigFile->getVar ("RadarLittlePosY"));
	cbUpdateRadar (ConfigFile->getVar ("RadarLittleRadius"));

        RadarMaterial = Driver->createMaterial ();
        RadarMaterial.initUnlit ();
        RadarMaterial.setBlendFunc (UMaterial::srcalpha, UMaterial::invsrcalpha);
        RadarMaterial.setBlend(true);
}

void updateRadar ()
{
	if (RadarDistance > RadarMaxDistance) RadarDistance = RadarMaxDistance;
	else if (RadarDistance < RadarMinDistance) RadarDistance = RadarMinDistance;

	switch (RadarState)
	{
	case 0:
		break;
	case 1:
		displayRadar ();
		break;
	case 2:
		displayLittleRadar();
		break;
	}
}

void releaseRadar ()
{
	ConfigFile->setCallback ("RadarPosX", NULL);
	ConfigFile->setCallback ("RadarPosY", NULL);
	ConfigFile->setCallback ("RadarWidth", NULL);
	ConfigFile->setCallback ("RadarHeight", NULL);
	ConfigFile->setCallback ("RadarBackColor", NULL);
	ConfigFile->setCallback ("RadarFrontColor", NULL);
	ConfigFile->setCallback ("RadarSelfColor", NULL);
	ConfigFile->setCallback ("RadarOtherColor", NULL);
	ConfigFile->setCallback ("RadarPlaceColor", NULL);
	ConfigFile->setCallback ("RadarEntitySize", NULL);
	ConfigFile->setCallback ("RadarState", NULL);
	ConfigFile->setCallback ("RadarDistance", NULL);
	ConfigFile->setCallback ("RadarMinDistance", NULL);
	ConfigFile->setCallback ("RadarMaxDistance", NULL);
	ConfigFile->setCallback ("RadarParticularPlaces", NULL);
	ConfigFile->setCallback ("RadarFontSize", NULL);
	ConfigFile->setCallback ("RadarLittlePosX", NULL);
	ConfigFile->setCallback ("RadarLittlePosY", NULL);
	ConfigFile->setCallback ("RadarLittleRadius", NULL);
	Driver->deleteMaterial(RadarMaterial); RadarMaterial = NULL;
}

NLMISC_COMMAND(go,"change position of the player with a player name or location","<player_name>|<location_name>")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() == 1)
	{
		bool gotoplayer = true;

		vector<RadarParticularPlace>::iterator itpp;
		for(itpp = RadarParticularPlaces.begin(); itpp != RadarParticularPlaces.end(); itpp++)
		{
			if((*itpp).name==args[0])
			{
				string cmd = "goto " + toString ((*itpp).x) + " " + toString ((*itpp).y);
				ICommand::execute (cmd, CommandsLog);

				gotoplayer = false;
				break;
			}
		}
		
		if(gotoplayer)
		{
			EIT itre;
			for(itre=Entities.begin(); itre!=Entities.end(); itre++)
			{
				if((*itre).second.Type != CEntity::Snowball)
				{
					if((*itre).second.Name == args[0])
					{
						string cmd = "goto " + toString ((*itre).second.Position.x) + " " + toString ((*itre).second.Position.y);
						ICommand::execute (cmd, CommandsLog);

						break;
					}
				}
			}
		}
	}
	else
		return false;


	return true;
}

} /* namespace SBCLIENT */

/* end of file */
