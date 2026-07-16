
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "src/cpptest.h"

using namespace std;

Test::Suite *createPrimitiveObjectTS(const std::string &workingPath);
Test::Suite *createModuleInterfaceTS();


// global test for any game share feature
class CGameShareTS : public Test::Suite
{
public:
	CGameShareTS(const std::string &workingPath)
	{
		add(auto_ptr<Test::Suite>(createPrimitiveObjectTS(workingPath)));
		add(auto_ptr<Test::Suite>(createModuleInterfaceTS()));

		// initialise the application context
		NLMISC::CApplicationContext::getInstance();

		NLMISC::createDebug();
	}
	
private:
};

//
//// register the misc test suite
//void registerNelMiscTestSuite(Test::Suite &mainTestSuite)
//{
//	mainTestSuite.add(auto_ptr<Test::Suite>(new CMiscTestSuite));
//}


auto_ptr<Test::Suite> intRegisterTestSuite(const std::string &workingPath)
{
	return static_cast<Test::Suite*>(new CGameShareTS(workingPath));
}

NL_LIB_EXPORT_SYMBOL(registerTestSuite, void, intRegisterTestSuite);

