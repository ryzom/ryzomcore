// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "prop_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "nel3d_interface.h"

#include <sstream>

#include <QMatrix4x4>

namespace MaterialEditor
{
	bool QStringToQMatrix4x4( const QString &s, QMatrix4x4 &m )
	{
		QString ms = s;
		bool ok = false;
		bool success = true;
		double da[ 16 ];

		QStringList sl = ms.split( " " );
		QStringListIterator it( sl );
		int i = 0;

		while( it.hasNext() )
		{
			double d = it.next().toDouble( &ok );

			if( ok )
			{
				da[ i ] = d;
			}
			else
			{
				da[ i ] = 0.0;
				success = false;
			}

			i++;
		}

		m = QMatrix4x4( da );

		return success;
	}


	int propToQVariant( unsigned char t )
	{
		int type = 0;

		switch( t )
		{
		case SMatProp::Color:
			type = QVariant::Color;
			break;
		
		case SMatProp::Double:
			type = QVariant::Double;
			break;
		
		case SMatProp::Float:
			type = QVariant::Double;
			break;
		
		case SMatProp::Int:
			type = QVariant::Int;
			break;
		
		case SMatProp::Matrix4:
			type = QVariant::String;
			break;
		
		case SMatProp::Texture:
			type = QVariant::String;
			break;
		
		case SMatProp::Uint:
			type = QVariant::Int;
			break;
		
		case SMatProp::Vector4:
			type = QVariant::String;
			break;
		
		default:
			type = QVariant::String;
			break;
		}

		return type;
	}

	void propValToQVariant( const SMatProp &p, QVariant &v )
	{
		bool ok = false;
		QString s;

		switch( p.type )
		{
		case SMatProp::Color:
			{
				std::stringstream ss = p.value;
				float c[ 4 ];
				std::fill( c, c + 4, 0.0f );

				for( int i = 0; i < 4; i++ )
				{
					ss >> c[ i ];
					if( !ss.good() )
						break;
				}

				QColor color;
				color.setRedF( c[ 0 ] / 255.0f );
				color.setGreenF( c[ 1 ] / 255.0f );
				color.setBlueF( c[ 2 ] / 255.0f );
				color.setAlphaF( c[ 3 ] / 255.0f );

				v = color;

				break;
			}
		
		case SMatProp::Double:
			double d;
			
			s = p.value.c_str();
			d = s.toDouble( &ok );
			if( ok )
				v = d;
			else
				v = 0.0;

			break;
		
		case SMatProp::Float:
			float f;

			s = p.value.c_str();
			f = s.toFloat( &ok );
			if( ok )
				v = f;
			else
				v = 0.0f;

			break;
		
		case SMatProp::Int:
			int i;

			s = p.value.c_str();
			i = s.toInt( &ok );
			if( ok )
				v = i;
			else
				v = 0;

			break;
		
		case SMatProp::Matrix4:
			{
				/*
				QMatrix4x4 m;
				m.fill( 0.0 );
				QStringToQMatrix4x4( p.value.c_str(), m );
				v = QVariant( m );
				*/
				v = p.value.c_str();
			}
			break;
		
		case SMatProp::Texture:
			v = p.value.c_str();
			break;
		
		case SMatProp::Uint:
			unsigned int u;

			s = p.value.c_str();
			u = s.toUInt( &ok );
			if( ok )
				v = u;
			else
				v = 0u;

			break;
		
		case SMatProp::Vector4:
			v = p.value.c_str();
			break;
		
		default:
			v = "";
			break;
		}
	}







	CPropBrowserCtrl::CPropBrowserCtrl( QObject *parent ) :
	QObject( parent )
	{
		browser = NULL;
		nel3dIface = NULL;
		manager = new QtVariantPropertyManager();
		factory = new QtVariantEditorFactory();

		setupConnections();
	}

	CPropBrowserCtrl::~CPropBrowserCtrl()
	{
		browser = NULL;
		nel3dIface = NULL;
		delete manager;
		manager = NULL;
		delete factory;
		factory = NULL;
	}
	
	void CPropBrowserCtrl::setBrowser( QtTreePropertyBrowser *b )
	{
		browser = b;
		browser->setFactoryForManager( manager, factory );
	}

	void CPropBrowserCtrl::setNel3DIface( CNel3DInterface *iface )
	{
		nel3dIface = iface;
	}

	void CPropBrowserCtrl::setupConnections()
	{
		connect( manager, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ),
			this, SLOT( onValueChanged( QtProperty*, const QVariant& ) ) );
	}

	void CPropBrowserCtrl::onSceneCleared()
	{
		clearProps();
	}

	void CPropBrowserCtrl::onPropsChanged()
	{
		clearProps();
		loadPropsForPass( currentPass );
	}

	void CPropBrowserCtrl::clearProps()
	{
		browser->clear();
		propToId.clear();
	}

	void CPropBrowserCtrl::loadPropsForPass( const QString &pass )
	{
		currentPass = pass;
		clearProps();

		if( pass.isEmpty() )
			return;

		CNelMaterialProxy m = nel3dIface->getMaterial();
		if( m.isEmpty() )
			return;

		CRenderPassProxy p = m.getPass( pass.toUtf8().data() );
		
		std::vector< SMatProp > v;
		p.getProperties( v );

		QtVariantProperty *vp = NULL;
		int type = 0;
		QVariant qv;

		std::vector< SMatProp >::const_iterator itr = v.begin();
		while( itr != v.end() )
		{
			const SMatProp &prop = *itr;

			type = propToQVariant( prop.type );
			
			vp = manager->addProperty( type, prop.label.c_str() );

			if( vp != NULL )
			{
				propValToQVariant( prop, qv );
				vp->setValue( qv );
				browser->addProperty( vp );
				propToId[ vp ] = prop.id;
			}
			++itr;
		}
	}

	void CPropBrowserCtrl::loadPropsForPass( int i )
	{
		clearProps();

		CNelMaterialProxy m = nel3dIface->getMaterial();
		if( m.isEmpty() )
			return;

		CRenderPassProxy p = m.getPass( i );

		std::string n;
		p.getName( n );
		currentPass = n.c_str();
		
		std::vector< SMatProp > v;
		p.getProperties( v );

		QtVariantProperty *vp = NULL;
		int type = 0;
		QVariant qv;

		std::vector< SMatProp >::const_iterator itr = v.begin();
		while( itr != v.end() )
		{
			const SMatProp &prop = *itr;

			type = propToQVariant( prop.type );
			
			vp = manager->addProperty( type, prop.label.c_str() );

			if( vp != NULL )
			{
				propValToQVariant( prop, qv );
				vp->setValue( qv );
				browser->addProperty( vp );
				propToId[ vp ] = prop.id;
			}
			++itr;
		}
	}

	void CPropBrowserCtrl::onValueChanged( QtProperty *p, const QVariant &v )
	{
		QString label = p->propertyName();
		std::string value = p->valueText().toUtf8().data();
		
		if( v.type() == QVariant::Color )
		{
			QColor c = v.value< QColor >();
			value.clear();
			QString val = "%1 %2 %3 %4";
			val = val.arg( c.red() ).arg( c.green() ).arg( c.blue() ).arg( c.alpha() );
			value = val.toUtf8().data();
		}

		CNelMaterialProxy m = nel3dIface->getMaterial();
		if( m.isEmpty() )
			return;

		CRenderPassProxy pass = m.getPass( currentPass.toUtf8().data() );

		std::map< QtProperty*, std::string >::const_iterator itr
			= propToId.find( p );
		if( itr == propToId.end() )
			return;

		SMatProp prop;
		bool ok = pass.getProperty( itr->second, prop );
		if( !ok )
			return;

		prop.value = value;
		pass.changeProperty( prop );
		

	}

}

