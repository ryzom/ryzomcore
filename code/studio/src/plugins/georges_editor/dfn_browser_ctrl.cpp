#include "dfn_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "3rdparty/qtpropertybrowser/qteditorfactory.h"
#include "3rdparty/qtpropertybrowser/qtpropertymanager.h"

#include "filepath_property_manager.h"

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

	NLGEORGES::UFormDfn::TEntryType enumToEntry( int value )
	{
		NLGEORGES::UFormDfn::TEntryType entry = NLGEORGES::UFormDfn::EntryType;

		switch( value )
		{
		case ENTRY_TYPE: entry = NLGEORGES::UFormDfn::EntryType; break;
		case ENTRY_DFN: entry = NLGEORGES::UFormDfn::EntryDfn; break;
		case ENTRY_VIRTUAL_DFN: entry = NLGEORGES::UFormDfn::EntryVirtualDfn; break;
		case ENTRY_TYPE_ARRAY: entry = NLGEORGES::UFormDfn::EntryType; break;
		case ENTRY_DFN_ARRAY: entry = NLGEORGES::UFormDfn::EntryDfn; break;
		}

		return entry;
	}

	bool enumToArray( int value )
	{
		bool isArray = false;

		switch( value )
		{
		case ENTRY_TYPE_ARRAY:
		case ENTRY_DFN_ARRAY:
			isArray = true;
			break;
		}

		return isArray;
	}

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
	m_fileMgr = new FileManager();
	m_fileFactory = new FileEditFactory();

	m_idx = -1;
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
	delete m_fileMgr;
	m_fileMgr = NULL;
	delete m_fileFactory;
	m_fileFactory = NULL;
}

void DFNBrowserCtrl::onElementSelected( int idx )
{
	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( idx );
	m_idx = idx;

	disconnectManagers();

	m_browser->clear();
	m_browser->setFactoryForManager( m_manager, m_factory );
	m_browser->setFactoryForManager( m_enumMgr, m_enumFactory );
	m_browser->setFactoryForManager( m_fileMgr, m_fileFactory );

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


	prop = m_fileMgr->addProperty( "value" );
	m_fileMgr->setValue( prop, entry.getFilename().c_str() );
	m_browser->addProperty( prop );

	p = m_manager->addProperty( QVariant::String, "default" );
	p->setValue( entry.getDefault().c_str() );
	m_browser->addProperty( p );

	p = m_manager->addProperty( QVariant::String, "extension" );
	p->setValue( entry.getFilenameExt().c_str() );
	m_browser->addProperty( p );

	connectManagers();
}

void DFNBrowserCtrl::onVariantValueChanged( QtProperty *p, const QVariant &v )
{
	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( m_idx );

	QString key = p->propertyName();
	std::string value = v.toString().toUtf8().constData();

	if( key == "name" )
	{
		entry.setName( value.c_str() );
	}
	else
	if( key == "default" )
	{
		entry.setDefault( value.c_str() );
	}
	else
	if( key == "extension" )
	{
		entry.setFilenameExt( value.c_str() );
	}
}

void DFNBrowserCtrl::onEnumValueChanged( QtProperty *p, int v )
{
	NLGEORGES::UFormDfn::TEntryType tentry = enumToEntry( v );
	bool isArray = enumToArray( v );

	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( m_idx );
	entry.setArrayFlag( isArray );
	entry.setType( tentry );
}

void DFNBrowserCtrl::onFileValueChanged( QtProperty *p, const QString &v )
{
	NLGEORGES::CFormDfn::CEntry &entry = m_dfn->getEntry( m_idx );
	entry.setFilename( v.toUtf8().constData() );
}

void DFNBrowserCtrl::connectManagers()
{
	connect( m_manager, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ), this, SLOT( onVariantValueChanged( QtProperty*, const QVariant& ) ) );
	connect( m_enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ), this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );
	connect( m_fileMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onFileValueChanged( QtProperty*, const QString& ) ) );
}

void DFNBrowserCtrl::disconnectManagers()
{
	disconnect( m_manager, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ), this, SLOT( onVariantValueChanged( QtProperty*, const QVariant& ) ) );
	disconnect( m_enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ), this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );
	disconnect( m_fileMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ), this, SLOT( onFileValueChanged( QtProperty*, const QString& ) ) );
}



