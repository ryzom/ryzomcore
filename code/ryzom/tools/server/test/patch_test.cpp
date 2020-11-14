/*
	Patch generation test

	project: RYZOM / TEST

*/

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "patch.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "game_share/singleton_registry.h"

//-----------------------------------------------------------------------------
// Handy utility functions
//-----------------------------------------------------------------------------

void normalisePackageDescriptionFileName(std::string& fileName)
{
	if (fileName.empty())
		fileName="package_description";
	if (NLMISC::CFile::getExtension(fileName).empty() && fileName[fileName.size()-1]!='.')
		fileName+=".xml";
}

//-----------------------------------------------------------------------------
// class CPatchTest
//-----------------------------------------------------------------------------

class CPatchTest: public IServiceSingleton
{
public:
	void init()
	{
		if (!NLMISC::CFile::fileExists("patch_test/test0/test_package.xml"))
			createPackage("patch_test/test0/test_package.xml");
	}

	void serviceUpdate()
	{
		static uint32 count=~0u;
		if (count++>=20)
		{
			count=0;
			if (!NLMISC::CFile::fileExists("patch_test/test0/test_package.wip"))
				processPackage("patch_test/test0/test_package.xml");
			else
				nlinfo("skipping 'process package' because test_package.wip 'work in progress' marker exists");
		}
	}

	static bool createPackage(std::string fileName)
	{
		// normalise the file name (and path)
		normalisePackageDescriptionFileName(fileName);

		// make sure the file doesn't exist
		BOMB_IF(NLMISC::CFile::fileExists(fileName),("Failed to careate new package because file already exists: "+fileName).c_str(),return false);

		// create the directory tree required for the file
		NLMISC::CFile::createDirectoryTree(NLMISC::CFile::getPath(fileName));

		// create a new package, store it to a persistent data record and write the latter to a file
		CPackageDescription package;
		CPersistentDataRecord pdr;
		package.storeToPdr(pdr);
		pdr.writeToTxtFile(fileName.c_str());
		package.setup(fileName);
		package.createDirectories();
		package.buildDefaultFileList();
		package.storeToPdr(pdr);
		pdr.writeToTxtFile(fileName.c_str());
		BOMB_IF(!NLMISC::CFile::fileExists(fileName),("Failed to create new package file: "+fileName).c_str(),return false);
		nlinfo("New package description file created successfully: %s",fileName.c_str());

		return true;
	}

	static bool processPackage(std::string fileName)
	{
		// normalise the file name (and path)
		normalisePackageDescriptionFileName(fileName);

		// make sure the file exists
		BOMB_IF(!NLMISC::CFile::fileExists(fileName),("Failed to process package because file not found: "+fileName).c_str(),return false);

		// read the package description file
		CPackageDescription thePackage;
		thePackage.setup(fileName);

		// read the index file for the package
		CBNPFileSet packageIndex;
		thePackage.readIndex(packageIndex);

		// update the files list in the index
		thePackage.updateIndexFileList(packageIndex);

		// update the index for the package
		thePackage.addVersion(packageIndex);

		// save the updated index file
		thePackage.writeIndex(packageIndex);

		// generate patches as required
		thePackage.generatePatches(packageIndex);

		// generate client index file
		CPackageDescriptionForClient theClientPackage;
		thePackage.generateClientIndex(theClientPackage,packageIndex);

		return true;
	}
};

static CPatchTest PatchTest;

NLMISC_COMMAND(createPackage,"create a new package description file","<package description file name>")
{
	if (args.size()!=1)
		return false;

	PatchTest.createPackage(args[0]);

	return true;
}

NLMISC_COMMAND(processPackage,"process a package","<package description file name>")
{
	if (args.size()!=1)
		return false;

	PatchTest.processPackage(args[0]);

	return true;
}

NLMISC_COMMAND(bin2xml,"convert a binary pdr file to xml","<input file name> <output file name>")
{
	if (args.size()!=2)
		return false;

	CPersistentDataRecord pdr;
	pdr.readFromBinFile(args[0].c_str());
	pdr.writeToTxtFile(args[1].c_str());

	return true;
}

