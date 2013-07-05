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


#ifndef SHADER_EDITOR_H
#define SHADER_EDITOR_H

#include "ui_shader_editor.h"

namespace MaterialEditor
{
	class ShaderEditorWidget : public QDialog, public Ui::ShaderEditorWidget
	{
		Q_OBJECT
	public:
		ShaderEditorWidget( QDialog *parent = NULL );		
		~ShaderEditorWidget();

		void getName( QString &name );
		void getDescription( QString &desc );
		void getVertexShader( QString &vs );
		void getFragmentShader( QString &fs );

		void setName( const QString &name );
		void setDescription( const QString &desc );
		void setVertexShader( const QString &vs );
		void setFragmentShader( const QString &fs );

		void reset();

	private Q_SLOTS:
		void onOKClicked();
		void onCancelClicked();

	private:
		void setupConnections();
	};
}

#endif

