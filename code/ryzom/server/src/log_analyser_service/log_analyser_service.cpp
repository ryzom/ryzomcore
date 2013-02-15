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

#include "log_analyser_service.h"

#include <nel/misc/log.h>
#include <nel/misc/variable.h>

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;


// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
    admin_modules_forceLink();
}


CBufServer*	WebServer = NULL;

CVariable<uint>	QueryTimeout("pd", "QueryTimeout", "Timeout of queries stored into LAS, in seconds", 300, 0, true);
CVariable<uint>	LinePerPage("pd", "LinePerPage", "number of lines per page in result", 50, 0, true);



//-----------------------------------------------
//	callback table for input message 
//
//-----------------------------------------------
TUnifiedCallbackItem CbArray[] =
{
	{ "",	NULL },
};


/*
 * Constructor
 */
CLogAnalyserService::CLogAnalyserService()
{

	_Current = NULL;
	_Thread = NULL;
	_NextQueryId = 0;

}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN(CLogAnalyserService, "LAS", "log_analyser_service", 0, CbArray, "", "" );





/*
 * Initialization
 */
void	CLogAnalyserService::init()
{
	nlassert(WebServer == NULL);

	uint16 port = (uint16) IService::getInstance ()->ConfigFile.getVar ("WebPort").asInt();

	nlinfo("Initialise Web socket on port %d", port);

	WebServer = new CBufServer ();
	nlassert(WebServer != NULL);
	WebServer->init (port);
}


/*
 * Release
 */
void	CLogAnalyserService::release()
{
	nlassert(WebServer != NULL);

	delete WebServer;
	WebServer = 0;
}


const char*	AllowedCommands[] =
{
	"searchEId",
	"searchEIds",
	"searchString",
};



class CStrDisplayer : public IDisplayer
{
public:

	std::vector<std::string>*	Result;

	CStrDisplayer(std::vector<std::string>* result) : IDisplayer(), Result(result)	{}
	
protected:

	virtual void doDisplay( const CLog::TDisplayInfo& args, const char *message)
	{
		// no header
		if (Result->size() < 10000)
			Result->push_back(message);
		else if (Result->size() == 10000)
			Result->push_back("#! Search truncated to 10000 lignes");
	}

};

class CLogTask : public IRunnable
{
public:

	CLogAnalyserService::CQuery*	Query;

	virtual void	run()
	{
		NLMISC::CLog	taskLog;
		CStrDisplayer	taskDisplayer(&(Query->Result));
		taskLog.addDisplayer(&taskDisplayer);

		ICommand::execute(Query->Query, taskLog);

		Query->Timeout = NLMISC::CTime::getLocalTime();

		Query->Finished = true;
		Query = NULL;
	}

};

CLogTask	LogTask;














/*
 * Update
 */
bool	CLogAnalyserService::update()
{
	updateWebConnection();

	// erase queries that have timed out
	NLMISC::TTime	ctime = NLMISC::CTime::getLocalTime();
	std::deque<CQuery*>::iterator	it;
	for (it=_Finished.begin(); it!=_Finished.end(); )
	{
		if (ctime - (*it)->Timeout > QueryTimeout.get() * 1000)
		{
			delete (*it);
			it = _Finished.erase(it);
		}
		else
		{
			++it;
		}
	}

	// current query finished? move to finished, destroy thread
	if (_Current != NULL && _Current->Finished)
	{
		_Finished.push_front(_Current);
		_Current = NULL;

		_Thread->wait();
		delete _Thread;
		_Thread = NULL;
	}

	// no current running task? create new thread, start query
	_Mutex.enter();
	if (_Current == NULL && !_Requests.empty())
	{
		_Current = _Requests.back();
		_Requests.pop_back();

		LogTask.Query = _Current;

		_Thread = IThread::create(&LogTask);
		_Thread->start();
	}
	_Mutex.leave();

	return true;
}


// Execute query
void	CLogAnalyserService::executeQuery(uint32 queryId, const std::string& query)
{
	CQuery*	pQuery = new CQuery(queryId, query);

	_Mutex.enter();
	_Requests.push_front(pQuery);
	_Mutex.leave();
}


// Get query result
bool	CLogAnalyserService::getQueryResult(uint32 queryId, std::string& result, sint page, uint& numpage, const std::string& filter, bool fmode, uint linePerPage)
{
	uint		i;
	for (i=0; i<_Finished.size(); ++i)
	{
		if (_Finished[i]->Id == queryId)
		{
			_Finished[i]->Timeout = NLMISC::CTime::getLocalTime();

			if (filter.empty())
			{
				numpage = ((uint)_Finished[i]->Result.size()+linePerPage-1) / linePerPage;
				if (page >= (sint)numpage)
					page = numpage-1;

				uint	lmin = page*linePerPage;
				uint	lmax = std::min((uint)(lmin+linePerPage), (uint)_Finished[i]->Result.size());
				uint	l;

				result.clear();

				for (l=lmin; l<lmax; ++l)
					result += _Finished[i]->Result[l];
			}
			else
			{
				// build filters
				std::vector<string>	filters;
				const char*	p = filter.c_str();
				while (*p != '\0')
				{
					std::string	f;
					while (*p != '\0')
					{
						if (*p == '%')
						{
							++p;
							if (*p != '%')
								break;
						}

						f += *(p++);
					}
					if (!f.empty())
						filters.push_back(f);
				}

				// select matching lines
				std::vector<uint>	matchingLines;
				sint				errmarker = -1;
				sint				dbgmarker = -1;
				sint				infmarker = -1;
				uint				l;
				for (l=0; l<_Finished[i]->Result.size(); ++l)
				{
					if (!strncmp(_Finished[i]->Result[l].c_str(), "#!", 2))
						errmarker = l;
					if (!strncmp(_Finished[i]->Result[l].c_str(), "#?", 2))
						dbgmarker = l;
					if (!strncmp(_Finished[i]->Result[l].c_str(), "##", 2))
						infmarker = l;
					uint	f;
					bool	match;
					if (fmode)
					{
						match = true;
						for (f=0; match && f<filters.size(); ++f)
							if (_Finished[i]->Result[l].find(filters[f]) == std::string::npos)
								match = false;
					}
					else
					{
						match = false;
						for (f=0; !match && f<filters.size(); ++f)
							if (_Finished[i]->Result[l].find(filters[f]) != std::string::npos)
								match = true;
					}
					if (match)
					{
						if (errmarker >= 0)
							matchingLines.push_back(errmarker);
						if (dbgmarker >= 0)
							matchingLines.push_back(dbgmarker);
						if (infmarker >= 0)
							matchingLines.push_back(infmarker);
						errmarker = -1;
						dbgmarker = -1;
						infmarker = -1;
						matchingLines.push_back(l);
					}
				}

				// refresh numpage
				numpage = ((uint)matchingLines.size()+linePerPage-1) / linePerPage;
				if (page >= (sint)numpage)
					page = numpage-1;

				uint	lmin = page*linePerPage;
				uint	lmax = std::min((uint)(lmin+linePerPage), (uint)matchingLines.size());

				result.clear();

				// dump filtered lines
				for (l=lmin; l<lmax; ++l)
					result += _Finished[i]->Result[matchingLines[l]];
			}

			return true;
		}
	}

	if (_Current != NULL && _Current->Id == queryId)
	{
		result = toString("Query is being treated (%d%%)", (int)_Current->Progress);
		return false;
	}

	result = "Unknown query (perhaps timed out)";

	_Mutex.enter();
	for (i=0; i<_Requests.size(); ++i)
	{
		if (_Requests[i]->Id == queryId)
		{
			result = "Query not yet treated";
			break;
		}
	}
	_Mutex.leave();

	return false;
}

// Get Query list
void	CLogAnalyserService::getQueryList(std::vector<CQuery*>& queries)
{
	_Mutex.enter();
	sint	i;
	for (i=(sint)_Requests.size()-1; i>=0; --i)
	{
		CQuery*	q = _Requests[i];
		q->State = QueryAwaiting;
		queries.push_back(q);
	}

	if (_Current != NULL)
	{
		_Current->State = QueryBeingTreated;
		queries.push_back(_Current);
	}

	for (i=(sint)_Finished.size()-1; i>=0; --i)
	{
		CQuery*	q = _Finished[i];
		q->State = QueryTreated;
		queries.push_back(q);
	}

	_Mutex.leave();
}

// Cancel awaiting query
void	CLogAnalyserService::cancelQuery(uint32 queryId)
{
	_Mutex.enter();

	std::deque<CQuery*>::iterator	it;
	for (it=_Requests.begin(); it!=_Requests.end(); ++it)
	{
		if ((*it)->Id == queryId)
		{
			_Requests.erase(it);
			break;
		}
	}

	_Mutex.leave();
}





void	cbQuery(CMemStream &msgin, TSockId host)
{
	uint32		queryId = CLogAnalyserService::getInstance()->getNextQueryId();
	std::string	queryStr;
	msgin.serial(queryStr);

	CLogAnalyserService::getInstance()->executeQuery(queryId, queryStr);

	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	std::string	result = NLMISC::toString("1:%d:Query '%s' stacked:ver=2", queryId, queryStr.c_str());
	msgout.serial (result);
	WebServer->send (msgout, host);
}

void	cbResult(CMemStream &msgin, TSockId host)
{
	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	std::string	idStr;
	msgin.serial(idStr);

	std::string	result;
	bool		success = false;
	uint		numPage = 0;
	sint		page = 0;
	std::string	filter;
	bool		filter_exclusive = false;
	uint32		queryId = 0xffffffff;

	//nlinfo("received query '%s'", idStr.c_str());

	string::size_type		pos = idStr.find("%!");
	if (pos != std::string::npos)
	{
		// parse input
		const char*	p = idStr.c_str()+pos+2;
		while (*p != '\0')
		{
			std::string	param;
			std::string	value;
			while (*p != '\0' && *p != '=')
				param += *(p++);
			if (*p != '=')
				break;
			++p;
			while (*p != '\0' && *p != ' ')
			{
				if (*p == '\\')
					++p;
				if (*p == '\0')
					break;
				value += *(p++);
			}

			while (*p == ' ')
				++p;

			//nlinfo("param=%s value=%s", param.c_str(), value.c_str());

			if (param == "id")				NLMISC::fromString(value, queryId);
			else if (param == "page")		NLMISC::fromString(value, page);
			else if (param == "filter")		filter = value;
			else if (param == "fmode")		filter_exclusive = (value == "exclusive");
		}
	}
	else
	{
		// old compatibility mode
		vector<string>	res;
		explode(idStr, string(":"), res);

		if (res.size() >= 1)
		{
			if (res.size() >= 2)
				NLMISC::fromString(res[1], page);
			else
				page = 0;

			NLMISC::fromString(res[0], queryId);
		}
	}

	//nlinfo("display result for: id=%d page=%d filter=%s", queryId, page, filter.c_str());

	if (queryId != 0xffffffff)
		success = CLogAnalyserService::getInstance()->getQueryResult(queryId, result, page, numPage, filter, filter_exclusive, LinePerPage);
	else
		result = "queryId not specified ('"+idStr+"' query provided)";

	if (success)
		result = "1:"+toString(numPage)+":"+toString(page)+":"+result;
	else
		result = "0:"+result;

	msgout.serial (result);
	WebServer->send (msgout, host);
}

void	cbQueryList(CMemStream &msgin, TSockId host)
{
	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	std::vector<CLogAnalyserService::CQuery*>	queries;
	CLogAnalyserService::getInstance()->getQueryList(queries);

	std::string	result = "1:"+toString(queries.size())+"\n";

	uint	i;
	for (i=0; i<queries.size(); ++i)
	{
		result += toString("%d:%d:%d:%s\n", queries[i]->Id, (queries[i]->State == CLogAnalyserService::QueryAwaiting ? 0 : (queries[i]->State == CLogAnalyserService::QueryBeingTreated ? 1 : 2)), (int)(queries[i]->Progress), queries[i]->Query.c_str());
	}

	msgout.serial (result);
	WebServer->send (msgout, host);
}

void	cbCancelQuery(CMemStream &msgin, TSockId host)
{
	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	std::string	idStr;
	msgin.serial(idStr);

	uint32	queryId;
	NLMISC::fromString(idStr, queryId);

	CLogAnalyserService::getInstance()->cancelQuery(queryId);

	std::string	result = "1:Query cancelled";

	msgout.serial (result);
	WebServer->send (msgout, host);
}


typedef void	(*WebCallback)(CMemStream &msgin, TSockId host);

WebCallback WebCallbackArray[] =
{
	cbQuery,
	cbResult,
	cbQueryList,
	cbCancelQuery,
};


void	CLogAnalyserService::updateWebConnection()
{

	nlassert(WebServer != NULL);

	try
	{
		WebServer->update ();

		while (WebServer->dataAvailable ())
		{
			// create a string mem stream to easily communicate with web server
			NLMISC::CMemStream msgin (true);
			TSockId		host;
			bool		success = false;
			std::string	reason;

			try
			{
				WebServer->receive (msgin, &host);

				uint32	fake = 0;
				msgin.serial(fake);

				uint8	messageType = 0xff;
				msgin.serial (messageType);

				if(messageType<sizeof(WebCallbackArray)/sizeof(WebCallbackArray[0]))
				{
					WebCallbackArray[messageType](msgin, host);
					success = true;
				}
				else
				{
					reason = "unknown command "+toString(messageType);
				}
			}
			catch (const Exception &e)
			{
				nlwarning ("Error during receiving: '%s'", e.what ());
				reason = e.what();
			}

			if(!success)
			{
				nlwarning ("Failed to decode Web command");

				CMemStream	msgout;
				uint32		fake = 0;
				msgout.serial(fake);

				std::string	result = "0:Failed to decode command";
				if (!reason.empty())
					result += " ("+reason+")";
				msgout.serial (result);
				WebServer->send (msgout, host);
			}
		}
	}
	catch (const Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}
