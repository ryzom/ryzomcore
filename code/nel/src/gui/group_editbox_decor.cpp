// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2014  Laszlo Kis-Adam
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
#include "nel/gui/group_editbox_decor.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_text.h"

namespace NLGUI
{
	class EBDPrivate
	{
	public:
		enum Textures
		{
			BG,
			L,
			R,
			TM,
			BM,
			TL,
			TR,
			BL,
			BR,
			TCOUNT
		};

		EBDPrivate()
		{
			for( int i = 0; i < TCOUNT; i++ )
			{
				_Textures.push_back( new CViewBitmap( CViewBase::TCtorParam() ) );
			}
		}

		~EBDPrivate()
		{
			for( int i = 0; i < _Textures.size(); i++ )
				delete _Textures[ i ];
			_Textures.clear();
		}

		void draw()
		{
			for( int i = 0; i < _Textures.size(); i++ )
			{
				CViewBitmap *bm = _Textures[ i ];
				bm->draw();
			}
		}

		void updateCoords()
		{
			for( int i = 0; i < _Textures.size(); i++ )
			{
				CViewBitmap *bm = _Textures[ i ];
				bm->fitTexture();
			}

			// W and H parameters depend on the sizes of the other textures
			// Negative sizes mean that the sizes are that much smaller than the parent
			sint32 w,h;
			h = _Textures[ TL ]->getHReal() + _Textures[ BL ]->getHReal();
			h *= -1;
			_Textures[ L ]->setH( h );

			h = _Textures[ TR ]->getHReal() + _Textures[ BR ]->getHReal();
			h *= -1;
			_Textures[ R ]->setH( h );

			w = _Textures[ TL ]->getWReal() + _Textures[ TR ]->getWReal();
			w *= -1;
			_Textures[ TM ]->setW( w );

			w = _Textures[ BL ]->getWReal() + _Textures[ BR ]->getWReal();
			w *= -1;
			_Textures[ BM ]->setW( w );

			h = _Textures[ TM ]->getHReal() + _Textures[ BM ]->getHReal();
			h *= -1;
			w = _Textures[ L ]->getWReal() + _Textures[ R ]->getWReal();
			w *= -1;
			_Textures[ BG ]->setW( w );
			_Textures[ BG ]->setH( h );

			for( int i = 0; i < _Textures.size(); i++ )
			{
				CViewBitmap *bm = _Textures[ i ];
				bm->updateCoords();
			}
		}

		void setup( CInterfaceGroup *parent )
		{
			for( int i = 0; i < _Textures.size(); i++ )
			{
				CViewBitmap *bm = _Textures[ i ];
				bm->setParent( parent );
				bm->setParentPos( parent );
				bm->setParentSize( parent );
				bm->setEditorSelectable( false );
			}

			_Textures[ TL ]->setPosRef( Hotspot_TL );
			_Textures[ TL ]->setParentPosRef( Hotspot_TL );

			_Textures[ TM ]->setPosRef( Hotspot_TM );
			_Textures[ TM ]->setParentPosRef( Hotspot_TM );
			_Textures[ TM ]->setScale( true );
			_Textures[ TM ]->setSizeRef( "w" );

			_Textures[ TR ]->setPosRef( Hotspot_TR );
			_Textures[ TR ]->setParentPosRef( Hotspot_TR );

			_Textures[ BL ]->setPosRef( Hotspot_BL );
			_Textures[ BL ]->setParentPosRef( Hotspot_BL );

			_Textures[ BM ]->setPosRef( Hotspot_BM );
			_Textures[ BM ]->setParentPosRef( Hotspot_BM );
			_Textures[ BM ]->setScale( true );
			_Textures[ BM ]->setSizeRef( "w" );

			_Textures[ BR ]->setPosRef( Hotspot_BR );
			_Textures[ BR ]->setParentPosRef( Hotspot_BR );

			_Textures[ L ]->setPosRef( Hotspot_ML );
			_Textures[ L ]->setParentPosRef( Hotspot_ML );
			_Textures[ L ]->setScale( true );
			_Textures[ L ]->setSizeRef( "h" );

			_Textures[ R ]->setPosRef( Hotspot_MR );
			_Textures[ R ]->setParentPosRef( Hotspot_MR );
			_Textures[ R ]->setScale( true );
			_Textures[ R ]->setSizeRef( "h" );

			_Textures[ BG ]->setPosRef( Hotspot_MM );
			_Textures[ BG ]->setParentPosRef( Hotspot_MM );
			_Textures[ BG ]->setScale( true );
			_Textures[ BG ]->setSizeRef( "wh" );
		}

		std::vector< CViewBitmap* > _Textures;
	};

	NLMISC_REGISTER_OBJECT( CViewBase, CGroupEditBoxDecor, std::string, "edit_box_decor" );

	CGroupEditBoxDecor::CGroupEditBoxDecor( const TCtorParam &param ) :
	CGroupEditBox( param )
	{
		_Pvt = new EBDPrivate();
		_Pvt->setup( this );

		createViewText();
		getVT()->setSerializable( false );
		getVT()->setEditorSelectable( false );
	}

	CGroupEditBoxDecor::~CGroupEditBoxDecor()
	{
		delete _Pvt;
		_Pvt = NULL;
	}

	void CGroupEditBoxDecor::moveBy( sint32 x, sint32 y )
	{
		CInterfaceElement::moveBy( x, y );

		_Pvt->updateCoords();
	}

	std::string CGroupEditBoxDecor::getProperty( const std::string &name ) const
	{
		if( name == "tx_tl" )
		{
			return _Pvt->_Textures[ EBDPrivate::TL ]->getTexture();
		}
		else
		if( name == "tx_tm" )
		{
			return _Pvt->_Textures[ EBDPrivate::TM ]->getTexture();
		}
		else
		if( name == "tx_tr" )
		{
			return _Pvt->_Textures[ EBDPrivate::TR ]->getTexture();
		}
		else
		if( name == "tx_bl" )
		{
			return _Pvt->_Textures[ EBDPrivate::BL ]->getTexture();
		}
		else
		if( name == "tx_bm" )
		{
			return _Pvt->_Textures[ EBDPrivate::BM ]->getTexture();
		}
		else
		if( name == "tx_br" )
		{
			return _Pvt->_Textures[ EBDPrivate::BR ]->getTexture();
		}
		else
		if( name == "tx_l" )
		{
			return _Pvt->_Textures[ EBDPrivate::L ]->getTexture();
		}
		else
		if( name == "tx_r" )
		{
			return _Pvt->_Textures[ EBDPrivate::R ]->getTexture();
		}
		else
		if( name == "tx_bg" )
		{
			return _Pvt->_Textures[ EBDPrivate::BG ]->getTexture();
		}
		else
			return CGroupEditBox::getProperty( name );
	}

	void CGroupEditBoxDecor::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "tx_tl" )
		{
			_Pvt->_Textures[ EBDPrivate::TL ]->setTexture( value );
		}
		else
		if( name == "tx_tm" )
		{
			_Pvt->_Textures[ EBDPrivate::TM ]->setTexture( value );
		}
		else
		if( name == "tx_tr" )
		{
			_Pvt->_Textures[ EBDPrivate::TR ]->setTexture( value );
		}
		else
		if( name == "tx_bl" )
		{
			_Pvt->_Textures[ EBDPrivate::BL ]->setTexture( value );
		}
		else
		if( name == "tx_bm" )
		{
			_Pvt->_Textures[ EBDPrivate::BM ]->setTexture( value );
		}
		else
		if( name == "tx_br" )
		{
			_Pvt->_Textures[ EBDPrivate::BR ]->setTexture( value );
		}
		else
		if( name == "tx_l" )
		{
			_Pvt->_Textures[ EBDPrivate::L ]->setTexture( value );
		}
		else
		if( name == "tx_r" )
		{
			_Pvt->_Textures[ EBDPrivate::R ]->setTexture( value );
		}
		else
		if( name == "tx_bg" )
		{
			_Pvt->_Textures[ EBDPrivate::BG ]->setTexture( value );
		}
		else
			CGroupEditBox::setProperty( name, value );
	}

	xmlNodePtr CGroupEditBoxDecor::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CGroupEditBox::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "edit_box_decor" );
		xmlSetProp( node, BAD_CAST "tx_tl", BAD_CAST _Pvt->_Textures[ EBDPrivate::TL ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_tr", BAD_CAST _Pvt->_Textures[ EBDPrivate::TR ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_tm", BAD_CAST _Pvt->_Textures[ EBDPrivate::TM ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_bl", BAD_CAST _Pvt->_Textures[ EBDPrivate::BL ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_bm", BAD_CAST _Pvt->_Textures[ EBDPrivate::BM ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_br", BAD_CAST _Pvt->_Textures[ EBDPrivate::BR ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_l", BAD_CAST _Pvt->_Textures[ EBDPrivate::L ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_r", BAD_CAST _Pvt->_Textures[ EBDPrivate::R ]->getTexture().c_str() );
		xmlSetProp( node, BAD_CAST "tx_bg", BAD_CAST _Pvt->_Textures[ EBDPrivate::BG ]->getTexture().c_str() );

		return node;
	}

	bool CGroupEditBoxDecor::parse( xmlNodePtr cur, CInterfaceGroup *parent )
	{
		if( !CGroupEditBox::parse( cur, parent ) )
			return false;

		CXMLAutoPtr prop;

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_tl" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::TL ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_tm" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::TM ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_tr" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::TR ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_bl" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::BL ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_bm" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::BM ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_br" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::BR ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_l" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::L ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_r" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::R ]->setTexture( ( const char* )prop );

		prop = ( char* ) xmlGetProp( cur, BAD_CAST "tx_bg" );
		if( prop )
			_Pvt->_Textures[ EBDPrivate::BG ]->setTexture( ( const char* )prop );

		return true;
	}

	void CGroupEditBoxDecor::draw()
	{
		CGroupEditBox::draw();

		_Pvt->draw();
	}

	void CGroupEditBoxDecor::updateCoords()
	{
		sint32 tw = _Pvt->_Textures[ EBDPrivate::L ]->getWReal();
		getVT()->setX( tw + 1 );

		CGroupEditBox::updateCoords();
		_Pvt->updateCoords();
	}

	void CGroupEditBoxDecor::forceLink()
	{
	}
}


