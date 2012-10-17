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

// Client
#include "spell_list.h"

// Misc
#include "nel/misc/debug.h"

using namespace NLMISC;


//-----------------------------------------------
// Constructor :
//-----------------------------------------------
CSpellList::CSpellList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint16 spacing, uint leftFunc, uint rightFunc)
	:CControlList( id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, spacing)
{
	_NumFuncOn = leftFunc;
	_NumFuncRightClick = rightFunc;
} // constructor //




//-----------------------------------------------
// Destructor :
//-----------------------------------------------
CSpellList::~CSpellList()
{
	const TListControl::iterator itEnd = _Items.end();

	for (TListControl::iterator it = _Items.begin() ; it != itEnd ; ++it)
	{
		if (*it != NULL)
		{
			delete (*it);
			(*it) = NULL;
		}
	}
	_Items.clear();
} // Destructor //


//-----------------------------------------------
// setButtonParam :
//-----------------------------------------------
void CSpellList::setButtonParam( float w, float h, uint tOn, const CRGBA &colorOn, uint tOff, const CRGBA &colorOff, uint tDis, const CRGBA &colorDis,
								const CButtonBase::TBG &modeOn, const CButtonBase::TBG &modeOff, const CButtonBase::TBG &modeDisable )
{
	_ButtonWidth = w;
	_ButtonHeight = h;

	_ButtonTextureOn = tOn;
	_ButtonTextureOff = tOff;
	_ButtonTextureDisable = tDis;

	_ButtonColorOn = colorOn;
	_ButtonColorOff = colorOff;
	_ButtonColorDisable = colorDis;

	_ButtonBGModeOn = modeOn;
	_ButtonBGModeOff = modeOff;
	_ButtonBGModeDisable = modeDisable;
} // setButtonParam //

//-----------------------------------------------
// setButtonParam :
//-----------------------------------------------
void CSpellList::setButtonParam( float w, float h, const CButtonBase &base )
{
	_ButtonWidth = w;
	_ButtonHeight = h;

	_ButtonTextureOn = base.textureOn();
	_ButtonTextureOff = base.textureOff();
	_ButtonTextureDisable = base.textureDisable();

	_ButtonColorOn = base.colorOn();
	_ButtonColorOff = base.colorOff();
	_ButtonColorDisable = base.colorDisable();

	_ButtonBGModeOn = base.bgModeOn();
	_ButtonBGModeOff = base.bgModeOff();
	_ButtonBGModeDisable = base.bgModeDisable();
} // setButtonParam //


//-----------------------------------------------
// addSpell :
//-----------------------------------------------
void CSpellList::addSpell(CSpellClient *spell)
{
	//nlassert( spell );
	if (!spell)
	{
		nlwarning("<CSpellList::addSpell> : spell in param is NULL, abort");
		return;
	}

	// get the scroll bar width and remove it from the width of the list
	float w, h, w_pixel, h_pixel;
	_VScroll->getSize( w, h, w_pixel, h_pixel );

	// create a CSpellControl Object
	CSpellControl *ctrl = new CSpellControl(0, _X, _Y, _X_Pixel, _Y_Pixel, _W - w, _Line_H , _W_Pixel - w_pixel, _Line_H_Pixels, 8, _SpellPen, _CommentPen);

	//nlassert( ctrl );
	if ( !ctrl)
	{
		nlwarning("<CSpellList::addSpell> : allocation of a new CSpellControl failed, abort");
		return;
	}

	ctrl->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
	ctrl->hotSpot(THotSpot::HS_TR);
	ctrl->origin(THotSpot::HS_BL);
	ctrl->setButtonSize(_ButtonWidth, _ButtonHeight );

	ctrl->setButtonTextureOn( _ButtonTextureOn );
	ctrl->setButtonTextureOff( _ButtonTextureOff );
	ctrl->setButtonTextureDisable( _ButtonTextureDisable );

	ctrl->setButtonColorOn( _ButtonColorOn );
	ctrl->setButtonColorOff( _ButtonColorOff );
	ctrl->setButtonColorDisable( _ButtonColorDisable );

	ctrl->setButtonBGModeOn( _ButtonBGModeOn );
	ctrl->setButtonBGModeOff( _ButtonBGModeOff );
	ctrl->setButtonBGModeDisable( _ButtonBGModeDisable );

	ctrl->setLeftClickFunc( _NumFuncOn );
	ctrl->setRightClickFunc( this->_NumFuncRightClick );

	ctrl->setSpell( spell );

	// update the internal list
	_Items.push_back( ctrl );

	++_NbElts;

	// if this is the first item, init the ending point
	if ( _NbElts == 1)
		_EndingIterator = _Items.rbegin();

} // addSpell //




//-----------------------------------------------
// addSpell :
//-----------------------------------------------
CSpellClient * CSpellList::removeSpell( uint16 spellId )
{
	CSpellClient *spell = NULL;

	CSpellControl *ctrl = NULL;

	TListControl::iterator it;
	const TListControl::iterator itEnd = _Items.end();

	for (it = _Items.begin() ; it != itEnd ; ++it)
	{
		nlassert( *it != NULL);
		ctrl = static_cast<CSpellControl *> (*it);
		spell = ctrl->getSpell();

		if ( spell->id() == spellId)
		{
			_Items.erase( it );

			// if it's the last displayed element, update the iterator
			if ( ctrl == static_cast<CSpellControl *> (*_EndingIterator) )
			{
				if ( _NbElts == 1)
					_EndingIterator = _Items.rbegin();
				else
				{
					if ( it == --_Items.end())
						_EndingIterator = _Items.rbegin();
					else
						++_EndingIterator;
				}
			}

			delete ctrl;
			ctrl = NULL;

			-- _NbElts;
			break;
		}
	}

	return spell;
} // removeSpell //


//-----------------------------------------------
// getSelection :
//-----------------------------------------------
CSpellControl * CSpellList::getSelection()
{
	CSpellControl *ctrl;

	TListControl::iterator it;
	const TListControl::iterator itEnd = _Items.end();

	for (it = _Items.begin() ; it != itEnd ; ++it)
	{
		nlassert( *it != NULL);
		ctrl = safe_cast<CSpellControl *> (*it);

		if (ctrl->isSelected())
			return ctrl;
	}

	return NULL;
} // getSelection //


//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
/*
void CSpellList::ref(float x, float y, float w, float h)
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

	// update controls in the list
	const TListControl::iterator itEnd = _Items.end();

	for (it = _Items.begin() ; it != itEnd ; ++it)
	{
		nlassert( *it != NULL);
		(*it)->ref( x, y, w, h);
	}

}// ref //
*/