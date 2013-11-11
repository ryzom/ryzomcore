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

#include "lights_widget.h"
#include <QColorDialog>
#include "nel3d_interface.h"

namespace MaterialEditor
{
	enum LWLightTypes
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

	enum LWColorButton
	{
		AMBIENT,
		DIFFUSE,
		SPECULAR
	};

	void LightsWidget::setButtonColor( unsigned char butt, int r, int g, int b )
	{
		QString sh;
		QPushButton *button;

		if( butt > SPECULAR )
			return;

		switch( butt )
		{
		case AMBIENT:
			button = ambientButton;
			break;

		case DIFFUSE:
			button = diffuseButton;
			break;

		case SPECULAR:
			button = specularButton;
			break;
		}

		sh = QString( "background-color: rgb(%1, %2, %3);" ).arg( r ).arg( g ).arg( b );
		button->setStyleSheet( sh );

		buttonColors[ butt ][ 0 ] = r;
		buttonColors[ butt ][ 1 ] = g;
		buttonColors[ butt ][ 2 ] = b;
	}



	LightsWidget::LightsWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();

		typeCB->addItem( "Directional light" );
		typeCB->addItem( "Point light" );
		typeCB->addItem( "Spot light" );
	}

	LightsWidget::~LightsWidget()
	{
	}

	void LightsWidget::loadValues()
	{
		disableChangeConnections();

		unsigned char c = iface->getMaxLights();

		lightList->clear();

		for( unsigned char i = 0; i < c; i++ )
		{
			QString s = "light";
			s += QString::number( i );
			lightList->addItem( s );
		}

		lightList->setCurrentRow( 0 );
		loadLight( 0 );

		// loadLight enables it anyways
		//setupChangeConnections();
	}

	void LightsWidget::setupConnections()
	{
	}

	void LightsWidget::setupChangeConnections()
	{
		connect( enableCB, SIGNAL( toggled( bool ) ), this, SLOT( onChanges() ) );
		connect( ambientButton, SIGNAL( clicked( bool ) ), this, SLOT( onAmbButtonClicked() ) );
		connect( diffuseButton, SIGNAL( clicked( bool ) ), this, SLOT( onDiffButtonClicked() ) );
		connect( specularButton, SIGNAL( clicked( bool ) ), this, SLOT( onSpecButtonClicked() ) );
		connect( lightList, SIGNAL( currentRowChanged( int ) ), this, SLOT( onLightChanged( int ) ) );

		connect( typeCB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onChanges() ) );
		connect( xSB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		connect( ySB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		connect( zSB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		connect( constAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		connect( linearAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		connect( quadAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
	}

	void LightsWidget::disableChangeConnections()
	{
		disconnect( enableCB, SIGNAL( toggled( bool ) ), this, SLOT( onChanges() ) );
		disconnect( ambientButton, SIGNAL( clicked( bool ) ), this, SLOT( onAmbButtonClicked() ) );
		disconnect( diffuseButton, SIGNAL( clicked( bool ) ), this, SLOT( onDiffButtonClicked() ) );
		disconnect( specularButton, SIGNAL( clicked( bool ) ), this, SLOT( onSpecButtonClicked() ) );
		disconnect( lightList, SIGNAL( currentRowChanged( int ) ), this, SLOT( onLightChanged( int ) ) );

		disconnect( typeCB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onChanges() ) );
		disconnect( xSB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		disconnect( ySB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		disconnect( zSB, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		disconnect( constAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		disconnect( linearAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
		disconnect( quadAttnButton, SIGNAL( valueChanged( double ) ), this, SLOT( onChanges() ) );
	}

	void LightsWidget::onAmbButtonClicked()
	{
		QColor c = QColorDialog::getColor();
		setButtonColor( AMBIENT, c.red(), c.green(), c.blue() );
		applyChanges();
	}

	void LightsWidget::onDiffButtonClicked()
	{
		QColor c = QColorDialog::getColor();
		setButtonColor( DIFFUSE, c.red(), c.green(), c.blue() );
		applyChanges();
	}

	void LightsWidget::onSpecButtonClicked()
	{
		QColor c = QColorDialog::getColor();
		setButtonColor( SPECULAR, c.red(), c.green(), c.blue() );
		applyChanges();
	}

	void LightsWidget::onLightChanged( int light )
	{
		loadLight( light );
	}

	void LightsWidget::onChanges()
	{
		applyChanges();
	}

	void LightsWidget::loadLight( unsigned char light )
	{
		disableChangeConnections();

		SLightInfo info;
		iface->getLightInfo( light, info );

		if( info.enabled )
			enableCB->setChecked( true );
		else
			enableCB->setChecked( false );

		switch( info.type )
		{
		case SLightInfo::Directional:
			typeCB->setCurrentIndex( DIRECTIONAL );
			break;
		case SLightInfo::Point:
			typeCB->setCurrentIndex( POINT );
			break;
		case SLightInfo::Spot:
			typeCB->setCurrentIndex( SPOT );
			break;
		}

		xSB->setValue( info.posOrDir[ 0 ] );
		ySB->setValue( info.posOrDir[ 1 ] );
		zSB->setValue( info.posOrDir[ 2 ] );

		constAttnButton->setValue( info.constAttn );
		linearAttnButton->setValue( info.linAttn );
		quadAttnButton->setValue( info.quadAttn );

		setButtonColor( AMBIENT, info.ambColor[ 0 ] * 255.0f,
											info.ambColor[ 1 ] * 255.0f,
											info.ambColor[ 2 ] * 255.0f );

		setButtonColor( DIFFUSE, info.diffColor[ 0 ] * 255.0f,
											info.diffColor[ 1 ] * 255.0f,
											info.diffColor[ 2 ] * 255.0f );

		setButtonColor( SPECULAR, info.specColor[ 0 ] * 255.0f,
											info.specColor[ 1 ] * 255.0f,
											info.specColor[ 2 ] * 255.0f );

		setupChangeConnections();
	}

	void LightsWidget::saveLight( unsigned char light )
	{
		SLightInfo info;

		info.enabled = enableCB->isChecked();
		switch( typeCB->currentIndex() )
		{
		case DIRECTIONAL:
			info.type = SLightInfo::Directional;
			break;
		case POINT:
			info.type = SLightInfo::Point;
			break;
		case SPOT:
			info.type = SLightInfo::Spot;
			break;
		}

		info.posOrDir[ 0 ] = static_cast< float >( xSB->value() );
		info.posOrDir[ 1 ] = static_cast< float >( ySB->value() );
		info.posOrDir[ 2 ] = static_cast< float >( zSB->value() );

		info.constAttn = static_cast< float >( constAttnButton->value() );
		info.linAttn = static_cast< float >( linearAttnButton->value() );
		info.quadAttn = static_cast< float >( quadAttnButton->value() );

		info.ambColor[ 0 ] = buttonColors[ AMBIENT ][ 0 ] / 255.0f;
		info.ambColor[ 1 ] = buttonColors[ AMBIENT ][ 1 ] / 255.0f;
		info.ambColor[ 2 ] = buttonColors[ AMBIENT ][ 2 ] / 255.0f;
		info.diffColor[ 0 ] = buttonColors[ DIFFUSE ][ 0 ] / 255.0f;
		info.diffColor[ 1 ] = buttonColors[ DIFFUSE ][ 1 ] / 255.0f;
		info.diffColor[ 2 ] = buttonColors[ DIFFUSE ][ 2 ] / 255.0f;
		info.specColor[ 0 ] = buttonColors[ SPECULAR ][ 0 ] / 255.0f;
		info.specColor[ 1 ] = buttonColors[ SPECULAR ][ 1 ] / 255.0f;
		info.specColor[ 2 ] = buttonColors[ SPECULAR ][ 2 ] / 255.0f;

		iface->setLightInfo( light, info );
	}

	void LightsWidget::applyChanges()
	{
		int row = lightList->currentRow();
		saveLight( static_cast< unsigned char >( row ) );
	}

}

