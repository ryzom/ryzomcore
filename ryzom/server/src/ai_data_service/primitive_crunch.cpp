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



//===================================================================


#include "nel/misc/debug.h"

#include "game_share/xml.h"

#include "ai_manager.h"
#include "ai_service.h"
#include "ai_files.h"

//===================================================================

//---------------------------------------------------
// INSTANTIATED CLASS: Public methods

//-------------
// a few read accessors (static properties)

// the manager id (0..255)
sint CAIManager::id() const
{
	return int(this-_managers);
}
// the manager name .. ie the source file name minus extension
const std::string &CAIManager::name() const
{
	return _name;
}

// the CPU load rating of the manager for auto-load ballancing purposes
uint CAIManager::weightCPU() const
{
	return _weightCPU;
}
// the RAM load rating of the manager for auto-load ballancing purposes
uint CAIManager::weightRAM() const
{
	return _weightRAM;
}


//-------------
// a few read accessors (state of the files on disk)

// indicates whether newer source files than object files have been located
bool CAIManager::needCompile() const
{
	return _needCompile;
}
// indicate whether an object file has been located in the object directory
bool CAIManager::objExists() const
{
	return _objExists;
}


//-------------
// a few read accessors (relating to assignment to & execution by an ai service)

// has the manager been opened (it may still be waiting to be assigned)
bool CAIManager::isOpen() const
{
	return _isOpen;
}
// has the manager been assigned to a service
bool CAIManager::isAssigned() const
{
	return _isAssigned;
}
// is the manager up and running on the assigned service
bool CAIManager::isUp()	const
{
	return _isUp;
}

// the id of the service to which the manager is assigned
sint CAIManager::serviceId() const
{
	return _service;
}


//-------------
// a few basic actions (relating to disk files)

const std::string &xmlDelimitedString(CxmlNode *xmlNode,const std::string &delimiter)
{
	static const std::string emptyString;

	for (uint i=0;i<xmlNode->childCount();++i)
	{
		CxmlNode *child=xmlNode->child(i);
		if (child->type()==delimiter)
			if (child->childCount()==1)
				if (child->child(0)->type()=="")
				{
					return child->child(0)->txt(); 
				}
	}

	return emptyString;
}

const std::string &getProp(CxmlNode *xmlNode,const std::string &propertyName)
{
	static const std::string emptyString;

	for (uint i=0;i<xmlNode->childCount();++i)
	{
		CxmlNode *child=xmlNode->child(i);
		if (child->type()=="PROPERTY")
		{
			const std::string &name= xmlDelimitedString(child,std::string("NAME"));
			if (name==propertyName)
				return xmlDelimitedString(child,std::string("STRING"));
		}
	}
	return emptyString;
}

// compile the source files to generate new object files
void CAIManager::compile()
{
	std::string srcFile=CAIFiles::fullSrcFileName(id());
	std::string objFile=CAIFiles::fullObjFileName(id());
	nlinfo("Compile %s => %s",srcFile.c_str(),objFile.c_str());

	// open and parse the xml file
	CxmlNode xmlfile;
	xmlfile.read(srcFile);

	// locate the mgr node in the file
	std::list <CxmlNode *> nodes;
	nodes.push_back(&xmlfile);
	while (!nodes.empty())
	{
		// pop the next node off the list
		CxmlNode *node=*nodes.begin();
		nodes.pop_front();

		// push our children so that they can be treated too
		for (uint i=0;i<node->childCount();++i)
			nodes.push_back(node->child(i));

		// if this is a child node then we're in business so dump a few infos to the screen
		if (node->type()=="CHILD" && !getProp(node,std::string("AI_TYPE")).empty())
		{
			nlinfo("child: AI_TYPE: %s  NAME: %s",getProp(node,std::string("AI_TYPE")).c_str(),getProp(node,std::string("Name")).c_str());
		}
	}
}

// delete the object files (but not the save files)
void CAIManager::clean()
{
	CAIFiles::clean(id());
}


//-------------
// a few basic actions (relating to assignment to & execution by an ai service)

// open the manager on an unspecified service 
// (may be queued until a service is available)
void CAIManager::open()
{
	CAIService::openMgr(id());
}

// assign manager to a specified service and begin execution 
void CAIManager::assign(sint serviceId)
{
	// make sure that the manager isn't assigned to a service already
	if (isAssigned())
	{
		nlwarning("Cannot assign manager %04d (%s) to service %d as already assigned to %d",
			id(), name(), serviceId, _service);
		return;
	}

	// flag me as assigned
	_isAssigned=true;
	_service=serviceId;

	// transfer control to the service's assignMgr() method
	CAIService *service=CAIService::getServiceById(serviceId);
	if (service!=NULL)
		service->assignMgr(id());
}

// stop execution on the current service and assign to a new service
void CAIManager::reassign(sint serviceId)
{
	CAIService *service=CAIService::getServiceById(_service);
	if (service!=NULL)
		service->reassignMgr(id(),serviceId);
}

// stop execution of a manager
void CAIManager::close()
{
	// make sure that the manager isn't assigned to a service already
	if (!isAssigned())
	{
		nlwarning("Cannot unassign manager %04d (%s) as it is already unassigned", id(), name());
		return;
	}

	// if the service is running then transfer control to the service singleton's closeMgr() method
	if (isUp())
	{
		CAIService::closeMgr(id());
		return;
	}

	// flag me as unassigned
	_isAssigned=false;
	_isOpen=false;
	_service=-1;
}


//-------------
// a few basic actions (miscelaneous)

// display information about the state of the manager
void CAIManager::display() const
{
	if (isAssigned())
		nlinfo("AI Manager %04d: %s: %s ON SERVICE %d (%s)", id(), _name.c_str(),
			isUp()?			"UP AND RUNNING": 
			/* else */		"ASSIGNED TO BUT NOT YET UP",
			_service,
			isOpen()?		"auto-assigned": 
			/* else */		"manualy assigned"
			);
	else
		nlinfo("AI Manager %04d: %s: %s", id(), _name.c_str(),
			isOpen()?		"OPEN - AWAITING ASSIGNMENT": 
			!objExists()?	"NOT OPEN - OBJECT FILE NOT FOUND":
			needCompile()?	"NOT OPEN - OBJECT FILE OLDER THAN SOURCE":
			/* else */		"NOT OPEN - OBJECT FILE IS UP TO DATE"
			);
}


//-------------
// a few write accessors (miscelaneous)

// set the name assigned to manager
// if no name previously assigned then reset all manager properties
// if a name already exists and does not match new name then do nohing and return false
bool CAIManager::set(const std::string &name)
{
	// if we already have a name associated with this slot then simply check that it matches the new name
	if (!_name.empty())
		return (_name==name);
	_reset();
	_name=name;
	return true;
}

// set the state of the needCompile flag
void CAIManager::setNeedCompile(bool val)
{
	_needCompile=val;
}

// set the state of the objFileExists flag
void CAIManager::setObjFileExists(bool val)
{
	_objExists=val;
}

// set the state of the isUp flag
void CAIManager::setIsUp(bool val)
{
	_isUp=val;
}

// set the state of the isOpen flag
void CAIManager::setIsOpen(bool val)
{
	_isOpen=val;
}


//---------------------------------------------------
// INSTANTIATED CLASS: Private methods

// default constructor - may only be instantiated by the singleton
CAIManager::CAIManager()
{
	// manager id - make sure that the managers are all in the one static array
	// note that id is calaulated from the array address and the addess of 'this'
	nlassert(uint(id())<maxManagers());
	// reset the rest of the properties
	_reset();
}	

void CAIManager::_reset()
{
	_name.clear();
	_weightCPU		= 0;
	_weightRAM		= 0;
	_needCompile	= false;
	_objExists		= false;
	_isOpen			= false;
	_isAssigned		= false;
	_isUp			= false;
	_service		= -1;
}	


//===================================================================
//  *** END OF THE INSTANTIATED CLASS *** START OF THE SINGLETON ***
//===================================================================


//---------------------------------------------------
// SINGLETON: Data

class CAIManager CAIManager::_managers[RYAI_AI_MANAGER_MAX_MANAGERS];


//---------------------------------------------------
// SINGLETON: Public methods

// get the number of allocated managers
uint CAIManager::numManagers()
{
	uint count=0;
	for (uint i=0;i<maxManagers();i++)
		if (!_managers[i]._name.empty())
			count++;
	return count;
}

// get a pointer to the manager with given handle (0..maxManagers-1)
CAIManager *CAIManager::getManagerById(sint id)
{
	if (uint(id)>=maxManagers())
	{
		nlwarning("CAIManager::getManagerById(id): id %d not in range 0..%d",id,maxManagers()-1);
		return NULL;
	}
	return &(_managers[id]);
}

// get a pointer to the manager with given index (0..numManagers-1)
CAIManager *CAIManager::getManagerByIdx(uint idx)
{
	uint count=0;
	for (uint i=0;i<maxManagers();i++)
		if (!_managers[i]._name.empty())
		{
			if (idx==count)
				return &(_managers[i]);
			count++;
		}
	nlwarning("CAIManager::getManagerByIdx(idx): idx (%d)>=numManagers (%d)",idx,count);
	return NULL;
}

// get the handle for the manager of given name and optionally create a new
// handle if none found - return -1 if none found or no free slots
int CAIManager::nameToId(std::string name, bool assignNewIfNotFound)
{
	// see if the name is a numeric version of an id
	uint val=atoi(name.c_str());
	if (!name.empty() && name.size()<=4 && 
		( (val>0 && val<maxManagers()) || 
		(val==0 && name==std::string("0000"+4-name.size())) ) )
		  return val;

	// see if the name is already assigned to one of the _managers
	for (uint i=0;i<maxManagers();i++)
		if (_managers[i]._name==name)
			return i;

	// the name's not been found so if assignNewIfNotFound then look for a free slot
	if (assignNewIfNotFound)
	{
		for (uint i=0;i<maxManagers();i++)
			if (_managers[i]._name.empty())
			{
				_managers[i].set(name);
				return i;
			}
		nlwarning("Failed to allocate a manager for name '%s' (all %d managers are already allocated)",name.c_str(),maxManagers());
	}
	return -1;
}

// clear file name assignments for managers that aren't currently running on 
// ai services
void CAIManager::liberateUnassignedManagers()
{
	for (uint i=0;i<maxManagers();i++)
		if (!_managers[i]._isOpen && !_managers[i]._isAssigned)
			_managers[i]._reset();
}


//===================================================================
