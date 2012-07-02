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

/**
  This file contains a system for loading and saving persistent data in a robust manor
  The system is hierarchical and all data elements are tagged.

  The system supports saving to text files and to binary files

  NOTE: The text file format is similar to XML but is not currently XML feature-complete

*/

#ifndef PERSISTENT_DATA_H
#define	PERSISTENT_DATA_H

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/sstring.h"
#include "utils.h"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------
// Global Macros
//-----------------------------------------------------------------------------

// The following macros define methods that are instantiated using the
// 'persistent_data_template.h' header file

#define DECLARE_PERSISTENCE_APPLY_METHOD\
	void apply(CPersistentDataRecord &pdr);

#define DECLARE_VIRTUAL_PERSISTENCE_APPLY_METHOD\
	virtual void apply(CPersistentDataRecord &pdr);

#define DECLARE_PURE_VIRTUAL_PERSISTENCE_APPLY_METHOD\
	virtual void apply(CPersistentDataRecord &pdr) =0;


#define DECLARE_PERSISTENCE_STORE_METHOD\
	void store(CPersistentDataRecord &pdr) const;

#define DECLARE_VIRTUAL_PERSISTENCE_STORE_METHOD\
	virtual void store(CPersistentDataRecord &pdr) const;

#define DECLARE_PURE_VIRTUAL_PERSISTENCE_STORE_METHOD\
	virtual void store(CPersistentDataRecord &pdr) const =0;


#define DECLARE_PERSISTENCE_METHODS\
	DECLARE_PERSISTENCE_APPLY_METHOD\
	DECLARE_PERSISTENCE_STORE_METHOD

#define DECLARE_VIRTUAL_PERSISTENCE_METHODS\
	DECLARE_VIRTUAL_PERSISTENCE_APPLY_METHOD\
	DECLARE_VIRTUAL_PERSISTENCE_STORE_METHOD

#define DECLARE_PURE_VIRTUAL_PERSISTENCE_METHODS\
	DECLARE_PURE_VIRTUAL_PERSISTENCE_APPLY_METHOD\
	DECLARE_PURE_VIRTUAL_PERSISTENCE_STORE_METHOD


#define DECLARE_PERSISTENCE_METHODS_WITH_STORE_ARG(arg)\
	void store(CPersistentDataRecord &pdr,arg) const;\
	void apply(CPersistentDataRecord &pdr);

#define DECLARE_PERSISTENCE_METHODS_WITH_APPLY_ARG(arg)\
	void store(CPersistentDataRecord &pdr) const;\
	void apply(CPersistentDataRecord &pdr,arg);

#define DECLARE_PERSISTENCE_METHODS_WITH_ARG(arg)\
	void store(CPersistentDataRecord &pdr,arg) const;\
	void apply(CPersistentDataRecord &pdr,arg);


#define DECLARE_PERSISTENCE_METHODS_WITH_TARGET(arg)\
	void store(CPersistentDataRecord &pdr,const arg) const;\
	void apply(CPersistentDataRecord &pdr,arg);


// A friendly macro to place before each inclusion of persistent_data_template.h to ease debug
#define PERSISTENT_GENERATION_MESSAGE \
	NL_LOC_MSG "generating persistence code for " NL_MACRO_TO_STR(PERSISTENT_CLASS)


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------

class CPersistentDataRecord;
class CPDRLookupTbl;

#pragma pack(push , 4)


//-----------------------------------------------------------------------------
// class CPersistentDataRecord
//-----------------------------------------------------------------------------

class CPersistentDataRecord
{
public:
	//-------------------------------------------------------------------------
	// public data structures
	//-------------------------------------------------------------------------

	typedef uint16 TToken;
	typedef std::vector<NLMISC::CSString> TStringTable;


private:
	//-------------------------------------------------------------------------
	// private data structures
	//-------------------------------------------------------------------------

	struct CArg
	{
		/*** NOTE: Tokens are coded on 3 bits so there should never be more than 8 ***/
		enum
		{
			BEGIN_TOKEN,
			END_TOKEN,
			SINT_TOKEN,
			UINT_TOKEN,
			FLOAT_TOKEN,
			STRING_TOKEN,
			FLAG_TOKEN,
			EXTEND_TOKEN
		};

		enum TType
		{
			STRUCT_BEGIN,
			STRUCT_END,
			FLAG,
			SINT32,
			UINT32,
			FLOAT32,
			STRING,
			SINT64,
			UINT64,
			FLOAT64,
			EXTEND_TYPE,
			NB_TYPE
		} _Type;

		enum TExtendType
		{
			ET_SHEET_ID,

			ET_64_BIT_EXTENDED_TYPES = 0x80000000,
			ET_ENTITY_ID = ET_64_BIT_EXTENDED_TYPES
		};

		union
		{
			struct
			{
				uint32	i32_1;
				uint32	i32_2;
			};

			sint32	i32;
			sint64	i64;
			float	f32;
			double	f64;

			struct
			{
				uint32	ExType;
				union
				{
					struct
					{
						uint32	ex32_1;
						uint32	ex32_2;
					};

					uint32	ExData32;
					uint64	ExData64;
				};
			};
		} _Value;
		NLMISC::CSString _String;


		CArg();
		CArg(const std::string& type,const std::string& value,CPersistentDataRecord& pdr);

		uint64 asUint() const;
		sint64 asSint() const;
		float asFloat() const;
		double asDouble() const;
		NLMISC::CSheetId asSheetId() const;
		NLMISC::CEntityId asEntityId() const;
		NLMISC::CSString asString() const;
		ucstring asUCString() const;
		NLMISC::CSString typeName() const;

		bool setType(const std::string &typeName);
		void setType(CArg::TType value);
		bool isExtended() const;
		bool isFlag() const;

		void push(TToken token, std::vector<TToken>& tokenTable, std::vector<uint32>& argTable) const;

		static CArg EntityId(NLMISC::CEntityId val);
		static CArg SheetId(NLMISC::CSheetId val);
		static CArg Int32(sint32 val);
		static CArg Int32(uint32 val);
		static CArg Int64(sint64 val);
		static CArg Int64(uint64 val);
		static CArg Float32(float val);
		static CArg Float64(double val);
		static CArg String(const std::string& value,CPersistentDataRecord& pdr);
		static CArg UCString(const ucstring& value,CPersistentDataRecord& pdr);
		static CArg Flag();

		static TType  token2Type(uint32 token,bool extend=false);
		static TToken type2Token(uint32 type);
		static bool   isTypeExtended(uint32 type);
	};

public:
	//-------------------------------------------------------------------------
	// class basics
	//-------------------------------------------------------------------------

	// ctor
	CPersistentDataRecord(const std::string& tokenFamily=std::string());


	//-------------------------------------------------------------------------
	// set of methods for storing data in a CPersistentDataRecord
	//-------------------------------------------------------------------------

	// clear out all data from the record
	void clear();

	// accessors for the string table
	// note that the second variant of addString() is often faster as it checks
	// whether the value of 'result' is already correct before looking further...
	// note that the tokenFamily is used to setup the initial state of the string table (at init time)
	uint16 addString(const std::string& name);
	void addString(const std::string& name,uint16 &result);
	void addString(const char* name,uint16 &result);
	const NLMISC::CSString& lookupString(uint32 idx) const;
	const NLMISC::CSString& getTokenFamily() const;

	// deal with the next data element (checking that the token matches)

	void push(TToken token,const CArg& arg);
	void push(TToken token,bool val);

	void push(TToken token,sint8 val);
	void push(TToken token,sint16 val);
	void push(TToken token,sint32 val);
	void push(TToken token,sint64 val);

	void push(TToken token,uint8 val);
	void push(TToken token,uint16 val);
	void push(TToken token,uint32 val);
	void push(TToken token,uint64 val);

	void push(TToken token,float val);
	void push(TToken token,double val);
	void push(TToken token,const std::string& val);
	void push(TToken token,const ucstring& val);

	void push(TToken token,NLMISC::CSheetId val);
	void push(TToken token,const NLMISC::CEntityId& val);

	void push(TToken token);	// for flag token that takes no args


	// deal with start and end of a 'structure' data block

	void pushStructBegin(TToken token);
	void pushStructEnd(TToken token);


	//-------------------------------------------------------------------------
	// set of methods for retrieving data from a CPersistentDataRecord
	//-------------------------------------------------------------------------

	// reset the read pointer to the start of the input data
	void rewind();

	bool isEndOfData() const;
	bool isEndOfStruct() const;
	bool isStartOfStruct() const;
	bool isTokenWithNoData() const;

	TToken peekNextToken() const;
	const NLMISC::CSString& peekNextTokenName() const;
	CArg::TType peekNextTokenType() const;

	void peekNextArg(CArg& result) const;
	const CArg& peekNextArg() const;

	void popNextArg(TToken token,CArg& result);
	const CArg& popNextArg(TToken token);

	void pop(TToken token,bool& result);

	void pop(TToken token,sint8& result);
	void pop(TToken token,sint16& result);
	void pop(TToken token,sint32& result);
	void pop(TToken token,sint64& result);

	void pop(TToken token,uint8& result);
	void pop(TToken token,uint16& result);
	void pop(TToken token,uint32& result);
	void pop(TToken token,uint64& result);

	void pop(TToken token,float& result);
	void pop(TToken token,double& result);
	void pop(TToken token,std::string& result);
	void pop(TToken token,ucstring& result);

	void pop(TToken token,NLMISC::CSheetId& result);
	void pop(TToken token,NLMISC::CEntityId& result);

	void pop(TToken token);	// for flag token that takes no args

	void popStructBegin(TToken token);
	void popStructEnd(TToken token);
	void skipStruct();					// Added by Ben, will skip a whole struct in record
	void skipData();					// Added by Ben, will skip next record, a struct as well as a simple token

	// accessors for the _LookupTbls container
	CPDRLookupTbl* getLookupTbl(uint32 id) const;
	void setLookupTbl(uint32 id, CPDRLookupTbl* tbl);

	// compare two persistent data records (verify that their contents are eqivalent)
	bool operator==(const CPersistentDataRecord& other) const;


	//-------------------------------------------------------------------------
	// debug methods for retrieving info from pdr records
	//-------------------------------------------------------------------------

	// get info as a single text string including field names and values in a man-readable format
	NLMISC::CSString getInfo() const;
	// get info as a string of comma separated values
	NLMISC::CSString getInfoAsCSV() const;
	// get the header row that correspond to the comma separated values in getInfoAsCSV() - note that this method is static
	static const NLMISC::CSString& getCSVHeaderLine();
	// get the number of values in a pdr (a value is a string, a uint32, uint64, CEntitiyId, flag or other such primitive)
	uint32 getNumValues() const;


	//-------------------------------------------------------------------------
	// API enums
	//-------------------------------------------------------------------------

	enum TFileFormat { BINARY_FILE, XML_FILE, LINES_FILE, ANY_FILE };
	enum TStringFormat { XML_STRING=XML_FILE, LINES_STRING=LINES_FILE };


	//-------------------------------------------------------------------------
	// set of methods for storing a data record to various destinations
	//-------------------------------------------------------------------------

	uint32 totalDataSize() const;
	uint32 stringDataSize() const;

	// the following routines save the data in different ways
	// NOTE: they are all inclined to rewind the 'read pointer'

	// write the contents of the pdr to a stream
	bool toStream(NLMISC::IStream& dest);

	// write data to a binary buffer - the size required for the buffer is totalDataSize()
	bool toBuffer(char *dest,uint32 sizeLimit);

	// write data to a text buffer in one of several txt formats
	bool toString(std::string& result,TStringFormat stringFormat=XML_STRING);

	// write data to a text buffer in an xml format
	bool toXML(std::string& result);

	// write data to a text buffer in line-based txt format
	bool toLines(std::string& result);

	// perform a toBuffer() and write the result to a binary file
	bool writeToBinFile(const char* fileName);

	// perform a toString() and write the result to a text file
	bool writeToTxtFile(const char* fileName,TStringFormat stringFormat=XML_STRING);

	// if the format is set to 'ANY_FILE' then use the extension provided in the 'fileName' argument to
	// determine the file type. In this case 'txt' and 'xml' have specific meanings
	// returns writeToTxtFile(...) or writeToBinFile(...) depending on the file format
	bool writeToFile(const char* fileName,TFileFormat fileFormat=ANY_FILE);


	//-------------------------------------------------------------------------
	// set of methods for retrieving a data record from various sources
	//-------------------------------------------------------------------------

	// read from a CMemStream (maybe either binary or text data)
	bool fromBuffer(NLMISC::IStream& stream);

	// read from a binary data buffer
	// if the input data looks like text then calls fromString()
	bool fromBuffer(const char *src,uint32 bufferSize);

	// read from a text string (either xml or lines)
	// note 1: This routine is not at all optimised
	// note 2: The content of s is destroyed by the routine
	bool fromString(const std::string& s);

	// read from an xml string
	// note 1: This routine is not at all optimised
	// note 2: The content of s is destroyed by the routine
	bool fromXML(const std::string& s);

	// read from a lines string
	// note 1: This routine is not at all optimised
	// note 2: The content of s is destroyed by the routine
	bool fromLines(const std::string& s);

	// read from a binary file
	bool readFromBinFile(const char* fileName);

	// read from a text file
	bool readFromTxtFile(const char* fileName);

	// read a file and determine whether it's a binary or text file from it's
	// content - then behave like readFromBinFile() or readFromTxtFile()
	bool readFromFile(const char* fileName);


private:

	bool	fromStream(NLMISC::IStream& stream, uint32 size);

	//-------------------------------------------------------------------------
	// private persistent data
	//-------------------------------------------------------------------------

	TStringTable _StringTable;
	std::vector<uint32> _ArgTable;
	std::vector<TToken> _TokenTable;


	//-------------------------------------------------------------------------
	// private work data - for writing
	//-------------------------------------------------------------------------

	NLMISC::CSString	_TokenFamily;
	std::vector<TToken> _WritingStructStack;


	//-------------------------------------------------------------------------
	// private work data - for reading
	//-------------------------------------------------------------------------

	mutable uint32 _ArgOffset;
	mutable uint32 _TokenOffset;

	typedef std::vector<TToken> TReadingStructStack;
	mutable TReadingStructStack _ReadingStructStack;

	typedef std::vector< NLMISC::CSmartPtr<CPDRLookupTbl> > TLookupTbls;
	TLookupTbls _LookupTbls;


	//-------------------------------------------------------------------------
	// globals
	//-------------------------------------------------------------------------
	static CArg TempArg;
};


//-----------------------------------------------------------------------------
// class CPersistentDataRecordRyzomStore
//-----------------------------------------------------------------------------
// This is just a specialisation of the class that register it into the "RyzomTokenFamily"
// Use it only to store (else useless copy of default string table at clear())

class CPersistentDataRecordRyzomStore : public CPersistentDataRecord
{
public:
	CPersistentDataRecordRyzomStore() : CPersistentDataRecord("RyzomTokenFamily") {}
};


//-----------------------------------------------------------------------------
// class CPDRLookupTbl
//-----------------------------------------------------------------------------

class CPDRLookupTbl: public NLMISC::CRefCount
{
public:
	// the data type used to represent enum values
	typedef sint16 TEnumValue;

	// the [] operator is used to lookup entries in the lookup table
	// in debug mode if the index 'idx' falls outside the table we assert
	// in release mode if the index 'idx' falls outside the table we return -1; (This is the 'Bad String' value)
	TEnumValue operator[](uint32 idx) const;

	// get the number of lookup table classes registered so far
	static uint32 getNumLookupTableClasses();

protected:
	// a static used by derived classes to setup unique ids
	static uint32 _NextLookupTblClassId;

	// table mapping from pdr string table indices to enum values
	typedef std::vector<TEnumValue> TTheTbl;
	TTheTbl _TheTbl;
};


//-----------------------------------------------------------------------------
// class CPdrTokenRegistry
//-----------------------------------------------------------------------------
// This is a singleton that can be used to setup families of tokens that
// are copied directly into a new pdr's string table at init time
// This class

class CPdrTokenRegistry
{
public:
	// singleton instantiation
	static CPdrTokenRegistry* getInstance();

	// release memory
	static void releaseInstance();

	// adding a token to the string table
	uint16 addToken(const std::string& family,const std::string& token);

	// get hold of the string table for a given family
	const CPersistentDataRecord::TStringTable& getStringTable(const std::string& family);

private:
	// private data
	typedef std::map<NLMISC::CSString,CPersistentDataRecord::TStringTable> TRegistry;
	TRegistry _Registry;

private:
	static CPdrTokenRegistry *_Instance;
	// this is a singleton so prohibit creation
	CPdrTokenRegistry();
};


#pragma pack(pop)

#include "persistent_data_inline.h"

//-----------------------------------------------------------------------------
#endif
