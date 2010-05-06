
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
