#include "georges_typ_dialog.h"

GeorgesTypDialog::GeorgesTypDialog( QWidget *parent ) :
GeorgesDockWidget( parent )
{
	m_ui.setupUi( this );
	setupConnections();
}

GeorgesTypDialog::~GeorgesTypDialog()
{
}


void GeorgesTypDialog::write()
{
}

void GeorgesTypDialog::onAddClicked()
{
}

void GeorgesTypDialog::onRemoveClicked()
{
}

void GeorgesTypDialog::setupConnections()
{
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
}

void GeorgesTypDialog::log( const QString &msg )
{
}

