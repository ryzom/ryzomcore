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

#ifndef BNP_PATCH_H
#define BNP_PATCH_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/sha1.h"
#include "persistent_data.h"


//-----------------------------------------------------------------------------
// class IVersionNumberGenerator
//-----------------------------------------------------------------------------

class IVersionNumberGenerator
{
public:
	// reserver the next version number (if we don't already have one)
	virtual void grabVersionNumber() =0;

	// get the current reserved version number
	// return ~0u if none reserved
	virtual uint32 getPackageVersionNumber() =0;
};


//-----------------------------------------------------------------------------
// class CBNPFileVersion
//-----------------------------------------------------------------------------

class CBNPFileVersion
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	// ctor
	CBNPFileVersion();

	// setup record contents from a file name and version number
	// returns false if the file didn't exist
	bool setup(const std::string &fileName, uint32 versionNumber);

	void setVersionNumber(uint32 nVersionNumber);
	void set7ZipFileSize(uint32 n7ZFileSize);
	void setPatchSize(uint32 nPatchSize);
	void setTimeStamp(uint32 nTimeStamp);

	// accessors
	uint32 getVersionNumber() const;
	uint32 getTimeStamp() const;
	uint32 get7ZFileSize() const;
	uint32 getFileSize() const;
	uint32 getPatchSize() const;
	CHashKey getHashKey() const;

	// == operator
	bool operator==(const CBNPFileVersion& other) const;

	// != operator
	bool operator!=(const CBNPFileVersion& other) const;

private:
	uint32				_VersionNumber;
	uint32				_FileTime;
	uint32				_FileSize;
	uint32				_7ZFileSize;
	uint32				_PatchSize;
	std::vector<uint32>	_HashKey;
};

//-----------------------------------------------------------------------------
// class CBNPFile
//-----------------------------------------------------------------------------

class CBNPFile
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	// ctor
	CBNPFile();

	// add a version to the file
	// returns false if file not found
	bool addVersion(const std::string& bnpDirectory, const std::string& refDirectory, IVersionNumberGenerator& version);

	// get the last existing version number less than or equal to parameter in the file's history
	uint32 getLatestVersionNumber(uint32 max=~0u) const;

	// get number of versions in the versions vector
	uint32 versionCount() const;

	// get the nth version from the version vector
	const CBNPFileVersion& getVersion(uint32 idx) const;

	CBNPFileVersion& getVersion(uint32 idx);

	// _FileName write accessor
	void setFileName(const std::string& fileName);

	// _FileName read accessor
	const std::string& getFileName() const;

	// _Incrmental flag write accessor
	void setIncremental(bool value);

	// _Incrmental flag read accessor
	bool isIncremental();

private:
	bool _IsIncremental;
	std::string _FileName;
	std::vector<CBNPFileVersion> _Versions;
};

//-----------------------------------------------------------------------------
// class CBNPFileSet
//-----------------------------------------------------------------------------

class CBNPFileSet
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	// add a version to the file
	// returns highest version number in files after operation
	uint32 addVersion(const std::string& bnpDirectory, const std::string& refDirectory, IVersionNumberGenerator& version);
	// look through the refferenced files for the highest version number
	uint32 getVersionNumber() const;

	void clear();

	uint32 fileCount() const;
	const CBNPFile& getFile(uint32 idx) const;
	const CBNPFile* getFileByName(const std::string& fileName) const;
	CBNPFile& getFile(uint32 idx);
	CBNPFile* getFileByName(const std::string& fileName);
	void addFile(const std::string& fileName,bool isIncremental=true);

	void removeFile(const std::string &filename);

private:
	std::vector<CBNPFile> _Files;
};

//-----------------------------------------------------------------------------
// class CBNPCategory
//-----------------------------------------------------------------------------

class CBNPCategory
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	// ctor
	CBNPCategory();

	const std::string& getName() const;
	void setName(const std::string& name);

	void setOptional(bool value);
	bool isOptional() const;

	void setUnpackTo(const std::string &pathName);
	const std::string &getUnpackTo() const;

	void setIncremental(bool value);
	bool isIncremental() const;

	void setCatRequired(const std::string &cat);
	const std::string &getCatRequired() const;

	void setHidden(bool value);
	bool isHidden() const;

	uint32 fileCount() const;
	const std::string& getFile(uint32 idx) const;
	void addFile(const std::string& fileName);

	bool hasFile(const std::string &fileName) const;

private:
	std::string		_Name;
	bool			_IsOptional;
	std::string		_UnpackTo;
	bool			_IsIncremental;
	std::string		_CatRequired;	// Name of the category required
	bool			_Hidden;		// If optional but not displayed
	std::vector<std::string> _Files;
};


//-----------------------------------------------------------------------------
// class CBNPCategorySet
//-----------------------------------------------------------------------------

class CBNPCategorySet
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	void clear();

	// file accessors
	uint32 fileCount() const;
	const std::string& getFile(uint32 idx) const;

	// category accessors
	uint32 categoryCount() const;
	CBNPCategory& getCategory(uint32 idx);
	const CBNPCategory& getCategory(uint32 idx) const;
	void deleteCategory(uint32 index);

	// check whether a named category exists and add a new one if need be
	const CBNPCategory* getCategory(const std::string& categoryName) const;
	CBNPCategory* getCategory(const std::string& categoryName, bool addIfNotExist);
	void addFile(const std::string& categoryName,const std::string& fileName);

	// lookup a file and check whether it's flagged as non-incremental
	bool isFileIncremental(const std::string& fileName) const;

	const CBNPCategory* getCategoryFromFile(const std::string &fileName) const;

private:
	std::vector<CBNPCategory>	_Category;
};


//-----------------------------------------------------------------------------
// class CProductDescriptionForClient
//-----------------------------------------------------------------------------
// This object is created by the patch generator, stored to a bin file, and
// used at patch time on the client to identify the required patch set

class CProductDescriptionForClient
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	void clear();

	// a few read and write accessors used for getting and setting parts of the complete data set
	void setCategories(CPersistentDataRecord &pdr);
	void setFiles(CPersistentDataRecord &pdr);
	void getCategories(CPersistentDataRecord &pdr);
	void getFiles(CPersistentDataRecord &pdr);

	// load from file
	bool load(const std::string& filePath);

	const CBNPCategorySet &getCategories() { return _Categories; }
	const CBNPFileSet &getFiles() { return _Files; }

	void serial(NLMISC::IStream &f);

private:
	CBNPCategorySet	_Categories;
	CBNPFileSet		_Files;
};


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
//class CBNPPatchDescription
//{
//public:
//	// ctor
//	CBNPPatchDescription(const std::string& targetFileName,uint32 version,uint32 size,bool requiresDownload);
//
//	// read accessors
//	const std::string&	getTargetFileName() const	{ return _TargetFileName;	}
//	const std::string&	getRefFileName() const		{ return _TargetFileName;	}
//	uint32				getVersion() const			{ return _Version;			}
//	uint32				getSize() const				{ return _Size;				}
//	bool				getRequiresDownload() const	{ return _RequiresDownload;	}
//
//	// accessors for the patcher
//	std::string			getPatchFileName() const;
//
//private:
//	std::string _TargetFileName;
//	uint32		_Version;
//	uint32		_Size;
//	bool		_RequiresDownload;
//};
//
//
////-----------------------------------------------------------------------------
//// class CBNPUnpatcher
////-----------------------------------------------------------------------------
//// This class, derived from CProductDescriptionForClient is used on the client
//// to read and interpret the index file retrieved from the patch server
//
//class CBNPUnpatcher: public CProductDescriptionForClient
//{
//public:
//	//-------------------------------------------------------------------------
//	// initialisation
//
//	// ctor - initialises paths	etc
//	// also call scanForFiles()
//	CBNPUnpatcher(const std::string& productName,uint32 version,const std::string& appRootDirectory,const std::string& patchDirectory);
//
//	// get the version number for this index file (deduced from the highest
//	// version number found in the file list that it contains) and compare to
//	// the version number supplied in the ctor
//	bool isIndexUpToDate();
//
//	// get hold of the download record for the index file
//	CBNPPatchDescription getIndexFileDownloadDescription();
//
//
//	//-------------------------------------------------------------------------
//	// accessors for directories
//
//	std::string getRootDirectory() const;
//	std::string getDataDirectory() const;
//	std::string getPatchDirectory() const;
//
//
//	//-------------------------------------------------------------------------
//	// accessors for retrieving info on required patches
//
//	// return true if the installed product is completely up to date (no new patches exist)
//	bool isUpToDate();
//
//	// return true if mandatory patches exist that have not been applied
//	bool isPatchMandatory();
//
//	// return true if there are no mandatory patches but there are optional patches
//	bool isPatchOptional();
//
//	// return true if patches exist that have not been applied for the
//	// optional categories that have been flagged as selected
//	bool isPatchRequired();
//
//	// return true if it's necessary to download patches for the selected options
//	bool isDownloadRequired();
//
//	// return true if it's necessary to download the next patch that needs to be applied (in order)
//	bool isNextPatchDownloadRequired();
//
//
//	//-------------------------------------------------------------------------
//	// crunching routines
//
//	// scan the directories for files - identifies the set of required patches
//	// and also the set of these patches that is missing from the patch directory
//	void scanForFiles();
//
//	// apply the mandatory and selected optional patches
//	// nlerror if isDownloadRequired() is not false
//	void applyPatches();
//
//	// apply the next patch (in order)
//	// nlerror if isNextPatchDownloadRequired() is not false
//	void applyNextPatch();
//
//
//	//-------------------------------------------------------------------------
//	// managing the set of selected optional patch categories
//
//	// get the names of all optional categories
//	void getAllOptionalCategories(std::vector<std::string>& result);
//
//	// get the names of the optional categories that require patching
//	void getPatchableOptionalCategories(std::vector<std::string>& result);
//
//	// select or unselect an optional package
//	void setOptionalCategorySelectFlag(const std::string& categoryName, bool value);
//
//	// select or unselect all optional packages
//	void setAllOptionalCategorySelectFlags(bool value);
//
//
//	//-------------------------------------------------------------------------
//	// getting lists of applicable patches
//
//	// get the ordered list of mandatory + optional patches that need to be applied to update selected packages
//	void getSelectedPatches(std::vector<CBNPPatchDescription>& result);
//
//	// get the ordered list of optional patches that need to be applied to update selected packages
//	void getSelectedOptionalPatches(std::vector<CBNPPatchDescription>& result);
//
//	// get the ordered list of patches that need to be applied for a minimum update
//	void getMandatoryPatches(std::vector<CBNPPatchDescription>& result);
//
//	// get an ordered list of the patches that need to be applied for a full update
//	void getAllPatches(std::vector<CBNPPatchDescription>& result);
//
//	// get the name of the next patch that needs to be applied (for progress display)
//	const std::string& getNextPatchName();
//
//	// get the patch description for the next patch to apply
//	CBNPPatchDescription getNextPatch();
//
//	// get the list of patches required for selected options that are not present in the local patch directory
//	void getSelectedDownloadPatches(std::vector<CBNPPatchDescription>& result);
//
//	// get the list of all patches that are not present in the local patch directory
//	void getAllDownloadPatches(std::vector<CBNPPatchDescription>& result);
//
//
//private:
//
//	//-------------------------------------------------------------------------
//	// private utility routines
//
//	// workhorse routine used by isPatchMandatory(), isPatchOptional() & isPatchRequired()
//	bool _isPatch(bool isBySelectionFlag,bool isOptionalFlag=false);
//
//
//	//-------------------------------------------------------------------------
//	// private data
//
//	std::string _AppRootDirectory;
//	std::string _PatchDirectory;
//	std::string _ProductName;
//	uint32 _Version;
//	std::set<std::string> _SelectedCategories;
//};
//
//#endif

#endif
