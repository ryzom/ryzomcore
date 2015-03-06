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



#ifndef NL_CTRL_TEXT_BUTTON_H
#define NL_CTRL_TEXT_BUTTON_H

#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/view_renderer.h"


namespace NLGUI
{
	class CEventDescriptor;
	class CViewText;

	// ***************************************************************************
	/**
	 * Text Button that can be either Push or Toggle button. Localized, auto-resize
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2002
	 */
	class CCtrlTextButton : public CCtrlBaseButton
	{
	public:
        DECLARE_UI_CLASS( CCtrlTextButton )

		/// Constructor
		CCtrlTextButton(const TCtorParam &param);
		~CCtrlTextButton();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		// Init part
		virtual bool parse (xmlNodePtr cur,CInterfaceGroup * parentGroup);

		virtual void checkCoords();
		virtual void updateCoords();

		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		// Display part
		virtual void draw();

		// Hide/Show the text also.
		virtual	void setActive(bool state);

		// Add also our ViewText
		virtual	void onAddToGroup();


		/// \from CInterfaceElement
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		// Special Text Colors accessors
		// Colors
		NLMISC::CRGBA	getTextColorNormal() const {return _TextColorNormal;}
		void			setTextColorNormal(NLMISC::CRGBA	v) {_TextColorNormal= v;}
		NLMISC::CRGBA	getTextColorPushed() const {return _TextColorPushed;}
		void			setTextColorPushed(NLMISC::CRGBA	v) {_TextColorPushed= v;}
		NLMISC::CRGBA	getTextColorOver() const {return _TextColorOver;}
		void			setTextColorOver(NLMISC::CRGBA	v) {_TextColorOver= v;}
		// Shadow Colors
		NLMISC::CRGBA	getTextShadowColorNormal() const {return _TextShadowColorNormal;}
		void			setTextShadowColorNormal(NLMISC::CRGBA	v) {_TextShadowColorNormal= v;}
		NLMISC::CRGBA	getTextShadowColorPushed() const {return _TextShadowColorPushed;}
		void			setTextShadowColorPushed(NLMISC::CRGBA	v) {_TextShadowColorPushed= v;}
		NLMISC::CRGBA	getTextShadowColorOver() const {return _TextShadowColorOver;}
		void			setTextShadowColorOver(NLMISC::CRGBA	v) {_TextShadowColorOver= v;}
		// Global Modulate Colors
		bool			getTextModulateGlobalColorNormal() const {return _TextModulateGlobalColorNormal;}
		void			setTextModulateGlobalColorNormal(bool v) {_TextModulateGlobalColorNormal= v;}
		bool			getTextModulateGlobalColorPushed() const {return _TextModulateGlobalColorPushed;}
		void			setTextModulateGlobalColorPushed(bool v) {_TextModulateGlobalColorPushed= v;}
		bool			getTextModulateGlobalColorOver() const {return _TextModulateGlobalColorOver;}
		void			setTextModulateGlobalColorOver(bool v) {_TextModulateGlobalColorOver= v;}
		// Set text (noop if text id)
		void			setText (const ucstring &text);
		ucstring		getText () const;

		void			setHardText (const std::string &text);
		std::string		getHardText () const;

		CViewText*		getViewText();
		void			setViewText(CViewText* text) {_ViewText=text;}

		void			setTextX(sint32 x);
		sint32			getTextX() const { return _TextX; }

		void			setWMargin(sint32 w) { _WMargin = w; }
		sint32			getWMargin() const { return _WMargin; }

		sint32			getWMin() const { return _WMin; }
		void			setWMin( sint32 wmin ) { _WMin = wmin; }

		// Compute Size according to bitmap and Text (Ensure as big as possible button)
		sint32			getWMax() const;

		int luaGetViewText(CLuaState &ls);

		REFLECT_EXPORT_START(CCtrlTextButton, CCtrlBaseButton)
			REFLECT_UCSTRING("uc_hardtext", getText, setText);
			REFLECT_STRING("hardtext", getHardText, setHardText);
			REFLECT_SINT32("text_x", getTextX, setTextX)
			REFLECT_SINT32("wmargin", getWMargin, setWMargin)
			REFLECT_SINT32("wmin", getWMin, setWMin)
			REFLECT_LUA_METHOD("getViewText", luaGetViewText)
		REFLECT_EXPORT_END

		void onRemoved();
		void onWidgetDeleted( CInterfaceElement *e );
		void moveBy( sint32 x, sint32 y );

	protected:

		enum	{NumTexture= 3};

		CViewRenderer::CTextureId	_TextureIdNormal[NumTexture];
		CViewRenderer::CTextureId	_TextureIdPushed[NumTexture];
		CViewRenderer::CTextureId	_TextureIdOver[NumTexture];

		// setup
		void			setup();

	private:

		CViewText	*_ViewText;

		bool	_Setuped;
		bool	_IsViewTextId;
		bool    _ForceTextOver; // text is displayed over the "over" texture
		// Size of Bitmaps
		sint32	_BmpLeftW, _BmpMiddleW, _BmpRightW, _BmpH;
		// Value to add to TextW to get button W.
		sint32	_WMargin;
		// Min W Value
		sint32	_WMin;
		sint32	_TextY;
		sint32	_TextX;
		THotSpot _TextPosRef;
		THotSpot _TextParentPosRef;
		// Special Colors for text
		NLMISC::CRGBA	_TextColorNormal;
		NLMISC::CRGBA	_TextColorPushed;
		NLMISC::CRGBA	_TextColorOver;
		NLMISC::CRGBA	_TextShadowColorNormal;
		NLMISC::CRGBA	_TextShadowColorPushed;
		NLMISC::CRGBA	_TextShadowColorOver;
		bool			_TextModulateGlobalColorNormal;
		bool			_TextModulateGlobalColorPushed;
		bool			_TextModulateGlobalColorOver;
		bool			_TextHeaderColor;
	};

}

#endif // NL_CTRL_TEXT_BUTTON_H

/* End of ctrl_text_button.h */
