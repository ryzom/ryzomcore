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

#ifndef NL_LOG_ANALYSER_SERVICE_H
#define NL_LOG_ANALYSER_SERVICE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/thread.h>
#include <nel/misc/mutex.h>
#include <nel/misc/time_nl.h>
#include <nel/net/service.h>


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CLogAnalyserService : public NLNET::IService
{
public:

	/// Constructor
	CLogAnalyserService();


	/// Initialization
	virtual void	init();

	/// Release
	virtual void	release();

	/// Update
	virtual bool	update();


public:

	enum TQueryState
	{
		QueryAwaiting,
		QueryBeingTreated,
		QueryTreated
	};

	class CQuery
	{
	public:

		CQuery(uint32 id, const std::string& query) : Id(id), Progress(0.0f), Query(query), State(QueryAwaiting), Finished(false)	{}

		uint32						Id;
		volatile float				Progress;
		TQueryState					State;
		std::string					Query;
		std::vector<std::string>	Result;
		NLMISC::TTime				Timeout;

		volatile bool				Finished;

	};

	/// Get Service Instance
	static CLogAnalyserService*	getInstance()
	{
		return (CLogAnalyserService*)IService::getInstance();
	}

	/// Get Next Query Id
	uint32			getNextQueryId()
	{
		return _NextQueryId++;
	}

	/// Execute query
	void			executeQuery(uint32 queryId, const std::string& query);

	/// Get query result
	bool			getQueryResult(uint32 queryId, std::string& result, sint page, uint& numpage, const std::string& filter, bool fmode, uint linePerPage);

	/// Get Query list
	void			getQueryList(std::vector<CQuery*>& queries);

	/// Cancel awaiting query
	void			cancelQuery(uint32 queryId);

	/// Get current query
	CQuery*			getCurrentQuery()				{ return _Current; }

private:

	NLMISC::CMutex			_Mutex;

	NLMISC::IThread*		_Thread;

	CQuery*					_Current;

	std::deque<CQuery*>		_Requests;
	std::deque<CQuery*>		_Finished;

	uint32					_NextQueryId;

	/// Update Web connection
	void	updateWebConnection();
};


#endif // NL_LOG_ANALYSER_SERVICE_H

/* End of log_analyser_service.h */
