#include "dfn_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "3rdparty/qtpropertybrowser/qteditorfactory.h"
#include "3rdparty/qtpropertybrowser/qtpropertymanager.h"

#include "nel/georges/form_dfn.h"

namespace
{
	enum EntryEnum
	{
		ENTRY_TYPE,
		ENTRY_DFN,
		ENTRY_VIRTUAL_DFN,
		ENTRY_TYPE_ARRAY,
		ENTRY_DFN_ARRAY
	};

	int entryToEnum( const NLGEORGES::UFormDfn::TEntryType &type, bool isArray )
	{
		int id = 0;

		switch( type )
		{
		case NLGEORGES::UFormDfn::EntryType:
			
			if( isArray )
				id = ENTRY_TYPE_ARRAY;
			else
				id = ENTRY_TYPE;

			break;

		case NLGEORGES::UFormDfn::EntryDfn:
			if( isArray )
				id = ENTRY_DFN_ARRAY;
			else
				id = ENTRY_DFN;

			break;

		case NLGEORGES::UFormDfn::EntryVirtualDfn:
			id = ENTRY_VIRTUAL_DFN;
			break;
		}

		return id;
	}

}


DFNBrowserCtrl::DFNBrowserCtrl( QObject *parent ) :
QObject( parent )
{
	m_browser = NULL;
	m_dfn = NULL;
	m_manager = new QtVariantPropertyManager();
	m_factory = new QtVariantEditorFactory();
	m_enumMgr = new QtEnumPropertyManager();
	m_enumFactory = new QtEnumEditorFactory();
}

DFNBrowserCtrl::~DFNBrowserCtrl()
{
	m_browser = NULL;
	m_dfn = NULL;

	delete m_manager;
	m_manager = NULL;
	delete m_factory;
	m_factory = NULL;
	delete m_enumMgr;
	m_enumMgr = NULL;
	delete m_enumFactory;
	m_enumFactory = NULL;
}

void DFNBrowserCtrl::onElementSelected( int idx )
{
	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( idx );

	m_browser->clear();
	m_browser->setFactoryForManager( m_manager, m_factory );
	m_browser->setFactoryForManager( m_enumMgr, m_enumFactory );

	QtVariantProperty *p = NULL;
	QtProperty *prop = NULL;

	p = m_manager->addProperty( QVariant::String, "name" );
	p->setValue( entry.getName().c_str() );
	m_browser->addProperty( p );
	

	NLGEORGES::UFormDfn::TEntryType et = entry.getType();
	bool isArray = entry.getArrayFlag();

	QStringList options;
	options.push_back( "Type" );
	options.push_back( "DFN" );
	options.push_back( "Virtual DFN" );
	options.push_back( "Type Array" );
	options.push_back( "DFN Array" );

	int enumId = entryToEnum( et, isArray );
	
	prop = m_enumMgr->addProperty( "type" );
	m_enumMgr->setEnumNames( prop, options );
	m_enumMgr->setValue( prop, enumId );
	m_browser->addProperty( prop );


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



