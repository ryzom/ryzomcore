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


//////////////
// INCLUDES //
//////////////
// Misc.
#include "nel/misc/common.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "rosace.h"
#include "math.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver *Driver;


///////////////
// FUNCTIONS //
///////////////
//////////////////////
// CRosaceComponent //
//////////////////////
//-----------------------------------------------
// CRosaceComponent :
// Constructor.
//-----------------------------------------------
CRosaceComponent::CRosaceComponent()
{
	// No callback function at the beginning.
	_Callback	= 0;
	// No texture at the beginning.
	_Texture	= 0;
	_X = 0.5f;
	_Y = 0.5f;
	_W = 0.05f;
	_H = 0.05f;
}// CRosaceComponent //

//-----------------------------------------------
// display :
// Display the component.
//-----------------------------------------------
void CRosaceComponent::display(bool selected)
{
	// If there is a texture to display.
	if(_Texture)
	{
		if(selected)
			Driver->drawBitmap(_X, _Y, _W, _H, *_Texture, true, CRGBA(255, 0, 0, 255));
		else
			Driver->drawBitmap(_X, _Y, _W, _H, *_Texture, true, CRGBA(255, 255, 255, 255));
	}
}// display //

//-----------------------------------------------
// execute :
// Execute the callback associated to the component.
//-----------------------------------------------
void CRosaceComponent::execute()
{
	// If there is a callback -> execute it.
	if(_Callback)
		_Callback();
}// execute //

//-----------------------------------------------
// texture :
// Set the texture for the component.
//-----------------------------------------------
void CRosaceComponent::texture(const string &filename)
{
	_Texture = Driver->createTextureFile(filename);
}// texture //

//-----------------------------------------------
// inside :
// Return true if the position (x,y) is inside the rosace.
//-----------------------------------------------
bool CRosaceComponent::inside(float x, float y)
{
	if(x>=_X && x<_X+_W && y>=_Y && y<_Y+_H)
		return true;
	return false;
}// inside //




/////////////////
// CRosacePage //
/////////////////
//-----------------------------------------------
// CRosacePage :
// Constructor.
//-----------------------------------------------
CRosacePage::CRosacePage()
{
	_OldX = 0.5f;
	_OldY = 0.5f;
	_Selected = -1;
	_Components.clear();
}// CRosacePage //

//-----------------------------------------------
// CRosacePage :
// Constructor. Create some components at the beginning.
//-----------------------------------------------
CRosacePage::CRosacePage(sint nb)
{
	_Selected = -1;
	_Components.clear();
	_Components.resize(nb);
	generate();
}// CRosaceContext //

//-----------------------------------------------
// display :
// Display all the components in the page.
//-----------------------------------------------
void CRosacePage::display()
{
	for(uint i = 0; i<size(); ++i)
	{
		if(_Selected == (sint)i)
			_Components[i].display(true);
		else
			_Components[i].display(false);
	}
}// display //

//-----------------------------------------------
// execute :
// Execute the callback associated to the selected component.
//-----------------------------------------------
void CRosacePage::execute()
{
	// If there is no component selected -> return.
	if(_Selected<0 || _Selected>=(sint)size())
		return;
	// Else execute the callback associated to the selected component.
	_Components[_Selected].execute();
}// execute //

//-----------------------------------------------
// next :
// Select the next valide component.
//-----------------------------------------------
void CRosacePage::next()
{
	_Selected++;
	if(!valide())
	{
		_Selected = 0;
		if(!valide())
			_Selected = -1;
	}
}// next //

//-----------------------------------------------
// previous :
// Select the previous valide component.
//-----------------------------------------------
void CRosacePage::previous()
{
	_Selected--;
	// If the selected component is not valide selected the last 1 (-1 is no component).
	if(!valide())
		_Selected = (sint)size()-1;
}// previous //

//-----------------------------------------------
// generate :
// Generate the rosace (all components).
//-----------------------------------------------
void CRosacePage::generate()
{
	float _W = 0.05f;
	float _H = 0.05f;

	double ang	= 2*Pi/(float)size();
	double r1	= _W/(2*sin(ang/2));			// r = opp/tan @
	double r2	= _H/(2*sin(ang/2));
	for(uint i=0; i<size(); ++i)
	{
		double x = r1*cos(i*ang);
		double y = r2*sin(i*ang);
		_Components[i].setPos((float)(0.5f+x-_W/2), (float)(0.5f+y-_H/2));
	}
}// generate //

//-----------------------------------------------
// update :
// Update the page.
//-----------------------------------------------
void CRosacePage::update(float x, float y, TMode mode)
{
	switch(mode)
	{
	case CursorMode:
		cursorMode(x, y);
		break;

	case CursorAngleMode:
		cursorAngleMode(x, y);
		break;

	case RelativeMode:
		relativeMode(x, y);
		break;

	case DirectMode:
		directMode(x, y);
		break;
	case NbRosaceMode:
		nlwarning("Rosace Mode reached.");
		break;
	default:
		break;
	}
}// update //

//-----------------------------------------------
// select :
// ...
//-----------------------------------------------
void CRosacePage::select(double ang)
{
	double angTmp = 2*Pi/(float)size();
	if(ang>=0)
	{
		ang = ang+angTmp/2;
		_Selected = (sint)(ang/angTmp);
	}
	else
	{
		ang = -ang+angTmp/2;
		_Selected = size()-(sint)(ang/angTmp);
	}

	if(_Selected >= (sint)size() || _Selected<0)
		_Selected = 0;
}// select //

//-----------------------------------------------
// cursorMode :
// Select the component under the position (x,y) or unselect all if nothing at this position.
//-----------------------------------------------
void CRosacePage::cursorMode(float x , float y)
{
	for(uint i=0; i<size(); ++i)
	{
		if(_Components[i].inside(x, y))
		{
			_Selected = i;
			return;
		}
	}

	// Unselect all.
	_Selected = -1;
}// cursorMode //

//-----------------------------------------------
// cursorAngleMode :
// Select elements of the rosace according to the angle generated by mouse position and screen center.
//-----------------------------------------------
void CRosacePage::cursorAngleMode(float x , float y)
{
	// If the mouse is near center -> unselect all.
	if(x>=0.5f-0.01f && x<0.5f+0.01f && y>=0.5f-0.01f && y<0.5f+0.01f)
		_Selected = -1;
	// Select the right element of the rosace according to the angle.
	else
	{
		uint32 width, height;
		Driver->getWindowSize(width, height);

		double x1, y1;
		x1 = (double)width/2.0;
		x1 = (x*(double)width) - x1;

		y1 = (double)height/2.0;
		y1 = (y*(double)height) - y1;

		select(atan2(y1, x1));
	}
}// CursorAngleMode //

//-----------------------------------------------
// directMode :
// ...
//-----------------------------------------------
void CRosacePage::directMode(float x , float y)
{
	float difX = x-_OldX;
	float difY = y-_OldY;

	if(fabs(difX)<0.01f && fabs(difY)<0.01f)
		return;

	uint32 width, height;
	Driver->getWindowSize(width, height);

	double x1 = (difX*(double)width);
	double y1 = (difY*(double)height);

	select(atan2(y1, x1));

	// Backup the x and y.
	_OldX = x;
	_OldY = y;
}//directMode //

//-----------------------------------------------
// relativeMode :
// ...
//-----------------------------------------------
void CRosacePage::relativeMode(float x, float /* y */)
{
	float difX = x-_OldX;
	if(fabs(difX)<0.01f)
		return;

	if(difX>0)
		next();
	else
		previous();

	// Backup the x and y.
	_OldX = x;
}// relativeMode //




///////////////////
// CRosaceContext//
///////////////////
//-----------------------------------------------
// CRosaceContext :
// Constructor.
//-----------------------------------------------
CRosaceContext::CRosaceContext()
{
	_Selected = 0;
	_Pages.clear();
}// CRosacePage //

//-----------------------------------------------
// CRosaceContext :
// Constructor. Create some pages at the beginning.
//-----------------------------------------------
CRosaceContext::CRosaceContext(sint nb)
{
	_Selected = 0;
	_Pages.clear();
	_Pages.resize(nb);
}// CRosacePage //

//-----------------------------------------------
// display :
// Display all the pages in the context.
//-----------------------------------------------
void CRosaceContext::display()
{
	// If there is no page selected -> return.
	if(!valide())
		return;
	// Else display the right page.
	_Pages[_Selected].display();
}// display //

//-----------------------------------------------
// execute :
// Execute the callback associated to the selected component in the selected page.
//-----------------------------------------------
void CRosaceContext::execute()
{
	// If there is no page selected -> return.
	if(!valide())
		return;
	// Else execute the callback associated to the selected component.
	_Pages[_Selected].execute();
}// execute //

//-----------------------------------------------
// add :
// Add a page.
//-----------------------------------------------
void CRosaceContext::add(const CRosacePage &page)
{
	_Pages.push_back(page);
}// add //

//-----------------------------------------------
// update :
// Update the context.
//-----------------------------------------------
void CRosaceContext::update(float x, float y, CRosacePage::TMode mode)
{
	// If there is no page selected -> return.
	if(!valide())
		return;
	// Update the selected page.
	_Pages[_Selected].update(x, y, mode);
}// update //

//-----------------------------------------------
// next :
// Select the next valide page.
//-----------------------------------------------
void CRosaceContext::next()
{
	_Selected++;
	if(!valide())
	{
		_Selected = 0;
		if(!valide())
			_Selected = -1;
	}
}// next //

//-----------------------------------------------
// previous :
// Select the previous valide page.
//-----------------------------------------------
void CRosaceContext::previous()
{
	_Selected--;
	// If the selected component is not valide selected the last 1 (-1 is no component).
	if(!valide())
		_Selected = (sint)size()-1;
}// previous //




/////////////
// CRosace //
/////////////
//-----------------------------------------------
// CRosace :
// Constructor.
//-----------------------------------------------
CRosace::CRosace()
{
	init();
}// CRosace //

//-----------------------------------------------
// ~CRosace :
// Destructor.
//-----------------------------------------------
CRosace::~CRosace()
{
}// ~CRosace //

//-----------------------------------------------
// init :
// Initialize the rosace.
//-----------------------------------------------
void CRosace::init()
{
	_Mode = CRosacePage::CursorAngleMode;
	_Selected = "";
}// init //

//-----------------------------------------------
// add :
// Add a page. ("" is not valide name).
//-----------------------------------------------
void CRosace::add(const string &name, const CRosaceContext &context)
{
	// If the name is not empty.
	if(name != "")
		_Contexts.insert(TContexts::value_type (name, context));
}// add //

//-----------------------------------------------
// valide :
// Is the current context valide.
//-----------------------------------------------
bool CRosace::valide()
{
	// Check if the Selected context is valide.
	TContexts::iterator it = _Contexts.find(_Selected);
	if(it != _Contexts.end())
		return true;
	return false;
}// valide //

//-----------------------------------------------
// display :
// Display the rosace.
//-----------------------------------------------
void CRosace::display()
{
	if(valide())
		_Contexts[_Selected].display();
}// display //

//-----------------------------------------------
// update :
// ...
//-----------------------------------------------
void CRosace::update(float x, float y)
{
	if(valide())
		_Contexts[_Selected].update(x, y, _Mode);
}// update //

//-----------------------------------------------
// execute :
// Execute the callback function corresponding to the selected component.
//-----------------------------------------------
void CRosace::execute()
{
	if(valide())
		_Contexts[_Selected].execute();
}// execute //

//-----------------------------------------------
// swap :
// Swap to next rosace page.
//-----------------------------------------------
void CRosace::swap()
{
	if(valide())
		_Contexts[_Selected].next();
}// swap //
