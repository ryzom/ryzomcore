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
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
// Client
#include "interfaces_manager.h"
#include "casting_bar.h"



///////////
// Using //
///////////
using namespace std;
using namespace NLMISC;
using namespace NL3D;

/////////////
// Externs //
/////////////
extern UDriver		*Driver;
extern UTextContext *TextContext;


//-----------------------------------------------
// display :
//-----------------------------------------------
void CCastingBar::display()
{
	_Time = ryzomGetLocalTime ();

	if (_Running)
	{
		// compute the pos
		const uint32 diff = (uint32)(_Time - _LastChangePosTime)/100; // we works in 1/10e s (10 units = 1s) and T1 is in 1/1000 s

		if (diff != 0)
		{
			// set the new pos
			if (diff < _CurrentPos )
				setPos( _CurrentPos - diff );
			else
				setPos(0);

			_LastChangePosTime = _Time;
		}
	}
	else
	{
		_LastChangePosTime = _Time;
	}

	// compute the temp pos if in smooth mode
	if (_Smooth)
	{
		uint32 nbUnit = (uint32)(_Time - _LastUpdateSmooth) / _SmoothFillRate;
		if (nbUnit > 0)
		{
			_LastUpdateSmooth = _Time;

			uint32 diff = abs( _TempPos - _CurrentPos );
			if (diff < nbUnit)
				nbUnit = diff;

			if ( _TempPos < _CurrentPos )
				_TempPos += nbUnit;
			else
				_TempPos -= nbUnit;
		}
	}
	else
	{
		nlassert( _TempPos == _CurrentPos );
	}

	// check auto hide
	if (_AutoHide && _TempPos == 0)
	{
		show( false );
		stop();
	}

	// If the control is hide -> return
	if(!_Show)
		return;


	// draw background
	UTextureFile *utexture = CInterfMngr::getTexture( _BackgroundTexture );
	if(utexture)
	{
		Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *utexture, true, _BackgroundColor);
	}

	// calculate _W_Display of the progress bar
	const float wBar = _W_Display * ( static_cast<float>(_TempPos) / static_cast<float>(_Range) );

	// Backup scissor and create the new scissor to clip the bar correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;

	float scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissor.X;
	scisY		= oldScissor.Y;
	scisWidth	= oldScissor.Width;
	scisHeight	= oldScissor.Height;

	float xtmp = _X_Display + wBar;
	float ytmp = _Y_Display + _H_Display;
	float xscistmp = scisX + scisWidth;
	float yscistmp = scisY + scisHeight;
	if( _X_Display > scisX )
		scisX = _X_Display;
	if( _Y_Display > scisY )
		scisY = _Y_Display;
	if( xtmp < xscistmp )
		scisWidth = xtmp - scisX;
	else
		scisWidth = xscistmp - scisX;
	if( ytmp < yscistmp )
		scisHeight = ytmp - scisY;
	else
		scisHeight = yscistmp - scisY;

	scissor.init(scisX, scisY, scisWidth, scisHeight);
	Driver->setScissor(scissor);

	// display progress bar
	utexture = CInterfMngr::getTexture( _ProgressBarTexture );
	Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *utexture, true, _ProgressBarColor);

	// restore old scissor
	Driver->setScissor(oldScissor);

	// display text
	if ( ! _Text.empty() )
	{
		TextContext->setShaded(_Shadow);
		TextContext->setHotSpot(NL3D::UTextContext::THotSpot::MiddleMiddle);
		TextContext->setColor(_Color);
		TextContext->setFontSize(_FontSize);
		TextContext->printAt(_X_Display + _W_Display/2, _Y_Display + _H_Display/2, _Text);
	}

	//
}// display //