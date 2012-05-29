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

#include "nel/misc/variable.h"
#include "nel/misc/path.h"

#include "game_share/persistent_data.h"

#include "character_scan_job.h"
#include "character.h"


//-------------------------------------------------------------------------------------------------
// Variables
//-------------------------------------------------------------------------------------------------

extern NLMISC::CVariable<std::string> OutputDirectory;


//-------------------------------------------------------------------------------------------------
// methods CCharacterScanJob
//-------------------------------------------------------------------------------------------------

CCharacterScanJob::CCharacterScanJob()
{
	// start by initialising simple properties
	_CharTblFile=NULL;
	_NextFile=0;
	_State=INIT;

	// setup the special reserved table columns 'account' and 'accountSlot'
	charTblAddCol("account");
	charTblAddCol("accountSlot");

	// open the output file for the character table
	std::string filename= "char_tbl.csv";
	_CharTblFile=fopen(filename.c_str(),"wb");
	if (_CharTblFile==NULL)
	{
		nlwarning("Failed to open output file: %s",filename.c_str());
		_State=ERROR;
	}
}

CCharacterScanJob::~CCharacterScanJob()
{
	if (_State!=ERROR)
		_State=CLOSED;

	if (_CharTblFile!=NULL)
		fclose(_CharTblFile);

	// flush the stats maps to their respective output files
	for (TCharStatsMap::iterator it=_CharStatsMap.begin();it!=_CharStatsMap.end();++it)
	{
		// create the output file name and open the file for writing
		std::string filename="char_stats_"+(*it).first+".csv";
		FILE* f=fopen(filename.c_str(),"wb");
		if (f==NULL)
		{
			nlwarning("Failed to open output file: %s",filename.c_str());
			continue;
		}

		// dump data to the file
		for (TCharStatsMapTbl::iterator it2=it->second.begin();it2!=it->second.end();++it2)
		{
			fprintf(f,"%s,%d,\n",it2->first.c_str(),it2->second);
		}

		// the writing is finished so close the file
		fclose(f);
	}
}

void CCharacterScanJob::update()
{
	if (_NextFile>=_Files.size())
		return;

	// load the file into a pdr record
	static CPersistentDataRecord	pdr;
	pdr.clear();
	pdr.readFromFile(_Files[_NextFile].c_str());
	++_NextFile;

	// create a character representation and apply the pdr
	CStatsScanCharacter c;
	c.apply(pdr);

	// iterate over the info extractors executing their core code
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		_InfoExtractors[i]->execute(this,&c);
	}

	// flush the info collected by the info extractors to the output file
	charTblFlushRow(123,456);
}

bool CCharacterScanJob::charTblAddCol(const std::string& name)
{
	nlassert(_State==INIT);

	// make sure the col doesn't already exist in the table
	for (uint32 i=0;i<_TblCols.size();++i)
	{
		nlassert(_TblCols[i]!=name);
	}

	// add the colt ot he table
	_TblCols.push_back(name);

	return true;
}

bool CCharacterScanJob::addInfoExtractor(ICharInfoExtractor* infoExtractor)
{
	// make sure this info extractor doesn't already exist
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		if(_InfoExtractors[i]->toString()== infoExtractor->toString())
		{
			nlwarning("Attempt to add info extractor to the same job more than once: %s",infoExtractor->toString().c_str());
			return false;
		}
	}

	// append the new info extractor to the buffer
	_InfoExtractors.push_back(infoExtractor);

	return true;
}

bool CCharacterScanJob::addFilter(ICharFilter* filter)
{
	// make sure this info extractor doesn't already exist
	for (uint32 i=0;i<_Filters.size();++i)
	{
		if(_Filters[i]->toString()== filter->toString())
		{
			nlwarning("Attempt to add filter to the same job more than once: %s",filter->toString().c_str());
			return false;
		}
	}

	// append the new info extractor to the buffer
	_Filters.push_back(filter);

	return true;
}

bool CCharacterScanJob::addFiles(const CFileDescriptionContainer& fdc)
{
	for (uint32 i=0;i<fdc.size();++i)
	{
		// generate a normalised a full file name with expanded path
		std::string fullFileName= NLMISC::CPath::getFullPath(NLMISC::CFile::getPath(fdc[i].FileName))+NLMISC::CFile::getFilename(fdc[i].FileName);

		// make sure the full file name doesn't already exist in the _Files vector
		uint32 j;
		for (j=0;j<_Files.size();++j)
		{
			if (fullFileName==_Files[j])
				break;
		}
		if (j<_Files.size())
			continue;

		// add the full file name to the _Files vector
		_Files.push_back(fullFileName);
	}
	return true;
}

bool CCharacterScanJob::setOutputPath(const std::string& path)
{
	nlinfo("Setting output path to: %s",path.c_str());
	_OutputPath= NLMISC::CPath::getFullPath(OutputDirectory)+path;
	bool result= NLMISC::CFile::createDirectoryTree(_OutputPath);
	if (result==false)
	{
		nlwarning("Failed to create directory tree: %s",path.c_str());
		return false;
	}
	return true;
}

void CCharacterScanJob::charTblFlushRow(uint32 account,uint32 slot)
{
	// setup the stuff for the ne
	charTblSetEntry("account",NLMISC::toString(account));
	charTblSetEntry("accountSlot",NLMISC::toString(slot));

	nlassert(_State==WORK);

	// build the row text from the _CurrentRowEntries entries and erase the entries as we go
	std::string rowTxt;
	for (uint32 i=0;i<_TblCols.size();++i)
	{
		if (!rowTxt.empty())
			rowTxt+=',';
		rowTxt+=_CurrentRowEntries[_TblCols[i]];
		_CurrentRowEntries.erase(_TblCols[i]);
	}

	// get rid of any excess entries (spew warnings to compain about the problem)
	while (!_CurrentRowEntries.empty())
	{
		nlwarning("Character Tbl entry found for unknown column: %s: %s",(*_CurrentRowEntries.begin()).first.c_str(),(*_CurrentRowEntries.begin()).second.c_str());
		_CurrentRowEntries.erase(_CurrentRowEntries.begin());
	}

	// add the row text to the output file
	fprintf(_CharTblFile,"%s\n",rowTxt.c_str());
	fflush(_CharTblFile);
}

void CCharacterScanJob::charTblSetEntry(const std::string& colName,const std::string& value)
{
	nlassert(_State==WORK);

	// ensure we don't already have a value for this col
	nlassert(_CurrentRowEntries.find(colName)==_CurrentRowEntries.end());

	// det the value for the col
	_CurrentRowEntries[colName]= value;
}

void CCharacterScanJob::freqTblAddEntry(const std::string& tblName, const std::string& key)
{
	nlassert(_State==WORK);

	// if the key doesn't exist in the given freq tbl then init the value to 1 else increment
	if(_CharStatsMap[tblName].find(key)==_CharStatsMap[tblName].end())
		_CharStatsMap[tblName][key]=1;
	else
		++_CharStatsMap[tblName][key];
}

bool CCharacterScanJob::finished()
{
	return _NextFile>= _Files.size();
}

std::string CCharacterScanJob::getShortStatus()
{
	return NLMISC::toString("CharacterFiles %d/%d",_NextFile,_Files.size());
}

std::string CCharacterScanJob::getStatus()
{
	return getShortStatus();
}

void CCharacterScanJob::display(NLMISC::CLog* log)
{
	log->displayNL("%s",getStatus().c_str());
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		log->displayNL("- %s",_InfoExtractors[i]->toString().c_str());
	}
}


