#include "nel/gui/db_manager.h"

namespace NLGUI
{
	CDBManager* CDBManager::instance = NULL;

	CDBManager::CDBManager() :
	NLMISC::CCDBManager( "ROOT", 0 )
	{
	}

	CDBManager::~CDBManager()
	{
	}

	CDBManager* CDBManager::getInstance()
	{
		if( instance == NULL )
			instance = new CDBManager();
		return instance;
	}

	void CDBManager::release()
	{
		nlassert( instance != NULL );
		delete instance;
		instance = NULL;
	}

	NLMISC::CCDBNodeLeaf* CDBManager::getDbProp( const std::string &name, bool create )
	{
		return getDbLeaf( name, create );
	}

	void CDBManager::delDbProp( const std::string &name )
	{
		delDbNode( name );
	}

	sint32 CDBManager::getDbValue32( const std::string &name )
	{
		NLMISC::CCDBNodeLeaf *node = getDbProp( name, false );
		if( node != NULL )
			return node->getValue32();
		else
			return 0;
	}

	NLMISC::CCDBNodeBranch* CDBManager::getDB() const
	{
		return _Database;
	}
}