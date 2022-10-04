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



#ifndef RYAI_AIDS_ACTIONS_H
#define RYAI_AIDS_ACTIONS_H

#include "ai_share/ai_actions.h"
#include "ai_manager.h"
#include <vector>

class CAIDSActions: public CAIActions::IExecutor
{
public:
	//----------------------------------------------------------------------------
	// init & release
	
	static void init() 
	{
		if (Instance==NULL)
			Instance=new CAIDSActions;
		CAIActions::init(Instance); 
	}
	static void release()
	{
		if (Instance!=NULL)
		{
			CAIActions::release(); 
			delete Instance;
			Instance=NULL;
		}
	}
	
	//----------------------------------------------------------------------------
	// inheritted virtual interface

	virtual void openFile(const std::string &fileName);
	virtual void closeFile(const std::string &fileName);
	virtual void begin(const std::string &contextName);
	virtual void end(const std::string &contextName);
	virtual void execute(uint64 action,const std::vector <CAIActions::CArg> &args);

	virtual void	begin(uint32 context)	{}
	virtual void	end(uint32 context)	{}

	//----------------------------------------------------------------------------
	// public singleton data

	static uint CurrentManager;

private:
	//----------------------------------------------------------------------------
	// This is a singleton class so make constructor private
	CAIDSActions() {}
	~CAIDSActions() {}

	//----------------------------------------------------------------------------
	// the singleton class instance
	static CAIDSActions *Instance;
};


#endif

