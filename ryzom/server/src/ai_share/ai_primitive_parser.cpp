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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "ai_primitive_parser.h"

// game share
#include "game_share/utils.h"
#include "game_share/persistent_data.h"

// ai share
#include "../ai_share/ai_share.h"
#include "../ai_share/ai_actions.h"
#include "../ai_share/ai_actions_dr.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;
using namespace AI_SHARE;

//-------------------------------------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
// class CAIPPActions
//-------------------------------------------------------------------------------------------------

class CAIPPActions: public CAIActions::IExecutor, public NLMISC::CSingleton<CAIPPActions>
{
public:
	//----------------------------------------------------------------------------
	// init & release
	
	static void init(CPersistentDataRecord* pdr); 
	static void release();
	
	//----------------------------------------------------------------------------
	// inheritted virtual interface

	virtual void openFile(const std::string &fileName);
	virtual void closeFile(const std::string &fileName);
	virtual void begin(uint32 contextAlias);
	virtual void end(uint32 contextAlias);
	virtual void execute(uint64 action,const std::vector <CAIActions::CArg> &args);

	AI_SHARE::CAIActionsDataRecord& getDataRecord()  { return _DataRecord; }
	
protected:

	
	//----------------------------------------------------------------------------
	// the instance data used by the single instance of the class
	CAIActionsDataRecord _DataRecord;
};



//-------------------------------------------------------------------------------------------------
// static data CAIPPActions
//-------------------------------------------------------------------------------------------------

//CAIPPActions *CAIPPActions::Instance=NULL;


//-------------------------------------------------------------------------------------------------
// methods CAIPPActions
//-------------------------------------------------------------------------------------------------


void CAIPPActions::init(CPersistentDataRecord* pdr) 
{
	getInstance()._DataRecord.init(pdr);
	CAIActions::init(Instance); 
}

void CAIPPActions::release()
{
	if (Instance!=NULL)
	{
		CAIActions::release(); 
		delete Instance;
		Instance=NULL;
	}
}

void CAIPPActions::openFile(const std::string &fileName)
{
	_DataRecord.addOpenFile(fileName);
}

void CAIPPActions::closeFile(const std::string &fileName)
{
	_DataRecord.addCloseFile(fileName);
}

void CAIPPActions::execute(uint64 action,const std::vector <CAIActions::CArg> &args)
{
	_DataRecord.addExecute(action,args);
}

void CAIPPActions::begin(uint32 context)
{
	_DataRecord.addBegin(context);
}

void CAIPPActions::end(uint32 context)
{
	_DataRecord.addEnd(context);
}





void CAIPrimitiveParser::init(CPersistentDataRecord* pdr)
{
	getInstance();
	CAIPPActions::init(pdr);

}

void CAIPrimitiveParser::release()
{
	CAIPPActions::release();
	if (Instance)
	{
		delete Instance;
		Instance = 0;
	}
}
	
void CAIPrimitiveParser::clear()
{
	CAIPPActions::getInstance().getDataRecord().clear();
}

void CAIPrimitiveParser::readFile(const std::string &fileName)
{
	CAIPPActions::getInstance().getDataRecord().readFile(fileName);
}

void CAIPrimitiveParser::writeFile(const std::string &fileName)
{
	CAIPPActions::getInstance().getDataRecord().writeFile(fileName);
}

void CAIPrimitiveParser::display()
{
	CAIPPActions::getInstance().getDataRecord().display();
}

void CAIPrimitiveParser::serial(NLMISC::IStream &stream)
{
	CAIPPActions::getInstance().getDataRecord().serial(stream);
}
