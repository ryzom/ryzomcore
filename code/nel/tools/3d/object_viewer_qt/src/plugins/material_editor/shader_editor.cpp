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


#include "shader_editor.h"
#include "nel3d_interface.h"

namespace MaterialEditor
{
	ShaderEditorWidget::ShaderEditorWidget( QDialog *parent ) :
	QDialog( parent )
	{
		setupUi( this );
		nl3dIface = NULL;
		setupConnections();
	}

	ShaderEditorWidget::~ShaderEditorWidget()
	{
	}

	void ShaderEditorWidget::getDescription( QString &desc )
	{
		desc = descriptionEdit->toPlainText();
	}

	void ShaderEditorWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) ) ;
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	}

	void ShaderEditorWidget::onOKClicked()
	{
		SShaderInfo info;
		info.name = nameEdit->text().toUtf8().data();
		info.description = descriptionEdit->toPlainText().toUtf8().data();
		info.vp = vsEdit->toPlainText().toUtf8().data();
		info.fp = fsEdit->toPlainText().toUtf8().data();

		bool ok = nl3dIface->updateShaderInfo( info );
		if( ok )
			nl3dIface->saveShader( info.name );

		accept();
	}

	void ShaderEditorWidget::onCancelClicked()
	{
		reject();
	}

	void ShaderEditorWidget::reset()
	{
		QString empty;
		nameEdit->setText( empty );
		descriptionEdit->setPlainText( empty );
		vsEdit->setPlainText( empty );
		fsEdit->setPlainText( empty );
		setResult( QDialog::Rejected );
	}

	bool ShaderEditorWidget::load( const QString &name )
	{
		SShaderInfo info;
		if( !nl3dIface->getShaderInfo( name.toUtf8().data(), info ) )
			return false;

		nameEdit->setText( info.name.c_str() );
		descriptionEdit->setPlainText( info.description.c_str() );
		vsEdit->setPlainText( info.vp.c_str() );
		fsEdit->setPlainText( info.fp.c_str() );
		return true;
	}
}


