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



#ifndef NL_RYZOM_DATABASE_BANKS_H
#define NL_RYZOM_DATABASE_BANKS_H

// Greater than the number of bits necessary for bank identifiers
const uint CDB_BANK_SHIFT = 5;

/**
 * Database bank identifiers (please change CDBBankNames in cpp accordingly)
 */
enum TCDBBank { CDBPlayer, CDBGuild, /* CDBContinent, */ CDBOutpost, /* CDBGlobal, */ NB_CDB_BANKS, INVALID_CDB_BANK };


// Utility macro
#define FILL_nbits_WITH_NB_BITS_FOR_CDBBANK \
	for ( nbits=1; (1<<nbits) < NB_CDB_BANKS; ++nbits ) {}

#endif // NL_RYZOM_DATABASE_BANKS_H

/* End of ryzom_database_banks.h */

