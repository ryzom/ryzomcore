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

#ifndef MATERIAL_PROPERTY_EDITOR_H
#define MATERIAL_PROPERTY_EDITOR_H

#include "ui_material_property_editor.h"

namespace MaterialEditor
{

	struct MaterialProperty
	{
		QString prop;
		QString label;
		QString type;

		bool operator==( const MaterialProperty &o )
		{
			if( o.prop != prop )
				return false;

			if( o.label != label )
				return false;

			if( o.type != type )
				return false;

			return true;
		}
	};

	class MatPropEditWidget : public QWidget, public Ui::MatPropEditWidget
	{
		Q_OBJECT
	public:
		MatPropEditWidget( QWidget *parent = NULL );
		~MatPropEditWidget();
		void getProperty( MaterialProperty &prop );
		void setProperty( const MaterialProperty &prop );
		void clear();

	Q_SIGNALS:
		void okClicked();

	private Q_SLOTS:
		void onOKClicked();
		void onCancelClicked();

	private:
		void setupConnections();
		
	};
}

#endif

