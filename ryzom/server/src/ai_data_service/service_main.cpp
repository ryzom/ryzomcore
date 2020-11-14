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



#include "nel/misc/types_nl.h"
#include "nel/net/service.h"
#include "nel/misc/path.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "game_share/ryzom_version.h"
#include "ai_share/ai_share.h"

#include "ai_files.h"
#include "ai_manager.h"
#include "ai_service.h"
#include "aids_actions.h"
#include "messages.h"		// singleton manager for transport class messages

using namespace NLMISC;
using namespace NLNET;
using namespace std;

// The ligo config
NLLIGO::CLigoConfig LigoConfig;

static TUnifiedCallbackItem CallbackArray[] = 
{
	{	"QWERTY",			0,	},
};

extern string				OutputPath;
extern vector<string>		PacsPrimPath;
extern vector<string>		LookupPath;
extern vector<string>		LookupNoRecursePath;


/*-----------------------------------------------------------------*\
						SERVICE CLASS
\*-----------------------------------------------------------------*/

class CAIDataService : public NLNET::IService
{
public:
	void init();
	bool update();
	void release();
};

// callback for the tick service 'tick' message
static void cbTick();


/*-----------------------------------------------------------------*\
						SERVICE INIT & RELEASE
\*-----------------------------------------------------------------*/

///init

void CAIDataService::init (void)
{
	setVersion (RYZOM_VERSION);

	// this is the init for the ligo pri;itive parser lib
	NLLIGO::Register ();

	// setup the update systems
	setUpdateTimeout(200);

	// Init ligo
	if (!LigoConfig.readPrimitiveClass ("world_editor_classes.xml", false))
	{
		// Should be in l:\leveldesign\world_edit_files
		nlerror ("Can't load ligo primitive config file world_editor_classes.xml");
	}

	// init sub systems
	AI_SHARE::init(&LigoConfig);
	CMessages::init();
	CAIDSActions::init();


	//
	CConfigFile			&cf = ConfigFile;
	CConfigFile::CVar	*var;

	var = cf.getVarPtr("Paths");
	if (var != NULL)
	{
		uint	i;
		for (i=0; (sint)i<var->size(); ++i)
			LookupPath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("NoRecursePaths");
	if (var != NULL)
	{
		uint	i;
		for (i=0; (sint)i<var->size(); ++i)
			LookupNoRecursePath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("PacsPrimPaths");
	if (var != NULL)
	{
		uint	i;
		for (i=0; (sint)i<var->size(); ++i)
			PacsPrimPath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("OutputPath");
	if (var != NULL)
	{
		OutputPath = CPath::standardizePath(var->asString());
	}
}


///release

void CAIDataService::release (void)	
{
	// release sub systems
	CMessages::release();
}


/*-----------------------------------------------------------------*\
						SERVICE UPDATES
\*-----------------------------------------------------------------*/

///update called every coplete cycle of service loop

bool CAIDataService::update (void)
{
	CAIService::update();
	return true;
}

/*-----------------------------------------------------------------*\
						NLNET_SERVICE_MAIN
\*-----------------------------------------------------------------*/
NLNET_SERVICE_MAIN (CAIDataService, "AIDS", "ai_data_service", 0, CallbackArray, "", "")

