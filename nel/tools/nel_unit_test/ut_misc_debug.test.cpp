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

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#include <nel/misc/debug.h>
#include <nel/misc/displayer.h>
#include <nel/misc/dynloadlib.h>
#include <nel/misc/log.h>

#if defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
#include <fstream>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#endif

using std::string;

class CMiscUnitTestNelLibrary : public NLMISC::INelLibrary
{
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CMiscUnitTestNelLibrary);

// Test suite for CInstanceCounter
class CFoo1
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo1);
};
NL_INSTANCE_COUNTER_IMPL(CFoo1);

class CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo2);
};

NL_INSTANCE_COUNTER_IMPL(CFoo2);

class CFoo3 : public CFoo2
{
public:
	NL_INSTANCE_COUNTER_DECL(CFoo3);
};

NL_INSTANCE_COUNTER_IMPL(CFoo3);

class CUTMiscDebugTest : public testing::Test
{
};

TEST_F(CUTMiscDebugTest, testInstanceCounter)
{
	sint32 n;
	{
		CFoo1 foo1;

		sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 1);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 1);

		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 1);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 0);
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == -1);

	{
		CFoo1 foo1;
		CFoo1 other(foo1);

		sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 2);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 1);

		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 2);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 0);
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == -2);

	{
		CFoo1 foo1;
		CFoo1 other;

		foo1 = other;

		sint32 n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 2);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 0);

		NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

		n = NL_GET_INSTANCE_COUNTER(CFoo1);
		ASSERT_TRUE(n == 2);
		n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
		ASSERT_TRUE(n == 0);
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == -2);

	CFoo1 *foo1s[10];
	CFoo2 *foo2s[10];
	CFoo3 *foo3s[10];
	for (uint i = 0; i < 10; ++i)
	{
		foo1s[i] = new CFoo1;
		foo2s[i] = new CFoo2;
		foo3s[i] = new CFoo3;
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 10);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == 8);
	n = NL_GET_INSTANCE_COUNTER(CFoo2);
	ASSERT_TRUE(n == 20);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
	ASSERT_TRUE(n == 20);
	n = NL_GET_INSTANCE_COUNTER(CFoo3);
	ASSERT_TRUE(n == 10);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
	ASSERT_TRUE(n == 10);

	NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 10);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER(CFoo2);
	ASSERT_TRUE(n == 20);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER(CFoo3);
	ASSERT_TRUE(n == 10);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
	ASSERT_TRUE(n == 0);

	for (uint i = 0; i < 5; ++i)
	{
		delete foo1s[i];
		delete foo2s[i];
		delete foo3s[i];
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 5);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == -5);
	n = NL_GET_INSTANCE_COUNTER(CFoo2);
	ASSERT_TRUE(n == 10);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
	ASSERT_TRUE(n == -10);
	n = NL_GET_INSTANCE_COUNTER(CFoo3);
	ASSERT_TRUE(n == 5);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
	ASSERT_TRUE(n == -5);
	for (uint i = 5; i < 10; ++i)
	{
		delete foo1s[i];
		delete foo2s[i];
		delete foo3s[i];
	}
	n = NL_GET_INSTANCE_COUNTER(CFoo1);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo1);
	ASSERT_TRUE(n == -10);
	n = NL_GET_INSTANCE_COUNTER(CFoo2);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo2);
	ASSERT_TRUE(n == -20);
	n = NL_GET_INSTANCE_COUNTER(CFoo3);
	ASSERT_TRUE(n == 0);
	n = NL_GET_INSTANCE_COUNTER_DELTA(CFoo3);
	ASSERT_TRUE(n == -10);
}

TEST_F(CUTMiscDebugTest, testInstanceCounterOutput)
{
	NLMISC::CInstanceCounterManager::getInstance().resetDeltaCounter();

	CFoo1 *foo1s[10];
	CFoo2 *foo2s[10];
	CFoo3 *foo3s[10];
	for (uint i = 0; i < 10; ++i)
	{
		foo1s[i] = new CFoo1;
		foo2s[i] = new CFoo2;
		foo3s[i] = new CFoo3;
	}

	string ref = "Listing 3 Instance counters :\n"
	             "  Class 'CFoo1               ', \t        10 instances, \t        10 delta\n"
	             "  Class 'CFoo2               ', \t        20 instances, \t        20 delta\n"
	             "  Class 'CFoo3               ', \t        10 instances, \t        10 delta\n";

	string ret = NLMISC::CInstanceCounterManager::getInstance().displayCounters();

	nlinfo("%s", ref.c_str());
	nlinfo("%s", ret.c_str());
	ASSERT_TRUE(ref == ret);
}

// CSetLogPosition takes the log's mutex when the log has a displayer and must release exactly what it took,
// whatever the displayers are when the message is done
namespace
{

class CCountingDisplayer : public NLMISC::IDisplayer
{
public:
	CCountingDisplayer() : NLMISC::IDisplayer("UT_COUNTING"), m_Count(0) { }
	int count() const { return m_Count; }

protected:
	virtual void doDisplay(const NLMISC::CLog::TDisplayInfo & /* args */, const char * /* message */) NL_OVERRIDE { ++m_Count; }

private:
	std::atomic<int> m_Count;
};

int removeDisplayer(NLMISC::CLog *log, NLMISC::IDisplayer *displayer)
{
	log->removeDisplayer(displayer);
	return 1;
}

int addDisplayer(NLMISC::CLog *log, NLMISC::IDisplayer *displayer)
{
	log->addDisplayer(displayer);
	return 1;
}

// Logs one message from another thread and waits for it at most a few seconds, so that a mutex left held fails
// the test instead of hanging it. On a timeout the thread stays blocked: the log and the displayer are then
// leaked on purpose, as the thread still uses them.
bool logFromAnotherThread(NLMISC::CLog *log)
{
	std::packaged_task<void()> task([log]() { NLMISC::CSetLogPosition(log, __LINE__, __FILE__).log()->displayNL("from another thread"); });
	std::future<void> done = task.get_future();
	std::thread(std::move(task)).detach();
	return done.wait_for(std::chrono::seconds(5)) == std::future_status::ready;
}

}

TEST_F(CUTMiscDebugTest, logWithoutDisplayer)
{
	NLMISC::CLog *log = new NLMISC::CLog(NLMISC::CLog::LOG_INFO);
	CCountingDisplayer *displayer = new CCountingDisplayer();

	NLMISC::CSetLogPosition(log, __LINE__, __FILE__).log()->displayNL("nobody listens");

	log->addDisplayer(displayer);
	ASSERT_TRUE(logFromAnotherThread(log));
	ASSERT_EQ(displayer->count(), 1);
	delete log;
	delete displayer;
}

TEST_F(CUTMiscDebugTest, logLosesItsLastDisplayerDuringAMessage)
{
	NLMISC::CLog *log = new NLMISC::CLog(NLMISC::CLog::LOG_INFO);
	CCountingDisplayer *displayer = new CCountingDisplayer();
	log->addDisplayer(displayer);

	// the argument is evaluated once the position is set: the displayer goes away in the middle of the message
	NLMISC::CSetLogPosition(log, __LINE__, __FILE__).log()->displayNL("removed %d", removeDisplayer(log, displayer));
	ASSERT_TRUE(log->noDisplayer());

	log->addDisplayer(displayer);
	ASSERT_TRUE(logFromAnotherThread(log));
	ASSERT_EQ(displayer->count(), 1);
	delete log;
	delete displayer;
}

TEST_F(CUTMiscDebugTest, logGainsItsFirstDisplayerDuringAMessage)
{
	NLMISC::CLog *log = new NLMISC::CLog(NLMISC::CLog::LOG_INFO);
	CCountingDisplayer *displayer = new CCountingDisplayer();

	NLMISC::CSetLogPosition(log, __LINE__, __FILE__).log()->displayNL("added %d", addDisplayer(log, displayer));
	ASSERT_FALSE(log->noDisplayer());

	ASSERT_TRUE(logFromAnotherThread(log));
	ASSERT_EQ(displayer->count(), 2);
	delete log;
	delete displayer;
}

#if defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)

// Crash signals: a child process logs a line, ends with a signal, and the parent looks at what the handler wrote.

/// Runs crash() in a child process whose log directory is a new empty directory. Returns the signal that ended
/// the child (0 when it ended otherwise), the text of the crash reports it left (empty when it left none) and
/// their number.
static int crashInChild(void (*crash)(), std::string &report, uint *reportCount = NULL)
{
	report.clear();
	if (reportCount != NULL)
		*reportCount = 0;

	// the handlers are set when the debug system starts
	NLMISC::createDebug();

	char directory[] = "/tmp/nel_crash_test_XXXXXX";
	if (mkdtemp(directory) == NULL)
		return 0;

	pid_t child = fork();
	if (child == 0)
	{
		// A crash that is meant leaves no core file, and nothing for the core handler of the system. A limit of
		// 0 stops the file; a handler that the system pipes the core to is run whatever the limit, and not for a
		// process that is not dumpable.
		struct rlimit noCore;
		noCore.rlim_cur = 0;
		noCore.rlim_max = 0;
		setrlimit(RLIMIT_CORE, &noCore);
#ifdef __linux__
		prctl(PR_SET_DUMPABLE, 0);
#endif

		// the report goes to the log directory, and to the current directory when there is no log file
		if (chdir(directory) != 0)
			_exit(3);
		NLMISC::changeLogDirectory(directory);
		NLMISC::setCrashAlreadyReported(false);
		nlinfo("a line of the log before the crash");
		crash();
		_exit(4);
	}

	// The child has a minute, twice the time the handler gives its report. One that hangs is killed, and the
	// test fails.
	int status = 0;
	if (child == -1)
	{
		ADD_FAILURE() << "fork failed: " << strerror(errno);
	}
	else
	{
		bool ended = false;
		bool lost = false;
		for (uint centiseconds = 0; centiseconds < 60 * 100 && !ended && !lost; ++centiseconds)
		{
			pid_t result = waitpid(child, &status, WNOHANG);
			if (result == child)
			{
				ended = true;
			}
			else if (result == -1 && errno != EINTR)
			{
				ADD_FAILURE() << "waitpid failed: " << strerror(errno);
				lost = true;
			}
			else
			{
				usleep(10000);
			}
		}
		if (!ended && !lost)
		{
			ADD_FAILURE() << "the child process did not end, it is killed";
			kill(child, SIGKILL);
			waitpid(child, &status, 0);
		}
		if (!ended)
			status = 0;
	}

	// the child left its log file there, and the report if it wrote one
	std::vector<std::string> names;
	DIR *dir = opendir(directory);
	if (dir != NULL)
	{
		while (struct dirent *entry = readdir(dir))
		{
			std::string name = entry->d_name;
			if (name != "." && name != "..")
				names.push_back(name);
		}
		closedir(dir);
	}

	for (size_t i = 0; i < names.size(); ++i)
	{
		std::string path = std::string(directory) + "/" + names[i];
		if (names[i].compare(0, 11, "nel_report_") == 0)
		{
			std::ifstream file(path.c_str(), std::ios::binary);
			std::stringstream content;
			content << file.rdbuf();
			report += content.str();
			if (reportCount != NULL)
				++*reportCount;
		}
		unlink(path.c_str());
	}
	rmdir(directory);

	return WIFSIGNALED(status) ? WTERMSIG(status) : 0;
}

static volatile int s_CrashTestZero = 0;

static void crashWithASentSignal()
{
	raise(SIGSEGV);
}

static void crashWithAFault()
{
	// a write to address 0, which the compiler cannot see
	volatile int *nowhere = (volatile int *)(size_t)s_CrashTestZero;
	*nowhere = 1;
}

static void crashWithAbort()
{
	abort();
}

static void abortAfterItsReport()
{
	// what the message box displayer and nlerror do when they end the program after their report
	NLMISC::markEndOfProgramReported();
	abort();
}

static void abortAfterASurvivedReport()
{
	// a failed assert that was reported and that the program went on from; much later, an abort of a library
	NLMISC::setCrashAlreadyReported(true);
	abort();
}

// the threads that wait for the word, and the word (no barrier of pthread: macOS has none)
static volatile int s_CrashTestThreadsReady = 0;
static volatile int s_CrashTestGo = 0;

static void *crashTestThread(void *)
{
	__sync_fetch_and_add(&s_CrashTestThreadsReady, 1);
	while (!s_CrashTestGo)
	{
	}
	crashWithAFault();
	return NULL;
}

static void crashOnSeveralThreadsAtOnce()
{
	const int threadCount = 4;
	pthread_t threads[threadCount];
	for (int i = 0; i < threadCount; ++i)
		pthread_create(&threads[i], NULL, crashTestThread, NULL);
	while (s_CrashTestThreadsReady < threadCount)
		usleep(1000);
	s_CrashTestGo = 1;
	for (int i = 0; i < threadCount; ++i)
		pthread_join(threads[i], NULL);
}

static void crashWithALogFileOfItsOwn()
{
	// a program that logs to a file of its own, as the client does to client.log
	{
		std::ofstream own("own.log");
		own << "a line of the log file of the program" << std::endl;
	}
	NLMISC::setCrashReportLogFile("own.log");

	// and that leaves the directory of its log afterwards
	if (chdir("/") != 0)
		_exit(5);
	crashWithAFault();
}

static void crashWithALogFileThatIsNotThere()
{
	NLMISC::setCrashReportLogFile("not_there.log");
	crashWithAFault();
}

TEST_F(CUTMiscDebugTest, crashSignalWritesAReportAndStillEndsTheProcess)
{
	std::string report;
	ASSERT_EQ(crashInChild(crashWithASentSignal, report), SIGSEGV);
	EXPECT_NE(report.find("SIGSEGV"), std::string::npos) << report;
	EXPECT_NE(report.find("Sent by process"), std::string::npos) << report;
	EXPECT_NE(report.find("Callstack:"), std::string::npos) << report;
	EXPECT_NE(report.find("a line of the log before the crash"), std::string::npos) << report;
}

TEST_F(CUTMiscDebugTest, faultWritesAReportWithItsAddress)
{
	std::string report;
	ASSERT_EQ(crashInChild(crashWithAFault, report), SIGSEGV);
	EXPECT_NE(report.find("SIGSEGV"), std::string::npos) << report;
	EXPECT_NE(report.find("Address: 0x0,"), std::string::npos) << report;
}

TEST_F(CUTMiscDebugTest, abortWritesACrashReport)
{
	std::string report;
	ASSERT_EQ(crashInChild(crashWithAbort, report), SIGABRT);
	EXPECT_NE(report.find("SIGABRT"), std::string::npos) << report;
}

TEST_F(CUTMiscDebugTest, abortThatWasReportedWritesNoSecondReport)
{
	std::string report;
	ASSERT_EQ(crashInChild(abortAfterItsReport, report), SIGABRT);
	EXPECT_TRUE(report.empty()) << report;
}

TEST_F(CUTMiscDebugTest, abortAfterAReportTheProgramSurvivedWritesAReport)
{
	std::string report;
	ASSERT_EQ(crashInChild(abortAfterASurvivedReport, report), SIGABRT);
	EXPECT_NE(report.find("SIGABRT"), std::string::npos) << report;
}

TEST_F(CUTMiscDebugTest, crashesOnSeveralThreadsWriteOneWholeReport)
{
	std::string report;
	uint reportCount = 0;
	ASSERT_EQ(crashInChild(crashOnSeveralThreadsAtOnce, report, &reportCount), SIGSEGV);
	EXPECT_EQ(reportCount, 1u);

	// whole: the log is the last part of a report, and its closing line the last line
	EXPECT_NE(report.find("a line of the log before the crash"), std::string::npos) << report;
	const std::string lastLine = "\n-------------------------------\n";
	ASSERT_GE(report.size(), lastLine.size());
	EXPECT_EQ(report.substr(report.size() - lastLine.size()), lastLine) << report;
}

TEST_F(CUTMiscDebugTest, crashReportEndsWithTheLogFileTheProgramNamed)
{
	std::string report;
	ASSERT_EQ(crashInChild(crashWithALogFileOfItsOwn, report), SIGSEGV);
	// by its full path: the program may have left its directory by the time it crashes
	EXPECT_NE(report.find("End of the log file /"), std::string::npos) << report;
	EXPECT_NE(report.find("/own.log:"), std::string::npos) << report;
	EXPECT_NE(report.find("a line of the log file of the program"), std::string::npos) << report;
}

TEST_F(CUTMiscDebugTest, crashReportSaysWhenItsLogFileCannotBeOpened)
{
	std::string report;
	ASSERT_EQ(crashInChild(crashWithALogFileThatIsNotThere, report), SIGSEGV);
	EXPECT_NE(report.find("not_there.log cannot be opened"), std::string::npos) << report;
}

#endif // defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
