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

#ifndef MATERIAL_PROPERTIES_H
#define MATERIAL_PROPERTIES_H

#include "ui_material_properties.h"

namespace MaterialEditor
{

	class MatPropEditWidget;

	class MatPropWidget : public QWidget, public Ui::MatPropWidget
	{
		Q_OBJECT
	public:
		MatPropWidget( QWidget *parent = NULL );
		~MatPropWidget();

	private Q_SLOTS:
		void onOKClicked();
		void onAddClicked();
		void onEditClicked();
		void onRemoveClicked();

	private:
		void setupConnections();
		MatPropEditWidget *matPropEditWidget;
	};

}

#endif
