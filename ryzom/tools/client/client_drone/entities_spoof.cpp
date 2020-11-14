/** \file entities_spoof.cpp
 * Spoofs of Ryzom client entity classes
 *
 * $Id: entities_spoof.cpp
 */

#include "nel/net/module_manager.h"
#include "entities_spoof.h"
#include "simulated_client.h"
//#include "../client/net_manager.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"

using namespace NLMISC;

extern CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;

/*
// extends spoofed client CUserEntity class 
 */
void CUserEntity::sendToServer(CBitMemStream &out)
{
	// Send Position & Orientation
	nlverify(GenericMsgHeaderMngr.pushNameToStream("POSITION", out));
	CPositionMsg positionMsg;
	positionMsg.X = (sint32)(pos().x * 1000);
	positionMsg.Y = (sint32)(pos().y * 1000);
	positionMsg.Z = (sint32)(pos().z * 1000);
	positionMsg.Heading = frontYaw(); //frand(6.28f);
	out.serial(positionMsg);
}



