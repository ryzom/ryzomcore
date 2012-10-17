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

#ifndef RY_PDS_DATABASE_H
#error pds_database_inline.h MUST be included by pds_database.h
#endif

#ifndef RY_PDS_DATABASE_INLINE_H
#define RY_PDS_DATABASE_INLINE_H


//
// Inlines
//





//
// Get methods
//

/*
 * Get Type
 */
inline const CType*	CDatabase::getType(TTypeId typeId) const
{
	if (typeId >= _Types.size())
	{
		//warning("getType(): type '"+NLMISC::toString(typeId)+"' is not initialised");
		return NULL;
	}

	return _Types[typeId];
}

/*
 * Get Table
 */
inline const CTable*	CDatabase::getTable(TTypeId tableId) const
{
	if (tableId >= _Tables.size())
	{
		//warning("getTable(): table '"+NLMISC::toString(tableId)+"' is not initialised");
		return NULL;
	}

	return _Tables[tableId];
}



/*
 * Private Get Table (non const)
 */
inline CTable*	CDatabase::getNonConstTable(RY_PDS::TTableIndex tableId)
{
	if (tableId >= _Tables.size())
		return NULL;

	return _Tables[tableId];
}

#endif //RY_PDS_DATABASE_INLINE_H

