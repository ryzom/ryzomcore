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

#include "stdmisc.h"
#include "nel/misc/cdb_bank_handler.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC{
	CCDBBankHandler::CCDBBankHandler(uint maxbanks) :
_CDBBankToUnifiedIndexMapping( maxbanks, std::vector< uint >() ),
_FirstLevelIdBitsByBank( maxbanks )
	{
		std::fill( _FirstLevelIdBitsByBank.begin(), _FirstLevelIdBitsByBank.end(), 0 );
		maxBanks = maxbanks;
	}

	uint CCDBBankHandler::getUIDForBank( uint bank ) const
	{
		uint uid = static_cast< uint >( -1 );

		for( uint i = 0; i < _UnifiedIndexToBank.size(); i++ )
			if( _UnifiedIndexToBank[ i ] == bank )
				return i;

		return uid;
	}

	uint CCDBBankHandler::getBankByName( const std::string &name ) const
	{
		uint b = static_cast< uint >( -1 );

		for( uint i = 0; i < _CDBBankNames.size(); i++ )
			if( _CDBBankNames[ i ].compare( name ) == 0 )
				return i;

		return b;
	}

	void CCDBBankHandler::mapNodeByBank( const std::string &bankName )
	{
		uint b = getBankByName( bankName );
		// no such bank
		if( b == static_cast< uint >( -1 ) )
			return;
		
		_CDBBankToUnifiedIndexMapping[ b ].push_back( _CDBLastUnifiedIndex );
		++_CDBLastUnifiedIndex;
		_UnifiedIndexToBank.push_back( b );
	}

	void CCDBBankHandler::fillBankNames( const char **strings, uint size )
	{
		_CDBBankNames.clear();

		for( uint i = 0; i < size; i++ )
			_CDBBankNames.push_back( std::string( strings[ i ] ) );
	}

	void CCDBBankHandler::reset()
	{
		for( std::vector< std::vector< uint > >::iterator itr =_CDBBankToUnifiedIndexMapping.begin();
			itr != _CDBBankToUnifiedIndexMapping.end(); ++itr )
			itr->clear();

		_UnifiedIndexToBank.clear();
		_CDBLastUnifiedIndex = 0;
	}

	void CCDBBankHandler::calcIdBitsByBank()
	{
		for( uint bank = 0; bank != maxBanks; bank++ )
		{
			uint nbNodesOfBank = static_cast< uint >( _CDBBankToUnifiedIndexMapping[ bank ].size() );
			uint idb = 0; 
			
			if ( nbNodesOfBank > 0 )
				for ( idb = 1; nbNodesOfBank > unsigned( 1 << idb ) ; idb++ )
					;

			_FirstLevelIdBitsByBank[ bank ] = idb;
		}
	}

	void CCDBBankHandler::resize( uint newSize )
	{
		reset();

		_CDBBankNames.clear();
		_CDBBankToUnifiedIndexMapping.clear();
		_FirstLevelIdBitsByBank.clear();

		_CDBBankToUnifiedIndexMapping.reserve( newSize );
		_FirstLevelIdBitsByBank.reserve( newSize );
	}
}


