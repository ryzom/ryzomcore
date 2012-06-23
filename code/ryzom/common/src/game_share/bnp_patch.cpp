// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "nel/misc/path.h"
#include "nel/misc/sha1.h"
#include "bnp_patch.h"


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//-----------------------------------------------------------------------------
// Handy utility routines
//-----------------------------------------------------------------------------

void normaliseBnpFileName(std::string& fileName)
{
	BOMB_IF(fileName.empty(),"Can't normalise an empty bnp file name",return);
	if (NLMISC::CFile::getExtension(fileName).empty() && fileName[fileName.size()-1]!='.')
		fileName+=".bnp";
}

void applyDate (const std::string &sFilename, uint32 nDate)
{
	// change the file time
	if(nDate != 0)
	{
//		_utimbuf utb;
//		utb.actime = utb.modtime = nDate;
		NLMISC::CFile::setRWAccess(sFilename);
		NLMISC::CFile::setFileModificationDate(sFilename, nDate);
//		_utime (sFilename.c_str (), &utb);
	}
}



//-----------------------------------------------------------------------------
// class CBNPFileVersion
//-----------------------------------------------------------------------------

CBNPFileVersion::CBNPFileVersion()
{
	_FileTime= 0;
	_FileSize= 0;
	_7ZFileSize=0;
	_PatchSize= 0;
	_VersionNumber= std::numeric_limits<uint32>::max();
}

// setup record contents from a file name and version number
// returns false if the file didn't exist
bool CBNPFileVersion::setup(const std::string &fileName, uint32 versionNumber)
{
	// make sure the file exists...
	BOMB_IF(!NLMISC::CFile::fileExists(fileName),("File not found: "+fileName).c_str(),return false);

	// generate a hash key for the file and store it in a vector of uint32
	CHashKey hashKey= getSHA1(fileName);
	nlassert(hashKey.HashKeyString.size()==20);
	_HashKey.clear();
	for (uint32 i=0;i<5;++i)
		_HashKey.push_back(*(uint32*)&hashKey.HashKeyString[4*i]);

	// get the other file properties
	_FileTime= NLMISC::CFile::getFileModificationDate(fileName);
	_FileSize= NLMISC::CFile::getFileSize(fileName);

	// setup the version number
	_VersionNumber= versionNumber;

	return true;
}

void CBNPFileVersion::setVersionNumber(uint32 nVersionNumber)
{
	_VersionNumber = nVersionNumber;
}

void CBNPFileVersion::set7ZipFileSize(uint32 n7ZFileSize)
{
	_7ZFileSize = n7ZFileSize;
}

void CBNPFileVersion::setPatchSize(uint32 nPatchSize)
{
	_PatchSize = nPatchSize;
}

void CBNPFileVersion::setTimeStamp(uint32 nTimeStamp)
{
	_FileTime = nTimeStamp;
}


// accessors
uint32 CBNPFileVersion::getVersionNumber() const
{
	return _VersionNumber;
}

uint32 CBNPFileVersion::getTimeStamp() const
{
	return _FileTime;
}

uint32 CBNPFileVersion::getFileSize() const
{
	return _FileSize;
}

uint32 CBNPFileVersion::get7ZFileSize() const
{
	return _7ZFileSize;
}

uint32 CBNPFileVersion::getPatchSize() const
{
	return _PatchSize;
}

CHashKey CBNPFileVersion::getHashKey() const
{
	nlassert(_HashKey.size()==5);
	CHashKey hashKey;
	for (uint32 i=0;i<5;++i)
	{
		*(uint32*)&hashKey.HashKeyString[4*i]=_HashKey[i];
	}
	return hashKey;
}

// == operator
bool CBNPFileVersion::operator==(const CBNPFileVersion& other) const
{
	// make sure the file sizes match
	if (_FileSize!=other._FileSize)
		return false;

	// make sure the hash keys match
	if (_HashKey!=other._HashKey)
		return false;

	// we don't compare version numbers or file dates as they're not interesting
	return true;
}

// != operator
bool CBNPFileVersion::operator!=(const CBNPFileVersion& other) const
{
	return !operator==(other);
}


//-----------------------------------------------------------------------------
// Persistent data for CBNPFileVersion
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CBNPFileVersion
#define PERSISTENT_DATA \
	PROP(uint32,_VersionNumber) \
	PROP(uint32,_FileSize) \
	PROP(uint32,_7ZFileSize) \
	PROP(uint32,_FileTime) \
	PROP(uint32,_PatchSize) \
	PROP_VECT(uint32,_HashKey)

//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// class CBNPFile
//-----------------------------------------------------------------------------

CBNPFile::CBNPFile()
{
	_IsIncremental= true;
}

bool CBNPFile::addVersion(const std::string& bnpDirectory, const std::string& /* refDirectory */, IVersionNumberGenerator& version)
{
	nlinfo("Checking need to add new version to file: %s",_FileName.c_str());

	// perform a quick check to see if the time stamp and file size of the new BNP file match the last version in the index
	std::string fullFileName= bnpDirectory+_FileName;
	if (!NLMISC::CFile::fileExists(fullFileName))
		return false;
	if (!_Versions.empty())
	{
		if ((NLMISC::CFile::getFileSize(fullFileName)==(uint32)_Versions.back().getFileSize())
		&&  (NLMISC::CFile::getFileModificationDate(fullFileName)==(uint32)_Versions.back().getTimeStamp()))
			return true;

		NLMISC::InfoLog->displayNL("File: %s\n size(%d != %d) || time(%d != %d)",
			fullFileName.c_str(),
			NLMISC::CFile::getFileSize(fullFileName),
			(uint32)_Versions.back().getFileSize(),
			NLMISC::CFile::getFileModificationDate(fullFileName),
			(uint32)_Versions.back().getTimeStamp()
			);
	}

	// create a new record for the BNP file that we have on the disk at the moment
	// if no file was found then give up (return)
	CBNPFileVersion fileVersion;
	bool result= fileVersion.setup(fullFileName,~0u);
	if (result==false)
		return false;

	// compare the fileVersion record to the last record in the history.
	// If they don't match then append it
	if (_Versions.empty() || _Versions.back()!=fileVersion)
	{
		// if we haven't yet generated the version number for this version then go for it now
		version.grabVersionNumber();
		fileVersion.setVersionNumber(version.getPackageVersionNumber());

		// make sure that our version numbers are ever increasing... it would be fatal to have an out-of-order version
		if (!_Versions.empty())
			nlassert(_Versions.back().getVersionNumber()<version.getPackageVersionNumber());

		// the file's current checksum doesn't match the previous checksum so add the new version
		nlinfo("- Adding version %05u to file: %s",version.getPackageVersionNumber(),_FileName.c_str());
		_Versions.push_back(fileVersion);

		// copy the file to create a new reference file...
//		NLMISC::CSString refFileName= NLMISC::CSString(refDirectory+_FileName).replace(".",NLMISC::toString("_%05u.",version.getPackageVersionNumber()).c_str());
//		NLMISC::CFile::copyFile(refFileName, fullFileName);
	}
	else
	{
		// the file's size & current checksum match the previous version so just fix the file's timestamp
		nlinfo("Files contents matches previous version but time stamp is different: %s",fullFileName.c_str());
		applyDate(fullFileName,_Versions.back().getTimeStamp());
	}

	// if we're flagged as non-incremental then we don't need a version history
	if (!_IsIncremental && _Versions.size()>1)
	{
		_Versions[0]= _Versions.back();
		_Versions.resize(1);
	}

	return true;
}

uint32 CBNPFile::getLatestVersionNumber(uint32 max) const
{
	if (_Versions.empty())
		return 0;
	uint32 i=0;
	for (i=(uint32)_Versions.size();i--;)
		if (_Versions[i].getVersionNumber()<=max)
			return _Versions[i].getVersionNumber();

	nlinfo("File %s didn't exist before version %d",_FileName.c_str(),max);
	return 0;
}

uint32 CBNPFile::versionCount() const
{
	return (uint32)_Versions.size();
}

const CBNPFileVersion& CBNPFile::getVersion(uint32 idx) const
{
	nlassert(idx<versionCount());
	return _Versions[idx];
}

CBNPFileVersion& CBNPFile::getVersion(uint32 idx)
{
	nlassert(idx<versionCount());
	return _Versions[idx];
}

void CBNPFile::setFileName(const std::string& fileName)
{
	_FileName= fileName;
}

const std::string& CBNPFile::getFileName() const
{
	return _FileName;
}

void CBNPFile::setIncremental(bool value)
{
	_IsIncremental=value;

	// if we're flagged as non-incremental then we don't need a version history
	if (!_IsIncremental && _Versions.size()>1)
	{
		_Versions[0]= _Versions.back();
		_Versions.resize(1);
	}
}

bool CBNPFile::isIncremental()
{
	return _IsIncremental;
}


//-----------------------------------------------------------------------------
// Persistent data for CBNPFile
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CBNPFile
#define PERSISTENT_DATA\
	PROP(std::string,_FileName)\
	STRUCT_VECT(_Versions)

//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// class CBNPFileSet
//-----------------------------------------------------------------------------

void CBNPFileSet::removeFile(const std::string &filename)
{
	for( uint k = 0; k < _Files.size(); ++k)
	{
		if (_Files[k].getFileName() == filename)
		{
			_Files.erase(_Files.begin() + k);
		}
	}
}


// add a version to the file
// returns highest version number in files after operation
uint32 CBNPFileSet::addVersion(const std::string& bnpDirectory, const std::string& refDirectory, IVersionNumberGenerator& version)
{
	nlinfo("Updating package index...");
	uint32 result=0;

	// add versions to different files
	for (uint32 i=(uint32)_Files.size();i--;)
		if (_Files[i].addVersion(bnpDirectory,refDirectory,version)!=false)
			result= std::max(result,_Files[i].getLatestVersionNumber());

	return result;
}

// look through the referenced files for the highest version number
uint32 CBNPFileSet::getVersionNumber() const
{
	uint32 result=0;

	for (uint32 i=(uint32)_Files.size();i--;)
		result= std::max(result,_Files[i].getLatestVersionNumber());

	return result;
}

void CBNPFileSet::clear()
{
	_Files.clear();
}

uint32 CBNPFileSet::fileCount() const
{
	return (uint32)_Files.size();
}

const CBNPFile& CBNPFileSet::getFile(uint32 idx) const
{
	return const_cast<CBNPFileSet*>(this)->getFile(idx);
}

const CBNPFile* CBNPFileSet::getFileByName(const std::string& fileName) const
{
	return const_cast<CBNPFileSet*>(this)->getFileByName(fileName);
}

CBNPFile& CBNPFileSet::getFile(uint32 idx)
{
	nlassert(idx<fileCount());
	return _Files[idx];
}

CBNPFile* CBNPFileSet::getFileByName(const std::string& fileName)
{
	// look for the file by name
	for (uint32 i=0;i<fileCount();++i)
		if (getFile(i).getFileName()==fileName)
			return &getFile(i);

	// file not found so return NULL
	return NULL;
}

void CBNPFileSet::addFile(const std::string& fileName,bool isIncremental)
{
	// see if the file already exists in the files container
	if (getFileByName(fileName)!=NULL)
	{
		if (!isIncremental)
			getFileByName(fileName)->setIncremental(false);
		return;
	}

	// file is new so need to add it
	std::string s= fileName;
	normaliseBnpFileName(s);
	nlinfo("- adding file: %s",s.c_str());
	_Files.resize(_Files.size()+1);
	_Files.back().setFileName(s);
	_Files.back().setIncremental(isIncremental);
}


//-----------------------------------------------------------------------------
// Persistent data for CBNPFileSet
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CBNPFileSet
#define PERSISTENT_DATA\
	STRUCT_VECT(_Files)
//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// class CBNPCategory
//-----------------------------------------------------------------------------

CBNPCategory::CBNPCategory()
{
	_IsOptional=true;
	_IsIncremental=true;
	_Hidden=false;
}

bool CBNPCategory::hasFile(const std::string &fileName) const
{
	return std::find(_Files.begin(), _Files.end(), fileName) != _Files.end();
}

const std::string& CBNPCategory::getName() const
{
	return _Name;
}

void CBNPCategory::setName(const std::string& name)
{
	_Name=name;
}

void CBNPCategory::setOptional(bool value)
{
	_IsOptional= value;
}

bool CBNPCategory::isOptional() const
{
	return _IsOptional;
}

void CBNPCategory::setUnpackTo(const std::string &n)
{
	_UnpackTo = n;
}

const std::string &CBNPCategory::getUnpackTo() const
{
	return _UnpackTo;
}

void CBNPCategory::setIncremental(bool value)
{
	_IsIncremental= value;
}

bool CBNPCategory::isIncremental() const
{
	return _IsIncremental;
}

void CBNPCategory::setCatRequired(const std::string &cat)
{
	_CatRequired = cat;
}

const std::string &CBNPCategory::getCatRequired() const
{
	return _CatRequired;
}

void CBNPCategory::setHidden(bool value)
{
	_Hidden = value;
}

bool CBNPCategory::isHidden() const
{
	return _Hidden;
}

uint32 CBNPCategory::fileCount() const
{
	return (uint32)_Files.size();
}

const std::string& CBNPCategory::getFile(uint32 idx) const
{
	nlassert(idx<fileCount());
	return _Files[idx];
}

void CBNPCategory::addFile(const std::string& fileName)
{
	// make sure file doesn't already exist
	for (uint32 i=0;i<_Files.size();++i)
		if (_Files[i]==fileName)
			return;

	// add the new file
	_Files.push_back(fileName);
}


//-----------------------------------------------------------------------------
// Persistent data for CBNPCategory
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CBNPCategory
#define PERSISTENT_DATA\
	PROP(std::string,		_Name)\
	LPROP(bool,				_IsOptional,	if(!_IsOptional))\
	LPROP(std::string,		_UnpackTo,		if(!_UnpackTo.empty()))\
	LPROP(bool,				_IsIncremental,	if(!_IsIncremental))\
	LPROP(std::string,		_CatRequired,	if(!_CatRequired.empty()))\
	LPROP(bool,				_Hidden,		if(_Hidden))\
	PROP_VECT(std::string,	_Files)\

//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// class CBNPCategorySet
//-----------------------------------------------------------------------------

void CBNPCategorySet::clear()
{
	_Category.clear();
}


const CBNPCategory* CBNPCategorySet::getCategoryFromFile(const std::string &fileName) const
{
	for(std::vector<CBNPCategory>::const_iterator it = _Category.begin(); it != _Category.end(); ++it)
	{
		if (it->hasFile(fileName))
		{
			return &(*it);
		}
	}
	return NULL;
}


void CBNPCategorySet::deleteCategory(uint32 index)
{
	nlassert(index <  _Category.size());
	_Category.erase(_Category.begin() + index);
}


uint32 CBNPCategorySet::fileCount() const
{
	uint32 result=0;
	for (uint32 i=0;i<_Category.size();++i)
		result+=_Category[i].fileCount();
	return result;
}

const std::string& CBNPCategorySet::getFile(uint32 idx) const
{
	uint32 i=0;
	for (;;++i)
	{
		nlassert(i<_Category.size());

		if (_Category[i].fileCount()>idx)
			break;

		idx-=_Category[i].fileCount();
	}

	return _Category[i].getFile(idx);
}

uint32 CBNPCategorySet::categoryCount() const
{
	return (uint32)_Category.size();
}

CBNPCategory& CBNPCategorySet::getCategory(uint32 idx)
{
	nlassert(idx<categoryCount());
	return _Category[idx];
}

const CBNPCategory& CBNPCategorySet::getCategory(uint32 idx) const
{
	return const_cast<CBNPCategorySet*>(this)->getCategory(idx);
}

const CBNPCategory* CBNPCategorySet::getCategory(const std::string& categoryName) const
{
	// look for a category with matching name
	for (uint32 i=0;i<categoryCount();++i)
		if (getCategory(i).getName()==categoryName)
			return &(getCategory(i));
	return NULL;
}

// check whether a named category exists and add a new one if need be
CBNPCategory* CBNPCategorySet::getCategory(const std::string& categoryName, bool addIfNotExist)
{
	// look for a category with matching name
	for (uint32 i=0;i<categoryCount();++i)
		if (getCategory(i).getName()==categoryName)
			return &(getCategory(i));

	// the category wasn't found so return NULL if need be
	if (!addIfNotExist)
		return NULL;

	// create a new category if need be
	_Category.resize(_Category.size()+1);
	_Category.back().setName(categoryName);
	nlinfo("- New category created: %s",categoryName.c_str());

	return &_Category.back();
}

void CBNPCategorySet::addFile(const std::string& categoryName,const std::string& fileName)
{
	// make sure the category exists
	CBNPCategory* theCategory= getCategory(categoryName,true);

	// look to see if the file already exists in the category
	for (uint32 i=0;i<theCategory->fileCount();++i)
		if (theCategory->getFile(i)==fileName)
			return;

	// the file doesn't already exist so add it
	theCategory->addFile(fileName);
	nlinfo("- File added to category %s::%s",categoryName.c_str(),fileName.c_str());
}

bool CBNPCategorySet::isFileIncremental(const std::string& fileName) const
{
	// for each category
	for (uint32 i=0;i<categoryCount();++i)
	{
		const CBNPCategory& theCategory= getCategory(i);

		// if the category is incremental then skip it
		if (theCategory.isIncremental())
			continue;

		// if the file exists in this category then return 'false' meaning non-incremental
		for (uint32 i=0;i<theCategory.fileCount();++i)
			if (theCategory.getFile(i)==fileName)
				return false;
	}

	// the file wasn't found in a non-incremental category so it must be incremental
	return true;
}


//-----------------------------------------------------------------------------
// Persistent data for CBNPCategorySet
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CBNPCategorySet
#define PERSISTENT_DATA\
	STRUCT_VECT(_Category)
//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//-----------------------------------------------------------------------------
// class CProductDescriptionForClient
//-----------------------------------------------------------------------------

void CProductDescriptionForClient::clear()
{
	_Categories.clear();
	_Files.clear();
}

void CProductDescriptionForClient::setCategories(CPersistentDataRecord &pdr)
{
	_Categories.clear();
	_Categories.apply(pdr);
}

void CProductDescriptionForClient::setFiles(CPersistentDataRecord &pdr)
{
	_Files.clear();
	_Files.apply(pdr);
}

void CProductDescriptionForClient::getCategories(CPersistentDataRecord &pdr)
{
	pdr.clear();
	_Categories.store(pdr);
}

void CProductDescriptionForClient::getFiles(CPersistentDataRecord &pdr)
{
	pdr.clear();
	_Files.store(pdr);
}

bool CProductDescriptionForClient::load(const std::string& filePath)
{
	// read new contents from input file
	if (!NLMISC::CFile::fileExists(filePath))
		return false;

	clear();
	static CPersistentDataRecord pdr;
	pdr.clear();
	pdr.readFromBinFile(filePath.c_str());
	apply(pdr);

	return true;
}

void CProductDescriptionForClient::serial(NLMISC::IStream &f)
{
	static CPersistentDataRecord pdr("RyzomTokenFamily");
	pdr.clear();
	if (f.isReading())
	{
		bool ok = pdr.fromBuffer(f);
		if (ok)
		{
			apply(pdr);
		}
		else
		{
			throw NLMISC::Exception("Can't retrieve file desc");
		}
	}
	else
	{
		store(pdr);
		pdr.toStream(f);
	}
}


//-----------------------------------------------------------------------------
// Persistent data for CProductDescriptionForClient
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CProductDescriptionForClient
#define PERSISTENT_DATA\
	STRUCT(_Files)\
	STRUCT(_Categories)

//#      pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "persistent_data_template.h"

#undef PERSISTENT_CLASS
#undef PERSISTENT_DATA


//================================================================================================
//================================================================================================
//================================================================================================
//================================================================================================

//#if 0
//
////-----------------------------------------------------------------------------
//// class CBNPPatchDescription
////-----------------------------------------------------------------------------
//// a little object used to describe patches in CBNPUnpatcher class
//
//std::string CBNPPatchDescription::getPatchFileName() const
//{
//	return NLMISC::CFile::getFilenameWithoutExtension(getTargetFileName())+NLMISC::toString("_%05d.patch",getVersion());
//}
//
//
////-----------------------------------------------------------------------------
//// class CBNPUnpatcher
////-----------------------------------------------------------------------------
//
////-----------------------------------------------------------------------------
//// initialisation
//
//CBNPUnpatcher::CBNPUnpatcher(const std::string& productName,uint32 version,const std::string& appRootDirectory,const std::string& patchDirectory)
//{
//	_AppRootDirectory=	NLMISC::CPath::standardizePath(appRootDirectory,true);
//	_PatchDirectory=	NLMISC::CPath::standardizePath(patchDirectory,true);
//	_ProductName=		productName;
//	_Version=			version;
//}
//
//bool CBNPUnpatcher::isIndexUpToDate()
//{
//	return (_Files.getVersionNumber()==_Version);
//}
//
//CBNPPatchDescription CBNPUnpatcher::getIndexFileDownloadDescription()
//{
//	return CBNPPatchDescription(productName+"_patch_index",_Version,0,!isIndexUpToDate())
//}
//
//
////-----------------------------------------------------------------------------
//// accessors for directories
//
//std::string getRootDirectory() const
//{
//	return _AppRootDirectory;
//}
//
//std::string getDataDirectory() const
//{
//	return _AppRootDirectory+"data/";
//}
//
//std::string getPatchDirectory() const
//{
//	return _PatchDirectory;
//}
//
//
////-----------------------------------------------------------------------------
//// accessors for retrieving info on required patches
//
//bool CBNPUnpatcher::isUpToDate()
//{
//	return (isIndexUpToDate() && !isPatchMandatory() && !isPatchOptional())
//}
//
//bool CBNPUnpatcher::_isPatch(bool isBySelectionFlag,bool isOptionalFlag=false)
//{
//	nlassert(isIndexUpToDate()==true);
//
//	// scan all categories for a mandatoy category with non-up-to-date files
//	for (uint32 i=0;i<_Categories.categoryCount();++i)
//	{
//		CBNPCategory* theCategory= _Categories.getCategory(i);
//
//		// decide whether or not to skip this category
//		if (isBySelectionFlag==true)
//		{
//			if (theCategory->isOptional() &&
//				_SelectedCategories.find(theCategory->getName())==_SelectedCategories.end()))
//				continue;
//		}
//		else
//		{
//			if (theCategory->isOptional()!=isOptionalFlag)
//				continue;
//		}
//
//		// if one of the files is not up to date then we return 'true' as we need to patch
//		for (uint32 j=0;j<theCategory->fileCount();++j)
//			if (!isFileUpToDate(theCategory->getFile(j)))
//				return true;
//	}
//
//	return false;
//}
//
//bool CBNPUnpatcher::isPatchMandatory()
//{
//	return _isPatch(false,false);
//}
//
//bool CBNPUnpatcher::isPatchOptional()
//{
//	return _isPatch(false,true);
//}
//
//bool CBNPUnpatcher::isPatchRequired()
//{
//	return _isPatch(true);
//}
//
//// return true if it's necessary to download patches for the selected options
//bool CBNPUnpatcher::isDownloadRequired()
//{
//	if (!isPatchRequired())
//		return false;
//
//	std::vector<CBNPPatchDescription> hold;
//	getDownloadPatches(hold);
//	return !hold.empty();
//}
//
//// return true if it's necessary to download the next patch that needs to be applied (in order)
//bool CBNPUnpatcher::isNextPatchDownloadRequired()
//{
//	if (!isPatchRequired())
//		return false;
//
//	return getNextPatch().getRequiresDownload();
//}
//
//
////-----------------------------------------------------------------------------
//// crunching routines
//
//// scan the directories for files - identifies the set of required patches
//// and also the set of these patches that is missing from the patch directory
//void CBNPUnpatcher::scanForFiles()
//{
//	std::vector<std::string> patchFiles;
//	std::vector<std::string> patchFiles;
//	std::vector<std::string> dataFiles;
//
//	// get the list of files in the patch directory
//	NLMISC::CPath::getPathContent(getPatchDirectory(),false,false,true,patchFiles);
//
//	// get the list of files in the data directory
//	NLMISC::CPath::getPathContent(getDataDirectory(),false,false,true,dataFiles);
//
//	for (uint32 i=0;i<_Categories.fileCount();++i)
//	{
//		_Categories.getFile()
//		result.push_back(_Categories.getCategory(i).getName());
//	}
//	xxx
//}
//
//// apply the mandatory and selected optional patches
//// nlerror if isDownloadRequired() is not false
//void CBNPUnpatcher::applyPatches()
//{
//	nlassert(isIndexUpToDate()==true);
//	nlassert(!isDownloadRequired());
//	xxx
//}
//
//// apply the next patch (in order)
//// nlerror if isNextPatchDownloadRequired() is not false
//void CBNPUnpatcher::applyNextPatch()
//{
//	// note that if the index isn't up to date then it is classed as the next patch
//	nlassert(!isNextPatchDownloadRequired());
//	xxx
//}
//
//
////-----------------------------------------------------------------------------
//// managing the set of selected optional patch categories
//
//// get the names of all optional categories
//void CBNPUnpatcher::getAllOptionalCategories(std::vector<std::string>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//
//	result.clear();
//	for (uint32 i=0;i<_Categories.categoryCount();++i)
//	{
//		result.push_back(_Categories.getCategory(i).getName());
//	}
//}
//
//// get the names of the optional categories that require patching
//void CBNPUnpatcher::getPatchableOptionalCategories(std::vector<std::string>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//
//	result.clear();
//	for (uint32 i=0;i<_Categories.categoryCount();++i)
//	{
//		CBNPCategory* theCategory= _Categories.getCategory(i);
//		uint32 j;
//
//		// if one of the files is not up to date then we return 'true' as we need to patch
//		for (j=0;j<theCategory->fileCount();++j)
//			if (!isFileUpToDate(theCategory->getFile(j)))
//				break;
//
//		// if we broke out before the end of the for loop then we need to add this category
//		if (j<theCategory->fileCount())
//			result.push_back(_Categories.getCategory(i).getName());
//	}
//}
//
//// select or unselect an optional package
//void CBNPUnpatcher::setOptionalCategorySelectFlag(const std::string& categoryName, bool value)
//{
//	nlassert(isIndexUpToDate()==true);
//	_SelectedCategories.insert(categoryName);
//}
//
//// select or unselect all optional packages
//void CBNPUnpatcher::setAllOptionalCategorySelectFlags(bool value)
//{
//	nlassert(isIndexUpToDate()==true);
//
//	_SelectedCategories.clear();
//	for (uint32 i=0;i<_Categories.categoryCount();++i)
//	{
//		_SelectedCategories.insert(_Categories.getCategory(i).getName());
//	}
//}
//
//
////-----------------------------------------------------------------------------
//// getting lists of applicable patches
//
//// get the ordered list of mandatory + optional patches that need to be applied to update selected packages
//void CBNPUnpatcher::getSelectedPatches(std::vector<CBNPPatchDescription>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//
//	std::vector<CBNPPatchDescription> mandatoryPatches;
//	getMandatoryPatches(mandatoryPatches);
//
//	std::vector<CBNPPatchDescription> optionalPatches;
//	getSelectedOptionalPatches(optionalPatches);
//
//	result=	mandatoryPatches+ optionalPatches;
//}
//
//// get the ordered list of optional patches that need to be applied to update selected packages
//void CBNPUnpatcher::getSelectedOptionalPatches(std::vector<CBNPPatchDescription>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//	xxx
//}
//
//// get the ordered list of patches that need to be applied for a minimum update
//void CBNPUnpatcher::getMandatoryPatches(std::vector<CBNPPatchDescription>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//	xxx
//}
//
//// get an ordered list of the patches that need to be applied for a full update
//void CBNPUnpatcher::getAllPatches(std::vector<CBNPPatchDescription>& result)
//{
//	// store the selected category set in temporary variable
//	std::set<std::string> selectedCategories= _SelectedCategories;
//
//	// select all of the categories and delegate to getSelectedPatches()
//	setAllOptionalCategorySelectFlags();
//	getSelectedPatches(result);
//
//	// restore the _SelectedCategories set from temp variable
//	_SelectedCategories= selectedCategories;
//}
//
//// get the name of the next patch that needs to be applied (for progress display)
//const std::string& CBNPUnpatcher::getNextPatchName()
//{
//	CBNPPatchDescription patch= getNextPatch();
//	return patch.getTargetFileName()+NLMISC::toString(":%d",patch.getVersion());
//}
//
//// get the patch description for the next patch to apply
//CBNPPatchDescription CBNPUnpatcher::getNextPatch()
//{
//	// make sure that index is up to date and patching is required
//	nlassert(isIndexUpToDate());
//	nlassert(isPatchRequired());
//
//	// treat the case of !uptodate() here and get the index file as the next patch
//	if (!isIndexUpToDate())
//	{
//		return getIndexFileDownloadDescription();
//	}
//
//	for (uint32 i=0;i<_Categories.categoryCount();++i)
//	{
//		CBNPCategory* theCategory= _Categories.getCategory(i);
//		uint32 j;
//
//		// if one of the files is not up to date then we return 'true' as we need to patch
//		for (j=0;j<theCategory->fileCount();++j)
//			if (!isFileUpToDate(theCategory->getFile(j)))
//				break;
//
//		// if we broke out before the end of the for loop then we need to add this category
//		if (j<theCategory->fileCount())
//			result.push_back(_Categories.getCategory(i).getName());
//	}
//
//	xxx
//}
//
//// get the list of patches that need to be downloaded
//void CBNPUnpatcher::getSelectedDownloadPatches(std::vector<CBNPPatchDescription>& result)
//{
//	nlassert(isIndexUpToDate()==true);
//	xxx
//}
//
//// get the list of patches that need to be downloaded
//void CBNPUnpatcher::getAllDownloadPatches(std::vector<CBNPPatchDescription>& result)
//{
//	// store the selected category set in temporary variable
//	std::set<std::string> selectedCategories= _SelectedCategories;
//
//	// select all of the categories and delegate to getSelectedDownloadPatches()
//	setAllOptionalCategorySelectFlags();
//	getSelectedDownloadPatches(result);
//
//	// restore the _SelectedCategories set from temp variable
//	_SelectedCategories= selectedCategories;
//}
//
//
////-----------------------------------------------------------------------------
//
//
//#endif
