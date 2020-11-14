/** \file tick_service.h
 * The TICK SERVICE deals with time management in the shard
 *
 * $Id: test.h,v 1.1 2004/06/07 15:26:49 miller Exp $
 */

#ifndef TEST_H
#define TEST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"

#include "nel/net/service.h"

/**
 * CTest
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CTest : public NLNET::IService
{
public :

	/// Initialise the service
	void init();

	/// Update
	bool update();

	/// Tick Update
	static void tickUpdate();

	/// Release
	void release();

};


#endif //TICK_S_H
