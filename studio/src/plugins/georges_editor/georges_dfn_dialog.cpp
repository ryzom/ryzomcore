// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2010-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include "georges_dfn_dialog.h"
#include <QInputDialog>
#include <QMessageBox>

#include "georges.h"
#include "dfn_browser_ctrl.h"

#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/path.h"

class GeorgesDFNDialogPvt
{
public:
	GeorgesDFNDialogPvt()
	{
		dfn = NULL;
		ctrl = new DFNBrowserCtrl();
	}

	~GeorgesDFNDialogPvt()
	{
		delete ctrl;
		ctrl = NULL;
		delete dfn;
		dfn = NULL;
	}

	NLGEORGES::CFormDfn *dfn;
	DFNBrowserCtrl *ctrl;
};

GeorgesDFNDialog::GeorgesDFNDialog( QWidget *parent ) :
GeorgesDockWidget( parent )
{
	m_ui.setupUi( this );
	
	m_pvt = new GeorgesDFNDialogPvt();
	m_pvt->ctrl->setBrowser( m_ui.browser );

	setupConnections();
}

GeorgesDFNDialog::~GeorgesDFNDialog()
{
	delete m_pvt;
	m_pvt = NULL;
}

bool GeorgesDFNDialog::load( const QString &fileName )
{
	GeorgesQt::CGeorges georges;
	NLGEORGES::UFormDfn *udfn = georges.loadFormDfn( fileName.toUtf8().constData() );
	if( udfn == NULL )
		return false;

	QFileInfo info( fileName );
	setWindowTitle( info.fileName() );

	NLGEORGES::CFormDfn *cdfn = static_cast< NLGEORGES::CFormDfn* >( udfn );
	m_pvt->dfn = cdfn;
	
	loadDfn();

	m_fileName = fileName;

	connect(m_ui.commentsEdit, SIGNAL(textChanged()), this, SLOT(onCommentsEdited()));

	return true;
}

void GeorgesDFNDialog::write()
{
	setModified( false );
	setWindowTitle( windowTitle().remove( "*" ) );

	m_pvt->dfn->Header.Log = m_ui.logEdit->toPlainText().toUtf8().constData();
	m_pvt->dfn->Header.Comments = m_ui.commentsEdit->toPlainText().toUtf8().constData();

	NLMISC::COFile file;
	if( !file.open( m_fileName.toUtf8().constData(), false, true, false ) )
		return;

	NLMISC::COXml xml;
	xml.init( &file );

	m_pvt->dfn->write( xml.getDocument(), m_fileName.toUtf8().constData() );

	xml.flush();
	file.close();
}

void GeorgesDFNDialog::newDocument( const QString &fileName )
{
	m_fileName = fileName;
	QFileInfo info( fileName );
	setWindowTitle( info.fileName() + "*" );
	setModified( true );

	m_pvt->dfn = new NLGEORGES::CFormDfn();

	loadDfn();

	log( "Created" );
}

void GeorgesDFNDialog::onAddClicked()
{
	QString name = QInputDialog::getText( this,
											tr( "New element" ),
											tr( "Enter name of the new element" ) );

	QList< QListWidgetItem* > list = m_ui.list->findItems( name, Qt::MatchFixedString );
	if( !list.isEmpty() )
	{
		QMessageBox::information( this,
									tr( "Item already exists" ),
									tr( "That item already exists!" ) );
		return;
	}

	m_ui.list->addItem( name );
	m_pvt->dfn->addEntry( name.toUtf8().constData() );

	log( "Added " + name );

	onModified();
}

void GeorgesDFNDialog::onRemoveClicked()
{
	int row = m_ui.list->currentRow();
	if( row < 0 )
		return;

	log( "Removed " + m_ui.list->currentItem()->text() );

	QListWidgetItem *item = m_ui.list->takeItem( row );
	delete item;

	m_pvt->dfn->removeEntry( row );

	onModified();
}

void GeorgesDFNDialog::onCurrentRowChanged( int row )
{
	if( row < 0 )
		return;

	m_pvt->ctrl->onElementSelected( row );
}

void GeorgesDFNDialog::onValueChanged( const QString &key, const QString &value )
{
	onModified();

	log( m_ui.list->currentItem()->text() + "." + key + " = " + value );

	if( key == "name" )
	{
		m_ui.list->currentItem()->setText( value );
	}
}

void GeorgesDFNDialog::onCommentsEdited()
{
	onModified();
}

void GeorgesDFNDialog::loadDfn()
{
	m_pvt->ctrl->setDFN( m_pvt->dfn );

	uint c = m_pvt->dfn->getNumEntry();
	for( uint i = 0; i < c; i++ )
	{
		NLGEORGES::CFormDfn::CEntry &entry = m_pvt->dfn->getEntry( i );
		m_ui.list->addItem( entry.getName().c_str() );
	}

	if( c > 0 )
	{
		m_ui.list->setCurrentRow( 0 );
	}

	m_ui.commentsEdit->setPlainText( m_pvt->dfn->getComment().c_str() );
	m_ui.logEdit->setPlainText( m_pvt->dfn->Header.Log.c_str() );
}

void GeorgesDFNDialog::onModified()
{
	if( !isModified() )
	{
		setModified( true );
		setWindowTitle( windowTitle() + "*" );
		
		Q_EMIT modified();
	}
}

void GeorgesDFNDialog::log( const QString &msg )
{
	QString logMsg = buildLogMsg( msg );
	m_ui.logEdit->appendPlainText( logMsg );
}

void GeorgesDFNDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
	connect( m_ui.list, SIGNAL( currentRowChanged( int ) ), this, SLOT( onCurrentRowChanged( int ) ) );
	connect( m_pvt->ctrl, SIGNAL( valueChanged( const QString&, const QString& ) ), this, SLOT( onValueChanged( const QString&, const QString& ) ) );
}

