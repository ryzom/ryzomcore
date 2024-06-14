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



#ifndef RY_ADMINCOMMAND_LOG_H
#define RY_ADMINCOMMAND_LOG_H

#include "nel/misc/log.h"
#include "nel/misc/debug.h"
#include "nel/misc/variable.h"


/**
 * separate log system for AdminCommands. Logs are in admin_commands.log. use ADMINCMDLOG to log in this file
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CAdminCommandLog
{
public:

	/// init the log
	void init(const std::string & logFile);
	///release the log
	void release();
	///\return the name of the log file
	inline const std::string & getLogFile(){return _LogFile;}

	void display(const char *format, ...);

	/// the NEL log used for in the ADMINCMDsion log
	NLADMINCMDC::CLog			*Log;
private:
	/// the file displayer used to log the ADMINCMDsion information
	NLADMINCMDC::CFileDisplayer	_Fd;
	/// log file name
	std::string				_LogFile;
};

/// the ADMINCMDsion log
extern CAdminCommandLog	AdminCommandLog;

/// macro used to log AdminCommands
#define ADMINCMDLOG AdminCommandLog.display
/// macro used to log AdminCommands
#define ADMINCMDDBG if ( !VerboseAdminCommands ){} else ADMINCMDLOG

// Syntax error logged to egs_ADMINCMDsion.log
#define ADMINCMDLOGSYNTAXERROR(_PHRASE_)				ADMINCMDLOG("sline:%u SYNTAX ERROR %s : " _PHRASE_, line, script[0].c_str());
#define ADMINCMDLOGSYNTAXERROR1(_PHRASE_,_PARAM_)	ADMINCMDLOG("sline:%u SYNTAX ERROR %s : " _PHRASE_, line, script[0].c_str(), _PARAM_);
#define ADMINCMDLOGERROR(_PHRASE_)						ADMINCMDLOG("sline:%u ERROR %s : " _PHRASE_, line, script[0].c_str());
#define ADMINCMDLOGERROR1(_PHRASE_,_PARAM_)				ADMINCMDLOG("sline:%u ERROR %s : " _PHRASE_, line, script[0].c_str(), _PARAM_);
#define ADMINCMDLOGERROR2(_PHRASE_,_PARAM1_,_PARAM2_)	ADMINCMDLOG("sline:%u ERROR %s : " _PHRASE_, line, script[0].c_str(), _PARAM1_, _PARAM2_);



#endif // RY_ADMINCOMMAND_LOG_H

/* End of ADMINCMDsion_log.h */





















