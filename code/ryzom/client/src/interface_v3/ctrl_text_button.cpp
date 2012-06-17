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

#include "ctrl_text_button.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/view_text.h"
#include "view_text_id.h"
#include "group_container.h"
#include "nel/gui/lua_ihm.h"
#include "lua_ihm_ryzom.h"


// ***************************************************************************
using namespace std;
using namespace NLMISC;
using namespace NL3D;


NLMISC_REGISTER_OBJECT(CViewBase, CCtrlTextButton, std::string, "text_button");

// ***************************************************************************
CCtrlTextButton::CCtrlTextButton(const TCtorParam &param)
:	CCtrlBaseButton(param)
{
	_BmpLeftW= _BmpMiddleW= _BmpRightW= _BmpH= 0;
	_WMargin= 0;
	_WMin= 0;
	_TextX= -2;
	_TextY= -2;
	_Setuped= false;
	_ViewText= NULL;
	_IsViewTextId= false;
	_TextColorNormal= CRGBA(255, 255, 255, 255);
	_TextColorPushed= CRGBA(255, 255, 255, 255);
	_TextColorOver= CRGBA(255, 255, 255, 255);
	_TextShadowColorNormal= CRGBA(0, 0, 0, 255);
	_TextShadowColorPushed= CRGBA(0, 0, 0, 255);
	_TextShadowColorOver= CRGBA(0, 0, 0, 255);
	_TextModulateGlobalColorNormal= true;
	_TextModulateGlobalColorPushed= true;
	_TextModulateGlobalColorOver= true;
	_TextHeaderColor= false;
	_TextPosRef = Hotspot_MM;
	_TextParentPosRef = Hotspot_MM;
	_ForceTextOver = false;
}

// ***************************************************************************
bool CCtrlTextButton::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();

	//try to get props that can be inherited from groups
	//if a property is not defined, try to find it in the parent group.
	//if it is undefined, set it to zero
	if (! CCtrlBaseButton::parse(cur,parentGroup) )
	{
		string tmp = "cannot parse view:"+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		return false;
	}


	// *** Read Textures.
	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_normal" );
	if (prop)
	{
		string TxName = toLower(std::string((const char *) prop));
		_TextureIdNormal[0].setTexture((TxName+"_l.tga").c_str());
		_TextureIdNormal[1].setTexture((TxName+"_m.tga").c_str());
		_TextureIdNormal[2].setTexture((TxName+"_r.tga").c_str());
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_pushed" );
	if (prop)
	{
		string TxName = toLower(std::string((const char *) prop));
		_TextureIdPushed[0].setTexture((TxName+"_l.tga").c_str());
		_TextureIdPushed[1].setTexture((TxName+"_m.tga").c_str());
		_TextureIdPushed[2].setTexture((TxName+"_r.tga").c_str());
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"tx_over" );
	if (prop)
	{
		string TxName = toLower(std::string((const char *) prop));
		_TextureIdOver[0].setTexture((TxName+"_l.tga").c_str());
		_TextureIdOver[1].setTexture((TxName+"_m.tga").c_str());
		_TextureIdOver[2].setTexture((TxName+"_r.tga").c_str());
	}

	// Compute Bmp Sizes
	nlctassert(NumTexture==3);
	rVR.getTextureSizeFromId(_TextureIdNormal[0], _BmpLeftW, _BmpH);
	rVR.getTextureSizeFromId(_TextureIdNormal[1], _BmpMiddleW, _BmpH);
	rVR.getTextureSizeFromId(_TextureIdNormal[2], _BmpRightW, _BmpH);


	// *** Create the ViewText for draw, and set text
	// ViewTextId or standard view?
	CXMLAutoPtr ptr((const char*)xmlGetProp(cur,(xmlChar*)"textid"));
	_IsViewTextId= (bool)ptr;
	if(_IsViewTextId)
		_ViewText= new CViewTextID(CViewBase::TCtorParam());
	else
		_ViewText= new CViewText(CViewBase::TCtorParam());
	_ViewText->setId(_Id+"_text");
	_ViewText->parseTextOptions(cur);
	_ViewText->setAvoidResizeParent(avoidResizeParent());
	if(_IsViewTextId)
		((CViewTextID*)_ViewText)->parseTextIdOptions(cur);
	// Same RenderLayer as us.
	_ViewText->setRenderLayer(getRenderLayer());
	// Parse the hardText (if not text id)
	if(!_IsViewTextId)
	{
		prop = (char*) xmlGetProp( cur, (xmlChar*)"hardtext" );
		if (prop)
		{
			const char *propPtr = prop;
			ucstring text = ucstring(propPtr);
			if ((strlen(propPtr)>2) && (propPtr[0] == 'u') && (propPtr[1] == 'i'))
				text = CI18N::get (propPtr);
			_ViewText->setText(text);
		}
	}

	// wmargin
	_WMargin= 0;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"wmargin" );
	if (prop)
	{
		fromString((const char *) prop, _WMargin);
	}

	// wmin
	_WMin= 0;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"wmin" );
	if (prop)
	{
		fromString((const char *) prop, _WMin);
	}
	// _WMin is at least the size of All W Bitmaps
	_WMin= max(_WMin, _BmpLeftW + _BmpMiddleW + _BmpRightW);

	// TextY
	_TextY= -2;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_y" );
	if (prop)
	{
		fromString((const char *) prop, _TextY);
	}
	_TextX = 0;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_x" );
	if (prop)
	{
		fromString((const char *) prop, _TextX);
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_underlined" );
	if (prop)
	{
		_ViewText->setUnderlined(convertBool(prop));
	}

	prop = (char*) xmlGetProp( cur, (xmlChar*) "text_posref" );
	_TextParentPosRef = Hotspot_MM;
	_TextPosRef = Hotspot_MM;
	if (prop)
	{
		char *seekPtr = prop.getDatas();
		seekPtr = strtok(seekPtr," \t");
		if (seekPtr == NULL)
		{
			// mean that there s a bad formated posref (missing space or tab)
			nlwarning("bad 'text_pos_ref' formatting");
		}
		else
		{
			_TextParentPosRef = convertHotSpot (seekPtr);
			seekPtr = strtok (seekPtr+1+strlen(seekPtr)," \t");
			_TextPosRef = convertHotSpot (seekPtr);
		}
	}


	// *** Read Text Colors
	// get color normal
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_color_normal" );
	_TextColorNormal = CRGBA(255,255,255,255);
	if (prop)
		_TextColorNormal = convertColor (prop);

	// Get ColorPushed
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_color_pushed" );
	_TextColorPushed = CRGBA(255,255,255,255);
	if (prop)
		_TextColorPushed = convertColor(prop);

	// Get ColorOver
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_color_over" );
	_TextColorOver = CRGBA(255,255,255,255);
	if (prop)
		_TextColorOver = convertColor(prop);


	// *** Read Text Shadow Colors
	// get color normal
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_shadow_color_normal" );
	_TextShadowColorNormal = CRGBA(0,0,0,255);
	if (prop)
		_TextShadowColorNormal = convertColor (prop);

	// Get ColorPushed
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_shadow_color_pushed" );
	_TextShadowColorPushed = CRGBA(0,0,0,255);
	if (prop)
		_TextShadowColorPushed = convertColor(prop);

	// Get ColorOver
	prop= (char*) xmlGetProp( cur, (xmlChar*)"text_shadow_color_over" );
	_TextShadowColorOver = CRGBA(0,0,0,255);
	if (prop)
		_TextShadowColorOver = convertColor(prop);

	// *** Read Text Global Color
	// Default: take "global_color" param interface_element option.
	_TextModulateGlobalColorNormal= _TextModulateGlobalColorPushed= _TextModulateGlobalColorOver= getModulateGlobalColor();

	// Read special text global_color for each state
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_global_color_normal" );
	if (prop)	_TextModulateGlobalColorNormal= convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_global_color_pushed" );
	if (prop)	_TextModulateGlobalColorPushed= convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_global_color_over" );
	if (prop)	_TextModulateGlobalColorOver= convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"force_text_over" );
	if (prop)	_ForceTextOver= convertBool(prop);

	// read Text header color
	prop = (char*) xmlGetProp( cur, (xmlChar*)"text_header_color" );
	if (prop)	_TextHeaderColor= convertBool(prop);

	return true;
}

// ***************************************************************************
void CCtrlTextButton::draw ()
{
	CViewRenderer::CTextureId *pTxId = NULL;
	CRGBA  color;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	CRGBA	globalColor= CWidgetManager::getInstance()->getGlobalColorForContent();

	// *** Detect Over
	bool	lastOver = false;
	updateOver(lastOver);

	// *** Choose Button State
	switch(_Type)
	{
		case ToggleButton:
		{
			if (_Pushed)
			{
				pTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				pTxId = _TextureIdNormal;
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
				pTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				if ((_Over) && (CWidgetManager::getInstance()->getCapturePointerLeft() == this))
				{
					pTxId = _TextureIdPushed;
					color  = getCurrentColorPushed(globalColor);
				}
				else
				{
					pTxId = _TextureIdNormal;
					color     = getCurrentColorNormal(globalColor);
					_Pushed = false;
				}
			}
		}
		break;
		case PushButton:
		{
			if (_Over && (CWidgetManager::getInstance()->getCapturePointerLeft() == this))
			{
				pTxId = _TextureIdPushed;
				color  = getCurrentColorPushed(globalColor);
			}
			else
			{
				pTxId = _TextureIdNormal;
				color     = getCurrentColorNormal(globalColor);
				_Pushed = false;
			}
		}
		break;
		default:
		break;
	}

	// *** Draw
	color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

	// Fromzen ?
	if (getFrozen() && getFrozenHalfTone())
		color.A >>= 2;

	sint32	x= _XReal;
	sint32	y= _YReal;
	sint32	txw, txh;
	txw = _WReal - _BmpLeftW - _BmpRightW;
	txh = _HReal;
	nlctassert(NumTexture==3);
	rVR.drawRotFlipBitmap (	_RenderLayer, x, y, _BmpLeftW, txh, 0, false, pTxId[0], color );
	rVR.drawRotFlipBitmap (	_RenderLayer, x+_BmpLeftW, y, txw, txh, 0, false, pTxId[1], color );
	rVR.drawRotFlipBitmap (	_RenderLayer, x+_BmpLeftW+txw, y, _BmpRightW, txh, 0, false, pTxId[2], color );

	// *** Draw Over
	if (_Over && (_OverWhenPushed || !(_Pushed || CWidgetManager::getInstance()->getCapturePointerLeft() == this)))
	{
		if ((lastOver == false) && (_AHOnOver != NULL))
			CAHManager::getInstance()->runActionHandler (_AHOnOver, this, _AHOverParams);

		// the pointer is over the button.
		color= getCurrentColorOver(globalColor);
		color.A = (uint8)(((sint32)color.A*((sint32)globalColor.A+1))>>8);

		// Fromzen ?
		if (getFrozen() && getFrozenHalfTone())
			color.A >>= 2;

		nlctassert(NumTexture==3);
		pTxId= _TextureIdOver;
		uint layerOffset = _ForceTextOver ? 0 : 1; // Must write Over On Top of the text ?

		rVR.drawRotFlipBitmap (	_RenderLayer+layerOffset, x, y, _BmpLeftW, txh, 0, false, pTxId[0], color );
		rVR.drawRotFlipBitmap (	_RenderLayer+layerOffset, x+_BmpLeftW, y, txw, txh, 0, false, pTxId[1], color );
		rVR.drawRotFlipBitmap (	_RenderLayer+layerOffset, x+_BmpLeftW+txw, y, _BmpRightW, txh, 0, false, pTxId[2], color );
	}

	// *** Setup ViewText Color according to selected one (should be drawn after because of eltorder)
	// update header color?
	CRGBA	viewTextColor;
	if(_TextHeaderColor)
	{
		CInterfaceGroup		*pIG= getRootWindow();
		if(pIG->isGroupContainer())
		{
			CGroupContainer		*pGC= static_cast<CGroupContainer*>(pIG);
			viewTextColor= pGC->getDrawnHeaderColor();
		}
	}
	// Setup ViewText color
	if ( pTxId==_TextureIdNormal )
	{
		if(_TextHeaderColor)	viewTextColor.A=  _TextColorNormal.A;
		else					viewTextColor= _TextColorNormal;
		_ViewText->setColor(viewTextColor);
		_ViewText->setShadowColor(_TextShadowColorNormal);
		_ViewText->setModulateGlobalColor(_TextModulateGlobalColorNormal);
	}
	else if ( pTxId==_TextureIdPushed )
	{
		if(_TextHeaderColor)	viewTextColor.A=  _TextColorPushed.A;
		else					viewTextColor= _TextColorPushed;
		_ViewText->setColor(viewTextColor);
		_ViewText->setShadowColor(_TextShadowColorPushed);
		_ViewText->setModulateGlobalColor(_TextModulateGlobalColorPushed);
	}
	else if ( pTxId==_TextureIdOver )
	{
		if(_TextHeaderColor)	viewTextColor.A=  _TextColorOver.A;
		else					viewTextColor= _TextColorOver;
		_ViewText->setColor(viewTextColor);
		_ViewText->setShadowColor(_TextShadowColorOver);
		_ViewText->setModulateGlobalColor(_TextModulateGlobalColorOver);
	}
	if(getFrozen() && getFrozenHalfTone())
		_ViewText->setAlpha(_ViewText->getAlpha()>>2);
}


// ***************************************************************************
void CCtrlTextButton::updateCoords()
{
	// Should have been setuped with addCtrl
	nlassert(_Setuped);

	// Compute Size according to bitmap and Text.
	if (!(_SizeRef & 1))
	{
		_W= _ViewText->getW() + _WMargin + _TextX;
		// Ensure as big as possible button
		_W= max(_W, _WMin);
	}
	if (!(_SizeRef & 2))
	{
		_H= _BmpH;
	}

	CViewBase::updateCoords();
}

// ***************************************************************************
sint32 CCtrlTextButton::getWMax() const
{
	return max(_ViewText->getW(false) + _WMargin + _TextX, _WMin);
}

// ***************************************************************************
void CCtrlTextButton::setup()
{
	_Setuped= true;

	// setup the viewText and add to parent
	_ViewText->setParent (getParent());
	_ViewText->setParentPos (this);
	_ViewText->setParentPosRef (_TextParentPosRef);
	_ViewText->setPosRef (_TextPosRef);
	_ViewText->setActive(_Active);
	_ViewText->setX(_TextX);
	_ViewText->setY(_TextY);

	getParent()->addView(_ViewText);
}

// ***************************************************************************
void CCtrlTextButton::setTextX(sint32 x)
{
	_TextX = x;
	if (_ViewText) _ViewText->setX(_TextX);
}

// ***************************************************************************
sint32	CCtrlTextButton::getMaxUsedW() const
{
	return _W;
}

// ***************************************************************************
sint32	CCtrlTextButton::getMinUsedW() const
{
	return _W;
}

// ***************************************************************************
void CCtrlTextButton::setActive(bool state)
{
	_ViewText->setActive(state);
	CCtrlBaseButton::setActive(state);
}


// ***************************************************************************
void CCtrlTextButton::onAddToGroup()
{
	// Add the view if not done
	if(!_Setuped)
		setup();
}


// ***************************************************************************
void CCtrlTextButton::setText (const ucstring &text)
{
	if (_ViewText && !_IsViewTextId)
		_ViewText->setText(text);
}

// ***************************************************************************
ucstring CCtrlTextButton::getText () const
{
	if (_ViewText && !_IsViewTextId)
		return _ViewText->getText();
	return ucstring("");
}

// ***************************************************************************
void CCtrlTextButton::setHardText (const std::string &text)
{
	if (_ViewText && !_IsViewTextId)
		_ViewText->setHardText(text);
}

// ***************************************************************************
string CCtrlTextButton::getHardText () const
{
	if (_ViewText && !_IsViewTextId)
		return _ViewText->getHardText();
	return string("");
}

// ***************************************************************************

CViewText*	CCtrlTextButton::getViewText()
{
	return _ViewText;
}

// ***************************************************************************

int CCtrlTextButton::luaGetViewText(CLuaState &ls)
{
	const char *funcName = "getViewText";
	CLuaIHM::checkArgCount(ls, funcName, 0);
	CLuaIHM::pushUIOnStack(ls, getViewText());
	return 1;
}

// ***************************************************************************
