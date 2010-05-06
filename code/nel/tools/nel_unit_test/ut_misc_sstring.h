#ifndef UT_MISC_SSTRING
#define UT_MISC_SSTRING

#include <nel/misc/sstring.h>

struct CUTMiscSString : public Test::Suite
{
	CUTMiscSString()
	{
		TEST_ADD(CUTMiscSString::testStrtok);
		// Add a line here when adding a new test METHOD
	}

	void testStrtok()
	{
		CSString testLine("  a=b  c   (a=e b=c) \t\t  c(a=e b=c) toto(bimbo(foo(bar(a=b))))");

		CSString part;
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "a=b");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "c");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "(a=e b=c)");
		part = testLine.strtok(" \t", true, false);
		TEST_ASSERT(part == "c");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "(a=e b=c)");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "toto");
		part = testLine.strtok(" \t=", true, false);
		TEST_ASSERT(part == "(bimbo(foo(bar(a=b))))");
		part = part.stripBlockDelimiters();
		CSString part2 = part.strtok(" \t=", true, false);
		TEST_ASSERT(part2 == "bimbo");
		part2 = part.strtok(" \t=", true, false);
		TEST_ASSERT(part2 == "(foo(bar(a=b)))");
	}
};

#endif
