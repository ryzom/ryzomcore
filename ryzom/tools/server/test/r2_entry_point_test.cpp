/** \file r2_entry_point_test.cpp
 *
 * $id$
 *
 */

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/hierarchical_timer.h"
#include "game_share/utils.h"
#include "game_share/timer.h"
#include "r2_share/scenario_entry_points.h"

// #define ENABLE_TESTS
#include "game_share/sadge_tests.h"

using namespace NLMISC;

class CR2EntryPointTest: public IServiceSingleton
{
public:
	void test1(const CSString& packageDefinition)
	{
		// display a fancy title line
		nlinfo("<------------------------------------------------------------------------------------------------------>");
		nlinfo("Displaying scenario entry points correspoding to package definition: '%s'",packageDefinition.c_str());

		// get the vector of islands that we are allowed access to
		CVectorSString islands;
		R2::CScenarioEntryPoints::getInstance().getIslands(packageDefinition,islands);

		// run through the islands displaying them with their lists of entry points
		for (uint32 i=0;i<islands.size();++i)
		{
			nlinfo("- Island: %s", islands[i].c_str());

			// get the set of valid entry points for this island
			CVectorSString entryPoints;
			R2::CScenarioEntryPoints::getInstance().getEntryPoints(packageDefinition,islands[i],entryPoints);

			// for each entry point...
			for (uint32 j=0;j<entryPoints.size();++j)
			{
				// lookup the entry point id from the package definition, island name and entry point name
				uint32 entryPointId= R2::CScenarioEntryPoints::getInstance().getEntryPointId(packageDefinition,islands[i],entryPoints[j]);

				// get the coordinates for the given entry point
				sint32 x, y;
				R2::CScenarioEntryPoints::getInstance().getEntryPointCoordsFromId(packageDefinition,entryPointId,x,y);

				// display a debug message
				nlinfo("    - Entry point %d: %s (%d,%d)", entryPointId, entryPoints[j].c_str(), x, y);
			}
		}
		nlinfo(">------------------------------------------------------------------------------------------------------<");
	}

	void init()
	{
		CPath::addSearchPath("./data_common/r2",1,0);
		test1("j");
		test1("j1");
		test1("j2");
		test1("j1:j2");
	}

	void serviceUpdate()
	{
	}

private:
};

static CR2EntryPointTest R2EntryPointTest;
