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
#include "nel/gui/group_editbox.h"
#include "nel/misc/command.h"
#include "nel/gui/view_text.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/interface_factory.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


namespace NLGUI
{


	/////////////
	// STATICS //
	/////////////

	sint32         CGroupEditBox::_SelectCursorPos = 0;
	CGroupEditBox *CGroupEditBox::_MenuFather = NULL;
	CGroupEditBox::IComboKeyHandler* CGroupEditBox::comboKeyHandler = NULL;


	// ----------------------------------------------------------------------------
	NLMISC_REGISTER_OBJECT(CViewBase, CGroupEditBox, std::string, "edit_box");

	CGroupEditBox::CGroupEditBox(const TCtorParam &param) :
									CGroupEditBoxBase(param),
									_BlinkTime(0.f),
									_CursorPos(0),
									_MaxNumChar(std::numeric_limits<uint32>::max()),
									_MaxNumReturn(std::numeric_limits<uint32>::max()),
									_MaxFloatPrec(20),
									_MaxCharsSize(32768),
									_FirstVisibleChar(0),
									_LastVisibleChar(0),
									_SelectingText(false),
									_ViewText(NULL),
									_MaxHistoric(0),
									_CurrentHistoricIndex(-1),
									_PrevNumLine(1),
									_EntryType(Text),
									_Setupped(false),
									_BypassNextKey(false),
									_BlinkState(false),
									_CursorAtPreviousLineEnd(false),
									_LooseFocusOnEnter(true),
									_ResetFocusOnHide(false),
									_BackupFatherContainerPos(false),
									_WantReturn(false),
									_Savable(true),
									_DefaultInputString(false),
									_Frozen(false),
									_CanRedo(false),
									_CanUndo(false),
									_CursorTexID(-1),
									_CursorWidth(0),
									_IntegerMinValue(INT_MIN),
									_IntegerMaxValue(INT_MAX),
									_PositiveIntegerMinValue(0),
									_PositiveIntegerMaxValue(UINT_MAX),
									_ViewTextDeltaX(0)

	{
		_Prompt = ">";
		_BackSelectColor= CRGBA::White;
		_TextSelectColor= CRGBA::Black;
	}

	// ----------------------------------------------------------------------------
	CGroupEditBox::~CGroupEditBox()
	{
		if (this == _CurrSelection) _CurrSelection = NULL;
		if (CWidgetManager::getInstance()->getCaptureKeyboard() == this || CWidgetManager::getInstance()->getOldCaptureKeyboard() == this)
		{
			CWidgetManager::getInstance()->resetCaptureKeyboard();
		}
	}

	std::string CGroupEditBox::getProperty( const std::string &name ) const
	{
		if( name == "onchange" )
		{
			return _AHOnChange;
		}
		else
		if( name == "onchange_params" )
		{
			return _ParamsOnChange;
		}
		else
		if( name == "on_focus_lost" )
		{
			return _AHOnFocusLost;
		}
		else
		if( name == "on_focus_lost_params" )
		{
			return _AHOnFocusLostParams;
		}
		else
		if( name == "on_focus" )
		{
			return _AHOnFocus;
		}
		else
		if( name == "on_focus_params" )
		{
			return _AHOnFocusParams;
		}
		else
		if( name == "max_num_chars" )
		{
			return toString( _MaxNumChar );
		}
		else
		if( name == "max_num_return" )
		{
			return toString( _MaxNumReturn );
		}
		else
		if( name == "max_chars_size" )
		{
			return toString( _MaxCharsSize );
		}
		else
		if( name == "enter_loose_focus" )
		{
			return toString( _LooseFocusOnEnter );
		}
		else
		if( name == "enter_recover_focus" )
		{
			return toString( _RecoverFocusOnEnter );
		}
		else
		if( name == "prompt" )
		{
			return _Prompt.toString();
		}
		else
		if( name == "enter_type" )
		{
			switch( _EntryType )
			{
			case Integer:
				return "integer";
				break;

			case PositiveInteger:
				return "positive_integer";
				break;

			case Float:
				return "float";
				break;

			case PositiveFloat:
				return "positive_float";
				break;

			case Alpha:
				return "alpha";
				break;

			case AlphaNum:
				return "alpha_num";
				break;

			case AlphaNumSpace:
				return "alpha_num_space";
				break;

			case Password:
				return "password";
				break;

			case Filename:
				return "filename";
				break;

			case PlayerName:
				return "playername";
				break;

			default:
				break;
			}
			
			return "text";
		}
		else
		if( name == "menu_r" )
		{
			return _ListMenuRight;
		}
		else
		if( name == "max_historic" )
		{
			return toString( _MaxHistoric );
		}
		else
		if( name == "backup_father_container_pos" )
		{
			return toString( _BackupFatherContainerPos );
		}
		else
		if( name == "want_return" )
		{
			return toString( _WantReturn );
		}
		else
		if( name == "savable" )
		{
			return toString( _Savable );
		}
		else
		if( name == "max_float_prec" )
		{
			return toString( _MaxFloatPrec );
		}
		else
		if( name == "negative_filter" )
		{
			std::string s;
			s.reserve( _NegativeFilter.size() );

			std::vector< char >::const_iterator itr;
			for( itr = _NegativeFilter.begin(); itr != _NegativeFilter.end(); ++itr )
				s.push_back( *itr );
			
			return s;
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CGroupEditBox::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "onchange" )
		{
			_AHOnChange = value;
			return;
		}
		else
		if( name == "onchange_params" )
		{
			_ParamsOnChange = value;
			return;
		}
		else
		if( name == "on_focus_lost" )
		{
			_AHOnFocusLost = value;
			return;
		}
		else
		if( name == "on_focus_lost_params" )
		{
			_AHOnFocusLostParams = value;
			return;
		}
		else
		if( name == "on_focus" )
		{
			_AHOnFocus = value;
			return;
		}
		else
		if( name == "on_focus_params" )
		{
			_AHOnFocusParams = value;
			return;
		}
		else
		if( name == "max_num_chars" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MaxNumChar = i;
			return;
		}
		else
		if( name == "max_num_return" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MaxNumReturn = i;
			return;
		}
		else
		if( name == "max_chars_size" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_MaxCharsSize = i;
			return;
		}
		else
		if( name == "enter_loose_focus" )
		{
			bool b;
			if( fromString( value, b ) )
				_LooseFocusOnEnter = b;
			return;
		}
		else
		if( name == "enter_recover_focus" )
		{
			bool b;
			if( fromString( value, b ) )
				_RecoverFocusOnEnter = b;
			return;
		}
		else
		if( name == "prompt" )
		{
			_Prompt = value;
			return;
		}
		else
		if( name == "enter_type" )
		{
			if( value == "integer" )
				_EntryType = Integer;
			else
			if( value == "positive_integer" )
				_EntryType = PositiveInteger;
			else
			if( value == "float" )
				_EntryType = Float;
			else
			if( value == "positive_float" )
				_EntryType = PositiveFloat;
			else
			if( value == "alpha" )
				_EntryType = Alpha;
			else
			if( value == "alpha_num" )
				_EntryType = AlphaNum;
			else
			if( value == "alpha_num_space" )
				_EntryType = AlphaNumSpace;
			else
			if( value == "password" )
				_EntryType = Password;
			else
			if( value == "filename" )
				_EntryType = Filename;
			else
			if( value == "playername" )
				_EntryType = PlayerName;

			return;
		}
		else
		if( name == "menu_r" )
		{
			_ListMenuRight = value;
			return;
		}
		else
		if( name == "max_historic" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MaxHistoric = i;
			return;
		}
		else
		if( name == "backup_father_container_pos" )
		{
			bool b;
			if( fromString( value, b ) )
				_BackupFatherContainerPos = b;
			return;
		}
		else
		if( name == "want_return" )
		{
			bool b;
			if( fromString( value, b ) )
				_WantReturn = b;
			return;
		}
		else
		if( name == "savable" )
		{
			bool b;
			if( fromString( value, b ) )
				_Savable = b;
			return;
		}
		else
		if( name == "max_float_prec" )
		{
			uint32 i;
			if( fromString( value, i ) )
				_MaxFloatPrec = i;
			return;
		}
		else
		if( name == "negative_filter" )
		{
			_NegativeFilter.clear();

			std::string::size_type i;
			for( i = 0; i < value.size(); i++ )
				_NegativeFilter.push_back( value[ i ] );
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CGroupEditBox::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "edit_box" );
		xmlSetProp( node, BAD_CAST "onchange", BAD_CAST _AHOnChange.c_str() );
		xmlSetProp( node, BAD_CAST "onchange_params", BAD_CAST _ParamsOnChange.c_str() );
		xmlSetProp( node, BAD_CAST "on_focus_lost", BAD_CAST _AHOnFocusLost.c_str() );
		xmlSetProp( node, BAD_CAST "on_focus_lost_params", BAD_CAST _AHOnFocusLostParams.c_str() );
		xmlSetProp( node, BAD_CAST "on_focus", BAD_CAST _AHOnFocus.c_str() );
		xmlSetProp( node, BAD_CAST "on_focus_params", BAD_CAST _AHOnFocusParams.c_str() );
		xmlSetProp( node, BAD_CAST "max_num_chars", BAD_CAST toString( _MaxNumChar ).c_str() );
		xmlSetProp( node, BAD_CAST "max_num_return", BAD_CAST toString( _MaxNumReturn ).c_str() );
		xmlSetProp( node, BAD_CAST "max_chars_size", BAD_CAST toString( _MaxCharsSize ).c_str() );
		xmlSetProp( node, BAD_CAST "enter_loose_focus", BAD_CAST toString( _LooseFocusOnEnter ).c_str() );
		xmlSetProp( node, BAD_CAST "enter_recover_focus", BAD_CAST toString( _RecoverFocusOnEnter ).c_str() );
		xmlSetProp( node, BAD_CAST "prompt", BAD_CAST _Prompt.toString().c_str() );

		std::string e;
		switch( _EntryType )
		{
		case Integer:
			e = "integer";
			break;

		case PositiveInteger:
			e = "positive_integer";
			break;

		case Float:
			e = "float";
			break;

		case PositiveFloat:
			e = "positive_float";
			break;

		case Alpha:
			e = "alpha";
			break;

		case AlphaNum:
			e = "alpha_num";
			break;

		case AlphaNumSpace:
			e = "alpha_num_space";
			break;

		case Password:
			e = "password";
			break;

		case Filename:
			e = "filename";
			break;

		case PlayerName:
			e = "playername";
			break;

		default:
			break;
		}

		xmlSetProp( node, BAD_CAST "enter_type", BAD_CAST e.c_str() );
		xmlSetProp( node, BAD_CAST "menu_r", BAD_CAST _ListMenuRight.c_str() );
		xmlSetProp( node, BAD_CAST "max_historic", BAD_CAST toString( _MaxHistoric ).c_str() );
		xmlSetProp( node, BAD_CAST "backup_father_container_pos",
			BAD_CAST toString( _BackupFatherContainerPos ).c_str() );
		xmlSetProp( node, BAD_CAST "want_return", BAD_CAST toString( _WantReturn ).c_str() );
		xmlSetProp( node, BAD_CAST "savable", BAD_CAST toString( _Savable ).c_str() );
		xmlSetProp( node, BAD_CAST "max_float_prec", BAD_CAST toString( _MaxFloatPrec ).c_str() );

		std::string s;
		s.reserve( _NegativeFilter.size() );

		std::vector< char >::const_iterator itr;
		for( itr = _NegativeFilter.begin(); itr != _NegativeFilter.end(); ++itr )
			s.push_back( *itr );

		xmlSetProp( node, BAD_CAST "negative_filter", BAD_CAST s.c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CGroupEditBox::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if(!CInterfaceGroup::parse(cur, parentGroup))
			return false;
		CXMLAutoPtr prop;

		if (! CInterfaceGroup::parse(cur,parentGroup) )
		{
			string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
			nlinfo(tmp.c_str());
			return false;
		}

		// NB: use InterfaceGroup "OnEnter" data. Different script params for an historic reason
		CAHManager::getInstance()->parseAH(cur, "onenter", "params", _AHOnEnter, _AHOnEnterParams);

		if( editorMode )
		{
			prop = (char*) xmlGetProp( cur, BAD_CAST "onenter" );
			if (prop)
				mapAHString( "onenter", std::string( (const char*)prop ) );
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"onchange" );
		if (prop) _AHOnChange = (const char *) prop;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"onchange_params" );
		if (prop) _ParamsOnChange = (const char *) prop;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_focus_lost" );
		if (prop) _AHOnFocusLost = (const char *) prop;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_focus_lost_params" );
		if (prop) _AHOnFocusLostParams = (const char *) prop;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_focus" );
		if (prop) _AHOnFocus = (const char *) prop;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"on_focus_params" );
		if (prop) _AHOnFocusParams = (const char *) prop;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"max_num_chars" );
		if (prop) fromString((const char*)prop, _MaxNumChar);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"max_num_return" );
		if (prop) fromString((const char*)prop, _MaxNumReturn);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"max_chars_size" );
		if (prop) fromString((const char*)prop, _MaxCharsSize);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"enter_loose_focus" );
		if (prop) _LooseFocusOnEnter = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"enter_recover_focus" );
		if (prop) _RecoverFocusOnEnter = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"reset_focus_on_hide" );
		if (prop) _ResetFocusOnHide = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"prompt" );
		if (prop) _Prompt = (const char*)prop;

		prop = (char*) xmlGetProp( cur, (xmlChar*)"entry_type" );
		_EntryType = Text;
		if (prop)
		{
			if (stricmp(prop, "text") == 0) _EntryType = Text;
			else if (stricmp(prop, "integer") == 0) _EntryType = Integer;
			else if (stricmp(prop, "positive_integer") == 0) _EntryType = PositiveInteger;
			else if (stricmp(prop, "float") == 0) _EntryType = Float;
			else if (stricmp(prop, "positive_float") == 0) _EntryType = PositiveFloat;
			else if (stricmp(prop, "alpha") == 0) _EntryType = Alpha;
			else if (stricmp(prop, "alpha_num") == 0) _EntryType = AlphaNum;
			else if (stricmp(prop, "alpha_num_space") == 0) _EntryType = AlphaNumSpace;
			else if (stricmp(prop, "password") == 0) _EntryType = Password;
			else if (stricmp(prop, "filename") == 0) _EntryType = Filename;
			else if (stricmp(prop, "playername") == 0) _EntryType = PlayerName;
			else
				nlwarning("<CGroupEditBox::parse> Unknown entry type %s", (const char *) prop);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_r" );
		if (prop)
		{
			string tmp = (const char *) prop;
			_ListMenuRight = strlwr(tmp);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"max_historic" );
		if (prop) fromString((const char*)prop, _MaxHistoric);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"backup_father_container_pos" );
		if (prop) _BackupFatherContainerPos = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"want_return" );
		if (prop) _WantReturn = convertBool(prop);

		prop = (char*) xmlGetProp( cur, (xmlChar*)"savable" );
		if (prop) _Savable = convertBool(prop);

		// For float conversion only
		prop = (char*) xmlGetProp( cur, (xmlChar*)"max_float_prec" );
		if (prop)
		{
			fromString((const char*)prop, _MaxFloatPrec);
			_MaxFloatPrec = min((uint32)20, _MaxFloatPrec);
		}

		// negative characters filter
		prop = (char*) xmlGetProp( cur, (xmlChar*)"negative_filter" );
		if (prop)
		{
			uint length = (uint)strlen(prop);
			_NegativeFilter.resize(length);
			std::copy((const char *) prop, (const char *) prop + length, _NegativeFilter.begin());
		}

		return true;
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::draw ()
	{
		//
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		//
		/*CRGBA col;
		col.modulateFromColor(CRGBA(64,64,64,255), pIM->getGlobalColorForContent());
		rVR.drawRotFlipBitmap (_RenderLayer, _XReal, _YReal, _WReal, _HReal, 0, false, rVR.getBlankTextureId(), col );
		*/

		// Frozen? display text in "grayed"
		CRGBA	bkupTextColor;
		if(_Frozen && _ViewText)
		{
			bkupTextColor= _ViewText->getColor();
			CRGBA	grayed= bkupTextColor;
			grayed.A>>=2;
			_ViewText->setColor(grayed);
		}

		// draw the group and thus the text
		CInterfaceGroup::draw();

		// restore the text color if changed
		if(_Frozen && _ViewText)
			_ViewText->setColor(bkupTextColor);

		// no text setuped? abort
		if (!_ViewText) return;

		// Display the selection if needed
		if (_CurrSelection == this && _SelectCursorPos!=_CursorPos)
		{
			sint32	blankTextId= rVR.getBlankTextureId();
			CRGBA	col= _BackSelectColor;
			col.A= CWidgetManager::getInstance()->getGlobalColorForContent().A;
			sint32	minPos= min(_CursorPos, _SelectCursorPos) + (sint32)_Prompt.length();
			sint32	maxPos= max(_CursorPos, _SelectCursorPos) + (sint32)_Prompt.length();

			// get its position on screen
			sint cxMinPos, cyMinPos;
			sint cxMaxPos, cyMaxPos;
			sint height;
			_ViewText->getCharacterPositionFromIndex(minPos, false, cxMinPos, cyMinPos, height);
			_ViewText->getCharacterPositionFromIndex(maxPos, false, cxMaxPos, cyMaxPos, height);

			// Multiline selection if cy different!
			if(cyMinPos!=cyMaxPos)
			{
				nlassert(cyMaxPos<cyMinPos);
				// draw the 1st quad from the first line from min pos to end of line
				rVR.drawRotFlipBitmap(_RenderLayer, _ViewText->getXReal() + cxMinPos, _ViewText->getYReal() + cyMinPos, _ViewText->getW()-cxMinPos, height, 0, 0, blankTextId, col);

				// Draw One quad for all lines into the big selection (if any)
				sint32	cyBigQuad= cyMaxPos+height;
				if(cyBigQuad<cyMinPos)
				{
					rVR.drawRotFlipBitmap(_RenderLayer, _ViewText->getXReal(), _ViewText->getYReal() + cyBigQuad, _ViewText->getW(), cyMinPos-cyBigQuad, 0, 0, blankTextId, col);
				}

				// draw the 3rd quad from the last line from start of line to max pos
				rVR.drawRotFlipBitmap(_RenderLayer, _ViewText->getXReal(), _ViewText->getYReal() + cyMaxPos, cxMaxPos, height, 0, 0, blankTextId, col);
			}
			else
			{
				rVR.drawRotFlipBitmap(_RenderLayer, _ViewText->getXReal() + cxMinPos, _ViewText->getYReal() + cyMinPos, cxMaxPos-cxMinPos, height, 0, 0, blankTextId, col);
			}

			// Draw The Selection String in black
			CRGBA	bkup= _ViewText->getColor();
			_ViewText->setColor(_TextSelectColor);
			_ViewText->enableStringSelection(minPos, maxPos);
			// redraw just this string,clipped by the group
			CInterfaceGroup::drawElement(_ViewText);
			// bkup
			_ViewText->setColor(bkup);
			_ViewText->disableStringSelection();
		}

		// Display the cursor if needed
		if (CWidgetManager::getInstance()->getCaptureKeyboard () == this)
		{
			const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

			_BlinkTime += ( static_cast< float >( times.frameDiffMs ) / 1000.0f );
			if (_BlinkTime > 0.25f)
			{
				_BlinkTime = fmodf(_BlinkTime, 0.25f);
				_BlinkState = !_BlinkState;
			}
			if (_BlinkState) // is the cursor shown ?
			{
				// get its position on screen
				sint cx, cy;
				sint height;
				_ViewText->getCharacterPositionFromIndex(_CursorPos + (sint)_Prompt.length(), _CursorAtPreviousLineEnd, cx, cy, height);
				// display the cursor
				// get the texture for the cursor
				if (_CursorTexID == -1)
				{
					_CursorTexID = rVR.getTextureIdFromName("text_cursor.tga");
					sint32 dummyCursorHeight;
					rVR.getTextureSizeFromId(_CursorTexID, _CursorWidth, dummyCursorHeight);
				}
				// draw in a layer after to be on TOP of the text
				sint32 cursorx = std::max((sint) 0, (sint)(cx - (_CursorWidth >> 1)));
				rVR.drawRotFlipBitmap(_RenderLayer+1, _ViewText->getXReal() + cursorx, _ViewText->getYReal() + cy, 3, height, 0, 0, _CursorTexID);
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::copy()
	{
		if (_CurrSelection != this)
		{
			nlwarning("Selection can only be on focus");
		}
		stopParentBlink();

		// get the selection and copy it
		if (CViewRenderer::getInstance()->getDriver()->copyTextToClipboard(getSelection()))
			nlinfo ("Chat input was copied in the clipboard");
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::paste()
	{
		if(_CurrSelection != NULL)
		{
			if (_CurrSelection != this)
			{
				nlwarning("Selection can only be on focus");
			}
			cutSelection();
		}

		ucstring sString;

		if (CViewRenderer::getInstance()->getDriver()->pasteTextFromClipboard(sString))
		{
			// append string now
			appendStringFromClipboard(sString);
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::appendStringFromClipboard(const ucstring &str)
	{
		stopParentBlink();
		makeTopWindow();

		writeString(str, true, false);
		nlinfo ("Chat input was pasted from the clipboard");

		triggerOnChangeAH();

		_CursorAtPreviousLineEnd = false;
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::writeString(const ucstring &str, bool replace, bool atEnd)
	{
		sint length = (sint)str.length();

		ucstring toAppend;
		// filter character depending on the entry type
		switch (_EntryType)
		{
			case Text:
			case Password:
			{
				if (_NegativeFilter.empty())
				{
					toAppend = str;
				}
				else
				{
					for (sint k = 0; k < length; ++k)
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
				// remove '\r' characters
				toAppend.erase(std::remove(toAppend.begin(), toAppend.end(), (ucchar) '\r'), toAppend.end());

			}
			break;
			case PositiveInteger:
			case PositiveFloat:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isdigit(str[k]) || str[k]== ' ' ||
						(_EntryType==PositiveFloat && str[k]=='.') )
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case Integer:
			case Float:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isdigit(str[k]) || str[k]== ' ' || str[k]== '-' ||
						(_EntryType==Float && str[k]=='.') )
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case AlphaNumSpace:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isValidAlphaNumSpace(str[k]))
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case AlphaNum:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isValidAlphaNum(str[k]))
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case Alpha:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isValidAlpha(str[k]))
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case Filename:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isValidFilenameChar(str[k]))
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
			break;
			case PlayerName:
			{
				for (sint k = 0; k < length; ++k)
				{
					if (isValidPlayerNameChar(str[k]))
					{
						if (!isFiltered(str[k]))
						{
							toAppend += str[k];
						}
					}
				}
			}
		}
		length = (sint)toAppend.size();
		if ((uint) (_InputString.length() + length) > _MaxNumChar)
		{
			length = _MaxNumChar - (sint)_InputString.length();
		}
		ucstring toAdd = toAppend.substr(0, length);
		sint32	minPos;
		sint32	maxPos;
		if (_CurrSelection == this)
		{
			minPos = min(_CursorPos, _SelectCursorPos);
			maxPos = max(_CursorPos, _SelectCursorPos);
		}
		else
		{
			minPos = _CursorPos;
			maxPos = _CursorPos;
		}

		nlinfo("%d, %d", minPos, maxPos);
		if (replace)
		{
			_InputString = _InputString.substr(0, minPos) + toAdd + _InputString.substr(maxPos);
			_CursorPos = minPos+(sint32)toAdd.length();
		}
		else
		{
			if (atEnd)
			{
				_InputString = _InputString.substr(0, maxPos) + toAdd + _InputString.substr(maxPos);
				_CursorPos = maxPos;
				_SelectCursorPos = _CursorPos;

			}
			else
			{
				_InputString = _InputString.substr(0, minPos) + toAdd + _InputString.substr(minPos);
				_CursorPos = minPos+(sint32)toAdd.length();
				_SelectCursorPos = maxPos+(sint32)toAdd.length();
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::handleEventChar(const NLGUI::CEventDescriptorKey &rEDK)
	{
		stopParentBlink();
		switch(rEDK.getChar())
		{
			case KeyESCAPE:
				_CurrentHistoricIndex= -1;
				CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
				// stop selection
				_CurrSelection = NULL;
				_CursorAtPreviousLineEnd = false;
			break;
			case KeyTAB:
				makeTopWindow();
			break;
			// OTHER
			default:
				if ((rEDK.getChar() == KeyRETURN) && !_WantReturn)
				{
					// update historic.
					if(_MaxHistoric)
					{
						if( !_InputString.empty() )
						{
							_Historic.push_front(_InputString);
							if(_Historic.size()>_MaxHistoric)
								_Historic.pop_back();
						}
						_CurrentHistoricIndex= -1;
					}

					_CursorPos = 0;
					// loose the keyboard focus
					if (NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:ENTER_DONT_QUIT_CB")->getValue32() == 0)
					{
						if(_LooseFocusOnEnter)
							CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
					}
					// stop selection
					_CurrSelection = NULL;
					_CursorAtPreviousLineEnd = false;
					CAHManager::getInstance()->runActionHandler(_AHOnEnter, this, _AHOnEnterParams);
				}
				else
				{
					// If the char is not alphanumeric -> return.
					//		if(!isalnum(ec.Char))
					//			return
					if( (rEDK.getChar()>=32) || (rEDK.getChar() == KeyRETURN) )
					{
						if (rEDK.getChar() == KeyRETURN)
						{
							ucstring	copyStr= _InputString;
							if ((uint) std::count(copyStr.begin(), copyStr.end(), '\n') >= _MaxNumReturn)
								break;
						}

						// if selection is activated, then cut the selection
						if(_CurrSelection != NULL)
						{
							if (_CurrSelection != this)
							{
								nlwarning("Selection can only be on focus");
							}
							cutSelection();
						}

						ucchar c = (rEDK.getChar() == KeyRETURN)?'\n':rEDK.getChar();
						if (isFiltered(c)) return;
						switch(_EntryType)
						{
							case Integer:
								if (c > 255 || !(c =='-' || c==' ' || isdigit((int) c)))
									return;
							break;
							case PositiveInteger:
								if (c > 255 || !(c == ' ' || isdigit((int) c)))
									return;
							break;
							case Float:
								if (c > 255 || !(c =='-' || c==' ' || c=='.' || isdigit((int) c)))
									return;
								break;
							case PositiveFloat:
								if (c > 255 || !(c == ' ' || c=='.' || isdigit((int) c)))
									return;
								break;
							case AlphaNumSpace:
								if (!isValidAlphaNumSpace(c))
									return;
							break;
							case AlphaNum:
								if (!isValidAlphaNum(c))
									return;
							break;
							case Alpha:
								if (!isValidAlpha(c))
									return;
							break;
							case Filename:
								if (!isValidFilenameChar(c))
									return;
								break;
							case PlayerName:
								if (!isValidPlayerNameChar(c))
									return;
							break;
							default:
								break;
						}
						// verify integer bounds
						if(_EntryType==Integer && (_IntegerMinValue!=INT_MIN || _IntegerMaxValue!=INT_MAX))
						{
							// estimate new string
							ucstring	copyStr= _InputString;
							ucstring::iterator it = copyStr.begin() + _CursorPos;
							copyStr.insert(it, c);
							sint32	value;
							fromString(copyStr.toString(), value);
							// if out of bounds, abort char
							if(value<_IntegerMinValue || value>_IntegerMaxValue)
								return;
						}
						// verify integer bounds
						if(_EntryType==PositiveInteger && (_PositiveIntegerMinValue!=0 || _PositiveIntegerMaxValue!=UINT_MAX))
						{
							// estimate new string
							ucstring	copyStr= _InputString;
							ucstring::iterator it = copyStr.begin() + _CursorPos;
							copyStr.insert(it, c);
							// \todo yoyo: this doesn't really work i think....
							uint32	value;
							fromString(copyStr.toString(), value);
							// if out of bounds, abort char
							if(value<_PositiveIntegerMinValue || value>_PositiveIntegerMaxValue)
								return;
						}
						// verify max num char
						if ((uint) _InputString.length() < _MaxNumChar)
						{
							makeTopWindow();
							ucstring::iterator it = _InputString.begin() + _CursorPos;
							_InputString.insert(it, c);
							++ _CursorPos;
							triggerOnChangeAH();
						}
						if (rEDK.getChar() == KeyRETURN)
						{
							CAHManager::getInstance()->runActionHandler(_AHOnEnter, this, _AHOnEnterParams);
						}
					}
					_CursorAtPreviousLineEnd = false;
				}
			break;
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::handleEventString(const NLGUI::CEventDescriptorKey &rEDK)
	{
		appendStringFromClipboard(rEDK.getString());
	}

	// ----------------------------------------------------------------------------
	bool CGroupEditBox::undo()
	{
		if (CWidgetManager::getInstance()->getCaptureKeyboard() != this) return false;
		if (!_CanUndo) return false;
		_ModifiedInputString = _InputString;
		setInputString(_StartInputString);
		_CanUndo = false;
		_CanRedo = true;
		setCursorPos((sint32)_InputString.length());
		setSelectionAll();
		return true;
	}

	// ----------------------------------------------------------------------------
	bool CGroupEditBox::redo()
	{
		if (CWidgetManager::getInstance()->getCaptureKeyboard() != this) return false;
		if (!_CanRedo) return false;
		setInputString(_ModifiedInputString);
		_CanUndo = true;
		_CanRedo = false;
		setCursorPos((sint32)_InputString.length());
		setSelectionAll();
		return true;
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::triggerOnChangeAH()
	{
		_CanUndo = true;
		_CanRedo = false;
		
		if (!_AHOnChange.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);

	}

	// ----------------------------------------------------------------------------
	bool CGroupEditBox::expand()
	{
		if ((_EntryType == Integer) ||
			(_EntryType == PositiveInteger) ||
			(_EntryType == Float) ||
			(_EntryType == PositiveFloat) ||
			(_EntryType == AlphaNumSpace) ||
			(_EntryType == AlphaNum) ||
			(_EntryType == Alpha) ||
			(_EntryType == Filename) ||
			(_EntryType == PlayerName)
			)
			return false;

		if(!_InputString.empty())
		{
			if (_InputString[0] == '/')
			{
				makeTopWindow();
				// for french, deutsch and russian, be aware of unicode
				std::string command = ucstring(_InputString.substr(1)).toUtf8();
				ICommand::expand(command);
				// then back to ucstring
				_InputString.fromUtf8(command);
				_InputString = '/' + _InputString;
				_CursorPos = (sint32)_InputString.length();
				_CursorAtPreviousLineEnd = false;
				triggerOnChangeAH();
				return true;
			}
		}
		return false;
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::back()
	{
		makeTopWindow();
		// if selection is activated and not same cursors pos, then cut the selection
		if(_CurrSelection != NULL && _CursorPos != _SelectCursorPos)
		{
			if (_CurrSelection != this)
			{
				nlwarning("Selection can only be on focus");
			}
			cutSelection();
			_CursorAtPreviousLineEnd = false;
		}
		// else delete last character
		else if(_InputString.size () > 0 && _CursorPos != 0)
		{
			ucstring::iterator it = _InputString.begin() + (_CursorPos - 1);
			_InputString.erase(it);
			-- _CursorPos;
			_CursorAtPreviousLineEnd = false;
			triggerOnChangeAH();
		}
		// must stop selection in all case
		if (_CurrSelection)
		{
			if (_CurrSelection != this)
			{
				nlwarning("Selection can only be on focus");
			}
			_CurrSelection = NULL;
		}
	}

	// ----------------------------------------------------------------------------
	bool CGroupEditBox::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (!_Active || !_ViewText)
			return false;
		if (event.getType() == NLGUI::CEventDescriptor::key)
		{
			if (_BypassNextKey)
			{
				_BypassNextKey = false;
				return true;
			}
			///////////////
			// KEY EVENT //
			///////////////
			const NLGUI::CEventDescriptorKey &rEDK = (const NLGUI::CEventDescriptorKey&)event;
			switch(rEDK.getKeyEventType())
			{
				case NLGUI::CEventDescriptorKey::keychar: handleEventChar(rEDK); break;
				case NLGUI::CEventDescriptorKey::keystring: handleEventString(rEDK); break;
				default: break;
			}
			// update the text
			setInputString(_InputString);

			// if event of type char or string, consider handle all of them
			if( rEDK.getKeyEventType()==NLGUI::CEventDescriptorKey::keychar || rEDK.getKeyEventType()==NLGUI::CEventDescriptorKey::keystring )
				return true;
			// Else filter the EventKeyDown AND EventKeyUp.
			else
			{
				// Look into the input handler Manager if the key combo has to be considered as handled
				if( ( CGroupEditBox::comboKeyHandler != NULL ) && comboKeyHandler->isComboKeyChat(rEDK) )
					return true;
				else
					return false;
			}
		}
		else
		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			/////////////////
			// MOUSE EVENT //
			/////////////////
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
			{
				if (CWidgetManager::getInstance()->getCapturePointerRight() == this)
				{
					CWidgetManager::getInstance()->setCapturePointerRight(NULL);
					if (!_ListMenuRight.empty())
					{
						if (CCtrlDraggable::getDraggedSheet() == NULL)
						{
							_MenuFather = this;
							CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuRight);
							stopParentBlink();
						}
					}
					return true;
				}
				else
				{
					return false;
				}
			}

			// if click, and not frozen, then get the focus
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup && !_Frozen)
			{
				_SelectingText = false;
				if (_SelectCursorPos == _CursorPos)
					_CurrSelection = NULL;
				
				return true;
			}

			if (!isIn(eventDesc.getX(), eventDesc.getY()))
				return false;

			// if click, and not frozen, then get the focus
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown && !_Frozen)
			{
				_SelectingText = true;
				stopParentBlink();
				CWidgetManager::getInstance()->setCaptureKeyboard (this);
				CWidgetManager::getInstance()->setCapturePointerLeft (this);
				// set the right cursor position
				uint newCurPos;
				bool cursorAtPreviousLineEnd;
				_ViewText->getCharacterIndexFromPosition(eventDesc.getX() - _ViewText->getXReal(), eventDesc.getY() - _ViewText->getYReal(), newCurPos, cursorAtPreviousLineEnd);
				_CursorAtPreviousLineEnd = cursorAtPreviousLineEnd;
				_CursorPos = newCurPos;
				_CursorPos -= (sint32)_Prompt.length();
				_CursorPos = std::max(_CursorPos, sint32(0));
				_SelectCursorPos = _CursorPos;
				_CurrSelection = NULL;

				return true;
			}
			// if click, and not frozen, then get the focus
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mousemove && !_Frozen && _SelectingText)
			{
				// set the right cursor position
				uint newCurPos;
				bool cursorAtPreviousLineEnd;
				_CurrSelection = this;
				_ViewText->getCharacterIndexFromPosition(eventDesc.getX() - _ViewText->getXReal(), eventDesc.getY() - _ViewText->getYReal(), newCurPos, cursorAtPreviousLineEnd);
				_SelectCursorPos = newCurPos;
				_SelectCursorPos -= (sint32)_Prompt.length();
				_SelectCursorPos = std::max(_SelectCursorPos, sint32(0));
				return true;
			}

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
			{
				CWidgetManager::getInstance()->setCapturePointerRight(this);
				return true;
			}
		}
		else
		{
			//////////////////
			// SYSTEM EVENT //
			//////////////////
			const NLGUI::CEventDescriptorSystem &eventDesc = (const NLGUI::CEventDescriptorSystem &)event;
			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::activecalledonparent)
			{
				NLGUI::CEventDescriptorActiveCalledOnParent &activeEvent = (NLGUI::CEventDescriptorActiveCalledOnParent &) eventDesc;
				if (activeEvent.getActive() == false && _ResetFocusOnHide)
				{
					CWidgetManager::getInstance()->resetCaptureKeyboard();
					// If a selection was shown, reset it
					if (_CurrSelection == this) _CurrSelection = NULL;
				}
				CInterfaceGroup::handleEvent(activeEvent);
			}

		}
		return false;
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::setupDisplayText()
	{
		if (_ViewText)
		{
			ucstring usTmp;
			if (_EntryType == Password)
			{
				usTmp = _Prompt;
				for (uint32 i = 0; i < _InputString.size(); ++i)
					usTmp += "*";
			}
			else
			{
				usTmp = _Prompt + _InputString;
			}
			_ViewText->setText (usTmp);
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::checkCoords()
	{
		if( !editorMode )
			setupDisplayText();

		CInterfaceGroup::checkCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::updateCoords()
	{
		if (!_Setupped)
		{
			setup();
			_Setupped = true;
		}
		CInterfaceGroup::updateCoords();

		if (_ViewText)
		{
			bool bRecalc = false;

			// if the length of the view text exceed the max w we allow, cut it
			while (_ViewText->getWReal() > _MaxCharsSize)
			{
				// Suppr last char
				_InputString = _InputString.substr(0, _InputString.size()-1);

				setupDisplayText();

				_ViewText->updateCoords();
				bRecalc = true;
			}

			// if the length of the view text exceed our real size, ensure the Cursor position is at least visible
			sint32 viewTextNewX = _ViewText->getX();
			if (_ViewText->getWReal() > _WReal)
			{
				// Check if cursor visible
				sint xCursVT, xCurs, yTmp, hTmp;
				// Get the cursor pos from the BL of the viewtext
				_ViewText->getCharacterPositionFromIndex(_CursorPos+(sint)_Prompt.size(), false, xCursVT, yTmp, hTmp);
				// Get the cursor pos from the BL of the edit box
				xCurs = xCursVT - (_XReal - _ViewText->getXReal());
				// If the cursor is outside the edit box move the view text to show the cursor
				if (xCurs > _WReal)
				{
					viewTextNewX = _ViewTextDeltaX - (xCursVT - _WReal);
				}
				if (xCurs < 0)
				{
					if ((xCursVT + xCurs) < 0)
						viewTextNewX = _ViewTextDeltaX;
					else
						viewTextNewX = _ViewTextDeltaX - (xCursVT + xCurs);
				}
				if (_CursorPos == 0)
				{
					viewTextNewX = _ViewTextDeltaX;
				}
			}
			else
			{
				viewTextNewX = _ViewTextDeltaX;
			}

			// if X position changed, must recompute
			if (viewTextNewX != _ViewText->getX())
			{
				_ViewText->setX(viewTextNewX);
				bRecalc = true;
			}

			// recompute position
			if (bRecalc)
				CInterfaceGroup::updateCoords();
		}

		if (_BackupFatherContainerPos)
		{
			CGroupContainerBase *gc = static_cast< CGroupContainerBase* >( getEnclosingContainer() );
		
			if (gc && !gc->getTouchFlag(true))
			{

				if (_ViewText && _ViewText->getNumLine() <= 1)
				{
					if (_PrevNumLine > 1)
					{
						gc->restorePosition();
						CInterfaceGroup::updateCoords();
					}
				}
				else
				{
					if (_PrevNumLine <= 1)
					{
						gc->backupPosition();
					}
				}
				_PrevNumLine = _ViewText->getNumLine();
			}
			else
			{
				gc->backupPosition();
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::clearViews()
	{
		CInterfaceGroup::clearViews();
	}

	// ----------------------------------------------------------------------------
	void CGroupEditBox::setup()
	{
		// bind to the controls
		_ViewText = dynamic_cast<CViewText *>(CInterfaceGroup::getView("edit_text"));

		if(_ViewText == NULL)
		{
			nlwarning("Interface: CGroupEditBox: text 'edit_text' missing or bad type");
			if( editorMode )
			{
				nlwarning( "Trying to create a new 'edit_text' for %s", getId().c_str() );
				_ViewText = dynamic_cast< CViewText* >( CInterfaceFactory::createClass( "text" ) );
				if( _ViewText != NULL )
				{
					_ViewText->setParent( this );
					_ViewText->setIdRecurse( "edit_text" );
					_ViewText->setHardText( "" );
					_ViewText->setPosRef( Hotspot_TL );
					_ViewText->setParentPosRef( Hotspot_TL );
					addView( _ViewText );

					sint32 w,h;
					w = std::max( sint32( _ViewText->getFontWidth() * _ViewText->getText().size() ), getW() );
					h = std::max( sint32(  _ViewText->getFontHeight() ), getH() );
					
					setH( h );
					setW( w );
					
				}
				else
					nlwarning( "Failed to create new 'edit_text' for %s", getId().c_str() );
			}
		}

		_ViewText->setEditorSelectable( false );

		// For MultiLine editbox, clip the end space, else weird when edit space at end of line (nothing happens)
		if(_ViewText)
			_ViewText->setMultiLineClipEndSpace(true);

		// Bakcup the delta X of this view text
		if(_ViewText)
			_ViewTextDeltaX= _ViewText->getX();
		else
			_ViewTextDeltaX= 0;

		// read options
		CInterfaceOptions *pIO = CWidgetManager::getInstance()->getOptions("text_selection");
		if (pIO != NULL)
		{
			_BackSelectColor= pIO->getValColor("back_select_color");
			_TextSelectColor= pIO->getValColor("text_select_color");
		}
	}


	// ----------------------------------------------------------------------------
	void CGroupEditBox::setInputString(const ucstring &str)
	{
		_InputString = str;
		if (_CursorPos > (sint32) str.length())
		{
			_CursorPos = (sint32)str.length();
			_CursorAtPreviousLineEnd = false;
		}
		if (!_ViewText) return;
		setupDisplayText();

		invalidateCoords();
	}


	// ***************************************************************************
	void	CGroupEditBox::setDefaultInputString(const ucstring &str)
	{
		_DefaultInputString= true;
		setInputString(str);
	}


	// ***************************************************************************
	sint32		CGroupEditBox::getInputStringAsInt() const
	{
		sint32 value;
		fromString(_InputString.toString(), value);
		return value;
	}

	// ***************************************************************************
	void		CGroupEditBox::setInputStringAsInt(sint32 val)
	{
		setInputString(NLMISC::toString(val));
	}

	// ***************************************************************************
	sint64		CGroupEditBox::getInputStringAsInt64() const
	{
		sint64 value;
		fromString(_InputString.toString(), value);
		return value;
	}

	// ***************************************************************************
	void		CGroupEditBox::setInputStringAsInt64(sint64 val)
	{
		setInputString(NLMISC::toString(val));
	}

	// ***************************************************************************
	float		CGroupEditBox::getInputStringAsFloat() const
	{
		float value;
		fromString(_InputString.toString(), value);
		return value;
	}

	// ***************************************************************************
	void		CGroupEditBox::setInputStringAsFloat(float val)
	{
		string	fmt= "%." + NLMISC::toString(_MaxFloatPrec) + "f";
		setInputString(NLMISC::toString(fmt.c_str(), val));
	}

	// ***************************************************************************
	void CGroupEditBox::cutSelection()
	{
		sint32	minPos= min(_CursorPos, _SelectCursorPos);
		sint32	maxPos= max(_CursorPos, _SelectCursorPos);
		// cut the selection
		if(!_InputString.empty())
		{
			_InputString= _InputString.substr(0, minPos) + _InputString.substr(maxPos);
		}
		_CurrSelection = NULL;
		_CursorPos= minPos;
		triggerOnChangeAH();
	}

	// ***************************************************************************
	ucstring	CGroupEditBox::getSelection()
	{
		sint32	minPos= min(_CursorPos, _SelectCursorPos);
		sint32	maxPos= max(_CursorPos, _SelectCursorPos);
		// get the selection
		return _InputString.substr(minPos, maxPos-minPos);
	}



	// ***************************************************************************
	void		CGroupEditBox::setSelectionAll()
	{
		if(!_InputString.empty())
		{
			_CurrSelection = this;
			_SelectCursorPos= 0;
			_CursorPos= (sint32)_InputString.size();
			_CursorAtPreviousLineEnd = false;
		}
	}

	// ***************************************************************************
	void CGroupEditBox::setActive(bool active)
	{
		if (!active && _ResetFocusOnHide)
			CWidgetManager::getInstance()->resetCaptureKeyboard();

		CInterfaceGroup::setActive(active);
	}

	// ***************************************************************************
	void CGroupEditBox::setInputStringAsStdString(const std::string &str)
	{
		setInputString(ucstring(str));
	}

	// ***************************************************************************
	std::string CGroupEditBox::getInputStringAsStdString() const
	{
		std::string result;
		_InputString.toString(result);
		return result;
	}

	// ***************************************************************************
	void	CGroupEditBox::setInputStringAsUtf8(const std::string &str)
	{
		ucstring	tmp;
		tmp.fromUtf8(str);
		setInputString(tmp);
	}

	// ***************************************************************************
	std::string	CGroupEditBox::getInputStringAsUtf8() const
	{
		return _InputString.toUtf8();
	}

	// ***************************************************************************
	void CGroupEditBox::setCommand(const ucstring &command, bool execute)
	{
		// do nothing if frozen
		if(_Frozen)
			return;

		// set the string and maybe execute
		setInputString((ucchar) '/' + command);
		if (execute)
		{
			// stop selection
			_CurrSelection = NULL;
			_CursorAtPreviousLineEnd = false;
			CAHManager::getInstance()->runActionHandler(_AHOnEnter, this, _AHOnEnterParams);
		}
		else
		{
			CWidgetManager::getInstance()->setCaptureKeyboard (this);
			_CursorPos = (sint32)_InputString.length();
		}
	}

	// ***************************************************************************
	void CGroupEditBox::makeTopWindow()
	{
		CInterfaceGroup *root = getRootWindow();
		if(root)
			CWidgetManager::getInstance()->setTopWindow(root);
	}

	// ***************************************************************************
	void CGroupEditBox::clearAllEditBox()
	{
		_InputString = "";
		_CursorPos = 0;
		_CursorAtPreviousLineEnd = false;
		if (!_ViewText) return;
		setupDisplayText();
		invalidateCoords();
	}

	// ***************************************************************************
	sint32	CGroupEditBox::getMaxUsedW() const
	{
		return _W;
	}

	// ***************************************************************************
	sint32	CGroupEditBox::getMinUsedW() const
	{
		return _W;
	}

	// ***************************************************************************
	bool CGroupEditBox::wantSerialConfig() const
	{
		return _Savable && !_InputString.empty();
	}

	// ***************************************************************************
	void CGroupEditBox::serialConfig(NLMISC::IStream &f)
	{
		f.serialVersion(0);
		if(_DefaultInputString)		// Don't want to save the default input
		{
			_DefaultInputString= false;
			_InputString.clear();
		}
		f.serial(_InputString);
		f.serial(_CursorPos);
		f.serial(_PrevNumLine);
		if (f.isReading())
		{
			setInputString(_InputString);

		}
		// serial selection
		bool isSelected = (_CurrSelection == this);
		f.serial(isSelected);
		if (isSelected)
		{
			_CurrSelection = this;
			f.serial(_SelectCursorPos);
		}
	}

	// ***************************************************************************
	void CGroupEditBox::onQuit()
	{
		// clear the text and restore backup pos before final save
		setInputString(ucstring(""));
		_CurrSelection = NULL;
	}

	// ***************************************************************************
	void CGroupEditBox::onLoadConfig()
	{
		// config is not saved when there's an empty string, so restore that default state.
		setInputString(ucstring(""));
		_CurrSelection = NULL;
		_PrevNumLine = 1;
	}


	// ***************************************************************************
	void CGroupEditBox::elementCaptured(CCtrlBase *capturedElement)
	{
		// If the input string is the default one, then reset it
		if(capturedElement == this)
		{
			if (_DefaultInputString)
			{
				_DefaultInputString= false;
				setInputString(ucstring());
			}
			_CanRedo = false;
			_CanUndo = false;
			_StartInputString = _ModifiedInputString = _InputString;
		}
		CInterfaceGroup::elementCaptured(capturedElement);
	}

	// ***************************************************************************
	void CGroupEditBox::onKeyboardCaptureLost()
	{
		if (!_AHOnFocusLost.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnFocusLost, this, _AHOnFocusLostParams);

	}

	// ***************************************************************************
	int CGroupEditBox::luaSetSelectionAll(CLuaState &ls)
	{
		const char *funcName = "setSelectionAll";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		setSelectionAll();
		return 0;
	}

	// ***************************************************************************
	void CGroupEditBox::setFocusOnText()
	{
		// do nothing if frozen
		if(_Frozen)
			return;

		// else set the focus
		CWidgetManager::getInstance()->setCaptureKeyboard (this);

		_CurrSelection = this;
		_SelectCursorPos= (sint32)_InputString.size();
		_CursorPos= (sint32)_InputString.size();
		_CursorAtPreviousLineEnd = false;
	}

	// ***************************************************************************
	int CGroupEditBox::luaSetFocusOnText(CLuaState &ls)
	{
		const char *funcName = "setFocusOnText";
		CLuaIHM::checkArgCount(ls, funcName, 0);

		setFocusOnText();

		return 0;
	}

	// ***************************************************************************
	int CGroupEditBox::luaCancelFocusOnText(CLuaState &ls)
	{
		const char *funcName = "cancelFocusOnText";
		CLuaIHM::checkArgCount(ls, funcName, 0);

		if (CWidgetManager::getInstance()->getCaptureKeyboard()==this || CWidgetManager::getInstance()->getOldCaptureKeyboard()==this)
			CWidgetManager::getInstance()->resetCaptureKeyboard();

		_CurrSelection = NULL;
		_SelectCursorPos= 0;
		_CursorPos= 0;
		_CursorAtPreviousLineEnd = false;

		return 0;
	}

	// ***************************************************************************
	int CGroupEditBox::luaSetupDisplayText(CLuaState &ls)
	{
		const char *funcName = "setupDisplayText";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		setupDisplayText();
		return 0;
	}

	// ***************************************************************************
	void CGroupEditBox::setColor(NLMISC::CRGBA col)
	{
		if (_ViewText)
			_ViewText->setColor(col);
	}

	// ***************************************************************************
	void CGroupEditBox::setFrozen (bool state)
	{
		_Frozen= state;

		// if frozen, loose the focus
		if(_Frozen)
		{
			// stop capture and selection
			CWidgetManager::getInstance()->setCaptureKeyboard (NULL);
			if(_CurrSelection==this)	_CurrSelection = NULL;
			// do not allow to recover focus
			if (CWidgetManager::getInstance()->getOldCaptureKeyboard() == this)
			{
				CWidgetManager::getInstance()->resetCaptureKeyboard();
			}
		}
	}
}

