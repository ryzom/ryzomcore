#include "dfn_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"

#include "nel/georges/form_dfn.h"

DFNBrowserCtrl::DFNBrowserCtrl( QObject *parent ) :
QObject( parent )
{
	m_browser = NULL;
	m_dfn = NULL;

	m_manager = new QtVariantPropertyManager();
	m_factory = new QtVariantEditorFactory();
}

DFNBrowserCtrl::~DFNBrowserCtrl()
{
	m_browser = NULL;
	m_dfn = NULL;

	delete m_manager;
	m_manager = NULL;
	delete m_factory;
	m_factory = NULL;
}

void DFNBrowserCtrl::onElementSelected( int idx )
{
	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( idx );

	m_browser->clear();
	m_browser->setFactoryForManager( m_manager, m_factory );

	QtVariantProperty *p = NULL;

	p = m_manager->addProperty( QVariant::String, "name" );
	p->setValue( entry.getName().c_str() );
	m_browser->addProperty( p );
	

	NLGEORGES::UFormDfn::TEntryType et = entry.getType();
	bool isArray = entry.getArrayFlag();
	QString type = "";

	switch( et )
	{
	case NLGEORGES::UFormDfn::EntryType: type = "type"; break;
	case NLGEORGES::UFormDfn::EntryDfn: type = "DFN"; break;
	case NLGEORGES::UFormDfn::EntryVirtualDfn: type = "Virtual DFN"; break;
	}

	if( isArray )
		type += " array";

	p = m_manager->addProperty( QVariant::String, "type" );
	p->setValue( type );
	m_browser->addProperty( p );

	p = m_manager->addProperty( QVariant::String, "value" );
	p->setValue( entry.getFilename().c_str() );
	m_browser->addProperty( p );

	p = m_manager->addProperty( QVariant::String, "default" );
	p->setValue( entry.getDefault().c_str() );
	m_browser->addProperty( p );

	p = m_manager->addProperty( QVariant::String, "extension" );
	p->setValue( entry.getFilenameExt().c_str() );
	m_browser->addProperty( p );
}



