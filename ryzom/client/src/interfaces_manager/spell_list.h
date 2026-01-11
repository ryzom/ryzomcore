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



#ifndef NL_SPELL_LIST_H
#define NL_SPELL_LIST_H

// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
// Client
#include "control_list.h"
#include "pen.h"
#include "spell_client.h"
#include "spell_control.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CSpellList : public CControlList
{
public:

	/// Constructor
	CSpellList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint16 spacing = 0, uint leftFunc = 0, uint rightFunc = 0);

	// destructor
	~CSpellList();

	/**
	 * set the line height
	 * \param
	 */
	void setLineHeight( float h, float hpixels)
	{
		_Line_H = h;
		_Line_H_Pixels = hpixels;
	}


	/**
	 * set the characteristics of the selection buttons
	 * \param float w button width in pixels
	 * \param float h button height in pixels
	 * \param CButtonBase& base the characteristics of the button (textures and colors)
	 */
	void setButtonParam( float w, float h, const CButtonBase &base);

	/**
	 * set the characteristics of the selection buttons
	 * \param float w button width in pixels
	 * \param float h button height in pixels
	 * \param uint tOn button texture when on
	 * \param CRGBA &colorOn button color when on
	 * \param uint tOff button texture when off
	 * \param CRGBA &colorOff button color when off
	 * \param uint tDis button texture when disable
	 * \param CRGBA &colorDis button color when disable
	 * \param CButtonBase::TBG &modeOn button background display mode when On
	 * \param CButtonBase::TBG &modeOff button background display mode when Off
	 * \param CButtonBase::TBG &modeDisable button background display mode when Disabled
	 */
	void setButtonParam( float w, float h, uint tOn, const CRGBA &colorOn, uint tOff, const CRGBA &colorOff, uint tDis, const CRGBA &colorDis,const CButtonBase::TBG &modeOn, const CButtonBase::TBG &modeOff, const CButtonBase::TBG &modeDisable  );

	/**
	 * set the pen used for spell phrase
	 * \param CPen &pen the new pen
	 */
	void setSpellPen( const CPen &pen) { _SpellPen = pen; }

	/**
	 * set the pen used for commen
	 * \param CPen &pen the new pen
	 */
	void setCommentPen( const CPen &pen) { _CommentPen = pen; }


	/**
	 * add a new spell to the list
	 * \param CSpellClient *spell the spell to add to the list
	 */
	void addSpell(CSpellClient *spell);

	/**
	 * remove the specified spell from list (may be slow : check items in insertion order until it find the right one)
	 * \param uint16 spellId id of the spell to remove
	 * \return CSpellClient* adress of the removed spell or NULL if the spell was not found
	 */
	CSpellClient * removeSpell( uint16 spellId );

	/**
	 * get the selected spell control (may be slow : check items in insertion order until it find the right one)
	 * \return CSpellControl *spellControl the selected control, or NULL if none is selected
	 */
	CSpellControl *getSelection();


	/// Set some references for the display.
//	virtual void ref(float x, float y, float w, float h);


private:
	/// forbid the use of the 'old' add method
	virtual void add(CControl *) {}

private:
	// \name parameters of the selection button
	//@{
	/// width in pixels
	float	_ButtonWidth;
	/// height in pixels
	float	_ButtonHeight;

	/// base button
	CButtonBase	_Base;

	/// associated texture when 'On'
	uint	_ButtonTextureOn;
	/// associated texture when 'Off'
	uint	_ButtonTextureOff;
	/// associated texture when 'Disable'
	uint	_ButtonTextureDisable;
	/// associated RGBA when 'On'
	NLMISC::CRGBA	_ButtonColorOn;
	/// associated RGBA when 'Off'
	NLMISC::CRGBA	_ButtonColorOff;
	/// associated RGBA when 'Disable'
	NLMISC::CRGBA	_ButtonColorDisable;

	/// background display mode when 'On'
	CButtonBase::TBG	_ButtonBGModeOn;
	/// background display mode when 'Off'
	CButtonBase::TBG	_ButtonBGModeOff;
	/// background display mode when 'Disable'
	CButtonBase::TBG	_ButtonBGModeDisable;
	//@}

	// \name parameters for texts
	//@{
	/// pen associated with spell phrase
	CPen	_SpellPen;
	/// pen associated with spell comment
	CPen	_CommentPen;
	//@}

	/// height of 'line' (SpellControls)
	float	_Line_H;
	float	_Line_H_Pixels;

	/// map x/y positions with controls
	typedef std::list< std::pair< std::pair<float,float>, const CControl*> >  TPairPosItem;
	TPairPosItem	_ItemsPos;

	/// function called with a left click on the button
	uint		_NumFuncOn;

	/// function called with a right click on the button
	uint		_NumFuncRightClick;
};


#endif // NL_SPELL_LIST_H

/* End of spell_list.h */
