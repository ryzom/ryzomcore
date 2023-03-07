// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2011-2013  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2014  Matthew LAGOE (Botanic) <cyberempires@gmail.com>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include <cstdio>
#include <limits>

#include "game_share/bnp_patch.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/command.h"
#include "nel/misc/sstring.h"
#include "nel/misc/seven_zip.h"
#include "game_share/singleton_registry.h"

using namespace std;
using namespace NLMISC;

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


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

void GeneratePatch(const std::string& srcFileName,const std::string& destFileName,const std::string& patchFileName)
{
	std::string cmd = toString("xdelta delta %s %s %s", srcFileName.c_str(), destFileName.c_str(), patchFileName.c_str());

	nlinfo("Executing system command: %s", cmd.c_str());

#ifdef NL_OS_WINDOWS
	_spawnlp(_P_WAIT, "xdelta.exe", "xdelta.exe", "delta", srcFileName.c_str(), destFileName.c_str(), patchFileName.c_str(), NULL);
#else // NL_OS_WINDOWS
	// xdelta-1.x behaves like "diff" and returns 0 for identical files, 1 for different files, 2 for errors
	sint error = system (cmd.c_str());

	if (error == 2)
		nlwarning("'%s' failed with error code %d", cmd.c_str(), error);
#endif // NL_OS_WINDOWS
}

void ApplyPatch(const std::string& srcFileName,const std::string& destFileName,const std::string& patchFileName=std::string())
{
	std::string cmd = toString("xdelta patch %s %s %s", patchFileName.c_str(), srcFileName.c_str(), destFileName.c_str());

	nlinfo("Executing system command: %s", cmd.c_str());

#ifdef NL_OS_WINDOWS
	_spawnlp(_P_WAIT, "xdelta.exe", "xdelta.exe", "patch",patchFileName.c_str(), srcFileName.c_str(), destFileName.c_str(), NULL);
#else // NL_OS_WINDOWS
	// xdelta-1.x behaves like "diff" and returns 0 for identical files, 1 for different files, 2 for errors
	sint error = system (cmd.c_str());

	if (error == 2)
		nlwarning("'%s' failed with error code %d", cmd.c_str(), error);
#endif // NL_OS_WINDOWS
}

void GenerateLZMA(const std::string &sourceFile, const std::string &outputFile)
{
	{
		nlinfo("Compressing %s to %s using LZMA...", sourceFile.c_str(), outputFile.c_str());
	}

	if (!packLZMA(sourceFile, outputFile))
	{
		nlwarning("LZMA compress failed");
	}
}


//-----------------------------------------------------------------------------
// class CPackageDescription
//-----------------------------------------------------------------------------

class CPackageDescription: public IVersionNumberGenerator
{
private:
	DECLARE_PERSISTENCE_METHODS

public:
	CPackageDescription();

	void clear();

	void setup(const std::string& packageName);
	void storeToPdr(CPersistentDataRecord& pdr) const;

	void readIndex(CBNPFileSet& packageIndex) const;
	void writeIndex(const CBNPFileSet& packageIndex) const;

	void getCategories(CPersistentDataRecord &pdr) const;

	void updateIndexFileList(CBNPFileSet& packageIndex) const;
	void generateClientIndex(CProductDescriptionForClient& theClientPackage,const CBNPFileSet& packageIndex) const;
	void addVersion(CBNPFileSet& packageIndex);
	void generatePatches(CBNPFileSet& packageIndex) const;
	void createDirectories() const;
	void buildDefaultFileList();

	void updatePatchSizes(CBNPFileSet& packageIndex) const;

	// specialisation of IVersionNumberGenerator
	void grabVersionNumber();
	uint32 getPackageVersionNumber();

private:
	CBNPCategorySet	_Categories;
	std::string		_IndexFileName;
	std::string		_ClientIndexFileName;
	std::string		_RootDirectory;
	std::string		_PatchDirectory;
	std::string		_BnpDirectory;
	std::string		_RefDirectory;
	std::string		_NextVersionFile;

	uint32			_NextVersionNumber;
	bool			_VersionNumberReserved;
};


//-----------------------------------------------------------------------------
// methods CPackageDescription
//-----------------------------------------------------------------------------

CPackageDescription::CPackageDescription()
{
	clear();
}

void CPackageDescription::clear()
{
	_NextVersionNumber= std::numeric_limits<uint32>::max();
	_VersionNumberReserved = false;
	_Categories.clear();
	_IndexFileName.clear();
	_ClientIndexFileName.clear();
	_PatchDirectory.clear();
	_BnpDirectory.clear();
	_RefDirectory.clear();
}

void CPackageDescription::setup(const std::string& packageName)
{
	nlinfo("Reading package description: %s ...",packageName.c_str());

	// clear out old contents before reading from input file
	clear();

	// read new contents from input file
	static CPersistentDataRecord pdr;
	pdr.clear();
	pdr.readFromTxtFile(packageName);
	apply(pdr);

	// root directory
	if (_RootDirectory.empty())
		_RootDirectory = NLMISC::CFile::getPath(packageName);
	_RootDirectory = NLMISC::CPath::standardizePath(_RootDirectory, true);

	// patch directory
	if (_PatchDirectory.empty())
		_PatchDirectory = _RootDirectory + "patch";
	_PatchDirectory = NLMISC::CPath::standardizePath(_PatchDirectory, true);

	// BNP directory
	if (_BnpDirectory.empty())
		_BnpDirectory = _RootDirectory+"bnp";
	_BnpDirectory = NLMISC::CPath::standardizePath(_BnpDirectory, true);

	// ref directory
	if (_RefDirectory.empty())
		_RefDirectory = _RootDirectory+"ref";
	_RefDirectory = NLMISC::CPath::standardizePath(_RefDirectory, true);

	// client index file
	if (_ClientIndexFileName.empty())
		_ClientIndexFileName = NLMISC::CFile::getFilenameWithoutExtension(packageName)+".idx";

	// index file
	if (_IndexFileName.empty())
		_IndexFileName = NLMISC::CFile::getFilenameWithoutExtension(_ClientIndexFileName)+".hist";
}

void CPackageDescription::storeToPdr(CPersistentDataRecord& pdr) const
{
	pdr.clear();
	store(pdr);
}

void CPackageDescription::readIndex(CBNPFileSet& packageIndex) const
{
	std::string indexPath = _RootDirectory + _IndexFileName;

	nlinfo("Reading history file: %s ...", indexPath.c_str());

	// clear out old contents before reading from input file
	packageIndex.clear();

	// read new contents from input file
	if (NLMISC::CFile::fileExists(indexPath))
	{
		static CPersistentDataRecord pdr;
		pdr.clear();
		pdr.readFromTxtFile(indexPath);
		packageIndex.apply(pdr);
	}
}

void CPackageDescription::writeIndex(const CBNPFileSet& packageIndex) const
{
	std::string indexPath = _RootDirectory + _IndexFileName;

	nlinfo("Writing history file: %s ...", indexPath.c_str());

	// write contents to output file
	static CPersistentDataRecordRyzomStore pdr;
	pdr.clear();
	packageIndex.store(pdr);
	pdr.writeToTxtFile(indexPath);
}

void CPackageDescription::getCategories(CPersistentDataRecord &pdr) const
{
	pdr.clear();
	_Categories.store(pdr);
}

void CPackageDescription::updateIndexFileList(CBNPFileSet& packageIndex) const
{
	nlinfo("Updating file list from package categories (%d files) ...",_Categories.fileCount());
	for (uint32 i=_Categories.fileCount();i--;)
	{
		const std::string& fileName= _Categories.getFile(i);
		packageIndex.addFile(fileName,_Categories.isFileIncremental(fileName));

		// if the file is flagged as non-incremental then we need to add its refference file too
		if (!_Categories.isFileIncremental(fileName))
		{
			std::string refName= NLMISC::CFile::getFilenameWithoutExtension(_Categories.getFile(i))+"_.ref";
			packageIndex.addFile(refName);

			// if the ref file doesn't exist then create it by copying the original
			if (NLMISC::CFile::fileExists(_BnpDirectory+fileName) && !NLMISC::CFile::fileExists(_BnpDirectory+refName))
			{
				NLMISC::CFile::copyFile(_BnpDirectory+refName,_BnpDirectory+fileName);
				nlassert(NLMISC::CFile::getFileSize(_BnpDirectory+refName)== NLMISC::CFile::getFileSize(_BnpDirectory+fileName));
			}
		}
	}
}

void CPackageDescription::generateClientIndex(CProductDescriptionForClient& theClientPackage, const CBNPFileSet& packageIndex) const
{
	std::string patchNumber = toString("%05u", packageIndex.getVersionNumber());
	std::string patchDirectory = _PatchDirectory + patchNumber;
	std::string patchFile = patchDirectory + "/" + _ClientIndexFileName;

	nlinfo("Generating client index: %s...", patchFile.c_str());

	// make sure the version sub directory exist
	CFile::createDirectory(patchDirectory);

	// clear out the client package before we start
	theClientPackage.clear();

	// copy the categories using a pdr record
	static CPersistentDataRecordRyzomStore pdr;
	pdr.clear();
	_Categories.store(pdr);
	theClientPackage.setCategories(pdr);

	// copy the files using a pdr record
	pdr.clear();
	packageIndex.store(pdr);
	theClientPackage.setFiles(pdr);

	// create the output file
	pdr.clear();
	theClientPackage.store(pdr);

	std::string newName = patchDirectory + "/" + NLMISC::CFile::getFilenameWithoutExtension(_ClientIndexFileName) + "_" + patchNumber;

	pdr.writeToBinFile(newName + ".idx");
	pdr.writeToTxtFile(newName + "_debug.xml");
}

void CPackageDescription::addVersion(CBNPFileSet& packageIndex)
{
	// calculate the last version number in the index file
//	nlinfo("Calculating package version number...");
//	uint32 versionNumber= packageIndex.getVersionNumber();
//	nlinfo("Last version number = %d",versionNumber);
//	uint32 newVersionNumber= packageIndex.addVersion(_BnpDirectory,versionNumber+1);
//	nlinfo("New version number = %d",newVersionNumber);

	// setup the default next version number by scanning the package index for the highest existing version number
	nlinfo("Calculating package version number...");
	_NextVersionNumber= packageIndex.getVersionNumber();
	nlinfo("Last version number = %u",_NextVersionNumber);
	++_NextVersionNumber;

	// have the package index check its file list to see if a new version is required
	uint32 newVersionNumber= packageIndex.addVersion(_BnpDirectory,_RefDirectory,*this);
	nlinfo("Added files for version: %u",newVersionNumber);
}

void CPackageDescription::generatePatches(CBNPFileSet& packageIndex) const
{
	nlinfo("Generating patches ...");

	for (uint32 i = packageIndex.fileCount(); i--;)
	{
		bool deleteRefAfterDelta = true;
		bool usingTemporaryFile = false;
		// generate file name root
		std::string bnpFileName = _BnpDirectory + packageIndex.getFile(i).getFileName();
		std::string refNameRoot = _RefDirectory + NLMISC::CFile::getFilenameWithoutExtension(bnpFileName);
		std::string patchNameRoot = _PatchDirectory + NLMISC::CFile::getFilenameWithoutExtension(bnpFileName);

		// if the file has no versions then skip on to the next file
		if (packageIndex.getFile(i).versionCount()==0)
			continue;

		// get the last version number and the related file name
		const CBNPFileVersion& curVersion= packageIndex.getFile(i).getVersion(packageIndex.getFile(i).versionCount()-1);
		std::string curVersionFileName= refNameRoot+NLMISC::toString("_%05u.%s",curVersion.getVersionNumber(),NLMISC::CFile::getExtension(bnpFileName).c_str());
//		std::string patchFileName= patchNameRoot+NLMISC::toString("_%05d.patch",curVersion.getVersionNumber());
		std::string patchFileName= _PatchDirectory + toString("%05u/",curVersion.getVersionNumber())+NLMISC::CFile::getFilenameWithoutExtension(bnpFileName)+toString("_%05u",curVersion.getVersionNumber())+".patch";

		// get the second last version number and the related file name
		std::string prevVersionFileName;
		if (packageIndex.getFile(i).versionCount()==1)
		{
			prevVersionFileName= _RootDirectory + "empty";
			CFile::createEmptyFile(prevVersionFileName);
			usingTemporaryFile = true;
			deleteRefAfterDelta= false;
		}
		else
		{
			const CBNPFileVersion& prevVersion= packageIndex.getFile(i).getVersion(packageIndex.getFile(i).versionCount()-2);
			prevVersionFileName= refNameRoot+NLMISC::toString("_%05u.%s",prevVersion.getVersionNumber(),NLMISC::CFile::getExtension(bnpFileName).c_str());
		}
		std::string refVersionFileName= prevVersionFileName;

		// create the subdirectory for this patch number
		string versionSubDir = _PatchDirectory + toString("%05u/", curVersion.getVersionNumber());
		CFile::createDirectory(versionSubDir);

		// generate the lzma packed version of the bnp if needed (lzma file are slow to generate)
		string lzmaFile = versionSubDir+CFile::getFilename(bnpFileName)+".lzma";
		if (!CFile::fileExists(lzmaFile))
		{
			// build the lzma compression in a temp file (avoid leaving dirty file if the
			// process cannot terminate)
			GenerateLZMA(bnpFileName, lzmaFile+".tmp");
			// rename the tmp file
			CFile::moveFile(lzmaFile, lzmaFile+".tmp");
		}

		// store the lzma file size in the descriptor
		packageIndex.getFile(i).getVersion(packageIndex.getFile(i).versionCount()-1).set7ZipFileSize(CFile::getFileSize(lzmaFile));

		// if we need to generate a new patch then do it and create the new ref file
		if (!NLMISC::CFile::fileExists(curVersionFileName))
		{
			nlinfo("- Creating patch: %s",patchFileName.c_str());

			// in the case where we compress against a ref file...
			if (!_Categories.isFileIncremental(NLMISC::CFile::getFilename(bnpFileName)))
			{
				// setup the name of the reference file to patch against
				refVersionFileName= _BnpDirectory+NLMISC::CFile::getFilenameWithoutExtension(bnpFileName)+"_.ref";

				// delete the previous patch - because we only need the latest patch for non-incremental files
				std::string lastPatch= _PatchDirectory + NLMISC::CFile::getFilenameWithoutExtension(prevVersionFileName)+".patch";
				if (NLMISC::CFile::fileExists(lastPatch.c_str()))
					NLMISC::CFile::deleteFile(lastPatch.c_str());
			}

			// call xdelta to generate the patch
			GeneratePatch(refVersionFileName, bnpFileName, patchFileName);
			nlassert(NLMISC::CFile::fileExists(patchFileName));

			uint32 nPatchSize = NLMISC::CFile::getFileSize(patchFileName);
			packageIndex.getFile(i).getVersion(packageIndex.getFile(i).versionCount()-1).setPatchSize(nPatchSize);

			// apply the incremental patch to the old ref file to create the new ref file
			// and ensure that the new ref file matches the BNP
			ApplyPatch(refVersionFileName, curVersionFileName, patchFileName);
			nlassert(NLMISC::CFile::fileExists(curVersionFileName));
			nlassert(NLMISC::CFile::thoroughFileCompare(bnpFileName, curVersionFileName));
		}

		// if we have a ref file still hanging about from the previous patch then delete it
		if (NLMISC::CFile::fileExists(prevVersionFileName))
		{
			NLMISC::CFile::deleteFile(prevVersionFileName);
		}
	}
}

void CPackageDescription::createDirectories() const
{
	NLMISC::CFile::createDirectoryTree(_RootDirectory);
	NLMISC::CFile::createDirectoryTree(_PatchDirectory);
	NLMISC::CFile::createDirectoryTree(_BnpDirectory);
	NLMISC::CFile::createDirectoryTree(_RefDirectory);
}

void CPackageDescription::buildDefaultFileList()
{
	// make sure the default categories exist
	CBNPCategory* packedCategory= _Categories.getCategory("main", true);
	packedCategory->setOptional(false);
	packedCategory->setIncremental(true);

	CBNPCategory* unpackedCategory= _Categories.getCategory("unpacked", true);
	unpackedCategory->setUnpackTo("./");
	unpackedCategory->setOptional(false);
	unpackedCategory->setIncremental(false);

	CBNPCategory* optionCategory= _Categories.getCategory("optional", true);
	optionCategory->setOptional(true);
	optionCategory->setIncremental(true);

	// look for BNP files in the BNP directry and add them to the main category
	std::vector<std::string> fileList;
	NLMISC::CPath::getPathContent(_BnpDirectory,false,false,true,fileList);
	for (uint32 i=0;i<fileList.size();++i)
		if (NLMISC::toLowerAscii(NLMISC::CFile::getExtension(fileList[i]))=="bnp"
			|| NLMISC::toLowerAscii(NLMISC::CFile::getExtension(fileList[i]))=="snp")
			_Categories.addFile("main",NLMISC::toLowerAscii(NLMISC::CFile::getFilename(fileList[i])));

	_Categories.addFile("unpacked","root.bnp");
}

void CPackageDescription::updatePatchSizes(CBNPFileSet& packageIndex) const
{
}

void CPackageDescription::grabVersionNumber()
{
	// if we've already grabbed the next version number then just return
	if (_VersionNumberReserved)
		return;

	// if we don't have a version file to deal with then we're done
	if (_NextVersionFile.empty())
		return;

	// read the version number from the '_NextVersion' file
	nlassert(NLMISC::CFile::fileExists(_NextVersionFile));
	NLMISC::CSString fileContents;
	fileContents.readFromFile(_NextVersionFile);
	uint32 versionFromFile= fileContents.atoui();
	nlinfo("Version number read from file (%s) = %u",_NextVersionFile.c_str(),versionFromFile);

	// select the higher of the 2 version numbers
	_NextVersionNumber= std::max(_NextVersionNumber,versionFromFile);
	nlinfo("New version number = %u",_NextVersionNumber);

	// write the result +1 back to the '_NextVersion' file
	(NLMISC::CSString()<<(_NextVersionNumber+1)).writeToFile(_NextVersionFile);
	fileContents.readFromFile(_NextVersionFile);
	versionFromFile= fileContents.atoui();
	nlassert( versionFromFile == (_NextVersionNumber+1) );

	// success so flag the version number as reserved
	_VersionNumberReserved= true;
}

uint32 CPackageDescription::getPackageVersionNumber()
{
	return _NextVersionNumber;
}


//-----------------------------------------------------------------------------
// Persistent data for CPackageDescription
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CPackageDescription
#define PERSISTENT_DATA\
	STRUCT(_Categories)\
	PROP(std::string,_IndexFileName)\
	PROP(std::string,_PatchDirectory)\
	PROP(std::string,_BnpDirectory)\
	PROP(std::string,_RefDirectory)\
	PROP(std::string,_NextVersionFile)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// work routines
//-----------------------------------------------------------------------------

static bool createNewProduct(std::string fileName)
{
	// normalise the file name (and path)
	normalisePackageDescriptionFileName(fileName);

	// make sure the file doesn't exist
	BOMB_IF(NLMISC::CFile::fileExists(fileName),("Failed to careate new package because file already exists: "+fileName).c_str(),return false);

	// create the directory tree required for the file
	NLMISC::CFile::createDirectoryTree(NLMISC::CFile::getPath(fileName));

	// create a new package, store it to a persistent data record and write the latter to a file
	CPackageDescription package;
	static CPersistentDataRecordRyzomStore pdr;
	pdr.clear();
	package.storeToPdr(pdr);
	pdr.writeToTxtFile(fileName);
	package.setup(fileName);
	package.createDirectories();
	package.buildDefaultFileList();
	package.storeToPdr(pdr);
	pdr.writeToTxtFile(fileName);

	BOMB_IF(!NLMISC::CFile::fileExists(fileName),("Failed to create new package file: "+fileName).c_str(),return false);
	nlinfo("New package description file created successfully: %s", fileName.c_str());

	return true;
}

static bool updateProduct(std::string fileName)
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

	// add patch sizes to index file
	thePackage.updatePatchSizes(packageIndex);

	// save the updated index file
	thePackage.writeIndex(packageIndex);

	// generate client index file
	CProductDescriptionForClient theClientPackage;
	thePackage.generateClientIndex(theClientPackage,packageIndex);

	return true;
}


//-----------------------------------------------------------------------------
// commands
//-----------------------------------------------------------------------------

NLMISC_COMMAND(createNewProduct,"create a new package description file","<package description file name>")
{
	if (args.size()!=1)
		return false;

	createNewProduct(args[0]);

	return true;
}

NLMISC_COMMAND(updateProduct,"process a package","<package description file name>")
{
	if (args.size()!=1)
		return false;

	updateProduct(args[0]);

	return true;
}

NLMISC_COMMAND(go,"perform a 'createNewProduct' if required and 'updateProduct' on patch_test/test0/test_package.xml","")
{
	if (args.size()!=0)
		return false;

	if (!NLMISC::CFile::fileExists("patch_test/test0/test_package.xml"))
		createNewProduct("patch_test/test0/test_package.xml");
	updateProduct("patch_test/test0/test_package.xml");

	return true;
}
