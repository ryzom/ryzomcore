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


//#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
#define TRACE_SET_VALUE
// using verbose command

//////////////
// Includes //
//////////////

#include <fstream>
#include <iostream>
#include <libxml/parser.h>
#include <fcntl.h>
#include <string.h>

#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/command.h"
#include "nel/misc/bit_mem_stream.h"


//#include "nel/net/unified_network.h"

#include "player_manager.h"
#include "player.h"
#include "character.h"

#include "cdb_synchronised.h"
#include "cdb.h"
#include "cdb_branch.h"
#include "cdb_leaf.h"
#include "db_string_updater.h"


bool VerboseDatabase = false;


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;



//-----------------------------------------------
//	CCDBSynchronised
//
//-----------------------------------------------
CCDBSynchronised::CCDBSynchronised() : _DataStructRoot(NULL), _Bank(INVALID_CDB_BANK), NbDatabaseChanges(0), _NotSentYet(true)
{
}

CCDBSynchronised::~CCDBSynchronised()
{
	// warn the string updater that this is no more
	CDBStringUpdater::getInstance().onClientDatabaseDeleted(this);
}

/*
 * Helper callback for init()
 */
void cbAtomBranchResetSubTracker( void *dataContainer, TCDBDataIndex index )
{
	// if ( VerboseDatabase )
	//nldebug( "CDB: Resetting sub tracker at index %d", index );
	((CCDBDataInstanceContainer*)dataContainer)->resetSubTracker( index );
}


//-----------------------------------------------
//	init (the singleton of CCDBStructBanks must have been initialized before)
//
//-----------------------------------------------
void CCDBSynchronised::init( TCDBBank bank, bool usePermanentTracker )
{
	H_AUTO(initCCDBSync);

	_Bank = bank;

	// Allocate a new data instance for the player bank
	_DataContainer.init( CCDBStructBanks::instance()->nbIndices( bank ), usePermanentTracker );
	_DataStructRoot = CCDBStructBanks::instance()->getStructRoot( bank );
	nlassert( _DataStructRoot );

	_DataStructRoot->foreachAtomBranchCall( cbAtomBranchResetSubTracker, &_DataContainer );
}


//-----------------------------------------------
//	read
//
//-----------------------------------------------
void CCDBSynchronised::read( const string& fileName )
{
#ifdef NL_DEBUG
	int linecount=1;
#endif

	if( ! _DataStructRoot )
	{
		throw CCDBSynchronised::EDBNotInit();
	}

	ifstream f(fileName.c_str(), ios::in);
	if( !f.is_open() )
	{
		nlerror("can't open file : %s\n", fileName.c_str());
	}
	
	while( !f.eof() )
	{
		string line;
		getline(f,line,'\n');
#ifdef NL_DEBUG
	nlinfo("%s:%i",fileName.c_str(),linecount);
	linecount++;
#endif

		char * token;
		char * buffer = new char[line.size()+1];
		strcpy(buffer,line.c_str()); 

		// value
		token = strtok(buffer," \t");
		if( token == NULL ) continue;
		sint64 value;
		NLMISC::fromString(token, value);
		
		// property name
		token = strtok(NULL," \n");
		if( token == NULL ) continue;
		string propName(token);
		
		// set the value of the property
		ICDBStructNode::CTextId txtId(propName);
		TCDBDataIndex index = _DataStructRoot->findDataIndex( txtId );
		if ( _DataContainer.checkIndex( index ) )
			_DataContainer.setValue64( index, value, false );
		else
			nlwarning( "CDB: Reading invalid index %d", index );

		delete [] buffer;
	}

	f.close();

} // read //


struct TWriteCallbackArg
{
	FILE						*F;
	CCDBDataInstanceContainer	*Container;
};
/*
void cbWrite( void *arg, TCDBDataIndex index, ICDBStructNode::CTextId *textId )
{
	TWriteCallbackArg *wca = (TWriteCallbackArg*)arg;
	fprintf( wca->F,"%d\t%s\n", wca->Container->getValue64( index ), textId->toString().c_str() );
}
*/

//-----------------------------------------------
//	write
//
//-----------------------------------------------
/*
void CCDBSynchronised::write( const string& fileName )
{
	if ( _DataStructRoot )
	{
		TWriteCallbackArg wca;
		wca.F = fopen( fileName.c_str(),"w" );
		wca.Container = &_DataContainer;
		ICDBStructNode::CTextId id;
		_DataStructRoot->foreachLeafCall( cbWrite, id, &wca );
		fclose( wca.F );
	}
	else
	{
		nlwarning("<CCDBSynchronised::write> can't write %s : the database has not been initialized",fileName.c_str());
	}

} // write //
*/

void cbNop( void *, CCDBStructNodeLeaf *, uint& )
{}


struct TPushAtomChangeStruct
{
	CCDBSynchronised	*CdbSync;
	uint32				*BitSize;
	CBitMemStream		*S;
	CBitSet				AtomBitfield;
};


void cbPushDeltaOfLeafInAtomIfChanged( void* arg, CCDBStructNodeLeaf *node, uint& indexInAtom )
{
	((TPushAtomChangeStruct*)arg)->CdbSync->pushDeltaOfLeafInAtomIfChanged( (TPushAtomChangeStruct*)arg, node, indexInAtom );
}

void cbPushDeltaOfLeafInAtomIfChangedPermanent( void* arg, CCDBStructNodeLeaf *node, uint& indexInAtom )
{
	((TPushAtomChangeStruct*)arg)->CdbSync->pushDeltaOfLeafInAtomIfChangedPermanent( (TPushAtomChangeStruct*)arg, node, indexInAtom );
}


/*
 * Push one change in an atom if the leaf needs it
 */
void CCDBSynchronised::pushDeltaOfLeafInAtomIfChanged( TPushAtomChangeStruct *arg, CCDBStructNodeLeaf *node, uint indexInAtom )
{
	if ( _DataContainer.isChanged( node->getDataIndex() ) )
	{
		if ( VerboseDatabase )
			nldebug( "CDB/ATOM: Pushing changed property[%u]", indexInAtom );
		pushDelta( *arg->S, node, *arg->BitSize );
		arg->AtomBitfield.set( indexInAtom, true );
	}
}

/*
 * Push one change in an atom if the leaf needs it
 */
void CCDBSynchronised::pushDeltaOfLeafInAtomIfChangedPermanent( TPushAtomChangeStruct *arg, CCDBStructNodeLeaf *node, uint indexInAtom )
{
	if ( _DataContainer.isPermanentChanged( node->getDataIndex() ) )
	{
		if ( VerboseDatabase )
			nldebug( "CDB/ATOM: Pushing permanent property[%u]", indexInAtom );
		pushDeltaPermanent( *arg->S, node, *arg->BitSize );
		arg->AtomBitfield.set( indexInAtom, true );
	}
}


/*
 * Write the id to a bit stream for server-client comms, and return the size of data written
 */
inline sint ICDBStructNode::CBinId::writeToBitMemStream( NLMISC::CBitMemStream& f ) const
{
	sint bitsize = 0;
	for ( uint i=0; i<Ids.size(); i++ )
	{
		uint32& nodeIndex = const_cast<uint32&>(Ids[i].first);
		f.serialAndLog2( nodeIndex, Ids[i].second );
		bitsize += Ids[i].second;
	}
	return bitsize;
}


/*
 * Fill the bitstream with the property changes that were pushed using setPropIntoClientonlyDB().
 * Empty the list of pending property changes.
 */
/*void CCDBSynchronised::writeClientonlyPropertyChanges( NLMISC::CBitMemStream& s )
{
	// Send property changes for "client only" database
	uint32 nbCOPropChanges = _PropsForClientOnly.size();
	if ( nbCOPropChanges < 15 )
	{
		s.serial( nbCOPropChanges, 4 ); // must be identical in the client readDelta()
	}
	else
	{
		uint32 nbCOPropChangesOver4bit = 15;
		s.serial( nbCOPropChangesOver4bit, 4 );
		s.serial( nbCOPropChanges );
	}
	for ( std::vector<CPropForClientOnly>::iterator ipco=_PropsForClientOnly.begin(); ipco!=_PropsForClientOnly.end(); ++ipco )
	{
		const CPropForClientOnly& prop = (*ipco);
		s.serial( (string&)prop.PropName ); // does not compile if directly prop.PropName
		//ICDBStructNode *node = _DataStructRoot->getICDBStructNodeFromNameFromRoot( prop.PropName );
		//if ( node )
		{
			//uint32 bitsize += static_cast<CCDBStructNodeLeaf*>(node)->binLeafId().writeToBitMemStream( s );

			// Push the value (TEMP: as 64 bits)
			s.serial( (sint64&)prop.Value ); //, bitsize );
		}
	}
	_PropsForClientOnly.clear();
}*/


//-----------------------------------------------
//	writeDelta
//
//-----------------------------------------------
bool CCDBSynchronised::writeDelta( CBitMemStream& s, uint32 maxBitSize )
{
	//if ( ! _DataStructRoot ) // nlwarning("<CCDBSynchronised::writeDelta> the database has not been initialized");
	//	return;

	// Test the changed property count and make room to store the number of property pushed (poked later)
	uint origChangedPropertyCount = getChangedPropertyCount();
	if ( origChangedPropertyCount == 0 )
		return false;
	uint bitposOfNbChanges = s.getPosInBit();
	uint32 dummy = 0;
	s.serial( dummy, CDBChangedPropertyCountBitSize ); // optimising s.reserveBits( CDBChangedPropertyCountBitSize )

	// Browse changes and write them
	uint32 bitsize = CDBChangedPropertyCountBitSize; // initialize with the size of the reserved bits for the number of changes
	TCDBDataIndex dataIndex;
	while ( ((dataIndex = _DataContainer.getFirstChanged()) != CDB_LAST_CHANGED)
			&& (bitsize < maxBitSize) )
	{
		// Retrieve the structure node corresponding to the index
		ICDBStructNode *node = CCDBStructBanks::instance()->getNodeFromDataIndex( _Bank, dataIndex );
		nlassert( node );

		if ( node->isAtomic() ) // counts for 1 change
		{
#ifdef NL_DEBUG
			nlassert( dynamic_cast<CCDBStructNodeBranch*>(node) ); // should not be leaf because the main tracker pushes the atom group index for atomic leaves
#endif
			// Build and push the binary atom id
			ICDBStructNode::CBinId binId;
			(static_cast<CCDBStructNodeBranch*>(node))->buildBinIdFromLeaf( binId );
			bitsize += binId.writeToBitMemStream( s );
			//nlinfo( "CDB/ATOM: Written bin id %s", binId.toString().c_str() );
			//_DataContainer.displayAtomChanges( node->getDataIndex() );

			// Make room to store the atom bitfield
			uint bitposOfAtomBitfield = s.getPosInBit();
			uint indexInAtom = 0;
			node->foreachLeafCall( cbNop, indexInAtom, NULL ); // count the number of siblings
			uint nbAtomElements = indexInAtom;
			s.reserveBits( nbAtomElements );
			bitsize += nbAtomElements;
			//nlinfo( "CDB/ATOM: Reserved %u bits (%d)", nbAtomElements, s.getPosInBit()-bitposOfAtomBitfield );

			// Browse the siblings of the atom node, and push the deltas for the properties marked as changes, updating the bitfield
			TPushAtomChangeStruct arg;
			arg.CdbSync = this;
			arg.BitSize = &bitsize;
			arg.S = &s;
			arg.AtomBitfield.resize( nbAtomElements );
			indexInAtom = 0;
			node->foreachLeafCall( cbPushDeltaOfLeafInAtomIfChanged, indexInAtom, (void*)&arg );
			
			// Fill the placeholder with the bitfield
			s.pokeBits( arg.AtomBitfield, bitposOfAtomBitfield );
			if ( VerboseDatabase )
			{
				nldebug( "CDB/ATOM: Bitfield: %s", arg.AtomBitfield.toString().c_str() );
			}

			// Pop the changes out of the tracker
			TCDBDataIndex atomGroupIndex = node->getDataIndex();
			TCDBDataIndex leafDataIndex;
			while ( (leafDataIndex = _DataContainer.popChangedIndexInAtom( atomGroupIndex )) != CDB_LAST_CHANGED );
		}
		else
		{
#ifdef NL_DEBUG
			nlassert( dynamic_cast<CCDBStructNodeLeaf*>(node) );
#endif

			// Push the binary property id
			bitsize += static_cast<CCDBStructNodeLeaf*>(node)->binLeafId().writeToBitMemStream( s );

			// Push the value
			pushDelta( s, static_cast<CCDBStructNodeLeaf*>(node), bitsize );
		}

		_DataContainer.popFirstChanged();
	}

	// Fill the placeholder with the number of changes
	uint32 nbChanges = origChangedPropertyCount - getChangedPropertyCount();
	s.poke( nbChanges, bitposOfNbChanges, CDBChangedPropertyCountBitSize );
	//s.displayStream( "writeDelta" );
	NbDatabaseChanges += nbChanges;

	// Check if all has been popped
	_DataContainer.quickCleanChanges();

	_NotSentYet = false;

#ifdef TRACE_SET_VALUE
	if ( VerboseDatabase )
		nldebug( "%u: CDB: Delta pushed (%u changes written, %u remaining)", CTickEventHandler::getGameCycle(), nbChanges, getChangedPropertyCount() );
#endif

	return true;
} // writeDelta //


//-----------------------------------------------
//	writePermanentDelta
//
// TODO: Maybe an optimization will be required to prevent sending null properties
// when a character is added to a CCDBGroup.
//-----------------------------------------------
bool	CCDBSynchronised::writePermanentDelta( NLMISC::CBitMemStream& s )
{
	static uint nbPermaDeltaSent = 0;

	// Test the changed property count and make room to store the number of property pushed (poked later)
	uint origChangedPropertyCount = _DataContainer.getPermanentChangedPropertyCount();
	if ( origChangedPropertyCount == 0 )
		return false;
	uint bitposOfNbChanges = s.getPosInBit();
	uint32 dummy = 0;
	s.serial( dummy, CDBChangedPropertyCountBitSize ); // optimising s.reserveBits( CDBChangedPropertyCountBitSize )

	// Browse changes and write them
	uint32 nbChanges = 0;

	uint32 bitsize = CDBChangedPropertyCountBitSize; // initialize with the size of the reserved bits for the number of changes
	TCDBDataIndex dataIndex = _DataContainer.getPermanentFirstChanged();
	while ( dataIndex != CDB_LAST_CHANGED
			/*&& (bitsize < maxBitSize)*/ )
	{
		++nbPermaDeltaSent;

		// Retrieve the structure node corresponding to the index
		ICDBStructNode *node = CCDBStructBanks::instance()->getNodeFromDataIndex( _Bank, dataIndex );
		nlassert( node );

		if ( node->isAtomic() ) // counts for 1 change
		{
#ifdef NL_DEBUG
			nlassert( dynamic_cast<CCDBStructNodeBranch*>(node) ); // should not be leaf because the main tracker pushes the atom group index for atomic leaves
#endif
			// Build and push the binary atom id
			ICDBStructNode::CBinId binId;
			(static_cast<CCDBStructNodeBranch*>(node))->buildBinIdFromLeaf( binId );
			bitsize += binId.writeToBitMemStream( s );
			//nlinfo( "CDB/ATOM: Written bin id %s", binId.toString().c_str() );
			//_DataContainer.displayAtomChanges( node->getDataIndex() );

			// Make room to store the atom bitfield
			uint bitposOfAtomBitfield = s.getPosInBit();
			uint indexInAtom = 0;
			node->foreachLeafCall( cbNop, indexInAtom, NULL ); // count the number of siblings
			uint nbAtomElements = indexInAtom;
			s.reserveBits( nbAtomElements );
			bitsize += nbAtomElements;
			//nlinfo( "CDB/ATOM: Reserved %u bits (%d)", nbAtomElements, s.getPosInBit()-bitposOfAtomBitfield );

			// Browse the siblings of the atom node, and push the deltas for the properties marked as changes, updating the bitfield
			TPushAtomChangeStruct arg;
			arg.CdbSync = this;
			arg.BitSize = &bitsize;
			arg.S = &s;
			arg.AtomBitfield.resize( nbAtomElements );
			indexInAtom = 0;
			node->foreachLeafCall( cbPushDeltaOfLeafInAtomIfChangedPermanent, indexInAtom, (void*)&arg );
			
			// Fill the placeholder with the bitfield
			s.pokeBits( arg.AtomBitfield, bitposOfAtomBitfield );
			if ( VerboseDatabase )
			{
				nldebug( "CDB/ATOM: Bitfield: %s", arg.AtomBitfield.toString().c_str() );
			}

		}
		else
		{
#ifdef NL_DEBUG
			nlassert( dynamic_cast<CCDBStructNodeLeaf*>(node) );
#endif

			// Push the binary property id
			bitsize += static_cast<CCDBStructNodeLeaf*>(node)->binLeafId().writeToBitMemStream( s );

			// Push the value
			pushDeltaPermanent( s, static_cast<CCDBStructNodeLeaf*>(node), bitsize );
		}

		dataIndex = _DataContainer.getPermanentNextChanged( dataIndex );
		++nbChanges;
	}

	// Fill the placeholder with the number of changes
	s.poke( nbChanges, bitposOfNbChanges, CDBChangedPropertyCountBitSize );
	//s.displayStream( "writeDelta" );

#ifdef TRACE_SET_VALUE
	if ( VerboseDatabase )
		nldebug( "%u: CDB: Permanent Delta pushed (%u changes written, %u remaining)", CTickEventHandler::getGameCycle(), nbChanges, getChangedPropertyCount() );
#endif

	nldebug( "Filled %u permanent changes", nbPermaDeltaSent );

	return true;
}


/*
 * Push one change to the stream
 */
void	CCDBSynchronised::pushDelta( CBitMemStream& s, CCDBStructNodeLeaf *node, uint32& bitsize )
{
	TCDBDataIndex index = node->getDataIndex();

	// Test if the property type is valid.
	if ( node->type() > ICDBStructNode::UNKNOWN && node->type() < ICDBStructNode::Nb_Prop_Type )
	{
		// Serialize the Property Value according to the Property Type.
		uint64 value;
		value = (uint64)_DataContainer.getValue64( index );
		//_DataContainer.archiveCurrentValue( index );

		if ( node->type() == ICDBStructNode::TEXT )
		{
			s.serialAndLog2( value, 32 );
			bitsize += 32;
			if ( VerboseDatabase )
				nldebug( "CDB: Pushing value %"NL_I64"d (TEXT-32) for index %d prop %s", (sint64)value, index, node->buildTextId().toString().c_str() );
		}
		else
		{
			s.serialAndLog2( value, (uint)node->type() );
			bitsize += (uint32)node->type();
			if ( VerboseDatabase )
				nldebug( "CDB: Pushing value %"NL_I64"d (%u bits) for index %d prop %s", (sint64)value, (uint32)node->type(), index, node->buildTextId().toString().c_str() );
		}
	}
	else
		nlwarning("CCDBStructNodeLeaf::writeDelta : Property Type Unknown ('%d') -> not serialized.", (uint)node->type());
}


/*
 * Push one change to the stream (permanent mode)
 */
void	CCDBSynchronised::pushDeltaPermanent( NLMISC::CBitMemStream& s, CCDBStructNodeLeaf *node, uint32& bitsize )
{
	TCDBDataIndex index = node->getDataIndex();

	// Test if the property type is valid.
	if ( node->type() > ICDBStructNode::UNKNOWN && node->type() < ICDBStructNode::Nb_Prop_Type )
	{
		// Serialize the Property Value according to the Property Type.
		uint64 value;
		value = (uint64)_DataContainer.getValue64( index ); // "delta from 0"

		if ( node->type() == ICDBStructNode::TEXT )
		{
			s.serialAndLog2( value, 32 );
			bitsize += 32;
			if ( VerboseDatabase )
				nldebug( "CDB: Pushing permanent value %"NL_I64"d (TEXT-32) for index %d prop %s", (sint64)value, index, node->buildTextId().toString().c_str() );
		}
		else
		{
			s.serialAndLog2( value, (uint)node->type() );
			bitsize += (uint32)node->type();
			if ( VerboseDatabase )
				nldebug( "CDB: Pushing permanent value %"NL_I64"d (%u bits) for index %d prop %s", (sint64)value, (uint32)node->type(), index, node->buildTextId().toString().c_str() );
		}
	}
	else
		nlwarning("CCDBStructNodeLeaf::writeDeltaPermanent : Property Type Unknown ('%d') -> not serialized.", (uint)node->type());
}


//-----------------------------------------------
//	getProp by name
//	
//-----------------------------------------------
sint64 CCDBSynchronised::x_getProp( const std::string& name ) const
{
	H_AUTO(CCDBSynchronisedGetProp)
	//if ( _DataStructRoot )
	//{
		ICDBStructNode::CTextId txtId( name );
		TCDBDataIndex dataIndex = _DataStructRoot->findDataIndex( txtId );
		if ( _DataContainer.checkIndex( dataIndex ) )
		{
			return _DataContainer.getValue64( dataIndex );
		}
		else
		{
			throw CCDBSynchronised::ECDBNotFound();
		}
	//}
	//else
	//	throw CCDBSynchronised::EDBNotInit();
} // getProp //


//-----------------------------------------------
//	getProp by node
//	Precondition: node not null.
//-----------------------------------------------
sint64 CCDBSynchronised::x_getProp( ICDBStructNode *node ) const
{
	BOMB_IF( !node, "Null node", return 0 );
	TCDBDataIndex dataIndex = node->getDataIndex();
	if ( _DataContainer.checkIndex( dataIndex ) )
	{
		return _DataContainer.getValue64( dataIndex );
	}
	else
	{
		throw CCDBSynchronised::ECDBNotFound();
	}
}
//-----------------------------------------------
//	getProp by name
//	
//-----------------------------------------------
//const ucstring &CCDBSynchronised::getPropString( const std::string& name ) const
//{
//	H_AUTO(CCDBSynchronisedGetProp)
//#error : very bad and not terminated code, don't uncomment or reimplement it
//	//if ( _DataStructRoot )
//	//{
//		ICDBStructNode::CTextId txtId( name );
//		TCDBDataIndex dataIndex = _DataStructRoot->findDataIndex( txtId );
//		if ( _DataContainer.checkIndex( dataIndex ) )
//		{
//			return CStringMapper::unmap(_DataContainer.getValue64( dataIndex ));
//		}
//		else
//		{
//			throw CCDBSynchronised::ECDBNotFound();
//		}
//	//}
//	//else
//	//	throw CCDBSynchronised::EDBNotInit();
//} // getProp //


//-----------------------------------------------
//	getProp by node
//	Precondition: node not null.
//-----------------------------------------------
ucstring CCDBSynchronised::x_getPropUcstring( ICDBStructNode *node ) const
{
	return CDBStringUpdater::getInstance().getStringLeaf(const_cast<CCDBSynchronised*>(this), node);
}

const std::string &CCDBSynchronised::x_getPropString( ICDBStructNode *node ) const
{
	return CDBStringUpdater::getInstance().getStringLeaf(const_cast<CCDBSynchronised*>(this), node);

}


// :KLUDGE: Returns ICDBStructNode non-const 'coz getName and getParent are not
// const, and to change 'em to const we need to change all the overloaded
// methods to const. Unless that calls to one of these methods on a const
// object will call the base class implementation which asserts. Ask jeromev
// if it's not clear why this is absolutely necessary (unless with following
// solution).
// :TODO: Change the base class methods in ICDBStructNode that assert to pure
// virtual to force overloading of const versions.
ICDBStructNode * CCDBSynchronised::getICDBStructNodeFromName(const std::string& name) const
{
	ICDBStructNode::CTextId txtId( name );
	return _DataStructRoot->getNode( txtId, false );
}

//-----------------------------------------------
//	setProp
// Set the value of a property (the update flag is set to true)
// \param name is the name of the property
// \param value is the value of the property
// \return bool : 'true' if the property was found.
//-----------------------------------------------
bool CCDBSynchronised::x_setProp( const std::string& name, sint64 value, bool forceSending )
{
	H_AUTO(CCDBSynchronisedSetProp);
	
	ICDBStructNode *leaf = getICDBStructNodeFromName(name);
	BOMB_IF(leaf==NULL,"Failed to find node in database: "+name,return false);

	return x_setProp( leaf, value, forceSending );
}

//-----------------------------------------------
//	setProp
// Set the value of a property (the update flag is set to true)
// \param name is the name of the property
// \param value is the value of the property
// \return bool : 'true' if the property was found.
//-----------------------------------------------
bool CCDBSynchronised::x_setPropString( const std::string& name, const ucstring &value, bool forceSending )
{
	H_AUTO(CCDBSynchronisedSetProp);
	
	ICDBStructNode *leaf = getICDBStructNodeFromName(name);
	BOMB_IF(leaf==NULL,"Failed to find node in database: "+name,return false);

	return x_setPropString( leaf, value, forceSending );
}

/*
 * Same as setProp(ICDBStructNode*,sint64,bool) but one level below.
 * If the child is not found, returns false.
 * Use getICDBStructNodeFromName() to store the node pointer.
 */
bool CCDBSynchronised::x_setProp( ICDBStructNode *node, const char *childName, sint64 value, bool forceSending )
{
	H_AUTO(CCDBSynchronisedSetProp3);

	string name = string(childName);
	ICDBStructNode::CTextId textId( name );
	ICDBStructNode *leaf = node->getNode( textId, false );
	if ( ! leaf )
		nlwarning( "Leaf %s not found", childName );
	return (leaf && x_setProp( leaf, value, forceSending ));
}

/*
 * Same as setProp(ICDBStructNode*,sint64,bool) but one level below.
 * If the child is not found, returns false.
 * Use getICDBStructNodeFromName() to store the node pointer.
 */
bool CCDBSynchronised::x_setPropString( ICDBStructNode *node, const char *childName, const ucstring &value, bool forceSending )
{
	H_AUTO(CCDBSynchronisedSetProp3);

	string name = string(childName);
	ICDBStructNode::CTextId textId( name );
	ICDBStructNode *leaf = node->getNode( textId, false );
	if ( ! leaf )
		nlwarning( "Leaf %s not found", childName );
	return (leaf && x_setPropString ( leaf, value, forceSending ));
}


/*
 * Same as setProp(ICDBStructNode*,const char*,sint64,bool) but increment the current value
 * Precondition: the child MUST be a leaf
 */
bool CCDBSynchronised::x_incProp( ICDBStructNode *node, const char *childName )
{
	H_AUTO(CCDBSynchronisedIncProp);

	string name = string(childName);
	ICDBStructNode::CTextId textId( name );
	CCDBStructNodeLeaf *leaf = static_cast<CCDBStructNodeLeaf*>(node->getNode( textId, false ));
	if ( ! leaf )
		nlwarning( "Leaf %s not found", childName );
	return (leaf && x_setProp( leaf, _DataContainer.getValue64( leaf->getDataIndex() ) + 1 ));

}


// :KLUDGE: ICDBStructNode non-const 'coz getName and getParent are not const
// methods. See CCDBSynchronised::getICDBStructNodeFromName for more info.
bool CCDBSynchronised::x_setProp( ICDBStructNode * node, sint64 value, bool forceSending )
{
	H_AUTO(CCDBSynchronisedSetProp2);

	TCDBDataIndex dataIndex = node->getDataIndex();

	// Set the property in the data instance
	if ( _DataContainer.checkIndex( dataIndex ) )
	{
		if ( node->isAtomic() )
		{
			// The leaf is a member of an atomic branch, set the value, record a change in the tracker
			// for the index of the parent node and record a change in the subtracker for the leaf

			// Find the group node (the first branch which has the atomic flag), navigating up through the tree
			ICDBStructNode * groupNodeFinder;
			do
			{
				groupNodeFinder = node;
				node = node->getParent();
			}
			while ( node && node->isAtomic() );
			//nlinfo( "Setting LeafIndex %hd, AtomGroupIndex %hd", node->getDataIndex(), groupNodeFinder->getDataIndex() );
			if ( _DataContainer.setValue64InAtom( dataIndex, value, groupNodeFinder->getDataIndex(), forceSending ) )
			{
				// check if there is a callback to call
				if (node->getAttachedCallback() != NULL)
					node->getAttachedCallback()(this, node);
#ifdef TRACE_SET_VALUE
				if ( VerboseDatabase )
				{
					std::string const* pname = node->getName();
					std::string name = pname?*pname:"'Unknown'";
					nlinfo( "CDB: Set new value %"NL_I64"d for prop %s in atom %s", value, name.c_str(), groupNodeFinder->getParent()?groupNodeFinder->getName()->c_str():"(root)" );
				}
			}
#endif
			//_DataContainer.displayAtomChanges( groupNodeFinder->getDataIndex() );
		}
		else
		{
			// The leaf is not a member of an atomic branch, simply set the value and record a change in the tracker
			if ( _DataContainer.setValue64( dataIndex, value, forceSending ) )
			{
				// check if there is a callback to call
				if (node->getAttachedCallback() != NULL)
					node->getAttachedCallback()(this, node);
#ifdef TRACE_SET_VALUE
				if ( VerboseDatabase )
				{
					std::string const* pname = node->getName();
					std::string name = pname?*pname:"'Unknown'";
					nlinfo( "CDB: Set new value %"NL_I64"d for prop %s", value, name.c_str() );
				}
#endif
			}
		}
		return true;
	}
	return false;
} // setProp //


// :KLUDGE: ICDBStructNode non-const 'coz getName and getParent are not const
// methods. See CCDBSynchronised::getICDBStructNodeFromName for more info.
bool CCDBSynchronised::x_setPropString( ICDBStructNode * node, const ucstring &value, bool forceSending )
{
	// assert that this node is a TEXT node
	nlassert(node->getType() == ICDBStructNode::TEXT);

	// transmit control to the string updater for IOS mapping stuff
	CDBStringUpdater::getInstance().setStringLeaf(this, node, value, forceSending);

	return true;
}

bool CCDBSynchronised::x_setPropString( ICDBStructNode * node, const std::string &value, bool forceSending )
{
	// assert that this node is a TEXT node
	nlassert(node->getType() == ICDBStructNode::TEXT);

	// transmit control to the string updater for IOS mapping stuff
	CDBStringUpdater::getInstance().setStringLeaf(this, node, value, forceSending);

	return true;
}


/*
 * Set the value of a property (but the update flag is NOT changed)
 * \param name is the name of the property
 * \param value is the value of the property
 * \return bool : 'true' if the property was found.
 */
bool CCDBSynchronised::x_setPropButDontSend( const std::string& name, sint64 value)
{
	//if( !_DataStructRoot ) //nlwarning("<CCDBSynchronised::setProp> the database has not been initialized");
	//	return false;

#ifdef TRACE_SET_VALUE
	if ( VerboseDatabase )
		nlinfo("Set value %"NL_I64"d for Prop %s, no change flag", value, name.c_str() );
#endif

	// Set the property.
	ICDBStructNode::CTextId txtId( name );
	TCDBDataIndex dataIndex = _DataStructRoot->findDataIndex( txtId );
	if ( _DataContainer.checkIndex( dataIndex ) )
	{
		_DataContainer.setValue64NoChange( dataIndex, value );
		return true;
	}
	else
		return false;
}


/*
 * Set the value of a property that is not in the server database tree.
 * It will be sent by impulsion message to the client at the end of the current game cycle.
 * Then it will be applied to the client database.
 * Do not use too frequently (no bandwidth regulation).
 */
/*bool CCDBSynchronised::setPropIntoClientonlyDB( const std::string& name, sint64 value )
{
	CPropForClientOnly prop( name, value );
	_PropsForClientOnly.push_back( prop );

	return true;
}*/


/*
 * Return true if the specified property has been modified and will be sent to the client
 */
bool CCDBSynchronised::isModified( const std::string& name ) const
{
	ICDBStructNode::CTextId txtId( name );
	TCDBDataIndex dataIndex = _DataStructRoot->findDataIndex( txtId );
	return _DataContainer.checkIndex( dataIndex ) && _DataContainer.isChanged( dataIndex );
}



//-----------------------------------------------
//	getString
//
//-----------------------------------------------
/*string CCDBSynchronised::getString( uint32 id )
{
	map<uint32,string>::iterator itStr = _Strings.find( id );
	if( itStr != _Strings.end() )
	{
		return (*itStr).second;
	}
	else
	{
		nlwarning("<CCDBSynchronised::getString> string with id %d was not found",id);
		return "";
	}
}*/ // getString //



//-----------------------------------------------
//	setString
//
//-----------------------------------------------
/*void CCDBSynchronised::setString( uint32 id, std::string str )
{
	_Strings[id]=str;
}*/



#ifdef TRACE_READ_DELTA
#undef TRACE_READ_DELTA
#endif

#ifdef TRACE_WRITE_DELTA
#undef TRACE_WRITE_DELTA
#endif

#ifdef TRACE_SET_VALUE

NLMISC_COMMAND( verboseDatabase, "Turn on/off database logging", "" )
{
	VerboseDatabase = !VerboseDatabase;
	log.displayNL( "verboseDatabase is now %s", VerboseDatabase?"ON":"OFF" );
	return true;
}

#undef TRACE_SET_VALUE
#endif

NLMISC_COMMAND( displayNbDatabaseChanges, "Number of database changes pushed to packets for the client", "" )
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c;
		c = PlayerManager.getChar(id);
		if (c)
		{
			log.displayNL("%u", c->_PropertyDatabase.NbDatabaseChanges );
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}	}
	return true;
}
