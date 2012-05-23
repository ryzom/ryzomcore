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

#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"

#include "view_text.h"
#include "interface_manager.h"
#include "group_container.h" // CCtrlResizer
#include "ctrl_tooltip.h"

#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_ihm.h"
#include "lua_ihm_ryzom.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

typedef std::string::size_type TCharPos; // index of a chracter in a string

REGISTER_UI_CLASS(CViewText)

// ***************************************************************************
void CViewText::setupDefault ()
{
	_CaseMode = CaseNormal;
	_Underlined = false;
	_ContinuousUpdate = false;
	_Active = true;
	_X = 0;
	_Y = 0;
	_W = 0;;
	_H = 0;
	_SizeRef = 0;
	_SizeDivW = 10;
	_SizeDivH = 10;
	_ParentPosRef = Hotspot_BL;
	_PosRef = Hotspot_BL;

	_FontSize = 12+CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
	_Color = CRGBA(255,255,255,255);
	_Shadow = false;
	_ShadowColor = CRGBA(0,0,0,255);

	_MultiLine = false;
	_TextMode = DontClipWord;
	_MultiLineSpace = 8;
	_LineMaxW = 16384;
	_MultiLineMaxWOnly = false;
	_MultiLineClipEndSpace = false;
	_LastMultiLineMaxW = 0;
	_MultiMaxLine = 0;
	_Index = 0xFFFFFFFF;

	_FontWidth= 0;
	_FontHeight = 0;
	_FontLegHeight = 0;

	_TextSelection= false;
	_TextSelectionStart= 0;
	_TextSelectionEnd= std::numeric_limits<uint>::max();

	_InvalidTextContext= true;
	_FirstLineX = 0;
	computeFontSize ();

	_SingleLineTextClamped= false;
	_OverExtendViewText= false;
	_OverExtendViewTextUseParentRect= false;

	_AutoClamp = false;
	_ClampRight = true; // clamp on the right of the text

	_LetterColors = NULL;
	_Setuped= false;
	_AutoClampOffset = 0;
}

// ***************************************************************************

NLMISC_REGISTER_OBJECT(CViewBase, CViewText, std::string, "text");

CViewText::CViewText(const TCtorParam &param)
:CViewBase(param)
{
	setupDefault ();
}

///constructor
// ***************************************************************************
CViewText::	CViewText (const std::string& id, const std::string Text, sint FontSize,
						NLMISC::CRGBA Color, bool Shadow)
						:CViewBase(TCtorParam())
{
	_Id = id;
	setupDefault ();

	_FontSize = FontSize+CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
	_Color = Color;
	_Shadow = Shadow;
	setText(Text);
	computeFontSize ();
}

// ***************************************************************************
CViewText::~CViewText()
{
	if (_Index != 0xFFFFFFFF)
		CInterfaceManager::getInstance()->getTextContext()->erase (_Index);
	clearLines();

	if (!_Setuped)
	for (uint i=0 ; i<_Tooltips.size() ; ++i)
		delete _Tooltips[i];

	_Tooltips.clear();
}

// ***************************************************************************
CViewText &CViewText::operator=(const CViewText &vt)
{
	if (_Index != 0xFFFFFFFF)
		CInterfaceManager::getInstance()->getTextContext()->erase (_Index);

	// Create database entries
	_Active = vt._Active;
	_X = vt._X;
	_Y = vt._Y;
	_W = vt._W;
	_H = vt._H;
	_SizeRef = vt._SizeRef;
	_SizeDivW = vt._SizeDivW;
	_SizeDivH = vt._SizeDivH;
	_ParentPosRef = vt._ParentPosRef;
	_PosRef = vt._PosRef;

	_FontSize = vt._FontSize;
	_Color = vt._Color;
	_Shadow = vt._Shadow;
	_ShadowColor = vt._ShadowColor;

	_MultiLine = false;
	_MultiLineSpace = 8;
	_LineMaxW= 16384;
	_MultiLineMaxWOnly = false;
	_MultiLineClipEndSpace = false;
	_LastMultiLineMaxW = 0;
	_Index = 0xFFFFFFFF;

	_ModulateGlobalColor= vt._ModulateGlobalColor;


	// remove previous lines
	clearLines();
	_InvalidTextContext = true;
	computeFontSize ();

	return *this;
}

// ***************************************************************************
void CViewText::parseTextOptions (xmlNodePtr cur)
{
	CXMLAutoPtr prop;

	prop= (char*) xmlGetProp( cur, (xmlChar*)"color" );
	_Color = CRGBA(255,255,255,255);
	if (prop)
		_Color = convertColor(prop);

	prop= (char*) xmlGetProp (cur, (xmlChar*)"global_color");
	if(prop)
		_ModulateGlobalColor= convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"fontsize" );
	_FontSize = 12+CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
	if (prop)
	{
		fromString((const char*)prop, _FontSize);
		_FontSize += CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"shadow" );
	_Shadow = false;
	if (prop)
		_Shadow = convertBool(prop);

	prop= (char*) xmlGetProp( cur, (xmlChar*)"shadow_color" );
	_ShadowColor = CRGBA(0,0,0,255);
	if (prop)
		_ShadowColor = convertColor(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_line" );
	_MultiLine = false;
	if (prop)
		_MultiLine = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"justification" );
	if (prop)
	{
		if (nlstricmp("clip_word", (const char *) prop) == 0)            _TextMode = ClipWord;
		else if (nlstricmp("dont_clip_word", (const char *) prop) == 0)  _TextMode = DontClipWord;
		else if (nlstricmp("justified", (const char *) prop) == 0)       _TextMode = Justified;
		else nlwarning("<CViewText::parse> bad text mode");
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"line_maxw" );
	_LineMaxW = 16384;
	if (prop)
		fromString((const char*)prop, _LineMaxW);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_line_space" );
	_MultiLineSpace = 8;
	if (prop)
		fromString((const char*)prop, _MultiLineSpace);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_line_maxw_only" );
	_MultiLineMaxWOnly = false;
	if (prop)
		_MultiLineMaxWOnly = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_max_line" );
	_MultiMaxLine = 0;
	if (prop)
		fromString((const char*)prop, _MultiMaxLine);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"underlined" );
	_Underlined = false;
	if (prop)
		_Underlined = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"case_mode" );
	_CaseMode = CaseNormal;
	if (prop)
	{
		sint32 caseMode;
		fromString((const char*)prop, caseMode);
		_CaseMode = (TCaseMode)caseMode;
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"over_extend_view_text" );
	_OverExtendViewText= false;
	if(prop)
		_OverExtendViewText= convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"over_extend_parent_rect" );
	_OverExtendViewTextUseParentRect= false;
	if(prop)
		_OverExtendViewTextUseParentRect= convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"auto_clamp" );
	_AutoClamp = false;
	if (prop)
		_AutoClamp = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"clamp_right" );
	_ClampRight = true;
	if (prop)
		_ClampRight = convertBool(prop);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"auto_clamp_offset" );
	_AutoClampOffset = 0;
	if (prop)
		fromString((const char*)prop, _AutoClampOffset);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"continuous_update" );
	if (prop)
	{
		_ContinuousUpdate = convertBool(prop);
	}


	computeFontSize ();
}

/*
* parse an xml node and initialize the base view mambers. Must call CViewBase::parse
* \param cur : pointer to the xml node to be parsed
* \param parentGroup : the parent group of the view
* \partam id : a refence to the string that will receive the view ID
* \return true if success
*/
// ***************************************************************************
bool CViewText::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{

//	const ucstring* tmp;
	CXMLAutoPtr prop;
	//try to get props that can be inherited from groups
	//if a property is not defined, try to find it in the parent group.
	//if it is undefined, set it to zero

	if (! CViewBase::parse(cur,parentGroup) )
		return false;

	//set w and h to 0 : they depend on the string contained
	_W = 0;
	_H = 0;

	//try to get the NEEDED specific props
	parseTextOptions(cur);

	prop = (char*) xmlGetProp( cur, (xmlChar*)"hardtext" );
	if (prop)
	{
		const char *propPtr = prop;
		_Text = ucstring(propPtr);

		if ((strlen(propPtr)>2) && (propPtr[0] == 'u') && (propPtr[1] == 'i'))
			_Text = CI18N::get (propPtr);
		setCase (_Text, _CaseMode);
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"hardtext_format" );
	if (prop)
	{
		const char *propPtr = prop;
		if (_MultiLine)
		{
			setTextFormatTaged(CI18N::get(propPtr));
		}
		else
		{
			setSingleLineTextFormatTaged(CI18N::get(propPtr));
		}
	}

	invalidateContent ();

	return true;
}

// ***************************************************************************
sint	CViewText::getCurrentMultiLineMaxW() const
{
	if(_MultiLineMaxWOnly)
		return _LineMaxW;
	else
	{
		sint parentWidth = std::min(_Parent->getMaxWReal(), _Parent->getWReal());
		return std::min(parentWidth-(sint)(_XReal-_Parent->getXReal()), (sint)_LineMaxW);
	}
}


// ***************************************************************************
void CViewText::checkCoords ()
{
	if ((_MultiLine)&&(_Parent != NULL))
	{
		// If never setuped, and if text is not empty
		if (_Lines.size() == 0 && !_Text.empty())
			invalidateContent ();

		sint	currentMaxW= getCurrentMultiLineMaxW();
		if ( _LastMultiLineMaxW != currentMaxW )
		{
			if (_ContinuousUpdate)
			{
				_LastMultiLineMaxW = currentMaxW;
				invalidateContent();
			}
			else
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CCtrlBase *pCB = pIM->getCapturePointerLeft();
				if (pCB != NULL)
				{
					CCtrlResizer *pCR = dynamic_cast<CCtrlResizer*>(pCB);
					if (pCR != NULL)
					{
						// We are resizing !!!!
					}
					else
					{
						_LastMultiLineMaxW = currentMaxW;
						invalidateContent();
					}
				}
				else
				{
					_LastMultiLineMaxW = currentMaxW;
					invalidateContent();
				}
			}
		}
	}
	else
	{
		if (_Index == 0xFFFFFFFF)
			invalidateContent ();
	}
}
/*
* draw the view
*/
// ***************************************************************************
void CViewText::draw ()
{
	H_AUTO( RZ_Interface_CViewText_draw  )

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = pIM->getViewRenderer();

	// *** Out Of Clip?
	sint32 ClipX, ClipY, ClipW, ClipH;
	rVR.getClipWindow (ClipX, ClipY, ClipW, ClipH);
	if (((_XReal) > (ClipX+ClipW)) || ((_XReal+_WReal) < ClipX) ||
		((_YReal) > (ClipY+ClipH)) || ((_YReal+_HReal) < ClipY))
		return;

	// *** Screen Minimized?
	uint32	w, h;
	float	oow, ooh;
	rVR.getScreenSize (w, h);
	if (rVR.isMinimized())
		return;
	rVR.getScreenOOSize (oow, ooh);

	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();


	// *** get current color
	CRGBA col, shcol;
	if(getModulateGlobalColor())
	{
		CRGBA gcfc = pIM->getGlobalColorForContent();
		col.modulateFromColor (_Color, gcfc);
		shcol.modulateFromColor (_ShadowColor, gcfc);
	}
	else
	{
		col = _Color;
		shcol = _ShadowColor;
		col.A = (uint8)(((sint)col.A*((sint)pIM->getGlobalColorForContent().A+1))>>8);
		shcol.A = (uint8)(((sint)shcol.A*((sint)pIM->getGlobalColorForContent().A+1))>>8);
	}


	// *** Draw multiline
	if ((_MultiLine)&&(_Parent != NULL))
	{
		if (_Lines.size() == 0) return;

		NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();

		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeColor (shcol);
		TextContext->setFontSize (_FontSize);

		float y = (float)(_YReal) * ooh; // y is expressed in scree, coordinates [0..1]
		//y += _LinesInfos[_LinesInfos.size()-1].StringLine / h;

		// Y is the base line of the string, so it must be grown up.
		y += (float)_FontLegHeight * ooh;

		sint y_line = _YReal+_FontLegHeight-2;

		// special selection code
		if(_TextSelection)
		{
			sint charIndex = 0;
			for (sint i = 0; i<(sint)_Lines.size(); i++)
			{
				CLine &currLine = *_Lines[i];
				for(uint k = 0; k < currLine.getNumWords(); ++k)
				{
					CWord &currWord = currLine.getWord(k);
					charIndex += currWord.NumSpaces;
					sint	cStart= max(charIndex, (sint)_TextSelectionStart);
					sint	cEnd= min(charIndex+(sint)currWord.Text.length(), (sint)_TextSelectionEnd);

					// range must be valid
					if(cStart<cEnd)
					{
						// select subset. Arg, must skip spaces because not inserted in VertexBuffer.
						setStringSelectionSkipingSpace(currWord.Index, currWord.Text, cStart-charIndex, cEnd-charIndex);
					}
					else
					{
						// select none
						TextContext->setStringSelection(currWord.Index, 0, 0);
					}

					// next word
					charIndex+= (sint)currWord.Text.length();
				}
				charIndex += currLine.getEndSpaces() + (currLine.getLF() ? 1 : 0);
			}
		}

		// draw
		for (sint i = (sint)_Lines.size()-1; i >= 0; --i)
		{
			CLine &currLine = *_Lines[i];
			// current x position
			float px = (float) (_XReal + ((i==0) ? (sint)_FirstLineX : 0));
			// draw each words of the line
			for(uint k = 0; k < currLine.getNumWords(); ++k)
			{
				CWord &currWord = currLine.getWord(k);

				// Change the current color
				if(currWord.Format.Color==CRGBA::White)
					TextContext->setStringColor(currWord.Index, col);
				else
				{
					CRGBA	mCol;
					mCol.modulateFromColor(col, currWord.Format.Color);
					TextContext->setStringColor(currWord.Index, mCol);
				}

				// skip spaces before current word
				float firstSpace = currWord.NumSpaces * currLine.getSpaceWidth();
				sint line_width = 0;
				if (_Underlined)
				{
					line_width = (sint)floorf(currLine.getWidthWithoutSpaces() + currLine.getSpaceWidth());
					line_width -= (sint)floorf(firstSpace);
				}
				px += firstSpace;
				// skip tabulation before current word
				if(currWord.Format.TabX)
					px= max(px, (float)(_XReal + currWord.Format.TabX*_FontWidth));

				// draw. We take floorf px to avoid filtering of letters that are not located on a pixel boundary
				rVR.drawText (_RenderLayer, floorf(px) * oow, y, currWord.Index, (float)ClipX * oow, (float)ClipY * ooh,
					(float)(ClipX+ClipW) * oow, (float)(ClipY+ClipH) * ooh, *TextContext);

				// Draw a line
				if (_Underlined)
					rVR.drawRotFlipBitmap (_RenderLayer, (sint)floorf(px), y_line, line_width, 1, 0, false, rVR.getBlankTextureId(), col);

				// skip word
				px += currWord.Info.StringWidth;
			}
			// go one line up
			y += (_FontHeight + _MultiLineSpace) * ooh;
			y_line += _FontHeight+_MultiLineSpace;
		}

		// reset selection
		if(_TextSelection)
		{
			for (sint i = 0; i<(sint)_Lines.size(); i++)
			{
				CLine &currLine = *_Lines[i];
				for(uint k = 0; k < currLine.getNumWords(); ++k)
				{
					TextContext->resetStringSelection(currLine.getWord(k).Index);
				}
			}
		}
	}
	// *** Single Line Version (currently no support for text justification)
	else
	{
		nlassert(_Index != 0xFFFFFFFF);

		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeColor (shcol);
		TextContext->setFontSize (_FontSize);


		if(_LetterColors!=NULL && !TextContext->isSameLetterColors(_LetterColors, _Index))
		{
			TextContext->setLetterColors(_LetterColors, _Index);
		}


		float x = (float)(_XReal) * oow;
		float y = (float)(_YReal) * ooh;

		// Y is the base line of the string, so it must be grown up.
		y += (float)_FontLegHeight * ooh;

		// special selection code
		if(_TextSelection)
			// select subset. Arg, must skip spaces because not inserted in VertexBuffer.
			setStringSelectionSkipingSpace(_Index, _Text, _TextSelectionStart, _TextSelectionEnd);

		// Change the current color
		TextContext->setStringColor(_Index, col);

		// draw
		rVR.drawText (_RenderLayer, x, y, _Index, (float)ClipX * oow, (float)ClipY * ooh,
					(float)(ClipX+ClipW) * oow, (float)(ClipY+ClipH) * ooh, *TextContext);

		// Draw a line
		if (_Underlined)
			rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal+_FontLegHeight-2, _WReal, 1, 0, false, rVR.getBlankTextureId(), col);

		// reset selection
		if(_TextSelection)
			TextContext->resetStringSelection(_Index);

		// if single line clamped, may allow to draw an over
		if(isSingleLineTextClamped() && _OverExtendViewText && pIM->getPointer())
		{
			// but must check first if mouse is over
			sint32 x = pIM->getPointer()->getX();
			sint32 y = pIM->getPointer()->getY();
			bool	mouseIn;
			// use parent clip or self clip?
			if(_OverExtendViewTextUseParentRect)
				mouseIn= _Parent && _Parent->isIn(x,y);
			else
				mouseIn= isIn(x,y);
			// if the mouse cursor is in the clip area
			if(mouseIn)
			{
				// check the window under the mouse is the root window
				CInterfaceGroup		*pIG = pIM->getWindowUnder(x,y);
				CInterfaceElement	*pParent = this;
				bool bFound = false;
				while (pParent != NULL)
				{
					if (pParent == pIG)
					{
						bFound = true;
						break;
					}
					pParent = pParent->getParent();
				}

				// ok => let this view text be the extend over one
				if(bFound)
				{
					// last check: the window must not be currently moved
					CGroupContainer *gc= dynamic_cast<CGroupContainer*>(pIG);
					if(!gc || !gc->isMoving())
					{
						CRGBA	col= pIM->getSystemOption(CInterfaceManager::OptionViewTextOverBackColor).getValColor();
						pIM->setOverExtendViewText(this, col);
					}
				}
			}
		}
	}

}

// ***************************************************************************
void CViewText::onAddToGroup()
{
	// Add tooltips if not done
	if(!_Setuped)
		setup();
}

// ***************************************************************************
void CViewText::setTextMode(TTextMode mode)
{
	if (mode != _TextMode)
	{
		_TextMode = mode;
		invalidateContent ();
	}
}

// ***************************************************************************
void CViewText::setText(const ucstring & text)
{
	// common case: no special format, no case mode => easy cache test
	if (_FormatTags.empty() && _CaseMode==CaseNormal)
	{
		if (text != _Text)
		{
			_Text = text;
			// no need to call  "setCase (_Text, _CaseMode);"  since CaseNormal:
			invalidateContent ();
		}
	}
	else
	{
		// if the view text had some format before, no choice, must recompute all
		if(!_FormatTags.empty())
		{
			_Text = text;
			setCase (_Text, _CaseMode);
			invalidateContent ();
		}
		// else test if after the case change the cache succeed
		else
		{
			// compute the temp cased text
			ucstring	tempText= text;
			setCase(tempText, _CaseMode);
			if(tempText!=_Text)
			{
				_Text = tempText;
				invalidateContent ();
			}
		}
	}

	// clear format tags if any
	_FormatTags.clear();
}

// ***************************************************************************
void CViewText::setFontSize (sint nFontSize)
{
	_FontSize = nFontSize+CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
	computeFontSize ();
	invalidateContent();
}

// ***************************************************************************
sint CViewText::getFontSize() const
{
	return _FontSize - CInterfaceManager::getInstance()->getSystemOption(CInterfaceManager::OptionAddCoefFont).getValSInt32();
}

// ***************************************************************************
void CViewText::setColor(const NLMISC::CRGBA & color)
{
	_Color = color;
}

// ***************************************************************************
void CViewText::setShadow (bool bShadow)
{
	_Shadow = bShadow;
	computeFontSize ();
	invalidateContent();
}

// ***************************************************************************
void CViewText::setShadowColor(const NLMISC::CRGBA & color)
{
	_ShadowColor = color;
}

// ***************************************************************************
void CViewText::setLineMaxW (sint nMaxW, bool invalidate)
{
	if(_LineMaxW!=nMaxW)
	{
		_LineMaxW = nMaxW;
		if (invalidate)
			invalidateContent();
	}
}

// ***************************************************************************
int CViewText::luaSetLineMaxW(CLuaState &ls)
{
	CLuaIHM::checkArgCount(ls, "setLineMaxW", 1);
	sint32 value;
	if(CLuaIHM::popSINT32(ls, value))
	{
		setLineMaxW(value);
	}
	return 0;
}

// ***************************************************************************
void CViewText::setMultiLine (bool bMultiLine)
{
	_MultiLine = bMultiLine;
	invalidateContent();
}

// ***************************************************************************
void CViewText::setMultiLineSpace (sint nMultiLineSpace)
{
	_MultiLineSpace = nMultiLineSpace;
	invalidateContent();
}

// ***************************************************************************
void CViewText::setMultiLineMaxWOnly (bool state)
{
	_MultiLineMaxWOnly = state;
	invalidateContent();
}

// ***************************************************************************
void CViewText::setMultiLineClipEndSpace (bool state)
{
	_MultiLineClipEndSpace= state;
	invalidateContent();
}

// ***************************************************************************
uint CViewText::getFontWidth() const
{
	return _FontWidth;
}

// ***************************************************************************
uint CViewText::getFontHeight() const
{
	return _FontHeight;
}

// ***************************************************************************
uint CViewText::getFontLegHeight() const
{
	return _FontLegHeight;
}

// ***************************************************************************
void CViewText::flushWordInLine(ucstring &ucCurrentWord, bool &linePushed, const CFormatInfo &wordFormat)
{
	// create a new line?
	if(!linePushed)
	{
		_Lines.push_back(TLineSPtr(new CLine));
		linePushed= true;
	}
	// Append to the last line
	_Lines.back()->addWord(ucCurrentWord, 0, wordFormat, _FontWidth);
	// reset the word
	ucCurrentWord = ucstring("");
}


// ***************************************************************************
void CViewText::updateTextContextMultiLine(uint nMaxWidth)
{
	ucchar ucLetter;
	UTextContext::CStringInfo si;
	uint i;
	// word state
	ucstring ucCurrentWord;
	CFormatInfo		wordFormat;
	// line state
	float	rWidthCurrentLine = 0, rWidthLetter;
	bool	linePushed= false;
	// for all the text
	uint	textSize= (uint)_Text.size();
	uint	formatTagIndex= 0;
	for (i = 0; i < textSize; ++i)
	{
		if(isFormatTagChange(i, formatTagIndex))
		{
			// If the word was not empty before this color tag
			if(!ucCurrentWord.empty())
				flushWordInLine(ucCurrentWord, linePushed, wordFormat);

			// get new color and skip ctIndex.
			getFormatTagChange(i, formatTagIndex, wordFormat);

			// Ensure the line witdh count the tab
			rWidthCurrentLine= max(rWidthCurrentLine, (float)wordFormat.TabX*_FontWidth);
		}

		NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();

		// Parse the letter
		{
			ucLetter = _Text[i];
			if (ucLetter == ucchar('\n'))
			{
				flushWordInLine(ucCurrentWord, linePushed, wordFormat);
				// reset line state
				linePushed= false;
				rWidthCurrentLine = 0;
			}
			else
			{
				ucstring ucStrLetter;
				ucStrLetter= ucLetter;
				si = TextContext->getStringInfo (ucStrLetter);
				rWidthLetter = (si.StringWidth);
				if ((rWidthCurrentLine + rWidthLetter) > nMaxWidth)
				{
					flushWordInLine(ucCurrentWord, linePushed, wordFormat);

					// reset line state, and begin with the cut letter
					linePushed= false;
					rWidthCurrentLine = rWidthLetter;
					ucCurrentWord = ucLetter;
				}
				else
				{
					// Grow the current word
					ucCurrentWord += ucLetter;
					rWidthCurrentLine += rWidthLetter;
				}
			}
		}
	}
	if (ucCurrentWord.length())
	{
		flushWordInLine(ucCurrentWord, linePushed, wordFormat);
	}
}


// ***************************************************************************
void CViewText::addDontClipWordLine(std::vector<CWord> &currLine)
{
	// create a new line
	_Lines.push_back(TLineSPtr(new CLine));

	// Fill it with words. if all words of same color, create only one CWord
	if (!currLine.empty())
	{
		CFormatInfo	lineWordFormat= currLine[0].Format;
		ucstring	lineWord;
		for(uint i=0;i<currLine.size();i++)
		{
			// If different from last, flush
			if(currLine[i].Format!=lineWordFormat)
			{
				// add the current lineWord to the line.
				_Lines.back()->addWord(lineWord, 0, lineWordFormat, _FontWidth);
				// get new lineWordFormat
				lineWordFormat= currLine[i].Format;
				// and clear
				lineWord.clear();
			}

			// Append the word with space to the lineWord.
			ucstring blank;
			blank.resize(currLine[i].NumSpaces, (ucchar) ' ');
			lineWord += blank;
			lineWord += currLine[i].Text;
		}

		if(!lineWord.empty())
			_Lines.back()->addWord(lineWord, 0, lineWordFormat, _FontWidth);

		// clear
		currLine.clear();
	}
}

// ***************************************************************************
void CViewText::updateTextContextMultiLineJustified(uint nMaxWidth, bool expandSpaces)
{
	UTextContext::CStringInfo si;
	//
	TCharPos currPos = 0;
	//
	static const ucstring spaceStr(" ");
	// precLineWidth valid only id precedent line is part of same paragraph.
	float precLineWidth= 0;
	float lineWidth = (float)_FirstLineX; // width of the current line
	uint  numWordsInLine = 0; // number of words in the current line
	bool  isParagraphStart = true; // A paragraph is a group of characters between 2 \n
	bool  lineFeed;
	bool  breakLine;
	//
	vector<CWord>	currLine; // if spaces are not expanded, all words of a line are inserted here (NB: index and stringInfo not filled)
	ucstring	wordValue;
	CFormatInfo	wordFormat;
	uint		formatTagIndex= 0;
	//
	while (currPos != _Text.length())
	{
		TCharPos spaceEnd;
		TCharPos wordEnd;
		uint numSpaces;
		float newLineWidth;
		breakLine = false;
		//
		if (_Text[currPos] == (ucchar) '\n')
		{
			lineFeed = true;
		}
		else
		{
			lineFeed = false;
			// Skip spaces and count them
			spaceEnd = _Text.find_first_not_of(spaceStr, currPos);
			if (spaceEnd == std::string::npos)
			{
				spaceEnd = _Text.length();
			}
			numSpaces = (uint) (spaceEnd - currPos);
			if (!isParagraphStart && numSpaces != 0 && numWordsInLine == 0) // Are these the first spaces of the line ?
			{
				if (!_Lines.empty())
				{
					/* Yoyo: I changed this (added the "cut space"), because in editBox, it is so strange when
						the word hit the end of line, and if you add spaces just after, nothing happens because
						cursor pos is clamped at end of line.
					*/
					// Cannot put all of thoses spaces to the prec end of line?
					if(_MultiLineClipEndSpace && precLineWidth + numSpaces * _SpaceWidth > nMaxWidth)
					{
						// put some of these spaces at the end of the previous line.
						sint	maxNumSpaces= (sint)floorf((nMaxWidth - precLineWidth) / _SpaceWidth);
						_Lines.back()->setEndSpaces(maxNumSpaces);
						// And start the new lines with the remaining spaces.
						numSpaces-= maxNumSpaces;
						currPos+= maxNumSpaces;
					}
					else
					{
						// ok, put all spaces to previous line
						_Lines.back()->setEndSpaces(numSpaces);
						currPos = spaceEnd;
						numSpaces= 0;
					}
					if(currPos >=_Text.length())
						break;
				}
			}

			// Detect change of wordFormat at the begining of the word
			if(isFormatTagChange((uint)spaceEnd, formatTagIndex))
			{
				getFormatTagChange((uint)spaceEnd, formatTagIndex, wordFormat);
			}

			// Get word until a space, a \n, or a FormatTagChange is encountered
			uint	i;
			for(i= (uint)spaceEnd;i<(uint)_Text.length();i++)
			{
				ucchar	c= _Text[i];
				if(c==' ' || c=='\n')
					break;
				// If change of color tag, stop the word, but don't take the new color now.
				if(isFormatTagChange(i, formatTagIndex))
					break;
			}
			wordEnd = i;


			// Get the word value.
			wordValue = _Text.substr(spaceEnd, wordEnd - spaceEnd);
			// compute width of word
			si = CInterfaceManager::getInstance()->getTextContext()->getStringInfo(wordValue);

			// compute size of spaces/Tab + word
			newLineWidth = lineWidth + numSpaces * _SpaceWidth;
			newLineWidth = max(newLineWidth, (float)wordFormat.TabX*_FontWidth);
			newLineWidth+= si.StringWidth;
		}
		//
		// Does the word go beyond the end of line ?
		if (!lineFeed && newLineWidth > (float) nMaxWidth)
		{
			// Have we enough room for this word on a line ?
			bool roomForThisWord = (numWordsInLine > 0) || ( (newLineWidth - lineWidth) < (float) nMaxWidth );

			// not enough room for that word
			// If it is the only word of the line, just split it
			// Otherwise, scale the spaces between words so that the line as the maximum width
			if (roomForThisWord)
			{
				if (expandSpaces)
				{
					nlassert(_Lines.size() > 0);
					nlassert(_Lines.back()->getNumWords() > 0);

					// Yoyo: if the line has tab, then don't justify
					if(wordFormat.TabX > 0)
						_Lines.back()->setSpaceWidth(_SpaceWidth);
					else
					{
						// Scale the width so that the line has the maximum width
						float roomForSpaces = nMaxWidth - _Lines.back()->getWidthWithoutSpaces();
						uint  startNumSpaces = _Lines.back()->getNumSpaces();
						if (startNumSpaces != 0)
						{
							_Lines.back()->setSpaceWidth(roomForSpaces / startNumSpaces);
						}
						else
						{
							_Lines.back()->setSpaceWidth(_SpaceWidth);
						}
					}
				}
				else
				{
					breakLine = true;
				}
				// we dont change the position in the input string so that the current will be processed on the next line

			}
			else // it is the only word on the line..
			{
				// .. so split it
				// 1) Check if spaces go beyond the end of line
				if (numSpaces * _SpaceWidth > nMaxWidth)
				{
					uint maxNumSpaces = std::max(1U, (uint) (nMaxWidth / _SpaceWidth));
					CWord spaceWord; // a word with only spaces in it
					spaceWord.build (ucstring (""), maxNumSpaces);
					spaceWord.Format= wordFormat;
					_Lines.push_back(TLineSPtr(new CLine));
					_Lines.back()->addWord(spaceWord, _FontWidth);
					if (expandSpaces)
					{
						_Lines.back()->setSpaceWidth(nMaxWidth / (float) maxNumSpaces);
					}
					else
					{
						_Lines.back()->setSpaceWidth(_SpaceWidth);
					}
					currPos = currPos + maxNumSpaces;
				}
				else
				{
					float px = numSpaces * _SpaceWidth;
					uint currChar = 0;
					ucstring oneChar(" ");
					for(currChar = 0; currChar < wordValue.length(); ++currChar)
					{
						oneChar = wordValue[currChar];
						si = CInterfaceManager::getInstance()->getTextContext()->getStringInfo(oneChar);
						if ((uint) (px + si.StringWidth) > nMaxWidth) break;
						px += si.StringWidth;
					}
					currChar = std::max((uint) 1, currChar); // must fit at least one character otherwise there's an infinite loop
					wordValue = _Text.substr(spaceEnd, currChar);
					CWord word;
					word.build(wordValue, numSpaces);
					word.Format= wordFormat;
					_Lines.push_back(TLineSPtr(new CLine));
					float roomForSpaces = (float) nMaxWidth - word.Info.StringWidth;
					if (expandSpaces && numSpaces != 0)
					{
						_Lines.back()->setSpaceWidth(roomForSpaces / (float) numSpaces);
					}
					else
					{
						_Lines.back()->setSpaceWidth(0);
					}
					_Lines.back()->addWord(word, _FontWidth);
					currPos = currPos + numSpaces + currChar;
				}
			}
			// reset line
			numWordsInLine = 0;
			precLineWidth= lineWidth;
			lineWidth = 0;
			isParagraphStart = false;
		}
		else if (!lineFeed) // the end of line hasn't been reached
		{
			if (expandSpaces)
			{
				// add in the current line (and create one if necessary)
				if (numWordsInLine == 0)
				{
					_Lines.push_back(TLineSPtr(new CLine));
					_Lines.back()->setSpaceWidth(_SpaceWidth);
				}
				if (!wordValue.empty() || numSpaces != 0)
				{
					CWord word;
					word.build(wordValue, numSpaces);
					word.Format= wordFormat;
					// update line width
					_Lines.back()->addWord(word, _FontWidth);
					++numWordsInLine;
				}
			}
			else
			{
				CWord word;
				// Don't build here, this is used as temp data.
				word.Text= wordValue;
				word.NumSpaces= numSpaces;
				word.Format= wordFormat;
				// Append to the temp Data.
				currLine.push_back(word);

				++numWordsInLine;
			}
			lineWidth = newLineWidth;
			currPos = wordEnd;
		}
		else
		{
			// '\n' was encountered
			++ currPos;
			isParagraphStart = true;
		}
		if (lineFeed || breakLine) // '\n' was encoutered, or a linefeed has been asked
		{
			// !expandSpaces => insert minimum words according to word color.
			if (!expandSpaces)
			{
				// Add the new line.
				addDontClipWordLine(currLine);
				// LineFeed?
				if (lineFeed)
				{
					_Lines.back()->setLF(true);
				}
			}
			// expandSpaces => just add a empty line.
			else
			{
				if (numWordsInLine == 0)
				{
					// if nothing has been inserted in this line, create at least an empty line
					_Lines.push_back(TLineSPtr(new CLine));
				}
				if (lineFeed)
				{
					_Lines.back()->setLF(true);
				}
			}
			lineWidth = 0.f;
			numWordsInLine = 0;
		}
	}

	// if current line hasn't been pushed, add it
	if (!expandSpaces && !currLine.empty())
	{
		// Add new line
		addDontClipWordLine(currLine);
	}

	// if the text ends with \n, must insert the last line ourself
	if (!_Text.empty() && _Text[_Text.length() - 1] == (ucchar) '\n')
	{
		_Lines.push_back(TLineSPtr(new CLine));
	}
}


// ***************************************************************************
void CViewText::updateTextContext ()
{
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();

	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setShaded (_Shadow);
	TextContext->setFontSize (_FontSize);

	// default state
	_SingleLineTextClamped= false;

	if ((_MultiLine)&&(_Parent != NULL))
	{
		sint nMaxWidth = getCurrentMultiLineMaxW();
		_LastMultiLineMaxW = nMaxWidth;
		clearLines();
		if (nMaxWidth <= 0)
		{
			// parent size may not be known yet
			return;
		}
		switch(_TextMode)
		{
			case ClipWord: updateTextContextMultiLine(nMaxWidth); break;
			case DontClipWord: updateTextContextMultiLineJustified(nMaxWidth, false); break;
			case Justified: updateTextContextMultiLineJustified(nMaxWidth, true); break;
		}

		// Special case for multiline limited in number of lines
		if ((_Lines.size() > 0) && (_MultiMaxLine > 0) && (_Lines.size() > _MultiMaxLine))
		{
			while (_Lines.size() > _MultiMaxLine)
			{
				_Lines.back()->clear();
				_Lines.pop_back();
			}
			_Lines.pop_back();
			CViewText::CLine *endLine = new CViewText::CLine;
			CViewText::CWord w;
			w.build(string("..."));
			endLine->addWord(w, _FontWidth);
			_Lines.push_back(TLineSPtr(endLine));
		}

		// Calculate size
		float rTotalW = 0;
		for (uint i = 0; i < _Lines.size(); ++i)
		{
			rTotalW = std::max(_Lines[i]->getWidth() + ((i==0)?_FirstLineX:0), rTotalW);
		}
		_W = (sint)rTotalW;
		_H = std::max(_FontHeight, uint(_FontHeight * _Lines.size() + std::max(0, sint(_Lines.size()) - 1) * _MultiLineSpace));

		// Compute tooltips size
		if (_Tooltips.size() > 0)
		for (uint i=0 ; i<_Lines.size() ; ++i)
		{
			for (uint j=0 ; j<_Lines[i]->getNumWords() ; ++j)
			{
				CWord word = _Lines[i]->getWord(j);
//				float w = _Lines[i]->getWidth();

				if (word.Format.IndexTt != -1)
				{
					if (_Tooltips.size() > (uint)word.Format.IndexTt)
					{
						CCtrlToolTip *pTooltip = _Tooltips[word.Format.IndexTt];

						sint y = (sint) ((_FontHeight + _MultiLineSpace) * (_Lines.size() - i - 1));

						pTooltip->setX(0);
						pTooltip->setY(y);
						pTooltip->setW(getCurrentMultiLineMaxW());
						pTooltip->setH(_FontHeight);
					}
				}
			}
		}
	}
	else // Single line code
	{
		if (_Index != 0xFFFFFFFF)
			TextContext->erase (_Index);

		// Common case: no W clamp
		_Index = TextContext->textPush (_Text);
		_Info = TextContext->getStringInfo (_Index);
		_W = (sint)(_Info.StringWidth);

		// Rare case: clamp W => recompute slowly, cut letters
		if(_W>_LineMaxW)
		{
			TextContext->erase (_Index);

			ucchar ucLetter;
			UTextContext::CStringInfo si;
			ucstring	ucCurrentLine;
			ucCurrentLine.reserve(_Text.size());
			// Append ... to the end of line
			si = TextContext->getStringInfo (ucstring("..."));
			float		dotWidth= si.StringWidth;
			float		rWidthCurrentLine = 0, rWidthLetter;
			// for all the text
			if (_ClampRight)
			{
				for (uint i = 0; i < _Text.size(); ++i)
				{
					ucLetter= _Text[i];
					ucstring ucStrLetter;
					ucStrLetter= ucLetter;
					si = TextContext->getStringInfo (ucStrLetter);
					rWidthLetter = (si.StringWidth);
					if ((rWidthCurrentLine + rWidthLetter + dotWidth) > _LineMaxW)
					{
						break;
					}
					else
					{
						// Grow the current line
						ucCurrentLine += ucLetter;
						rWidthCurrentLine += rWidthLetter;
					}
				}

				// Add the dots
				ucCurrentLine+= "...";
			}
			else
			{
				for (sint i = (sint)_Text.size() - 1; i >= 0; --i)
				{
					ucLetter= _Text[i];
					ucstring ucStrLetter;
					ucStrLetter= ucLetter;
					si = TextContext->getStringInfo (ucStrLetter);
					rWidthLetter = (si.StringWidth);
					if ((rWidthCurrentLine + rWidthLetter + dotWidth) > _LineMaxW)
					{
						break;
					}
					else
					{
						// Grow the current line
						ucCurrentLine = ucLetter + ucCurrentLine;
						rWidthCurrentLine += rWidthLetter;
					}
				}

				// Add the dots
				ucCurrentLine = "..." + ucCurrentLine;
			}

			// And so setup this trunc text
			_Index = TextContext->textPush (ucCurrentLine);
			_Info = TextContext->getStringInfo (_Index);
			_W = (sint)(_Info.StringWidth);

			_SingleLineTextClamped= true;
		}

		// same height always
		_H = _FontHeight;
	}

	_InvalidTextContext= false;
}

// ***************************************************************************
void CViewText::updateCoords()
{
	if (_AutoClamp)
	{
		CViewBase::updateCoords ();
		if (_Parent)
		{
			CInterfaceGroup *parent = _Parent;
			// avoid resizing parents to compute the limiter
			while (parent && (parent->getResizeFromChildW() || dynamic_cast<CGroupList *>(parent)))
			{
				// NB nico : the dynamic_cast for CGroupList is bad!!
				// can't avoid it for now, because, CGroupList implicitly does a "resize from child" in its update coords
				// ...
				parent = parent->getParent();
			}
			if (parent)
			{
				if (_ClampRight)
				{
					sint32 parentRight = parent->getXReal() + parent->getWReal() - (sint32) _AutoClampOffset;
					setLineMaxW(std::max((sint32) 0, parentRight - _XReal));
				}
				else
				{
					sint32 parentLeft = parent->getXReal() + (sint32) _AutoClampOffset;
					setLineMaxW(std::max((sint32) 0, _XReal + _WReal - parentLeft));
				}
			}
		}
	}

	if(_InvalidTextContext)
		updateTextContext();

	CViewBase::updateCoords ();
}

// ***************************************************************************
sint CViewText::getLineFromIndex(uint index, bool cursorDisplayedAtEndOfPreviousLine /* = true*/) const
{
	if (index > _Text.length()) return -1;
	if (_MultiLine)
	{
		uint charIndex = 0;
		for(sint i = 0; i < (sint) _Lines.size(); ++i)
		{
			CLine &currLine = *_Lines[i];
			uint newCharIndex = charIndex + currLine.getNumChars() + currLine.getEndSpaces() + (currLine.getLF() ? 1 : 0);
			if (newCharIndex > index)
			{
				if (i != 0 && cursorDisplayedAtEndOfPreviousLine && charIndex == index)
				{
					return i - 1;
				}
				else
				{
					return i;
				}
			}
			charIndex = newCharIndex;
		}
		return (sint)_Lines.size() - 1;
	}
	else
	{
		return 0;
	}
}

// ***************************************************************************
sint CViewText::getLineStartIndex(uint line) const
{
	uint charIndex = 0;
	if (line >= _Lines.size()) return -1;
	for(uint i = 0; i < line; ++i)
	{
		CLine &currLine = *_Lines[i];
	    charIndex += currLine.getNumChars() + currLine.getEndSpaces() + (currLine.getLF() ? 1 : 0);
	}
	// skip all spaces at start of line (unless there are only spaces in the line)
	std::string::size_type nextPos = _Text.find_first_not_of((ucchar) ' ', charIndex);
	if (nextPos != std::string::npos)
	{
		if (getLineFromIndex(charIndex) == (sint) line)
		{
			return (sint) nextPos;
		}
	}
	return charIndex;
}

// ***************************************************************************
void CViewText::getLineEndIndex(uint line, sint &index, bool &endOfPreviousLine) const
{
	sint startIndex = getLineStartIndex(line);
	if (startIndex == -1)
	{
		index = -1;
		endOfPreviousLine = false;
		return;
	}
	index = startIndex + _Lines[line]->getNumChars() + _Lines[line]->getEndSpaces();
	endOfPreviousLine = !_Lines[line]->getLF();
}

// ***************************************************************************
void CViewText::setHardText (const std::string &ht)
{
//	ucstring Text = ucstring(ht);
	ucstring Text;
	if ((ht.size()>2) && (ht[0] == 'u') && (ht[1] == 'i'))
		Text = CI18N::get (ht);
	else
		Text.fromUtf8(ht);
	setText(Text);
}

// ***************************************************************************
string CViewText::getColorAsString() const
{
	return NLMISC::toString(_Color.R) + " " + NLMISC::toString(_Color.G) + " " + NLMISC::toString(_Color.B) + " " + NLMISC::toString(_Color.A);
}

// ***************************************************************************
void CViewText::setColorAsString(const string &ht)
{
	_Color = convertColor (ht.c_str());
}

// ***************************************************************************
NLMISC::CRGBA CViewText::getColorRGBA() const
{
	return _Color;
}

// ***************************************************************************
void        CViewText::setColorRGBA(NLMISC::CRGBA col)
{
	_Color = col;
}

// ***************************************************************************
void CViewText::getCharacterPositionFromIndex(sint index, bool cursorAtPreviousLineEnd, sint &x, sint &y, sint &height) const
{
	NLMISC::clamp(index, 0, (sint) _Text.length());
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setShaded (_Shadow);
	TextContext->setFontSize (_FontSize);
//	CViewRenderer &rVR = CInterfaceManager::getInstance()->getViewRenderer();
	height = getFontHeight();
	//
	if (_MultiLine)
	{
		uint charIndex = 0;
		// special case for end of text
		if (index == (sint) _Text.length())
		{
			y = 0;
			if (_Lines.empty())
			{
				x = 0;
			}
			else
			{
				CLine &lastLine = *_Lines.back();
				x = (sint) (lastLine.getWidth() + lastLine.getEndSpaces() * lastLine.getSpaceWidth());
				sint nMaxWidth = getCurrentMultiLineMaxW();
				x = std::min(x, nMaxWidth);
			}
			return;
		}
		for(sint i = 0; i < (sint) _Lines.size(); ++i)
		{
			if (i != 0 && charIndex == (uint) index && cursorAtPreviousLineEnd)
			{
				// should display the character at the end of previous line
				CLine &currLine = *_Lines[i - 1];
				y = (sint) ((_FontHeight + _MultiLineSpace) * (_Lines.size() - i));
				x = (sint) (currLine.getWidth() + currLine.getEndSpaces() * currLine.getSpaceWidth());
				sint nMaxWidth = getCurrentMultiLineMaxW();
				x = std::min(x, nMaxWidth);
				return;
			}
			CLine &currLine = *_Lines[i];
			uint newCharIndex = charIndex + currLine.getNumChars() + currLine.getEndSpaces() + (_Lines[i]->getLF() ? 1 : 0);
			if ((sint) newCharIndex > index)
			{
				// ok, this line contains the character, now, see which word contains it.
				y = (sint) ((_FontHeight + _MultiLineSpace) * (_Lines.size() - 1 - i));
				// see if the index is in the spaces at the end of line
				if (index - charIndex >= currLine.getNumChars())
				{
					uint numSpaces = index - charIndex - currLine.getNumChars();
					x = (sint) (currLine.getWidth() + numSpaces * _SpaceWidth);
					sint nMaxWidth = getCurrentMultiLineMaxW();
					x = std::min(x, nMaxWidth);
					return;
				}
				// now, search containing word in current line
				float px = (float)_FirstLineX;
				for(uint k = 0; k < currLine.getNumWords(); ++k)
				{
					CWord &currWord = currLine.getWord(k);
					if ((sint) (charIndex + currWord.NumSpaces + currWord.Text.length()) >= index)
					{
						// character is in currWord or the in spaces preceding it
						// check if the character is in the word
						if ((uint) (index - charIndex) > currWord.NumSpaces)
						{
							// get the x position
							ucstring subStr = currWord.Text.substr(0, index - charIndex - currWord.NumSpaces);
							// compute the size
							UTextContext::CStringInfo si = TextContext->getStringInfo(subStr);
							x = (sint) (px + si.StringWidth + currWord.NumSpaces * currLine.getSpaceWidth());
							height = getFontHeight();
							return;
						}
						else
						{
							// character is in the spaces preceding the word
							x = (sint) (px + currLine.getSpaceWidth() * (index - charIndex));
							height = getFontHeight();
							return;
						}
					}
					charIndex += (uint)currWord.Text.length() + currWord.NumSpaces;
					px += currWord.NumSpaces * currLine.getSpaceWidth() + currWord.Info.StringWidth;
				}
			}
			charIndex = newCharIndex;
		}

	}
	else
	{
		// get the x position
		ucstring subStr = _Text.substr(0, index);
		// compute the size
		UTextContext::CStringInfo si = TextContext->getStringInfo(subStr);
		y = 0;
		x = (sint) si.StringWidth;
	}
}

// ***************************************************************************
// Tool fct : From a word and a x coordinate, give the matching character index
static uint getCharacterIndex(const ucstring &textValue, float x)
{
	float px = 0.f;
	UTextContext::CStringInfo si;
	ucstring singleChar(" ");
	uint i;
	for (i = 0; i < textValue.length(); ++i)
	{
		// get character width
		singleChar[0] = textValue[i];
		si = CInterfaceManager::getInstance()->getTextContext()->getStringInfo(singleChar);
		px += si.StringWidth;
		 // the character is at the i - 1 position
		if (px > x)
		{
			// if the half of the character is after the cursor, then prefer select the next one (like in Word)
			if(px-si.StringWidth/2 < x)
				i++;
			break;
		}
	}
	return i;
}

// ***************************************************************************
void CViewText::getCharacterIndexFromPosition(sint x, sint y, uint &index, bool &cursorAtPreviousLineEnd) const
{
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();

	// setup the text context
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setShaded (_Shadow);
	TextContext->setFontSize (_FontSize);
	 // find the line where the character is
//	CViewRenderer &rVR = CInterfaceManager::getInstance()->getViewRenderer();
	uint      charPos = 0;
	if (_MultiLine)
	{
		// seek the line
		float py = 0.f;
		if (py > y)
		{
			index = (uint)_Text.length();
			cursorAtPreviousLineEnd = false;
			return;
		}
		sint line;
		for (line = (uint)_Lines.size() - 1; line >= 0; --line)
		{
			float newPy = py + _FontHeight + _MultiLineSpace;
			if (newPy > y)
			{
				break;
			}
			py = newPy;
		}
		if (line == -1)
		{
			index = 0;
			cursorAtPreviousLineEnd = false;
			return; // above the first line, so take character 0
		}
		// compute character index at start of line
		sint i;
		for (i = 0; i < line; ++i)
		{
			charPos += _Lines[i]->getNumChars() + _Lines[i]->getEndSpaces() + (_Lines[i]->getLF() ? 1 : 0);
		}
		// seek word that contains the character
		CLine &currLine = *_Lines[line];
		// See if character is in the ending spaces
		if (x >= (sint) currLine.getWidth())
		{
			// Add _SpaceWidth/2 to select between chars
			sint numSpaces = _SpaceWidth != 0 ? (sint) (((float) x + _SpaceWidth/2 - currLine.getWidth()) / _SpaceWidth)
											  : 0;
			clamp(numSpaces, 0, (sint)currLine.getEndSpaces());
			index = charPos + currLine.getNumChars() + numSpaces;
			cursorAtPreviousLineEnd = !_Lines[i]->getLF();
			return;
		}

		float px = (float)_FirstLineX;
		for(uint k = 0; k < currLine.getNumWords(); ++k)
		{
			CWord &currWord = currLine.getWord(k);
			float spacesWidth = currLine.getSpaceWidth() * currWord.NumSpaces;
			float newPx = px + currWord.Info.StringWidth + spacesWidth;
			if (newPx >= x) // if the word contains the x position..
			{
				if (x < (px + spacesWidth))
				{
					// the coords x is in the spaces that are preceding the word
					// Add spaceWidth/2 to select between chars
					sint numSpaces = currLine.getSpaceWidth() != 0 ? (sint) ((x + currLine.getSpaceWidth()/2 - px) / currLine.getSpaceWidth())
													               : 0;
					clamp(numSpaces, 0, (sint)currWord.NumSpaces);
					index =  numSpaces + charPos;
					cursorAtPreviousLineEnd = false;
					return;
				}
				else
				{
					// the coord is in the word itself
					index = charPos + currWord.NumSpaces + getCharacterIndex(currWord.Text, (float) x - (px + spacesWidth));
					cursorAtPreviousLineEnd = false;
					return;
				}
			}
			px = newPx;
			charPos += (uint)currWord.Text.length() + currWord.NumSpaces;
		}
		index =  charPos;
		cursorAtPreviousLineEnd = false;
		return;
	}
	else
	{
		cursorAtPreviousLineEnd = false;
		if (y < 0)
		{
			index = (uint)_Text.length();
			return;
		}
		if (y > (sint) _FontHeight)
		{
			index = 0;
			return;
		}
		index = getCharacterIndex(_Text, (float) x);
		return;
	}
}

// ***************************************************************************
void CViewText::enableStringSelection(uint start, uint end)
{
	_TextSelection= true;
	_TextSelectionStart= start;
	_TextSelectionEnd= end;
}

// ***************************************************************************
void CViewText::disableStringSelection()
{
	_TextSelection= false;
	_TextSelectionStart= 0;
	_TextSelectionEnd= std::numeric_limits<uint>::max();
}

// ***************************************************************************
void CViewText::setStringSelectionSkipingSpace(uint stringId, const ucstring &text, sint charStart, sint charEnd)
{
	sint	quadStart= charStart;
	sint	quadSize= charEnd-charStart;
	sint j;
	for(j=0;j<charStart;j++)
	{
		if(text[j]==' ')
			quadStart--;
	}
	for(j=charStart;j<charEnd;j++)
	{
		if(text[j]==' ')
			quadSize--;
	}
	// select what quad to skip
	CInterfaceManager::getInstance()->getTextContext()->setStringSelection(stringId, quadStart, quadSize);
}

// ***************************************************************************
void CViewText::clearLines()
{
	for(uint k = 0; k < _Lines.size(); ++k)
	{
		_Lines[k]->clear();
	}
	_Lines.clear();
}

// ***************************************************************************
uint CViewText::getNumLine() const
{
	if (_MultiLine)
	{
		return (uint)_Lines.size();
	}
	else
	{
		return _Text.empty() ? 0 : 1;
	}
}

// ***************************************************************************
uint CViewText::getFirstLineX() const
{
	return _FirstLineX;
}

// ***************************************************************************
uint CViewText::getLastLineW () const
{
	if (!_Lines.empty())
		return (uint)_Lines.back()->getWidth();
	return 0;
}

// ***************************************************************************
void CViewText::setFirstLineX(uint firstLineX)
{
	_FirstLineX = firstLineX;
}

/////////////////////////////////////
// CViewText::CLine implementation //
/////////////////////////////////////

// ***************************************************************************
CViewText::CLine::CLine() : _NumChars(0),
							_NumSpaces(0),
                            _SpaceWidth(0.f),
							_StringLine(0),
							_WidthWithoutSpaces(0.f),
							_EndSpaces(0),
							_HasLF(false)
{
}

// ***************************************************************************
void CViewText::CLine::addWord(const ucstring &text, uint numSpaces, const CFormatInfo &wordFormat, uint fontWidth)
{
	CWord word;
	word.build(text, numSpaces);
	word.Format= wordFormat;
	addWord(word, fontWidth);
}

// ***************************************************************************
void CViewText::CLine::addWord(const CWord &word, uint fontWidth)
{
	_Words.push_back(word);
	_NumChars += word.NumSpaces + uint(word.Text.length());
	_NumSpaces += word.NumSpaces;
	if (fabsf(word.Info.StringLine) > fabsf(_StringLine))
	{
		_StringLine = word.Info.StringLine;
	}
	// the width of the line must reach at least the Tab
	_WidthWithoutSpaces= max(_WidthWithoutSpaces, word.Format.TabX * float(fontWidth));
	// append the text space
	_WidthWithoutSpaces += word.Info.StringWidth;
}

// ***************************************************************************
void CViewText::CLine::clear()
{
	for(uint k = 0; k < _Words.size(); ++k)
	{
		if (_Words[k].Index != 0xffffffff)
			CInterfaceManager::getInstance()->getTextContext()->erase(_Words[k].Index);
	}
	_Words.clear();
	_NumChars = 0;
	_SpaceWidth = 0.f;
}

// ***************************************************************************
void CViewText::CLine::resetTextIndex()
{
	for(uint k = 0; k < _Words.size(); ++k)
	{
		_Words[k].Index = 0xffffffff;
	}
}

// ***************************************************************************
void CViewText::CWord::build(const ucstring &text, uint numSpaces/*=0*/)
{
	Text = text;
	NumSpaces = numSpaces;
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
	Index = TextContext->textPush(text);
	Info = TextContext->getStringInfo(Index);
}

// ***************************************************************************
void CViewText::removeEndSpaces()
{
	sint i = (sint)_Text.size()-1;
	while ((i>=0) && ((_Text[i] < 0x20) || (_Text[i] == ' ')))
	{
		i--;
	}
	_Text.resize (i+1);
}

// ***************************************************************************
sint32 CViewText::getMaxUsedW() const
{
	static const ucstring spaceStr(" \t");
	static const ucstring lineFeedStr("\n");
	float maxWidth = 0;

	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setShaded (_Shadow);
	TextContext->setFontSize (_FontSize);

	TCharPos linePos = 0;
	while (linePos < _Text.length())
	{
		// Get the end of the line
		float lineWidth = 0;
		TCharPos lineEnd;
		lineEnd = _Text.find_first_of(lineFeedStr, linePos);
		if (lineEnd == std::string::npos)
		{
			lineEnd = _Text.length();
		}

		ucstring lineValue;
		lineValue = _Text.substr(linePos, lineEnd - linePos);

		TCharPos currPos = 0;
		while (currPos != lineValue.length())
		{
			TCharPos spaceEnd;
			TCharPos wordEnd;
			uint numSpaces;

			// Skip spaces and count them
			spaceEnd = lineValue.find_first_not_of(spaceStr, currPos);
			if (spaceEnd == std::string::npos)
			{
				spaceEnd = lineValue.length();
			}
			numSpaces = (uint) (spaceEnd - currPos);

			// Get word until a space or a \n is encountered
			wordEnd = lineValue.find_first_of(spaceStr, spaceEnd);
			if (wordEnd == std::string::npos)
			{
				wordEnd = lineValue.length();
			}

			ucstring wordValue;
			wordValue = lineValue.substr(spaceEnd, wordEnd - spaceEnd);

			// compute width of word
			UTextContext::CStringInfo si;
			si = TextContext->getStringInfo(wordValue);

			// compute size of spaces + word
			lineWidth += numSpaces * _SpaceWidth + si.StringWidth;

			currPos = wordEnd;
		}

		// Update line width
		if (lineWidth > maxWidth)
			maxWidth = lineWidth;

		linePos = lineEnd+1;
	}

	return (sint32)maxWidth;
}

// ***************************************************************************
sint32 CViewText::getMinUsedW() const
{
	static const ucstring spaceOrLineFeedStr(" \n\t");
	sint32 maxWidth = 0;

	// Not multi line ? Same size than min
	if (!_MultiLine)
		return getMaxUsedW();

	// If we can clip word, size of the largest word
	if (_TextMode == ClipWord)
	{
		// No largest font parameter, return the font height
		return _FontHeight;
	}
	// If we can't clip the words, return the size of the largest word
	else if ((_TextMode == DontClipWord) || (_TextMode == Justified))
	{
		NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setFontSize (_FontSize);

		// Current position in text
		TCharPos currPos = 0;
		while (currPos < _Text.length())
		{
			// Current word
			ucstring wordValue;
			UTextContext::CStringInfo si;
			TCharPos wordEnd;

			// Get word until a space or a \n is encountered
			currPos = _Text.find_first_not_of(spaceOrLineFeedStr, currPos);
			if (currPos == std::string::npos)
				break;
			wordEnd = _Text.find_first_of(spaceOrLineFeedStr, currPos);
			if (wordEnd == std::string::npos)
				wordEnd = _Text.length();

			// Get the word
			wordValue = _Text.substr(currPos, wordEnd - currPos);

			// Compute width of word
			si = TextContext->getStringInfo(wordValue);

			// Larger ?
			sint32 stringWidth = (sint32)si.StringWidth;
			if (stringWidth>maxWidth)
				maxWidth = stringWidth;

			// Next word
			currPos = wordEnd;
		}
	}

	return maxWidth;
}

// ***************************************************************************
void CViewText::onInvalidateContent()
{
	_InvalidTextContext= true;
	invalidateCoords();
}

// ***************************************************************************
void CViewText::computeFontSize ()
{
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->setShaded (_Shadow);
	TextContext->setFontSize (_FontSize);

	// Letter size
	UTextContext::CStringInfo si = TextContext->getStringInfo(ucstring("|")); // for now we can't now that directly from UTextContext
	_FontHeight = (uint) si.StringHeight + (_Shadow?1:0);
	_FontLegHeight = (uint) si.StringLine + (_Shadow?1:0);

	// Space width
	si = TextContext->getStringInfo(ucstring(" "));
	_SpaceWidth = si.StringWidth;

	// Font Width
	si = TextContext->getStringInfo(ucstring("_"));
	_FontWidth = (uint)si.StringWidth;
}


// ***************************************************************************
static inline bool	isColorTag(const ucstring &s, uint index, uint textSize)
{
	// Format is @{RGBA}
	if(s[index]=='@')
	{
		if( textSize>index+1 && s[index+1]=='{')
		{
			// verify 1st letter is a xdigit
			if( textSize>index+2 && isxdigit(s[index+2]))
			{
				// We have good chance its a color tag. Do last verification
				if(textSize>index+6 && s[index+6]=='}')
				{
					return true;
				}
			}
		}
	}

	return false;
}

// ***************************************************************************
// isColorTag must be ok.
static inline CRGBA	getColorTag(const ucstring &s, uint &index)
{
	// extract the color string: "FABC"
	char	tmpCol[5];
	for(uint i=0;i<4;i++)
		tmpCol[i]= (char)s[index+2+i];
	tmpCol[4]= 0;

	// Convert to color
	CRGBA	color;
	uint	pCol;
	sscanf(tmpCol, "%x", &pCol);
	// Transform 4 bits to 8 bit.
	color.R= (pCol>>12)&0xF;	color.R+= color.R<<4;
	color.G= (pCol>>8)&0xF;		color.G+= color.G<<4;
	color.B= (pCol>>4)&0xF;		color.B+= color.B<<4;
	color.A= (pCol)&0xF;		color.A+= color.A<<4;

	// skip tag
	index+= 7;

	return color;
}


// ***************************************************************************
const	uint	MaxTabDigit= 3;
static inline bool	isTabTag(const ucstring &s, uint index, uint textSize)
{
	// Format is @{Tvalue}, where value ,1,2,3 digit.
	if(s[index]=='@')
	{
		if( textSize>index+1 && s[index+1]=='{')
		{
			if( textSize>index+2 && s[index+2]=='T')
			{
				// We have good chance its a Tab tag. Do last verification
				for(uint i=4;i<4+MaxTabDigit;i++)
				{
					if(textSize>index+i && s[index+i]=='}')
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

// ***************************************************************************
// isTabTag must be ok.
static inline sint	getTabTag(const ucstring &s, uint &index)
{
	// extract the tab min X value
	char	tmpTab[MaxTabDigit+1];
	uint	i;
	for(i=0;i<MaxTabDigit;i++)
	{
		if(s[index+3+i]=='}')
			break;
		tmpTab[i]= (char)s[index+3+i];
	}
	tmpTab[i]= 0;

	// skip tag
	index+= 3+i+1;

	sint ret;
	fromString(tmpTab, ret);
	return ret;
}


// ***************************************************************************
static inline bool	isTooltipTag(const ucstring &s, uint index, uint textSize)
{
	// Format is @{Huitt*}
	if(s[index]=='@')
	{
		if( textSize>index+1 && s[index+1]=='{')
		{
			if( textSize>index+2 && s[index+2]=='H')
			{
				uint i = 3;
				while (textSize>index+i && s[index+i]!='}')
					i++;

				if (textSize>index+i && s[index+i]=='}')
					return true;
			}
		}
	}

	return false;
}

// ***************************************************************************
// isTooltipTag must be ok.
static inline ucstring	getTooltipTag(const ucstring &s, uint &index)
{
	ucstring result;
	uint i = 3;
	while (s[index+i] != '}')
	{
		result += s[index+i];
		i++;
	}

	// skip tag
	index += i+1;

	return result;
}


// ***************************************************************************
void		CViewText::buildFormatTagText(const ucstring &text, ucstring &textBuild, std::vector<CViewText::CFormatTag> &formatTags, std::vector<ucstring> &tooltips)
{
	formatTags.clear();
	tooltips.clear();

	// Build the text without the formatTags, and get the color tags separately
	textBuild.reserve(text.size());
	uint	textSize= (uint)text.size();
	// Must herit all the props from old tags.
	CViewText::CFormatTag	precTag;	// set default.
	precTag.Index = 0;
	for (uint i = 0; i < textSize;)
	{
		if(isColorTag(text, i, textSize))
		{
			// get old tag.
			CViewText::CFormatTag ct= precTag;
			// get new color and skip tag.
			ct.Color= getColorTag(text, i);
			ct.Index= (uint)textBuild.size();
			formatTags.push_back(ct);
		}
		else if(isTabTag(text, i, textSize))
		{
			// get old tag.
			CViewText::CFormatTag ct= precTag;
			// get new Tab and skip tag.
			ct.TabX= getTabTag(text, i);
			ct.Index= (uint)textBuild.size();
			formatTags.push_back(ct);
		}
		else if(isTooltipTag(text, i, textSize))
		{
			// get old tag.
			CViewText::CFormatTag ct= precTag;
			// get new Tab and skip tag.
			ucstring uitt = getTooltipTag(text, i);
			if (uitt.empty())
			{
				ct.IndexTt= -1;
			}
			else
			{
				ct.IndexTt= (uint)tooltips.size();
				tooltips.push_back(uitt);
			}

			ct.Index= (uint)textBuild.size();
			formatTags.push_back(ct);
		}
		else
		{
			bool	lineFeed= text[i]=='\n';

			// append to textBuild
			textBuild+= text[i];
			++i;

			// if \n, reset tabulations
			if(lineFeed)
			{
				CViewText::CFormatTag ct= precTag;
				ct.TabX= 0;
				ct.Index= (uint)textBuild.size();
				formatTags.push_back(ct);
			}
		}
		// bkup
		if(!formatTags.empty())
			precTag= formatTags.back();
	}
}


// ***************************************************************************
void	CViewText::setTextFormatTaged(const ucstring &text)
{

	// to allow cache (avoid infinite recurse in updateCoords() in some case), compute in temp
	ucstring					tempText;
	// static to avoid reallocation
	static std::vector<CFormatTag>		tempFormatTags;
	static std::vector<ucstring>		tempTooltips;
	buildFormatTagText(text, tempText, tempFormatTags, tempTooltips);
	setCase (tempText, _CaseMode);

	// compare Tag arrays
	bool	sameTagArray= false;
	if(_FormatTags.size()==tempFormatTags.size())
	{
		sameTagArray= true;
		for(uint i=0;i<_FormatTags.size();i++)
		{
			if(!_FormatTags[i].sameTag(tempFormatTags[i]))
			{
				sameTagArray= false;
				break;
			}
		}
	}

	// test transformed text with current one
	if(tempText!=_Text || !sameTagArray )
	{
		// copy tags
		_FormatTags= tempFormatTags;
		// Copy to Text (preserve Memory)
		contReset(_Text);
		_Text= tempText;

		CInterfaceGroup *parent = getParent();

		// Delete old dynamic tooltips
		for (uint i=0 ; i<_Tooltips.size() ; ++i)
		{
			if (parent)
				parent->delCtrl(_Tooltips[i]);
			else
				delete _Tooltips[i];
		}
		_Tooltips.clear();

		// Add new dynamic tooltips
		for (uint i=0 ; i<tempTooltips.size() ; ++i)
		{
			CCtrlToolTip *pTooltip = new CCtrlToolTip(CCtrlToolTip::TCtorParam());
			pTooltip->setId(_Id+"_tt"+toString(i));
			pTooltip->setAvoidResizeParent(avoidResizeParent());
			pTooltip->setRenderLayer(getRenderLayer());
			pTooltip->setDefaultContextHelp(CI18N::get(tempTooltips[i].toString()));
			pTooltip->setParentPos(this);
			pTooltip->setParentPosRef(Hotspot_BR);
			pTooltip->setPosRef(Hotspot_BR);
			pTooltip->setToolTipParent(CCtrlBase::TTWindow);
			pTooltip->setToolTipParentPosRef(Hotspot_TTAuto);
			pTooltip->setToolTipPosRef(Hotspot_TTAuto);
			pTooltip->setActive(true);

			_Tooltips.push_back(pTooltip);

			if (parent)
			{
				pTooltip->setParent(parent);
				parent->addCtrl(_Tooltips.back());
			}
		}

		if (parent)
			_Setuped = true;
		else
			_Setuped = false;

		invalidateContent ();
	}

	// color format is available only if multilined
	if (!_MultiLine)
		CLuaIHMRyzom::debugInfo(toString("ViewText isn't multilined : uc_hardtext_format will not act as wanted !\n%s", text.toString().c_str()));
}


void CViewText::setSingleLineTextFormatTaged(const ucstring &text)
{
	// to allow cache (avoid infinite recurse in updateCoords() in some case), compute in temp
	ucstring					tempText;
	static std::vector<CFormatTag>		tempLetterColors;
	static std::vector<ucstring>		tempTooltips;

	// parse text
	buildFormatTagText(text, tempText, tempLetterColors, tempTooltips);
	setCase (tempText, _CaseMode);

	// decal for spaces (not inserted in VertexBuffer)
	uint textIndex = 0;
	uint spacesNb = 0;
	for(uint i=0; i<tempLetterColors.size(); i++)
	{
		CFormatTag & formatTag = tempLetterColors[i];

		while(textIndex<formatTag.Index)
		{
			if(tempText[textIndex] == ucchar(' '))
				spacesNb++;

			textIndex++;
		}

		formatTag.Index -= spacesNb;
	}

	// convert in ULetterColors
	NL3D::UTextContext *TextContext = CInterfaceManager::getInstance()->getTextContext();
	ULetterColors * letterColors = TextContext->createLetterColors();
	for(uint i=0; i<tempLetterColors.size(); i++)
	{
		const CFormatTag & formatTag = tempLetterColors[i];
		letterColors->pushLetterColor(formatTag.Index, formatTag.Color);
	}

	// test transformed text with current one
	if(tempText!=_Text || !_LetterColors || !_LetterColors->isSameLetterColors(letterColors))
	{
		_LetterColors = letterColors;

		TextContext->setLetterColors(letterColors, _Index);

		// Copy to Text (preserve Memory)
		contReset(_Text);
		_Text= tempText;
		invalidateContent ();
	}

	// this color format is available only if not multilined
	if (_MultiLine)
		CLuaIHMRyzom::debugInfo(toString("ViewText is multilined : uc_hardtext_single_line_format will not act as wanted !\n%s", text.toString().c_str()));
}


// ***************************************************************************
bool	CViewText::isFormatTagChange(uint textIndex, uint ctIndex) const
{
	if(ctIndex>=_FormatTags.size())
		return false;
	// return true if the textIndex is > (eg if some skip with spaces) or = (common case)
	return _FormatTags[ctIndex].Index <= textIndex;
}

// ***************************************************************************
void	CViewText::getFormatTagChange(uint textIndex, uint &ctIndex, CFormatInfo &wordFormat) const
{
	// support the possible case with multiple color tags with same textIndex.
	while(ctIndex<_FormatTags.size() && _FormatTags[ctIndex].Index<=textIndex)
	{
		// Take the last tag.
		wordFormat.Color= _FormatTags[ctIndex].Color;
		wordFormat.TabX= _FormatTags[ctIndex].TabX;
		wordFormat.IndexTt= _FormatTags[ctIndex].IndexTt;
		// skip it.
		ctIndex++;
	}
}


// ***************************************************************************

void CViewText::setCaseMode (TCaseMode caseMode)
{
	_CaseMode = caseMode;
	setCase (_Text, _CaseMode);
}

// ***************************************************************************

TCaseMode CViewText::getCaseMode () const
{
	return _CaseMode;
}

// ***************************************************************************

void CViewText::resetTextIndex()
{
	_Index = 0xffffffff;
	for(uint k = 0; k < _Lines.size(); ++k)
		_Lines[k]->resetTextIndex();
}

// ***************************************************************************
void CViewText::setup()
{
	_Setuped= true;

	// Add dynamic tooltips
	for (uint i=0 ; i<_Tooltips.size() ; ++i)
	{
		CInterfaceGroup *parent = getParent();
		if (parent)
		{
			_Tooltips[i]->setParent(parent);
			parent->addCtrl(_Tooltips.back());
		}
	}
}

// ***************************************************************************
void CViewText::serial(NLMISC::IStream &f)
{
	#define SERIAL_UINT(val) { uint32 tmp = (uint32) val; f.serial(tmp); val = (uint) tmp; }
	#define SERIAL_SINT(val) { sint32 tmp = (sint32) val; f.serial(tmp); val = (sint) tmp; }
	CViewBase::serial(f);
	SERIAL_SINT(_FontSize);
	SERIAL_UINT(_FontWidth);
	SERIAL_UINT(_FontHeight);
	SERIAL_UINT(_FontLegHeight);
	f.serial(_SpaceWidth);
	f.serial(_Color);
	f.serial(_Shadow);
	f.serialEnum(_CaseMode);
	f.serial(_ShadowColor);
	f.serial(_LineMaxW);
	f.serial(_SingleLineTextClamped);
	f.serial(_MultiLine);
	f.serial(_MultiLineMaxWOnly);
	f.serial(_MultiLineClipEndSpace);
	f.serial(_AutoClampOffset);
	f.serialEnum(_TextMode);
	SERIAL_SINT(_MultiLineSpace);
	SERIAL_SINT(_LastMultiLineMaxW);
	f.serial(_MultiMaxLine);

	bool hasTag = !_FormatTags.empty();
	f.serial(hasTag);
	if (f.isReading())
	{
		ucstring text;
		f.serial(text);
		if (hasTag)
		{
			if (_MultiLine)
			{
				setTextFormatTaged(text);
			}
			else
			{
				setSingleLineTextFormatTaged(text);
			}
		}
		else
		{
			setText(text);
		}
	}
	else
	{
		f.serial(_Text);
	}

	#undef SERIAL_UINT
	#undef SERIAL_SINT
}


// ***************************************************************************
