#include "georges_typ_dialog.h"
#include "georges.h"

#include <QInputDialog>
#include <QMessageBox>

class GeorgesTypDialogPvt
{
public:
	GeorgesTypDialogPvt()
	{
		typ = NULL;
	}

	~GeorgesTypDialogPvt()
	{
		delete typ;
		typ = NULL;
	}


	NLGEORGES::CType *typ;
};

GeorgesTypDialog::GeorgesTypDialog( QWidget *parent ) :
GeorgesDockWidget( parent )
{
	m_ui.setupUi( this );
	m_pvt = new GeorgesTypDialogPvt();
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

	return true;
}


void GeorgesTypDialog::write()
{
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

	m_ui.tree->takeTopLevelItem( i );
	delete item;

	std::vector< NLGEORGES::CType::CDefinition >::iterator itr = m_pvt->typ->Definitions.begin() + i;
	m_pvt->typ->Definitions.erase( itr );

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
	
	if( i == 0 )
		def.Label = item->text( 0 ).toUtf8().constData();
	else
		def.Value = item->text( 1 ).toUtf8().constData();
}

void GeorgesTypDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );

	connect( m_ui.tree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( onItemChanged( QTreeWidgetItem*, int ) ) );
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


}

