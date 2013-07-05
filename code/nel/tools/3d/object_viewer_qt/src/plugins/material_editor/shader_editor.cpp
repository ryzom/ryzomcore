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

namespace MaterialEditor
{
	ShaderEditorWidget::ShaderEditorWidget( QDialog *parent ) :
	QDialog( parent )
	{
		setupUi( this );
		setupConnections();
	}

	ShaderEditorWidget::~ShaderEditorWidget()
	{
	}

	void ShaderEditorWidget::getName( QString &name )
	{
		name = nameEdit->text();
	}

	void ShaderEditorWidget::getDescription( QString &desc )
	{
		desc = descriptionEdit->toPlainText();
	}

	void ShaderEditorWidget::getVertexShader( QString &vs )
	{
		vs = vsEdit->toPlainText();
	}

	void ShaderEditorWidget::getFragmentShader( QString &fs )
	{
		fs = fsEdit->toPlainText();
	}

	void ShaderEditorWidget::setName( const QString &name )
	{
		nameEdit->setText( name );
	}

	void ShaderEditorWidget::setDescription( const QString &desc )
	{
		descriptionEdit->setPlainText( desc );
	}

	void ShaderEditorWidget::setVertexShader( const QString &vs )
	{
		vsEdit->setPlainText( vs );
	}

	void ShaderEditorWidget::setFragmentShader( const QString &fs )
	{
		fsEdit->setPlainText( fs );
	}

	void ShaderEditorWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) ) ;
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	}

	void ShaderEditorWidget::onOKClicked()
	{
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
}


