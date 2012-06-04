// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_SHEET_ID_H
#define NL_SHEET_ID_H

// misc
#include "types_nl.h"
#include "stream.h"
#include "static_map.h"

// std
#include <string>
#include <map>

namespace NLMISC {

#ifdef NL_DEBUG
#	define NL_DEBUG_SHEET_ID
#endif

// Use 24 bits id and 8 bits file types
#define NL_SHEET_ID_ID_BITS		24
#define NL_SHEET_ID_TYPE_BITS	32 - NL_SHEET_ID_ID_BITS

/**
 * CSheetId
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CSheetId
{

public :
	/// Unknow CSheetId is similar as an NULL pointer.
	static const CSheetId Unknown;

	/**
	 *	Constructor
	 */
	explicit CSheetId( uint32 sheetRef = 0 );

	/**
	 *	Constructor
	 */
	explicit CSheetId( const std::string& sheetName );

	/**
	 * Constructor, uses defaultType as extension when sheetName
	 * contains no file extension.
	 */
	explicit CSheetId( const std::string& sheetName, const std::string &defaultType );

	// build from a string and returns true if the build succeed
	bool	 buildSheetId(const std::string& sheetName);

	// build from a SubSheetId and a type
	void	 buildSheetId(uint32 shortId, uint32 type);

	/**
	 *	Load the association sheet ref / sheet name
	 */
	static void init(bool removeUnknownSheet = true);

	/**
	 *	Init the sheet id to work without knowlege of sheet name
	 */
	static void initWithoutSheet();

	/**
	 * Remove all allocated memory
	 */
	static void uninit();

	/**
	 * Return the **whole** sheet id (id+type)
	 */
	uint32 asInt() const { return _Id.Id; }

	/**
	 * Return the sheet type (sub part of the sheetid)
	 */
	uint32 getSheetType() const { return _Id.IdInfos.Type; }

	/**
	 * Return the sheet sub id (sub part of the sheetid)
	 */
	uint32 getShortId() const { return _Id.IdInfos.Id; }

	/**
	 *	Operator=
	 */
	CSheetId& operator=( const CSheetId& sheetId );

	/**
	 *	Operator=
	 */
	CSheetId& operator=( const std::string& sheetName );

	/**
	 *	Operator=
	 */
	CSheetId& operator=( uint32 sheetRef );

	/**
	 *	Operator<
	 */
	bool operator < (const CSheetId& sheetRef ) const;

	/**
	 *	Operator==
	 */
	inline bool operator == (const CSheetId& sheetRef ) const { return ( _Id.Id == sheetRef._Id.Id) ; }

	/**
	 *	Operator !=
	 */
	inline bool operator != (const CSheetId& sheetRef ) const { return (_Id.Id != sheetRef._Id.Id) ; }




	/**
	 * Return the sheet id as a string
	 * If the sheet id is not found, then:
	 * - if 'ifNotFoundUseNumericId==false' the returned string is "<Sheet %d not found in sheet_id.bin>" with the id in %d
	 * - if 'ifNotFoundUseNumericId==tue'   the returned string is "#%u" with the id in %u
	 */
	std::string toString(bool ifNotFoundUseNumericId=false) const;

	/**
	 *	Serial
	 */
	void serial(NLMISC::IStream	&f) throw(NLMISC::EStream);
	void serialString(NLMISC::IStream &f, const std::string &defaultType = "") throw(NLMISC::EStream);

	/**
	 *  Display the list of valid sheet ids with their associated file names
	 *  if (type != -1) then restrict list to given type
	 */
	static void display();
	static void display(uint32 type);

	/**
	 *  Generate a vector of all the sheet ids of a given type
	 *  This operation is non-destructive, the new entries are appended to the result vector
	 *  note: fileExtension *not* include the '.' eg "bla" and *not* ".bla"
	 **/
	static void buildIdVector(std::vector <CSheetId> &result);
	static void buildIdVector(std::vector <CSheetId> &result, uint32 type);
	static void buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames, uint32 type);
	static void buildIdVector(std::vector <CSheetId> &result, const std::string &fileExtension);
	static void buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames, const std::string &fileExtension);

	/**
	 *  Convert between file extensions and numeric sheet types
	 *  note: fileExtension *not* include the '.' eg "bla" and *not* ".bla"
	 **/
	static const std::string &fileExtensionFromType(uint32 type);
	static uint32 typeFromFileExtension(const std::string &fileExtension);

private :

	/// sheet id
	union TSheetId
	{
		uint32		Id;

		struct
		{
			uint32	Type	: NL_SHEET_ID_TYPE_BITS;
			uint32	Id		: NL_SHEET_ID_ID_BITS;
		} IdInfos;
	};
	TSheetId _Id;

#ifdef NL_DEBUG_SHEET_ID
	// Add some valuable debug information to sheetId
	const char	*_DebugSheetName;
#endif

	/// associate sheet id and sheet name
	//static std::map<uint32,std::string> _SheetIdToName;
	//static std::map<std::string,uint32> _SheetNameToId;

	class CChar
	{
	public:
		char *Ptr;
		CChar() { Ptr = NULL; }
		CChar(const CChar& c) { Ptr = c.Ptr; } // WARNING : Share Pointer
	};

	class CCharComp
	{
	public:
		bool operator()(CChar x, CChar y) const
		{
			return strcmp(x.Ptr, y.Ptr) < 0;
		}
	};

	static CChar _AllStrings;
	static CStaticMap<uint32, CChar> _SheetIdToName;
	static CStaticMap<CChar,uint32, CCharComp> _SheetNameToId;

	static std::vector<std::string> _FileExtensions;
	static bool _Initialised;

	static bool _RemoveUnknownSheet;

	static void loadSheetId ();
	static void loadSheetAlias ();
	static void cbFileChange (const std::string &filename);

	/**
	 * When initialized without sheet_id.bin, the sheet id are assigned 
	 * dynamically. Separate maps are used, because in sheet_id.bin 
	 * mode it uses static maps optimized during load.
	 */
	static bool _DontHaveSheetKnowledge;
	static std::map<std::string, uint32> _DevTypeNameToId;
	/// outer vector is type, inner vector is sheet id
	static std::vector<std::vector<std::string> > _DevSheetIdToName;
	static std::map<std::string, uint32> _DevSheetNameToId;
};


/**
 * Class to be used as a hash traits for a hash_map accessed by CSheetId
 * Ex: hash_map< CSheetId, CMyData, CSheetIdHashMapTraits> _MyHashMap;
 */
class CSheetIdHashMapTraits
{
public:
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
	inline size_t operator() ( const CSheetId& sheetId ) const
	{
		return sheetId.asInt() >> 5;
	}
	bool operator() (const CSheetId &strId1, const CSheetId &strId2) const
	{
		return strId1.asInt() < strId2.asInt();
	}
};

} // NLMISC

#endif // NL_SHEET_ID_H

/* End of sheet_id.h */
