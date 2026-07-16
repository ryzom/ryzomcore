/* 
	file_description_container test

	project: RYZOM / TEST
*/

#include "nel/misc/file.h"
#include "game_share/file_description_container.h"
#include "game_share/singleton_registry.h"

using namespace std;
using namespace NLMISC;

class CFileDescriptionTest: public IServiceSingleton
{
public:
	void init()
	{
		CFileDescriptionContainer fdc;
		nlassert(fdc.empty());

		fdc.addFile("some file that doesn't exist 0");
		fdc.addFile("some file that doesn't exist 1",0,0);
		fdc.addFileSpec("some file that doesn't exist 2");
		nlassert(!fdc.empty());

		fdc.addFileSpec("*");

		for (uint32 i=0;i<fdc.size();++i)
		{
			nlinfo(fdc[i].FileName.c_str());
		}

		COFile outf;
		outf.open("file_description_container_test.bin");
		outf.serial(fdc);
		outf.close();

		nlassert(!fdc.empty());
		fdc.clear();
		nlassert(fdc.empty());

		CFileDescriptionContainer fdc2;
		CIFile inf;
		inf.open("file_description_container_test.bin");
		inf.serial(fdc2);
		inf.close();

		fdc2.display(NLMISC::InfoLog);
	}
};

static CFileDescriptionTest Test;
