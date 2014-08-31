#include "georges_dfn_dialog.h"
#include <QInputDialog>
#include <QMessageBox>

GeorgesDFNDialog::GeorgesDFNDialog( QWidget *parent ) :
QDockWidget( parent )
{
	m_ui.setupUi( this );
	setupConnections();

	m_ui.addButton->setEnabled( false );
	m_ui.removeButton->setEnabled( false );
}

GeorgesDFNDialog::~GeorgesDFNDialog()
{
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
}

void GeorgesDFNDialog::onRemoveClicked()
{
	int row = m_ui.list->currentRow();
	if( row < 0 )
		return;

	QListWidgetItem *item = m_ui.list->takeItem( row );
	delete item;
}

void GeorgesDFNDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
}

