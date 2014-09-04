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

#include "georges_typ_dialog.h"
#include "georges.h"
#include "typ_browser_ctrl.h"

#include <QInputDialog>
#include <QMessageBox>

#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/path.h"

class GeorgesTypDialogPvt
{
public:
	GeorgesTypDialogPvt()
	{
		typ = NULL;
		ctrl = new TypBrowserCtrl();
	}

	~GeorgesTypDialogPvt()
	{
		delete typ;
		typ = NULL;
		delete ctrl;
		ctrl = NULL;
	}


	NLGEORGES::CType *typ;
	TypBrowserCtrl *ctrl;
};

GeorgesTypDialog::GeorgesTypDialog( QWidget *parent ) :
GeorgesDockWidget( parent )
{
	m_ui.setupUi( this );
	m_pvt = new GeorgesTypDialogPvt();
	m_pvt->ctrl->setBrowser( m_ui.browser );

	setupConnections();
}

GeorgesTypDialog::~GeorgesTypDialog()
{
	delete m_pvt;
	m_pvt = NULL;
}


bool GeorgesTypDialog::load( const QString &fileName )
{
	GeorgesQt::CGeorges georges;
	NLGEORGES::UType *utyp = georges.loadFormType( fileName.toUtf8().constData() );
	if( utyp == NULL )
		return false;

	m_pvt->typ = dynamic_cast< NLGEORGES::CType* >( utyp );

	loadTyp();

	m_fileName = fileName;

	QFileInfo info( fileName );
	setWindowTitle( info.fileName() );

	return true;
}


void GeorgesTypDialog::write()
{
	NLMISC::COFile file;
	if( !file.open( m_fileName.toUtf8().constData(), false, true, false ) )
		return;

	NLMISC::COXml xml;
	xml.init( &file );
	
	m_pvt->typ->Header.Log = m_ui.logEdit->toPlainText().toUtf8().constData();
	m_pvt->typ->write( xml.getDocument() );
	
	xml.flush();
	file.close();

	setModified( false );
	setWindowTitle( windowTitle().remove( "*" ) );
}

void GeorgesTypDialog::newDocument( const QString &fileName )
{
	m_pvt->typ = new NLGEORGES::CType();
	m_fileName = fileName;

	QFileInfo info( fileName );
	setWindowTitle( info.fileName() + "*" );
	setModified( true );

	loadTyp();

	log( "Created" );
}

void GeorgesTypDialog::onAddClicked()
{
	QString label = QInputDialog::getText( this,
											tr( "Adding new definition" ),
											tr( "Please specify the label" ) );
	if( label.isEmpty() )
		return;

	QList< QTreeWidgetItem* > l = m_ui.tree->findItems( label, Qt::MatchExactly, 0 );
	if( !l.isEmpty() )
	{
		QMessageBox::information( this,
									tr( "Failed to add item" ),
									tr( "You can't add an item with the same label more than once!" ) );
		return;
	}

	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled );
	item->setText( 0, label );
	item->setText( 1, "" );
	m_ui.tree->addTopLevelItem( item );

	NLGEORGES::CType::CDefinition def;
	def.Label = label.toUtf8().constData();
	def.Value = "";
	m_pvt->typ->Definitions.push_back( def );

	log( "Added definition " + label );

	onModified();
}

void GeorgesTypDialog::onRemoveClicked()
{
	QTreeWidgetItem *item = m_ui.tree->currentItem();
	if( item == NULL )
		return;

	int i = 0;
	for( i = 0; i < m_ui.tree->topLevelItemCount(); i++ )
	{
		if( item == m_ui.tree->topLevelItem( i ) )
			break;
	}

	QString definition = item->text( 0 );

	m_ui.tree->takeTopLevelItem( i );
	delete item;

	std::vector< NLGEORGES::CType::CDefinition >::iterator itr = m_pvt->typ->Definitions.begin() + i;
	m_pvt->typ->Definitions.erase( itr );

	log( "Removed definition" + definition );

	onModified();
}

void GeorgesTypDialog::onItemChanged( QTreeWidgetItem *item, int column )
{
	int i = 0;
	for( i = 0; i < m_ui.tree->topLevelItemCount(); i++ )
	{
		if( item == m_ui.tree->topLevelItem( i ) )
			break;
	}
	
	NLGEORGES::CType::CDefinition &def = m_pvt->typ->Definitions[ i ];

	QString logMsg;
	logMsg = "Changed definition" + QString( def.Label.c_str() );
	
	if( i == 0 )
	{
		logMsg += ".label = ";
		logMsg += item->text( 0 );
		def.Label = item->text( 0 ).toUtf8().constData();
	}
	else
	{
		logMsg += ".value = ";
		logMsg += item->text( 1 );
		def.Value = item->text( 1 ).toUtf8().constData();
	}

	log( logMsg );

	onModified();
}

void GeorgesTypDialog::onModified()
{
	if( isModified() )
		return;

	setModified( true );
	setWindowTitle( windowTitle() + "*" );

	Q_EMIT modified();
}

void GeorgesTypDialog::onModified( const QString &k, const QString &v )
{
	log( "Changed " + k + " = " + v );
	onModified();
}

void GeorgesTypDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );

	connect( m_ui.tree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( onItemChanged( QTreeWidgetItem*, int ) ) );
	connect( m_pvt->ctrl, SIGNAL( modified( const QString&, const QString& ) ), this, SLOT( onModified( const QString&, const QString& ) ) );
}

void GeorgesTypDialog::log( const QString &msg )
{
	QString logMsg = buildLogMsg( msg );
	m_ui.logEdit->appendPlainText( logMsg );
}

void GeorgesTypDialog::loadTyp()
{
	m_ui.logEdit->setPlainText( m_pvt->typ->Header.Log.c_str() );
	m_ui.commentEdit->setPlainText( m_pvt->typ->Header.Comments.c_str() );

	std::vector< NLGEORGES::CType::CDefinition >::iterator itr = m_pvt->typ->Definitions.begin();
	while( itr != m_pvt->typ->Definitions.end() )
	{
		NLGEORGES::CType::CDefinition &def = *itr;

		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled );
		item->setText( 0, def.Label.c_str() );
		item->setText( 1, def.Value.c_str() );
		m_ui.tree->addTopLevelItem( item );

		++itr;
	}

	m_pvt->ctrl->setTyp( m_pvt->typ );
	m_pvt->ctrl->load();
}

