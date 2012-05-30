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


#ifndef WIDGET_MANAGER_H
#define WIDGET_MANAGER_H

#include <string>
#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/gui/interface_common.h"

class CInterfaceGroup;

/// Manages the GUI widgets
class CWidgetManager{
public:


	struct SMasterGroup
	{
		SMasterGroup()
		{
			Group = NULL;
			LastTopWindowPriority = WIN_PRIORITY_NORMAL;
		}

		CInterfaceGroup *Group;
		std::list< CInterfaceGroup* > PrioritizedWindows[ WIN_PRIORITY_MAX ];

		void addWindow( CInterfaceGroup *pIG, uint8 nPrio = WIN_PRIORITY_NORMAL );
		void delWindow( CInterfaceGroup *pIG );
		CInterfaceGroup *getWindowFromId( const std::string &winID );
		bool isWindowPresent( CInterfaceGroup *pIG );
		// Set a window top in its priority queue
		void setTopWindow( CInterfaceGroup *pIG );
		void setBackWindow( CInterfaceGroup *pIG );
		void deactiveAllContainers();
		void centerAllContainers();
		void unlockAllContainers();

		// Sort the world space group
		void sortWorldSpaceGroup ();

		uint8 LastTopWindowPriority;
	};


	static CWidgetManager* getInstance();
	static void release();
	
	CInterfaceGroup* getMasterGroupFromId( const std::string &MasterGroupName );
	std::vector< SMasterGroup > &getAllMasterGroup(){ return _MasterGroups; }
	SMasterGroup& getMasterGroup( uint8 i ) { return _MasterGroups[ i ]; }
	CInterfaceGroup* getWindowFromId( const std::string &groupId );
	void addWindowToMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
	void removeWindowFromMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
	void removeAllMasterGroups();

private:
	CWidgetManager();
	~CWidgetManager();

	static CWidgetManager *instance;

	// Master groups encapsulate all windows
	std::vector< SMasterGroup > _MasterGroups;
};

#endif

