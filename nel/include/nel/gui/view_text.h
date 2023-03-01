// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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



#ifndef NL_VIEW_TEXT_H
#define NL_VIEW_TEXT_H

#include "nel/gui/view_base.h"
#include "nel/gui/string_case.h"
#include "nel/3d/u_text_context.h"

namespace NLGUI
{
	class CCtrlToolTip;

	/**
	 * class implementing a text view
	 * \author Matthieu 'TrapII' Besson
	 * \author Nicolas Vizerie
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewText : public CViewBase
	{
	public:
		enum TTextMode { ClipWord, DontClipWord, Justified, Centered };
	public:

		DECLARE_UI_CLASS(CViewText)


		/// Constructor
		CViewText (const TCtorParam &param);

		/// Constructor
		CViewText (const std::string& id, const std::string Text="", sint FontSize=12,
					NLMISC::CRGBA Color=NLMISC::CRGBA(255,255,255), bool Shadow=false, bool ShadowOutline=false);

		virtual ~CViewText();

		CViewText &operator=(const CViewText &vt);

		std::string getProperty( const std::string &name ) const;
		std::string getTextProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		bool setTextProperty( const std::string &name, const std::string &value );
		bool serializeTextOptions( xmlNodePtr node ) const;
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		void parseTextOptions (xmlNodePtr cur);
		bool parse (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		/// Updating
		virtual void draw ();
		void updateTextContext ();
		virtual void checkCoords();
		virtual void updateCoords();
		virtual	void onAddToGroup();
		virtual void onInterfaceScaleChanged();

		/// From CInterfaceElement
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		/// Accessors

		/// Set

		void setText(const std::string &text);
		void setTextLocalized(const std::string &text, bool localized);
#ifdef RYZOM_LUA_UCSTRING
		void setTextAsUtf16 (const ucstring &text); // Compatibility
#endif
		void setLocalized(bool localized);
		void setFontName (const std::string &name);
		void setFontSize (sint nFontSize, bool coef = true);
		void setEmbolden (bool nEmbolden);
		void setOblique (bool nOblique);
		void setColor (const NLMISC::CRGBA &color);
		void setShadow (bool bShadow);
		void setShadowOutline (bool bShadowOutline);
		void setShadowColor (const NLMISC::CRGBA &color);
		void setShadowOffset (sint x, sint y);
		void setLineMaxW (sint nMaxW, bool invalidate=true);
		void setOverflowText(const std::string &text) { _OverflowText = text; }
#ifdef RYZOM_LUA_UCSTRING
		void setOverflowTextAsUtf16(const ucstring &text) { _OverflowText = text.toUtf8(); } // Compatibility
#endif
		void setMultiLine (bool bMultiLine);
		void setMultiLineSpace (sint nMultiLineSpace);
		void setMultiLineMaxWOnly (bool state);
		void setMultiLineClipEndSpace (bool state);	// use it for multiline edit box for instance
		void setFirstLineX (sint firstLineX);
		void setMultiMaxLine(uint l) { _MultiMaxLine = l; }
		void setMultiMinLine(uint l) { _MultiMinLine = l; }

		// Override chars used to compute font size
		void setFontSizing(const std::string &chars, const std::string &fallback);

		// Force only a subset of letter to be displayed. Default is 0/0xFFFFFFFF
		void enableStringSelection(uint start, uint end);
		void disableStringSelection();
		void setShadowInSelection(bool s) { m_DisableShadowInSelection = !s; }
		bool getShadowInSelection() const { return !m_DisableShadowInSelection; }

		/// Get displayed text
		std::string		getText() const { return _Text; }
#ifdef RYZOM_LUA_UCSTRING
		ucstring		getTextAsUtf16() const; // Compatibility
		ucstring		getHardTextAsUtf16() const; // Compatibility
#endif
		bool			isLocalized() const { return _Localized;  }
		sint			getFontSize() const;
		std::string		getFontName() const { return _FontName; }
		bool			getEmbolden() 		{ return _Embolden; }
		bool			getOblique() 		{ return _Oblique; }
		NLMISC::CRGBA	getColor()			{ return _Color; }
		bool			getShadow()			{ return _Shadow; }
		bool			getShadowOutline()	{ return _ShadowOutline; }
		NLMISC::CRGBA	getShadowColor()	{ return _ShadowColor; }
		void			getShadowOffset(sint &x, sint &y) { x = _ShadowX; y = _ShadowY; }
		sint			getLineMaxW()		const { return _LineMaxW; }
#ifdef RYZOM_LUA_UCSTRING
		ucstring		getOverflowTextAsUtf16()	const { return _OverflowText; } // Compatibility
#endif
		bool			getMultiLine() const		{ return _MultiLine; }
		sint			getMultiLineSpace()	const	{ return _MultiLineSpace; }
		bool			getMultiLineMaxWOnly()	const	{ return _MultiLineMaxWOnly; }
		uint32			getMultiMaxLine() const { return _MultiMaxLine; }
		uint32			getMultiMinLine() const { return _MultiMinLine; }
		uint32			getMultiMinOffsetY() const;

		// get current Hint font width, in pixels
		uint            getFontWidth() const;
		// get current font height, in pixels
		uint            getFontHeight() const;
		// get current font leg height, in pixels
		uint            getFontLegHeight() const;
		// get current line height, in pixels
		float           getLineHeight() const;
		// Set the display mode (supported with multiline only for now)
		void			setTextMode(TTextMode mode);
		TTextMode		getTextMode() const	{ return _TextMode; }
		uint			getNumLine() const;
		uint			getFirstLineX() const;
		uint			getLastLineW () const;
		void			setUnderlined (bool underlined) { _Underlined = underlined; }
		bool			getUnderlined () const { return _Underlined; }
		void			setStrikeThrough (bool linethrough) { _StrikeThrough = linethrough; }
		bool			getStrikeThrough () const { return _StrikeThrough; }
		// true if the viewText is a single line clamped.
		bool			isSingleLineTextClamped() const {return _SingleLineTextClamped;}

		// Character positions

		/** Get position of the ith character, position are relative to the BR corner of the text.
		  * \param lineEnd. When set to true, return the coordinate of the previous line if the index is at the start of a line.
		  *  When looking at standard edit box, we see that if a line is split accross to line with no
		  * This also returns the height of the line
		  */
		void getCharacterPositionFromIndex(sint index, bool lineEnd, float &x, float &y, float &height) const;
		/** From a coordinate relative to the BR BR corner of the text, return the index of a character.
		  * If no character is found at the given position, the closest character is returned (first or last character, for the line or the whole text)
		  */
		void getCharacterIndexFromPosition(float x, float y, uint &index, bool &lineEnd) const;
		/** From a character index, get the index of the line it belongs to, or -1 if the index is invalid
		  * \param cursorDisplayedAtEndOfPreviousLine true if the cursor is displayed at the end of the previous line that match its index
		  */
		sint getLineFromIndex(uint index, bool cursorDisplayedAtEndOfPreviousLine = true) const;
		/// From a line number, get the character at which it starts, or -1 if invalid
		sint getLineStartIndex(uint line) const;
		/// From a line number, get the character at which it ends (not including any '\n' ), or -1 if invalid
		void getLineEndIndex(uint line, sint &index, bool &endOfPreviousLine) const;

		/// If localized, return localization key (ie "uiLanguage"), else return displayed text.
		std::string getHardText() const { return _HardText.empty() ? _Text : _HardText; }
		void        setHardText (const std::string &ht); //< Localizes strings starting with "ui"
#ifdef RYZOM_LUA_UCSTRING
		void		setHardTextAsUtf16(const ucstring &ht); // Compatibility
#endif

		std::string getColorAsString() const;
		void        setColorAsString(const std::string &ht);

		NLMISC::CRGBA getColorRGBA() const;
		void        setColorRGBA(NLMISC::CRGBA col);

		virtual sint32 getAlpha() const { return _Color.A; }
		virtual void setAlpha (sint32 a) { _ShadowColor.A = _Color.A = (uint8)a; }

		/** Setup a Text with Format Tags. Text is store without color/format tags, and special array is allocated for Format association
		 */
		void	setTextFormatTaged(const std::string &text);
#ifdef RYZOM_LUA_UCSTRING
		void	setTextFormatTagedAsUtf16(const ucstring &text); // Compatibility
#endif

		void	setSingleLineTextFormatTaged(const std::string &text);
#ifdef RYZOM_LUA_UCSTRING
		void	setSingleLineTextFormatTagedAsUtf16(const ucstring &text); // Compatibility
#endif

		// Remove end space
		void removeEndSpaces();

		// Reset the text index because the text context has changed
		void resetTextIndex();

		// Case mode
		void	setCaseMode (TCaseMode caseMode);
		TCaseMode	getCaseMode () const;

		// OverExtendViewText
		void	setOverExtendViewText(bool state) {_OverExtendViewText= state;}
		bool	getOverExtendViewText() const {return _OverExtendViewText;}

		// OverExtendViewTextUseParentRect
		void	setOverExtendViewTextUseParentRect(bool state) {_OverExtendViewTextUseParentRect= state;}
		bool	getOverExtendViewTextUseParentRect() const {return _OverExtendViewTextUseParentRect;}

		// see if text ellipsis if done at right side of the text
		bool	isClampRight() const { return _ClampRight; }

		int luaSetLineMaxW(CLuaState &ls);

		REFLECT_EXPORT_START(CViewText, CViewBase)
			REFLECT_BOOL("localize", isLocalized, setLocalized);
			REFLECT_STRING("hardtext", getHardText, setHardText); // Same as text, but localize is implicitly set true
			REFLECT_STRING("text", getText, setText);
			REFLECT_STRING("text_format", getText, setTextFormatTaged);
			REFLECT_STRING("text_single_line_format", getText, setSingleLineTextFormatTaged);
#ifdef RYZOM_LUA_UCSTRING
			// REFLECT_UCSTRING("uc_text", getTextAsUtf16, setTextAsUtf16); // Deprecate uc_ functions
			REFLECT_UCSTRING("uc_hardtext", getHardTextAsUtf16, setHardTextAsUtf16); // Compatibility
			REFLECT_UCSTRING("uc_hardtext_format", getTextAsUtf16, setTextFormatTagedAsUtf16); // Compatibility
			REFLECT_UCSTRING("uc_hardtext_single_line_format", getTextAsUtf16, setSingleLineTextFormatTagedAsUtf16); // Compatibility
#endif
			REFLECT_STRING ("color", getColorAsString, setColorAsString);
			REFLECT_RGBA ("color_rgba", getColorRGBA, setColorRGBA);
			REFLECT_SINT32 ("alpha", getAlpha, setAlpha);
			REFLECT_BOOL ("overExtendViewText", getOverExtendViewText, setOverExtendViewText);
			REFLECT_BOOL ("overExtendViewTextUseParentRect", getOverExtendViewTextUseParentRect, setOverExtendViewTextUseParentRect);
			REFLECT_LUA_METHOD("setLineMaxW", luaSetLineMaxW);
		REFLECT_EXPORT_END

		virtual void serial(NLMISC::IStream &f);

		// Sets the parent element
		// See the comment at the field
		void setParentElm( CInterfaceElement *parent ){ _ParentElm = parent; }

	protected:
		/// Text to display.
		std::string _HardTextFormat;
		std::string _HardText;
		std::string _Text;
		mutable sint _TextLength;
		bool _Localized;
		/// index of the computed String associated to this text control
		uint _Index;
		/// info on the computed String associated to this text control
		NL3D::UTextContext::CStringInfo _Info;
		/// Font name to get TextContext
		std::string _FontName;
		/// the font size
		sint	_FontSize;
		bool	_FontSizeCoef;
		bool	_Embolden;
		bool	_Oblique;
		// width of the font in pixel. 
		float	_FontWidth;
		// width of tabs
		float	_TabWidth;
		// strings to use when computing font size
		std::string _FontSizingChars;
		std::string _FontSizingFallback;
		// height of the font in pixel.
		// use getFontHeight
		float	_FontHeight;
		float	_FontLegHeight;
		float	_SpaceWidth;
		/// last UI scale used to calculate font size
		float	_Scale;
		/// the text color
		NLMISC::CRGBA _Color;
		/// the shadow mode
		bool	_Shadow;
		bool	_ShadowOutline;
		sint32	_ShadowX;
		sint32	_ShadowY;
		/// the case mode
		TCaseMode	_CaseMode;
		/// the text shadow color
		NLMISC::CRGBA _ShadowColor;
		/// Is the line (under p loop) should be considered at bottom (if false bottom is under p leg)
		/// maxw for the line/multiline
		sint32		_LineMaxW;
		/// For single line, true if the text is clamped (ie displayed with "...")
		bool		_SingleLineTextClamped;
		std::string	_OverflowText;

		/// Multiple lines handling
		bool		 _MultiLine;
		bool		 _MultiLineMaxWOnly;
		bool		 _MultiLineClipEndSpace;
		uint8		 _AutoClampOffset;
		TTextMode    _TextMode;
		sint		_MultiLineSpace;
		sint		_LastMultiLineMaxW;
		uint32		_MultiMaxLine;
		uint32		_MultiMinLine;


		/// FormatTag handling
		struct CFormatInfo
		{
			// The color to change
			NLMISC::CRGBA	Color;
			// The Tabulation to apply, in number of "_" characters.
			uint			TabX;
			// Index in vector
			sint			IndexTt;

			CFormatInfo()
			{
				Color= NLMISC::CRGBA::White;
				TabX= 0;
				IndexTt = -1;
			}

			bool	operator==(const CFormatInfo &o) const {return Color==o.Color && TabX==o.TabX && IndexTt==o.IndexTt;}
			bool	operator!=(const CFormatInfo &o) const {return !operator==(o);}
		};
		struct CFormatTag : public CFormatInfo
		{
			uint			Index;

			// compare 2 tags, not a tag and a CFormatInfo
			bool	sameTag(const CFormatTag &o) const
			{
				return CFormatInfo::operator==(o) && Index==o.Index;
			}
		};
		std::vector<CFormatTag>		_FormatTags;

		/// Get the current maxW for multiline, accordgin to parent and _MultiLineOptionMaxW
		sint		getCurrentMultiLineMaxW() const;




		NL3D::ULetterColors * _LetterColors;

	private:
		// A word in a line
		class CWord
		{
			public:
				// default ctor
				CWord(uint numSpaces = 0) : Index(0), NumSpaces(numSpaces) {}
				std::string							Text;
				uint								Index; // index of the info for this word
				NL3D::UTextContext::CStringInfo		Info;
				uint								NumSpaces; // number of spaces before this word
				// The specialized color/format of this word. White if none
				CFormatInfo							Format;
			public:
				// build from a string, using the current text context
				void build(const std::string &text, NL3D::UTextContext &textContext, uint numSpaces= 0);
		};
		typedef std::vector<CWord> TWordVect;

		// A line of text (which is made of one word with space, or of several words with no spaces in them)
		class CLine : public NLMISC::CRefCount
		{
			public:
				// ctor
				CLine();
				// Clear the line & remove text contexts
				void clear(NL3D::UTextContext &textContext);
				// Add a new word (and its context) in the line + a number of spaces to append at the end of the line
				void	addWord(const std::string &word, uint numSpaces, const CFormatInfo &wordFormat, float fontWidth, float tabWidth, NL3D::UTextContext &textContext);
				void    addWord(const CWord &word, float fontWidth, float tabWidth);
				uint	getNumWords() const { return (uint)_Words.size(); }
				CWord   &getWord(uint index) { return _Words[index]; }
				float	getSpaceWidth() const { return _SpaceWidth; }
				void    setSpaceWidth(float width) { _SpaceWidth = width; }
				// Get the number of chars in the line, not counting the end spaces, but couting the spaces in words
				uint	getNumChars() const { return _NumChars; }
				// Get the total number of spaces between words (not including those in words, but there should not be if text is justified)
				uint	getNumSpaces() const { return _NumSpaces; }
				float   getStringLine() const { return _StringLine; }
				float   getWidthWithoutSpaces() const { return _WidthWithoutSpaces; }
				// get total width including spaces, but not including end spaces
				float   getWidth() const { return _WidthWithoutSpaces + _SpaceWidth * _NumSpaces; }
				// Get the number of spaces at the end of the line
				void	setEndSpaces(uint numSpaces) { _EndSpaces = numSpaces; }
				// Set the number of spaces at the end of the line
				uint    getEndSpaces() const { return _EndSpaces; }
				// Test if there's a line feed at the end of the line
				bool    getLF() const { return _HasLF; }
				void	setLF(bool lf) { _HasLF = lf; }
				void	resetTextIndex();
			private:
				TWordVect _Words;
				uint	  _NumChars;
				uint      _NumSpaces;
				float	  _SpaceWidth; // width of a space, in pixels (used with multispace alignment)
				float     _StringLine;
				float     _WidthWithoutSpaces; // width without space (see the Field NumSpaces in the CWord class).
											   // NB : space inserted inside a word are counted, however!
				uint	  _EndSpaces; // spaces at the end of the line
				bool	  _HasLF; // a linefeed is at end of line (no breaking due to line full)
		};
		/// NB : we keep pointers on lines (each line contains a vector, that we don't want to be copied, and this occurs as the vector of lines grows..)

		typedef NLMISC::CSmartPtr<CLine> TLineSPtr;
		typedef std::vector<TLineSPtr> TLinePtVect;
	private:
		/** Data of the updated text for multiline. It is built from the _Text field in the updateTextContext member function,
		  * and is used to perform the draw
		  */
		TLinePtVect _Lines;

		/// if true, and if the view text is isSingleLineTextClamped(), then an over will be drawn, with the text
		bool	_OverExtendViewText : 1;
		/// if true and _OverExtendViewText true too, use the parent rectangle to know if must display the over or not
		bool	_OverExtendViewTextUseParentRect : 1;
		/// Letter selection handling
		bool	_AutoClamp : 1;
		bool	_ClampRight : 1;
		bool	_TextSelection : 1;
		bool	_InvalidTextContext : 1;
		bool	_Underlined : 1;
		bool	_StrikeThrough : 1;
		bool    _ContinuousUpdate : 1;
		bool	_Setuped : 1;
		bool	m_DisableShadowInSelection : 1;

		uint	_TextSelectionStart;
		uint	_TextSelectionEnd;

		// First line X coordinate
		float	_FirstLineX;

		/// Dynamic tooltips
		std::vector<CCtrlToolTip*>	_Tooltips;

		// Parent element is the element where this text belongs to
		// For example: text button
		CInterfaceElement *_ParentElm;

	private:
		void setup ();
		void setupDefault ();

		void setTextLocalized(const std::string &text);

		void setStringSelectionSkipingSpace(uint stringId, const std::string &text, sint charStart, sint charEnd);

	//	void pushString(const ucstring &str, bool deleteSpaceAtStart = false); // OLD

		/// \from CInterfaceElement
		void onInvalidateContent();

		// may append a new line, and append a word to the last line (no spaces)
		void flushWordInLine(std::string &ucCurrentWord, bool &linePushed, const CFormatInfo &wordFormat);
		// Clear all the lines and free their datas
		void clearLines();
		// Update in the case of a multiline text
		void updateTextContextMultiLine(float nMaxWidth);
		// Update in the case of a multiline text with justification
		void updateTextContextMultiLineJustified(float nMaxWidth, bool expandSpaces);
		// Recompute font size info
		void computeFontSize ();

		// used for "donctClipWord" case in updateTextContextMultiLineJustified(). currLine is reseted
		void	addDontClipWordLine(std::vector<CWord> &currLine);

		// FormatTag build.
		static void		buildFormatTagText(const std::string &text, std::string &textBuild, std::vector<CFormatTag> &formatTags, std::vector<std::string> &tooltips);
		// FormatTag parsing.
		bool			isFormatTagChange(uint textIndex, uint ctIndex) const;
		void			getFormatTagChange(uint textIndex, uint &ctIndex, CFormatInfo &wordFormat) const;
	};

}

#endif // NL_VIEW_TEXT_H

/* End of view_text.h */
