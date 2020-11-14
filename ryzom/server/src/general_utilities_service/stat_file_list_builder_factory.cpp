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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stat_file_list_builder_factory.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CFileListBuilderFactory
//-------------------------------------------------------------------------------------------------

CFileListBuilderFactory* CFileListBuilderFactory::getInstance()
{
	static CFileListBuilderFactory* ptr=NULL;
	if (ptr==NULL)
		ptr=new CFileListBuilderFactory;
	return ptr;
}

void CFileListBuilderFactory::registerFileList(NLMISC::CSmartPtr<IFileListBuilderBuilder> filter)
{
	// ensure that we don't have a name conflict with an existing info extractor
	for (uint32 i=0;i<_FileLists.size();++i)
	{
		nlassert(filter->getName()!=_FileLists[i]->getName());
	}

	// add the new info extractor
	_FileLists.push_back(filter);
}

void CFileListBuilderFactory::displayFileListBuilderList(NLMISC::CLog* log)
{
	uint32 longestName=4;

	// iterate over the filters to determine the length of the longest name
	for (uint32 i=0;i<_FileLists.size();++i)
	{
		std::string s= _FileLists[i]->getName();
		if (s.size()>longestName)
			longestName=(uint32)s.size();
	}

	// iterate over the filters displaying names and description
	for (uint32 i=0;i<_FileLists.size();++i)
	{
		log->displayNL("%-*s  %s",longestName,_FileLists[i]->getName(),_FileLists[i]->getDescription());
	}
}

uint32 CFileListBuilderFactory::getFileListBuilderCount()
{
	return (uint32)_FileLists.size();
}

IFileListBuilderBuilder* CFileListBuilderFactory::getFileListBuilder(uint32 idx)
{
	nlassert(idx<_FileLists.size());
	return _FileLists[idx];
}

IFileListBuilder* CFileListBuilderFactory::build(const NLMISC::CSString& cmdLine)
{
	// split the command line into a command and a command tail (or arg set)
	CSString cmdTail=cmdLine;
	CSString cmd=cmdTail.firstWord(true).strip();

	for (uint32 i=0;i<_FileLists.size();++i)
	{
		if (cmd==_FileLists[i]->getName())
		{
			nlinfo("Adding filter: %s",cmdLine.c_str());
			return _FileLists[i]->build(cmdTail);
		}
	}

	nlwarning("Unknown filter '%s' in line: %s",cmd.c_str(),cmdLine.c_str());
	return NULL;
}


//-------------------------------------------------------------------------------------------------
