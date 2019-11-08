/** \file sabrina_test.h
 *
 * $Id: sabrina_test.h,v 1.2 2004/03/01 19:22:19 lecroart Exp $
 */




#ifndef GD_SABRINA_TEST_H
#define GD_SABRINA_TEST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/net/service.h"
#include "player_manager.h"
#include "bot_manager.h"

//------------------------------------------------------------------------
// Class for the srevice objects...

class CServiceTest : public NLNET::IService
{
public:
	// Service basics...
	void init ();
	bool update ();
	void release ();

	static void tickUpdate();

	// accessors for entoty managers
	CBotManager* getBotManager()
	{
		return &_BotManager;
	}

	CPlayerManager* getPlrManager()
	{
		return &_PlayerManager;
	}

private:
	CBotManager _BotManager;
	CPlayerManager _PlayerManager;
};

//------------------------------------------------------------------------
// Globals and Handy routines for accessing the service object

namespace SABTEST
{
	extern CServiceTest* TheService;

	inline CPlayerManager* plrMgr()
	{
		return TheService->getPlrManager();
	}

	inline CBotManager* botMgr()
	{
		return TheService->getBotManager();
	}
}

#endif // GD_SERVICE_TEST_H


/* End of sabrina_test.h */
