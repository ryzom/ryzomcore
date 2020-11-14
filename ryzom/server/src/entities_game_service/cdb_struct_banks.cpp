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



#include "stdpch.h"
#include "cdb_struct_banks.h"
#include "egs_progress_callback.h"
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include "player_manager/cdb.h"
#include "player_manager/cdb_branch.h"

using namespace NLMISC;


extern const char *CDBBankNames[INVALID_CDB_BANK+1];


/*
 * Return the bank id from a bank name, or INVALID_CDB_BANK
 */
TCDBBank CCDBStructBanks::readBankName( const std::string& bankName )
{
	for ( sint i=0; i!=INVALID_CDB_BANK; ++i )
	{
		if ( std::string(CDBBankNames[i]) == bankName )
		{
			return (TCDBBank)i;
		}
	}
	return INVALID_CDB_BANK;
}


/*
 * Return the string for the bank identifier
 */
const char *CCDBStructBanks::getBankName( TCDBBank bank )
{
	return CDBBankNames[bank];
}


CCDBStructBanks	*CCDBStructBanks::_Instance = NULL;


void cbSetNodeForIndex( ICDBStructNode *node, void *bank )
{
	//nldebug( "CDB: Mapping index %d of bank %s", node->getDataIndex(), CCDBStructBanks::getBankName((TCDBBank)(uint32)(size_t)bank) );
	nlassert( node->getDataIndex() < CCDBStructBanks::instance()->_IndexNb[(TCDBBank)(uint32)(size_t)bank] );
	nlassert( ! CCDBStructBanks::instance()->_IndexToNode [(TCDBBank)(uint32)(size_t)bank] [node->getDataIndex()] );
	CCDBStructBanks::instance()->_IndexToNode [(TCDBBank)(uint32)(size_t)bank] [node->getDataIndex()] = node;
}


/*
 *	Build the structure of the database from a XML file
 */
void					CCDBStructBanks::doInit( const std::string& filename )
{
	try
	{
		CIFile file;
		if ( file.open( filename ) )
		{
			// Init an xml stream
			CIXml read;
			read.init ( file );

			// Parse the parser output!!!
			CCDBStructNodeBranch *mainRoot = new CCDBStructNodeBranch();
			CEGSProgressCallback progressCallBack;
			mainRoot->init( read.getRootNode(), progressCallBack );

			// Label the tree branches with the matching bank name
			for ( uint b=0; b!=NB_CDB_BANKS; ++b )
				mainRoot->labelBranch( (TCDBBank)b );
			
			// Scan the tree and dispatch the branches in the banks, using the bankname attribute
			for ( uint b=0; b!=NB_CDB_BANKS; ++b )
			{
				_StructTreeRootBanks[b] = new CCDBStructNodeBranch();
				mainRoot->moveBranchesToBank( _StructTreeRootBanks[b], (TCDBBank)b );
			}
			delete mainRoot;

			// Init the banks
			for ( uint b=0; b!=NB_CDB_BANKS; ++b )
				initBank( (TCDBBank)b );
		}
		else
		{
			nlerror( "CDB: Cannot open file %s", filename.c_str() );
		}
	}
	catch (const Exception &e)
	{
		// Output error
		nlerror ("CDB: Error while loading the form %s: %s", filename.c_str(), e.what());
	}
}


/*
 * Init a bank
 */
void					CCDBStructBanks::initBank( TCDBBank bank )
{
	// Calculate the number of indices
	TCDBDataIndex index = 0;
	_StructTreeRootBanks[bank]->initDataIndex( index );
	nlinfo( "CDB: Bank '%s' has %u properties", getBankName(bank), index );

	// Setup _IndexToNode with the right size
	_IndexNb[bank] = index;
	if ( index != 0 )
	{
		_IndexToNode[bank] = new ICDBStructNode* [index];
		for ( sint i=0; i!=index; ++i )
		{
			_IndexToNode[bank][i] = NULL;
		}
	}
	else
	{
		_IndexToNode[bank] = NULL;
	}

	// Init the bank tree
	ICDBStructNode::CBinId binId;
	_StructTreeRootBanks[bank]->initIdAndCallForEachIndex( binId, cbSetNodeForIndex, (void*)bank );
}


/*
 * Return a pointer to a node corresponding to a bank and a property name
 */
ICDBStructNode *		CCDBStructBanks::getICDBStructNodeFromName( TCDBBank bank, const std::string& name ) const
{
	return _StructTreeRootBanks[bank]->getICDBStructNodeFromNameFromRoot( name );
}

	
/*
 * Release a bank
 */
void					CCDBStructBanks::releaseBank( TCDBBank bank )
{
	delete _StructTreeRootBanks[bank];
	if ( _IndexToNode[bank] )
	{
		delete [] (_IndexToNode[bank]);
	}
}


/*
 * Destructor
 */
CCDBStructBanks::~CCDBStructBanks()
{
	for ( uint b=0; b!=NB_CDB_BANKS; ++b )
		releaseBank( (TCDBBank)b );
}
