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



#include "stdpch.h"
#include "visual_properties_interface.h"
#include "ai.h"
#include "mirrors.h"
#include "ai_instance.h"
#include "ai_mgr.h"
#include "game_share/synchronised_message.h"

//#include "nel/misc/ucstring.h"

/*
// Nel Include
#include "nel/net/unified_network.h"
#include "game_share/mode_and_behaviour.h"
#include "visual_properties_interface.h"
*/

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//-----------------------------------------------
/// Macro to read a Bool from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define READ_BOOL(variableName)												\
	/* Read the Variable Value From Script */								\
	{CConfigFile::CVar *varPtr = IService::getInstance()->ConfigFile.getVarPtr(#variableName);	\
	/* Value found, set the Variable */										\
	if(varPtr)																\
		variableName = varPtr->asInt() ? true : false;						\
	nlinfo("IOS Entity naming constrol: "#variableName" %d",variableName?"1":"0");}

bool CVisualPropertiesInterface::UseIdForName=false;
bool CVisualPropertiesInterface::ForceNames=false;

// classic init(), update() and release()
void CVisualPropertiesInterface::init()
{
	READ_BOOL(UseIdForName);
	READ_BOOL(ForceNames);
}

void	CVisualPropertiesInterface::update()
{
}

void	CVisualPropertiesInterface::release()
{
}

//	set different visual properties for a bot.
void	CVisualPropertiesInterface::setName(const TDataSetRow&	dataSetRow, ucstring name)
{
	if (!IOSHasMirrorReady)
		return;
	
	NLNET::CMessage	msgout("CHARACTER_NAME");
	CEntityId		eid=CMirrors::DataSet->getEntityId(dataSetRow);
	msgout.serial	(const_cast<TDataSetRow&>(dataSetRow));
	msgout.serial	(name);
	sendMessageViaMirror("IOS",msgout);
}
