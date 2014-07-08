// Ryzom Core Studio GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "texture_property_manager.h"
#include "texture_chooser.h"
#include <QMap>
#include <QList>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>


////////////////////////////////////////////////////////////////// Manager ///////////////////////////////////////////////////////////////////////////


struct TexturePropertyManagerPrivate
{
	QMap< const QtProperty*, QString > values;
};

TexturePropertyManager::TexturePropertyManager( QObject *parent ) :
QtAbstractPropertyManager( parent )
{
	d_ptr = new TexturePropertyManagerPrivate();
}

TexturePropertyManager::~TexturePropertyManager()
{
	delete d_ptr;
	d_ptr = NULL;
}

QString TexturePropertyManager::value( const QtProperty *p ) const
{
	return valueText( p );
}

void TexturePropertyManager::setValue( QtProperty *p, const QString &value )
{
	if( !d_ptr->values.contains( p ) )
		return;

	if( d_ptr->values[ p ] == value )
		return;

	d_ptr->values[ p ] = value;

	Q_EMIT propertyChanged( p );
	Q_EMIT valueChanged( p, value );
}

bool TexturePropertyManager::hasValue( const QtProperty *p ) const
{
	return d_ptr->values.contains( p );
}

QString TexturePropertyManager::valueText( const QtProperty *p ) const
{
	if( !d_ptr->values.contains( p ) )
		return "";

	return d_ptr->values[ p ];
}

void TexturePropertyManager::initializeProperty( QtProperty *p )
{
	if( d_ptr->values.contains( p ) )
		return;

	d_ptr->values[ p ] = "";
}

void TexturePropertyManager::uninitializeProperty( QtProperty *p )
{
	d_ptr->values.remove( p );
}



//////////////////////////////////////////////////////////////////// Factory ///////////////////////////////////////////////////////////////////////



struct TextureEditorFactoryPrivate
{
	QMap< QtProperty*, QList< TexturePropertyEditor* > > createdEditors;
	QMap< TexturePropertyEditor*, QtProperty* > editorToProperty;

	~TextureEditorFactoryPrivate()
	{
		createdEditors.clear();

		QMap< TexturePropertyEditor*, QtProperty* >::iterator itr = editorToProperty.begin();
		while( itr != editorToProperty.end() )
		{
			delete itr.key();
			++itr;
		}
		editorToProperty.clear();
	}

	void addEditor( QtProperty *p, TexturePropertyEditor *editor )
	{
		QMap< QtProperty*, QList< TexturePropertyEditor* > >::iterator itr = createdEditors.find( p );

		if( itr != createdEditors.end() )
		{
			itr->push_back( editor );
		}
		else
		{
			QList< TexturePropertyEditor* > l;
			l.push_back( editor );
			createdEditors.insert( p, l );
		}

		editorToProperty.insert( editor, p );
	}

	void removeEditor( QObject *o )
	{
		// Remove from editorToProperty first
		QMap< TexturePropertyEditor*, QtProperty* >::iterator itr1 = editorToProperty.begin();
		while( itr1 != editorToProperty.end() )
		{
			if( itr1.key() == o )
				break;

			++itr1;
		}
		if( itr1 != editorToProperty.end() )
			editorToProperty.erase( itr1 );

		// Then from createdEditors
		QMap< QtProperty*, QList< TexturePropertyEditor* > >::iterator itr2 = createdEditors.begin();
		while( itr2 != createdEditors.end() )
		{
			QList< TexturePropertyEditor* > &l = *itr2;
			QList< TexturePropertyEditor* >::iterator itr = l.begin();
			while( itr != l.end() )
			{
				if( *itr == o )
				{
					QList< TexturePropertyEditor* >::iterator d = itr;
					++itr;
					l.erase( d );
					continue;
				}

				++itr;
			}

			++itr2;
		}
	}

};

TextureEditorFactory::TextureEditorFactory( QObject *parent ) :
QtAbstractEditorFactory( parent )
{
	d_ptr = new TextureEditorFactoryPrivate();
}

TextureEditorFactory::~TextureEditorFactory()
{
	delete d_ptr;
	d_ptr = NULL;
}

void TextureEditorFactory::connectPropertyManager( TexturePropertyManager *manager )
{
	connect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

void TextureEditorFactory::disconnectPropertyManager( TexturePropertyManager *manager )
{
	disconnect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

QWidget* TextureEditorFactory::createEditor( TexturePropertyManager *manager, QtProperty *p, QWidget *parent )
{
	TexturePropertyEditor *editor = new TexturePropertyEditor( parent );
	editor->setValue( manager->value( p ) );

	connect( editor, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onSetValue( const QString& ) ) );
	connect( editor, SIGNAL( destroyed( QObject* ) ), this, SLOT( onEditorDestroyed( QObject* ) ) );

	d_ptr->addEditor( p, editor );

	return editor;
}

void TextureEditorFactory::onPropertyChanged( QtProperty *p, const QString &value )
{
	QMap< QtProperty*, QList< TexturePropertyEditor* > >::iterator itr = d_ptr->createdEditors.find( p );
	if( itr == d_ptr->createdEditors.end() )
		return;

	QList< TexturePropertyEditor* > &l = *itr;
	QList< TexturePropertyEditor* >::iterator i = l.begin();
	while( i != l.end() )
	{
		TexturePropertyEditor *editor = *i;
		
		editor->blockSignals( true );
		editor->setValue( value );
		editor->blockSignals( false );

		++i;
	}
}

void TextureEditorFactory::onSetValue( const QString &value )
{
	QObject *s = sender();
	TexturePropertyEditor *editor = qobject_cast< TexturePropertyEditor* >( s );
	if( editor == NULL )
		return;

	QMap< TexturePropertyEditor*, QtProperty* >::iterator itr = d_ptr->editorToProperty.find( editor );
	if( itr == d_ptr->editorToProperty.end() )
		return;

	QtProperty *p = *itr;
	
	TexturePropertyManager *manager = qobject_cast< TexturePropertyManager* >( p->propertyManager() );
	if( manager == NULL )
		return;

	blockSignals( true );
	manager->setValue( p, value );
	blockSignals( false );
}

void TextureEditorFactory::onEditorDestroyed( QObject *editor )
{
	d_ptr->removeEditor( editor );
}



//////////////////////////////////////////////////////////////////////// Editor //////////////////////////////////////////////////////////////////



TexturePropertyEditor::TexturePropertyEditor( QWidget *parent ) :
QWidget( parent )
{
	setupUi();
	setupConnections();
}

TexturePropertyEditor::~TexturePropertyEditor()
{
}

void TexturePropertyEditor::setValue( const QString &value )
{
	if( lineEdit->text() == value )
		return;

	disableConnections();
	lineEdit->setText( value );
	setupConnections();
}

void TexturePropertyEditor::showEvent( QShowEvent *e )
{
	QWidget::showEvent( e );
}

void TexturePropertyEditor::onToolButtonClicked()
{
	TextureChooser d;
	d.load();

	int result = d.exec();
	if( QDialog::Accepted != result )
		return;

	lineEdit->setText( d.getSelection() );
}

void TexturePropertyEditor::onTextChanged( const QString &text )
{
	Q_EMIT valueChanged( text );
}

void TexturePropertyEditor::setupConnections()
{
	connect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	connect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void TexturePropertyEditor::disableConnections()
{
	disconnect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	disconnect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void TexturePropertyEditor::setupUi()
{
	lineEdit = new QLineEdit();
	toolButton = new QToolButton();
	toolButton->setText( "..." );
	
	QHBoxLayout *lt = new QHBoxLayout( this );
	lt->setContentsMargins( 0, 0, 0, 0 );
	lt->setSpacing( 0 );
	lt->addWidget( lineEdit );
	lt->addWidget( toolButton );
	
	setFocusProxy( lineEdit );
	setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );

}


