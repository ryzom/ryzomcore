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

#include "char_info_extractor_factory.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CCharInfoExtractorFactory
//-------------------------------------------------------------------------------------------------

CCharInfoExtractorFactory* CCharInfoExtractorFactory::getInstance()
{
	static CCharInfoExtractorFactory* ptr=NULL;
	if (ptr==NULL)
		ptr=new CCharInfoExtractorFactory;
	return ptr;
}

void CCharInfoExtractorFactory::registerInfoExtractor(NLMISC::CSmartPtr<ICharInfoExtractorBuilder> infoExtractor)
{
	// ensure that we don't have a name conflict with an existing info extractor
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		nlassert(infoExtractor->getName()!=_InfoExtractors[i]->getName());
	}

	// add the new info extractor
	_InfoExtractors.push_back(infoExtractor);
}

void CCharInfoExtractorFactory::displayInfoExtractorList(NLMISC::CLog* log)
{
	uint32 longestName=4;

	// iterate over the infoExtractors to determine the length of the longest name
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		std::string s= _InfoExtractors[i]->getName();
		if (s.size()>longestName)
			longestName=(uint32)s.size();
	}

	// iterate over the infoExtractors displaying names and description
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		log->displayNL("%-*s  %s",longestName,_InfoExtractors[i]->getName(),_InfoExtractors[i]->getDescription());
	}
}

uint32 CCharInfoExtractorFactory::getInfoExtractorBuilderCount()
{
	return (uint32)_InfoExtractors.size();
}

ICharInfoExtractorBuilder* CCharInfoExtractorFactory::getInfoExtractorBuilder(uint32 idx)
{
	nlassert(idx<_InfoExtractors.size());
	return _InfoExtractors[idx];
}

ICharInfoExtractor* CCharInfoExtractorFactory::build(const NLMISC::CSString& cmdLine)
{
	// split the command line into a command and a command tail (or arg set)
	CSString cmdTail=cmdLine;
	CSString cmd=cmdTail.firstWord(true).strip();

	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		if (cmd==_InfoExtractors[i]->getName())
		{
			nlinfo("Adding info extractor: %s",cmdLine.c_str());
			return _InfoExtractors[i]->build(cmdTail);
		}
	}

	nlwarning("Unknown info extractor '%s' in line: %s",cmd.c_str(),cmdLine.c_str());
	return NULL;
}
