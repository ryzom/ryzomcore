#ifndef UT_MISC_DYNLIBLOAD
#define UT_MISC_DYNLIBLOAD

#include <nel/misc/dynloadlib.h>

class CUTMiscDynLibLoad : public Test::Suite
{
public:
	CUTMiscDynLibLoad ()
	{
		TEST_ADD(CUTMiscDynLibLoad ::libraryNameDecoration)
	}

	void libraryNameDecoration()
	{
		string libName = "libmylib_with_dll_so_some_very_bad_rd_df_tag_inside_df";
		string fileName = "some/path/to/add/difficulties/"+NLMISC::CLibrary::makeLibName(libName);
		string cleanedName = NLMISC::CLibrary::cleanLibName(fileName);

		TEST_ASSERT(cleanedName == libName);
	}
};

#endif
