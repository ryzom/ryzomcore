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

#ifndef CDB_BANK_HANDLER
#define CDB_BANK_HANDLER

#include <vector>
#include "nel/misc/types_nl.h"

namespace NLMISC{

class CCDBBankHandler{
public:
	CCDBBankHandler( uint maxbanks );

	~CCDBBankHandler(){}

	uint getUIDForBank( uint bank ) const;

	uint getBankForUID( uint uid ) const{ return _UnifiedIndexToBank[ uid ]; }

	uint getLastUnifiedIndex() const{ return _CDBLastUnifiedIndex; }

	uint getFirstLevelIdBits( uint bank ) const{ return _FirstLevelIdBitsByBank[ bank ]; }

	std::string getBankName( uint bank ) const{ return _CDBBankNames[ bank ]; }

	uint getBankByName( const std::string &name ) const;

	void mapNodeByBank( const std::string &bankName );

	void fillBankNames( const char **strings, uint size );

	void resetNodeBankMapping(){ _UnifiedIndexToBank.clear(); }

	void reset();

	uint getUnifiedIndexToBankSize() const{ return _UnifiedIndexToBank.size(); }

	void calcIdBitsByBank();

	uint getServerToClientUIDMapping( uint bank, uint index ) const{ return _CDBBankToUnifiedIndexMapping[ bank ][ index ]; }

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

	uint maxBanks;
};

}

#endif

