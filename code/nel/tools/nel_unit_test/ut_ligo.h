#ifndef UT_LIGO
#define UT_LIGO

#include <nel/ligo/primitive.h>

using namespace NLLIGO;

#include "ut_ligo_primitive.h"
// Add a line here when adding a new test CLASS

struct CUTLigo : public Test::Suite
{
	CUTLigo()
	{
		add(auto_ptr<Test::Suite>(new CUTLigoPrimitive));
		// Add a line here when adding a new test CLASS
	}
};

#endif
