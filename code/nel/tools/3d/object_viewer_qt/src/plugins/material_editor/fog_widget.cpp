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


#include "fog_widget.h"
#include "nel3d_interface.h"
#include <QColorDialog>

namespace MaterialEditor
{
	FogWidget::FogWidget( QWidget *parent ) :
	QWidget( parent )
	{
		iface = NULL;
		setupUi( this );
		setupConnections();
	}

	FogWidget::~FogWidget()
	{
		iface = NULL;
	}

	void FogWidget::loadValues()
	{
		SFogSettings s;
		iface->getFogSettings( s );

		fogCB->setChecked( s.enable );
		startSB->setValue( s.start );
		endSB->setValue( s.end );
		setColorButtonColor( s.color[ 0 ], s.color[ 1 ], s.color[ 2 ] );
	}

	void FogWidget::setupConnections()
	{
		connect( fogCB, SIGNAL( clicked( bool ) ), this, SLOT( onFogCBClicked() ) );
		connect( startSB, SIGNAL( valueChanged( double ) ), this, SLOT( onStartSBChanged() ) );
		connect( endSB, SIGNAL( valueChanged( double ) ), this, SLOT( onEndSBChanged() ) );
		connect( colorButton, SIGNAL( clicked( bool ) ), this, SLOT( onColorButtonClicked() ) );
	}

	void FogWidget::onFogCBClicked()
	{
		SFogSettings s;
		iface->getFogSettings( s );

		s.enable = fogCB->isChecked();

		iface->setFogSettings( s );

		if( !s.enable )
			iface->setBGColor( 255, 255, 255, 255 );
	}

	void FogWidget::onStartSBChanged()
	{
		SFogSettings s;
		iface->getFogSettings( s );

		s.start = startSB->value();

		iface->setFogSettings( s );
	}

	void FogWidget::onEndSBChanged()
	{
		SFogSettings s;
		iface->getFogSettings( s );

		s.end = endSB->value();

		iface->setFogSettings( s );
	}

	void FogWidget::onColorButtonClicked()
	{
		QColor c = QColorDialog::getColor();

		setColorButtonColor( c.red(), c.green(), c.blue() );

		SFogSettings s;
		iface->getFogSettings( s );
		
		s.color[ 0 ] = c.red();
		s.color[ 1 ] = c.green();
		s.color[ 2 ] = c.blue();
		s.color[ 3 ] = 255;

		iface->setFogSettings( s );
	}

	void FogWidget::setColorButtonColor( int r, int g, int b )
	{
		QString sh;
		sh = QString( "background-color: rgb(%1, %2, %3);" ).arg( r ).arg( g ).arg( b );
		colorButton->setStyleSheet( sh );
	}

}



