
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"
#include "nel/net/service.h"

#include "src/cpptest.h"

#include "game_share/mysql_wrapper.h"

#include "test_mapping.h"

#include <cstdlib>
#include <mysql.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace TM;


class CNopeTS: public Test::Suite
{
	string			_WorkingPath;
//	string			_BaseDir;
//	string			_CurrentPath;
//	string			_CurrentPathToRestore;

	// database connection
	MSW::CConnection _Conn;

	uint32			_RootId;

//	NLMISC::CApplicationContext	*_Context;

public:

	CNopeTS(const std::string &workingPath)
	{

		NLMISC::CPath::setCurrentPath((workingPath+"/test_files/shard_unifier_test").c_str());

		// connect to the database
		MYSQL *mysql;
		mysql = mysql_init(NULL);
		
		nlverify(mysql_real_connect(mysql, "borisb2", NULL, NULL, NULL, 0,NULL,0) == mysql);
		const char *error = mysql_error(mysql);
		
		// build the database and table needed for the test
		nlverify(mysql_query(mysql, "DROP DATABASE test") == 0);
		nlverify(mysql_query(mysql, "CREATE DATABASE test") == 0);

		nlverify(mysql_select_db(mysql, "test") == 0);


		nlverify(mysql_query(mysql, "CREATE TABLE `map_child` ( \
			`Id` int(10) unsigned NOT NULL default '0',\
			`parent_id` int(10) unsigned NOT NULL default '0',\
			PRIMARY KEY  (`Id`)\
			) TYPE=MyISAM;") == 0);
		nlverify(mysql_query(mysql, "CREATE TABLE `one_child` (\
					  `Id` int(10) unsigned NOT NULL default '0',\
					PRIMARY KEY  (`Id`)\
					) TYPE=MyISAM;") == 0);
		nlverify(mysql_query(mysql, "CREATE TABLE `root_table` (\
					`Id` int(10) unsigned NOT NULL auto_increment,\
					PRIMARY KEY  (`Id`)\
					) TYPE=MyISAM;") == 0);
		nlverify(mysql_query(mysql, "CREATE TABLE `vector_child` (\
					`Id` int(10) unsigned NOT NULL default '0',\
					`parent_id` int(10) unsigned NOT NULL default '0',\
					`info` int(10) NOT NULL default '0',\
					PRIMARY KEY  (`Id`)\
					) TYPE=MyISAM;") == 0);

	
		_WorkingPath = workingPath;
		TEST_ADD(CNopeTS::store)
		TEST_ADD(CNopeTS::load)
		TEST_ADD(CNopeTS::storeOneChild)
		TEST_ADD(CNopeTS::loadOneChild)
		TEST_ADD(CNopeTS::badOneChild)
		TEST_ADD(CNopeTS::removeOneChild)
		TEST_ADD(CNopeTS::storeVectorChild)
		TEST_ADD(CNopeTS::loadVectorChild)
		TEST_ADD(CNopeTS::getVectorChildByIndex)
		TEST_ADD(CNopeTS::removeVectorChild)
		TEST_ADD(CNopeTS::storeMapChild)
		TEST_ADD(CNopeTS::loadMapChild)
		TEST_ADD(CNopeTS::removeMapChild)
		TEST_ADD(CNopeTS::onDelete)

	}

	~CNopeTS()
	{
		// terminate cleanly the embedded database server
//		mysql_server_end();

		CPath::setCurrentPath(_WorkingPath.c_str());

//		delete _Context;

	}

	void setup()
	{
		NLMISC::CPath::setCurrentPath((_WorkingPath+"/test_files/shard_unifier_test").c_str());

//		_Conn.addOption(MYSQL_READ_DEFAULT_GROUP, "libmysqld_client");
//		_Conn.addOption(MYSQL_OPT_USE_EMBEDDED_CONNECTION, NULL);

		nlassert(_Conn.connect("borisb2", "", "", "test"));

		uint32 cacheSize = NOPE::CPersistentCache::getInstance().getInstanceCount();
	}

	void tear_down()
	{
		_Conn.closeConn();

		// clear the object cache
		NOPE::CPersistentCache::getInstance().clearCache();
	}

	void onDelete()
	{
		// create a root element, then add some childs, delete the root and check the state or existence
		// of child depending on 'on-delete' specified behavior

		CRootTablePtr rt = CRootTable::createTransient(__FILE__, __LINE__);
		TEST_ASSERT(rt->create(_Conn));

		uint32 rootId = rt->getObjectId();

		CMapChildPtr	mapChilds[10];
		CVectorChildPtr	vectorChilds[10];
		COneChildPtr	oneChild;
		// create some childs
		for (uint i=0; i<10; ++i)
		{
			mapChilds[i] = CMapChild::createTransient(__FILE__, __LINE__);
			mapChilds[i]->setParentId(rootId);
			mapChilds[i]->setObjectId(i+1000);
			TEST_ASSERT(mapChilds[i]->create(_Conn));
		}
		for (uint i=0; i<10; ++i)
		{
			vectorChilds[i] = CVectorChild::createTransient(__FILE__, __LINE__);
			vectorChilds[i]->setParentId(rootId);
			vectorChilds[i]->setObjectId(i+1000);
			TEST_ASSERT(vectorChilds[i]->create(_Conn));
		}
		oneChild = COneChild::createTransient(__FILE__, __LINE__);
		oneChild->setObjectId(rootId);
		oneChild->create(_Conn);

		// ok, now delete the root and check the childs
		TEST_ASSERT(rt->remove(_Conn));

		TEST_ASSERT(oneChild->getPersistentState() == NOPE::os_removed);
		for (uint i =0; i<10; ++i)
		{
			TEST_ASSERT(mapChilds[i]->getObjectId() == i+1000);
			TEST_ASSERT(mapChilds[i]->getPersistentState() == NOPE::os_removed);
		}
		for (uint i =0; i<10; ++i)
		{
			TEST_ASSERT(vectorChilds[i]->getObjectId() == i+1000);
			TEST_ASSERT(vectorChilds[i]->getPersistentState() == NOPE::os_clean);
			TEST_ASSERT(vectorChilds[i]->getParentId() == 0);
		}


	}

	void removeMapChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		TEST_ASSERT(rt->loadMapChilds(_Conn, __FILE__, __LINE__));

		TEST_ASSERT(rt->getMapChildsById(5) != NULL);
		TEST_ASSERT(rt->getMapChildsById(5)->remove(_Conn));

		TEST_ASSERT(rt->getMapChildsById(5) == NULL);
		TEST_ASSERT(rt->getMapChilds().size() == 9);

	}

	void loadMapChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		TEST_ASSERT(rt->loadMapChilds(_Conn, __FILE__, __LINE__));

		TEST_ASSERT(rt->getMapChilds().size() == 10);
	}

	void storeMapChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);
		rt->loadMapChilds(_Conn, __FILE__, __LINE__);

		// create 10 element in the map childs
		for (uint i=0; i<10; ++i)
		{
			CMapChildPtr mc = CMapChild::createTransient(__FILE__, __LINE__);

			mc->setObjectId(i+1);
			mc->setParentId(rt->getObjectId());

			TEST_ASSERT(mc->create(_Conn));
			TEST_ASSERT(rt->getMapChilds().size() == i+1);
			TEST_ASSERT(rt->getMapChildsById(mc->getObjectId()) == mc);
		}
	}

	void removeVectorChild()
	{
		// remove one element in the middle of the vector

		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		// load the vector childs
		TEST_ASSERT(rt->loadVectorChilds(_Conn, __FILE__, __LINE__));

		TEST_ASSERT(rt->getVectorChildsByIndex(5)->remove(_Conn));
		TEST_ASSERT(rt->getVectorChilds().size() == 9);
	}

	void getVectorChildByIndex()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		// load the vector childs
		TEST_ASSERT(rt->loadVectorChilds(_Conn, __FILE__, __LINE__));

		TEST_ASSERT(rt->getVectorChildsById(15) == NULL);
		TEST_ASSERT(rt->getVectorChildsById(5) != NULL);

	}

	void loadVectorChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		// load the vector childs
		TEST_ASSERT(rt->loadVectorChilds(_Conn, __FILE__, __LINE__));

		// check the number of elements
		TEST_ASSERT(rt->getVectorChilds().size() == 10);
	}

	void storeVectorChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);
		// load the vector childs (this load nothing as there are no child for now)
		rt->loadVectorChilds(_Conn, __FILE__, __LINE__);

		// create some vector child
		for (uint i=0; i<10; ++i)
		{
			CVectorChildPtr vc = CVectorChild::createTransient(__FILE__, __LINE__);

			vc->setObjectId(i+1);
			vc->setParentId(rt->getObjectId());
			vc->setInfo(sint32(i));

			TEST_ASSERT(vc->create(_Conn));

			TEST_ASSERT(rt->getVectorChilds().size() == i+1);
			TEST_ASSERT(rt->getVectorChilds().back() == vc);
		}
	}

	void removeOneChild()
	{
		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);
		TEST_ASSERT(rt != NULL);
		// load the child
		TEST_ASSERT(rt->loadOneChild(_Conn, __FILE__, __LINE__));

		// remove the child from the persistent store
		TEST_ASSERT(rt->getOneChild()->remove(_Conn));

		// check that the parent link is updated
		TEST_ASSERT(rt->getOneChild() == NULL);
	}

	void badOneChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);
		// load the child
		rt->loadOneChild(_Conn, __FILE__, __LINE__);

		// create a new one child
		COneChildPtr oc = COneChild::createTransient(__FILE__, __LINE__);
		oc->setObjectId(rt->getObjectId());

		// this must fail (either during insert (if db support constraint) or after, when updating the root table)
		TEST_ASSERT(oc->create(_Conn) == false);
	}

	void loadOneChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);
		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		{
			// load the child
			COneChildPtr oc = COneChild::load(_Conn, rt->getObjectId(), __FILE__, __LINE__);

			TEST_ASSERT(oc != NULL);
//			TEST_ASSERT(rt->getOneChild() == NULL);
		}

		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 1);

		rt->loadOneChild(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(rt->getOneChild() != NULL);
		TEST_ASSERT(rt->getOneChild()->getObjectId() == rt->getObjectId());
	
	}

	void storeOneChild()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		// load the root element
		CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

		// load the child (in fact, this load nothing because there is no child for now)
		rt->loadOneChild(_Conn, __FILE__, __LINE__);

		TEST_ASSERT(rt->getOneChild() == NULL);

		// create the one child relation element
		COneChildPtr oc = COneChild::createTransient(__FILE__, __LINE__);
		
		oc->setObjectId(rt->getObjectId());

		// store the element
		TEST_ASSERT(oc->create(_Conn));

		// check that the parent relation have been updated
		TEST_ASSERT(rt->getOneChild() == oc)
	}

	void load()
	{
		clearCache();
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);

		{
			CRootTablePtr rt = CRootTable::load(_Conn, _RootId, __FILE__, __LINE__);

			TEST_ASSERT(rt != NULL);
			TEST_ASSERT(rt->getObjectId() == _RootId);
		}
		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 1);
		
		NOPE::CPersistentCache::getInstance().clearCache();

		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);
	}

	void store()
	{
		// make sure the cache is clean
		clearCache();
		
		{
			CRootTablePtr rt = CRootTable::createTransient(__FILE__, __LINE__);

			TEST_ASSERT(rt->create(_Conn));
			_RootId = rt->getObjectId();
		}

		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 1);

		NOPE::CPersistentCache::getInstance().clearCache();

		TEST_ASSERT(NOPE::CPersistentCache::getInstance().getInstanceCount() == 0);
	}

	void	clearCache()
	{
		
		CVectorChild::clearCache();
			NOPE::CPersistentCache::getInstance().clearCache();
	}

};

Test::Suite *createCNopeTS(const std::string &workingPath)
{
	return new CNopeTS(workingPath);
}


