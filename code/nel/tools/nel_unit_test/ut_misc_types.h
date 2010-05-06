#ifndef UT_MISC_TYPES
#define UT_MISC_TYPES

#include <nel/misc/variable.h>

class CUTMiscTypes: public Test::Suite
{
public:
	CUTMiscTypes ()
	{
		TEST_ADD(CUTMiscTypes::basicTypes)
	}

	void basicTypes()
	{
		// this doesn't work on 64bit architectures
		//Test_ASSERT(sizeof(uint) == sizeof(void*));

		TEST_ASSERT(sizeof(sint8) == 1);
		TEST_ASSERT(sizeof(uint8) == 1);
		TEST_ASSERT(sizeof(sint16) == 2);
		TEST_ASSERT(sizeof(uint16) == 2);
		TEST_ASSERT(sizeof(sint32) == 4);
		TEST_ASSERT(sizeof(uint32) == 4);
		TEST_ASSERT(sizeof(sint64) == 8);
		TEST_ASSERT(sizeof(uint64) == 8);

		TEST_ASSERT(sizeof(sint) >= 4);
		TEST_ASSERT(sizeof(uint) >= 4);
	}
};

#endif
