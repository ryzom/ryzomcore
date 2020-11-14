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

#ifndef LOGGER_SERVICE_CLIENT_H
#define LOGGER_SERVICE_CLIENT_H

#include "nel/misc/singleton.h"
#include "logger_service_itf.h"


namespace LGS
{

	class ILoggerServiceClient : public NLMISC::CManualSingleton<ILoggerServiceClient>
	{
	public:

		/// Add a set of log description 
		static void addLogDefinitions(const std::vector<TLogDefinition> &logDefs);

		/// Activate comm with the logger service asap
		static void startLoggerComm();

		/// Send a log to the logger service (actual send granularity is handled by CLoggerServiceClient)
		virtual void sendLog(const TLogInfo &logInfo) =0;

		/// Push a new log context (any following logs will be stored inside this context)
		virtual void pushLogContext(const std::string &contextName) =0;
		/// Pop a log context
		virtual void popLogContext(const std::string &contextName) =0;

	};

} // namespace LGS

#endif //LOGGER_SERVICE_CLIENT_H
