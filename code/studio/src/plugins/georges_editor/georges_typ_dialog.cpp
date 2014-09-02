#include "georges_typ_dialog.h"
#include "georges.h"

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

	return true;
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
	QString logMsg = buildLogMsg( msg );
	m_ui.logEdit->appendPlainText( logMsg );
}

