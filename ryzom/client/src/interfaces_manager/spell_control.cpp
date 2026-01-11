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
// Includes //
//////////////
// Client
#include "spell_control.h"
#include "interfaces_manager.h"

// 3D
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"



///////////
// Using //
///////////
using namespace NL3D;


////////////
// Extern //
////////////
extern UDriver *Driver;
extern UTextContext *TextContext;


//-----------------------------------------------
// Constructor :
//-----------------------------------------------
CSpellControl::CSpellControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
				uint16 spacing, const CPen &spellPen, const CPen &commentPen )
	:CControl (id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
{
	init(spacing, spellPen, commentPen);
} // constructor //

//-----------------------------------------------
// Constructor :
//-----------------------------------------------
CSpellControl::CSpellControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
				uint16 spacing,  uint32 fontSize1, const CRGBA &color1, bool shadow1,  uint32 fontSize2, const CRGBA &color2, bool shadow2 )
	:CControl (id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
{
	init(spacing, CPen(fontSize1, color1, shadow1), CPen(fontSize2, color2, shadow2));
} // constructor //



//-----------------------------------------------
// Destructor :
//-----------------------------------------------
CSpellControl::~CSpellControl()
{
} // destructor //

//-----------------------------------------------
// init :
//-----------------------------------------------
void CSpellControl::init( uint16 spacing, const CPen &spellPen, const CPen &commentPen)
{
	_Spacing = spacing;

	_SpellText.fontSize( spellPen.fontSize() );
	_SpellText.color( spellPen.color() );
	_SpellText.shadow( spellPen.shadow() );

	_Comment.fontSize( commentPen.fontSize() );
	_Comment.color( commentPen.color() );
	_Comment.shadow( commentPen.shadow() );

	_Button.ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
	_Button.hotSpot( THotSpot::HS_TR );
	_Button.origin( THotSpot::HS_BL );

	_RootIcon.ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
	_RootIcon.hotSpot( THotSpot::HS_TR );
	_RootIcon.origin( THotSpot::HS_BL );

	_SpellText.ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
	_SpellText.hotSpot( THotSpot::HS_TR );
	_SpellText.origin( THotSpot::HS_BL );

	_Comment.ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
	_Comment.hotSpot( THotSpot::HS_TR );
	_Comment.origin( THotSpot::HS_BL );
}// init //

//-----------------------------------------------
// initRootBitmap :
//-----------------------------------------------
/*void CSpellControl::initRootBitmap( CSpellClient *spell)
{
	nlassert(spell);

} // initRootBitmap //
*/

//-----------------------------------------------
// display :
//-----------------------------------------------
void CSpellControl::display()
{
		// If the control is hide -> return
	if(!_Show)
		return;

	/// \todo Malkav : found a way to refresh the comment line when it's edited somwhere else...
	if (_Spell)
		_Comment.text( _Spell->getComment() );

	/// \todo GUIGUI : initialize the scissor with oldScissor and remove tmp variables.
	// Backup scissor and create the new scissor to clip the list correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;

	float scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissor.X;
	scisY		= oldScissor.Y;
	scisWidth	= oldScissor.Width;
	scisHeight	= oldScissor.Height;

	float xtmp = _X_Display + _W_Display;
	float ytmp = _Y_Display + _H_Display;
	float xscistmp = scisX + scisWidth;
	float yscistmp = scisY + scisHeight;

	if( _X_Display > scisX )
		scisX = _X_Display;
	if( _Y_Display > scisY)
		scisY = _Y_Display;
	if( xtmp < xscistmp)
		scisWidth = xtmp-scisX;
	else
		scisWidth = xscistmp-scisX;
	if( ytmp < yscistmp)
		scisHeight = ytmp-scisY;
	else
		scisHeight = yscistmp-scisY;

	scissor.init(scisX, scisY, scisWidth, scisHeight);
//	Driver->setScissor(scissor);

// draw background
// ONLY FOR DEBUG
/*	UTextureFile *utexture = CInterfMngr::getTexture(2);
	if(utexture)
	{
		Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *utexture, true);
	}
*/
// draw the visible controls
	float x = _X;
	float xPixel = _X_Pixel;

	float w = 0, h = 0, wPixel = 0, hPixel = 0;

	// display button
	_Button.setPosition( x, _Y, xPixel , _Y_Pixel);
	_Button.display();

	_Button.getSize(w, h, wPixel, hPixel);

	x += w;
	xPixel += wPixel;

	// display root brick icon

	//------------------------------------------------------
	xPixel += _Spacing;
	_RootIcon.setPosition( x, _Y, xPixel , _Y_Pixel);
	//--- if the spell is 'latent', grey the icon
	if (_Spell->isLatent())
	{
		CRGBA old = _RootIcon.rgba();
		_RootIcon.rgba( CRGBA(128,128,128,255) );
		_RootIcon.display();
		_RootIcon.rgba( old);
	}
	else
	{
		_RootIcon.display();
	}

	_RootIcon.getSize(w, h, wPixel, hPixel);
	x += w;
	xPixel += wPixel;

	//------------------------------------------------------

	// display spell text
	if ( _Comment.text().length() == 0)
	{
		xPixel += _Spacing;

		_SpellText.setPosition( x, _Y, xPixel , _Y_Pixel);
		_SpellText.display();

		_SpellText.getSize(w, h, wPixel, hPixel);
		x += w;
		xPixel += wPixel;
	}
	else
	{
		// display comment
		xPixel += _Spacing;

		_Comment.setPosition( x, _Y, xPixel , _Y_Pixel);
		_Comment.display();

		_Comment.getSize(w, h, wPixel, hPixel);
		x += w;
		xPixel += wPixel;
	}

	// restore scissor
	Driver->setScissor(oldScissor);

} // display //


//-----------------------------------------------
// click :
//-----------------------------------------------
void CSpellControl::click(float x, float y, bool &taken)
{
	if (taken)
	{
		_Button.unSelect();

		return;
	}
	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		_Button.select();

		if (_NumFuncOn != 0)
			CInterfMngr::runFuncCtrl(_NumFuncOn, id(), _Spell);

		taken = true;
	}
	else
	{
		_Button.unSelect();
	}
} // click //


//-----------------------------------------------
// clickRight :
//-----------------------------------------------
void CSpellControl::clickRight(float x, float y, bool &taken)
{
	if (taken)
		return;

	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		if (_NumFuncRightClick != 0)
			CInterfMngr::runFuncCtrl(_NumFuncRightClick, id(), this);

		taken = true;
	}

} // clickRight //


//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
void CSpellControl::ref(float x, float y, float w, float h)
{
	_X_Ref	= x;
	_Y_Ref	= y;
	_W_Ref	= w;
	_H_Ref	= h;

	calculateDisplay();

	// Update chidren.
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float x, y;
			calculateOrigin(x, y, (*it)->origin());
			(*it)->ref(x, y, _W_Ref, _H_Ref);
		}
	}

	// update internal controls
	_Button.ref(x, y, w, h);
	_SpellText.ref(x, y, w, h);
	_Comment.ref(x, y, w, h);
	_RootIcon.ref(x,y,w,h);
}// ref //



