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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/algo.h"
#include "nel/misc/command.h"
#include "nel/net/unified_network.h"

// game share
#include "game_share/singleton_registry.h"
#include "game_share/utils.h"

// local
#include "gus_module.h"
#include "gus_module_factory.h"
#include "ss_command_executor.h"
#include "ss_state_manager.h"
#include "ss_service_comms_manager.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// class CServiceCommsManagerImplementation
//-----------------------------------------------------------------------------
// A singleton for handling the basics comms activities
//
// TRequestId uses positive values for dynamically allocated request ids and negative values
// for reserved request ids

class CServiceCommsManagerImplementation
:	public CServiceCommsManager, 
	public IModule
{
public:
	// get hold of the singleton instance
	static CServiceCommsManagerImplementation* getInstance();

private:
	// this is a singleton so prohibit instantiation
	CServiceCommsManagerImplementation();
	void clear();

public:
	// public typedefs
	typedef sint32 TRequestId;

	// GUS::IModule methods
	bool initialiseModule(const NLMISC::CSString& rawArgs);
	void release();
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

	// methods used for network callbacks
	void serviceUp(NLNET::TServiceId serviceId,const std::string& serviceName);
	void serviceDown(NLNET::TServiceId serviceId,const std::string& serviceName);
	static void cbView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid);

	// methods for treating 'VIEW' messages
	void requestVariableLookup(NLNET::TServiceId sid,const CSString& cmdTxt);
	void treatReplyToServiceUpGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid);
	void treatReplyToVariableSetGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid);
	void treatReplyToVariableLookupGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid,const CSString& cmdTxt);

	// public interface exposed via CServiceManager class
	void execute(const string& serviceNameWildcard,const string& cmdLine);
	void display(CLog* log=InfoLog);

private:
	// private data
	// module control - module parameters and flag to say whether module is active
	bool _IsActive;

	typedef map<NLNET::TServiceId,string> TServiceIds;
	TServiceIds _ServiceIds;

	typedef map<TRequestId,CSString> TPendingViewRequests;
	TPendingViewRequests _PendingViewRequests;
};


//-------------------------------------------------------------------------------------------------
// Management of GET_VIEW / VIEW messages
//-------------------------------------------------------------------------------------------------

// TRequestId uses positive values for dynamically allocated request ids and negative values
// for reserved request ids
const CServiceCommsManagerImplementation::TRequestId ReservedRIDServiceUp = -1;
const CServiceCommsManagerImplementation::TRequestId ReservedRIDVariableSet = -2;

static CServiceCommsManagerImplementation::TRequestId allocateNewRid()
{
	static CServiceCommsManagerImplementation::TRequestId nextFreeRid=0;
	if (nextFreeRid<0) nextFreeRid=0;
	return nextFreeRid++;
}

static void requestView(NLNET::TServiceId sid,CServiceCommsManagerImplementation::TRequestId rid,const CSString& viewTxt)
{
	CMessage msgout("GET_VIEW");
	msgout.serial(rid);
	msgout.serial(const_cast<CSString&>(viewTxt));
	CUnifiedNetwork::getInstance()->send(sid,msgout);
}

void CServiceCommsManagerImplementation::cbView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid)
{
	nldebug("SS Receiving 'view' message: %3d: %s",sid.get(),serviceName.c_str());

	TRequestId rid;
	msgin.serial(rid);

	switch(rid)
	{
	case ReservedRIDServiceUp:
		getInstance()->treatReplyToServiceUpGetView(msgin,serviceName,sid);
		break;

	case ReservedRIDVariableSet:
		getInstance()->treatReplyToVariableSetGetView(msgin,serviceName,sid);
		break;

	default:
		BOMB_IF(getInstance()->_PendingViewRequests.find(rid)==getInstance()->_PendingViewRequests.end(),"VIEW message received with unrecognised request id",return);
		getInstance()->treatReplyToVariableLookupGetView(msgin,serviceName,sid,getInstance()->_PendingViewRequests[rid]);
		getInstance()->_PendingViewRequests.erase(rid);
	}
}


//-------------------------------------------------------------------------------------------------
// Management of command execution
//-------------------------------------------------------------------------------------------------

static void executeCommand(NLNET::TServiceId sid,const CSString& cmdLine)
{
	// compose and send the command execution message
	CMessage msgout("EXEC_COMMAND");
	msgout.serial(const_cast<CSString&>(cmdLine));
	CUnifiedNetwork::getInstance()->send(sid,msgout);
}

static bool isValidSetVarCmdLine(const CSString& cmdTxt)
{
	// keyVarName#desiredVal.targetVarName=value
	if (!cmdTxt.word(0).isValidKeyword()) return false;
	if (!cmdTxt.word(2).isValidKeyword()) return false;
	if (!cmdTxt.word(4).isValidKeyword()) return false;
	if (cmdTxt.word(1)!='#') return false;
	if (cmdTxt.word(3)!='.') return false;
	if (cmdTxt.word(5)!='=') return false;
	if (cmdTxt.word(6).empty()) return false;

	return true;
}

static bool splitSetVarCmdLine(const CSString& cmdTxt,CSString& keyVarName,CSString& desiredVal,CSString& targetVarName,CSString& newVal)
{
	// keyVarName#desiredVal.targetVarName=value
	CSString preEquals=cmdTxt.splitTo('=').strip();
	newVal=cmdTxt.splitFrom('=').strip();
	keyVarName=preEquals.word(0);
	desiredVal=preEquals.word(2);
	targetVarName=preEquals.word(4);

	// do this at the end to ensure that the returned variables are initialised first
	BOMB_IF(!isValidSetVarCmdLine(cmdTxt),"Invalid var set format: "+cmdTxt,return false);

	return true;
}

void CServiceCommsManagerImplementation::requestVariableLookup(NLNET::TServiceId sid,const CSString& cmdTxt)
{
	// generate the text to send in the GET_VIEW message
	CSString keyVarName,desiredVal,targetVarName,newVal;
	if(!splitSetVarCmdLine(cmdTxt,keyVarName,desiredVal,targetVarName,newVal)) return;
	CSString viewTxt=NLMISC::toString("*.[entity,%s,%s]",keyVarName.c_str(),targetVarName.c_str());

	// generate a new request id
	TRequestId rid;
	rid=allocateNewRid();

	// send the GET_VIEW message
	requestView(sid,rid,viewTxt);

	// store our command in the pending commands map - for retrieval later
	_PendingViewRequests[rid]=cmdTxt;
}

void CServiceCommsManagerImplementation::treatReplyToVariableLookupGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid,const CSString& cmdTxt)
{
	CSString keyVarName;
	CSString desiredVal;
	CSString targetVarName;
	CSString newVal;
	bool allOK=true;

	allOK=splitSetVarCmdLine(cmdTxt,keyVarName,desiredVal,targetVarName,newVal);
	if (!allOK) return;

	nldebug("SS Scanning VIEW message for var %s val %s to set var %s to val %s",keyVarName.c_str(),desiredVal.c_str(),targetVarName.c_str(),newVal.c_str());
	nldebug("SS Message pos: %u, Message size: %u",(uint32)msgin.getPos(),(uint32)msgin.length());  

	uint32 successCount=0;
	while ((uint32)msgin.getPos() < msgin.length())
	{
		vector<string> var;
		vector<string> val;
		try
		{
			msgin.serialCont(var);
			msgin.serialCont(val);
			BOMB_IF(var.size()!=val.size(),"Bad VIEW message format!",return);
		}
		catch(...)
		{
			BOMB("Bad VIEW message format!",return);
		}

		string oldVal;
		string keyVal;
		string keyId;
		for (uint32 i=0;i<var.size();++i)
		{
			if (var[i]==targetVarName) oldVal=val[i];
			if (var[i]==keyVarName) keyVal=val[i];
			if (var[i]=="entity") keyId=val[i];
		}
		if (keyVal==desiredVal)
		{
			CSString viewTxt= keyId+"."+targetVarName+"="+newVal;
			nldebug("SS Executing variable assignment (via GET_VIEW): %s(%s-%d): %s OLD VALUE: %s",
				serviceName.c_str(),
				_ServiceIds[sid].c_str(),
				sid.get(),
				viewTxt.c_str(),
				oldVal.c_str());
			requestView(sid,ReservedRIDVariableSet,viewTxt);
			++successCount;
		}
	}
	nldebug("SS Scanning VIEW ... found %d matches",successCount);
}

void CServiceCommsManagerImplementation::treatReplyToVariableSetGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid)
{
	nldebug("SS received VIEW message to acknowledged setVar");
}

void CServiceCommsManagerImplementation::execute(const string& serviceNameWildcard,const string& cmdLine)
{
	// treat the special case where the wildcard is "**this**" whch means this service and not some other
	if (serviceNameWildcard=="**this**")
	{
		// execute the command on this service and exit
//		NLMISC::InfoLog->displayNL("Executing command : '%s'",cmdLine.c_str());
		ICommand::execute(cmdLine,*NLMISC::InfoLog);
		return;
	}

	// iterate over all connected services looking for wildcard matches to execute command on
	for (TServiceIds::iterator it=getInstance()->_ServiceIds.begin();it!=getInstance()->_ServiceIds.end();++it)
	{
		if (testWildCard((*it).second,serviceNameWildcard))
		{
			if (isValidSetVarCmdLine(cmdLine))
			{
				// compose and send the command execution message
				nldebug("SS sending service a view request for executing variable set: SERVICE: %s(%d): CMD: %s",(*it).second.c_str(),(*it).first.get(),cmdLine.c_str());
				requestVariableLookup((*it).first,cmdLine);
			}
			else
			{
				// compose and send the command execution message
				nldebug("SS executing command: SERVICE: %s(%d): CMD: %s",(*it).second.c_str(),(*it).first.get(),cmdLine.c_str());
				executeCommand((*it).first,cmdLine);
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// CServiceCommsManagerImplementation Singleton Management
//-------------------------------------------------------------------------------------------------

CServiceCommsManagerImplementation::CServiceCommsManagerImplementation()
{
	_IsActive= false;
}

CServiceCommsManagerImplementation* CServiceCommsManagerImplementation::getInstance()
{
	static NLMISC::CSmartPtr<CServiceCommsManagerImplementation> ptr;//=NULL;
	if (ptr==NULL)
		ptr= new CServiceCommsManagerImplementation;
	return ptr;
}


//-------------------------------------------------------------------------------------------------
// displaying the state of the service comms handler singleton
//-------------------------------------------------------------------------------------------------

void CServiceCommsManagerImplementation::display(CLog* log)
{
	nlassert(log!=NULL);

	log->displayNL("Services:");
	for (TServiceIds::iterator it=_ServiceIds.begin();it!=_ServiceIds.end();++it)
	{
		log->displayNL("\tSID: %3d  NAME: %s",(*it).first.get(),(*it).second.c_str());
	}

	log->displayNL("Pending Requests:");
	for (TPendingViewRequests::iterator it=_PendingViewRequests.begin();it!=_PendingViewRequests.end();++it)
	{
		log->displayNL("\tRID: %4d  CMD: %s",(*it).first,(*it).second.c_str());
	}
}


//-------------------------------------------------------------------------------------------------
// serviceUp and serviceDown management
//-------------------------------------------------------------------------------------------------

void CServiceCommsManagerImplementation::serviceUp(NLNET::TServiceId serviceId,const std::string& serviceName)
{
	nldebug("SS Service connection: %d: %s",serviceId.get(),serviceName.c_str());
	requestView(serviceId,ReservedRIDServiceUp,"State");
}

void CServiceCommsManagerImplementation::treatReplyToServiceUpGetView(CMessage &msgin, const string &serviceName, NLNET::TServiceId sid)
{
	vector<string> var;
	vector<string> val;
	try
	{
		msgin.serialCont(var);
		msgin.serialCont(val);
		BOMB_IF(var.size()!=val.size(),"Bad VIEW message format!",return);
	}
	catch(...)
	{
		BOMB("Bad VIEW message format!",return);
	}

	for (uint32 i=0;i<var.size();++i)
	{
		if (var[i]=="service")
		{
			// eg: val[i]== "EUS_AIS_Fyros/AIS-134"

			// get rid of the number at the end of the service name and of any other qualifiers
			// eg: fullName= "EUS_AIS_Fyros"
			CSString fullName= val[i];
			fullName= fullName.splitTo("-").splitTo("/");
			_ServiceIds[sid]= fullName;

			// eg: shortName= "AIS"
			CSString shortName= val[i];
			shortName= shortName.splitFrom("/").splitTo("-");
			if (shortName==fullName)
				shortName.clear();

			// eg: midName= "AIS_Fyros"
			CSString midName= fullName.splitFrom("_");
			if (shortName.empty() || shortName==midName || fullName==midName || midName.left(shortName.size())!=shortName)
				midName.clear();

			// have the active states execute any serviceUp scripts that they need
			if (!fullName.empty())  CStateManager::getInstance()->serviceUp(fullName,sid);
			if (!midName.empty())   CStateManager::getInstance()->serviceUp(midName,sid);
			if (!shortName.empty()) CStateManager::getInstance()->serviceUp(shortName,sid);

			nldebug("SS Adding service: %s[%d] (SHORT: %s  MID: %s)",fullName.c_str(),sid.get(),shortName.c_str(),midName.c_str());
			return;
		}
	}
	nlwarning("Failed to find 'service' variable");
	for (uint32 i=0;i<var.size();++i)
		nldebug("SS - VAR %3d: %s = %s",i,var[i].c_str(),val[i].c_str());
}

void CServiceCommsManagerImplementation::serviceDown(NLNET::TServiceId serviceId,const std::string& serviceName)
{
	// locate the service in the 'serviceIds' map
	for (TServiceIds::iterator it=getInstance()->_ServiceIds.begin();it!=getInstance()->_ServiceIds.end();++it)
	{
		if ((*it).first==serviceId)
		{
			// have the active states execute any serviceDown scripts that they need
			CStateManager::getInstance()->serviceDown((*it).second,(*it).first);

			nldebug("SS Removing service: %s[%d]",(*it).second.c_str(),serviceId.get());
			getInstance()->_ServiceIds.erase(it);
			return;
		}
	}
	nlwarning("Service down received for service that is not in the services map!: %d: %s",serviceId.get(),serviceName.c_str());
}


//-----------------------------------------------------------------------------
// methods CServiceCommsManagerImplementation / IModule
//-----------------------------------------------------------------------------

bool CServiceCommsManagerImplementation::initialiseModule(const NLMISC::CSString& rawArgs)
{
	// part of the initialisation must only be performed the very first time that an SS module is instantiated
	static bool firstTime= true;
	if (firstTime)
	{
		firstTime= false;
		TUnifiedCallbackItem cbArray[] = 
		{
			{	"VIEW",	cbView,	},
		};
		CUnifiedNetwork::getInstance()->addCallbackArray( cbArray, sizeof(cbArray)/sizeof(cbArray[0]) );
	}

	// make sure that only one module is created for the singleton
	DROP_IF(_IsActive,"There can only be one SS module activated at a time",return false);
	DROP_IF(rawArgs.countWordOrWords()!=0,"error expected no parameters but found: \""+rawArgs+"\"",return false);

	// done (success)
	_IsActive= true;
	return true;
}

void CServiceCommsManagerImplementation::release()
{
}

NLMISC::CSString CServiceCommsManagerImplementation::getState() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	CSString result;
	result+= getName();
	result+= ' ';
	result+= getParameters();
	result+= ':';

	return result;
}

NLMISC::CSString CServiceCommsManagerImplementation::getName() const
{
	// the IModule interface is only valid if a CE module has been instantiated
	nlassert(_IsActive);

	return "SS";
}

NLMISC::CSString CServiceCommsManagerImplementation::getParameters() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	return "";
}

void CServiceCommsManagerImplementation::displayModule() const
{
	// the IModule interface is only valid if an SS module has been instantiated
	nlassert(_IsActive);

	// display info
	InfoLog->displayNL("MODULE %s %s",getName().c_str(),getParameters().c_str());
}


//-----------------------------------------------------------------------------
// Register the module
//-----------------------------------------------------------------------------

REGISTER_GUS_SINGLETON_MODULE(CServiceCommsManagerImplementation,"SS","","Shard script")


//-------------------------------------------------------------------------------------------------
// CServiceCommsManager Singleton Management
//-------------------------------------------------------------------------------------------------

CServiceCommsManager* CServiceCommsManager::getInstance()
{
	return CServiceCommsManagerImplementation::getInstance();
}


//-----------------------------------------------------------------------------
