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



#ifndef RYAI_AI_MANAGER_H
#define RYAI_AI_MANAGER_H

#define RYAI_AI_MANAGER_MAX_MANAGERS 1024

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/net/unified_network.h"
#include "ai_share/ai_actions.h"


/*
  ---------------------------------------------------------------------------

  This class defines both the singleton that manages the managers and the
  class type of the managers themselves. The singleton interface can be found
  at the end of the class

  ---------------------------------------------------------------------------
*/

class CAIManager
{

//===================================================================
//  ***        SUB-CLASSES FOR MGR DEFINITON DATA TREE           ***
//===================================================================

public:
	struct SMgrDfnElm 
	{
		SMgrDfnElm() {}
		SMgrDfnElm(uint64 action,const std::vector <CAIActions::CArg> &args): Action(action)
		{
			Args.resize(args.size());
			for (uint i=0;i<args.size();++i)
				Args[i]=args[i];
		}
		SMgrDfnElm(const SMgrDfnElm &other): Action(other.Action), Args(other.Args) {}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			//f.xmlPushBegin("CMD");
				std::string s=getAction();
				//f.xmlSetAttrib("Action");
				f.serial(s);
				setAction(s);
			//f.xmlPushEnd();
				f.serialCont(Args);
			//f.xmlPop();
		}

		void serialToString(std::string &s) throw()
		{
			s+=char(Args.size());
			s+=NLMISC::toString("%*s",sizeof(uint64),"");
			((uint64*)&(s[s.size()]))[-1]=Action;
			for (uint i=0;i<Args.size();++i)
				Args[i].serialToString(s);
		}

		std::string getAction()
		{
			std::string s;
			for (uint i=0;i<8 && ((char *)&Action)[i];++i) s+=((char *)&Action)[i];
			return s;
		}

		void setAction(std::string action)
		{
			Action=0;
			for (uint i=0;i<8 && action[i];++i) ((char *)&Action)[i]=action[i];
		}

		uint64 Action;
		std::vector <CAIActions::CArg> Args;
	};

	struct SMgrDfnNode : public NLMISC::CRefCount
	{
		SMgrDfnNode() {}
		SMgrDfnNode(const std::string &name)
			:	Name(name) 
		{}
		SMgrDfnNode(const SMgrDfnNode &other)
			:	Name(other.Name), 
				Data(other.Data), 
				Child(other.Child) 
		{}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Name);
			f.serialCont(Data);

			uint32 count;
			f.serial(count);
			Child.resize(count);
			if (f.isReading())
			{
				for (uint i=0; i<count; ++i)
				{
					Child[i] = new SMgrDfnNode;
					f.serial(*Child[i]);
				}
			}
			else
			{
				for (uint i=0; i<count; ++i)
				{
					f.serial(*Child[i]);
				}
			}
		}

		void serialToString(std::string &s) throw()
		{
			uint i;
			for (i=0;i<Data.size();++i)
				Data[i].serialToString(s);
			for (i=0;i<Child.size();++i)
				Child[i]->serialToString(s);
		}

		std::string Name;
		std::vector <SMgrDfnElm> Data;
		std::vector <NLMISC::CSmartPtr<SMgrDfnNode> > Child;

		bool Visited;		// used to identify modified nodes in file re-parse operations
	};


//===================================================================
//  ***            START OF THE INSTANTIATED CLASS               ***
//===================================================================


public:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Public methods

	//-------------
	// a few read accessors (static properties)

	// the manager id (0..255)
	sint id() const;
	// the manager name .. ie the source file name minus extension
	const std::string &name() const;

	// the CPU load rating of the manager for auto-load ballancing purposes
	uint weightCPU() const;
	// the RAM load rating of the manager for auto-load ballancing purposes
	uint weightRAM() const;


	//-------------
	// a few read accessors (state of the files on disk)

	// indicates whether newer source files than object files have been located
	bool needCompile() const;
	// indicate whether an object file has been located in the object directory
	bool objExists() const;


	//-------------
	// a few read accessors (relating to assignment to & execution by an ai service)

	// has the manager been opened (it may still be waiting to be assigned)
	bool isOpen() const;
	// has the manager been assigned to a service
	bool isAssigned() const;
	// is the manager up and running on the assigned service
	bool isUp() const;

	// the id of the service to which the manager is assigned
	NLNET::TServiceId serviceId() const;


	//-------------
	// a few basic actions (relating to disk files)

	// compile the source files to generate new object files
	void compile();
	// delete the object files (but not the save files)
	void clean();


	//-------------
	// a few basic actions (relating to assignment to & execution by an ai service)

	// open the manager on an unspecified service 
	// (may be queued until a service is available)
	void open();
	// assign manager to a specified service and begin execution 
	void assign(NLNET::TServiceId serviceId);
	// stop execution on the current service and assign to a new service
	void reassign(NLNET::TServiceId serviceId);
	// stop execution of a manager
	void close();


	//-------------
	// a few basic actions (miscelaneous)

	// display information about the state of the manager
	void display() const;


	//-------------
	// a few write accessors (miscelaneous)

	// set the name assigned to manager
	// if no name previously assigned then reset all manager properties
	// if a name already exists and does not match new name then do nohing and return false
	bool set(const std::string &name);
	// set the state of the needCompile flag
	void setNeedCompile(bool val);
	// set the state of the objFileExists flag
	void setObjFileExists(bool val);
	// set the state of the isUp flag
	void setIsUp(bool val);
	// set the state of the isOpen flag
	void setIsOpen(bool val);


private:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Private methods

	// default constructor - may only be instantiated by the singleton
	CAIManager();	
	// reset properties to initial values
	void _reset();

private:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Private data

	// manager name (file name of source file minus extension)
	std::string _name;
	// cpu rating for load ballancing
	uint _weightCPU;
	// ram rating for load ballancing
	uint _weightRAM;
	// do we need to recompile
	bool _needCompile;
	// does an object file exist
	bool _objExists;
	// is the manager open (at least queued in CAIService singleton)
	bool _isOpen;
	// is manager assigned to a service
	bool _isAssigned;
	// is manager up and running on the assigned service
	bool _isUp;
	// id of the service manager is assigned to
	NLNET::TServiceId _service;


public:
	//---------------------------------------------------
	// INSTANTIATED CLASS: Public data

	// the tree of definition data read from (and written to) data files
	SMgrDfnNode MgrDfnRootNode;


//===================================================================
//  *** END OF THE INSTANTIATED CLASS *** START OF THE SINGLETON ***
//===================================================================


public:
	//---------------------------------------------------
	// SINGLETON: Public methods

	// get the number of valid handles (ie the maximum number of managers allowed)
	static uint maxManagers() { return RYAI_AI_MANAGER_MAX_MANAGERS; }

	// get the number of allocated managers
	static uint numManagers();

	// get a pointer to the manager with given handle (0..maxManagers-1)
	static CAIManager *getManagerById(sint id);

	// get a pointer to the manager with given index (0..numManagers-1)
	static CAIManager *getManagerByIdx(uint idx);

	// get the handle for the manager of given name and optionally create a new
	// handle if none found - return -1 if none found or no free slots
	static sint nameToId(std::string name, bool assignNewIfNotFound=false);

	// clear file name assignments for managers that aren't currently running on 
	// ai services
	static void liberateUnassignedManagers();

private:
	//---------------------------------------------------
	// SINGLETON: Private methods


private:
	//---------------------------------------------------
	// SINGLETON: Private data
	static CAIManager _managers[RYAI_AI_MANAGER_MAX_MANAGERS];


//===================================================================
//  ***                 END OF THE SINGLETON                     ***
//===================================================================

};

#endif
