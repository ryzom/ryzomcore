// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
// Copyright (C) 2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifdef NL_OS_UNIX
#	include <errno.h>
#	include <signal.h>
#	include <string.h>
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/wait.h>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

#define NL_REPORT_POST_URL_ENVVAR "NL_REPORT_POST_URL"
#define NL_REPORT_TOOL_ENVVAR "NL_REPORT_TOOL"
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

void setReportToolEnabled(bool enabled)
{
#ifdef NL_OS_WINDOWS
	if (enabled)
		SetEnvironmentVariableA(NL_REPORT_TOOL_ENVVAR, nullptr);
	else
		SetEnvironmentVariableA(NL_REPORT_TOOL_ENVVAR, "0");
#else
	if (enabled)
		unsetenv(NL_REPORT_TOOL_ENVVAR);
	else
		setenv(NL_REPORT_TOOL_ENVVAR, "0", 1);
#endif
}

bool isReportToolEnabled()
{
#ifdef NL_OS_WINDOWS
	char buf[4];
	DWORD res = GetEnvironmentVariableA(NL_REPORT_TOOL_ENVVAR, buf, sizeof(buf));
	return !(res == 1 && buf[0] == '0');
#else
	const char *res = getenv(NL_REPORT_TOOL_ENVVAR);
	return !(res != nullptr && res[0] == '0' && res[1] == '\0');
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
	if (res == nullptr || res[0] == '\0') return nullptr;
	return res;
#endif
}

// One more argument of the tool, and its value if it has one: for the command line that the launchers of Windows
// take, and for the list of arguments that Unix gives the tool as it is.
static void addReportArgument(std::string &params, std::vector<std::string> &arguments, const char *name, const char *value = nullptr)
{
	params += " ";
	params += name;
	arguments.push_back(name);
	if (value != nullptr)
	{
		params += NLMISC::toString(" \"%s\"", value);
		arguments.push_back(value);
	}
}

#ifdef NL_OS_UNIX
// Starts the tool with these arguments and waits for its answer, which is its exit code. There is no shell in
// between: every argument reaches the tool as it is, whatever characters a path, a title or an address has.
// Returns -1 when the tool cannot be started or does not end by itself.
static sint launchReportToolAndWait(const std::string &tool, const std::vector<std::string> &arguments)
{
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(tool.c_str()));
	for (size_t i = 0; i < arguments.size(); ++i)
		argv.push_back(const_cast<char *>(arguments[i].c_str()));
	argv.push_back(nullptr);

	// A process that ignores SIGCHLD cannot wait for a child: the system takes the child away when it ends, and
	// its answer with it. launchProgram makes the process ignore that signal from its first use on. For the time
	// of this wait the signal has its default action then.
	struct sigaction previousAction;
	const bool ignored = sigaction(SIGCHLD, nullptr, &previousAction) == 0
		&& ((previousAction.sa_flags & SA_NOCLDWAIT) || (!(previousAction.sa_flags & SA_SIGINFO) && previousAction.sa_handler == SIG_IGN));
	if (ignored)
	{
		struct sigaction defaultAction;
		memset(&defaultAction, 0, sizeof(defaultAction));
		defaultAction.sa_handler = SIG_DFL;
		sigemptyset(&defaultAction.sa_mask);
		sigaction(SIGCHLD, &defaultAction, nullptr);
	}

	sint result = -1;
	pid_t child = fork();
	if (child == 0)
	{
		execv(tool.c_str(), &argv[0]);
		_exit(127);
	}
	else if (child > 0)
	{
		int status = 0;
		pid_t ended;
		do
		{
			ended = waitpid(child, &status, 0);
		} while (ended == -1 && errno == EINTR);

		if (ended == child && WIFEXITED(status))
			result = WEXITSTATUS(status);
	}

	if (ignored)
	{
		// the children that ended meanwhile, which the system would have taken away, and the signal is ignored
		// again
		while (waitpid(-1, nullptr, WNOHANG) > 0)
		{
		}
		sigaction(SIGCHLD, &previousAction, nullptr);
	}

	return result;
}
#endif

TReportResult report(const std::string &title, const std::string &subject, const std::string &body, bool synchronous, bool sendReport, TReportResult defaultResult)
{
	std::string reportPath;
	if (!body.empty())
	{
		std::string reportFile = getLogDirectory() + NLMISC::toString("nel_report_%u.log", (uint)time(nullptr));
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

	// The tool. On Windows it is started from the current directory. On Unix it is the one beside the running
	// program, else the one in PATH, by its full path: a bare name is only looked for in PATH, and a file of
	// that name in the current directory can be anything.
#ifdef NL_OS_UNIX
	const std::string reportTool = getCrashReportToolPath();
#else
	const std::string reportTool = CFile::isExists(NL_CRASH_REPORT_TOOL) ? NL_CRASH_REPORT_TOOL : "";
#endif

	if (((INelContext::isContextInitialised()
		&& INelContext::getInstance().isWindowedApplication())
		|| CSystemUtils::detectWindowedApplication())
		&& !reportTool.empty() && isReportToolEnabled())
	{
		std::string params;
		std::vector<std::string> arguments;

		if (!reportPath.empty())
			addReportArgument(params, arguments, "-log", reportPath.c_str());

		if (!title.empty())
			addReportArgument(params, arguments, "-title", title.c_str());

		if (!subject.empty())
			addReportArgument(params, arguments, "-subject", subject.c_str());

		const char *reportPostUrl = getReportPostURL();
		if (reportPostUrl)
			addReportArgument(params, arguments, "-host", reportPostUrl);

		if (synchronous)
			addReportArgument(params, arguments, "-dev");

		if (sendReport)
			addReportArgument(params, arguments, "-sendreport");

		if (synchronous)
		{
#ifdef NL_OS_UNIX
			TReportResult result = (TReportResult)launchReportToolAndWait(reportTool, arguments);
#else
			TReportResult result = (TReportResult)NLMISC::launchProgramAndWaitForResult(reportTool, params);
#endif

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
			// Only log if required, avoid infinite loop
			const bool log = NL_DEBUG_REPORT ? INelContext::isContextInitialised() : false;
#ifdef NL_OS_UNIX
			// the list of arguments there: taken apart again, a command line loses an argument at a quote in it
			NLMISC::launchProgramArray(reportTool, arguments, log);
#else
			NLMISC::launchProgram(reportTool, params, log);
#endif
			return defaultResult;
		}
	}
	else
	{
#if NL_DEBUG_REPORT
		if (INelContext::isContextInitialised() && reportTool.empty())
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
