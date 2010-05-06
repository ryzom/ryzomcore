/*
 * Snowballs service.
 *
 * $Id: empty_service.cpp 409 2007-12-28 13:24:17Z Kaetemi $
 */

#include "empty_service.h"

using namespace SBSERVICE;
using namespace NLMISC;
using namespace NLNET;
using namespace std;


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                               VARIABLES                                ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

// ...


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            BASIC FUNCTIONS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

void CEmptyService::commandStart()
{

}

void CEmptyService::init()
{
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbUp);
}

bool CEmptyService::update()
{
	msgWater("FS", 0.1f);
	return true;
}

void CEmptyService::release()
{

}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            OTHER FUNCTIONS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

// ...


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                            MESSAGE SENDERS                             ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   msgWater
 *             Send WATER message to target service
 * 
 * Arguments:
 *             - service:   target service name
 *             - water:     floating point number
 ****************************************************************************/
void CEmptyService::msgWater(const std::string service, float water)
{
	CMessage msgout("WATER");
	msgout.serial(water);
	CUnifiedNetwork::getInstance()->send(service, msgout);
	nldebug("Sent WATER %f", water);
}

/****************************************************************************
 * Function:   msgFire
 *             Send FIRE message to target service
 * 
 * Arguments:
 *             - sid:       target service id
 *             - fire:      unicode string
 ****************************************************************************/
void CEmptyService::msgFire(TServiceId sid, ucstring fire)
{
	CMessage msgout("FIRE");
	msgout.serial(fire);
	CUnifiedNetwork::getInstance()->send(sid, msgout);
	nldebug("Sent FIRE %s", fire.toString().c_str());
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                           MESSAGE CALLBACKS                            ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   cbSnow
 *             Receives a "SNOW" message with a uint32, increases it by one, 
 *             and forwards it to all frontend services.
 ****************************************************************************/
void CEmptyService::cbSnow(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	uint32 snow;
	msgin.serial(snow);
	nldebug("Received SNOW %u", snow);
	
	// Do something with it
	++snow;
	
	// Send a message to all frontends.
	CMessage msgout("SNOW");
	msgout.serial(snow);
	CUnifiedNetwork::getInstance()->send("FS", msgout);
	nldebug("Sent SNOW %u", snow);
}

/****************************************************************************
 * Function:   cbIce
 *             Receives an "ICE" message with a ucstring, duplicates it,
 *             and replies the result to the sender.
 ****************************************************************************/
void CEmptyService::cbIce(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// Read incoming message
	ucstring ice;
	msgin.serial(ice);
	nldebug("Received ICE %s", ice.toString().c_str());
	
	// Do something with it
	ice += ice;
	
	// Send a message to all frontends.
	CMessage msgout("ICE");
	msgout.serial(ice);
	CUnifiedNetwork::getInstance()->send(sid, msgout);
	nldebug("Sent ICE %s", ice.toString().c_str());
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                           NETWORK CALLBACKS                            ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Function:   cbUp
 *             Called when any service comes up
 ****************************************************************************/
void CEmptyService::cbUp(const string &serviceName, TServiceId sid, void *arg)
{
	// Send a message in reply
	msgFire(sid, ucstring("BURN"));
}


//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///                         SERVICE CONFIGURATION                          ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * CallbackArray
 *
 * It define the initial functions to call when receiving a specific message
 ****************************************************************************/
TUnifiedCallbackItem CallbackArray[] =
{
	{ "SNOW", CEmptyService::cbSnow },
	{ "ICE", CEmptyService::cbIce }
};

/****************************************************************************
 * SNOWBALLS SERVICE MAIN Function
 *
 * This call create a main function for the world_service:
 *
 *    - based on the service class CEmptyService inherited from IService
 *    - having the short name "EMPTY"
 *    - having the long name "empty_service"
 *    - listening on an automatically allocated port (0) by the naming service
 *    - and callback actions set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN(CEmptyService, "EMPTY", "empty_service", 0, CallbackArray, "", "")

/* end of file */
