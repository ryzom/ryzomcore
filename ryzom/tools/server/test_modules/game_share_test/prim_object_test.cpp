
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"
#include "nel/net/service.h"
#include "nel/ligo/ligo_config.h"
#include "src/cpptest.h"
#include "game_share/primitive_object.h"

#include <cstdlib>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;

// define some object to be linked with the primitive tree

class CCounterBase : public IPrimitiveObject
{
public:
	CCounterBase(const std::string &primitiveFileName, TAIAlias alias)
		: IPrimitiveObject(primitiveFileName, alias)
	{
		PreUpdateCount = 0;
		UpdateCount = 0;
		PostUpdateCount = 0;
		PreDeleteCount = 0;
		PreDeleteObjectCount = 0;
	}

	virtual void onPreUpdatePrimitiveFile()
	{
		++PreUpdateCount;
	}
	
	virtual void onUpdatePrimObject(NLLIGO::IPrimitive *prim, bool firstTime)
	{
		if (!prim->getPropertyByName("name", Name))
			throw "Can't read property 'name'";
		if (!prim->getPropertyByName("value", Value))
			throw "Can't read property 'value'";
		if (!((firstTime && UpdateCount == 0) || (!firstTime && UpdateCount > 0)))
			throw "Invalid 'firstTime' flag";

		++UpdateCount;
	}

	virtual void onPostUpdatePrimitiveFile()
	{
		++PostUpdateCount;
	}

	virtual void onPreDeleteObjects(const std::set<IPrimitiveObject*> &objectsList) 
	{
		++PreDeleteCount;
		PreDeleteObjectCount += objectsList.size();
	}

	string	Name;
	string	Value;
	uint	PreUpdateCount;
	uint	UpdateCount;
	uint	PostUpdateCount;
	uint	PreDeleteCount;
	uint	PreDeleteObjectCount;
};

// Type 1 object =========================================
class CType1 : public CCounterBase
{
	NL_INSTANCE_COUNTER_DECL(CType1);
public:
	CType1(const std::string &primitiveFileName, TAIAlias alias)
		: CCounterBase(primitiveFileName, alias)
	{
		nlinfo("Created CType1 \t@%p", this);
		ObjectIndex = Type1Objects.size();
		Type1Objects.push_back(this);
	}

	~CType1()
	{
		nlinfo("Deleted CType1 \t@%p", this);
		Type1Objects[ObjectIndex] = NULL;
	}


	virtual class IPrimitiveObjectDescriptor *getDescriptor();

	int		ObjectIndex;
	static	vector<CType1*>	Type1Objects;
};
NL_INSTANCE_COUNTER_IMPL(CType1);
vector<CType1*>	CType1::Type1Objects;

class CType1Descriptor : public IPrimitiveObjectDescriptor
{
public:
	CType1Descriptor() : IPrimitiveObjectDescriptor("type1") {}

	virtual IPrimitiveObject *create(const std::string &fileName, TAIAlias alias)
	{
		return new CType1(fileName, alias);
	}
	
};

CType1Descriptor	type1Descriptor;

IPrimitiveObjectDescriptor *CType1::getDescriptor()
{
	return &type1Descriptor;
}


// Type 2 mode 1 object =========================================
class CType2Mode1 : public CCounterBase
{
	NL_INSTANCE_COUNTER_DECL(CType2Mode1);
public:
	CType2Mode1(const std::string &primitiveFileName, TAIAlias alias)
		: CCounterBase(primitiveFileName, alias)
	{
		nlinfo("Created CType2Mode1 \t@%p", this);
		ObjectIndex = Type2Mode1Objects.size();
		Type2Mode1Objects.push_back(this);
	}

	~CType2Mode1()
	{
		nlinfo("Deleted CType2Mode1 \t@%p", this);
		Type2Mode1Objects[ObjectIndex] = NULL;
	}

	virtual class IPrimitiveObjectDescriptor *getDescriptor();

	int		ObjectIndex;
	static	vector<CType2Mode1*>	Type2Mode1Objects;
};
NL_INSTANCE_COUNTER_IMPL(CType2Mode1);
vector<CType2Mode1*>	CType2Mode1::Type2Mode1Objects;

class CType2Mode1Descriptor : public IPrimitiveObjectDescriptor
{
public:
	CType2Mode1Descriptor() : IPrimitiveObjectDescriptor("type2") {}

	virtual IPrimitiveObject *create(const std::string &fileName, TAIAlias alias)
	{
		return new CType2Mode1(fileName, alias);
	}

	bool additionnalFilter(NLLIGO::IPrimitive *prim) 
	{
		string mode;
		prim->getPropertyByName("mode", mode);
		if (mode == "mode1")
			return true;

		return false;
	}
	
};

CType2Mode1Descriptor	type2Mode1Descriptor;



IPrimitiveObjectDescriptor *CType2Mode1::getDescriptor()
{
	return &type2Mode1Descriptor;
}

// Type 2 mode 2 object =========================================
class CType2Mode2 : public CCounterBase
{
	NL_INSTANCE_COUNTER_DECL(CType2Mode2);
public:
	CType2Mode2(const std::string &primitiveFileName, TAIAlias alias)
		: CCounterBase(primitiveFileName, alias)
	{
		nlinfo("Created CType2Mode2 \t@%p", this);
		ObjectIndex = Type2Mode2Objects.size();
		Type2Mode2Objects.push_back(this);
	}

	~CType2Mode2()
	{
		nlinfo("Deleted CType2Mode2 \t@%p", this);
		Type2Mode2Objects[ObjectIndex] = NULL;
	}


	virtual class IPrimitiveObjectDescriptor *getDescriptor();

	int		ObjectIndex;
	static	vector<CType2Mode2*>	Type2Mode2Objects;
};
NL_INSTANCE_COUNTER_IMPL(CType2Mode2);
vector<CType2Mode2*>	CType2Mode2::Type2Mode2Objects;

class CType2Mode2Descriptor : public IPrimitiveObjectDescriptor
{
public:
	CType2Mode2Descriptor() : IPrimitiveObjectDescriptor("type2") {}

	virtual IPrimitiveObject *create(const std::string &fileName, TAIAlias alias)
	{
		return new CType2Mode2(fileName, alias);
	}
	
	bool additionnalFilter(NLLIGO::IPrimitive *prim) 
	{
		string mode;
		prim->getPropertyByName("mode", mode);
		if (mode == "mode2")
			return true;

		return false;
	}
};

CType2Mode2Descriptor	type2Mode2Descriptor;



IPrimitiveObjectDescriptor *CType2Mode2::getDescriptor()
{
	return &type2Mode2Descriptor;
}

// Type 3 object =========================================
class CType3 : public CCounterBase
{
	NL_INSTANCE_COUNTER_DECL(CType3);
public:
	CType3(const std::string &primitiveFileName, TAIAlias alias)
		: CCounterBase(primitiveFileName, alias)
	{
		nlinfo("Created CType3 \t@%p", this);
		ObjectIndex = Type3Objects.size();
		Type3Objects.push_back(this);
	}

	~CType3()
	{
		nlinfo("Deleted CType3 \t@%p", this);
		Type3Objects[ObjectIndex] = NULL;
	}


	virtual class IPrimitiveObjectDescriptor *getDescriptor();

	int		ObjectIndex;
	static	vector<CType3*>	Type3Objects;
};
NL_INSTANCE_COUNTER_IMPL(CType3);
vector<CType3*>	CType3::Type3Objects;

class CType3Descriptor : public IPrimitiveObjectDescriptor
{
public:
	CType3Descriptor() : IPrimitiveObjectDescriptor("poly_type") {}

	virtual IPrimitiveObject *create(const std::string &fileName, TAIAlias alias)
	{
		return new CType3(fileName, alias);
	}

};

CType3Descriptor	type3Descriptor;



IPrimitiveObjectDescriptor *CType3::getDescriptor()
{
	return &type3Descriptor;
}

// Type 3 object =========================================
class CType4 : public CCounterBase
{
	NL_INSTANCE_COUNTER_DECL(CType4);
public:
	CType4(const std::string &primitiveFileName, TAIAlias alias)
		: CCounterBase(primitiveFileName, alias)
	{
		nlinfo("Created CType4 \t@%p", this);
		ObjectIndex = Type4Objects.size();
		Type4Objects.push_back(this);
	}

	~CType4()
	{
		nlinfo("Deleted CType4 \t@%p", this);
		Type4Objects[ObjectIndex] = NULL;
	}


	virtual class IPrimitiveObjectDescriptor *getDescriptor();

	int		ObjectIndex;
	static	vector<CType4*>	Type4Objects;
};
NL_INSTANCE_COUNTER_IMPL(CType4);
vector<CType4*>	CType4::Type4Objects;

class CType4Descriptor : public IPrimitiveObjectDescriptor
{
public:
	CType4Descriptor() : IPrimitiveObjectDescriptor("poly_type") {}

	virtual IPrimitiveObject *create(const std::string &fileName, TAIAlias alias)
	{
		return new CType4(fileName, alias);
	}

};

CType4Descriptor	type4Descriptor;



IPrimitiveObjectDescriptor *CType4::getDescriptor()
{
	return &type4Descriptor;
}

// fake service class =========================================

class CFakeService : public IService
{

};

class CPrimitiveObjectTS: public Test::Suite
{
	string			_WorkingPath;
	string			_BaseDir;
	string			_CurrentPath;
	string			_CurrentPathToRestore;

	CFakeService	*_TheService;

public:
	CPrimitiveObjectTS(const std::string &workingPath)
	{
		_WorkingPath = workingPath;
		TEST_ADD(CPrimitiveObjectTS::initialLoading)
		TEST_ADD(CPrimitiveObjectTS::changedContent)
		TEST_ADD(CPrimitiveObjectTS::addedPrimitive)
		TEST_ADD(CPrimitiveObjectTS::removedContent)
		TEST_ADD(CPrimitiveObjectTS::removedPrimitive)
		TEST_ADD(CPrimitiveObjectTS::mutiObject)
		TEST_ADD(CPrimitiveObjectTS::fullCleanup)
	}

	void setup()     
	{
		_TheService = new CFakeService();
//		_BaseDir =     _WorkingPath+"/test_files";
		_BaseDir =     "test_files";
//		_CurrentPath = "R:/cvs_rework/code/ryzom/test_files/game_share_test";
		_CurrentPath = _BaseDir+"/game_share_test";

		// Set default directory
		_CurrentPathToRestore = CPath::getCurrentPath();
		CPath::setCurrentPath(_CurrentPath.c_str());

		_TheService->setArgs("prim_object_test -Zu"); 
//		_TheService.main("POT", "prim_object_test", 0, _CurrentPath.c_str(), _CurrentPath.c_str(), __DATE__" "__TIME__);
		_TheService->main("POT", "prim_object_test", 0, _WorkingPath.c_str(), _WorkingPath.c_str(), __DATE__" "__TIME__);

		// register ligo class factory
		static bool registered = false;
		if (!registered)
		{
			NLLIGO::Register();
			registered = true;
		}

		// init ligo
		CPrimitiveLoader::getInstance()->initLigoConfig("ligo_class.xml");

	}
	void tear_down() 
	{
		delete _TheService;
		// Restore default path
		CPath::setCurrentPath(_CurrentPathToRestore.c_str());
	}

	// Build the files content for the current step
	void installStepFiles(const std::string &prefix)
	{
		// cleanup the tmp directory
		vector<string> files;
//		CPath::getPathContent(_BaseDir+"/tmp", true, false, true, files);
		CPath::getPathContent("../tmp", true, false, true, files);
		while (!files.empty())
		{
			if (!CFile::deleteFile(files.back()))
				throw "Can not delete temp file !";
			files.pop_back();
		}

		CPath::getPathContent(".", false, false, true, files);

		for (uint i=0; i<files.size(); ++i)
		{
			string filename = CFile::getFilename(files[i]);
			if (filename.find(prefix) != string::npos)
			{
				string destName = filename.substr(prefix.size()+1);
				// We found a candidate for moving
				if (!CFile::copyFile((string("../tmp/")+destName).c_str(), files[i].c_str()))
					throw "Can't copy file !";
			}
		}
	}

	// step 1
	void initialLoading()
	{
		installStepFiles("step_1");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do varius check on created object & counters

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 3);
		TEST_ASSERT(CType1::Type1Objects.size() == 3);

		TEST_ASSERT(CType1::Type1Objects[0]->PreUpdateCount == 0);
		TEST_ASSERT(CType1::Type1Objects[0]->UpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[0]->PostUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteCount== 0);
		TEST_ASSERT(CType1::Type1Objects[0]->Name == "object1_type1");
		TEST_ASSERT(CType1::Type1Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteObjectCount == 0);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 0);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount== 0);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 0);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 0);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 0);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 0);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 1);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->Name == "object4_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0]->PreDeleteObjectCount == 0);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 1);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->Name == "object5_type2_mode2");
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0]->PreDeleteObjectCount == 0);

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 0);
	
		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 0);
	}

	// step 2
	void changedContent()
	{
		installStepFiles("step_2");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 3);
		TEST_ASSERT(CType1::Type1Objects.size() == 3);

		TEST_ASSERT(CType1::Type1Objects[0]->PreUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[0]->UpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[0]->PostUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[0]->Name == "object1_type1");
		TEST_ASSERT(CType1::Type1Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteObjectCount == 2);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "changed_value_1");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 2);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 2);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 2);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->getAlias() ==  5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Name == "object7_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteObjectCount == 0);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->getAlias() ==  6);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->Name == "object6_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreDeleteObjectCount == 0);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 1);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 0);
	
		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 0);
	}

	// step 3
	void addedPrimitive()
	{
		installStepFiles("step_3");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 4);
		TEST_ASSERT(CType1::Type1Objects.size() == 4);

		TEST_ASSERT(CType1::Type1Objects[0]->PreUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[0]->UpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[0]->PostUpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[0]->Name == "object1_type1");
		TEST_ASSERT(CType1::Type1Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[0]->PreDeleteObjectCount == 2);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "changed_value_1");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 2);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 1);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 2);

		TEST_ASSERT(CType1::Type1Objects[3]->PreUpdateCount == 0);
		TEST_ASSERT(CType1::Type1Objects[3]->UpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[3]->PostUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[3]->PreDeleteCount== 0);
		TEST_ASSERT(CType1::Type1Objects[3]->Name == "object8_type1");
		TEST_ASSERT(CType1::Type1Objects[3]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[3]->PreDeleteObjectCount == 0);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->getAlias() ==  5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->UpdateCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PostUpdateCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Name == "object7_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteObjectCount == 0);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->getAlias() ==  6);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->UpdateCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PostUpdateCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->Name == "object6_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2]->PreDeleteObjectCount == 0);

		TAIAlias alias = CType2Mode1::Type2Mode1Objects[3]->getAlias();
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->getAlias() ==  CPrimitiveLoader::getInstance()->getLigoConfig().buildAlias(1, 2));
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->Name == "object9_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3]->PreDeleteObjectCount == 0);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 2);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->getAlias() ==  CPrimitiveLoader::getInstance()->getLigoConfig().buildAlias(1, 3));
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->PreUpdateCount == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->UpdateCount == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->PostUpdateCount == 1);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->PreDeleteCount == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->Name == "object10_type2_mode2");
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1]->PreDeleteObjectCount == 0);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 0);
	
		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 0);
	}

	// step 4
	void removedContent()
	{
		installStepFiles("step_4");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 3);
		TEST_ASSERT(CType1::Type1Objects.size() == 4);

		TEST_ASSERT(CType1::Type1Objects[0] == NULL);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount == 3);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "changed_value_1");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 6);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 3);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 3);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 6);

		TEST_ASSERT(CType1::Type1Objects[3]->PreUpdateCount == 1);
		TEST_ASSERT(CType1::Type1Objects[3]->UpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[3]->PostUpdateCount == 2);
		TEST_ASSERT(CType1::Type1Objects[3]->PreDeleteCount == 2);
		TEST_ASSERT(CType1::Type1Objects[3]->Name == "object8_type1");
		TEST_ASSERT(CType1::Type1Objects[3]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[3]->PreDeleteObjectCount == 4);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->getAlias() ==  5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreUpdateCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->UpdateCount == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PostUpdateCount == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteCount == 2);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Name == "object7_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteObjectCount == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3] == NULL);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 2);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 0);
	
		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 0);
	}

	// step 5
	void removedPrimitive()
	{
		installStepFiles("step_5");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 2);
		TEST_ASSERT(CType1::Type1Objects.size() == 4);

		TEST_ASSERT(CType1::Type1Objects[0] == NULL);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount == 4);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "changed_value_1");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 7);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 4);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 4);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 7);

		TEST_ASSERT(CType1::Type1Objects[3] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->getAlias() ==  5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreUpdateCount == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->UpdateCount == 4);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PostUpdateCount == 4);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteCount == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Name == "object7_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteObjectCount == 5);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3] == NULL);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 2);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 0);
	
		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 0);
	}

	// step 6
	void mutiObject()
	{
		installStepFiles("step_6");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 2);
		TEST_ASSERT(CType1::Type1Objects.size() == 4);

		TEST_ASSERT(CType1::Type1Objects[0] == NULL);

		TEST_ASSERT(CType1::Type1Objects[1]->PreUpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[1]->UpdateCount == 6);
		TEST_ASSERT(CType1::Type1Objects[1]->PostUpdateCount == 6);
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteCount == 4);
		TEST_ASSERT(CType1::Type1Objects[1]->Name == "object2_type1");
		TEST_ASSERT(CType1::Type1Objects[1]->Value == "changed_value_1");
		TEST_ASSERT(CType1::Type1Objects[1]->PreDeleteObjectCount == 7);

		TEST_ASSERT(CType1::Type1Objects[2]->PreUpdateCount == 5);
		TEST_ASSERT(CType1::Type1Objects[2]->UpdateCount == 6);
		TEST_ASSERT(CType1::Type1Objects[2]->PostUpdateCount == 6);
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteCount== 4);
		TEST_ASSERT(CType1::Type1Objects[2]->Name == "object3_type1");
		TEST_ASSERT(CType1::Type1Objects[2]->Value == "initial_value");
		TEST_ASSERT(CType1::Type1Objects[2]->PreDeleteObjectCount == 7);

		TEST_ASSERT(CType1::Type1Objects[3] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 1);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->getAlias() ==  5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreUpdateCount == 4);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->UpdateCount == 5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PostUpdateCount == 5);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteCount == 3);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Name == "object7_type2_mode1");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->Value == "initial_value");
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1]->PreDeleteObjectCount == 5);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3] == NULL);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 2);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 1);
		TEST_ASSERT(CType3::Type3Objects.size() == 1);
	
		TEST_ASSERT(CType3::Type3Objects[0]->getAlias() ==  6);
		TEST_ASSERT(CType3::Type3Objects[0]->PreUpdateCount == 0);
		TEST_ASSERT(CType3::Type3Objects[0]->UpdateCount == 1);
		TEST_ASSERT(CType3::Type3Objects[0]->PostUpdateCount == 1);
		TEST_ASSERT(CType3::Type3Objects[0]->PreDeleteCount == 0);
		TEST_ASSERT(CType3::Type3Objects[0]->Name == "object8_poly_type");
		TEST_ASSERT(CType3::Type3Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType3::Type3Objects[0]->PreDeleteObjectCount == 0);

		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 1);
		TEST_ASSERT(CType4::Type4Objects.size() == 1);
	
		TEST_ASSERT(CType4::Type4Objects[0]->getAlias() ==  6);
		TEST_ASSERT(CType4::Type4Objects[0]->PreUpdateCount == 0);
		TEST_ASSERT(CType4::Type4Objects[0]->UpdateCount == 1);
		TEST_ASSERT(CType4::Type4Objects[0]->PostUpdateCount == 1);
		TEST_ASSERT(CType4::Type4Objects[0]->PreDeleteCount == 0);
		TEST_ASSERT(CType4::Type4Objects[0]->Name == "object8_poly_type");
		TEST_ASSERT(CType4::Type4Objects[0]->Value == "initial_value");
		TEST_ASSERT(CType4::Type4Objects[0]->PreDeleteObjectCount == 0);
	
	}

	// step 7
	void fullCleanup()
	{
		installStepFiles("step_7");

		// Cleanup search path
		CPath::clearMap();

		// Add the primitive cfg in the search path
		CPath::addSearchFile("../tmp/primitives.cfg");

		// And load the primitives
		CPrimitiveObjectManager::getInstance()->updatePrimitives();

		// Do various checks on objects & counters

		uint32 counter = NL_GET_INSTANCE_COUNTER(CType1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType1) == 0);
		TEST_ASSERT(CType1::Type1Objects.size() == 4);

		TEST_ASSERT(CType1::Type1Objects[0] == NULL);

		TEST_ASSERT(CType1::Type1Objects[1] == NULL);

		TEST_ASSERT(CType1::Type1Objects[2] == NULL);

		TEST_ASSERT(CType1::Type1Objects[3] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType2Mode1);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode1) == 0);
		TEST_ASSERT(CType2Mode1::Type2Mode1Objects.size() == 4);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[0] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[1] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[2] == NULL);

		TEST_ASSERT(CType2Mode1::Type2Mode1Objects[3] == NULL);

		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType2Mode2) == 0);
		TEST_ASSERT(CType2Mode2::Type2Mode2Objects.size() == 2);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[0] == NULL);

		TEST_ASSERT(CType2Mode2::Type2Mode2Objects[1] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType3);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType3) == 0);
		TEST_ASSERT(CType3::Type3Objects.size() == 1);
	
		TEST_ASSERT(CType3::Type3Objects[0] == NULL);

		counter = NL_GET_INSTANCE_COUNTER(CType4);
		TEST_ASSERT(NL_GET_INSTANCE_COUNTER(CType4) == 0);
		TEST_ASSERT(CType4::Type4Objects.size() == 1);
	
		TEST_ASSERT(CType4::Type4Objects[0] == NULL);
	
	}
};


Test::Suite *createPrimitiveObjectTS(const std::string &workingPath)
{
	return static_cast<Test::Suite*>(new CPrimitiveObjectTS(workingPath));
}

