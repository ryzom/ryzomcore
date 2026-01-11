// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef NL_CVAR_LOG_FILTER_H
#define NL_CVAR_LOG_FILTER_H

#include "nel/misc/config_file.h"
#include "nel/net/service.h"

/** Declare an info logging function that works as nlinfo but that is activated with the given service config file variable
  * Example of use :
  * DECLARE_CVAR_INFO_LOG_FUNCTION(my_info, MyInfoEnabled)
  *
  * my_info("my_message"); // no-op if "MyInfoEnabled = 0;" Is found in the service config file
  */
#ifdef NL_NO_DEBUG
	#define NL_DECLARE_CVAR_INFO_LOG_FUNCTION(func, cvar, defaultValue)	inline void func(const char *format, ...) {}
#else
	#define NL_DECLARE_CVAR_INFO_LOG_FUNCTION(func, cvar, defaultValue)                                              \
	inline void func(const char *format, ...)                                                                        \
	{                                                                                                                \
		bool logWanted = (defaultValue);                                                                             \
		NLMISC::CConfigFile::CVar *logWantedPtr = NLNET::IService::getInstance()->ConfigFile.getVarPtr(#cvar);       \
		if (logWantedPtr)                                                                                            \
		{                                                                                                            \
			logWanted = logWantedPtr->asInt() != 0;                                                                  \
		}                                                                                                            \
		if (logWanted)                                                                                               \
		{                                                                                                            \
			char *out;																								 \
			NLMISC_CONVERT_VARGS(out, format, 256);                                                                  \
			nlinfo(out);                                                                                             \
		}                                                                                                            \
	}
#endif


#endif
