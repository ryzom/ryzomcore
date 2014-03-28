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


#ifndef LIGHTS_WIDGET_H
#define LIGHTS_WIDGET_H

#include "ui_lights_widget.h"

namespace MaterialEditor
{
	class CNel3DInterface;

	class LightsWidget : public QWidget, public Ui::LightsWidget
	{
		Q_OBJECT

	private:
		void setButtonColor( unsigned char butt, int r, int g, int b );

	public:
		LightsWidget( QWidget *parent = NULL );
		~LightsWidget();
		void setNL3DIface( CNel3DInterface *iface ){ this->iface = iface; }
		void loadValues();

	private Q_SLOTS:
		void onAmbButtonClicked();
		void onDiffButtonClicked();
		void onSpecButtonClicked();
		void onLightChanged( int light );
		void onChanges();

	private:
		void setupConnections();
		void setupChangeConnections();
		void disableChangeConnections();
		void loadLight( unsigned char light );
		void saveLight( unsigned char light );
		void applyChanges();

		CNel3DInterface *iface;

		enum LightType
		{
			Directional,
			Point,
			Spot
		};

		int buttonColors[ 3 ][ 3 ];
	};

}


#endif


