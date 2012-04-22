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

#include "cdb_synchronised.h"

#include "interface_v3/interface_manager.h"

//#include <iostream.h>
#include <libxml/parser.h>
#include <fcntl.h>
#include <string.h>

#include <string>

#include "../../common/src/game_share/ryzom_database_banks.h"


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;


bool NLMISC::VerboseDatabase = false;
uint32 NbDatabaseChanges = 0;


//-----------------------------------------------
//	CCDBSynchronised
//
//-----------------------------------------------
CCDBSynchronised::CCDBSynchronised() : _Database(0), _InitInProgress(true), _InitDeltaReceived(0), bankHandler( NB_CDB_BANKS )
{
}

extern const char *CDBBankNames[INVALID_CDB_BANK+1];

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
			bankHandler.resetNodeBankMapping(); // in case the game is restarted from start
			bankHandler.fillBankNames( CDBBankNames, INVALID_CDB_BANK + 1 );
			_Database = new CCDBNodeBranch("SERVER");
			_Database->init( read.getRootNode (), progressCallBack, true, &bankHandler );
		}
	}
	catch (const Exception &e)
	{
		// Output error
		nlwarning ("CFormLoader: Error while loading the form %s: %s", fileName.c_str(), e.what());
	}
}


//-----------------------------------------------
//	read
//
//-----------------------------------------------
void CCDBSynchronised::read( const string &fileName )
{
#ifdef _DEBUG
	int linecount=1;
#endif

	if( _Database == 0 )
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
#ifdef _DEBUG
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
		fromString((const char*)token, value);

		// property name
		token = strtok(NULL," \n");
		if( token == NULL ) continue;
		string propName(token);

		// set the value of the property
		ICDBNode::CTextId txtId(propName);
		_Database->setProp(txtId,value);
	}

	f.close();

} // read //



//-----------------------------------------------
//	write
//
//-----------------------------------------------
void CCDBSynchronised::write( const string &fileName )
{
	if( _Database != 0 )
	{
		FILE * f;
		f = fopen(fileName.c_str(),"w");
		ICDBNode::CTextId id;
		_Database->write(id,f);
		fclose(f);
	}
	else
	{
		nlwarning("<CCDBSynchronised::write> can't write %s : the database has not been initialized",fileName.c_str());
	}

} // write //


//-----------------------------------------------
//	readDelta
//
//-----------------------------------------------
void CCDBSynchronised::readDelta( NLMISC::TGameCycle gc, CBitMemStream& s, uint bank )
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
		_Database->readAndMapDelta( gc, s, bank, &bankHandler );
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

	// clear CCDBNodeBranch static data
	branchObservingHandler.reset();
	bankHandler.reset();
}


void CCDBSynchronised::writeInitInProgressIntoUIDB()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (pIM)
		pIM->getDbProp("UI:VARIABLES:CDB_INIT_IN_PROGRESS")->setValueBool(_InitInProgress);
	else
		nlwarning("InterfaceManager not created");
}


void CCDBSynchronised::resetBank( uint gc, uint bank ){
	_Database->resetNode( gc, bankHandler.getUIDForBank( bank ) );
}

void CCDBSynchronised::addBranchObserver( const char *branchName, NLMISC::ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter )
{
	CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
	if( b == NULL )
		return;

	branchObservingHandler.addBranchObserver( b, observer, positiveLeafNameFilter );
}

void CCDBSynchronised::addBranchObserver( NLMISC::CCDBNodeBranch *branch, NLMISC::ICDBNode::IPropertyObserver *observer, const std::vector< std::string >& positiveLeafNameFilter )
{
	if( branch == NULL )
		return;

	branchObservingHandler.addBranchObserver( branch, observer, positiveLeafNameFilter );
}

void CCDBSynchronised::addBranchObserver( const char *branchName, const char *dbPathFromThisNode, NLMISC::ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize )
{
	CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
	if( b == NULL )
		return;

	branchObservingHandler.addBranchObserver( b, dbPathFromThisNode, observer, positiveLeafNameFilter, positiveLeafNameFilterSize );
}

void CCDBSynchronised::addBranchObserver( NLMISC::CCDBNodeBranch *branch, const char *dbPathFromThisNode, NLMISC::ICDBNode::IPropertyObserver &observer, const char **positiveLeafNameFilter, uint positiveLeafNameFilterSize )
{
	if( branch == NULL )
		return;
	branchObservingHandler.addBranchObserver( branch, dbPathFromThisNode, observer, positiveLeafNameFilter, positiveLeafNameFilterSize );
}

void CCDBSynchronised::removeBranchObserver( const char *branchName, NLMISC::ICDBNode::IPropertyObserver* observer )
{
	CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
	if( b == NULL )
		return;
	branchObservingHandler.removeBranchObserver( b, observer );
}

void CCDBSynchronised::removeBranchObserver( NLMISC::CCDBNodeBranch *branch, NLMISC::ICDBNode::IPropertyObserver* observer )
{
	if( branch == NULL )
		return;
	branchObservingHandler.removeBranchObserver( branch, observer );
}

void CCDBSynchronised::removeBranchObserver( const char *branchName, const char *dbPathFromThisNode, NLMISC::ICDBNode::IPropertyObserver &observer )
{
	CCDBNodeBranch *b = dynamic_cast< CCDBNodeBranch* >( _Database->getNode( ICDBNode::CTextId( std::string( branchName ) ), false ) );
	if( b == NULL )
		return;
	branchObservingHandler.removeBranchObserver( b, dbPathFromThisNode, observer );
}

void CCDBSynchronised::removeBranchObserver( NLMISC::CCDBNodeBranch *branch, const char *dbPathFromThisNode, NLMISC::ICDBNode::IPropertyObserver &observer )
{
	if( branch == NULL )
		return;
	branchObservingHandler.removeBranchObserver( branch, dbPathFromThisNode, observer );
}

void CCDBSynchronised::addFlushObserver( NLMISC::CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer )
{
	if( observer == NULL )
		return;
	branchObservingHandler.addFlushObserver( observer );
}

void CCDBSynchronised::removeFlushObserver( NLMISC::CCDBBranchObservingHandler::IBranchObserverCallFlushObserver *observer )
{
	if( observer == NULL )
		return;
	branchObservingHandler.removeFlushObserver( observer );
}

void CCDBSynchronised::flushObserverCalls()
{
	branchObservingHandler.flushObserverCalls();
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


