#ifndef UT_MISC
#define UT_MISC

using namespace NLMISC;

#include "ut_misc_co_task.h"
#include "ut_misc_command.h"
#include "ut_misc_config_file.h"
#include "ut_misc_debug.h"
#include "ut_misc_dynlibload.h"
#include "ut_misc_file.h"
#include "ut_misc_pack_file.h"
#include "ut_misc_singleton.h"
#include "ut_misc_sstring.h"
#include "ut_misc_stream.h"
#include "ut_misc_variable.h"
#include "ut_misc_types.h"
#include "ut_misc_string_common.h"
// Add a line here when adding a new test CLASS

struct CUTMisc : public Test::Suite
{
	CUTMisc()
	{
		add(auto_ptr<Test::Suite>(new CUTMiscCoTask));
		add(auto_ptr<Test::Suite>(new CUTMiscCommand));
		add(auto_ptr<Test::Suite>(new CUTMiscConfigFile));
		add(auto_ptr<Test::Suite>(new CUTMiscDebug));
		add(auto_ptr<Test::Suite>(new CUTMiscDynLibLoad));
		add(auto_ptr<Test::Suite>(new CUTMiscFile));
		add(auto_ptr<Test::Suite>(new CUTMiscPackFile));
		add(auto_ptr<Test::Suite>(new CUTMiscSingleton));
		add(auto_ptr<Test::Suite>(new CUTMiscSString));
		add(auto_ptr<Test::Suite>(new CUTMiscStream));
		add(auto_ptr<Test::Suite>(new CUTMiscVariable));
		add(auto_ptr<Test::Suite>(new CUTMiscTypes));
		add(auto_ptr<Test::Suite>(new CUTMiscStringCommon));
		// Add a line here when adding a new test CLASS
	}
};

#endif
