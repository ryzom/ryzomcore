// Ryzom Core Studio World Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "const_string_array_property.h"
#include "const_string_array_editor.h"
#include <QMap>
#include <QList>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>


////////////////////////////////////////////////////////////////// Manager ///////////////////////////////////////////////////////////////////////////


struct ConstStrArrPropMgrPriv
{
	QMap< const QtProperty*, QString > values;
};

ConstStrArrPropMgr::ConstStrArrPropMgr( QObject *parent ) :
QtAbstractPropertyManager( parent )
{
	d_ptr = new ConstStrArrPropMgrPriv();
}

ConstStrArrPropMgr::~ConstStrArrPropMgr()
{
	delete d_ptr;
	d_ptr = NULL;
}

QString ConstStrArrPropMgr::value( const QtProperty *p ) const
{
	return valueText( p );
}

void ConstStrArrPropMgr::setValue( QtProperty *p, const QString &value )
{
	if( !d_ptr->values.contains( p ) )
		return;

	if( d_ptr->values[ p ] == value )
		return;

	d_ptr->values[ p ] = value;

	Q_EMIT propertyChanged( p );
	Q_EMIT valueChanged( p, value );
}

void ConstStrArrPropMgr::setStrings( QtProperty *p, const QStringList &strings )
{
	Q_EMIT stringsChanged( p, strings );
}

bool ConstStrArrPropMgr::hasValue( const QtProperty *p ) const
{
	return d_ptr->values.contains( p );
}

QString ConstStrArrPropMgr::valueText( const QtProperty *p ) const
{
	if( !d_ptr->values.contains( p ) )
		return "";

	return d_ptr->values[ p ];
}

void ConstStrArrPropMgr::initializeProperty( QtProperty *p )
{
	if( d_ptr->values.contains( p ) )
		return;

	d_ptr->values[ p ] = "";
}

void ConstStrArrPropMgr::uninitializeProperty( QtProperty *p )
{
	d_ptr->values.remove( p );
}



//////////////////////////////////////////////////////////////////// Factory ///////////////////////////////////////////////////////////////////////



struct ConstStrArrEditorFactoryPriv
{
	QMap< QtProperty*, QList< ConstStrArrEditor* > > createdEditors;
	QMap< ConstStrArrEditor*, QtProperty* > editorToProperty;
	QMap< QtProperty*, QStringList > strings;

	~ConstStrArrEditorFactoryPriv()
	{
		createdEditors.clear();

		QMap< ConstStrArrEditor*, QtProperty* >::iterator itr = editorToProperty.begin();
		while( itr != editorToProperty.end() )
		{
			delete itr.key();
			++itr;
		}
		editorToProperty.clear();
	}

	void addEditor( QtProperty *p, ConstStrArrEditor *editor )
	{
		QMap< QtProperty*, QList< ConstStrArrEditor* > >::iterator itr = createdEditors.find( p );

		if( itr != createdEditors.end() )
		{
			itr->push_back( editor );
		}
		else
		{
			QList< ConstStrArrEditor* > l;
			l.push_back( editor );
			createdEditors.insert( p, l );
		}

		editorToProperty.insert( editor, p );
	}

	void removeEditor( QObject *o )
	{
		// Remove from editorToProperty first
		QMap< ConstStrArrEditor*, QtProperty* >::iterator itr1 = editorToProperty.begin();
		while( itr1 != editorToProperty.end() )
		{
			if( itr1.key() == o )
				break;

			++itr1;
		}
		if( itr1 != editorToProperty.end() )
			editorToProperty.erase( itr1 );

		// Then from createdEditors
		QMap< QtProperty*, QList< ConstStrArrEditor* > >::iterator itr2 = createdEditors.begin();
		while( itr2 != createdEditors.end() )
		{
			QList< ConstStrArrEditor* > &l = *itr2;
			QList< ConstStrArrEditor* >::iterator itr = l.begin();
			while( itr != l.end() )
			{
				if( *itr == o )
				{
					QList< ConstStrArrEditor* >::iterator d = itr;
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

ConstStrArrEditorFactory::ConstStrArrEditorFactory( QObject *parent ) :
QtAbstractEditorFactory( parent )
{
	d_ptr = new ConstStrArrEditorFactoryPriv();
}

ConstStrArrEditorFactory::~ConstStrArrEditorFactory()
{
	delete d_ptr;
	d_ptr = NULL;
}

void ConstStrArrEditorFactory::connectPropertyManager( ConstStrArrPropMgr *manager )
{
	connect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
	connect( manager, SIGNAL( stringsChanged( QtProperty*, const QStringList& ) ), this, SLOT( onStringsChanged( QtProperty*, const QStringList & ) ) );
}

void ConstStrArrEditorFactory::disconnectPropertyManager( ConstStrArrPropMgr *manager )
{
	disconnect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
	disconnect( manager, SIGNAL( stringsChanged( const QStringList& ) ), this, SLOT( onStringsChanged( const QStringList & ) ) );
}

QWidget* ConstStrArrEditorFactory::createEditor( ConstStrArrPropMgr *manager, QtProperty *p, QWidget *parent )
{
	ConstStrArrEditor *editor = new ConstStrArrEditor( parent );
	editor->setValue( manager->value( p ) );

	QMap< QtProperty*, QStringList >::iterator itr = d_ptr->strings.find( p );
	if( itr != d_ptr->strings.end() )
	{
		editor->setStrings( *itr );
	}

	connect( editor, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onSetValue( const QString& ) ) );
	connect( editor, SIGNAL( destroyed( QObject* ) ), this, SLOT( onEditorDestroyed( QObject* ) ) );

	d_ptr->addEditor( p, editor );

	return editor;
}

void ConstStrArrEditorFactory::onPropertyChanged( QtProperty *p, const QString &value )
{
	QMap< QtProperty*, QList< ConstStrArrEditor* > >::iterator itr = d_ptr->createdEditors.find( p );
	if( itr == d_ptr->createdEditors.end() )
		return;

	QList< ConstStrArrEditor* > &l = *itr;
	QList< ConstStrArrEditor* >::iterator i = l.begin();
	while( i != l.end() )
	{
		ConstStrArrEditor *editor = *i;
		
		editor->blockSignals( true );
		editor->setValue( value );
		editor->blockSignals( false );

		++i;
	}
}

void ConstStrArrEditorFactory::onStringsChanged( QtProperty *p, const QStringList &strings )
{
	if( p == NULL )
		return;

	d_ptr->strings[ p ] = strings;
}

void ConstStrArrEditorFactory::onSetValue( const QString &value )
{
	QObject *s = sender();
	ConstStrArrEditor *editor = qobject_cast< ConstStrArrEditor* >( s );
	if( editor == NULL )
		return;

	QMap< ConstStrArrEditor*, QtProperty* >::iterator itr = d_ptr->editorToProperty.find( editor );
	if( itr == d_ptr->editorToProperty.end() )
		return;

	QtProperty *p = *itr;
	
	ConstStrArrPropMgr *manager = qobject_cast< ConstStrArrPropMgr* >( p->propertyManager() );
	if( manager == NULL )
		return;

	blockSignals( true );
	manager->setValue( p, value );
	blockSignals( false );
}

void ConstStrArrEditorFactory::onEditorDestroyed( QObject *editor )
{
	d_ptr->removeEditor( editor );
}



//////////////////////////////////////////////////////////////////////// Editor //////////////////////////////////////////////////////////////////

ConstStrArrEditor::ConstStrArrEditor( QWidget *parent ) :
QWidget( parent )
{
	setupUi();
	setupConnections();
}

ConstStrArrEditor::~ConstStrArrEditor()
{
}

void ConstStrArrEditor::setStrings( const QStringList &strings )
{
	this->strings.clear();

	QStringListIterator itr( strings );
	while( itr.hasNext() )
	{
		this->strings.push_back( itr.next() );
	}
}

void ConstStrArrEditor::setValue( const QString &value )
{
	if( lineEdit->text() == value )
		return;

	disableConnections();
	lineEdit->setText( value );
	setupConnections();
}

void ConstStrArrEditor::showEvent( QShowEvent *e )
{
	QWidget::showEvent( e );
}

void ConstStrArrEditor::onToolButtonClicked()
{
	ConstStrArrEditDialog d;
	d.setStrings( strings );
	d.setValue( lineEdit->text() );
	int result = d.exec();
	if( QDialog::Accepted != result )
		return;
	lineEdit->setText( d.getValue() );
}

void ConstStrArrEditor::onTextChanged( const QString &text )
{
	Q_EMIT valueChanged( text );
}

void ConstStrArrEditor::setupConnections()
{
	connect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	connect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void ConstStrArrEditor::disableConnections()
{
	disconnect( toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onToolButtonClicked() ) );
	disconnect( lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onTextChanged( const QString& ) ) );
}

void ConstStrArrEditor::setupUi()
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


