// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef CDB_BANK_HANDLER
#define CDB_BANK_HANDLER

#include <vector>
#include "nel/misc/types_nl.h"

namespace NLMISC{

/**
 @brief Manages the bank names and mappings of the CDB it's associated with

 Banks are numeric identifiers for the top-level branches of the CDB.
 They are used for saving bandwidth, because the local CDBs are updated with deltas,
 that identify the updatable top-level branch with this id.
 The CCDBBankHandler manages the mapping of banks to their names, unified (node) index,
 and the other way around.

 */
class CCDBBankHandler{
public:
	/**
	 @brief The class' constructor
	 @param maxbanks the maximum number of banks we need to handle
	*/
	CCDBBankHandler( uint maxbanks );

	/// Very surprisingly this is the destructor
	~CCDBBankHandler(){}

	/**
	 @brief   Returns the unified (node) index for the specified bank Id.
	 @param   bank The bank whose uid we need.
	 @return  Returns an uid or static_cast< uint >( -1 ) on failure.
	*/
	uint getUIDForBank( uint bank ) const;

	/**
	 @brief   Returns the bank Id for the specified unified (node) index.
	 @param   uid The unified (node) index we need to translate to bank Id.
	 @return  Returns a bank Id.
	*/
	uint getBankForUID( uint uid ) const{ return _UnifiedIndexToBank[ uid ]; }

	/// Returns the last unified (node) index we mapped.
	uint getLastUnifiedIndex() const{ return _CDBLastUnifiedIndex; }

	/**
	 @brief   Returns the number of bits used to store the number of nodes that belong to this bank.
	 @param   bank The banks whose id bits we need.
	 @return  Returns the number of bits used to store the number of nodes that belong to this bank.
	*/
	uint getFirstLevelIdBits( uint bank ) const{ return _FirstLevelIdBitsByBank[ bank ]; }

	/**
	 @brief   Returns the name of the specified bank.
	 @param   bank The id of the bank we need the name of.
	 @return  Returns the name of the specified bank.
	*/
	std::string getBankName( uint bank ) const{ return _CDBBankNames[ bank ]; }

	/**
	 @brief   Looks up the bank Id of the bank name specified.
	 @param   name The name of the bank whose Id we need.
	 @return  Returns the id of the bank, or static_cast< uint >( -1 ) on fail.
	*/
	uint getBankByName( const std::string &name ) const;

	/**
	 @brief   Maps the specified bank name to a unified (node) index and vica versa.
	 @param   bankName Name of the bank to map.
	*/
	void mapNodeByBank( const std::string &bankName );

	/**
	 @brief   Loads the known bank names from an array ( the order decides the bank Id ).
	 @param   strings The array of the banks names.
	 @param   size    The size of the array.
	*/
	void fillBankNames( const char **strings, uint size );

	/// Resets the node to bank mapping vector
	void resetNodeBankMapping(){ _UnifiedIndexToBank.clear(); }

	/// Resets all maps, and sets _CDBLastUnifiedIndex to 0.
	void reset();

	uint getUnifiedIndexToBankSize() const{ return _UnifiedIndexToBank.size(); }

	/// Calculates the number of bits used to store the number of nodes that belong to the banks.
	void calcIdBitsByBank();

	/**
	 @brief   Looks up the unified (node) index of a bank node.
	 @param   bank  The bank id of the node we are looking up.
	 @param   index The index of the node within the bank.
	 @return  Returns the unified (node) index of the specified bank node.
	*/
	uint getServerToClientUIDMapping( uint bank, uint index ) const{ return _CDBBankToUnifiedIndexMapping[ bank ][ index ]; }

	/**
	 @brief Resizes the bank holders. WARNING: Resets data contained.
	 @param newSize - The new maximum number of banks.
	 */
	void resize( uint newSize );

private:
	/// Mapping from server database index to client database index (first-level nodes)
	std::vector< std::vector< uint > > _CDBBankToUnifiedIndexMapping;
	
	/// Mapping from client database index to bank IDs (first-level nodes)
	std::vector< uint > _UnifiedIndexToBank;
	
	/// Last index mapped
	uint _CDBLastUnifiedIndex;
	
	/// Number of bits for first-level branches, by bank
	std::vector< uint > _FirstLevelIdBitsByBank;

	/// Names of the CDB banks
	std::vector< std::string > _CDBBankNames;

	/// The number of banks used
	uint maxBanks;
};

}

#endif

