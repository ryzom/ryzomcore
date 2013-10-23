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

#ifndef STAT_FILE_LIST_FACTORY_H
#define STAT_FILE_LIST_FACTORY_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"

#include "game_share/file_description_container.h"


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CStatsScanCharacter;


//-------------------------------------------------------------------------------------------------
// class IFileListBuilder
//-------------------------------------------------------------------------------------------------

class IFileListBuilder: public NLMISC::CRefCount
{
public:
	virtual ~IFileListBuilder() {}
	virtual std::string toString() const=0;
	virtual bool execute(CFileDescriptionContainer &fdc)=0;
};


//-------------------------------------------------------------------------------------------------
// class IFileListBuilderBuilder
//-------------------------------------------------------------------------------------------------

class IFileListBuilderBuilder: public NLMISC::CRefCount
{
public:
	virtual ~IFileListBuilderBuilder() {}
	virtual const char* getName()=0;
	virtual const char* getDescription()=0;
	virtual IFileListBuilder* build(const std::string& rawArgs)=0;
};


//-------------------------------------------------------------------------------------------------
// class CFileListBuilderFactory
//-------------------------------------------------------------------------------------------------

class CFileListBuilderFactory
{
private:
	// this is a singleton so ctor is private
	CFileListBuilderFactory() {}

public:
	static CFileListBuilderFactory* getInstance();

public:
	// register an info extractor instance
	void registerFileList(NLMISC::CSmartPtr<IFileListBuilderBuilder> filter);

	// display the set of names and descriptions of info extractor instances
	void displayFileListBuilderList(NLMISC::CLog* log=NLMISC::InfoLog);

	// basic accessors for getting hold of the registered info extractors
	uint32 getFileListBuilderCount();
	IFileListBuilderBuilder* getFileListBuilder(uint32 idx);

	// the all important build method
	IFileListBuilder* build(const NLMISC::CSString& cmdLine);

private:
	// private data
	typedef std::vector<NLMISC::CSmartPtr<IFileListBuilderBuilder> > TFileLists;
	TFileLists _FileLists;
};


//-------------------------------------------------------------------------------------------------
// class CFileListRegisterer
//-------------------------------------------------------------------------------------------------

template <class C>
class CFileListRegisterer
{
public:
	CFileListRegisterer()
	{
		CFileListBuilderFactory::getInstance()->registerFileList(new C);
	}
};


//-------------------------------------------------------------------------------------------------
// MACRO FILE_LIST_BUILDER()
//-------------------------------------------------------------------------------------------------

#define FILE_LIST_BUILDER(name,description)\
class CFileList_##name: public IFileListBuilder\
{\
public:\
	CFileList_##name(const std::string& rawArgs) {_RawArgs=rawArgs;}\
	virtual std::string toString() const {return std::string(#name)+" "+_RawArgs;}\
	virtual bool execute(CFileDescriptionContainer &fdc);\
private:\
	NLMISC::CSString _RawArgs;\
};\
class CFileListBuilder_##name: public IFileListBuilderBuilder\
{\
public:\
	virtual const char* getName()			{return #name;}\
	virtual const char* getDescription()	{return description;}\
	virtual IFileListBuilder* build(const std::string& rawArgs)	{return new CFileList_##name(rawArgs);}\
};\
CFileListRegisterer<CFileListBuilder_##name> __Registerer_CFileList_##name;\
bool CFileList_##name::execute(CFileDescriptionContainer &fdc)


//-------------------------------------------------------------------------------------------------
#endif
