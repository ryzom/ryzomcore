/*
	Utils test

	project: RYZOM / TEST

*/

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "game_share/singleton_registry.h"
#include "game_share/utils.h"

void traceTest2(int i)
{
	CSTRACE;
	if (i<2)
		traceTest2(++i);
	else
		SHOW_CALLSTACK;
}

void traceTest()
{
	CSTRACE_MSG("foo");
	traceTest2(0);
	{
		int i=0;
		CSTRACE_VAR(int,i);
		CSTRACE_VAL(int,i);
		i=1;
		SHOW_CALLSTACK;
	}
	WARN_CALLSTACK;
}

class CUtilsTest: public IServiceSingleton
{
public:
	void init() 
	{
		// nel info, warning and debug redirectrion tests
		nldebug("debug"); nlinfo("info"); nlwarning("warning");
		{
			CNLLogOverride hold(NLMISC::DebugLog);
			nldebug("debug"); nlinfo("info"); nlwarning("warning");
		}
		{
			CNLLogOverride hold(NLMISC::InfoLog);
			nldebug("debug"); nlinfo("info"); nlwarning("warning");
		}
		{
			CNLLogOverride hold(NLMISC::WarningLog);
			nldebug("debug"); nlinfo("info"); nlwarning("warning");
		}
		nldebug("debug"); nlinfo("info"); nlwarning("warning");


		// handy vector method tests
		std::vector<sint64> vect;

		vectAppend(vect)= 0;	nlassert(vect.size()==1);	nlassert(vect[0]==0);
		vectAppend(vect)= 1;	nlassert(vect.size()==2);	nlassert(vect[1]==1);
		vectAppend(vect)= 2;	nlassert(vect.size()==3);	nlassert(vect[2]==2);

		vectInsert(vect,2);		nlassert(vect.size()==3);	nlassert(vect[0]+vect[1]+vect[2]==3);
		vectInsert(vect,4);		nlassert(vect.size()==4);	nlassert(vect[3]==4);

		// handy ptr class tests
		CTestClass tc;

		CTestClass* tcptr= &tc;
		CTestClass& tcref= tc; 
		IPtr<CTestClass> ptr;		
		ptr= tcptr;
		ptr= &tc;
		ptr= &tcref;
		++ptr;
		--ptr;
		tcptr= ptr.operator->();
		tcref= *ptr;
		nlassert(ptr==ptr);
		nlassert(!(ptr!=ptr));
		nlassert(ptr==tcptr);
		nlassert(!(ptr!=tcptr));

		CTestClass const* ctcptr= &tc; 
		CTestClass const& ctcref= tc; 
		IConstPtr<CTestClass> cptr;
		cptr= ctcptr;
		cptr= &tc;
		cptr= &ctcref;
		++cptr;
		--cptr;
		ctcptr= cptr.operator->();
//		ctcref= *cptr;
		nlassert(cptr==cptr);
		nlassert(!(cptr!=cptr));
		nlassert(cptr==ctcptr);
		nlassert(!(cptr!=ctcptr));

		// trace system test
		traceTest();
	}

	class CTestClass
	{
	public:
		int a,b;
	};
};

static CUtilsTest UtilsTest;

