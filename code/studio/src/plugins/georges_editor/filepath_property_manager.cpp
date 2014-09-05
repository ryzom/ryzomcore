// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

#include "filepath_property_manager.h"

#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMap>


///////////////////////////////////////////////////////////////////// Manager ////////////////////////////////////////////////////////////////////


class FileManagerPvt
{
public:
	QMap< const QtProperty*, QString > values;
};

FileManager::FileManager( QObject *parent ) :
QtAbstractPropertyManager( parent )
{
	m_pvt = new FileManagerPvt();
}

FileManager::~FileManager()
{
	delete m_pvt;
	m_pvt = NULL;
}

QString FileManager::value( const QtProperty *p ) const
{
	QMap< const QtProperty*, QString >::const_iterator itr = m_pvt->values.find( p );
	if( itr == m_pvt->values.end() )
		return "";
	else
		return itr.value();
}

void FileManager::setValue( QtProperty *p, const QString &v )
{
	if( !m_pvt->values.contains( p ) )
		return;

	if( m_pvt->values[ p ] == v )
		return;

	m_pvt->values[ p ] = v;

	Q_EMIT propertyChanged( p );
	Q_EMIT valueChanged( p, v );
}

bool FileManager::hasValue( const QtProperty *p ) const
{
	if( m_pvt->values.contains( p ) )
		return true;
	else
		return false;
}

QString FileManager::valueText( const QtProperty *p ) const
{
	return value( p );
}

void FileManager::initializeProperty( QtProperty *p )
{
	if( m_pvt->values.contains( p ) )
		return;

	m_pvt->values[ p ] = "";
}

void FileManager::uninitializeProperty( QtProperty *p )
{
	m_pvt->values.remove( p );
}

///////////////////////////////////////////////////////////////// Factory ///////////////////////////////////////////////////////////////////////////////

class FileEditFactoryPvt
{
public:
	QMap< QtProperty*, QList< FileEdit* > > createdEditors;
	QMap< FileEdit*, QtProperty* > editorToProperty;

	void addEditor( QtProperty *p, FileEdit *editor )
	{
		editorToProperty[ editor ] = p;

		QMap< QtProperty*, QList< FileEdit* > >::iterator itr = createdEditors.find( p );
		if( itr == createdEditors.end() )
		{
			QList< FileEdit* > l;
			l.push_back( editor );
			createdEditors[ p ] = l;
		}
		else
		{
			QList< FileEdit* > &l = itr.value();
			l.push_back( editor );
		}
	}

	void removeEditor( QObject *o )
	{
		// Find in editorToProperty
		QMap< FileEdit*, QtProperty* >::iterator itr = editorToProperty.begin();
		while( itr != editorToProperty.end() )
		{
			if( itr.key() == o )
				break;
			++itr;
		}

		// Store the property, and remove the editor from editorToProperty
		QtProperty *p = NULL;
		if( itr != editorToProperty.end() )
		{
			p = itr.value();
			editorToProperty.erase( itr );
		}

		// Find the property in createdEditors
		QMap< QtProperty*, QList< FileEdit* > >::iterator mitr = createdEditors.find( p );
		QList< FileEdit* > &l = mitr.value();

		// Find the editor in the list
		QList< FileEdit* >::iterator litr = l.begin();
		while( litr != l.end() )
		{
			if( o == *litr )
				break;
			litr++;
		}

		// Remove the editor and remove the list too if it's empty
		if( litr != l.end() )
			l.erase( litr );

		if( l.isEmpty() )
			createdEditors.erase( mitr );
	}
};


FileEditFactory::FileEditFactory( QObject *parent ) :
QtAbstractEditorFactory( parent )
{
	m_pvt = new FileEditFactoryPvt();
}

FileEditFactory::~FileEditFactory()
{
	delete m_pvt;
	m_pvt = NULL;
}

void FileEditFactory::connectPropertyManager( FileManager *manager )
{
	connect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
		this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

void FileEditFactory::disconnectPropertyManager( FileManager *manager )
{
	disconnect( manager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
		this, SLOT( onPropertyChanged( QtProperty*, const QString& ) ) );
}

QWidget* FileEditFactory::createEditor( FileManager *manager, QtProperty *p, QWidget *parent )
{
	FileEdit *editor = new FileEdit( parent );
	editor->setValue( p->valueText() );

	connect( editor, SIGNAL( valueChanged( const QString& ) ), this, SLOT( onSetValue( const QString& ) ) );
	connect( editor, SIGNAL( destroyed( QObject* ) ), this, SLOT( onEditorDestroyed( QObject* ) ) );

	m_pvt->addEditor( p, editor );

	return editor;
}

void FileEditFactory::onPropertyChanged( QtProperty *p, const QString &v )
{
	QMap< QtProperty*, QList< FileEdit* > >::iterator itr = m_pvt->createdEditors.find( p );
	if( itr == m_pvt->createdEditors.end() )
		return;

	QList< FileEdit* > &l = itr.value();
	QList< FileEdit* >::iterator litr = l.begin();
	while( litr != l.end() )
	{
		FileEdit *editor = *litr;
		editor->blockSignals( true );
		editor->setValue( v );
		editor->blockSignals( false );

		++litr;
	}
}

void FileEditFactory::onSetValue( const QString& v )
{
	QObject *s = sender();
	FileEdit *editor = qobject_cast< FileEdit* >( s );
	if( editor == NULL )
		return;

	QMap< FileEdit*, QtProperty* >::iterator itr = m_pvt->editorToProperty.find( editor );
	if( itr == m_pvt->editorToProperty.end() )
		return;

	QtProperty *p = *itr;
	FileManager *manager = qobject_cast< FileManager* >( p->propertyManager() );
	if( manager == NULL )
		return;

	blockSignals( true );
	manager->setValue( p, v );
	blockSignals( false );
}

void FileEditFactory::onEditorDestroyed( QObject *editor )
{
	m_pvt->removeEditor( editor );
}


//////////////////////////////////////////////////////////////// Editor /////////////////////////////////////////////////////////////////////////////////


class FileEditPvt
{
public:
	QLineEdit *lineEdit;
	QToolButton *toolButton;
};



FileEdit::FileEdit( QWidget *parent ) :
QWidget( parent )
{
	m_pvt = new FileEditPvt();

	setupUI();
	setupConnections();
}

FileEdit::~FileEdit()
{
	delete m_pvt;
	m_pvt = NULL;

	Q_EMIT destroyed( this );
}

void FileEdit::setValue( const QString &value )
{
	m_pvt->lineEdit->setText( value );
}

void FileEdit::onButtonClicked()
{
	QString file = QFileDialog::getOpenFileName( this,
													tr( "" ),
													tr( "" ) );
	if( file.isEmpty() )
		return;

	if( m_pvt->lineEdit->text() == file )
		return;

	m_pvt->lineEdit->setText( file );
	
	Q_EMIT valueChanged( file );
}

void FileEdit::setupUI()
{
	m_pvt->lineEdit = new QLineEdit( this );
	m_pvt->toolButton = new QToolButton( this );
	m_pvt->toolButton->setText( "..." );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->setSpacing( 0 );
	layout->addWidget( m_pvt->lineEdit );
	layout->addWidget( m_pvt->toolButton );
	setLayout( layout );

	setFocusProxy( m_pvt->lineEdit );
	setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed );
}

void FileEdit::setupConnections()
{
	connect( m_pvt->toolButton, SIGNAL( clicked( bool ) ), this, SLOT( onButtonClicked() ) );
}

