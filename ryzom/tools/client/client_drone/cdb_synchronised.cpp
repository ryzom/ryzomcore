/** \file cdb_synchronised.cpp
 * <File description>
 *
 * $Id$
 */



//#include "stdpch.h"

//#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
//#define TRACE_SET_VALUE


//////////////
// Includes //
//////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/bit_mem_stream.h"

#include "cdb_synchronised.h"
#include "../client/dummy_progress.h"

#include <iostream.h>
#include <libxml/parser.h>
#include <fcntl.h>
#include <string.h>

#include <string>


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;


bool VerboseDatabase = false;
uint32 NbDatabaseChanges = 0;

extern CDummyProgress progressCallback;


//-----------------------------------------------
//	CCDBSynchronised
//
//-----------------------------------------------
CCDBSynchronised::CCDBSynchronised() : _Database(0), _InitInProgress(true), _InitDeltaReceived(0)
{
}



//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CCDBSynchronised::init( const string &fileName, NLMISC::IProgressCallback &progressCallBack )
{
	try
	{
		CIFile file;
		if (file.open (fileName))
		{
			// Init an xml stream
			CIXml read;
			read.init (file);

			//Parse the parser output!!!
			CCDBNodeBranch::resetNodeBankMapping(); // in case the game is restarted from start
			_Database = new CCDBNodeBranch("SERVER");
			_Database->init( read.getRootNode (), progressCallBack, true );
		}
	}
	catch (Exception &e)
	{
		// Output error
		nlwarning ("CFormLoader: Error while loading the form %s: %s", fileName.c_str(), e.what());
	}
}


//-----------------------------------------------
//	readDelta
//
//-----------------------------------------------
void CCDBSynchronised::readDelta( NLMISC::TGameCycle gc, CBitMemStream& s, TCDBBank bank )
{
	nldebug("Update DB");

	if( _Database == 0 )
	{
		nlwarning("<CCDBSynchronised::readDelta> the database has not been initialized");
		return;
	}

	//displayBitStream2( f, f.getPosInBit(), f.getPosInBit() + 64 );
	uint16 propertyCount = 0;
	s.serial( propertyCount );

	if ( VerboseDatabase )
		nlinfo( "CDB: Reading delta (%hu changes)", propertyCount );
	NbDatabaseChanges += propertyCount;

	for( uint i=0; i!=propertyCount; ++i )
	{
		_Database->readAndMapDelta( gc, s, bank );
	}

	/*// Read "client only" property changes
	bool hasCOPropChanges;
	s.serialBit( hasCOPropChanges );
	if ( hasCOPropChanges )
	{
		uint32 nbCOPropChanges;
		s.serial( nbCOPropChanges, 4 );
		if ( nbCOPropChanges == 15 )
		{
			s.serial( nbCOPropChanges );
		}
		for ( uint i=0; i!=nbCOPropChanges; ++i )
		{
			string propName; //TEMP
			sint64 value; //TEMP
			s.serial( propName );
			s.serial( value );
			setProp( propName, value );
		}
	}*/

} // readDelta //


//-----------------------------------------------
//	getProp
//	
//-----------------------------------------------
sint64 CCDBSynchronised::getProp( const string &name )
{
	if( _Database != 0 )
	{
		ICDBNode::CTextId txtId( name );
		return _Database->getProp( txtId );
	}
	else
		throw CCDBSynchronised::EDBNotInit();
} // getProp //


//-----------------------------------------------
//	setProp
// Set the value of a property (the update flag is set to true)
// \param name is the name of the property
// \param value is the value of the property
// \return bool : 'true' if the property was found.
//-----------------------------------------------
bool CCDBSynchronised::setProp(const string &name, sint64 value)
{
	if(_Database == 0)
	{
		nlwarning("<CCDBSynchronised::setProp> the database has not been initialized");
		return false;
	}

#ifdef TRACE_SET_VALUE
	nlinfo("Set value %"NL_I64"d for Prop %s", value,name.c_str() );
#endif

	// Set the property.
	ICDBNode::CTextId txtId( name );
	return _Database->setProp( txtId, value );
} // setProp //



//-----------------------------------------------
//	getString
//
//-----------------------------------------------
string CCDBSynchronised::getString( uint32 id )
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
} // getString //



//-----------------------------------------------
//	setString
//
//-----------------------------------------------
void CCDBSynchronised::setString( uint32 id, const std::string &str )
{
	_Strings[id]=str;
}


//-----------------------------------------------
//	clear
//
//-----------------------------------------------
void CCDBSynchronised::clear()
{
	if(_Database)
	{
		_Database->clear();
		delete _Database;
		_Database = 0;
	}
}



#ifdef TRACE_READ_DELTA
#undef TRACE_READ_DELTA
#endif

#ifdef TRACE_WRITE_DELTA
#undef TRACE_WRITE_DELTA
#endif

#ifdef TRACE_SET_VALUE
#undef TRACE_SET_VALUE
#endif


