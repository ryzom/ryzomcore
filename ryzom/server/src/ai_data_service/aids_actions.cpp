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




//----------------------------------------------------------------------------

#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/net/service.h"
#include "aids_actions.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//----------------------------------------------------------------------------
// Stuff for debug message logging
//----------------------------------------------------------------------------

static bool verboseLog=false;
#define TAB toString("%*s",CurrentMgrDfnNodes.size()*4,"").c_str()
#define TAB1 toString("%*s",CurrentMgrDfnNodes.size()*4-4,"").c_str()
#define LOG if (!verboseLog) {} else nlinfo

NLMISC_COMMAND(verboseAIDSActionLog,"Turn on or off or check the state of verbose AIDSAction parser logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(verboseLog, args[0]);

	nlinfo("verboseAIDSActionLogging is %s",verboseLog?"ON":"OFF");
	return true;
}


//----------------------------------------------------------------------------
// Singleton data instantiation
//----------------------------------------------------------------------------

CAIDSActions *CAIDSActions::Instance=NULL;
uint CAIDSActions::CurrentManager=0;


//----------------------------------------------------------------------------
// Local utility functions and variables
//----------------------------------------------------------------------------

static uint BadParseDepth=0;
static CAIManager::SMgrDfnNode DefaultNode;
static std::vector<CAIManager::SMgrDfnNode *> CurrentMgrDfnNodes;
static bool FoundAIData=false;
static std::string FileName;

static void Prepare(CAIManager::SMgrDfnNode &node)
{
	// recurse through children
	for (uint i=0;i<node.Child.size();++i)
		Prepare(*node.Child[i]);

	// deal with self
	node.Visited=false;
	node.Data.clear();
}

static void Clean(CAIManager::SMgrDfnNode &node)
{
	// recurse through children
	for (uint i=0;i<node.Child.size();++i)
		Clean(*node.Child[i]);

	// deal with self
	if (!node.Visited)
	{
		node.Child.clear();
		node.Name.clear();		// clearing the name makes the slot re-usable next compile
	}
}

//----------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------

void CAIDSActions::openFile(const std::string &fileName)
{
	LOG("Scanning file: %s",fileName.c_str());

	// setup local vars
	BadParseDepth=0;
	DefaultNode.Child.clear();
	DefaultNode.Data.clear();
	CurrentMgrDfnNodes.clear();
	FoundAIData=false;
}

void CAIDSActions::closeFile(const std::string &fileName)
{
	// if the primitive file didn't contain anything interesting then add it to the ignore list
	if (!FoundAIData)
	{
		nlinfo("No manager found in primitive file: %s - adding to ignore list",fileName.c_str());

		// lookup the ignore files in the config file and copy into a temp vector
		CConfigFile::CVar *varPtr=IService::getInstance()->ConfigFile.getVarPtr(std::string("IgnorePrimitives"));
		std::vector<std::string> ignoreFiles;
		for (uint i=0;i<varPtr->size();++i)
			ignoreFiles.push_back(CFile::getFilenameWithoutExtension(varPtr->asString(i)));
		
		//append this file to the vector and pass it back to the config file manager for storage
		ignoreFiles.push_back(NLMISC::CFile::getFilenameWithoutExtension(fileName));
		varPtr->setAsString(ignoreFiles);
		IService::getInstance()->ConfigFile.save();
	}
}

void CAIDSActions::begin(const std::string &contextName)
{
	LOG("%s{ // %s",TAB,contextName.c_str());
	uint i;

	// if we've encountered a parse problem just count levels of indentation
	if (BadParseDepth)
	{
		++BadParseDepth;
		return;
	}

	// if the CurrentMgrDfnNodes vector is empty then we must be parsing a new manager
	if (CurrentMgrDfnNodes.empty())
	{
		nlinfo("- Parsing: %s",contextName.c_str());

		// update variables used in this source file (AIDS_ACTIONS.CPP)
		FoundAIData=true;
		CurrentMgrDfnNodes.push_back(&(CAIManager::getManagerById(CurrentManager)->MgrDfnRootNode));
		CurrentMgrDfnNodes[0]->Name=contextName;

		// reset the 'visited' flags for the manager's data tree
		Prepare(*CurrentMgrDfnNodes[0]);
		CurrentMgrDfnNodes[0]->Visited=true;

		// copy the default dfn nodes to the current node and add a 'set_slot' action
		CAIManager::SMgrDfnNode *curNode=CurrentMgrDfnNodes[CurrentMgrDfnNodes.size()-1];
		for (uint i=0;i<DefaultNode.Data.size();++i)
			curNode->Data.push_back(DefaultNode.Data[i]);
		// add a set_slot action with value uint slot id = CurrentManager
		std::vector <CAIActions::CArg> args;
		args.push_back(CAIActions::CArg(CurrentManager));
		curNode->Data.push_back(CAIManager::SMgrDfnElm(*(uint64*)"SET_SLOT",args));
	}
	else
	{
		CAIManager::SMgrDfnNode *curNode=CurrentMgrDfnNodes[CurrentMgrDfnNodes.size()-1];

		// try to find a node with the name matching contextName
		// if there are a number of nodes with the same name then this code should work ok as
		// it uses the 'visited' flags to avoid multi-use of same node slot
		CAIManager::SMgrDfnNode *node=NULL;
		for (i=0;i<curNode->Child.size();++i)
			if ((curNode->Child[i]->Name.empty() || curNode->Child[i]->Name==contextName) && !curNode->Child[i]->Visited)
			{
				node=(curNode->Child[i]);
				break;
			}

		// if need be allocate a new data node 
		if (i==curNode->Child.size())
		{
			curNode->Child.push_back(new CAIManager::SMgrDfnNode(contextName));
			node=curNode->Child[i];
		}

		// add a set_slot action with value uint slot id = i
		std::vector <CAIActions::CArg> args;
		args.push_back(CAIActions::CArg(i));
		curNode->Child[i]->Data.push_back(CAIManager::SMgrDfnElm(*(uint64*)"SET_SLOT",args));

		// update variables used in this source file (AIDS_ACTIONS.CPP)
		CurrentMgrDfnNodes.push_back(node);
		node->Visited=true;
	}
}

void CAIDSActions::end(const std::string &contextName)
{
	LOG("%s} // %s",TAB1,contextName.c_str());

	// if we've encountered a parse problem just count levels of indentation
	if (BadParseDepth)
	{
		--BadParseDepth;
		return;
	}

	// make sure the name of the context that we're closing matches the name of the context that we opened
	CAIManager::SMgrDfnNode *curNode=CurrentMgrDfnNodes[CurrentMgrDfnNodes.size()-1];
	nlassert(contextName==curNode->Name);

	// close this context
	CurrentMgrDfnNodes.pop_back();

	// if we've finished then do some housekeeping
	if (CurrentMgrDfnNodes.empty())
	{
		// run back through tree stripping out unvisitted entries
		Clean(*curNode);
	}
}

void CAIDSActions::execute(uint64 action,const std::vector <CAIActions::CArg> &args)
{
	// if we've encountered a parse problem don't execute...
	if (BadParseDepth)
	{
		return;
	}

	// generate a string to describe the action
	std::string txt;
	txt+=toString("%-8.8s",&action);
	txt+='(';
	for (uint i=0;i<args.size();++i)
	{
		txt+=args[i].toString();
		if (i<args.size()-1) txt+=", ";
	}
	txt+=')';
	LOG("%s%s",TAB,txt.c_str());

	CAIManager::SMgrDfnNode *curNode;

	// if we havent yet opened our first context then store this data record in the default node
	// otherwise store it in the node on the top of the CurrentMgrDfnNodes vector
	if (CurrentMgrDfnNodes.empty())
		curNode=&DefaultNode;
	else
		curNode=CurrentMgrDfnNodes[CurrentMgrDfnNodes.size()-1];

	// add the data record to the top node on the CurrentMgrDfnNodes vector
	curNode->Data.push_back(CAIManager::SMgrDfnElm(action,args));
}

