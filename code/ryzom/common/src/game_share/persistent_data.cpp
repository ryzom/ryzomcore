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

/*
  NOTES:

  Strings are limited to 256 characters - this is OK for the basic uses of the system - may require review later...
  - Note that for many apps use of a string vector in place of a string will solve the 255 limit.

  => deprecated: the test checking the string length has been disabled, which allows string to exceed a 256 char length
*/

//-------------------------------------------------------------------------
// incudes
//-------------------------------------------------------------------------

#include "stdpch.h"
#include "utils.h"
#include "persistent_data.h"
#include "persistent_data_tree.h"

#ifdef NL_OS_WINDOWS
#include <io.h>
#endif


//-------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------

using namespace NLMISC;
using namespace std;

CPdrTokenRegistry *CPdrTokenRegistry::_Instance = NULL;


//-------------------------------------------------------------------------
// globals
//-------------------------------------------------------------------------

CPersistentDataRecord::CArg CPersistentDataRecord::TempArg;


//-------------------------------------------------------------------------
// basics...
//-------------------------------------------------------------------------

// ctor
CPersistentDataRecord::CPersistentDataRecord(const std::string& tokenFamily)
{
	// setup the token family
	_TokenFamily=tokenFamily;

	// clear write data/ properties
	clear();

	// clear read data/ properties
	rewind();
}

//-------------------------------------------------------------------------
// set of accessors for storing data in a CPersistentDataRecord
//-------------------------------------------------------------------------

void CPersistentDataRecord::clear()
{
	H_AUTO(CPersistentDataRecordClear);

	// clear persistent data buffers
	_ArgTable.clear();
	_TokenTable.clear();

	// setup the string table from the token faimly's string table
	_StringTable= CPdrTokenRegistry::getInstance()->getStringTable(_TokenFamily);

	// clear working variables and buffers
	_WritingStructStack.clear();
	_LookupTbls.clear();

	// make sure read pointers don't point past end of data
	rewind();

	// slot '0' in string table is reserved
	addString("BAD_STRING");
}

uint16 CPersistentDataRecord::addString(const string& name)
{
//	H_AUTO(CPersistentDataRecordAddString);

	// store the length of the input string for speed of access (just to help the optimiser do its job)
	uint32 len=	(uint32)name.size();

	//Disabled to allow >=256 char strings.
	//DROP_IF(len>=256,"Attempt to add a string of > 256 characters to the string table",return 0);

	// depending on the string length choose a well suited algorithm for performing a fast search of the string table
	switch(len)
	{
	case 0:
		// run through the existing strings looking for a match
		for (uint32 i=(uint32)_StringTable.size();i--;)
		{
			if (_StringTable[i].empty())
				return (uint16)i;
		}
		break;

	case 1:
		{
			char c0= name[0];	// first and only char of name

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && s[0]==c0)
					return (uint16)i;
			}
		}
		break;

	case 2:
		{
			uint16 c01= *(uint16*)&name[0];	// first and only 2 chars of name

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint16*)&s[0]==c01) )
					return (uint16)i;
			}
		}
		break;

	case 3:
		{
			uint16 c01= *(uint16*)&name[0];	// first 2 chars of name
			char c2= name[2];				// third and final char of name

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint16*)&s[0]==c01)  && s[2]==c2)
					return (uint16)i;
			}
		}
		break;

	case 4:
		{
			uint32 c0123= *(uint32*)&name[0];	// first and only 4 chars of name

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint32*)&s[0]==c0123) )
					return (uint16)i;
			}
		}
		break;

	case 5:
	case 6:
	case 7:
	case 8:
		{
			uint32 cFirst= *(uint32*)&name[0];		// first 4 chars of name
			uint32 endOffs=len-4;					// offset to last 4 characters of the name
			uint32 cLast= *(uint32*)&name[endOffs];	// last 4 chars of name (touch or overlap with first 4 chars)

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint32*)&s[0]==cFirst) && (*(uint32*)&s[endOffs]==cLast))
					return (uint16)i;
			}
		}
		break;

	case 9:
	case 10:
	case 11:
	case 12:
		{
			uint32 cFirst= *(uint32*)&name[0];		// first 4 chars of name
			uint32 cMid= *(uint32*)&name[4];		// middle 4 chars of name (touch first 4 chars)
			uint32 endOffs=len-4;					// offset to last 4 characters of the name
			uint32 cLast= *(uint32*)&name[endOffs];	// last 4 chars of name (touch or overlap with middle 4 chars)

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint32*)&s[0]==cFirst) && (*(uint32*)&s[endOffs]==cLast) && (*(uint32*)&s[4]==cMid))
					return (uint16)i;
			}
		}
		break;

	case 13:
	case 14:
	case 15:
	case 16:
		{
			uint32 cFirst= *(uint32*)&name[0];		// first 4 chars of name
			uint32 cSecond= *(uint32*)&name[4];		// second 4 chars of name (touch first 4 chars)
			uint32 cThird= *(uint32*)&name[8];		// third 4 chars of name (touch second 4 chars)
			uint32 endOffs=len-4;					// offset to last 4 characters of the name
			uint32 cLast= *(uint32*)&name[endOffs];	// last 4 chars of name (touch or overlap with third 4 chars)

			// run through the existing strings looking for a match
			for (uint32 i=(uint32)_StringTable.size();i--;)
			{
				const string &s= _StringTable[i];
				if (s.size()==len && (*(uint32*)&s[0]==cFirst) && (*(uint32*)&s[endOffs]==cLast)
								  && (*(uint32*)&s[4]==cSecond) && (*(uint32*)&s[8]==cThird) )
					return (uint16)i;
			}
		}
		break;

	default:
		{
			uint32 cFirst= *(uint32*)&name[0];			// first 4 chars of name
			uint32 endOffs=len-4;						// offset to last 4 characters of the name
			uint32* nameEnd= (uint32*)&name[endOffs];	// pointer to the last 4 chars of name
			uint32 cLast= *nameEnd;						// last 4 chars of name (touch or overlap with middle 4 chars)

			// run through the existing strings looking for a match
			for (uint32 i=0;i<_StringTable.size();++i)
			{
				// store a ref to the next string in the string table just to help optimiser do its job
				const string &s= _StringTable[i];

				// if string lengths or first r last dwords  don't match then abort compare
				if (s.size()!=len || (*(uint32*)&s[0]!=cFirst) || (*(uint32*)&s[endOffs]!=cLast) )
					continue;

				uint32* sIt= (uint32*)&s[4];		// iterator for 's' - init to point to second dword of string
				uint32* nameIt= (uint32*)&name[4]; 	// iterator for 'name'

				// run through the strings comparing 4 bytes at a time
				while (*sIt==*nameIt)
				{
					++sIt;
					++nameIt;
					if (nameIt>=nameEnd)
					{
						return (uint16)i;
					}
				}
			}
		}
	}

	// no match found so add this string to the string table and return its index
	{
//		H_AUTO(CPersistentDataRecordAddString_NoMatchFound);

		uint16 result= (uint16)_StringTable.size();
		_StringTable.push_back(name);
		BOMB_IF(result==std::numeric_limits<uint16>::max(),"No more room in string table!!!",_StringTable.pop_back());
		return result;
	}
}

const NLMISC::CSString& CPersistentDataRecord::lookupString(uint32 idx) const
{
	// note that the string table size is never less than 1 as entry 0 is pre-set with the 'invalid string' value
	BOMB_IF(idx>=_StringTable.size(),"Attempting to access past end of string table",return lookupString(0));
	return _StringTable[idx];
}

const NLMISC::CSString& CPersistentDataRecord::getTokenFamily() const
{
	return _TokenFamily;
}


//-------------------------------------------------------------------------
// set of accessors for retrieving data from a CPersistentDataRecord
//-------------------------------------------------------------------------

void CPersistentDataRecord::rewind()
{
	_ArgOffset=0;
	_TokenOffset=0;
	_ReadingStructStack.clear();
}

void CPersistentDataRecord::skipStruct()
{
	DROP_IF(!isStartOfStruct(), "Attempting to skip a struct whereas next token is not a struct", return);
	skipData();
}

void CPersistentDataRecord::skipData()
{
	H_AUTO(CPersistentDataRecordSkipData);

	// if this is a structure then skip the whole thing
	std::vector<uint16> stack;
	stack.reserve(16);
	do
	{
		if (isStartOfStruct())
		{
			stack.push_back(peekNextToken());
			popStructBegin(stack.back());
		}
		else if (isEndOfStruct())
		{
			popStructEnd(stack.back());
			if (!stack.empty())
				stack.pop_back();
		}
		else
		{
			popNextArg(peekNextToken());
		}
	}
	while (!stack.empty() && !isEndOfData());
}

CPDRLookupTbl* CPersistentDataRecord::getLookupTbl(uint32 id) const
{
	return (id>=_LookupTbls.size())? NULL: _LookupTbls[id];
}

void CPersistentDataRecord::setLookupTbl(uint32 id, CPDRLookupTbl* tbl)
{
	// if the container is too small for the id then grow it
	if (id>=_LookupTbls.size())
	{
		// make sure the lookup table id is valid
		nlassert(id<CPDRLookupTbl::getNumLookupTableClasses());
		// grow the container
		_LookupTbls.resize(CPDRLookupTbl::getNumLookupTableClasses(),NULL);
	}

	// make sure we don't already have a lookup table allocated for this slot
	nlassert(_LookupTbls[id]==NULL);

	// store away the new lookup table
	_LookupTbls[id]= tbl;
}

bool CPersistentDataRecord::operator==(const CPersistentDataRecord& other) const
{
	#define RESTORE_STATE_VARS {\
		_ArgOffset=oldArgOffset; _TokenOffset=oldTokenOffset; _ReadingStructStack=oldRSS;\
		other._ArgOffset=otherOldArgOffset; other._TokenOffset=otherOldTokenOffset; other._ReadingStructStack=otherOldRSS;\
		}
	#define RETURN_FALSE { RESTORE_STATE_VARS return false; }
	#define RETURN_TRUE  { RESTORE_STATE_VARS return true;  }

	// record the old values of the state variables (to be restored on exit)
	uint32 oldArgOffset(_ArgOffset);
	uint32 oldTokenOffset(_TokenOffset);
	TReadingStructStack	oldRSS(_ReadingStructStack);

	uint32 otherOldArgOffset(other._ArgOffset);
	uint32 otherOldTokenOffset(other._TokenOffset);
	TReadingStructStack	otherOldRSS(other._ReadingStructStack);

	// reset state variables - this is equivalent to rewind()
	_ArgOffset			=0;
	_TokenOffset		=0;
	_ReadingStructStack.clear();

	other._ArgOffset	=0;
	other._TokenOffset	=0;
	other._ReadingStructStack.clear();

	// iterate over the tokens in our PDRs comparing them as we go...
	while ( !isEndOfData() && !other.isEndOfData() )
	{
		// make sure basic type info for next token matches on both sides
		if ( isStartOfStruct()		!= other.isStartOfStruct()	 )
			RETURN_FALSE
		if ( isEndOfStruct()		!= other.isEndOfStruct()	 )
			RETURN_FALSE
		if ( isTokenWithNoData()	!= other.isTokenWithNoData() )
			RETURN_FALSE
		if ( peekNextTokenName()	!= other.peekNextTokenName() )
			RETURN_FALSE

		// deal with the token
		if (isStartOfStruct())
		{
			// skip start of struct token
			const_cast<CPersistentDataRecord*>(this)->popStructBegin(peekNextToken());
			const_cast<CPersistentDataRecord&>(other).popStructBegin(other.peekNextToken());
		}
		else if (isEndOfStruct())
		{
			// skip end of struct token
			const_cast<CPersistentDataRecord*>(this)->popStructEnd(peekNextToken());
			const_cast<CPersistentDataRecord&>(other).popStructEnd(other.peekNextToken());
		}
		else if (isTokenWithNoData())
		{
			// skip token with no data
			const_cast<CPersistentDataRecord*>(this)->pop(peekNextToken());
			const_cast<CPersistentDataRecord&>(other).pop(other.peekNextToken());
		}
		else
		{
			// get value for token and convert to text for comparison
			// - this allows us to compare sint32(123) with uint8(123) etc correctly
			CSString thisValue;
			CSString otherValue;
			const_cast<CPersistentDataRecord*>(this)->pop(peekNextToken(),thisValue);
			const_cast<CPersistentDataRecord&>(other).pop(other.peekNextToken(),otherValue);
			if (thisValue!=otherValue)
				RETURN_FALSE
		}
	}

	// make sure we're at the end of both data buffers
	if ( !isEndOfData() || !other.isEndOfData() )
		RETURN_FALSE

	// all of the failure tests passed so we can conclude that our structures match
	RETURN_TRUE

	#undef RETURN_TRUE
	#undef RETURN_FALSE
	#undef RESTORE_STATE_VARS
}


//-------------------------------------------------------------------------
// debug methods for retrieving info from pdr records
//-------------------------------------------------------------------------

NLMISC::CSString CPersistentDataRecord::getInfo() const
{
	H_AUTO(CPersistentDataRecordGetInfo);
	return NLMISC::toString("TotalSize=%u TokenCount=%u DataCount=%u StringCount=%u StringSize=%u ValueCount=%u",
		totalDataSize(),_TokenTable.size(),_ArgTable.size(),_StringTable.size(),stringDataSize(),getNumValues());
}

NLMISC::CSString CPersistentDataRecord::getInfoAsCSV() const
{
	H_AUTO(CPersistentDataRecordGetInfoAsCSV);
	return NLMISC::toString("%u,%u,%u,%u,%u,%u",
		totalDataSize(),_TokenTable.size(),_ArgTable.size(),_StringTable.size(),stringDataSize(),getNumValues());
}

const NLMISC::CSString& CPersistentDataRecord::getCSVHeaderLine()
{
	static NLMISC::CSString headerLine="TotalSize,TokenCount,DataCount,StringCount,StringSize,ValueCount";
	return headerLine;
}

uint32 CPersistentDataRecord::getNumValues() const
{
	H_AUTO(CPersistentDataRecordGetNumValues);

	// setup a counter variable to buil our result
	uint32 result=0;

	// record the old values of the state variables (to be restored on exit)
	uint32 oldArgOffset(_ArgOffset);
	uint32 oldTokenOffset(_TokenOffset);
	TReadingStructStack	oldRSS(_ReadingStructStack);

	// reset state variables - this is equivalent to rewind()
	_ArgOffset			=0;
	_TokenOffset		=0;
	_ReadingStructStack.clear();

	// iterate over the tokens in our PDRs comparing them as we go...
	while ( !isEndOfData() )
	{
		uint16 nextToken= peekNextToken();
		CArg::TType nextTokenType= peekNextTokenType();

		// deal with the token
		if (nextTokenType==CArg::STRUCT_BEGIN) // isStartOfStruct()
		{
			// skip start of struct token
			const_cast<CPersistentDataRecord*>(this)->popStructBegin(nextToken);
		}
		else if (nextTokenType==CArg::STRUCT_END) // isEndOfStruct()
		{
			// skip end of struct token
			const_cast<CPersistentDataRecord*>(this)->popStructEnd(nextToken);
		}
		else if (nextTokenType==CArg::FLAG) // isTokenWithNoData()
		{
			// skip token with no data
			const_cast<CPersistentDataRecord*>(this)->pop(nextToken);
			++result;
		}
		else
		{
			// get the next value and discard it immediately
			const_cast<CPersistentDataRecord*>(this)->popNextArg(nextToken);
			++result;
		}
	}

	// restore the original values of teh state variables
	_ArgOffset=oldArgOffset;
	_TokenOffset=oldTokenOffset;
	_ReadingStructStack=oldRSS;

	// return the number of values that we found
	return result;
}


//-------------------------------------------------------------------------
// set of accessors for storing a data record to various destinations
//-------------------------------------------------------------------------

// return the buffer size required to store this record
uint32 CPersistentDataRecord::totalDataSize() const
{
	uint32 result=0;
	result+= sizeof(uint32);						// sizeof 'version number' variable
	result+= sizeof(uint32);						// sizeof 'data buffer size' variable
	result+= sizeof(uint32);						// sizeof 'number of tokens in the token table' variable
	result+= sizeof(uint32);						// sizeof 'number of args in the arg table' variable
	result+= sizeof(uint32);						// sizeof 'number of strings in the string table' variable
	result+= sizeof(uint32);						// sizeof 'string table data size' variable
	result+= (uint32)_TokenTable.size()*sizeof(TToken);		// sizeof the token data
	result+= (uint32)_ArgTable.size()*sizeof(_ArgTable[0]);	// size of the args data
	result+= stringDataSize();						// the data size for the strings in the string table

	return result;
}

// return the buffer size required to store this record
uint32 CPersistentDataRecord::stringDataSize() const
{
	uint32 result=0;
	for (uint32 i=0;i<_StringTable.size();++i)
		result+=(uint32)_StringTable[i].size()+1;			// the data size for the strings in the string table
	return result;
}

bool CPersistentDataRecord::toStream(NLMISC::IStream& dest)
{
	H_AUTO(CPersistentDataRecordWriteToStream);

	#define WRITE(type,what) { type v= (type)(what); dest.serial(v); }
	#define WRITE_BUFF(type,what) dest.serialBuffer( (uint8*)&what[0], sizeof(type) * (uint)what.size() )

	// mark the amount of data in output stream before we start adding pdr contents
	uint32 dataStart= dest.getPos();

	// write the header block
	WRITE(uint32,0);
	WRITE(uint32,totalDataSize());
	WRITE(uint32,_TokenTable.size());
	WRITE(uint32,_ArgTable.size());
	WRITE(uint32,_StringTable.size());
	WRITE(uint32,stringDataSize());

	// write the tokens
	WRITE_BUFF(TToken,_TokenTable);

	// write the arguments
	WRITE_BUFF(uint32,_ArgTable);

	// mark the amount of data in output stream before we start adding string table
	uint32 stringTableStart= dest.getPos();

	// write the string table data
	for (uint32 i=0;i<_StringTable.size();++i)
	{
		WRITE_BUFF(char,_StringTable[i]);
		WRITE(char,0);
	}

	// make sure the info written to the header corresponds to the reality of data written to file
	BOMB_IF(dest.getPos()- stringTableStart!= stringDataSize(), "Error writing pdr string table to output stream", return false);
	BOMB_IF(dest.getPos()- dataStart!= totalDataSize(), "Error writing pdr to output stream", return false);

	#undef WRITE

	return true;
}

bool CPersistentDataRecord::toBuffer(char *dest,uint32 bufferSize)
{
	H_AUTO(CPersistentDataRecordWriteToBuffer);

	BOMB_IF(bufferSize<totalDataSize(),"Buffer too small to write data to",return false);

	uint32 offset=0;
	#define WRITE(type,what) { BOMB_IF(offset+sizeof(type)>bufferSize,"Buffer overflow!",return false); *(type*)&dest[offset]= what; offset+=sizeof(type); }

	// write the header block
	WRITE(uint32,0);
	WRITE(uint32,totalDataSize());
	WRITE(uint32,(uint32)_TokenTable.size());
	WRITE(uint32,(uint32)_ArgTable.size());
	WRITE(uint32,(uint32)_StringTable.size());
	WRITE(uint32,stringDataSize());

	// write the tokens
	for (uint32 i=0;i<_TokenTable.size();++i)
		WRITE(TToken,_TokenTable[i]);

	// write the arguments
	for (uint32 i=0;i<_ArgTable.size();++i)
		WRITE(uint32,_ArgTable[i]);

	// write the string table data
	for (uint32 i=0;i<_StringTable.size();++i)
	{
		for (uint32 j=0;j<_StringTable[i].size();++j)
			WRITE(char,_StringTable[i][j]);
		WRITE(char,0);
	}

	#undef WRITE

	BOMB_IF(offset!=totalDataSize(),"Buffer size calculation doesn't match with data written",return false);
	return true;
}

bool CPersistentDataRecord::toString(std::string& result,TStringFormat stringFormat)
{
	H_AUTO(CPersistentDataRecordWriteToString);

	switch (stringFormat)
	{
	case XML_STRING:	return toXML(result);
	case LINES_STRING:	return toLines(reinterpret_cast<CSString&>(result));
	}

	BOMB("Invalid string format",return false);
}

bool CPersistentDataRecord::toXML(std::string& result)
{
	H_AUTO(CPersistentDataRecordWriteToXMLString);

	// clear out the result string before we begin
	result.clear();

	// build the text buffer
	rewind();
	while (!isEndOfData())
	{
		if (isStartOfStruct())
		{
			// start of a structure block...
			for (uint32 i=0;i<=_ReadingStructStack.size();++i) result+='\t';
			result+= "<";
			result+= lookupString(peekNextToken());
			result+= ">\n";
			popStructBegin(peekNextToken());
		}
		else if (isEndOfStruct())
		{
			// end of a structure block...
			for (uint32 i=0;i<=_ReadingStructStack.size()-1;++i) result+='\t';
			result+= "</";
			result+= lookupString(peekNextToken());
			result+= ">\n";
			popStructEnd(peekNextToken());
		}
		else if (isTokenWithNoData())
		{
			// a standard property without value
			for (uint32 i=0;i<=_ReadingStructStack.size();++i) result+='\t';
			result+= "<";
			result+= lookupString(peekNextToken());
			result+= " ";
			result+= "type=\"";
			result+= CArg::Flag().typeName();
			result+= "\" value=\"";
			result+= "1";
			result+= "\"/>\n";
			pop(peekNextToken());
		}
		else
		{
			// a standard property with value
			string token=	lookupString(peekNextToken());
			string argType=	peekNextArg().typeName();

			CSString argTxt;
			pop(peekNextToken(),argTxt);

			for (uint32 i=0;i<=_ReadingStructStack.size();++i) result+='\t';
			result+= "<";
			result+= token;
			result+= " ";
			result+= "type=\"";
			result+= argType;
			result+= "\" value=\"";
			result+= argTxt.encodeXML(true);
			result+= "\"/>\n";
		}
	}

	result="<xml>\n"+result+"</xml>\n";

	// rewind the read pointer 'cos it's at the end of file
	rewind();

	return true;
}

bool CPersistentDataRecord::toLines(std::string& result)
{
	H_AUTO(CPersistentDataRecordWriteToLinesString);

	// setup a persistent data tree and have it scan our input buffer
	rewind();
	CPersistentDataTree pdt;
	return pdt.readFromPdr(*this) && pdt.writeToBuffer(reinterpret_cast<NLMISC::CSString&>(result));
}

bool CPersistentDataRecord::writeToBinFile(const char* fileName)
{
	H_AUTO(CPersistentDataRecordWriteToBinFile);

	// build the buffer
	uint32 bufSize= totalDataSize();
	vector<char> buffer;
	buffer.resize(bufSize);
	toBuffer(&buffer[0],bufSize);

	// write the buffer to a file
	COFile f;
	bool open = f.open(fileName);
	DROP_IF(!open,NLMISC::toString("Failed to open output file %s",fileName).c_str(),return false);

	// write the binary data to file
	try
	{
		f.serialBuffer((uint8*)&buffer[0],bufSize);
	}
	catch(...)
	{
		DROP(NLMISC::toString("Failed to write output file: %s",fileName),return false);
	}

	// rewind the read pointer 'cos it's at the end of file
	rewind();

	return true;
}

bool CPersistentDataRecord::writeToTxtFile(const char* fileName,TStringFormat stringFormat)
{
	H_AUTO(CPersistentDataRecordWriteToTxtFile);

	// build the output text buffer
	string s;
	toString(s,stringFormat);

	// write the text buffer to a file
	COFile f;
	bool open = f.open(fileName);
	DROP_IF(!open,NLMISC::toString("Failed to open output file %s",fileName).c_str(),return false);

	// write the binary data to file
	try
	{
		f.serialBuffer((uint8*)&s[0],(uint)s.size());
	}
	catch(...)
	{
		DROP(NLMISC::toString("Failed to write output file: %s",fileName),return false);
	}

	// rewind the read pointer 'cos it's at the end of file
	rewind();

	return true;
}

bool CPersistentDataRecord::writeToFile(const char* fileName,TFileFormat fileFormat)
{
	H_AUTO(CPersistentDataRecordWriteToFile);

	switch(fileFormat)
	{
	case BINARY_FILE:
	binary_file:
		nlinfo("saving binary file: %s",fileName);
		return writeToBinFile(fileName);

	case XML_FILE:
	xml_file:
		nlinfo("saving xml file: %s",fileName);
		return writeToTxtFile(fileName,XML_STRING);

	case LINES_FILE:
	lines_file:
		nlinfo("saving line-based txt file: %s",fileName);
		return writeToTxtFile(fileName,LINES_STRING);

	case ANY_FILE:
		{
			if (CSString(fileName).right(4)==".xml") goto xml_file;
			if (CSString(fileName).right(4)==".txt") goto lines_file;
			goto binary_file;
		}
	}
	BOMB("Bad file type supplied to writeToFile() - file not saved: "<<fileName,return false);
}

//-------------------------------------------------------------------------
// set of accessors for retrieving a data record from various sources
//-------------------------------------------------------------------------

bool CPersistentDataRecord::fromBuffer(const char *src,uint32 bufferSize)
{
	H_AUTO(CPersistentDataRecordFromBuffer);

	// the second dword of a bin buffer contains the buffer length so use it to check whether
	// this buffer looks like a correct binary buffer.
	bool isValidBinary=(bufferSize>24 && *(uint32*)&src[4]==bufferSize);

	// ensure that the data is binary... otherwise try to consider it as text
	DROP_IF(!isValidBinary,"Failed to parse buffer due to invalid header",return false);

	// make sure the persistent data record is cleared out before we fill it with data
	clear();
	// Must clear the string table, because read from file (and clear() init it with token family)
	_StringTable.clear();

	uint32 offset=0;
	#define READ(type,what) { DROP_IF(offset+sizeof(type)>bufferSize,"PDR ERROR: Buffer overflow reading: " #type " " #what, clear(); return false); what=*(const type*)&src[offset]; offset+=sizeof(type); }
	#define READBUF(type,count,what) { DROP_IF(offset+sizeof(type)*count>bufferSize,"PDR ERROR: Buffer overflow reading buffer: " #what, clear(); return false); if(count>0) { memcpy(what,&src[offset],sizeof(type)*count); offset+=sizeof(type)*count; } }

	// READ the header block
	uint32 version;		READ(uint32,version);
	uint32 totalSize;	READ(uint32,totalSize);
	uint32 tokenCount;	READ(uint32,tokenCount);
	uint32 argCount;	READ(uint32,argCount);
	uint32 stringCount;	READ(uint32,stringCount);
	uint32 stringsSize;	READ(uint32,stringsSize);

	DROP_IF(version>0,"PDR ERROR: Wrong file format version!",clear();return false);
	DROP_IF(totalSize!=bufferSize,"PDR ERROR: Invalid source data",clear();return false);
	DROP_IF(totalSize!=offset+tokenCount*sizeof(TToken)+argCount*sizeof(uint32)+stringsSize,"PDR ERROR: Invalid source data",clear();return false);

	// READ the tokens
	{
		H_AUTO(CPersistentDataRecordFromBufferTokenTable)
		_TokenTable.resize(tokenCount);
		READBUF(TToken,tokenCount,&_TokenTable[0]);
	}

	// READ the arguments
	{
		H_AUTO(CPersistentDataRecordFromBufferArgTable)
		_ArgTable.resize(argCount);
		READBUF(uint32,argCount,&_ArgTable[0]);
	}

	// READ the string table data
	_StringTable.resize(stringCount);
	DROP_IF( (stringsSize==0) != (stringCount==0) , "PDR ERROR: Invalid string table parameters", clear(); return false);

	if (stringCount!=0)
	{
		H_AUTO(CPersistentDataRecordFromBufferStringTable)
		TStringTable::iterator stringTableIt= _StringTable.begin();
		TStringTable::iterator stringTableEnd= _StringTable.end();
		const char* stringDataIt= (const char*)&src[offset];
		const char* stringDataEnd= (const char*)&src[offset+stringsSize];

		DROP_IF(stringDataEnd[-1]!=0,"PDR ERROR: Last string table entry isn't zero terminated", clear(); return false);

		do
		{
			// prepare to push a new string into the string table
			NLMISC::CSString& theTableEntry= *stringTableIt;
			++stringTableIt;

			// copy out the string
			{
				H_AUTO(CPersistentDataRecordFromBufferStringCopy)
				theTableEntry= stringDataIt;
			}

			// update the string start marker
			stringDataIt+= theTableEntry.size()+1;

			// make sure we haven't run out of string data or string slots
			DROP_IF( (stringTableIt!=stringTableEnd) != (stringDataIt!=stringDataEnd), "PDR ERROR: Invalid string table", clear(); return false);
		}
		while (stringDataIt!=stringDataEnd);

		DROP_IF(stringTableIt!=stringTableEnd,"PDR ERROR: Too few strings found in string table",clear(); return false);
		offset+= stringsSize;
	}

	#undef READBUF
	#undef READ

	BOMB_IF(offset!=totalSize,"Buffer size calculation doesn't match with data written",return false);

	return true;
}

std::string calculateLineAndColumn(const CSString& buff,uint32 index)
{
	uint32 line=1;
	uint32 col=1;
	for (uint32 i=0;i<index;++i)
	{
		switch(buff[i])
		{
		case '\r':	break;					// ignore cr
		case '\n':	++line; col=0; break;	// treat lf
		case '\t':	col=(col+4)&~3; break;	// assume 4 point tab
		default:	++col; break;
		}
	}
	return NLMISC::toString("line %u: col %u: ",line,col);
}

bool CPersistentDataRecord::fromXML(const std::string& s)
{
	H_AUTO(CPersistentDataRecordFromXML);

	// we need to treat our input data as a CSString - we static cast because CSStrings are not supposed
	// to contain any data of their own - they are just a fonctionality wrapper round a std::string
	nlctassert(sizeof(string)==sizeof(CSString));
	const CSString& buff= static_cast<const CSString&>(s);
	uint32 len=(uint32)s.size();

	// make sure the persistent data record is cleared out before we fill it with data
	clear();
	// Must clear the string table, because read from file (and clear() init it with token family)
	_StringTable.clear();

	// we have a buffer of xml-like blocks so we're going to start by chunking it up (from the back)
	vector<CSString> clauses;
	bool clauseOpen=false;
	uint32 clauseEnd = 0;
	for (uint32 i=len;i--;)
	{
		switch(buff[i])
		{
		case '\n': case ' ':  case '\t': case '\r': case 26: break;

		case '>':
			DROP_IF(clauseOpen==true,calculateLineAndColumn(buff,i)+"Found 2 '>'s with no matching '<'",return false);
			clauseOpen=true;
			clauseEnd=i;
			break;

		case '<':
			DROP_IF(clauseOpen==false,calculateLineAndColumn(buff,i)+"Found '<' with no matching '>'",return false);
			clauses.push_back(buff.substr(i+1,clauseEnd-i-1));
			clauseOpen=false;
			break;

		default:
			DROP_IF((uint8)(buff[i])<32,calculateLineAndColumn(buff,i)+NLMISC::toString("Invalid (non-ascii text) character encountered: %i",buff[i]),return false);
			break;
		}
	}

	DROP_IF(clauses.size()<2||clauses[0]!="/xml"||clauses.back()!="xml","Invalid data file - didn't find <xml>..</xml> structure",return false)
	// run through the set of clauses to add them to the data block...
	for (uint32 i=(uint32)clauses.size()-1;--i;)
	{
		// clauses are of four types: <...>=struct_begin  </..>=struct_end, <../>=prop, <!..>=comment
		if (clauses[i].left(1)=="!" || clauses[i].left(1)=="?")
		{
			// comment
		}
		else if (clauses[i].left(1)=="/")
		{
			// struct end
			CSString ss= clauses[i].leftCrop(1);
			pushStructEnd(addString(ss));
		}
		else if (clauses[i].right(1)=="/")
		{
			// prop
			CSString s= clauses[i].rightCrop(1);
			CSString token= s.firstWord(true).strip();
			CSString keyword0= s.strtok("=").strip();
			CSString value0= s.firstWordOrWords(true,false,false);
			s=s.strip();
			CSString keyword1= s.strtok("=").strip();
			CSString value1= s.firstWordOrWords(true,false,false);
			if (keyword0=="value" && keyword1=="type")
			{
				swap(value0,value1);
				swap(keyword0,keyword1);
			}
			DROP_IF(keyword0!="type" || keyword1!="value","Expecting 'type' and 'value' in property - but not found",continue);
			CArg arg(value0,value1.decodeXML(),*this);
			push(addString(token),arg);
		}
		else
		{
			// struct begin
			pushStructBegin(addString(clauses[i]));
		}
	}
	return true;
}

bool CPersistentDataRecord::fromLines(const std::string& inputBuffer)
{
	H_AUTO(CPersistentDataRecordFromLines);

	// make sure the persistent data record is cleared out before we fill it with data
	clear();
	// Must clear the string table, because read from file (and clear() init it with token family)
	_StringTable.clear();

	// setup a persistent data tree and have it scan our input buffer
	CPersistentDataTree pdt;
	return pdt.readFromBuffer(reinterpret_cast<const NLMISC::CSString&>(inputBuffer)) && pdt.writeToPdr(*this);
}

bool CPersistentDataRecord::fromString(const std::string& s)
{
	H_AUTO(CPersistentDataRecordFromString);

	// start by skipping any blank characters at the start of file
	uint32 i;
	for (i=0;i<s.size() && CSString::isWhiteSpace(s[i]);++i)
		{}

	// make sure there are non blank characters in the string
	DROP_IF(i==s.size(),"string is empty",return false);

	// determine whether we have an xml string or a txt string
	// we assume that all xml files begin with a '<' character
	if (s[i]=='<')
		return fromXML(s);
	else
		return fromLines(s);
}

bool CPersistentDataRecord::fromStream(NLMISC::IStream& stream, uint32 size)
{
	H_AUTO(pdrFromStream)

	// setup a string as a buffer to hold the stream data
	CSString buff;
	buff.resize(size);

	// read the file data
	try
	{
		stream.serialBuffer((uint8*)&buff[0],(uint)buff.size());
	}
	catch(...)
	{
		DROP(NLMISC::toString("Failed to read stream input"),return false);
	}

	// the second dword of a bin buffer contains the buffer length so use it to check whether
	// this buffer looks like a correct binary buffer.
	bool isBinary=(size>8 && *(uint32*)&buff[4]==size);
	if (isBinary) return fromBuffer(&buff[0],size);

	// it's not a valid binary file so see whether it looks like a valid text file
	DROP_IF(!buff.isValidText(),"File is binary but 'file size' header entry doesn't match true file size",return false);

	// parse the data as text...
	return fromString(buff);
}

// read from a CMemStream (maybe either binary or text data)
bool CPersistentDataRecord::fromBuffer(NLMISC::IStream& stream)
{
	H_AUTO(pdrFromBuffer)

	// try with a CMemStream
	CMemStream *memStream = dynamic_cast<CMemStream*>(&stream);
	if (memStream != NULL)
	{
		return fromBuffer((const char *)(memStream->buffer()+memStream->getPos()), memStream->length()-memStream->getPos());
	}
	// try with a IFile
	NLMISC::CIFile *fileStream = dynamic_cast<NLMISC::CIFile*>(&stream);
	if (fileStream != NULL)
	{
		return fromStream(*fileStream, fileStream->getFileSize());
	}

	return false;
}


bool CPersistentDataRecord::readFromFile(const char* fileName)
{
	H_AUTO(pdrReadFromFile)

#ifdef NL_OS_WINDOWS

	// open the file
	FILE* inf= fopen(fileName,"rb");
	DROP_IF( inf==NULL, "Failed to open input file "<<fileName, return false);

	// get the file size
	uint32 length= filelength(fileno(inf));

	// allocate a buffer
	CSString buffer;
	buffer.resize(length);

	// read the data
	uint32 blocksRead= (uint32)fread(&buffer[0],length,1,inf);
	fclose(inf);
	DROP_IF( blocksRead!=1, "Failed to read data from file "<<fileName, return false);

	// test whether our data buffer is binary
	bool isBinary=(length>8 && *(uint32*)&buffer[4]==length);
	if (isBinary)
	{
		return fromBuffer(&buffer[0],length);
	}

	// it's not a valid binary file so see whether it looks like a valid text file
	DROP_IF(!buffer.isValidText(),"File is binary but 'file size' header entry doesn't match true file size",return false);

	// parse the data as text...
	return fromString(buffer);

#else

	// open the file
	CIFile f;
	bool open = f.open(fileName);
	DROP_IF( !open, "Failed to open input file "<<fileName, return false);

	// get the file size
	uint32 len= f.getFileSize();

	bool result= fromStream(f, len);
	DROP_IF( !result, "Failed to parse input file "<<fileName, return false);

	return true;

#endif
}

bool CPersistentDataRecord::readFromBinFile(const char* fileName)
{
	H_AUTO(CPersistentDataRecordReadFromBinFile);

	// open the file
	CIFile f;
	bool open = f.open(fileName);
	DROP_IF(!open,NLMISC::toString("Failed to open input file %s",fileName).c_str(),return false)

	// get the file size
	uint32 len=CFile::getFileSize(fileName);

	// setup a string as a buffer to hold the file data
	string s;
	s.resize(len);

	// read the file data
	try
	{
		f.serialBuffer((uint8*)&s[0],(uint)s.size());
	}
	catch(...)
	{
		DROP(NLMISC::toString("Failed to read input file: %s",fileName),return false);
	}

	// parse the buffer contents to re-generate the data
	bool result= fromBuffer(&s[0],(uint32)s.size());
	DROP_IF( !result, "Failed to parse input file "<<fileName, return false);

	return true;
}

bool CPersistentDataRecord::readFromTxtFile(const char* fileName)
{
	H_AUTO(CPersistentDataRecordReadFromTxtFile);

	// open the file
	CIFile f;
	bool open = f.open(fileName);
	DROP_IF(!open,NLMISC::toString("Failed to open input file %s",fileName).c_str(),return false)

	// get the file size
	uint32 len=CFile::getFileSize(fileName);

	// setup a string as a buffer to hold the file data
	CSString buff;
	buff.resize(len);

	// read the file data
	try
	{
		f.serialBuffer((uint8*)&buff[0],(uint)buff.size());
	}
	catch(...)
	{
		DROP(NLMISC::toString("Failed to read input file: %s",fileName),return false);
	}

	// parse the buffer contents to re-generate the data
	bool result= fromString(buff);
	DROP_IF( !result, "Failed to parse input file "<<fileName, return false);

	return true;
}


//-----------------------------------------------------------------------------
// methods & data CPDRLookupTbl
//-----------------------------------------------------------------------------

uint32 CPDRLookupTbl::_NextLookupTblClassId;

CPDRLookupTbl::TEnumValue CPDRLookupTbl::operator[](uint32 idx) const
{
	BOMB_IF(idx>=_TheTbl.size(),"ERROR: Failed to retrieve entry from PDR lookup table (idx is out of bounds) - pdr must be corrupt",return -1);
	return _TheTbl[idx];
}

uint32 CPDRLookupTbl::getNumLookupTableClasses()
{
	return _NextLookupTblClassId;
}


//-----------------------------------------------------------------------------
// methods CPdrTokenRegistry
//-----------------------------------------------------------------------------

CPdrTokenRegistry* CPdrTokenRegistry::getInstance()
{
	// first time the method is called instantiate the singleton object
	if (_Instance==NULL)
		_Instance= new CPdrTokenRegistry;

	// return the pointer to our singleton
	return _Instance;
}

void CPdrTokenRegistry::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}


uint16 CPdrTokenRegistry::addToken(const std::string& family,const std::string& token)
{
	// get the map entry correponding to 'family' (or create a new one if need be)
	CPersistentDataRecord::TStringTable& stringTable= _Registry[family];

	// look for an existing match in the string table
	for (uint16 i=0;i<stringTable.size();++i)
	{
		if (stringTable[i]==token)
			return i;
		BOMB_IF(i>=8191,"Failed to add more then 8192 static token to a pdr string table",return 0);
	}

	// append new entry to the string table and return the new index
	stringTable.push_back(token);
	return (uint16)stringTable.size()-1;
}

const CPersistentDataRecord::TStringTable& CPdrTokenRegistry::getStringTable(const std::string& family)
{
	// return the map entry correponding to 'family' (creating a new one if need be)
	return _Registry[family];
}

CPdrTokenRegistry::CPdrTokenRegistry()
{
}

