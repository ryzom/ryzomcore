/** \file service_test.cpp
 * <File description>
 *
 * $Id: service_test.cpp,v 1.20 2004/03/01 19:22:19 lecroart Exp $
 */



#include "service_test.h"
#include "game_share/ryzom_entity_id.h"

#include "nel/misc/command.h"
#include "nel/misc/path.h"

#include "front_end_property_receiver.h"

using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;
using namespace std;

CServiceTest* pServiceTest = NULL;

/**
 *	CallbackArray
 */
TUnifiedCallbackItem CallbackArray[] = 
{
	{	"AZERTY",	NULL	},
};

// includes pour les register class qui suivent (grrrr !!!!)
#include "nel/georges/form_body_elt.h"
#include "nel/georges/form_body_elt_atom.h"
#include "nel/georges/form_body_elt_list.h"
#include "nel/georges/form_body_elt_struct.h"

//---------------------------------------------------
// Service Init :
// 
//---------------------------------------------------
void CServiceTest::init (void)
{
	// init obligatoire pour george (penser à demander l'encapsulation de ça dans une methode init)
	NLMISC_REGISTER_CLASS( CFormBodyElt );
	NLMISC_REGISTER_CLASS( CFormBodyEltAtom );
	NLMISC_REGISTER_CLASS( CFormBodyEltList );
	NLMISC_REGISTER_CLASS( CFormBodyEltStruct );

	setUpdateTimeout(10);
	pServiceTest = this;

//	DebugLog->addNegativeFilter(" ");

	// Define path where to search the sheets.
	CPath::addSearchPath( "service_test_data/sheets", true, false );

	CUnifiedNetwork::getInstance()->setServiceUpCallback ("*", cbServiceUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbServiceDown, 0);

	CPath::addSearchPath( "fe_data/sheets", true, false );
	CFrontEndPropertyReceiver::initFrontEndPropertyReceiver( string("entity_prop.entity_prop"), string("rien") );
}

//---------------------------------------------------
// Service update :
// 
//---------------------------------------------------
bool CServiceTest::update (void)
{
	serviceUpdate();
	return true;
}

//---------------------------------------------------
// Service release :
// 
//---------------------------------------------------
void CServiceTest::release (void)	
{
	CFrontEndPropertyReceiver::freeFrontEndPropertyReceiver();
}

//---------------------------------------------------
// Service release :
// 
//---------------------------------------------------
void CServiceTest::serviceUpdate(void)
{
	//-----------------------------------------------
	// implementation d'exemple
	//
	//-----------------------------------------------

	// Read properties
	uint64 value;
	CFrontEndPropertyReceiver::TPropertiesIndex index = CFrontEndPropertyReceiver::getFirstUpdatedProperties();
	while( index != -1 )
	{
		CFrontEndPropertyReceiver::SEntity* p = CFrontEndPropertyReceiver::getEntity( index );

		bool is_new = ( ( p->code & PROPERTY_CODE_NEW ) != 0 ); // true = new entity
		bool is_del = ( ( p->code & PROPERTY_CODE_ERASE ) != 0 ); // true = deleted entity
		
		if( is_new ) // Entity is new, we can read it's CEntityId and SheetId
		{
			nlinfo( "Entity %s is new, SheetId of entity is %d", p->id.toString().c_str(), p->SheetId );
		}

		if( is_del )
		{
			nlinfo( "Entity %s is deleted", p->id.toString().c_str() );
		}

		uint32 mask = 1;
		for( uint32 i = 0; i < NB_PROPERTIES_PER_ENTITY; ++i )
		{
			if( p->bitfield & mask )
			{
				value = p->properties[i]; //read here a modified property

				nlinfo( "Property %d of entity %s change to value %"NL_I64"d at TGameCyle %d", i, p->id.toString().c_str(), value, p->getGameCycleForProperty( i ) );
			}
			mask <<= 1;
		}
		index = CFrontEndPropertyReceiver::getNextUpdatedProperties( index );
	}
	CFrontEndPropertyReceiver::endUpdatedProperties();

	// read vision
	index = CFrontEndPropertyReceiver::getFirstUpdatedVision();
	uint32 sizeout = 0;	
	uint32 sizein = 0;	
	while( index != -1 )
	{
		CFrontEndPropertyReceiver::SEntity* p = CFrontEndPropertyReceiver::getEntity( index );
		sizeout = p->VisionOut.size();	// p->VisionOut entities out of vision
		sizein = p->VisionIn.size();	// p->VisionIn entities in of vision
		if(sizeout != 0)
		{
			nlinfo("entity out of vision for entity %s", p->id.toString().c_str());
			for( set< CFrontEndPropertyReceiver::TPropertiesIndex >::iterator itOut = p->VisionOut.begin(); itOut != p->VisionOut.end(); ++itOut )
			{
				CFrontEndPropertyReceiver::SEntity* pOut = CFrontEndPropertyReceiver::getEntity( *itOut );
				nlinfo("====> Entity exit of vision: %s", pOut->id.toString().c_str() );
			}
		}
		if(sizein != 0)
		{
			nlinfo("new entity in vision for entity %s", p->id.toString().c_str());
			for( map< CFrontEndPropertyReceiver::TPropertiesIndex, CFrontEndPropertyReceiver::TVisionSlot >::iterator itIn = p->VisionIn.begin(); itIn != p->VisionIn.end(); ++itIn )
			{
				CFrontEndPropertyReceiver::SEntity* pIn = CFrontEndPropertyReceiver::getEntity( (*itIn).first );
				nlinfo("====> Entity enter in vision: %s to Slot %d", pIn->id.toString().c_str(), (*itIn).second );
			}
		}

		index = CFrontEndPropertyReceiver::getNextUpdatedVision( index );
	}
	CFrontEndPropertyReceiver::endUpdatedVision();
}

/****************************************************************\
 ****************************************************************
						Callback functions
 ****************************************************************
\****************************************************************/
// Callback called at service connexion
void cbServiceUp( const string& serviceName, uint16 serviceId, void * )
{
	CFrontEndPropertyReceiver::initFrontEndPropertySubscription( serviceName );
}

// Callback called at service down
void cbServiceDown( const string& serviceName, uint16 serviceId, void * )
{
	CFrontEndPropertyReceiver::serviceDown( serviceId );
}


/****************************************************************\
 ****************************************************************
						Service register
 ****************************************************************
\****************************************************************/
NLNET_SERVICE_MAIN (CServiceTest, "ServiceTest", "service_test", 0, CallbackArray, "", "")


/****************************************************************\
 ****************************************************************
						Command section
 ****************************************************************
\****************************************************************/
// Command for display data changed to receiver
NLMISC_COMMAND(subscribe,"Subscribe to delta update"," ")
{
	// Define path where to search the sheets.
//	CPath::addSearchPath( "service_test_data/sheets", true, false );
//	CFrontEndPropertyReceiver::initFrontEndPropertyReceiver( string("entity_prop.entity_prop") );
	
	list< pair< string, uint32 > > Properties;
	Properties.push_back( make_pair( string("X"), 0x00000001 ) );
	Properties.push_back( make_pair( string("Y"), 0x00000001 ) );
	Properties.push_back( make_pair( string("Z"), 0x00000001 ) );
	Properties.push_back( make_pair( string("Theta"), 0x00000001 ) );
	CFrontEndPropertyReceiver::askPropertiesSubscribe( string("GPMS"), Properties );

	return true;
}





// Command to add an entity to the GPMS
NLMISC_COMMAND(addEntity,"Add entity to GPMS","entity Id, entity PosX(meters), entity PosY, entity PosZ, service Id")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 5) return false;
	
	// get the values
	uint32 Id = atoi(args[0].c_str());
	uint32 PosX = atoi(args[1].c_str()) * 1000;
	uint32 PosY = atoi(args[2].c_str()) * 1000;
	sint32 PosZ = atoi(args[3].c_str()) * 1000;

	uint16 FeId = atoi(args[4].c_str());

	// Init Entity
	CEntityId id;
	id.Type = RYZOMID::player;
	id.Id = Id;
	id.DynamicId = FeId;
	
//	CWorldPositionManager::addEntity(id, 1000 * PosX, 1000*PosY, 1000*PosZ, 0.0f, CTickEventHandler::getGameCycle() - 1/*CTickEventHandler::getGameCycles()*/,/*sheet*/0,FeId);
	CMessage msgout("ADD_ENTITY");

	msgout.serial( id );

	msgout.serial( PosX );
	msgout.serial( PosY );
	msgout.serial( PosZ );
	float theta = 0.0f;
	msgout.serial( theta );

	NLMISC::TGameCycle tick = 0;
//	tick = CTickEventHandler::getGameCycle();
	msgout.serial( tick );
	uint32 sheet = 0;
	msgout.serial( sheet );


	CUnifiedNetwork::getInstance()->send( "GPMS", msgout );

	return true;
}
