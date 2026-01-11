// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2016  Winch Gate Property Limited
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



#ifndef RY_ADMIN_LOG_H
#define RY_ADMIN_LOG_H

#include "nel/misc/log.h"
#include "nel/misc/debug.h"
#include "nel/misc/variable.h"


/**
 * separate log system for admin commands. Logs are in egs_mission.log. use ADMINLOG to log in this file
 * \author Ulukyn
 * \date 2016
 */
class CAdminLog
{
public:
	/// init the log
	void init(const std::string & logFile);
	///release the log
	void release();
	///\return the name of the log file
	inline const std::string & getLogFile(){return _LogFile;}
	
	void display(const char *format, ...);
	
	/// the NEL log used for in the mission log
	NLMISC::CLog			*Log;
private:
	/// the file displayer used to log the mission information
	NLMISC::CFileDisplayer	_Fd;
	/// log file name
	std::string				_LogFile;
};

/// the mission log
extern CAdminLog	AdminLog;

/// macro used to log missions script error 
#define ADMINLOG AdminLog.display

#endif // RY_ADMIN_LOG_H

/* End of admin_log.h */





















