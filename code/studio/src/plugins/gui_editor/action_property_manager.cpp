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


#include "action_property_manager.h"
#include "action_list.h"
#include <QMap>
#include <QList>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>


////////////////////////////////////////////////////////////////// Manager ///////////////////////////////////////////////////////////////////////////


struct ActionPropertyManagerPrivate
{
	QMap< const QtProperty*, QString > values;
};

ActionPropertyManager::ActionPropertyManager( QObject *parent ) :
QtAbstractPropertyManager( parent )
{
	d_ptr = new ActionPropertyManagerPrivate();
}

ActionPropertyManager::~ActionPropertyManager()
{
	delete d_ptr;
	d_ptr = NULL;
}

QString ActionPropertyManager::value( const QtProperty *p ) const
{
	return valueText( p );
}

void ActionPropertyManager::setValue( QtProperty *p, const QString &value )
{
	if( !d_ptr->values.contains( p ) )
		return;

	if( d_ptr->values[ p ] == value )
		return;

	d_ptr->values[ p ] = value;

	Q_EMIT propertyChanged( p );
	Q_EMIT valueChanged( p, value );
}

bool ActionPropertyManager::hasValue( const QtProperty *p ) const
{
	return d_ptr->values.contains( p );
}

QString ActionPropertyManager::valueText( const QtProperty *p ) const
{
	if( !d_ptr->values.contains( p ) )
		return "";

	return d_ptr->values[ p ];
}

void ActionPropertyManager::initializeProperty( QtProperty *p )
{
	if( d_ptr->values.contains( p ) )
		return;

	d_ptr->values[ p ] = "";
}

void ActionPropertyManager::uninitializeProperty( QtProperty *p )
{
	d_ptr->values.remove( p );
}



//////////////////////////////////////////////////////////////////// Factory ///////////////////////////////////////////////////////////////////////



struct ActionEditorFactoryPrivate
{
	QMap< QtProperty*, QList< ActionPropertyEditor* > > createdEditors;
	QMap< ActionPropertyEditor*, QtProperty* > editorToProperty;

	~ActionEditorFactoryPrivate()
	{
		createdEditors.clear();

		QMap< ActionPropertyEditor*, QtProperty* >::iterator itr = editorToProperty.begin();
		while( itr != editorToProperty.end() )
		{
			delete itr.key();
			++itr;
		}
		editorToProperty.clear();
	}

	void addEditor( QtProperty *p, ActionPropertyEditor *editor )
	{
		QMap< QtProperty*, QList< ActionPropertyEditor* > >::iterator itr = createdEditors.find( p );

		if( itr != createdEditors.end() )
		{
			itr->push_back( editor );
		}
		else
		{
			QList< ActionPropertyEditor* > l;
			l.push_back( editor );
			createdEditors.insert( p, l );
		}

		editorToProperty.insert( editor, p );
	}

	void removeEditor( QObject *o )
	{
		// Remove from editorToProperty first
		QMap< ActionPropertyEditor*, QtProperty* >::iterator itr1 = editorToProperty.begin();
		while( itr1 != editorToProperty.end() )
		{
			if( itr1.key() == o )
				break;

			++itr1;
		}
		if( itr1 != editorToProperty.end() )
			editorToProperty.erase( itr1 );

		// Then from createdEditors
		QMap< QtProperty*, QList< ActionPropertyEditor* > >::iterator itr2 = createdEditors.begin();
		while( itr2 != createdEditors.end() )
		{
			QList< ActionPropertyEditor* > &l = *itr2;
			QList< ActionPropertyEditor* >::iterator itr = l.begin();
			while( itr != l.end() )
			{
				if( *itr == o )
				{
					QList< ActionPropertyEditor* >::iterator d = itr;
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

ActionEditorFactory::ActionEditorFactory( QObject *parent ) :
QtAbstractEditorFactory( parent )
{
	d_ptr = new ActionEditorFactoryPrivate();
}

ActionEditorFactory::~ActionEditorFactory()
{
	delete d_ptr;
	d_ptr = NULL;
}

void ActionEditorFactory::connectPropertyManager( ActionPropertyManager *manager )
{
	connect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

void ActionEditorFactory::disconnectPropertyManager( ActionPropertyManager *manager )
{
	disconnect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

QWidget* ActionEditorFactory::createEditor( ActionPropertyManager *manager, QtProperty *p, QWidget *parent )
{
	ActionPropertyEditor *editor = new ActionPropertyEditor( parent );
	editor->setValue( manager->value( p ) );

	connect( editor, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onSetValue( const QString& ) ) );
	connect( editor, SIGNAL( destroyed( QObject* ) ), this, SLOT( onEditorDestroyed( QObject* ) ) );

	d_ptr->addEditor( p, editor );

	return editor;
}

void ActionEditorFactory::onPropertyChanged( QtProperty *p, const QString &value )
{
	QMap< QtProperty*, QList< ActionPropertyEditor* > >::iterator itr = d_ptr->createdEditors.find( p );
	if( itr == d_ptr->createdEditors.end() )
		return;

	QList< ActionPropertyEditor* > &l = *itr;
	QList< ActionPropertyEditor* >::iterator i = l.begin();
	while( i != l.end() )
	{
		ActionPropertyEditor *editor = *i;
		
		editor->blockSignals( true );
		editor->setValue( value );
		editor->blockSignals( false );

		++i;
	}
}

void ActionEditorFactory::onSetValue( const QString &value )
{
	QObject *s = sender();
	ActionPropertyEditor *editor = qobject_cast< ActionPropertyEditor* >( s );
	if( editor == NULL )
		return;

	QMap< ActionPropertyEditor*, QtProperty* >::iterator itr = d_ptr->editorToProperty.find( editor );
	if( itr == d_ptr->editorToProperty.end() )
		return;

	QtProperty *p = *itr;
	
	ActionPropertyManager *manager = qobject_cast< ActionPropertyManager* >( p->propertyManager() );
	if( manager == NULL )
		return;

	blockSignals( true );
	manager->setValue( p, value );
	blockSignals( false );
}

void ActionEditorFactory::onEditorDestroyed( QObject *editor )
{
	d_ptr->removeEditor( editor );
}



//////////////////////////////////////////////////////////////////////// Editor //////////////////////////////////////////////////////////////////



ActionPropertyEditor::ActionPropertyEditor( QWidget *parent ) :
QWidget( parent )
{
	setupUi();
	setupConnections();
}

ActionPropertyEditor::~ActionPropertyEditor()
{
}

void ActionPropertyEditor::setValue( const QString &value )
{
	if( lineEdit->text() == value )
		return;

	disableConnections();
	lineEdit->setText( value );
	setupConnections();
}

void ActionPropertyEditor::showEvent( QShowEvent *e )
{
	QWidget::showEvent( e );
}

void ActionPropertyEditor::onToolButtonClicked()
{
	ActionList d;
	d.load();

	int result = d.exec();
	if( QDialog::Accepted != result )
		return;

	lineEdit->setText( d.getSelectedAction() );
}

void ActionPropertyEditor::onTextChanged( const QString &text )
{
	Q_EMIT valueChanged( text );
}

void ActionPropertyEditor::setupConnections()
{
	connect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	connect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void ActionPropertyEditor::disableConnections()
{
	disconnect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	disconnect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void ActionPropertyEditor::setupUi()
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


