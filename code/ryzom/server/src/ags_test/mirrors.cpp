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



#include "mirrors.h"
#include <nel/net/unified_network.h>
#include "game_share/ryzom_entity_id.h"
//#include <nel/misc/command.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace MBEHAV;

//static inits
CMirror				CMirrors::Mirror;
CMirroredDataSet	*CMirrors::DataSet = NULL;


/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	CMirrors::initMirror();
}


/*
 * Initialisation 1
 */
void CMirrors::init( void (*cbUpdate)(), void (*cbSync)(), void (*cbRelease)() )
{
	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	Mirror.init( datasetNames, cbMirrorIsReady, cbUpdate, cbSync, cbRelease );

	// register the service up and service down callbacks
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbServiceUp, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, 0);
}


/*
 * Init after the mirror init
 */
void CMirrors::initMirror()
{
	Mirror.declareEntityTypeOwner( RYZOMID::npc, 10000 );
	Mirror.declareEntityTypeOwner( RYZOMID::creature, 10000 );

	DataSet = &(Mirror.getDataSet("fe_temp"));
	DataSet->declareProperty( "X", PSOReadWrite );
	DataSet->declareProperty( "Y", PSOReadWrite );
	DataSet->declareProperty( "Z", PSOReadWrite );
	DataSet->declareProperty( "Theta", PSOReadWrite );
	DataSet->declareProperty( "Sheet", PSOReadWrite ); // read/write
	DataSet->declareProperty( "Mode", PSOReadWrite );
	DataSet->declareProperty( "Behaviour", PSOReadOnly );
	DataSet->declareProperty( "VisualPropertyA", PSOReadWrite );
	DataSet->declareProperty( "WhoSeesMe", PSOReadWrite );
	DataSet->declareProperty( "RiderEntity", PSOReadOnly );
	DataSet->declareProperty( "TargetList", PSOReadWrite );

	Mirror.setNotificationCallback( CMirrors::processMirrorUpdates );

	// Note: here we don't call initRyzomVisualPropertyIndices(), so we won't be able to used the
	// DSProperty* "constants" in this service
}


void CMirrors::processMirrorUpdates()
{
	DataSet->processAddedEntities();
	DataSet->processRemovedEntities();
}


void CMirrors::release()
{
	// Calling the destructor explicitely, not waiting for the end
	Mirror.release();
}

void CMirrors::cbServiceUp( const std::string& serviceName, uint16 serviceId, void * )
{
}

void CMirrors::cbServiceDown( const std::string& serviceName, uint16 serviceId, void * )
{
}



/*
 * Test of CMirrorPropValueList
 */
/*NLMISC_COMMAND( testMirrorList, "", "" )
{
	CEntityId id = CEntityId( RYZOMID::npc, 99987, 0x82, 0x85 );
	CMirrors::Mirror.addEntity( id, true );
	TDataSetRow datasetRow = TheDataset.getDataSetRow( id );
	CMirrorPropValueList<TDataSetRow> list( TheDataset, id, "TargetList" );
	log.displayNL( "SIZE %u", list.size() );
	list.resize( 2 );
	log.displayNL( "SIZE %u", list.size() );
	list.push_front( datasetRow );
	CMirrorPropValueList<TDataSetRow>::iterator it;
	log.displayNL( "SIZE %u", list.size() );
	for ( it=list.begin(); it!=list.end(); ++it )
		log.displayNL( "%u", (*it)().getIndex() );
	list.resize( 2 );
	log.displayNL( "SIZE %u", list.size() );
	for ( it=list.begin(); it!=list.end(); ++it )
		log.displayNL( "%u", (*it)().getIndex() );
	list.push_front( datasetRow );
	log.displayNL( "SIZE %u", list.size() );
	list.resize( 4 );
	log.displayNL( "SIZE %u", list.size() );
	for ( it=list.begin(); it!=list.end(); ++it )
		log.displayNL( "%u", (*it)().getIndex() );
	return true;
}*/
