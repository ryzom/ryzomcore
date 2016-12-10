// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef UT_MISC
#define UT_MISC

#include "ut_misc_co_task.h"
#include "ut_misc_command.h"
#include "ut_misc_common.h"
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
		add(CUniquePtr<Test::Suite>(new CUTMiscCoTask));
		add(CUniquePtr<Test::Suite>(new CUTMiscCommand));
		add(CUniquePtr<Test::Suite>(new CUTMiscCommon));
		add(CUniquePtr<Test::Suite>(new CUTMiscConfigFile));
		add(CUniquePtr<Test::Suite>(new CUTMiscDebug));
		add(CUniquePtr<Test::Suite>(new CUTMiscDynLibLoad));
		add(CUniquePtr<Test::Suite>(new CUTMiscFile));
		add(CUniquePtr<Test::Suite>(new CUTMiscPackFile));
		add(CUniquePtr<Test::Suite>(new CUTMiscSingleton));
		add(CUniquePtr<Test::Suite>(new CUTMiscSString));
		add(CUniquePtr<Test::Suite>(new CUTMiscStream));
		add(CUniquePtr<Test::Suite>(new CUTMiscVariable));
		add(CUniquePtr<Test::Suite>(new CUTMiscTypes));
		add(CUniquePtr<Test::Suite>(new CUTMiscStringCommon));
		// Add a line here when adding a new test CLASS
	}
};

#endif
