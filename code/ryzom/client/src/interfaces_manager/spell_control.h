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



#ifndef NL_SPELL_CONTROL_H
#define NL_SPELL_CONTROL_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

// Client
#include "control.h"
#include "bitmap.h"
#include "spell_client.h"
#include "pen.h"
#include "button.h"
#include "text.h"



/**
 * define the control used to display spells in control list objects
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CSpellControl : public CControl
{
public:

	/// Constructor
	explicit CSpellControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
				uint16 spacing, const CPen &pen1, const CPen &pen2);

	/// Constructor
	explicit CSpellControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
				uint16 spacing,  uint32 fontSize1, const CRGBA &color1, bool shadow1,  uint32 fontSize2, const CRGBA &color2, bool shadow2);

	/// destuctor
	~CSpellControl();

	/// display the control
	virtual void display();

	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click for the control.
	virtual void clickRight(float x, float y, bool &taken);

	/**
	 * set the spell pointed by this control
	 * \param CSpellClient *spell
	 */
	void setSpell( CSpellClient *spell)
	{
		//nlassert( spell );
		if (!spell)
		{
			nlwarning("<CSpellControl::setSpell> : spell in param is NULL, abort");
			return;
		}

		_Spell = spell ;
		_SpellText.text(spell->getPhrase());
		_Comment.text( spell->getComment() );

		TBrickList::iterator it = spell->getBricks().begin();

		if (it != spell->getBricks().end() )
		{
			nlassert( *it );
			CBrickClient *brick = (*it);
			_RootIcon.texture( brick->getTexture() );
		}
	}

	/**
	 * get the spell pointed by this control
	 * \return CSpellClient *spell
	 */
	CSpellClient *getSpell() { return _Spell; }


	/**
	 * set the size of the selection button
	 * \param float wPixel the new width (in pixels) of the button
	 * \param float hPixel the new height (in pixels) of the button
	 */
	void setButtonSize(float wPixel, float hPixel)
	{
		_Button.setSize(0,0,wPixel,hPixel);
		_RootIcon.setSize(0,0,wPixel,hPixel);
	}


	/// \name set the textures, color and BGMode for the selection button
	// @{
	/**
	 * set the texture when 'on'
	 * \param uint tOn new texture when the button is selected
	 */
	void setButtonTextureOn( uint tOn) { _Button.textureOn( tOn ); }
	/**
	 * set the texture when 'off'
	 * \param uint tOff new texture when the button is unselected
	 */
	void setButtonTextureOff( uint tOff) { _Button.textureOff( tOff ); }
	/**
	 * set the texture when 'disabled'
	 * \param uint tDis new texture when the button is disabled
	 */
	void setButtonTextureDisable( uint tDis) { _Button.textureDisable( tDis ); }
	/**
	 * set the color when 'on'
	 * \param CRGBA& color new color when the button is selected
	 */
	void setButtonColorOn( const CRGBA &color) { _Button.colorOn( color ); }
	/**
	 * set the color when 'off'
	 * \param CRGBA& color new color when the button is unselected
	 */
	void setButtonColorOff(const CRGBA &color) { _Button.colorOff( color ); }
	/**
	 * set the color when 'disabled'
	 * \param CRGBA& color new color when the button is disabled
	 */
	void setButtonColorDisable(const CRGBA &color) { _Button.colorDisable( color ); }

	/**
	 * set the color when 'on'
	 * \param CRGBA& color new color when the button is selected
	 */
	void setButtonBGModeOn( const CButtonBase::TBG &mode) { _Button.bgModeOn( mode ); }
	/**
	 * set the color when 'off'
	 * \param CRGBA& color new color when the button is unselected
	 */
	void setButtonBGModeOff(const CButtonBase::TBG &mode) { _Button.bgModeOff( mode ); }
	/**
	 * set the color when 'disabled'
	 * \param CRGBA& color new color when the button is disabled
	 */
	void setButtonBGModeDisable(const CButtonBase::TBG &mode) { _Button.bgModeDisable( mode ); }
	//@}

	/// \name text properties
	// @{
	/// get spell text
	const CText &getSpellText() const { return _SpellText; }

	/// set spell text pen
	void setSpellTextPen( const CPen &pen)
	{
		_SpellText.fontSize( pen.fontSize());
		_SpellText.color( pen.color());
		_SpellText.shadow( pen.shadow());
	}

	/// get comment
	const CText &getComment() const { return _Comment; }
	/// set Comment pen
	void setCommentPen( const CPen &pen)
	{
		_Comment.fontSize( pen.fontSize());
		_Comment.color( pen.color());
		_Comment.shadow( pen.shadow());
	}
	// @}

	/// \name functions
	// @{
	/// set function called on left click on the control
	void setLeftClickFunc( uint num) { _NumFuncOn = num; }
	/// set function called on right click on the control
	void setRightClickFunc( uint num) { _NumFuncRightClick = num; }
	// @}

	void ref(float x, float y, float w, float h);


	/**
	 * get the state of this control (slected or not)
	 * \return bool true of the control is selected, false otherwise
	 */
	bool isSelected() const { return _Button.isSelected(); }

private:
	/// Initialize the object (1 function called for all constructors -> easier).
	inline void init( uint16 spacing, const CPen &spellPen, const CPen &commentPen);

	/// init the root icon bitmap
//	inline void initRootBitmap( CSpellClient *spell); // ????

private:
	/// the spell pointed by this control
	CSpellClient *_Spell;

	/// the bitmap for the root brick of the spell
	CBitm		_RootIcon;

	/// the selection button
	CButton		_Button;

	/// the spell text
	CText		_SpellText;

	/// the comment
	CText		_Comment;

	/// function called with a left click on the control
	uint		_NumFuncOn;

	/// function called with a right click on the control
	uint		_NumFuncRightClick;

	/// spacing (in pixels)
	uint16		_Spacing;
};


#endif // NL_SPELL_CONTROL_H

/* End of spell_control.h */
