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
#include "nel/gui/interface_group.h"
#include "nel/gui/interface_property.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/interface_link.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/mem_stream.h"
//

using namespace std;
using namespace NLMISC;

namespace NLGUI
{
	bool CInterfaceElement::editorMode = false;

	// ------------------------------------------------------------------------------------------------
	CInterfaceElement::~CInterfaceElement()
	{
		if (_Links) // remove any link that point to that element
		{
			for(TLinkVect::iterator it = _Links->begin(); it != _Links->end(); ++it)
			{
				(*it)->removeTarget(this);
			}
			delete _Links;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::parseError(CInterfaceGroup * parentGroup, const char *reason)
	{
		string tmp = string("cannot parse view:")+getId()+", parent:"+parentGroup->getId();
		nlinfo(tmp.c_str());
		if (reason)
			nlinfo("reason : %s", reason);
	}


	void CInterfaceElement::setIdRecurse(const std::string &newID)
	{
		std::string baseId = _Parent ? _Parent->getId() : "ui";
		setId(baseId + ":" + newID);
	}

	// ------------------------------------------------------------------------------------------------
	std::string CInterfaceElement::getShortId() const
	{
		std::string::size_type last = _Id.find_last_of(':');
		if (last != std::string::npos)
		{
			return _Id.substr(last + 1);
		}
		return _Id;
	}

	std::string CInterfaceElement::stripId( const std::string &fullId )
	{
		std::string id = fullId;
		std::string::size_type i = id.find_last_of( ':' );
		if( i != std::string::npos )
			id = id.substr( i + 1, id.size() - 1 );
		return id;
	}

	std::string CInterfaceElement::getProperty( const std::string &name ) const
	{
		if( name == "id" )
		{
			return stripId( getId() );
		}
		else
		if( name == "active" )
		{
			if( getActive() )
				return "true";
			else
				return "false";
		}
		else
		if( name == "x" )
		{
			return NLMISC::toString( getX() );
		}
		else
		if( name == "y" )
		{
			return NLMISC::toString( getY() );
		}
		else
		if( name == "w" )
		{
			return NLMISC::toString( getW() );
		}
		else
		if( name == "h" )
		{
			return NLMISC::toString( getH() );
		}
		else
		if( name == "posref" )
		{
			std::string posref;
			posref = HotSpotToString( getParentPosRef() );
			posref += " ";
			posref += HotSpotToString( getPosRef() );
			return posref;
		}
		else
		if( name == "sizeref" )
		{
			return getSizeRefAsString( _SizeRef, _SizeDivW, _SizeDivH );
		}
		if( name == "posparent" )
		{
			return CWidgetManager::getInstance()->getParser()->getParentPosAssociation( (CInterfaceElement*)this );
		}
		else
		if( name == "sizeparent" )
		{
			return CWidgetManager::getInstance()->getParser()->getParentSizeAssociation( (CInterfaceElement*)this );
		}
		else
		if( name == "global_color" )
		{
			return toString( _ModulateGlobalColor );
		}
		else
		if( name == "render_layer" )
		{
			return toString( _RenderLayer );
		}
		else
		if( name == "avoid_resize_parent" )
		{
			return toString( _AvoidResizeParent );
		}
		else
		{
			nlwarning( "Invalid property '%s' queried for widget '%s'", name.c_str(), _Id.c_str() );
			return "";
		}
	}

	void CInterfaceElement::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "id" )
		{
			setIdRecurse( stripId( value ) );
			return;
		}
		else
		if( name == "active" )
		{
			bool b;
			if( fromString( value, b ) )
				setActive( b );
			return;
		}
		else
		if( name == "x" )
		{
			sint32 x;
			if( fromString( value, x ) )
				setX( x );
			return;
		}
		else
		if( name == "y" )
		{
			sint32 y;
			if( fromString( value, y ) )
				setY( y );
			return;
		}
		else
		if( name == "w" )
		{
			sint32 w;
			if( fromString( value, w ) )
				setW( w );
			return;
		}
		else
		if( name == "h" )
		{
			sint32 h;
			if( fromString( value, h ) )
				setH( h );
			return;
		}
		else
		if( name == "posref" )
		{
			convertHotSpotCouple( value.c_str(), _ParentPosRef, _PosRef );
			return;
		}
		else
		if( name == "sizeref" )
		{
			parseSizeRef( value.c_str() );
			return;
		}
		if( name == "posparent" )
		{
			setPosParent( value );
			return;
		}
		else
		if( name == "sizeparent" )
		{
			setSizeParent( value );
			return;
		}
		else
		if( name == "global_color" )
		{
			bool b;
			if( fromString( value, b ) )
				setModulateGlobalColor( b );
			return;
		}
		else
		if( name == "render_layer" )
		{
			sint8 l;
			if( fromString( value, l ) )
				setRenderLayer( l );
			return;
		}
		else
		if( name == "avoid_resize_parent" )
		{
			bool b;
			if( fromString( value, b ) )
				setAvoidResizeParent( b );
			return;
		}
		else
			nlwarning( "Tried to set invalid property '%s' for widget '%s'", name.c_str(), _Id.c_str() );
	}

	xmlNodePtr CInterfaceElement::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = xmlNewNode( NULL, BAD_CAST type );
		if( node == NULL )
			return NULL;

		xmlAddChild( parentNode, node );

		xmlNewProp( node, BAD_CAST "id", BAD_CAST stripId( getId() ).c_str() );
		xmlNewProp( node, BAD_CAST "active", BAD_CAST toString( _Active ).c_str() );
		xmlNewProp( node, BAD_CAST "x", BAD_CAST toString( _X ).c_str() );
		xmlNewProp( node, BAD_CAST "y", BAD_CAST toString( _Y ).c_str() );
		xmlNewProp( node, BAD_CAST "w", BAD_CAST toString( _W ).c_str() );
		xmlNewProp( node, BAD_CAST "h", BAD_CAST toString( _H ).c_str() );
		xmlNewProp( node, BAD_CAST "posref", BAD_CAST HotSpotCoupleToString( _ParentPosRef, _PosRef ).c_str() );
		xmlNewProp( node, BAD_CAST "posparent",
			BAD_CAST CWidgetManager::getInstance()->getParser()->getParentPosAssociation( (CInterfaceElement*)this ).c_str() );
		xmlNewProp( node, BAD_CAST "sizeref", BAD_CAST getSizeRefAsString().c_str() );
		xmlNewProp( node, BAD_CAST "sizeparent",
			BAD_CAST CWidgetManager::getInstance()->getParser()->getParentSizeAssociation( (CInterfaceElement*)this ).c_str() );

		xmlNewProp( node, BAD_CAST "global_color", BAD_CAST toString( _ModulateGlobalColor ).c_str() );
		xmlNewProp( node, BAD_CAST "render_layer", BAD_CAST toString( _RenderLayer ).c_str() );
		xmlNewProp( node, BAD_CAST "avoid_resize_parent", BAD_CAST toString( _AvoidResizeParent ).c_str() );

		return node;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceElement::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		// parse the basic properties
		CXMLAutoPtr ptr((const char*) xmlGetProp( cur, (xmlChar*)"id" ));
		if (ptr)
		{
			if (parentGroup)
			{
				_Id = ( (CInterfaceElement*)parentGroup )->_Id;
			}
			else
			{
				_Id ="ui";
			}
			_Id += ":"+ string((const char*)ptr);
		}
		else
		{
			nlinfo(" error no id in an element");
			return false;
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"active" );
		_Active = true;
		if (ptr)
		{
			_Active = convertBool(ptr);
		}

		_Parent = parentGroup;

		// parse location. If these properties are not specified, set them to 0
		ptr = (char*) xmlGetProp( cur, (xmlChar*)"x" );
		_X = 0;
		if (ptr) fromString((const char*)ptr, _X);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"y" );
		_Y = 0;
		if (ptr) fromString((const char*)ptr, _Y);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"w" );
		_W = 0;
		if (parentGroup != NULL)
			_W = parentGroup->getW();
		if (ptr) fromString((const char*)ptr, _W);

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"h" );
		_H = 0;
		if (parentGroup != NULL)
			_H = parentGroup->getH();
		if (ptr) fromString((const char*)ptr, _H);

		// snapping
	//	ptr = (char*) xmlGetProp( cur, (xmlChar*)"snap" );
	//	_Snap = 1;
	//	if (ptr)
	//		fromString((const char*)ptr, _Snap);
	//	if (_Snap <= 0)
	//	{
	//		parseError(parentGroup, "snap must be > 0" );
	//		return false;
	//	}

		ptr = (char*) xmlGetProp( cur, (xmlChar*) "posref" );
		_ParentPosRef = Hotspot_BL;
		_PosRef = Hotspot_BL;
		if (ptr)
		{
			convertHotSpotCouple(ptr.getDatas(), _ParentPosRef, _PosRef);
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"posparent" );
		if (ptr)
		{
			setPosParent( std::string( (const char*)ptr ) );
		}

		ptr = (char*) xmlGetProp( cur, (xmlChar*)"sizeparent" );
		if (ptr)
		{
			setSizeParent( std::string( (const char*)ptr ) );
		}

		ptr = (char*) xmlGetProp (cur, (xmlChar*)"sizeref");
		_SizeRef = 0;
		_SizeDivW = 10;
		_SizeDivH = 10;
		if (ptr)
		{
			parseSizeRef(ptr.getDatas());
		}

	//	snapSize();

		ptr= (char*) xmlGetProp (cur, (xmlChar*)"global_color");
		if(ptr)
		{
			_ModulateGlobalColor= convertBool(ptr);
		}

		ptr= (char*) xmlGetProp (cur, (xmlChar*)"render_layer");
		if(ptr)		fromString((const char*)ptr, _RenderLayer);

		ptr= (char*) xmlGetProp (cur, (xmlChar*)"avoid_resize_parent");
		if(ptr)		_AvoidResizeParent= convertBool(ptr);

		return true;
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::setSizeRef(const std::string &sizeref)
	{
		parseSizeRef(sizeref.c_str());
	}

	// ------------------------------------------------------------------------------------------------
	std::string CInterfaceElement::getSizeRefAsString() const
	{
		return getSizeRefAsString( _SizeRef, _SizeDivW, _SizeDivH );
	}

	std::string CInterfaceElement::getSizeRefAsString( const sint32 &sizeRef, const sint32 &sizeDivW, sint32 const &sizeDivH ) const
	{
		std::string s;
		if( ( sizeRef & 1 ) != 0 )
		{
			s += "w";
			if( sizeDivW < 10 )
				s += toString( sizeDivW );
		}

		if( ( _SizeRef & 2 ) != 0 )
		{
			s += "h";
			if( sizeDivH < 10 )
				s += toString( sizeDivH );
		}

		return s;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::parseSizeRef(const char *sizeRefStr)
	{
		parseSizeRef(sizeRefStr, _SizeRef, _SizeDivW, _SizeDivH);
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::parseSizeRef(const char *sizeRefStr, sint32 &sizeRef, sint32 &sizeDivW, sint32 &sizeDivH)
	{
		nlassert(sizeRefStr);

		sizeRef = 0;
		sizeDivW = 10;
		sizeDivH = 10;
		sint32 nWhat = 0;
		const char *seekPtr = sizeRefStr;
		while (*seekPtr != 0)
		{
			if ((*seekPtr=='w')||(*seekPtr=='W'))
			{
				sizeRef |= 1;
				nWhat = 1;
			}

			if ((*seekPtr=='h')||(*seekPtr=='H'))
			{
				sizeRef |= 2;
				nWhat = 2;
			}

			if ((*seekPtr>='1')&&(*seekPtr<='9'))
			{
				if (nWhat != 0)
				{
					if (nWhat == 1)
						sizeDivW = *seekPtr-'0';
					if (nWhat == 2)
						sizeDivH = *seekPtr-'0';
				}
			}

			++seekPtr;
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::updateCoords()
	{
		_XReal = _X;
		_YReal = _Y;
		_WReal = getW();
		_HReal = getH();

		CInterfaceElement *el = NULL;

		// Modif Pos

		if (_ParentPos != NULL)
			el = _ParentPos;
		else
			el = _Parent;

		if (el == NULL)
			return;

		_XReal += el->_XReal;
		_YReal += el->_YReal;

		THotSpot hsParent = _ParentPosRef;
		if (hsParent &  Hotspot_Mx)
			_YReal += el->_HReal/2;
		if (hsParent & Hotspot_Tx)
			_YReal += el->_HReal;
		if (hsParent & Hotspot_xM)
			_XReal += el->_WReal/2;
		if (hsParent & Hotspot_xR)
			_XReal += el->_WReal;

		// Modif Size

		if (_ParentSize != NULL)
		{
			el = _ParentSize;
		}
		else
		{
			if (_ParentPos != NULL)
				el = _ParentPos;
			else
				el = _Parent;
		}

		if (el == NULL)
			return;

		if (_SizeRef&1)
			_WReal += _SizeDivW * el->_WReal / 10;

		if (_SizeRef&2)
			_HReal += _SizeDivH * el->_HReal / 10;

		THotSpot hs = _PosRef;
		if (hs & Hotspot_Mx)
			_YReal -= _HReal/2;
		if (hs & Hotspot_Tx)
			_YReal -= _HReal;
		if (hs & Hotspot_xM)
			_XReal -= _WReal/2;
		if (hs & Hotspot_xR)
			_XReal -= _WReal;
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::getCorner(sint32 &px, sint32 &py, THotSpot hs)
	{
		px = _XReal;
		py = _YReal;
		if (hs & 1) px += _WReal;
		if (hs & 2) px += _WReal >> 1;
		if (hs & 8) py += _HReal;
		if (hs & 16) py += _HReal >> 1;
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::move (sint32 dx, sint32 dy)
	{
		_X += dx;
		_Y += dy;
		invalidateCoords();
	}



	// ------------------------------------------------------------------------------------------------
	/*void CInterfaceElement::resizeBR (sint32 sizeW, sint32 sizeH)
	{
		uint32 i = i / 0;
		THotSpot hs = _PosRef;

		sint32 dw = sizeW - _W;
		sint32 dh = sizeH - _H;

		sint32 snap = _Snap;
		nlassert(snap > 0);

		if (hs&8) // is top ?
		{
			sint32 newH = dh + _H;
			if (snap > 1)
				newH -= newH % snap;
			_H = newH;
		}
		if (hs&32) // is bottom ?
		{
			sint32 newH = dh + _H;
			if (snap > 1)
				newH -= newH % snap;
			_Y = _H - newH + _Y;
			_H = newH;
		}

		if (hs&1) // is right ?
		{
			sint32 newW = dw + _W;
			if (snap > 1)
				newW -= newW % snap;
			_X = newW - _W + _X;
			_W = newW;
		}
		if (hs&4) // is left ?
		{
			sint32 newW = dw + _W;
			if (snap > 1)
				newW -= newW % snap;
			_W = newW;
		}

		// DO NOT TREAT THE MIDDLE HOTSPOT CASE

		invalidateCoords();
	}*/


	// ------------------------------------------------------------------------------------------------
	/*void CInterfaceElement::snapSize()
	{
		sint32 snap = _Snap;
		nlassert(snap > 0);
		if (snap > 1)
		{
			_W = _W - (_W % snap);
			_H = _H - (_H % snap);
		}
	}*/


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::setW (sint32 w)
	{
		_W = w;
	//	sint32 snap = _Snap;
	//	nlassert(snap > 0);
	//	if (snap > 1)
	//	{
	//		_W = _W - (_W % snap);
	//	}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::setH (sint32 h)
	{
		_H = h;
	//	sint32 snap = _Snap;
	//	nlassert(snap > 0);
	//	if (snap > 1)
	//	{
	//		_H = _H - (_H % snap);
	//	}
	}


	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CInterfaceElement::getRootWindow ()
	{
		if (_Parent == NULL)
			return NULL;
		if (_Parent->getParent() == NULL)
			return dynamic_cast<CInterfaceGroup*>(this);
		return _Parent->getRootWindow();
	}

	// ------------------------------------------------------------------------------------------------
	uint	CInterfaceElement::getParentDepth() const
	{
		uint	depth= 0;
		CInterfaceGroup *parent= _Parent;
		while(parent!=NULL)
		{
			parent= parent->getParent();
			depth++;
		}
		return depth;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceElement::isActiveThroughParents() const
	{
		if(!getActive())
			return false;
		if(_Parent == NULL)
			return false;
		// is it the root window?
		if (_Parent->getParent() == NULL)
			// yes and getActive() is true => the element is visible!
			return true;
		else
			return _Parent->isActiveThroughParents();
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::relativeSInt64Read (CInterfaceProperty &rIP, const string &prop, const char *val,
														   const string &defVal)
	{
		if (val == NULL)
		{
			rIP.readSInt64 (defVal.c_str(), _Id+":"+prop);
		}
		else
		{
			if ( isdigit(*val) || *val=='-')
			{
				rIP.readSInt64 (val, _Id+":"+prop);
				return;
			}

			sint32 decal = 0;
			if (val[0] == ':')
				decal = 1;
			if (NLGUI::CDBManager::getInstance()->getDbProp(val+decal, false) != NULL)
			{
				rIP.readSInt64 (val+decal, _Id+":"+prop);
				return;
			}
			else
			{
				string sTmp;
				CInterfaceElement *pIEL = this;

				while (pIEL != NULL)
				{
					sTmp = pIEL->getId()+":"+string(val+decal);
					if (NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false) != NULL)
					{
						rIP.readSInt64 (sTmp.c_str(), _Id+":"+prop);
						return;
					}
					pIEL = pIEL->getParent();
				}

				rIP.readSInt64 (val+decal, _Id+":"+prop);
			}
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::relativeSInt32Read (CInterfaceProperty &rIP, const string &prop, const char *val,
														   const string &defVal)
	{
		if (val == NULL)
		{
			rIP.readSInt32 (defVal.c_str(), _Id+":"+prop);
		}
		else
		{
			if ( isdigit(*val) || *val=='-')
			{
				rIP.readSInt32 (val, _Id+":"+prop);
				return;
			}

			sint32 decal = 0;
			if (val[0] == ':')
				decal = 1;
			if (NLGUI::CDBManager::getInstance()->getDbProp(val+decal, false) != NULL)
			{
				rIP.readSInt32 (val+decal, _Id+":"+prop);
				return;
			}
			else
			{
				string sTmp;
				CInterfaceElement *pIEL = this;

				while (pIEL != NULL)
				{
					sTmp = pIEL->getId()+":"+string(val+decal);
					if (NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false) != NULL)
					{
						rIP.readSInt32 (sTmp.c_str(), _Id+":"+prop);
						return;
					}
					pIEL = pIEL->getParent();
				}

				rIP.readSInt32 (val+decal, _Id+":"+prop);
			}
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::relativeBoolRead (CInterfaceProperty &rIP, const string &prop, const char *val,
														   const string &defVal)
	{
		if (val == NULL)
		{
			rIP.readBool (defVal.c_str(), _Id+":"+prop);
		}
		else
		{
			sint32 decal = 0;
			if (val[0] == ':')
				decal = 1;
			if (NLGUI::CDBManager::getInstance()->getDbProp(val+decal, false) != NULL)
			{
				rIP.readBool (val+decal, _Id+":"+prop);
				return;
			}
			else
			{
				string sTmp;
				CInterfaceElement *pIEL = this;

				while (pIEL != NULL)
				{
					sTmp = pIEL->getId()+":"+string(val+decal);
					if (NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false) != NULL)
					{
						rIP.readBool (sTmp.c_str(), _Id+":"+prop);
						return;
					}
					pIEL = pIEL->getParent();
				}

				rIP.readBool (val+decal, _Id+":"+prop);
			}
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::relativeRGBARead(CInterfaceProperty &rIP,const std::string &prop,const char *val,const std::string &defVal)
	{
		if (val == NULL)
		{
			rIP.readRGBA (defVal.c_str(), _Id+":"+prop);
		}
		else
		{
			if ( isdigit(*val) || *val=='-')
			{
				rIP.readRGBA (val, _Id+":"+prop);
				return;
			}

			sint32 decal = 0;
			if (val[0] == ':')
				decal = 1;
			if (NLGUI::CDBManager::getInstance()->getDbProp(val+decal, false) != NULL)
			{
				rIP.readRGBA (val+decal, _Id+":"+prop);
				return;
			}
			else
			{
				string sTmp;
				CInterfaceElement *pIEL = this;

				while (pIEL != NULL)
				{
					sTmp = pIEL->getId()+":"+string(val+decal);
					if (NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false) != NULL)
					{
						rIP.readRGBA (sTmp.c_str(), _Id+":"+prop);
						return;
					}
					pIEL = pIEL->getParent();
				}

				rIP.readRGBA (val+decal, _Id+":"+prop);
			}
		}
	}

	std::string CInterfaceElement::HotSpotToString( THotSpot spot )
	{
		switch( spot )
		{
		case Hotspot_TL:
			return "TL";
			break;

		case Hotspot_TM:
			return "TM";
			break;

		case Hotspot_TR:
			return "TR";
			break;

		case Hotspot_ML:
			return "ML";
			break;

		case Hotspot_MM:
			return "MM";
			break;

		case Hotspot_MR:
			return "MR";
			break;

		case Hotspot_BL:
			return "BL";
			break;

		case Hotspot_BM:
			return "BM";
			break;

		case Hotspot_BR:
			return "BR";
			break;
		}

		return "";
	}

	std::string CInterfaceElement::HotSpotCoupleToString( THotSpot parentPosRef, THotSpot posRef )
	{
		std::string hs;
		hs = HotSpotToString( parentPosRef );
		hs += " ";
		hs += HotSpotToString( posRef );
		
		return hs;
	}

	// ------------------------------------------------------------------------------------------------
	THotSpot CInterfaceElement::convertHotSpot (const char *ptr)
	{
		if ( !strnicmp(ptr,"TL",2) )
		{
			return Hotspot_TL;
		}
		else if ( !strnicmp(ptr,"TM",2) )
		{
			return Hotspot_TM;
		}
		else if ( !strnicmp(ptr,"TR",2) )
		{
			return Hotspot_TR;
		}
		else if ( !strnicmp(ptr,"ML",2) )
		{
			return Hotspot_ML;
		}
		else if ( !strnicmp(ptr,"MM",2) )
		{
			return Hotspot_MM;
		}
		else if ( !strnicmp(ptr,"MR",2) )
		{
			return Hotspot_MR;
		}
		else if ( !strnicmp(ptr,"BL",2) )
		{
			return Hotspot_BL;
		}
		else if ( !strnicmp(ptr,"BM",2) )
		{
			return Hotspot_BM;
		}
		else if ( !strnicmp(ptr,"BR",2) )
		{
			return Hotspot_BR;
		}
		else
			return Hotspot_BL;
	}

	// ------------------------------------------------------------------------------------------------
	void		CInterfaceElement::convertHotSpotCouple (const char *ptr, THotSpot &parentPosRef, THotSpot &posRef)
	{
		nlassert(ptr);

		// *** first hotspot
		// skip any space or tab
		while(*ptr=='\t' || *ptr==' ')
			ptr++;
		// convert first
		parentPosRef = convertHotSpot (ptr);

		// *** second hotspot
		// must be at least 2 letter and a space
		nlassert(strlen(ptr)>=3);
		ptr+=3;
		// skip any space or tab
		while(*ptr=='\t' || *ptr==' ')
			ptr++;
		// convert second
		posRef = convertHotSpot (ptr);
	}

	// ------------------------------------------------------------------------------------------------
	NLMISC::CRGBA CInterfaceElement::convertColor (const char *ptr)
	{
		return NLMISC::CRGBA::stringToRGBA(ptr);
	}

	// ------------------------------------------------------------------------------------------------
	bool			CInterfaceElement::convertBool (const char *ptr)
	{
		std::string str = ptr;
		NLMISC::strlwr( str );
		bool b = false;
		fromString( str, b );
		return b;
	}

	// ------------------------------------------------------------------------------------------------
	NLMISC::CVector CInterfaceElement::convertVector (const char *ptr)
	{
		float x = 0.0f, y = 0.0f, z = 0.0f;

		sscanf (ptr, "%f %f %f", &x, &y, &z);

		return CVector(x,y,z);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::convertPixelsOrRatio(const char *ptr, sint32 &pixels, float &ratio)
	{
		std::string value = ptr;
		if (!value.empty())
		{
			if (value[value.size() - 1] == '%')
			{
				value.resize(value.size() - 1);
				fromString(value, ratio);
				ratio /= 100.f;
				clamp(ratio, 0.f, 1.f);
			}
			else
			{
				fromString(value, pixels);
			}
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::addLink(CInterfaceLink *link)
	{
		nlassert(link != NULL);
		if (!_Links)
		{
			_Links = new TLinkVect;
		}
		TLinkSmartPtr linkPtr(link);
		TLinkVect::const_iterator it = std::find(_Links->begin(), _Links->end(), linkPtr);
		if (it != _Links->end())
		{
			// Link already appened : this can be the case when a link has several targets property that belong to the same element, in this case, one single ptr in the vector is enough.
			// nlwarning("Link added twice");
		}
		else
		{
			_Links->push_back(linkPtr);
		}
	}


	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::removeLink(CInterfaceLink *link)
	{
		nlassert(link != NULL);
		if (!_Links)
		{
			nlwarning("No link added");
			return;
		}
		TLinkVect::iterator it = std::find(_Links->begin(), _Links->end(), TLinkSmartPtr(link));
		if (it == _Links->end())
		{
			nlwarning("Unknown link");
			return;
		}
		_Links->erase(it); // kill the smart ptr, maybe deleting the link.
		if (_Links->empty())
		{
			delete _Links;
			_Links = NULL;
		}
	}


	// ------------------------------------------------------------------------------------------------
	CInterfaceElement* CInterfaceElement::getMasterGroup() const
	{
		if(getParent()==NULL)
			return const_cast<CInterfaceElement*>(this);
		else
			return getParent()->getMasterGroup();
	}

	// ------------------------------------------------------------------------------------------------
	CInterfaceGroup* CInterfaceElement::getParentContainer()
	{
		CInterfaceElement *parent = this;
		while (parent)
		{
			CInterfaceGroup *gc = dynamic_cast< CInterfaceGroup* >( parent );
			if( ( gc != NULL ) && gc->isGroupContainer() )
				return gc;

			parent = parent->getParent();
		}
		return NULL;
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceElement::isIn(sint x, sint y) const
	{
		return  (x >= _XReal) &&
				(x < (_XReal + _WReal))&&
				(y > _YReal) &&
				(y <= (_YReal+ _HReal));
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceElement::isIn(sint x, sint y, uint width, uint height) const
	{
		return (x + (sint) width) >= _XReal &&
			   (y + (sint) height) > _YReal &&
			   x < (_XReal + _WReal) &&
			   y <= (_YReal + _HReal);
	}

	// ------------------------------------------------------------------------------------------------
	bool CInterfaceElement::isIn(const CInterfaceElement &other) const
	{
		return isIn(other._XReal, other._YReal, other._WReal, other._HReal);
	}

	// ------------------------------------------------------------------------------------------------
	void CInterfaceElement::setActive (bool state)
	{
		if (_Active != state)
		{
			_Active = state;
			invalidateCoords();
		}
	}


	// ***************************************************************************
	void		CInterfaceElement::invalidateCoords(uint8 numPass)
	{
		// Get the "Root Group" ie the 1st son of the master group of us (eg "ui:interface:rootgroup" )
		CInterfaceGroup		*parent= getParent();
		// if our parent is NULL, then we are the master group (error!)
		if(parent==NULL)
			return;
		// if our grandfather is NULL, then our father is the Master Group => we are the "Root group"
		if(parent->getParent()==NULL)
		{
			parent= dynamic_cast<CInterfaceGroup*>(this);
		}
		else
		{
			// parent is the root group when is grandFather is NULL
			while( parent->getParent()->getParent()!=NULL )
			{
				parent= parent->getParent();
			}
		}

		// invalidate the "root group"
		if(parent)
		{
			uint8	&val= static_cast<CInterfaceElement*>(parent)->_InvalidCoords;
			val= max(val, numPass);
		}
	}


	// ***************************************************************************
	void	CInterfaceElement::checkCoords()
	{
	}

	// ***************************************************************************
	bool CInterfaceElement::isSonOf(const CInterfaceElement *other) const
	{
		const CInterfaceElement  *currElem = this;
		do
		{
			if (currElem == other) return true;
			currElem = currElem->_Parent;
		}
		while (currElem);
		return false;
	}

	// ***************************************************************************
	void	CInterfaceElement::resetInvalidCoords()
	{
		_InvalidCoords= 0;
	}

	// ***************************************************************************
	void CInterfaceElement::updateAllLinks()
	{
		if (_Links)
		{
			for(TLinkVect::iterator it = _Links->begin(); it != _Links->end(); ++it)
			{
				(*it)->update();
			}
		}
	}

	// ***************************************************************************
	void CInterfaceElement::copyOptionFrom(const CInterfaceElement &other)
	{
		_Active = other._Active;
		_InvalidCoords = other._InvalidCoords;
		_XReal = other._XReal;
		_YReal = other._YReal;
		_WReal = other._WReal;
		_HReal = other._HReal;
		_X = other._X;
		_Y = other._Y;
		_XReal = other._XReal;
		_YReal = other._YReal;
		_PosRef = other._PosRef;
		_ParentPosRef = other._ParentPosRef;
		_SizeRef = other._SizeRef;
		_SizeDivW = other._SizeDivW;
		_SizeDivH = other._SizeDivH;
		_ModulateGlobalColor = other._ModulateGlobalColor;
		_RenderLayer = other._RenderLayer;

	}

	// ***************************************************************************
	void CInterfaceElement::center()
	{
		// center the pc
		CViewRenderer &vr = *CViewRenderer::getInstance();
		uint32 sw, sh;
		vr.getScreenSize(sw, sh);
		setX(sw / 2 - getWReal() / 2);
		setY(sh / 2 + getHReal() / 2);
	}

	// ***************************************************************************
	void CInterfaceElement::renderWiredQuads(TRenderWired type, const std::string &uiFilter)
	{
		CCtrlBase *ctrlBase = dynamic_cast<CCtrlBase*>(this);
		CInterfaceGroup *groupBase = dynamic_cast<CInterfaceGroup*>(this);
		if (
			((type == RenderView) && (ctrlBase==NULL) && (groupBase==NULL)) ||
			((type == RenderCtrl) && (ctrlBase!=NULL) && (groupBase==NULL)) ||
			((type == RenderGroup) && (ctrlBase!=NULL) && (groupBase!=NULL)))
		{
			if (!_Active) return;
			// if there is an uiFilter, the end of _Id must match it
			if (!uiFilter.empty() && (uiFilter.size()>_Id.size() ||
				_Id.compare(_Id.size()-uiFilter.size(),string::npos,uiFilter)!=0)
				)
				return;
			CViewRenderer &vr = *CViewRenderer::getInstance();
			vr.drawWiredQuad(_XReal, _YReal, _WReal, _HReal);
			drawHotSpot(_PosRef, CRGBA::Red);
			if (_Parent) _Parent->drawHotSpot(_ParentPosRef, CRGBA::Blue);
		}
	}

	// ***************************************************************************
	void CInterfaceElement::drawHotSpot(THotSpot hs, CRGBA col)
	{
		const sint32 radius = 2;
		sint32 px, py;
		//
		if (hs & Hotspot_Bx)
		{
			py = _YReal + radius;
		}
		else if (hs & Hotspot_Mx)
		{
			py = _YReal + _HReal / 2;
		}
		else
		{
			py = _YReal + _HReal - radius;
		}
		//
		if (hs & Hotspot_xL)
		{
			px = _XReal + radius;
		}
		else if (hs & Hotspot_xM)
		{
			px = _XReal + _WReal / 2;
		}
		else
		{
			px = _XReal + _WReal - radius;
		}
		CViewRenderer &vr = *CViewRenderer::getInstance();
		vr.drawFilledQuad(px - radius, py - radius, radius * 2, radius * 2, col);

	}

	// ***************************************************************************
	void CInterfaceElement::invalidateContent()
	{
		CInterfaceElement *elm = this;
		while (elm)
		{
			// Call back
			elm->onInvalidateContent();

			// Get the parent
			elm = elm->getParent();
		}
	}

	// ***************************************************************************
	void CInterfaceElement::visit(CInterfaceElementVisitor *visitor)
	{
		nlassert(visitor);
		visitor->visit(this);
	}

	// ***************************************************************************
	void CInterfaceElement::serialConfig(NLMISC::IStream &f)
	{
		if (f.isReading())
		{
			throw NLMISC::ENewerStream(f);
			nlassert(0);
		}
	}

	// ***************************************************************************
	void CInterfaceElement::onFrameUpdateWindowPos(sint dx, sint dy)
	{
		_XReal+= dx;
		_YReal+= dy;
	}

	// ***************************************************************************
	void CInterfaceElement::dummySet(sint32 /* value */)
	{
		nlwarning("Element can't be written.");
	}

	// ***************************************************************************
	void CInterfaceElement::dummySet(const std::string &/* value */)
	{
		nlwarning("Element can't be written.");
	}

	// ***************************************************************************
	int CInterfaceElement::luaUpdateCoords(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "updateCoords", 0);
		updateCoords();
		return 0;
	}

	// ***************************************************************************
	int CInterfaceElement::luaInvalidateCoords(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "updateCoords", 0);
		invalidateCoords();
		return 0;
	}

	// ***************************************************************************
	int CInterfaceElement::luaInvalidateContent(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "invalidateContent", 0);
		invalidateContent();
		return 0;
	}

	// ***************************************************************************
	int CInterfaceElement::luaCenter(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "center", 0);
		center();
		return 0;
	}

	// ***************************************************************************
	int CInterfaceElement::luaSetPosRef(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "setPosRef", 1);
		CLuaIHM::check(ls,   ls.isString(1),    "setPosRef() requires a string in param 1");

		// get hotspot
		THotSpot	newParentPosRef, newPosRef;
		convertHotSpotCouple(ls.toString(1), newParentPosRef, newPosRef);

		// if different from current, set,a nd invalidate coords
		if(newParentPosRef!=getParentPosRef() || newPosRef!=getPosRef())
		{
			setParentPosRef(newParentPosRef);
			setPosRef(newPosRef);
			invalidateCoords();
		}

		return 0;
	}

	// ***************************************************************************
	int CInterfaceElement::luaSetParentPos(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "setParentPos", 1);
		CInterfaceElement *ie = CLuaIHM::getUIOnStack(ls, 1);
		if(ie)
		{
			setParentPos(ie);
		}
		return 0;
	}



	// ***************************************************************************
	CInterfaceElement *CInterfaceElement::clone()
	{
		NLMISC::CMemStream dupStream;
		nlassert(!dupStream.isReading());
		CInterfaceGroup *oldParent = _Parent;
		_Parent = NULL;
		CInterfaceElement *oldParentPos = _ParentPos;
		CInterfaceElement *oldParentSize = _ParentSize;
		if (_ParentPos == oldParent) _ParentPos = NULL;
		if (_ParentSize == oldParent) _ParentSize = NULL;
		CInterfaceElement *begunThisCloneWarHas = NULL;
		try
		{
			if (dupStream.isReading())
			{
				dupStream.invert();
			}
			CInterfaceElement *self = this;
			dupStream.serialPolyPtr(self);
			std::vector<uint8> datas(dupStream.length());
			std::copy(dupStream.buffer(), dupStream.buffer() + dupStream.length(), datas.begin());
			dupStream.resetPtrTable();
			dupStream.invert();
			dupStream.fill(&datas[0], (uint32)datas.size());
			dupStream.serialPolyPtr(begunThisCloneWarHas);
		}
		catch(const NLMISC::EStream &)
		{
			// no-op -> caller has to handle the failure because NULL will be returned
		}
		//
		_Parent		  = oldParent;
		_ParentPos	  = oldParentPos;
		_ParentSize	  = oldParentSize;
		//
		return begunThisCloneWarHas;
	}

	// ***************************************************************************
	void CInterfaceElement::serial(NLMISC::IStream &f)
	{
		f.serialPolyPtr(_Parent);
		f.serial(_Id);
		f.serial(_Active);
		f.serial(_InvalidCoords);
		f.serial(_XReal, _YReal, _WReal, _HReal);
		f.serial(_X, _Y, _W, _H);
		f.serialEnum(_PosRef);
		f.serialEnum(_ParentPosRef);
		_ParentPos.serialPolyPtr(f);
		f.serial(_SizeRef);
		f.serial(_SizeDivW, _SizeDivH);
		_ParentSize.serialPolyPtr(f);
		f.serial(_ModulateGlobalColor);
		f.serial(_RenderLayer);
		f.serial(_AvoidResizeParent);
		nlassert(_Links == NULL); // not supported
	}


	// ***************************************************************************
	void CInterfaceElement::serialAH(NLMISC::IStream &f, IActionHandler *&ah)
	{
		std::string ahName;
		if (f.isReading())
		{
			f.serial(ahName);
			ah = CAHManager::getInstance()->getActionHandler(ahName);
		}
		else
		{
			ahName = CAHManager::getInstance()->getActionHandlerName(ah);
			f.serial(ahName);
		}
	}

	
	bool CInterfaceElement::isInGroup( CInterfaceGroup *group )
	{
		CInterfaceGroup *parent = getParent();
		while( parent != NULL )
		{
			if( parent == group )
				return true;
			else
				parent = parent->getParent();
		}
		return false;
	}

	void CInterfaceElement::setPosParent( const std::string &id )
	{
		std::string Id = stripId( id );

		if( Id != "parent" )
		{
			std::string idParent;
			if( _Parent != NULL )
				idParent = _Parent->getId() + ":";
			else
				idParent = "ui:";
			CWidgetManager::getInstance()->getParser()->addParentPositionAssociation( this, idParent + Id );
		}
	}

	void CInterfaceElement::setSizeParent( const std::string &id )
	{
		std::string Id = stripId( id );
		std::string idParent;

		if( Id != "parent" )
		{
			if( _Parent != NULL )
				idParent = _Parent->getId() + ":";
			else
				idParent = "ui:";
			CWidgetManager::getInstance()->getParser()->addParentSizeAssociation( this, idParent + Id );
		}
		else
		{
			if( _Parent != NULL )
			{
				idParent = _Parent->getId();
				CWidgetManager::getInstance()->getParser()->addParentSizeAssociation( this, idParent );
			}
		}
	}

	CStringMapper* CStringShared::_UIStringMapper = NULL;


	void CStringShared::createStringMapper()
	{
		if( _UIStringMapper == NULL )
			_UIStringMapper = CStringMapper::createLocalMapper();
	}

	void CStringShared::deleteStringMapper()
	{
		delete _UIStringMapper;
	}
}



