#ifndef UT_MISC_VARIABLE
#define UT_MISC_VARIABLE

#include <nel/misc/variable.h>

class CUTMiscVariable : public Test::Suite
{
public:
	CUTMiscVariable ()
	{
		TEST_ADD(CUTMiscVariable ::declareVar)
	}

	void declareVar()
	{
		{
			NLMISC::CVariable<std::string> myLocalVar("test", "myLocalVar", "no help", "");

			TEST_ASSERT(myLocalVar.get() == string(""));
			TEST_ASSERT(NLMISC::CCommandRegistry::getInstance().execute("myLocalVar foo", (*NLMISC::InfoLog)));
			TEST_ASSERT(myLocalVar.get() == string("foo"));
		}

		TEST_ASSERT(!NLMISC::CCommandRegistry::getInstance().execute("myLocalVar foo", (*NLMISC::InfoLog)));
	}
};

#endif
