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

#include "char_filter_factory.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CCharFilterFactory
//-------------------------------------------------------------------------------------------------

CCharFilterFactory* CCharFilterFactory::getInstance()
{
	static CCharFilterFactory* ptr=NULL;
	if (ptr==NULL)
		ptr=new CCharFilterFactory;
	return ptr;
}

void CCharFilterFactory::registerFilter(NLMISC::CSmartPtr<ICharFilterBuilder> filter)
{
	// ensure that we don't have a name conflict with an existing info extractor
	for (uint32 i=0;i<_Filters.size();++i)
	{
		nlassert(filter->getName()!=_Filters[i]->getName());
	}

	// add the new info extractor
	_Filters.push_back(filter);
}

void CCharFilterFactory::displayFilterList(NLMISC::CLog* log)
{
	uint32 longestName=4;

	// iterate over the filters to determine the length of the longest name
	for (uint32 i=0;i<_Filters.size();++i)
	{
		std::string s= _Filters[i]->getName();
		if (s.size()>longestName)
			longestName=(uint32)s.size();
	}

	// iterate over the filters displaying names and description
	for (uint32 i=0;i<_Filters.size();++i)
	{
		log->displayNL("%-*s  %s",longestName,_Filters[i]->getName(),_Filters[i]->getDescription());
	}
}

uint32 CCharFilterFactory::getFilterBuilderCount()
{
	return (uint32)_Filters.size();
}

ICharFilterBuilder* CCharFilterFactory::getFilterBuilder(uint32 idx)
{
	nlassert(idx<_Filters.size());
	return _Filters[idx];
}

ICharFilter* CCharFilterFactory::build(const NLMISC::CSString& cmdLine)
{
	// split the command line into a command and a command tail (or arg set)
	CSString cmdTail=cmdLine;
	CSString cmd=cmdTail.firstWord(true).strip();

	for (uint32 i=0;i<_Filters.size();++i)
	{
		if (cmd==_Filters[i]->getName())
		{
			nlinfo("Adding filter: %s",cmdLine.c_str());
			return _Filters[i]->build(cmdTail);
		}
	}

	nlwarning("Unknown filter '%s' in line: %s",cmd.c_str(),cmdLine.c_str());
	return NULL;
}
