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

#ifndef NL_DB_DESCRIPTION_PARSER_H
#define NL_DB_DESCRIPTION_PARSER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/sha1.h>

#include "pds_types.h"


class CTypeNode
{
public:

	enum
	{
		TypeSimple,
		TypeEnum,
		TypeDimension
	};

	std::string										Name;
	uint											Id;
	TDataType										DataType;
	uint32											ByteSize;
	uint32											Type;
	uint32											Dimension;

	std::vector<std::pair<std::string, uint32> >	EnumValues;

	std::string			getEnumName(uint32 value) const
	{
		for (uint i=0; i<EnumValues.size(); ++i)
			if (EnumValues[i].second == value)
				return EnumValues[i].first;
		return "<UNKNOWN>";
	}
};

class CAttributeNode
{
public:
	std::string										Name;
	uint											Id;
	uint											ColumnId;
	uint											Columns;
	TMetaType										MetaType;
	uint											TypeId;
	uint											Index;
	uint											Reference;
	bool											AllowNull;
};

class CColumnNode
{
public:

	uint											Index;
	std::string										Name;

	TDataType										DataType;
	uint											TypeId;
	uint											ByteSize;
};

class CTableNode
{
public:

	CTableNode() : ColumnsBuilt(false)				{ }

	std::string										Name;
	uint											Id;
	sint											Key;
	sint											Inherit;
	sint											Mapped;
	std::vector<CAttributeNode>						Attributes;
	std::vector<CColumnNode>						Columns;

	bool											ColumnsBuilt;
};

// Added so strings can be logged in logmsg or logcontext
enum
{
	ExtLogTypeString = 0xffff0000,
};

class CLogNode
{
public:

	uint											Id;
	bool											Context;

	class CParameter
	{
	public:

		uint32										DataType;
		uint32										TypeId;
		uint32										ByteOffset;
		uint32										ByteSize;
	};

	std::vector<CParameter>							Parameters;
	std::string										Message;
};

class CDatabaseNode
{
public:
	std::string										Name;

	std::vector<CTypeNode>							Types;
	std::vector<CTableNode>							Tables;
	std::vector<CLogNode>							Logs;
};




/**
 * Parse an xml input stream and setup a database
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDBDescriptionParser
{
public:

	/// Constructor
	CDBDescriptionParser();

	/**
	 * Load database description
	 */
	bool			loadDescriptionFile(const std::string& filename);

	/**
	 * Load database description
	 */
	bool			loadDescription(const uint8* description);

	/**
	 * Save database description
	 */
//	bool			saveDescription(const std::string& filename);

	/**
	 * Get HashKey
	 */
	const CHashKey&	getHashKey() const				{ return _HashKey; }

	/**
	 * Get Description
	 */
	std::string&	getDescription()				{ return _Description; }


	/**
	 * Get Database Node
	 */
	const CDatabaseNode	&getDatabaseNode() const	{ return _Database; }


	/**
	 * Build Columns
	 */
	bool			buildColumns();


	/**
	 * Display whole database node info
	 */
	void			display(NLMISC::CLog& log) const;

	/**
	 * Display Table node info
	 */
	void			displayTable(uint table, NLMISC::CLog& log) const;


private:

	/// Database description
	std::string										_Description;

	/// Description hashkey
	CHashKey										_HashKey;

	/// Database node
	CDatabaseNode									_Database;



	/**
	 * Load database description
	 */
	bool			loadDescription(NLMISC::CIXml& xmlStream);

	/**
	 * Load Database description node
	 */
	bool			loadDatabase(xmlNodePtr node);

	/**
	 * Load Type description node
	 */
	bool			loadType(xmlNodePtr node);

	/**
	 * Load Table description node
	 */
	bool			loadTable(xmlNodePtr node);

	/**
	 * Load attribute description node
	 */
	bool			loadAttribute(xmlNodePtr node, CTableNode& table);

	/**
	 * Load log description node
	 */
	bool			loadLog(xmlNodePtr node);

	/**
	 * Build Table Columns
	 */
	bool			buildColumns(uint tableIndex);
};


#endif // NL_DB_DESCRIPTION_PARSER_H

/* End of db_description_parser.h */
