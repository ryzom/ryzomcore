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

#ifndef NL_DATABASE_ADAPTER_H
#define NL_DATABASE_ADAPTER_H

#include "nel/misc/types_nl.h"

#include "pds_database.h"

/**
 * A Database adapter
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDatabaseAdapter
{
public:

	/// Constructor
	CDatabaseAdapter();



	/**
	 * Build the adapter using 2 database description
	 * \param from is the old database to adapt from
	 * \param into is the new database to adapt into
	 */
	bool			build(CDatabase* from, CDatabase* into);


	/**
	 * Build new database content
	 */
	bool			buildContent();


private:

	const CDatabase*	From;
	CDatabase*			Into;

	class CColumnAdapter
	{
	public:

		CColumnAdapter() : From(NULL), Into(NULL)	{ }

		const CColumn*	From;
		const CColumn*	Into;
	};

	class CTableAdapter
	{
	public:

		CTableAdapter() : From(NULL), Into(NULL)	{ }

		const CTable*	From;
		CTable*			Into;

		std::vector<CColumnAdapter>	Columns;
	};

	std::vector<CTableAdapter>	Tables;


	/// Fast remapping for tables, from old to new indices
	std::vector<TTypeId>		TableRemap;

	/// Fast remapping for enums, from old enum value to new enum value
	typedef std::vector<TEnumValue>	TEnumRemap;
	std::vector<TEnumRemap>		EnumRemap;


	/**
	 * Build adapter for 2 tables
	 */
	bool			buildTableAdapter(const CTable* from, const CTable* into, CTableAdapter& adapter);

	/**
	 * Build column content
	 */
	bool			buildColumnContent(const CColumnAdapter& adapter, const uint8* from, uint8* into);

	/**
	 * Build Enum Remapping
	 */
	bool			buildEnumRemapping(const CType* fromType, const CType* intoType);


	/// \name Type Adapt Methods
	// @{

	bool			adaptBool(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptChar(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptUcchar(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptUint8(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptUint16(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptUint32(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptUint64(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptSint8(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptSint16(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptSint32(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptSint64(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptFloat(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptDouble(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptCSheetId(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptCEntityId(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptEnum(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptDimension(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptIndex(const CColumnAdapter& adapter, const uint8* from, uint8* into);
	bool			adaptList(const CColumnAdapter& adapter, const uint8* from, uint8* into);

	// @}

};


#endif // NL_DATABASE_ADAPTER_H

/* End of database_adapter.h */
