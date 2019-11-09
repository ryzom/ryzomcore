/*
	Utils tests - macros that assert or break execution

	project: RYZOM / TEST

*/

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "game_share/singleton_registry.h"
#include "game_share/utils.h"

static class CUtilsAssertTest: public IServiceSingleton
{
public:
	void init() 
	{
		// warning system tests
		WARN("WARN");
		STOP("STOP");
		BOMB("BOMB",nldebug("BOMB action"));
		WARN_IF(true,"WARN_IF");
		STOP_IF(true,"STOP_IF");
		BOMB_IF(true,"BOMB_IF",nldebug("BOMB_IF action"));

		// this should fail in debug but not in release
		nlassertd(false);

		// testing the onChange macros
		int i=0;
		{
			ON_CHANGE_ASSERT(int,i);
			{
				++i;
				ON_CHANGE_ASSERT(int,i);
				// this should drop through fine
			}
			// there should be an assert here
			nldebug("There should be an assert after this message");
		}
		nldebug("There should be an assert before this message");

	}

}
Test;
