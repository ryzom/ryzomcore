// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2018  Winch Gate Property Limited
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

#include "stdpch.h"

#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"

#include "nel/gui/view_text.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/ctrl_tooltip.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/view_pointer_base.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

typedef std::string::size_type TCharPos; // index of a chracter in a string

REGISTER_UI_CLASS(CViewText)

namespace NLGUI
{

	// ***************************************************************************
	void CViewText::setupDefault ()
	{
		_ParentElm = NULL;

		_CaseMode = CaseNormal;
		_Underlined = false;
		_StrikeThrough = false;
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

		_FontSize = 12 +
			CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32();
		_FontSizeCoef = true;
		_FontName.clear();
		_Embolden = false;
		_Oblique = false;
		_Color = CRGBA(255,255,255,255);
		_Shadow = false;
		_ShadowOutline = false;
		_ShadowColor = CRGBA(0,0,0,255);
		_ShadowX = 1;
		_ShadowY = 1;

		_MultiLine = false;
		_TextMode = DontClipWord;
		_MultiLineSpace = 8;
		_LineMaxW = std::numeric_limits<sint32>::max();
		_MultiLineMaxWOnly = false;
		_MultiLineClipEndSpace = false;
		_LastMultiLineMaxW = 0;
		_MultiMinLine = 0;
		_MultiMaxLine = 0;
		_Index = 0xFFFFFFFF;

		_Scale = CWidgetManager::getInstance()->getInterfaceScale();
		_FontWidth= 0;
		_FontHeight = 0;
		_FontLegHeight = 0;

		_TextSelection= false;
		_TextSelectionStart= 0;
		_TextSelectionEnd= std::numeric_limits<uint>::max();

		_InvalidTextContext= true;
		_FirstLineX = 0;

		_SingleLineTextClamped= false;
		_OverExtendViewText= false;
		_OverExtendViewTextUseParentRect= false;

		_AutoClamp = false;
		_ClampRight = true; // clamp on the right of the text
		_OverflowText = "...";

		_Localized = true;

		_LetterColors = NULL;
		_Setuped= false;
		_AutoClampOffset = 0;

		// Letter size
		// - "_" that should be the character with the lowest part
		// - A with an accent for the highest part
		// https://www.compart.com/en/unicode/U+00C4
		_FontSizingChars = "_\xC3\x84q";
		// fallback if SizingChars are not supported by font
		_FontSizingFallback = "|XO";
		computeFontSize ();
	}

	// ***************************************************************************

	NLMISC_REGISTER_OBJECT(CViewBase, CViewText, std::string, "text");

	CViewText::CViewText(const TCtorParam &param)
	:CViewBase(param)
	{
		setupDefault ();

		CWidgetManager::getInstance()->registerInterfaceScaleWatcher(this);
	}

	///constructor
	// ***************************************************************************
	CViewText::	CViewText (const std::string& id, const std::string Text, sint FontSize,
							NLMISC::CRGBA Color, bool Shadow, bool ShadowOutline)
							:CViewBase(TCtorParam())
	{
		_Id = id;
		setupDefault ();

		_FontSize = FontSize + CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();
		_FontSizeCoef = true;
		_Color = Color;
		_Shadow = Shadow;
		_ShadowOutline = ShadowOutline;
		setHardText(Text);
		computeFontSize ();

		CWidgetManager::getInstance()->registerInterfaceScaleWatcher(this);
	}

	// ***************************************************************************
	CViewText::~CViewText()
	{
		CWidgetManager::getInstance()->unregisterInterfaceScaleWatcher(this);

		if (_Index != 0xFFFFFFFF)
			CViewRenderer::getTextContext(_FontName)->erase (_Index);
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
			CViewRenderer::getTextContext(_FontName)->erase (_Index);

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
		_FontSizeCoef = vt._FontSizeCoef;
		_Embolden = vt._Embolden;
		_Oblique = vt._Oblique;
		_Underlined = vt._Underlined;
		_StrikeThrough = vt._StrikeThrough;
		_Color = vt._Color;
		_Shadow = vt._Shadow;
		_ShadowOutline = vt._ShadowOutline;
		_ShadowColor = vt._ShadowColor;

		_MultiLine = false;
		_MultiLineSpace = 8;
		_LineMaxW= std::numeric_limits<sint32>::max();
		_MultiLineMaxWOnly = false;
		_MultiLineClipEndSpace = false;
		_LastMultiLineMaxW = 0;
		_Index = 0xFFFFFFFF;

		_ModulateGlobalColor= vt._ModulateGlobalColor;
		_Localized = vt._Localized;

		// remove previous lines
		clearLines();
		_InvalidTextContext = true;
		computeFontSize ();

		return *this;
	}

	std::string CViewText::getProperty( const std::string &name ) const
	{
		std::string prop = getTextProperty( name );

		if( !prop.empty() )
			return prop;
		else
		if( name == "hardtext" )
		{
			return getHardText();
		}
		else
		if( name == "hardtext_format" )
		{
			return _HardTextFormat;
		}
		else
			return CViewBase::getProperty( name );
	}


	std::string CViewText::getTextProperty( const std::string &name ) const
	{
		if( name == "localize" )
		{
			return toString(_Localized);
		}
		else
		if( name == "color" )
		{
			return toString( _Color );
		}
		else
		if( name == "global_color" )
		{
			return toString( _ModulateGlobalColor );
		}
		else
		if( name == "fontsize" )
		{
			if (_FontSizeCoef)
				return toString(_FontSize - CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32());

			return toString(_FontSize);
		}
		else
		if ( name == "fontsize_coef" )
		{
			return toString(_FontSizeCoef);
		}
		else
		if( name == "fontweight" )
		{
			if (_Embolden)
				return "bold";

			return "normal";
		}
		if( name == "fontstyle" )
		{
			if (_Oblique)
				return "oblique";

			return "normal";
		}
		else
		if( name == "shadow" )
		{
			return toString( _Shadow );
		}
		else
		if( name == "shadow_outline" )
		{
			return toString( _ShadowOutline );
		}
		else
		if( name == "shadow_color" )
		{
			return toString( _ShadowColor );
		}
		else
		if( name == "multi_line" )
		{
			return toString( _MultiLine );
		}
		else
		if( name == "justification" )
		{
			switch( _TextMode )
			{
			case ClipWord:
				return "clip_word";
				break;

			case DontClipWord:
				return "dont_clip_word";
				break;

			case Justified:
				return "justified";
				break;

			case Centered:
				return "centered";
			}

			return "";
		}
		else
		if( name == "line_maxw" )
		{
			return toString( _LineMaxW );
		}
		else
		if( name == "multi_line_space" )
		{
			return toString( _MultiLineSpace );
		}
		else
		if( name == "multi_line_maxw_only" )
		{
			return toString( _MultiLineMaxWOnly );
		}
		else
		if( name == "multi_max_line" )
		{
			return toString( _MultiMaxLine );
		}
		else
		if( name == "multi_min_line" )
		{
			return toString( _MultiMinLine );
		}
		else
		if( name == "underlined" )
		{
			return toString( _Underlined );
		}
		else
		if( name == "strikethrough" )
		{
			return toString( _StrikeThrough );
		}
		else
		if( name == "case_mode" )
		{
			return toString( uint32( _CaseMode ) );
		}
		else
		if( name == "over_extend_view_text" )
		{
			return toString( _OverExtendViewText );
		}
		else
		if( name == "over_extend_parent_rect" )
		{
			return toString( _OverExtendViewTextUseParentRect );
		}
		else
		if( name == "auto_clamp" )
		{
			return toString( _AutoClamp );
		}
		else
		if( name == "clamp_right" )
		{
			return toString( _ClampRight );
		}
		else
		if( name == "auto_clamp_offset" )
		{
			return toString( _AutoClampOffset );
		}
		else
		if( name == "continuous_update" )
		{
			return toString( _ContinuousUpdate );
		}
		else
		if ( name == "sizing_chars" )
		{
			return _FontSizingChars;
		}
		else
		if ( name == "sizing_fallback" )
		{
			return _FontSizingFallback;
		}
		else
			return "";
	}

	void CViewText::setProperty( const std::string &name, const std::string &value )
	{
		if( setTextProperty( name, value ) )
			invalidateContent();
		else
			CViewBase::setProperty( name, value );
	}

	bool CViewText::setTextProperty( const std::string &name, const std::string &value )
	{
		if( name == "localize" )
		{
			bool b;
			if (fromString(value, b))
			{
				_Localized = b;
				setTextLocalized(_HardText.empty() ? _Text : _HardText); // FIXME: setCase?
				_TextLength = 0;
			}
			return true;
		}
		else
		if( name == "color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_Color = c;
			return true;
		}
		else
		if( name == "global_color" )
		{
			bool b;
			if( fromString( value, b ) )
				_ModulateGlobalColor = b;
			return true;
		}
		else
		if( name == "fontsize" )
		{
			sint i;
			if( fromString( value, i ) )
				_FontSize = i + CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32();
			return true;
		}
		else
		if( name == "fontsize_coef" )
		{
			bool b;
			bool oldValue = _FontSizeCoef;
			if (fromString( value, b) )
				_FontSizeCoef = b;
			// must only change font size when current state changes
			if (_FontSizeCoef != oldValue)
			{
				if (_FontSizeCoef)
					_FontSize += CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32();
				else
					_FontSize -= CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32();
			}
		}
		else
		if( name == "fontweight" )
		{
			if (value == "bold")
				_Embolden = true;

			return true;
		}
		if( name == "fontstyle" )
		{
			if( value == "oblique" )
				_Oblique = true;
			return true;
		}
		else
		if( name == "shadow" )
		{
			bool b;
			if( fromString( value, b ) )
				_Shadow = b;
			return true;
		}
		else
		if( name == "shadow_outline" )
		{
			bool b;
			if( fromString( value, b ) )
				_ShadowOutline = b;
			return true;
		}
		else
		if( name == "shadow_color" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_ShadowColor = c;
			return true;
		}
		else
		if( name == "shadow_x" )
		{
			sint sx;
			if( fromString( value, sx ) )
				_ShadowX = sx;
			return true;
		}
		else
		if( name == "shadow_y" )
		{
			sint sy;
			if( fromString( value, sy ) )
				_ShadowY = sy;
			return true;
		}
		else
		if( name == "multi_line" )
		{
			bool b;
			if( fromString( value, b ) )
				_MultiLine = b;
			return true;
		}
		else
		if( name == "justification" )
		{
			if( value == "clip_word" )
				_TextMode = ClipWord;
			else
			if( value == "dont_clip_word" )
				_TextMode = DontClipWord;
			else
			if( value == "justified" )
				_TextMode = Justified;
			else
			if( value == "centered" )
				_TextMode = Centered;

			return true;
		}
		else
		if( name == "line_maxw" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_LineMaxW = i;
			return true;
		}
		else
		if( name == "multi_line_space" )
		{
			sint i;
			if( fromString( value, i ) )
				_MultiLineSpace = i;
			return true;
		}
		else
		if( name == "multi_line_maxw_only" )
		{
			bool b;
			if( fromString( value, b ) )
				_MultiLineMaxWOnly = b;
			return true;
		}
		else
		if( name == "multi_max_line" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MultiMaxLine = i;
			return true;
		}
		else
		if( name == "multi_min_line" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MultiMinLine = i;
			return true;
		}
		else
		if( name == "underlined" )
		{
			bool b;
			if( fromString( value, b ) )
				_Underlined = b;
			return true;
		}
		else
		if( name == "strikethrough" )
		{
			bool b;
			if( fromString( value, b ) )
				_StrikeThrough = b;
			return true;
		}
		else
		if( name == "case_mode" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_CaseMode = (TCaseMode)i;
			return true;
		}
		else
		if( name == "over_extend_view_text" )
		{
			bool b;
			if( fromString( value, b ) )
				_OverExtendViewText = b;
			return true;
		}
		else
		if( name == "over_extend_parent_rect" )
		{
			bool b;
			if( fromString( value, b ) )
				_OverExtendViewTextUseParentRect = b;
			return true;
		}
		else
		if( name == "auto_clamp" )
		{
			bool b;
			if( fromString( value, b ) )
				_AutoClamp = b;
			return true;
		}
		else
		if( name == "clamp_right" )
		{
			bool b;
			if( fromString( value, b ) )
				_ClampRight = b;
			return true;
		}
		else
		if( name == "auto_clamp_offset" )
		{
			uint8 i;
			if( fromString( value, i ) )
				_AutoClampOffset = i;
			return true;
		}
		else
		if( name == "continuous_update" )
		{
			bool b;
			if( fromString( value, b ) )
				_ContinuousUpdate = b;
			return true;
		}
		else
		if( name == "text" )
		{
			setTextLocalized(value); // FIXME: setCase?
			_TextLength = 0;
			invalidateContent();
			return true;
		}
		else
		if( name == "hardtext" )
		{
			_Localized = true;
			setTextLocalized(value); // FIXME: setCase?
			_TextLength = 0;
			invalidateContent();
			return true;
		}
		else
		if( name == "hardtext_format" )
		{
			_HardTextFormat = value;

			if( _MultiLine )
				setTextFormatTaged( _HardTextFormat );
			else
				setSingleLineTextFormatTaged( _HardTextFormat );

			return true;
		}
		else
		if( name == "sizing_chars" )
		{
			_FontSizingChars = value;
			return true;
		}
		else
		if( name == "sizing_fallback" )
		{
			_FontSizingFallback = value;
			return true;
		}
		else
			return false;
	}


	bool CViewText::serializeTextOptions( xmlNodePtr node ) const
	{
		xmlSetProp( node, BAD_CAST "localize", BAD_CAST toString( _Localized ).c_str() );

		xmlSetProp( node, BAD_CAST "color", BAD_CAST toString( _Color ).c_str() );
		xmlSetProp( node, BAD_CAST "global_color", BAD_CAST toString( _ModulateGlobalColor ).c_str() );

		sint32 fontSize = _FontSize;
		if (_FontSizeCoef) fontSize -= CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont ).getValSInt32();
		xmlSetProp( node, BAD_CAST "fontsize", BAD_CAST toString(fontSize).c_str() );
		xmlSetProp( node, BAD_CAST "fontsize_coef", BAD_CAST toString(_FontSizeCoef).c_str() );

		std::string fontweight("normal");
		if (_Embolden)
			fontweight = "bold";
		xmlSetProp( node, BAD_CAST "fontweight", BAD_CAST fontweight.c_str() );

		std::string fontstyle("normal");
		if (_Oblique)
			fontstyle = "oblique";
		xmlSetProp( node, BAD_CAST "fontstyle", BAD_CAST fontstyle.c_str() );

		xmlSetProp( node, BAD_CAST "shadow", BAD_CAST toString( _Shadow ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_outline", BAD_CAST toString( _ShadowOutline ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_color", BAD_CAST toString( _ShadowColor ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_x", BAD_CAST toString( _ShadowX ).c_str() );
		xmlSetProp( node, BAD_CAST "shadow_y", BAD_CAST toString( _ShadowY ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_line", BAD_CAST toString( _MultiLine ).c_str() );

		std::string just;

		switch( _TextMode )
		{
		case ClipWord:
			just = "clip_word";
			break;

		case DontClipWord:
			just = "dont_clip_word";
			break;

		case Justified:
			just = "justified";
			break;

		case Centered:
			just = "centered";
			break;
		}
		
		xmlSetProp( node, BAD_CAST "justification", BAD_CAST just.c_str() );
		xmlSetProp( node, BAD_CAST "line_maxw", BAD_CAST toString( _LineMaxW ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_line_space", BAD_CAST toString( _MultiLineSpace ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_line_maxw_only", BAD_CAST toString( _MultiLineMaxWOnly ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_max_line", BAD_CAST toString( _MultiMaxLine ).c_str() );
		xmlSetProp( node, BAD_CAST "multi_min_line", BAD_CAST toString( _MultiMinLine ).c_str() );
		xmlSetProp( node, BAD_CAST "underlined", BAD_CAST toString( _Underlined ).c_str() );
		xmlSetProp( node, BAD_CAST "strikethrough", BAD_CAST toString( _StrikeThrough ).c_str() );
		xmlSetProp( node, BAD_CAST "case_mode", BAD_CAST toString( uint32( _CaseMode ) ).c_str() );
		xmlSetProp( node, BAD_CAST "over_extend_view_text", BAD_CAST toString( _OverExtendViewText ).c_str() );
		xmlSetProp( node, BAD_CAST "over_extend_parent_rect",
			BAD_CAST toString( _OverExtendViewTextUseParentRect ).c_str() );
		xmlSetProp( node, BAD_CAST "auto_clamp", BAD_CAST toString( _AutoClamp ).c_str() );
		xmlSetProp( node, BAD_CAST "clamp_right", BAD_CAST toString( _ClampRight ).c_str() );
		xmlSetProp( node, BAD_CAST "auto_clamp_offset", BAD_CAST toString( _AutoClampOffset ).c_str() );
		xmlSetProp( node, BAD_CAST "continuous_update", BAD_CAST toString( _ContinuousUpdate ).c_str() );
		xmlSetProp( node, BAD_CAST "sizing_chars", BAD_CAST _FontSizingChars.c_str() );
		xmlSetProp( node, BAD_CAST "sizing_fallback", BAD_CAST _FontSizingFallback.c_str() );

		return true;
	}


	xmlNodePtr CViewText::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text" );

		serializeTextOptions( node );

		xmlSetProp( node, BAD_CAST "text", BAD_CAST (_HardText.empty() ? _Text.c_str() : _HardText.c_str()) );
		xmlSetProp( node, BAD_CAST "hardtext_format", BAD_CAST _HardTextFormat.c_str() );

		return node;
	}

	// ***************************************************************************
	void CViewText::parseTextOptions (xmlNodePtr cur)
	{
		CXMLAutoPtr prop;

		prop = xmlGetProp (cur, (xmlChar*)"localize");
		if (prop) _Localized = convertBool((const char*)prop);

		prop= (char*) xmlGetProp( cur, (xmlChar*)"color" );
		_Color = CRGBA(255,255,255,255);
		if (prop)
			_Color = convertColor(prop);

		prop= (char*) xmlGetProp (cur, (xmlChar*)"global_color");
		if(prop)
			_ModulateGlobalColor= convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"fontsize" );
		_FontSize = 12 + CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();
		if (prop)
		{
			fromString((const char*)prop, _FontSize);
			_FontSize += CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"fontsize_coef" );
		_FontSizeCoef = true;
		if (prop)
		{
			_FontSizeCoef = convertBool(prop);
			if (!_FontSizeCoef)
				_FontSize -= CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();
		}


		prop = (char*) xmlGetProp( cur, (xmlChar*)"fontweight" );
		_Embolden = false;
		if (prop)
		{
			if (nlstricmp("bold", (const char*)prop) == 0) _Embolden = true;
			else nlwarning("<CViewText::parse> bad fontweight '%s'", (const char *)prop);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"fontstyle" );
		_Oblique = false;
		if (prop)
		{
			if (nlstricmp("oblique", (const char *) prop) == 0) _Oblique = true;
			else nlwarning("<CViewText::parse> bad fontstyle '%s'", (const char *)prop);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"shadow" );
		_Shadow = false;
		if (prop)
			_Shadow = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"shadow_outline" );
		_ShadowOutline = false;
		if (prop)
			_ShadowOutline = convertBool(prop);

		prop= (char*) xmlGetProp( cur, (xmlChar*)"shadow_color" );
		_ShadowColor = CRGBA(0,0,0,255);
		if (prop)
			_ShadowColor = convertColor(prop);

		prop= (char*) xmlGetProp( cur, (xmlChar*)"shadow_x" );
		_ShadowX = 1;
		if (prop)
			fromString( (const char *)prop, _ShadowX);

		prop= (char*) xmlGetProp( cur, (xmlChar*)"shadow_y" );
		_ShadowY = 1;
		if (prop)
			fromString( (const char *)prop, _ShadowY);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_line" );
		_MultiLine = false;
		if (prop)
			_MultiLine = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"justification" );
		if (prop)
		{
			if (nlstricmp("clip_word", (const char *) prop) == 0)			_TextMode = ClipWord;
			else if (nlstricmp("dont_clip_word", (const char *) prop) == 0)	_TextMode = DontClipWord;
			else if (nlstricmp("justified", (const char *) prop) == 0)		_TextMode = Justified;
			else if (nlstricmp("centered", (const char *) prop) == 0)		_TextMode = Centered;
			else nlwarning("<CViewText::parse> bad text mode");
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"line_maxw" );
		_LineMaxW = std::numeric_limits<sint32>::max();
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

		prop = (char*) xmlGetProp( cur, (xmlChar*)"multi_min_line" );
		_MultiMinLine = 0;
		if (prop)
			fromString((const char*)prop, _MultiMinLine);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"underlined" );
		_Underlined = false;
		if (prop)
			_Underlined = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"strikethrough" );
		_StrikeThrough = false;
		if (prop)
			_StrikeThrough = convertBool(prop);

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

		// "_Ä" lowest/highest chars (underscore, A+diaeresis)
		prop = (char*) xmlGetProp( cur, (xmlChar*)"sizing_chars" );
		if (prop)
			_FontSizingChars = (const char*)prop;
		else
			_FontSizingChars = "_\xC3\x84q";

		// fallback if SizingChars are not supported by font
		prop = (char*) xmlGetProp( cur, (xmlChar*)"sizing_fallback" );
		if (prop)
			_FontSizingFallback = (const char *)prop;
		else
			_FontSizingFallback = "|XO";

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
			_Localized = true;
			setTextLocalized(propPtr);
			setCase(_Text, _CaseMode);
			_TextLength = 0;
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"hardtext_format" );
		if (prop)
		{
			const char *propPtr = prop;
			_HardTextFormat = propPtr;

			if (_MultiLine)
			{
				if (NLMISC::startsWith(propPtr, "u:"))
					setTextFormatTaged(std::string(propPtr).substr(2));
				else
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
			sint offset = (sint)_XReal - (sint)_Parent->getXReal();
			return std::min(_Parent->getInnerWidth() - offset, (sint)_LineMaxW);
		}
	}


	// ***************************************************************************
	void CViewText::checkCoords ()
	{
		if ((_MultiLine)&&(_Parent != NULL))
		{
			// If never setuped, and if text is not empty
			if (_Lines.empty() && !_Text.empty())
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
					CCtrlBase *pCB = CWidgetManager::getInstance()->getCapturePointerLeft();
					if (pCB != NULL)
					{
						if( pCB->isResizer() )
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

		CViewRenderer &rVR = *CViewRenderer::getInstance();

#if 0
		//rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal, _WReal, _HReal, 0, false, rVR.getBlankTextureId(), CRGBA(64,64,64,255));

		// debug text with mouse hover
		if(CWidgetManager::getInstance()->getPointer())
		{
			// but must check first if mouse is over
			sint32 x = CWidgetManager::getInstance()->getPointer()->getX();
			sint32 y = CWidgetManager::getInstance()->getPointer()->getY();
			bool	mouseIn;
			// use parent clip or self clip?
			if(_OverExtendViewTextUseParentRect)
				mouseIn= _Parent && _Parent->isIn(x,y);
			else
				mouseIn= isIn(x,y);
			// if the mouse cursor is in the clip area
			if(mouseIn)
			{
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal,		_YReal,			_WReal,	1, 0, false, rVR.getBlankTextureId(), CRGBA(200,200,200,255));
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal,		_YReal+_HReal,	_WReal,	1, 0, false, rVR.getBlankTextureId(), CRGBA(200,200,200,255));
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal,		_YReal,			1,		_HReal, 0, false, rVR.getBlankTextureId(), CRGBA(200,200,200,255));
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal+_WReal,	_YReal,			1,		_HReal, 0, false, rVR.getBlankTextureId(), CRGBA(200,200,200,255));
			}
		}
#endif

		// *** Out Of Clip?
		sint32 ClipX, ClipY, ClipW, ClipH;
		rVR.getClipWindow (ClipX, ClipY, ClipW, ClipH);
		if (((_XReal) > (ClipX+ClipW)) || ((_XReal+_WReal) < ClipX) ||
			((_YReal) > (ClipY+ClipH)) || ((_YReal+_HReal) < ClipY))
			return;

		// hack: allow shadow to overflow outside parent box.
		// In CGroupHTML context, clip is set for row
		if (std::abs(_ShadowX) > 0)
		{
			ClipX -= 3;
			ClipW += 3;
		}
		if (std::abs(_ShadowY) > 0)
		{
			ClipY -= 3;
			ClipH += 3;
		}

		// *** Screen Minimized?
		if (rVR.isMinimized())
			return;

		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);


		// *** get current color
		CRGBA col, shcol;
		if(getModulateGlobalColor())
		{
			CRGBA gcfc = CWidgetManager::getInstance()->getGlobalColorForContent();
			col.modulateFromColor (_Color, gcfc);
			shcol.modulateFromColor (_ShadowColor, gcfc);
		}
		else
		{
			col = _Color;
			shcol = _ShadowColor;
			col.A = (uint8)(((sint)col.A*((sint)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
			shcol.A = (uint8)(((sint)shcol.A*((sint)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
		}

		float	oow, ooh;
		rVR.getScreenOOSize (oow, ooh);

		// *** Draw multiline
		if ((_MultiLine)&&(_Parent != NULL))
		{
			if (_Lines.empty()) return;

			TextContext->setHotSpot (UTextContext::BottomLeft);
			TextContext->setShaded (_Shadow);
			TextContext->setShadeOutline (_ShadowOutline);
			TextContext->setShadeColor (shcol);
			TextContext->setShadeExtent (_ShadowX*oow, _ShadowY*ooh);
			TextContext->setFontSize (_FontSize*_Scale);
			TextContext->setEmbolden (_Embolden);
			TextContext->setOblique (_Oblique);

			// Y is the base line of the string, so it must be grown up.
			float y = _YReal * _Scale + _FontLegHeight;

			if (_MultiMinLine > _Lines.size())
			{
				y += getMultiMinOffsetY() * _Scale;
			}

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
						sint	cEnd= min(charIndex+(sint)currWord.Info.StringLength, (sint)_TextSelectionEnd);

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
						charIndex+= (sint)currWord.Info.StringLength;
					}
					charIndex += currLine.getEndSpaces() + (currLine.getLF() ? 1 : 0);
				}
			}

			// draw
			for (sint i = (sint)_Lines.size()-1; i >= 0; --i)
			{
				CLine &currLine = *_Lines[i];
				// current x position
				float px = (float) (_XReal * _Scale + ((i==0) ? _FirstLineX : 0.f));

				// Center line to computed maximum line width (_WReal)
				//
				// Does not give most accurate result when _WReal is much smaller than parent,
				// but _WReal also defines mouseover hotspot/tooltip area.
				//
				// May not work correctly in CGroupParagraph (multiple text elements).
				//
				if (_TextMode == Centered)
					px += (float)(_WReal * _Scale - (currLine.getWidth() + (i == 0 ? _FirstLineX : 0.f)) )/ 2.f;

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
					if (_Underlined || _StrikeThrough)
					{
						line_width = (sint)floorf(currLine.getWidthWithoutSpaces() + currLine.getSpaceWidth());
						line_width -= (sint)floorf(firstSpace);
					}
					px += firstSpace;
					// skip tabulation before current word
					if(currWord.Format.TabX)
						px= max(px, (float)(_XReal * _Scale + currWord.Format.TabX*_FontWidth));

					// draw. We take floorf px to avoid filtering of letters that are not located on a pixel boundary
					float fx = px / _Scale;
					float fy = y / _Scale;
					rVR.drawText (_RenderLayer, fx, fy, currWord.Index, ClipX, ClipY, ClipX+ClipW, ClipY+ClipH, *TextContext);

					// Draw a line
					if (_Underlined)
						rVR.drawRotFlipBitmap (_RenderLayer, fx, fy - _FontLegHeight*0.6f / _Scale, line_width / _Scale, 1.0f / _Scale, 0, false, rVR.getBlankTextureId(), col);

					if (_StrikeThrough)
						rVR.drawRotFlipBitmap (_RenderLayer, fx, fy + _FontHeight*0.2f / _Scale, line_width / _Scale, 1.0f / _Scale, 0, false, rVR.getBlankTextureId(), col);

					// skip word
					px += currWord.Info.StringWidth;
				}
				// go one line up
				y += (_FontHeight + _MultiLineSpace * _Scale);
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
			TextContext->setShadeOutline (_ShadowOutline);
			TextContext->setShadeColor (shcol);
			TextContext->setShadeExtent (_ShadowX*oow, _ShadowY*ooh);
			TextContext->setFontSize (_FontSize*_Scale);
			TextContext->setEmbolden (_Embolden);
			TextContext->setOblique (_Oblique);

			if(_LetterColors!=NULL && !TextContext->isSameLetterColors(_LetterColors, _Index))
			{
				TextContext->setLetterColors(_LetterColors, _Index);
			}

			// Y is the base line of the string, so it must be grown up.
			float y = _YReal * _Scale + _FontLegHeight;

			// special selection code
			if(_TextSelection)
				// select subset. Arg, must skip spaces because not inserted in VertexBuffer.
				setStringSelectionSkipingSpace(_Index, _Text, _TextSelectionStart, _TextSelectionEnd);

			// Change the current color
			TextContext->setStringColor(_Index, col);

			// draw
			rVR.drawText (_RenderLayer, _XReal, y / _Scale, _Index, ClipX, ClipY, ClipX+ClipW, ClipY+ClipH, *TextContext);

			// Draw a line
			if (_Underlined)
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal - _FontLegHeight*0.3f/_Scale, _WReal, 1, 0, false, rVR.getBlankTextureId(), col);

			if (_StrikeThrough)
				rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal + _FontHeight*0.2f / _Scale, _WReal, 1, 0, false, rVR.getBlankTextureId(), col);

			// reset selection
			if(_TextSelection)
				TextContext->resetStringSelection(_Index);

			// if single line clamped, may allow to draw an over
			if(isSingleLineTextClamped() && _OverExtendViewText && CWidgetManager::getInstance()->getPointer())
			{
				// but must check first if mouse is over
				sint32 x = CWidgetManager::getInstance()->getPointer()->getX();
				sint32 y = CWidgetManager::getInstance()->getPointer()->getY();
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
					CInterfaceGroup		*pIG = CWidgetManager::getInstance()->getWindowUnder(x,y);
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
						CGroupContainerBase *gc= dynamic_cast<CGroupContainerBase*>(pIG);
						if(!gc || !gc->isMoving())
						{
							CRGBA col= CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionViewTextOverBackColor).getValColor();
							CWidgetManager::getInstance()->setOverExtendViewText(this, col);
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

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	void CViewText::setTextAsUtf16(const ucstring &text)
	{
		setText(text.toUtf8());
	}
#endif

	// ***************************************************************************
	void CViewText::setTextLocalized(const std::string &text, bool localized)
	{
		if (localized != _Localized)
		{
			_Localized = localized;

			// Always recompute if localization and text changed
			setTextLocalized(text);
			setCase(_Text, _CaseMode);
			_TextLength = 0;
			invalidateContent();
		}
		else
		{
			setText(text);
		}
	}

	// ***************************************************************************
	void CViewText::setLocalized(bool localized)
	{
		if (localized != _Localized)
		{
			const std::string &text = _HardText.empty() ? _Text : _HardText;
			_Localized = localized;
			if (!text.empty() && NLMISC::startsWith(text, "ui"))
			{
				setTextLocalized(text);
				setCase(_Text, _CaseMode);
				_TextLength = 0;
				invalidateContent();
			}
		}

		nlassert(_Text.empty() || ((_Localized && (NLMISC::startsWith(getText(), "ui"))) == (_HardText.empty() == _Text.empty())));
	}

	// ***************************************************************************
	void CViewText::setTextLocalized(const std::string &text)
	{
		if (_Localized && NLMISC::startsWith(text, "ui"))
		{
			_HardText = text;
			_Text = CI18N::get(text);
		}
		else
		{
			_Text = text;
			_HardText.clear();
		}
	}

	// ***************************************************************************
    void CViewText::setText(const std::string &text)
    {
	    // common case: no special format, no case mode => easy cache test
	    if (_FormatTags.empty() && _CaseMode == CaseNormal)
	    {
		    if (_HardText.empty() ? text != _Text : text != _HardText)
		    {
			    setTextLocalized(text);
			    _TextLength = 0;
			    // no need to call  "setCase (_Text, _CaseMode);"  since CaseNormal:
			    invalidateContent();
		    }
	    }
	    else
	    {
		    // if the view text had some format before, no choice, must recompute all
		    if (!_FormatTags.empty())
		    {
			    setTextLocalized(text);
			    setCase(_Text, _CaseMode);
			    _TextLength = 0;
			    invalidateContent();
		    }
		    // else test if after the case change the cache succeed
		    else
		    {
			    // compute the temp cased text
			    std::string holdText, holdHardText;
			    holdText.swap(_Text);
			    holdHardText.swap(_HardText);
			    setTextLocalized(text);
			    setCase(_Text, _CaseMode);
			    if (holdText != _Text)
			    {
				    _TextLength = 0;
				    invalidateContent();
			    }
			    else
			    {
				    holdText.swap(_Text);
			    }
		    }
	    }

		nlassert(_Text.empty() || ((_Localized && (NLMISC::startsWith(text, "ui"))) == (_HardText.empty() == _Text.empty())));

	    // clear format tags if any
	    _FormatTags.clear();
    }

	// ***************************************************************************
	void CViewText::setFontSizing(const std::string &chars, const std::string &fallback)
	{
		_FontSizingChars = chars;
		_FontSizingFallback = fallback;
	}

	// ***************************************************************************
	void CViewText::setFontName(const std::string &name)
	{
		if (_FontName == name)
			return;

		if (_Index != 0xFFFFFFFF)
			CViewRenderer::getTextContext(_FontName)->erase (_Index);
		clearLines();

		resetTextIndex();

		_FontName = name;
		computeFontSize ();
		invalidateContent();
	}

	// ***************************************************************************
	void CViewText::setFontSize (sint nFontSize, bool coef)
	{
		_FontSize = nFontSize;
		if (coef) _FontSize += CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();
		_FontSizeCoef = coef;
		computeFontSize ();
		invalidateContent();
	}

	// ***************************************************************************
	sint CViewText::getFontSize() const
	{
		if (_FontSizeCoef)
			return _FontSize - CWidgetManager::getInstance()->getSystemOption( CWidgetManager::OptionAddCoefFont).getValSInt32();

		return _FontSize;
	}

	// ***************************************************************************
	void CViewText::setEmbolden (bool embolden)
	{
		_Embolden = embolden;
		computeFontSize ();
		invalidateContent();
	}

	// ***************************************************************************
	void CViewText::setOblique (bool oblique)
	{
		_Oblique = oblique;
		computeFontSize ();
		invalidateContent();
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
	void CViewText::setShadowOutline (bool bShadowOutline)
	{
		_ShadowOutline = bShadowOutline;
		computeFontSize ();
		invalidateContent();
	}

	// ***************************************************************************
	void CViewText::setShadowColor(const NLMISC::CRGBA & color)
	{
		_ShadowColor = color;
	}

	// ***************************************************************************
	void CViewText::setShadowOffset(sint32 x, sint32 y)
	{
		_ShadowX = x;
		_ShadowY = y;
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
			setLineMaxW(ceilf(value / _Scale));
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
		return _FontWidth / _Scale;
	}

	// ***************************************************************************
	uint CViewText::getFontHeight() const
	{
		return _FontHeight / _Scale;
	}

	// ***************************************************************************
	uint CViewText::getFontLegHeight() const
	{
		return _FontLegHeight / _Scale;
	}

	// ***************************************************************************
	float CViewText::getLineHeight() const
	{
		return _FontHeight / _Scale + _MultiLineSpace;
	}

	// ***************************************************************************
	uint CViewText::getMultiMinOffsetY() const
	{
		uint dy = 0;
		if (_MultiMinLine > _Lines.size())
		{
			// first line is always present even if _Lines is empty
			uint nbLines = _MultiMinLine - std::max((sint)1, (sint)_Lines.size());
			dy = (nbLines * _FontHeight + (nbLines - 1) * _MultiLineSpace * _Scale) / _Scale;
		}
		return dy;
	}

	// ***************************************************************************
	void CViewText::flushWordInLine(std::string &ucCurrentWord, bool &linePushed, const CFormatInfo &wordFormat)
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		// create a new line?
		if(!linePushed)
		{
			_Lines.push_back(TLineSPtr(new CLine));
			linePushed= true;
		}
		// Append to the last line
		_Lines.back()->addWord(ucCurrentWord, 0, wordFormat, _FontWidth, *TextContext);
		// reset the word
		ucCurrentWord.clear();
	}


	// ***************************************************************************
	void CViewText::updateTextContextMultiLine(float nMaxWidth)
	{
		// ucchar ucLetter;
		UTextContext::CStringInfo si;
		uint i = 0;
		// word state
		std::string ucCurrentWord;
		CFormatInfo		wordFormat;
		// line state
		float	rWidthCurrentLine = 0;
		bool	linePushed= false;
		// for all the text
		// uint	textSize= (uint)_Text.size();
		uint	formatTagIndex= 0;
		nMaxWidth *= _Scale;
		//for (i = 0; i < textSize; ++i)
		CUtfStringView sv(_Text);
		::u32string ucStrLetter(1, ' ');
		for (CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it, ++i)
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

			NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);

			// Parse the letter
			{
				if (*it == '\n')
				{
					flushWordInLine(ucCurrentWord, linePushed, wordFormat);
					// reset line state
					linePushed= false;
					rWidthCurrentLine = 0;
				}
				else
				{
					ucStrLetter[0] = *it;
					si = TextContext->getStringInfo (ucStrLetter);
					if ((rWidthCurrentLine + si.StringWidth) > nMaxWidth)
					{
						flushWordInLine(ucCurrentWord, linePushed, wordFormat);

						// reset line state, and begin with the cut letter
						linePushed= false;
						rWidthCurrentLine = si.StringWidth;
						ucCurrentWord.clear();
						CUtfStringView::append(ucCurrentWord, *it);
					}
					else
					{
						// Grow the current word
						CUtfStringView::append(ucCurrentWord, *it);
						rWidthCurrentLine += si.StringWidth;
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
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		// create a new line
		_Lines.push_back(TLineSPtr(new CLine));

		// Fill it with words. if all words of same color, create only one CWord
		if (!currLine.empty())
		{
			CFormatInfo	lineWordFormat= currLine[0].Format;
			std::string	lineWord;
			for(uint i=0;i<currLine.size();i++)
			{
				// If different from last, flush
				if(currLine[i].Format!=lineWordFormat)
				{
					// add the current lineWord to the line.
					_Lines.back()->addWord(lineWord, 0, lineWordFormat, _FontWidth, *TextContext);
					// get new lineWordFormat
					lineWordFormat= currLine[i].Format;
					// and clear
					lineWord.clear();
				}

				// Append the word with space to the lineWord.
				std::string blank(currLine[i].NumSpaces, ' ');
				lineWord += blank;
				lineWord += currLine[i].Text;
			}

			if(!lineWord.empty())
				_Lines.back()->addWord(lineWord, 0, lineWordFormat, _FontWidth, *TextContext);

			// clear
			currLine.clear();
		}
	}

	// ***************************************************************************
	void CViewText::updateTextContextMultiLineJustified(float nMaxWidth, bool expandSpaces)
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		UTextContext::CStringInfo si;
		//
		TCharPos currPos = 0;
		//
		// precLineWidth valid only id precedent line is part of same paragraph.
		float precLineWidth= 0;
		float lineWidth = (float)_FirstLineX; // width of the current line
		uint  numWordsInLine = 0; // number of words in the current line
		bool  isParagraphStart = true; // A paragraph is a group of characters between 2 \n
		bool  lineFeed;
		bool  breakLine;
		//
		vector<CWord>	currLine; // if spaces are not expanded, all words of a line are inserted here (NB: index and stringInfo not filled)
		std::string	wordValue;
		CFormatInfo	wordFormat;
		uint		formatTagIndex= 0;
		//
		nMaxWidth *= _Scale;
		while (currPos != _Text.length())
		{
			TCharPos spaceEnd;
			TCharPos wordEnd;
			uint numSpaces;
			float newLineWidth = 0;
			breakLine = false;
			//
			if (_Text[currPos] == '\n')
			{
				lineFeed = true;
			}
			else
			{
				lineFeed = false;
				// Skip spaces and count them
				spaceEnd = _Text.find_first_not_of(" ", currPos);
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

				// Detect change of wordFormat at the beginning of the word
				if(isFormatTagChange((uint)spaceEnd, formatTagIndex))
				{
					getFormatTagChange((uint)spaceEnd, formatTagIndex, wordFormat);
				}

				// Get word until a space, a \n, or a FormatTagChange is encountered
				uint	i;
				for(i= (uint)spaceEnd;i<(uint)_Text.length();i++)
				{
					char	c= _Text[i];
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
				si = TextContext->getStringInfo(wordValue);

				// compute size of spaces/Tab + word
				newLineWidth = lineWidth + numSpaces * _SpaceWidth;
				newLineWidth = max(newLineWidth, (float)wordFormat.TabX*_FontWidth);
				newLineWidth+= si.StringWidth;
			}
			//
			// Does the word go beyond the end of line ?
			if (!lineFeed && newLineWidth > nMaxWidth)
			{
				// Have we enough room for this word on a line ?
				bool roomForThisWord = (numWordsInLine > 0) || ( (newLineWidth - lineWidth) < nMaxWidth );

				// not enough room for that word
				// If it is the only word of the line, just split it
				// Otherwise, scale the spaces between words so that the line as the maximum width
				if (roomForThisWord)
				{
					if (expandSpaces)
					{
						nlassert(!_Lines.empty());
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
						spaceWord.build ("", *TextContext, maxNumSpaces);
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
						::u32string oneChar(1, ' ');
						CUtfStringView wsv(wordValue);
						CUtfStringView::iterator wit(wsv.begin()), wend(wsv.end());
						for (; wit != wend; ++wit)
						{
							oneChar[0] = *wit;
							si = TextContext->getStringInfo(oneChar);
							if ((uint) (px + si.StringWidth) > nMaxWidth) break;
							px += si.StringWidth;
						}
						ptrdiff_t currChar = std::max((ptrdiff_t)1, (ptrdiff_t)wit.ptr() - (ptrdiff_t)wsv.ptr()); // must fit at least one character otherwise there's an infinite loop
						wordValue = _Text.substr(spaceEnd, currChar);
						CWord word;
						word.build(wordValue, *TextContext, numSpaces);
						word.Format= wordFormat;
						_Lines.push_back(TLineSPtr(new CLine));
						float roomForSpaces = nMaxWidth - word.Info.StringWidth;
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
						word.build(wordValue, *TextContext, numSpaces);
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
		if (!_Text.empty() && _Text[_Text.length() - 1] == '\n')
		{
			_Lines.push_back(TLineSPtr(new CLine));
		}
	}


	// ***************************************************************************
	void CViewText::updateTextContext ()
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);

		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeOutline (_ShadowOutline);
		TextContext->setFontSize (_FontSize*_Scale);
		TextContext->setEmbolden (_Embolden);
		TextContext->setOblique (_Oblique);

		// default state
		_SingleLineTextClamped= false;

		if ((_MultiLine)&&(_Parent != NULL))
		{
			float nMaxWidth = getCurrentMultiLineMaxW();
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
				case Centered: // fallthru to DontClipWord
				case DontClipWord: updateTextContextMultiLineJustified(nMaxWidth, false); break;
				case Justified: updateTextContextMultiLineJustified(nMaxWidth, true); break;
			}

			// Special case for multiline limited in number of lines
			if (!_Lines.empty() && (_MultiMaxLine > 0) && (_Lines.size() > _MultiMaxLine))
			{
				while (_Lines.size() > _MultiMaxLine)
				{
					_Lines.back()->clear(*TextContext);
					_Lines.pop_back();
				}
				if (_OverflowText.size() > 0)
				{
					_Lines.pop_back();
					CViewText::CLine *endLine = new CViewText::CLine;
					CViewText::CWord w;
					w.build(_OverflowText, *TextContext);
					endLine->addWord(w, _FontWidth);
					_Lines.push_back(TLineSPtr(endLine));
				}
			}

			// Calculate size
			float rMultiLineSpace = _MultiLineSpace * _Scale;
			float rTotalW = 0;
			for (uint i = 0; i < _Lines.size(); ++i)
			{
				rTotalW = std::max(_Lines[i]->getWidth() + ((i==0)?_FirstLineX:0), rTotalW);
			}
			_W = (sint)ceilf(rTotalW / _Scale);
			_H = std::max(_FontHeight / _Scale, ceilf((_FontHeight * _Lines.size() + std::max(0, sint(_Lines.size()) - 1) * rMultiLineSpace) / _Scale));

			// See if we should pretend to have at least X lines
			if (_MultiMinLine > 1)
				_H = std::max(_H, sint((_FontHeight * _MultiMinLine + (_MultiMinLine - 1) * rMultiLineSpace)/_Scale));

			// Compute tooltips size
			if (!_Tooltips.empty())
			for (uint i=0 ; i<_Lines.size() ; ++i)
			{
				for (uint j=0 ; j<_Lines[i]->getNumWords() ; ++j)
				{
					CWord word = _Lines[i]->getWord(j);

					if (word.Format.IndexTt != -1)
					{
						if (_Tooltips.size() > (uint)word.Format.IndexTt)
						{
							CCtrlToolTip *pTooltip = _Tooltips[word.Format.IndexTt];

							sint y = (sint) ceilf(((_FontHeight + rMultiLineSpace) * (_Lines.size() - i - 1))/_Scale);

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
			_W = (sint)ceilf(_Info.StringWidth / _Scale);

			// Rare case: clamp W => recompute slowly, cut letters
			if(_W > _LineMaxW)
			{
				TextContext->erase (_Index);

				// char ucLetter;
				UTextContext::CStringInfo si;
				std::string	ucCurrentLine;
				ucCurrentLine.reserve(_Text.size());

				// Append ... to the end of line
				float dotWidth = 0.f;
				if (_OverflowText.size() > 0)
				{
					si = TextContext->getStringInfo(_OverflowText);
					dotWidth = si.StringWidth;
				}

				// scale LineMaxW to actual font size
				float fLineMaxW = (float)_LineMaxW * _Scale;
				float rWidthCurrentLine = 0;
				// for all the text
				if (_ClampRight)
				{
					CUtfStringView sv(_Text);
					::u32string ucStrLetter = ::u32string(1, (u32char)' ');
					for (CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it)
					{
						ucStrLetter[0] = *it;
						si = TextContext->getStringInfo (ucStrLetter);
						if ((rWidthCurrentLine + si.StringWidth + dotWidth) > fLineMaxW)
						{
							break;
						}
						else
						{
							// Grow the current line
							CUtfStringView::append(ucCurrentLine, *it);
							rWidthCurrentLine += si.StringWidth;
						}
					}

					// Add the dots
					if (_OverflowText.size() > 0)
					{
						ucCurrentLine += _OverflowText;
					}
				}
				else
				{
					// FIXME: Optimize reverse UTF iteration
					::u32string uctext = CUtfStringView(_Text).toUtf32();
					::u32string ucStrLetter = ::u32string(1, (u32char)' ');
					for (sint i = (sint)uctext.size() - 1; i >= 0; --i)
					{
						ucStrLetter[0] = uctext[i];
						si = TextContext->getStringInfo (ucStrLetter);
						if ((rWidthCurrentLine + si.StringWidth + dotWidth) > fLineMaxW)
						{
							break;
						}
						else
						{
							// Grow the current line
							std::string tmp;
							CUtfStringView::append(tmp, uctext[i]);
							ucCurrentLine = tmp + ucCurrentLine;
							rWidthCurrentLine += si.StringWidth;
						}
					}

					// Add the dots
					if (_OverflowText.size() > 0)
					{
						ucCurrentLine = _OverflowText + ucCurrentLine;
					}
				}

				// And so setup this trunc text
				_Index = TextContext->textPush (ucCurrentLine);
				_Info = TextContext->getStringInfo (_Index);
				_W = (sint)ceilf(_Info.StringWidth / _Scale);

				_SingleLineTextClamped= true;
			}

			// same height always
			_H = (sint)ceilf(_FontHeight / _Scale);
		}

		_InvalidTextContext= false;
	}

	// ***************************************************************************
	void CViewText::updateCoords()
	{
		if (_AutoClamp)
		{
			CViewBase::updateCoords ();

			// If there's no parent, try the parent of the parent element.
			// Since we will be under the same group
			CInterfaceGroup *parent = _Parent;
			if( parent == NULL )
			{
				if( _ParentElm != NULL )
					parent = _ParentElm->getParent();
			}

			if (parent)
			{
				// avoid resizing parents to compute the limiter
				while (parent && (parent->getResizeFromChildW() || parent->isGroupList() ))
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
			// return (sint)_Lines.size() - 1;
			return -1;
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
		std::string::size_type nextPos = _Text.find_first_not_of(' ', charIndex);
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

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	void CViewText::setHardTextAsUtf16(const ucstring &ht)
	{
		setHardText(ht.toUtf8());
	}
#endif

	// ***************************************************************************
	void CViewText::setHardText (const std::string &ht)
	{
		if (!_Localized)
		{
			setText(std::string());
			_Localized = true;
		}
		setText(ht);
	}

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	ucstring CViewText::getTextAsUtf16() const
	{
		return CUtfStringView(getText()).toUtf16();
	}

	// ***************************************************************************
	ucstring CViewText::getHardTextAsUtf16() const
	{
		return CUtfStringView(getHardText()).toUtf16();
	}
#endif

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
	void CViewText::setColorRGBA(NLMISC::CRGBA col)
	{
		_Color = col;
	}

	// ***************************************************************************
	void CViewText::getCharacterPositionFromIndex(sint index, bool cursorAtPreviousLineEnd, float &x, float &y, float &height) const
	{
		if (!_TextLength && _Text.size())
			_TextLength = CUtfStringView(_Text).count();
		NLMISC::clamp(index, 0, (sint)_TextLength);
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeOutline (_ShadowOutline);
		TextContext->setFontSize (_FontSize*_Scale);
		TextContext->setEmbolden (_Embolden);
		TextContext->setOblique (_Oblique);
		height = getLineHeight();
		//
		if (_MultiLine)
		{
			float fx, fy;
			float dy = getMultiMinOffsetY() * _Scale;
			float nMaxWidth = getCurrentMultiLineMaxW() * _Scale;

			uint charIndex = 0;
			// special case for end of text
			if (index >= (sint)_TextLength)
			{
				fy = dy;
				if (_Lines.empty())
				{
					x = 0;
					fx = 0;
				}
				else
				{
					CLine &lastLine = *_Lines.back();
					fx = lastLine.getWidth() + lastLine.getEndSpaces() * lastLine.getSpaceWidth();
					fx = std::min(fx, nMaxWidth);
				}

				x = fx / _Scale;
				y = fy / _Scale;
				return;
			}
			for(sint i = 0; i < (sint) _Lines.size(); ++i)
			{
				if (i != 0 && charIndex == (uint) index && cursorAtPreviousLineEnd)
				{
					// should display the character at the end of previous line
					CLine &currLine = *_Lines[i - 1];
					fy = (_FontHeight + _MultiLineSpace * _Scale) * (_Lines.size() - i) + dy;
					fx = currLine.getWidth() + currLine.getEndSpaces() * currLine.getSpaceWidth();
					fx = std::min(fx, nMaxWidth);

					x = fx / _Scale;
					y = fy / _Scale;
					return;
				}
				CLine &currLine = *_Lines[i];
				uint newCharIndex = charIndex + currLine.getNumChars() + currLine.getEndSpaces() + (_Lines[i]->getLF() ? 1 : 0);
				if ((sint) newCharIndex > index)
				{
					// ok, this line contains the character, now, see which word contains it.
					fy = (_FontHeight + _MultiLineSpace * _Scale) * (_Lines.size() - 1 - i) + dy;
					// see if the index is in the spaces at the end of line
					if (index - charIndex >= currLine.getNumChars())
					{
						uint numSpaces = index - charIndex - currLine.getNumChars();
						fx = currLine.getWidth() + numSpaces * _SpaceWidth;
						fx = std::min(fx, nMaxWidth);

						x = fx / _Scale;
						y = fy / _Scale;
						return;
					}
					// now, search containing word in current line
					float px = (float)_FirstLineX;
					for(uint k = 0; k < currLine.getNumWords(); ++k)
					{
						CWord &currWord = currLine.getWord(k);
						if ((sint) (charIndex + currWord.NumSpaces + currWord.Info.StringLength) >= index)
						{
							// character is in currWord or the in spaces preceding it
							// check if the character is in the word
							if ((uint) (index - charIndex) > currWord.NumSpaces)
							{
								// get the x position
								// compute the size
								UTextContext::CStringInfo si = TextContext->getStringInfo(currWord.Text, (ptrdiff_t)index - charIndex - currWord.NumSpaces);
								fx = px + si.StringWidth + currWord.NumSpaces * currLine.getSpaceWidth();

								x = fx / _Scale;
								y = fy / _Scale;
								return;
							}
							else
							{
								// character is in the spaces preceding the word
								fx = px + currLine.getSpaceWidth() * (index - charIndex);

								x = fx / _Scale;
								y = fy / _Scale;
								return;
							}
						}
						charIndex += (uint)currWord.Info.StringLength + currWord.NumSpaces;
						px += currWord.NumSpaces * currLine.getSpaceWidth() + currWord.Info.StringWidth;
					}
				}
				charIndex = newCharIndex;
			}

		}
		else
		{
			// get the x position
			// compute the size
			UTextContext::CStringInfo si = TextContext->getStringInfo(_Text, index);
			y = 0;
			x = (sint) ceilf(si.StringWidth / _Scale);
		}
	}

	// ***************************************************************************
	// Tool fct : From a word and a x coordinate (font scale), give the matching character index
	static uint getCharacterIndex(const std::string &textValue, float x, NL3D::UTextContext &textContext)
	{
		float px = 0.f;

		UTextContext::CStringInfo si;
		::u32string singleChar(1, ' ');
		uint i = 0;
		NLMISC::CUtfStringView sv(textValue);
		for (NLMISC::CUtfStringView::iterator it(sv.begin()), end(sv.end()); it != end; ++it, ++i)
		{
			// get character width
			singleChar[0] = *it;
			si = textContext.getStringInfo(singleChar);
			px += si.StringWidth;
			 // the character is at the i - 1 position
			if (px > x)
			{
				// if the half of the character is after the cursor, then prefer select the next one (like in Word)
				if(px-si.StringWidth/2.f < x)
					i++;
				break;
			}
		}
		return i;
	}

	// ***************************************************************************
	void CViewText::getCharacterIndexFromPosition(float x, float y, uint &index, bool &cursorAtPreviousLineEnd) const
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);

		if (!_TextLength && _Text.size())
			_TextLength = CUtfStringView(_Text).count();

		x *= _Scale;
		y = roundf(y * _Scale);

		// setup the text context
		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeOutline (_ShadowOutline);
		TextContext->setFontSize (_FontSize*_Scale);
		TextContext->setEmbolden (_Embolden);
		TextContext->setOblique (_Oblique);
		 // find the line where the character is
	//	CViewRenderer &rVR = *CViewRenderer::getInstance();
		uint charPos = 0;
		if (_MultiLine)
		{
			y -= getMultiMinOffsetY() * _Scale;
			// seek the line
			float py = 0.f;
			if (py > y)
			{
				index = (uint)_TextLength;
				cursorAtPreviousLineEnd = false;
				return;
			}
			sint line;
			for (line = (uint)_Lines.size() - 1; line >= 0; --line)
			{
				float newPy = py + _FontHeight + _MultiLineSpace * _Scale;
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

			cursorAtPreviousLineEnd = false;
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
						return;
					}
					else
					{
						// the coord is in the word itself
						index = charPos + currWord.NumSpaces + getCharacterIndex(currWord.Text, x - (px + spacesWidth), *TextContext);
						return;
					}
				}
				px = newPx;
				charPos += (uint)currWord.Info.StringLength + currWord.NumSpaces;
			}
			index =  charPos;
			return;
		}
		else
		{
			cursorAtPreviousLineEnd = false;
			if (y < 0)
			{
				index = (uint)_TextLength;
				return;
			}
			if (y > (sint) _FontHeight)
			{
				index = 0;
				return;
			}
			index = getCharacterIndex(_Text, x, *TextContext);
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
	void CViewText::setStringSelectionSkipingSpace(uint stringId, const std::string &text, sint charStart, sint charEnd)
	{
		sint	quadStart= charStart;
		sint	quadSize= charEnd-charStart;
		sint j;
		CUtfStringView sv(text);
		CUtfStringView::iterator it(sv.begin()), end(sv.end());
		for (j = 0; it != end && j < charStart; ++j, ++it)
		{
			if(*it == (u32char)' ')
				quadStart--;
		}
		for (j = charStart; it != end && j < charEnd; ++j, ++it)
		{
			if(*it == (u32char)' ')
				quadSize--;
		}
		// select what quad to skip
		CViewRenderer::getTextContext(_FontName)->setStringSelection(stringId, quadStart, quadSize); // FIXME: This assumes spaces are the only empty glyphs!
	}

	// ***************************************************************************
	void CViewText::clearLines()
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		for(uint k = 0; k < _Lines.size(); ++k)
		{
			_Lines[k]->clear(*TextContext);
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
		return _FirstLineX / _Scale;
	}

	// ***************************************************************************
	uint CViewText::getLastLineW () const
	{
		if (!_Lines.empty())
			return (uint)_Lines.back()->getWidth() / _Scale;
		return 0;
	}

	// ***************************************************************************
	void CViewText::setFirstLineX(sint firstLineX)
	{
		_FirstLineX = firstLineX * _Scale;
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
	void CViewText::CLine::addWord(const std::string &text, uint numSpaces, const CFormatInfo &wordFormat, float fontWidth, NL3D::UTextContext &textContext)
	{
		CWord word;
		word.build(text, textContext, numSpaces);
		word.Format= wordFormat;
		addWord(word, fontWidth);
	}

	// ***************************************************************************
	void CViewText::CLine::addWord(const CWord &word, float fontWidth)
	{
		_Words.push_back(word);
		_NumChars += word.NumSpaces + uint(word.Info.StringLength);
		_NumSpaces += word.NumSpaces;
		if (fabsf(word.Info.StringLine) > fabsf(_StringLine))
		{
			_StringLine = word.Info.StringLine;
		}
		// the width of the line must reach at least the Tab
		_WidthWithoutSpaces= max(_WidthWithoutSpaces, word.Format.TabX * fontWidth);
		// append the text space
		_WidthWithoutSpaces += word.Info.StringWidth;
	}

	// ***************************************************************************
	void CViewText::CLine::clear(NL3D::UTextContext &textContext)
	{
		for(uint k = 0; k < _Words.size(); ++k)
		{
			if (_Words[k].Index != 0xffffffff)
				textContext.erase(_Words[k].Index);
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
	void CViewText::CWord::build(const std::string &text, NL3D::UTextContext &textContext, uint numSpaces)
	{
		Text = text;
		NumSpaces = numSpaces;
		Index = textContext.textPush(text);
		Info = textContext.getStringInfo(Index);
		nlassert(Info.StringLength == CUtfStringView(text).count());
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
		static const char *spaceStr(" \t");
		static const char *lineFeedStr("\n");
		float maxWidth = 0;

		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeOutline (_ShadowOutline);
		TextContext->setFontSize (_FontSize*_Scale);
		TextContext->setEmbolden (_Embolden);
		TextContext->setOblique (_Oblique);

		TCharPos linePos = 0;
		while (linePos < _Text.length())
		{
			// Get the end of the line
			float lineWidth = 0;
			TCharPos lineEnd;
			lineEnd = _Text.find_first_of("\n", linePos);
			if (lineEnd == std::string::npos)
			{
				lineEnd = _Text.length();
			}

			std::string lineValue;
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

				std::string wordValue;
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

		return (sint32)ceilf(maxWidth / _Scale);
	}

	// ***************************************************************************
	sint32 CViewText::getMinUsedW() const
	{
		static const char *spaceOrLineFeedStr(" \n\t");
		float maxWidth = 0.0f;

		// Not multi line ? Same size than min
		if (!_MultiLine)
			return getMaxUsedW();

		// If we can clip word, size of the largest word
		if (_TextMode == ClipWord)
		{
			// No largest font parameter, return the font height
			return (sint32)ceilf(_FontHeight / _Scale);
		}
		// If we can't clip the words, return the size of the largest word
		else if ((_TextMode == DontClipWord) || (_TextMode == Justified) || (_TextMode == Centered))
		{
			NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
			TextContext->setHotSpot (UTextContext::BottomLeft);
			TextContext->setShaded (_Shadow);
			TextContext->setShadeOutline (_ShadowOutline);
			TextContext->setFontSize (_FontSize*_Scale);
			TextContext->setEmbolden (_Embolden);
			TextContext->setOblique (_Oblique);

			// Current position in text
			TCharPos currPos = 0;
			while (currPos < _Text.length())
			{
				// Current word
				std::string wordValue;
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
				if (maxWidth < si.StringWidth)
					maxWidth = si.StringWidth;

				// Next word
				currPos = wordEnd;
			}
		}

		return ceilf(maxWidth / _Scale);
	}

	// ***************************************************************************
	void CViewText::onInvalidateContent()
	{
		_InvalidTextContext= true;

		if (_ParentElm)
			_ParentElm->invalidateCoords();

		invalidateCoords();
	}

	// ***************************************************************************
	void CViewText::onInterfaceScaleChanged()
	{
		_Scale = CWidgetManager::getInstance()->getInterfaceScale();

		computeFontSize ();
		invalidateContent();

		CViewBase::onInterfaceScaleChanged();
	}

	// ***************************************************************************
	void CViewText::computeFontSize ()
	{
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
		TextContext->setHotSpot (UTextContext::BottomLeft);
		TextContext->setShaded (_Shadow);
		TextContext->setShadeOutline (_ShadowOutline);
		TextContext->setFontSize (_FontSize * _Scale);
		TextContext->setEmbolden (_Embolden);
		TextContext->setOblique (_Oblique);

#if 1

		UTextContext::CStringInfo si = TextContext->getStringInfo("XO");
		float xoHeight = si.StringHeight;

		si = TextContext->getStringInfo("XO\xC3\x81\xC3\x83");
		float upHeight = si.StringHeight;

		si = TextContext->getStringInfo("XOgq");
		float downHeight = si.StringHeight;
		float legHeight = si.StringLine;

		nlassert(upHeight >= xoHeight);
		nlassert(downHeight >= xoHeight);
		float diff;
		if (downHeight > upHeight)
		{
			diff = downHeight - xoHeight;
		}
		else
		{
			diff = upHeight - xoHeight;
			legHeight += upHeight - downHeight;
		}

		_FontHeight = xoHeight + diff + diff;
		_FontLegHeight = legHeight;

		// Space width
		si = TextContext->getStringInfo(" ");
		_SpaceWidth = si.StringWidth;

		// Font Width (used for <tab>)
		si = TextContext->getStringInfo("O");
		_FontWidth = si.StringWidth;

#else

		// Letter size
		UTextContext::CStringInfo si = TextContext->getStringInfo(_FontSizingChars);

		// font generator changes unknown glyphs to dot '.'. use fallback if it looks odd
		if (_FontSize > (si.StringHeight + si.StringLine))
		{
			si = TextContext->getStringInfo(_FontSizingFallback);
		}

		// add a padding of 1 pixel else the top will be truncated
		_FontHeight = si.StringHeight + 1;
		_FontLegHeight = si.StringLine;

		// Space width
		si = TextContext->getStringInfo(" ");
		_SpaceWidth = si.StringWidth;

		// Font Width (used for <tab>)
		si = TextContext->getStringInfo("_");
		_FontWidth = si.StringWidth;

#endif
	}


	// ***************************************************************************
	static inline bool	isColorTag(const std::string &s, uint index, uint textSize)
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
	static inline CRGBA	getColorTag(const std::string &s, uint &index)
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
	static inline bool	isTabTag(const std::string &s, uint index, uint textSize)
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
	static inline sint	getTabTag(const std::string &s, uint &index)
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
	static inline bool	isTooltipTag(const std::string &s, uint index, uint textSize)
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
	static inline std::string	getTooltipTag(const std::string &s, uint &index)
	{
		std::string result;
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
	void		CViewText::buildFormatTagText(const std::string &text, std::string &textBuild, std::vector<CViewText::CFormatTag> &formatTags, std::vector<std::string> &tooltips)
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
				string uitt = getTooltipTag(text, i);
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

#ifdef RYZOM_LUA_UCSTRING
	// ***************************************************************************
	void	CViewText::setTextFormatTagedAsUtf16(const ucstring &text)
	{
		setTextFormatTaged(text.toUtf8());
	}
#endif

	// ***************************************************************************
	void	CViewText::setTextFormatTaged(const std::string &text)
	{
		if( text.empty() )
			return;

		// to allow cache (avoid infinite recurse in updateCoords() in some case), compute in temp
		std::string							tempText;
		// static to avoid reallocation
		static std::vector<CFormatTag>		tempFormatTags;
		static std::vector<std::string>		tempTooltips;
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
			_Text = tempText;
			_HardText.clear();
			_TextLength = 0;

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
				std::string tempTooltipStr = tempTooltips[i];
				bool isI18N = NLMISC::startsWith(tempTooltipStr, "ui");
				pTooltip->setDefaultContextHelp(isI18N ? CI18N::get(tempTooltipStr) : tempTooltipStr);
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
			nlwarning("ViewText isn't multilined : uc_hardtext_format will not act as wanted !\n%s", text.c_str());
	}

#ifdef RYZOM_LUA_UCSTRING
	void CViewText::setSingleLineTextFormatTagedAsUtf16(const ucstring &text)
	{
		setSingleLineTextFormatTaged(text.toUtf8());
	}
#endif

	void CViewText::setSingleLineTextFormatTaged(const std::string &text)
	{
		if( text.empty() )
			return;

		// to allow cache (avoid infinite recurse in updateCoords() in some case), compute in temp
		std::string							tempText;
		static std::vector<CFormatTag>		tempLetterColors;
		static std::vector<std::string>		tempTooltips;

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
				if(tempText[textIndex] == ' ')
					spacesNb++;

				textIndex++;
			}

			formatTag.Index -= spacesNb;
		}

		// convert in ULetterColors
		NL3D::UTextContext *TextContext = CViewRenderer::getTextContext(_FontName);
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
			_Text = tempText;
			_HardText.clear();
			_TextLength = 0;
			invalidateContent ();
		}

		// this color format is available only if not multilined
		if (_MultiLine)
			nlwarning("ViewText is multilined : uc_hardtext_single_line_format will not act as wanted !\n%s", text.c_str());
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
		_TextLength = 0;
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

		int version = f.serialVersion(101);
		nlassert(version >= 100);

		f.serial(_Localized);
		SERIAL_SINT(_FontSize);
		SERIAL_UINT(_FontWidth);
		SERIAL_UINT(_FontHeight);
		SERIAL_UINT(_FontLegHeight);
		f.serial(_SpaceWidth);
		f.serial(_Color);
		f.serial(_Shadow);
		f.serial(_ShadowOutline);
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
					setTextFormatTaged(text.toUtf8());
				}
				else
				{
					setSingleLineTextFormatTaged(text.toUtf8());
				}
			}
			else
			{
				setText(text.toUtf8());
			}
		}
		else
		{
			ucstring text = CUtfStringView(_Text).toUtf16();
			f.serial(text);
		}

		#undef SERIAL_UINT
		#undef SERIAL_SINT
	}


	// ***************************************************************************

}

