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

// game share
#include "game_share/utils.h"

// local
#include "gus_module_manager.h"
#include "gus_net_messages.h"
#include "gus_net_remote_module.h"

#include "cl_module_messages.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// class CSession
//-----------------------------------------------------------------------------

class CSession
{
public:
	// dtor
	~CSession();

	// log methods
	void log(const NLMISC::CSString& txt);
	void log(const NLMISC::CSString& lang,const NLMISC::CSString& txt);

private:
	// private data
	typedef std::map<CSString,FILE*> TFiles;
	TFiles _Files;
};


//-----------------------------------------------------------------------------
// class CContestLogger
//-----------------------------------------------------------------------------

class CContestLogger: public GUS::IModule
{
public:
	// IModule specialisation implementation
	bool initialiseModule(const NLMISC::CSString& rawArgs);

	void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule);
	void receiveModuleMessage(GUSNET::CModuleMessage& msg);
	
	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

public:
	// remaining public interface
	CContestLogger();

private:
	typedef std::map<uint32,CSession> TSessions;
	TSessions _Sessions;
};

//-----------------------------------------------------------------------------
// methods CSession
//-----------------------------------------------------------------------------

CSession::~CSession()
{
	for (TFiles::iterator it=_Files.begin(); it!=_Files.end();++it)
	{
		if ((*it).second!=NULL)
			fclose((*it).second);
	}
}

void CSession::log(const NLMISC::CSString& lang,const NLMISC::CSString& txt)
{
	TFiles::iterator it= _Files.find(lang);
	
	// if we need to create a new file then let's do it
	if (it==_Files.end())
	{
		CSString fileName;
		for (uint32 i=0;i<10000;++i)
		{
			fileName= NLMISC::toString("contest_%s_%04d.log",lang.c_str(),i);
			if (!NLMISC::CFile::fileExists(fileName))
				break;
			fileName.clear();
		}
		nlassert(!fileName.empty());

		nlinfo("Opening new log file: %s",fileName.c_str());
		_Files[lang]= fopen(fileName.c_str(),"wb");
		DROP_IF(_Files[lang]==NULL,"Failed to open log file for writing: "+fileName,return);
	}
	fprintf(_Files[lang],"%s\n",txt.c_str());
	fflush(_Files[lang]);
}

void CSession::log(const NLMISC::CSString& txt)
{
	for (TFiles::iterator it=_Files.begin(); it!=_Files.end();++it)
	{
		log((*it).first,txt);
	}
}


//-----------------------------------------------------------------------------
// methods CContestLogger
//-----------------------------------------------------------------------------

CContestLogger::CContestLogger()
{
}

bool CContestLogger::initialiseModule(const NLMISC::CSString& rawArgs)
{
	return true;
}

void CContestLogger::moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule)
{
	TSessions::iterator it= _Sessions.find(remoteModule->getUniqueId());
	if (it!=_Sessions.end())
		_Sessions.erase(it);
}

void CContestLogger::receiveModuleMessage(GUSNET::CModuleMessage& msg)
{
	if (msg.getMessageName()==CMsgCLText().getName())
	{
		// 'log'
		CMsgCLText logMsg(msg.getMsgBody());
		_Sessions[msg.getSenderId()].log(logMsg.getLang(),"SPEAKER "+logMsg.getSpeaker());
		_Sessions[msg.getSenderId()].log(logMsg.getLang(),"TEXT "+logMsg.getTxt());
	}
	else if (msg.getMessageName()==CMsgCLWinner().getName())
	{
		// 'winner'
		CMsgCLWinner winnerMsg(msg.getMsgBody());
		_Sessions[msg.getSenderId()].log("WINNER "+winnerMsg.getWinner());
	}
	else if (msg.getMessageName()==CMsgCLEnd().getName())
	{
		// 'end'
		TSessions::iterator it= _Sessions.find(msg.getSenderId());
		if (it!=_Sessions.end())
		{
			_Sessions.erase(it);
		}
	}
}
	
NLMISC::CSString CContestLogger::getState() const
{
	return getName()+" "+getParameters();
}

NLMISC::CSString CContestLogger::getName() const
{
	return "CL";
}

NLMISC::CSString CContestLogger::getParameters() const
{
	return "";
}

void CContestLogger::displayModule() const
{
}


//-----------------------------------------------------------------------------
// CContestLogger registration
//-----------------------------------------------------------------------------

REGISTER_GUS_MODULE(CContestLogger,"CL","","Contest logger")

