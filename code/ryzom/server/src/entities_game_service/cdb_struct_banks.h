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

#ifndef NL_CDB_STRUCT_BANKS_H
#define NL_CDB_STRUCT_BANKS_H


// Context-specific
#include "game_share/ryzom_database_banks.h"


typedef sint16 TCDBDataIndex;
const TCDBDataIndex CDB_INVALID_DATA_INDEX = (TCDBDataIndex)~0;
const TCDBDataIndex CDB_MAX_DATA_INDEX = 0x7FFF;
const TCDBDataIndex CDB_LAST_CHANGED = CDB_INVALID_DATA_INDEX-1;

class ICDBStructNode;
class CCDBStructNodeBranch;

/**
 * Main data structure banks for the client database system (CDB)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CCDBStructBanks
{
public:

	///	Build the structure of the database from a XML file, and create the singleton instance
	static void				init( const std::string& filename ) 
	{ 
		nlassert( !_Instance ); 
		_Instance = new CCDBStructBanks(); 
		_Instance->doInit( filename ); 
	}

	/// Release (delete all structure banks)
	static void				release() { delete _Instance; }

	/// Get the singleton instance
	static CCDBStructBanks	*instance() { return _Instance; }

	/// Return the root of the structure tree corresponding to the specified bank
	CCDBStructNodeBranch	*getStructRoot( TCDBBank bank ) { return _StructTreeRootBanks[bank]; }

	/// Return a pointer to a node corresponding to a bank and a property name
	ICDBStructNode *		getICDBStructNodeFromName( TCDBBank bank, const std::string& name ) const;

	/// Return the node corresponding to an index
	ICDBStructNode			*getNodeFromDataIndex( TCDBBank bank, TCDBDataIndex index )
	{
#ifdef NL_DEBUG
		nlassert( (bank < NB_CDB_BANKS) && (index < _IndexNb[bank]) );
#endif
		return _IndexToNode[bank][index];
	}

	/// Return the number of indices (properties) of the bank
	TCDBDataIndex			nbIndices( TCDBBank bank ) const { return _IndexNb[bank]; }

	/// Destructor
	~CCDBStructBanks();

	/// Convert a bank name to a bank identifier
	static TCDBBank readBankName( const std::string& bankName );

	/// Return the string for the bank identifier
	static const char *getBankName( TCDBBank bank );

protected:

	/// Helper for init()
	void					doInit( const std::string& filename );

	/// Init a bank
	void					initBank( TCDBBank bank );

	/// Release a bank
	void					releaseBank( TCDBBank bank );

	friend void cbSetNodeForIndex( ICDBStructNode *node, void *bank );

private:

	static CCDBStructBanks	*_Instance;

	/// Array of structure trees, indexed by TCDBBank
	CCDBStructNodeBranch	*_StructTreeRootBanks [NB_CDB_BANKS];

	/// Array of arrays to find the node from the index (2-dimension array storing pointers)
	ICDBStructNode			**_IndexToNode [NB_CDB_BANKS];

	/// Numbers of indices per bank
	TCDBDataIndex			_IndexNb [NB_CDB_BANKS];

};


#endif // NL_CDB_STRUCT_BANKS_H

/* End of cdb_struct_banks.h */
