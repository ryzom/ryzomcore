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


#ifndef LOGGER_SERVICE_H
#define LOGGER_SERVICE_H

#include "server_share/logger_service_itf.h"
#include "nel/misc/sstring.h"
#include "nel/misc/file.h"
#include "nel/net/service.h"
#include <time.h>

typedef uint32 TShardId;

/// struct to hold a log entry
struct TLogEntry
{
	// A unique ID for this logging sessions
	uint32		LogId;
	// that shard that sent the log
	TShardId	ShardId;
	// The log info
	LGS::TLogInfo	LogInfo;
};

/// A list of log entry
typedef std::list<TLogEntry>		TLogInfos;


/// A vector of log definitions
typedef std::vector < LGS::TLogDefinition >	TLogDefinitions;

class CQueryParser	*createQueryParser(const TLogDefinitions &logDefs);


inline std::string formatDate(time_t date)
{
	date &= 0x7fffffff;
	struct tm *t=localtime(&date);

	char dateStr[1024];
	strftime(dateStr, 1024, "%Y-%m-%d %H:%M:%S", t);

	return dateStr;

//
//	return NLMISC::toString("%4u-%02u-%02u %2u:%02u:%02u",
//		t->tm_year+1900,
//		t->tm_mon+1,
//		t->tm_mday,
//		t->tm_hour,
//		t->tm_min,
//		t->tm_sec);
}




#endif //LOGGER_SERVICE_H
