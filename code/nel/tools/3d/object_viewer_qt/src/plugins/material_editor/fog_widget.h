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


#ifndef FOG_WIDGET_H
#define FOG_WIDGET_H

#include "ui_fog_widget.h"

namespace MaterialEditor
{
	class CNel3DInterface;

	class FogWidget : public QWidget, public Ui::FogWidget
	{
		Q_OBJECT

	public:
		FogWidget( QWidget *parent = NULL );
		~FogWidget();

		void loadValues();

		void setNl3DIface( CNel3DInterface *iface ){ this->iface = iface; }

	private Q_SLOTS:
		void onFogCBClicked();
		void onStartSBChanged();
		void onEndSBChanged();
		void onColorButtonClicked();

	private:
		void setupConnections();
		void setColorButtonColor( int r, int g, int b );


		CNel3DInterface *iface;

	};
}

#endif

