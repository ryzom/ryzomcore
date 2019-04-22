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

#include "stdmisc.h"

#include <stdlib.h>
#include <sstream>

#include "nel/misc/common.h"
#include "nel/misc/ucstring.h"

#include "nel/misc/report.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/system_utils.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

#define NL_REPORT_POST_URL_ENVVAR "NL_REPORT_POST_URL"
#ifdef NL_OS_WINDOWS
#define NL_CRASH_REPORT_TOOL "crash_report.exe"
#else
#define NL_CRASH_REPORT_TOOL "crash_report"
#endif
#define NL_DEBUG_REPORT 0
// Set to 1 if you want command line report tool
#define NL_REPORT_CONSOLE 0
// Set to 1 if you want command line report tool even when the debugger is present
#define NL_REPORT_CONSOLE_DEBUGGER 1

namespace NLMISC
{

void setReportPostUrl(const char *postUrl)
{
#if NL_DEBUG_REPORT
	if (INelContext::isContextInitialised())
		nldebug("Set report post url to '%s'", postUrl);
#endif
#ifdef NL_OS_WINDOWS
	SetEnvironmentVariableA(NL_REPORT_POST_URL_ENVVAR, postUrl);
#else
	setenv(NL_REPORT_POST_URL_ENVVAR, postUrl, 1);
#endif
}

inline const char *getReportPostURL()
{
#ifdef NL_OS_WINDOWS
	static char buf[512];
	buf[0] = '\0';
	int res = GetEnvironmentVariableA(NL_REPORT_POST_URL_ENVVAR, buf, sizeof(buf));
	if (res <= 0 || res > 511) return NULL;
	if (buf[0] == '\0') return NULL;
	return buf;
#else
	char *res = getenv(NL_REPORT_POST_URL_ENVVAR);
	if (res == NULL || res[0] == '\0') return NULL;
	return res;
#endif
}

TReportResult report(const std::string &title, const std::string &subject, const std::string &body, const std::string &attachment, bool synchronous, bool sendReport, TReportResult defaultResult)
{
	std::string reportPath;
	if (!body.empty())
	{
		std::string reportFile = getLogDirectory() + NLMISC::toString("nel_report_%u.log", (uint)time(NULL));
		reportPath = CFile::findNewFile(reportFile);

		FILE *f = nlfopen(reportPath, "wb"); // write as binary so \n are preserved

		if (!f)
		{
#if NL_DEBUG_REPORT
			if (INelContext::isContextInitialised())
				nldebug("Failed to write report log to '%s'", reportPath.c_str());
#endif
			reportPath.clear();
		}
		else
		{
			size_t written = fwrite(body.c_str(), 1, body.length(), f);

			if (written != body.length())
			{
				nlwarning("Unable to write %u bytes to %s, only %u written", (uint)body.length(), reportPath.c_str(), (uint)written);
			}

			fclose(f);
		}
	}

	if (((INelContext::isContextInitialised()
		&& INelContext::getInstance().isWindowedApplication())
		|| CSystemUtils::detectWindowedApplication())
		&& CFile::isExists(NL_CRASH_REPORT_TOOL))
	{
		std::string params;

		if (!reportPath.empty())
			params += NLMISC::toString(" -log \"%s\"", reportPath.c_str());

		if (!subject.empty())
			params += NLMISC::toString(" -attachment \"%s\"", attachment.c_str());

		if (!title.empty())
			params += NLMISC::toString(" -title \"%s\"", title.c_str());

		if (!subject.empty())
			params += NLMISC::toString(" -subject \"%s\"", subject.c_str());

		const char *reportPostUrl = getReportPostURL();
		if (reportPostUrl)
			params += NLMISC::toString(" -host \"%s\"", reportPostUrl);

		if (synchronous)
			params += " -dev";

		if (sendReport)
			params += " -sendreport";

		if (synchronous)
		{
			TReportResult result = (TReportResult)NLMISC::launchProgramAndWaitForResult(NL_CRASH_REPORT_TOOL, params);

			if (result != ReportAlwaysIgnore
				&& result != ReportIgnore
				&& result != ReportAbort
				&& result != ReportBreak)
			{
#if NL_DEBUG_REPORT
				if (INelContext::isContextInitialised())
					nldebug("Return default result, invalid return code %i", (int)result);
#endif
				return defaultResult;
			}
			return result;
		}
		else
		{
			NLMISC::launchProgram(NL_CRASH_REPORT_TOOL, params,
				NL_DEBUG_REPORT ? INelContext::isContextInitialised() : false); // Only log if required, avoid infinite loop
			return defaultResult;
		}
	}
	else
	{
#if NL_DEBUG_REPORT
		if (INelContext::isContextInitialised() && !CFile::isExists(NL_CRASH_REPORT_TOOL))
			nldebug("Crash report tool '%s' does not exist", NL_CRASH_REPORT_TOOL);
#endif
#if defined(NL_OS_WINDOWS) && !FINAL_VERSION && !NL_REPORT_CONSOLE_DEBUGGER
		if (IsDebuggerPresent())
		{
			return defaultResult;
		}
		else
#endif
		if (synchronous)
		{
#if NL_REPORT_CONSOLE
			// An interactive console based report
			printf("\n");
			if (!title.empty())
				printf("%s\n", title.c_str());
			else
				printf("NeL report\n");
			printf("\n");
			if (!subject.empty())
				printf("\tsubject: '%s'\n", subject.c_str());
			if (!body.empty())
				printf("\tbody: '%s'\n", reportPath.c_str());
			if (!attachment.empty())
				printf("\tattachment: '%s'\n", attachment.c_str());
			for (;;)
			{
				printf("\n");
				printf("Always Ignore (S), Ignore (I), Abort (A), Break (B)?\n"); // S for Surpress
				printf("> ");
				int c = getchar();
				getchar();
				switch (c)
				{
				case 'S':
				case 's':
					return ReportAlwaysIgnore;
				case 'I':
				case 'i':
					return ReportIgnore;
				case 'A':
				case 'a':
					return ReportAbort;
				case 'B':
				case 'b':
					return ReportBreak;
				}
			}
#else
			return defaultResult;
#endif
		}
		else
		{
			return defaultResult;
		}
	}
}

} // NLMISC
