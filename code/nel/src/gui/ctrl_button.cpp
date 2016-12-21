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
#include "nel/gui/ctrl_button.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_group.h"

// ----------------------------------------------------------------------------
using namespace std;
using namespace NLMISC;
using namespace NL3D;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlButton, std::string, "button");

namespace NLGUI
{

	void CCtrlButton::setAlignFromString( const std::string &s )
	{
		_Align = 0;

		std::string::size_type i;
		for( i = 0; i < s.size(); i++ )
		{
			char c = toLower( s[ i ] );

			switch( c )
			{
				case 'l':
					_Align &= ~1;
					break;

				case 'r':
					_Align |= 1;
					break;

				case 'b':
					_Align &= ~2;
					break;

				case 't':
					_Align |= 2;
					break;
			}
		}
	}

	std::string CCtrlButton::getProperty( const std::string &name ) const
	{
		if( name == "tx_normal" )
		{
			return CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdNormal );
		}
		else
		if( name == "tx_pushed" )
		{
			return CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdPushed );
		}
		else
		if( name == "tx_over" )
		{
			return CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdOver );
		}
		else
		if( name == "scale" )
		{
			return toString( _Scale );
		}
		else
		if( name == "align" )
		{
			std::string align;
			if( ( _Align & 1 ) != 0 )
				align = "r";
			else
				align = "l";
			if( ( _Align & 2 ) != 0 )
				align += "t";
			else
				align += "b";
			return align;
		}
		else
			return CCtrlBaseButton::getProperty( name );
	}

	void CCtrlButton::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "tx_normal" )
		{
			std::string s = CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdNormal );
			if( !_TextureIdNormal.setTexture( value.c_str() ) )
			{
				_TextureIdNormal.setTexture( s.c_str() );
			}
			return;
		}
		else
		if( name == "tx_pushed" )
		{
			std::string s = CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdPushed );
			if( !_TextureIdPushed.setTexture( value.c_str() ) )
			{
				_TextureIdPushed.setTexture( s.c_str() );
			}
			return;
		}
		else
		if( name == "tx_over" )
		{
			std::string s = CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdOver );
			if( !_TextureIdOver.setTexture( value.c_str() ) )
			{
				_TextureIdOver.setTexture( s.c_str() );
			}
			return;
		}
		else
		if( name == "scale" )
		{
			bool b;
			if( fromString( value, b ) )
				_Scale = b;
			return;
		}
		else
		if( name == "align" )
		{
			setAlignFromString( value );
			return;
		}
		else
			CCtrlBaseButton::setProperty( name, value );
	}


	xmlNodePtr CCtrlButton::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBaseButton::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "button" );

		xmlNewProp( node, BAD_CAST "tx_normal",
			BAD_CAST CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdNormal ).c_str() );

		xmlNewProp( node, BAD_CAST "tx_pushed",
			BAD_CAST CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdPushed ).c_str() );

		xmlNewProp( node, BAD_CAST "tx_over",
			BAD_CAST CViewRenderer::getInstance()->getTextureNameFromId( _TextureIdOver ).c_str() );

		xmlNewProp( node, BAD_CAST "scale", BAD_CAST toString( _Scale ).c_str() );

		std::string align;
		if( ( _Align & 1 ) != 0 )
			align = "r";
		else
			align = "l";
		if( ( _Align & 2 ) != 0 )
			align += "t";
		else
			align += "b";
		
		xmlNewProp( node, BAD_CAST "align", BAD_CAST align.c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CCtrlButton::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		CXMLAutoPtr prop;
		//try to get props that can be inherited from groups
		//if a property is not defined, try to find it in the parent group.
		//if it is undefined, set it to zero
		if (! CCtrlBaseButton::parse(cur,parentGroup) )
		{
			string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
			nlinfo(tmp.c_str());
			return false;
		}


		// *** Read Textures
		prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_normal" );
		if (prop)
		{
			string TxName = NLMISC::toLower((const char *) prop);
			_TextureIdNormal.setTexture(TxName.c_str());
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_pushed" );
		if (prop)
		{
			string TxName = NLMISC::toLower((const char *) prop);
			_TextureIdPushed.setTexture(TxName.c_str());
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_over" );
		if (prop)
		{
			string TxName = NLMISC::toLower((const char *) prop);
			_TextureIdOver.setTexture(TxName.c_str());
		}

		// *** Misc.
		prop = (char*) xmlGetProp( cur, (xmlChar*)"scale" );
		_Scale = false;
		if (prop)
			_Scale = convertBool(prop);


		prop = (char*) xmlGetProp (cur, (xmlChar*)"align");
		if (prop)
		{
			setAlignFromString( std::string( (const char*)prop ) );
		}


		return true;
	}

	// ----------------------------------------------------------------------------
	void CCtrlButton::draw ()
	{
		sint32 nTxId = -1;
		CRGBA  color;

		CViewRenderer &rVR = *CViewRenderer::getInstance();
		CRGBA  globalColor= CWidgetManager::getInstance()->getGlobalColorForContent();

		// *** Detect Over
		bool	lastOver = false;

		updateOver(lastOver);


		sint32 x = _XReal;
		sint32 y = _YReal;
		sint32 txw, txh;
		// the pointer is over the button
		if (_Scale)
		{
			x = _XReal;
			y = _YReal;
			txw = _WReal;
			txh = _HReal;
		}
		else
		{
			x = _XReal;
			y = _YReal;
		}

		switch(_Type)
		{
			case ToggleButton:
			{
				if (_Pushed && !editorMode )
				{
					nTxId = _TextureIdPushed;
					color  = getCurrentColorPushed(globalColor);
				}
				else
				{
					nTxId = _TextureIdNormal;
					color  = getCurrentColorNormal(globalColor);
				}
			}
			break;
			case RadioButton:
			{
	//			CViewPointer &rIP = *CInterfaceManager::getInstance()->getPointer();
				// Init the radio button
				initRBRef();

				if (*_RBRef == this)
				{
					// if it is equal to the ref value, then the button must appear pushed
					nTxId = _TextureIdPushed;
					color  = getCurrentColorPushed(globalColor);
				}
				else
				{
					if ( (_Over) && (CWidgetManager::getInstance()->getCapturePointerLeft() == this) && !editorMode )
					{
						nTxId = _TextureIdPushed;
						color  = getCurrentColorPushed(globalColor);
					}
					else
					{
						nTxId = _TextureIdNormal;
						color     = getCurrentColorNormal(globalColor);
						_Pushed = false;
					}
				}
			}
			break;
			case PushButton:
			{
				if ( _Over && (CWidgetManager::getInstance()->getCapturePointerLeft() == this) && !editorMode )
				{
					nTxId = _TextureIdPushed;
					color  = getCurrentColorPushed(globalColor);
				}
				else
				{
					nTxId = _TextureIdNormal;
					color     = getCurrentColorNormal(globalColor);
					_Pushed = false;
				}
			}
			break;
			default:
			break;
		}

		color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

		// Fromzen ?
		if (getFrozen() && getFrozenHalfTone())
			color.A >>= 2;

		if (!_Scale)
		{
			CViewRenderer::getInstance()->getTextureSizeFromId (nTxId, txw, txh);
			if (_Align&1)
				x = x + _WReal - txw;
			if (_Align&2)
				y = y + _HReal - txh;
		}
		rVR.drawRotFlipBitmap (	_RenderLayer, x, y, txw, txh,
								0, false,
								nTxId,
								color );

		if ((_OverWhenPushed == false) && (_Pushed == true || (CWidgetManager::getInstance()->getCapturePointerLeft() == this)))
			return;



		if ( ( _Over && !editorMode ) )
		{

			if( !editorMode && (lastOver == false) && (_AHOnOver != NULL))
				CAHManager::getInstance()->runActionHandler (_AHOnOver, this, _AHOverParams);

			// the pointer is over the button
			color= getCurrentColorOver(globalColor);
			color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

			// Frozen ?
			if (getFrozen())
				color.A >>= 2;

			// draw the over. force upper layer to avoid problem with DXTC/tga
			rVR.drawRotFlipBitmap (	_RenderLayer+1, x, y, txw, txh,
									0, false,
									_TextureIdOver,
									color );
		}
	}


	// ----------------------------------------------------------------------------
	void CCtrlButton::updateCoords()
	{
		if (!_Scale)
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			sint32 txw, txh;
			rVR.getTextureSizeFromId (_TextureIdNormal, txw, txh);
			_W = txw;
			_H = txh;
		}
		CViewBase::updateCoords();
	}

	// ----------------------------------------------------------------------------
	void CCtrlButton::setTexture(const std::string&name)
	{
	//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	//	CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextureIdNormal.setTexture(name.c_str (), 0, 0, -1, -1, false);
	}

	// ----------------------------------------------------------------------------
	void CCtrlButton::setTexturePushed(const std::string&name)
	{
	//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	//	CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextureIdPushed.setTexture(name.c_str (), 0, 0, -1, -1, false);
	}

	// ----------------------------------------------------------------------------
	void CCtrlButton::setTextureOver(const std::string&name)
	{
	//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	//	CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextureIdOver.setTexture(name.c_str (), 0, 0, -1, -1, false);
	}

	// ----------------------------------------------------------------------------
	std::string CCtrlButton::getTexture() const
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		return rVR.getTextureNameFromId(_TextureIdNormal);
	}

	// ----------------------------------------------------------------------------
	std::string CCtrlButton::getTexturePushed() const
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		return rVR.getTextureNameFromId(_TextureIdPushed);
	}

	// ----------------------------------------------------------------------------
	std::string CCtrlButton::getTextureOver() const
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		return rVR.getTextureNameFromId(_TextureIdOver);
	}

	// ***************************************************************************
	sint32	CCtrlButton::getMaxUsedW() const
	{
		if (_Scale)
			return _WReal;

		sint32 txw, txh;
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.getTextureSizeFromId (_TextureIdNormal, txw, txh);
		return txw;
	}

	// ***************************************************************************
	sint32	CCtrlButton::getMinUsedW() const
	{
		return getMaxUsedW();
	}

	// ***************************************************************************
	void CCtrlButton::fitTexture()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 w, h;
		rVR.getTextureSizeFromId(_TextureIdNormal, w, h);
		setW(w);
		setH(h);
	}

	// ***************************************************************************
	bool CCtrlButton::getMouseOverShape(string &texName, uint8 &rot, CRGBA &col)
	{
		if (_AHOnLeftClickString == "browse")
		{
			if (!_AHOnLeftClickStringParams.empty())
			{
				texName = "@curs_pick.tga@"+_AHOnLeftClickStringParams;
			}
			else
			{
				texName = "curs_pick.tga";
			}
			rot= 0;
			col = CRGBA::White;
			return true;
		}
		
		return false;
	}

}

