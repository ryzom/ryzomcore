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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#	include <io.h>
#	include <fcntl.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#else
#	include <cerrno>
#endif // NL_OS_WINDOWS

#include "nel/misc/path.h"
#include "nel/misc/mutex.h"
#include "nel/misc/report.h"
#include "nel/misc/system_utils.h"
#include "nel/misc/variable.h"

#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	define NOMINMAX
#	include <windows.h>
#else
#	define IsDebuggerPresent() false
#endif

#include "nel/misc/displayer.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

CVariable<bool> StdDisplayerColor("nel", "StdDisplayerColor", "Enable colors in std displayer", true, 0, true);

static const char *LogTypeToString[][8] = {
	{ "", "ERR", "WRN", "INF", "DBG", "STT", "AST", "UKN" },
	{ "", "Error", "Warning", "Information", "Debug", "Statistic", "Assert", "Unknown" },
	{ "", "A fatal error occurs. The program must quit", "", "", "", "", "A failed assertion occurs", "" },
};

const char *IDisplayer::logTypeToString (CLog::TLogType logType, bool longFormat)
{
	if (logType < CLog::LOG_NO || logType > CLog::LOG_UNKNOWN)
		return "<NotDefined>";

	return LogTypeToString[longFormat?1:0][logType];
}

const char *IDisplayer::dateToHumanString ()
{
	time_t date;
	time (&date);
	return dateToHumanString (date);
}

const char *IDisplayer::dateToHumanString (time_t date)
{
	static char cstime[25];
	struct tm *tms = localtime(&date);
	if (tms)
		strftime (cstime, 25, "%Y/%m/%d %H:%M:%S", tms);
	else
		sprintf(cstime, "bad date %d", (uint32)date);
	return cstime;
}

const char *IDisplayer::dateToComputerString (time_t date)
{
	static char cstime[25];
	smprintf (cstime, 25, "%ld", &date);
	return cstime;
}

const char *IDisplayer::HeaderString ()
{
	static char header[1024];
	smprintf(header, 1024, "\nLog Starting [%s]\n", dateToHumanString());
	return header;
}


IDisplayer::IDisplayer(const char *displayerName)
{
	_Mutex = new CMutex (string(displayerName)+"DISP");
	DisplayerName = displayerName;
}

IDisplayer::~IDisplayer()
{
	delete _Mutex;
}

/*
 * Display the string where it does.
 */
void IDisplayer::display ( const CLog::TDisplayInfo& args, const char *message )
{
	_Mutex->enter();
	try
	{
		doDisplay( args, message );
	}
	catch (const Exception &)
	{
		// silence
	}
	_Mutex->leave();
}


// Log format : "<LogType> <ThreadNo> <FileName> <Line> <ProcessName> : <Msg>"
void CStdDisplayer::doDisplay ( const CLog::TDisplayInfo& args, const char *message )
{
	bool needSpace = false;
	//stringstream ss;
	string str;
#ifdef NL_OS_UNIX
	bool colorSet = false;
#endif

	if (args.LogType != CLog::LOG_NO)
	{
#ifdef NL_OS_UNIX
		if (StdDisplayerColor.get())
		{
			if (args.LogType == CLog::LOG_ERROR || args.LogType == CLog::LOG_ASSERT) { str += "\e[0;30m\e[41m"; colorSet = true; } // black text, red background
			else if (args.LogType == CLog::LOG_WARNING) { str += "\e[0;91m"; colorSet = true; } // bright red text
			else if (args.LogType == CLog::LOG_DEBUG) { str += "\e[0;34m"; colorSet = true; } // blue text
		}
#endif
		//ss << logTypeToString(args.LogType);
		str += logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		//ss << setw(5) << args.ThreadId;
		if (needSpace) { str += " "; needSpace = false; }
#ifdef NL_OS_WINDOWS
		str += NLMISC::toString("%5x", args.ThreadId);
#else
		str += NLMISC::toString("%08x", args.ThreadId);
#endif
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		//ss << CFile::getFilename(args.FileName);
		str += CFile::getFilename(args.FileName);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		//ss << args.Line;
		str += NLMISC::toString(args.Line);
		needSpace = true;
	}

	if (args.FuncName != NULL)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		//ss << args.FuncName;
		str += args.FuncName;
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		//ss << args.ProcessName;
		str += args.ProcessName;
		needSpace = true;
	}

	//if (needSpace) { ss << " : "; needSpace = false; }
	if (needSpace) { str += " : "; needSpace = false; }

	//ss << message;
	str += message;

//	string s = ss.str();

	static bool consoleMode = true;

#if defined(NL_OS_WINDOWS)
	static bool consoleModeTest = false;
	if (!consoleModeTest)
	{
		HANDLE handle = CreateFile ("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
		consoleMode = handle != INVALID_HANDLE_VALUE;
		if (consoleMode)
			CloseHandle (handle);
		consoleModeTest = true;
	}
#endif // NL_OS_WINDOWS

#ifdef NL_OS_UNIX
	if (colorSet)
	{
		str += "\e[0m";
	}
#endif

	// Printf ?
	if (consoleMode)
	{
		// we don't use cout because sometimes, it crashs because cout isn't already init, printf doesn t crash.
		if (!str.empty())
			printf ("%s", str.c_str());

		if (!args.CallstackAndLog.empty())
			printf ("%s", args.CallstackAndLog.c_str());

		fflush(stdout);
	}

#ifdef NL_OS_WINDOWS
	// display the string in the debugger is the application is started with the debugger
	if (IsDebuggerPresent ())
	{
		//stringstream ss2;
		string str2;
		needSpace = false;

		if (args.FileName != NULL) str2 += args.FileName;

		if (args.Line != -1)
		{
			str2 += "(" + NLMISC::toString(args.Line) + ")";
			needSpace = true;
		}

		if (needSpace) { str2 += " : "; needSpace = false; }

		if (args.FuncName != NULL) str2 += string(args.FuncName) + " ";

		if (args.LogType != CLog::LOG_NO)
		{
			str2 += logTypeToString(args.LogType);
			needSpace = true;
		}

		// Write thread identifier
		if ( args.ThreadId != 0 )
		{
			str2 += NLMISC::toString("%5x: ", args.ThreadId);
		}

		str2 += message;

		const sint maxOutString = 2*1024;

		if(str2.size() < maxOutString)
		{
			//////////////////////////////////////////////////////////////////
			// WARNING: READ THIS !!!!!!!!!!!!!!!! ///////////////////////////
			// If at the release time, it freezes here, it's a microsoft bug:
			// http://support.microsoft.com/support/kb/articles/q173/2/60.asp
			OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(str2).c_str());
		}
		else
		{
			/*OutputDebugString(ss2.str().c_str());
			OutputDebugString("\n\t\t\t");
			OutputDebugString("message end: ");
			OutputDebugString(&message[strlen(message) - 1024]);
			OutputDebugString("\n");*/

			sint count = 0;
			uint n = (uint)strlen(message);
			std::string s(&str2.c_str()[0], (str2.size() - n));
			OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(s).c_str());

			for(;;)
			{

				if((n - count) < maxOutString )
				{
					s = std::string(&message[count], (n - count));
					OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(s).c_str());
					OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8("\n").c_str());
					break;
				}
				else
				{
					s = std::string(&message[count] , count + maxOutString);
					OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(s).c_str());
					OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8("\n\t\t\t").c_str());
					count += maxOutString;
				}
			}
		}

		// OutputDebugString is a big shit, we can't display big string in one time, we need to split
		uint32 pos = 0;
		string splited;
		for(;;)
		{
			if (pos+1000 < args.CallstackAndLog.size ())
			{
				splited = args.CallstackAndLog.substr (pos, 1000);
				OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(splited).c_str());
				pos += 1000;
			}
			else
			{
				splited = args.CallstackAndLog.substr (pos);
				OutputDebugStringW((LPCWSTR)ucstring::makeFromUtf8(splited).c_str());
				break;
			}
		}
	}
#endif
}

CFileDisplayer::CFileDisplayer (const std::string &filename, bool eraseLastLog, const char *displayerName, bool raw) :
	IDisplayer (displayerName), _NeedHeader(true), _LastLogSizeChecked(0), _Raw(raw)
{
	_FilePointer = (FILE*)1;
	setParam (filename, eraseLastLog);
}

CFileDisplayer::CFileDisplayer () :
	IDisplayer (""), _NeedHeader(true), _LastLogSizeChecked(0), _Raw(false)
{
	_FilePointer = (FILE*)1;
}

CFileDisplayer::~CFileDisplayer ()
{
	if (_FilePointer > (FILE*)1)
	{
		fclose(_FilePointer);
		_FilePointer = NULL;
	}
}

void CFileDisplayer::setParam (const std::string &filename, bool eraseLastLog)
{
	_FileName = filename;

	if (filename.empty())
	{
		// can't do nlwarning or infinite recurs
		printf ("CFileDisplayer::setParam(): Can't create file with empty filename\n");
		return;
	}

	if (eraseLastLog)
	{
/*		ofstream ofs (filename.c_str(), ios::out | ios::trunc);
		if (!ofs.is_open())
		{
			// can't do nlwarning or infinite recurs
			printf ("CFileDisplayer::setParam(): Can't open and clear the log file '%s'\n", filename.c_str());
		}*/

		// Erase all the derived log files
		int i = 0;
		bool fileExist = true;
		while (fileExist)
		{
			string fileToDelete = CFile::getPath (filename) + CFile::getFilenameWithoutExtension (filename) +
				toString ("%03d.", i) + CFile::getExtension (filename);
			fileExist = CFile::isExists (fileToDelete);
			if (fileExist)
				CFile::deleteFile (fileToDelete);
			i++;
		}
	}

	if (_FilePointer > (FILE*)1)
	{
		fclose (_FilePointer);
		_FilePointer = (FILE*)1;
	}
}

// Log format: "2000/01/15 12:05:30 <ProcessName> <LogType> <ThreadId> <FileName> <Line> : <Msg>"
void CFileDisplayer::doDisplay ( const CLog::TDisplayInfo& args, const char *message )
{
	bool needSpace = false;
	//stringstream ss;
	string str;

	// if the filename is not set, don't log
	if (_FileName.empty()) return;

	if (args.Date != 0 && !_Raw)
	{
		str += dateToHumanString(args.Date);
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
#ifdef NL_OS_WINDOWS
		str += NLMISC::toString("%4x", args.ThreadId);
#else
		str += NLMISC::toString("%4u", args.ThreadId);
#endif
		needSpace = true;
	}

	if (!args.ProcessName.empty() && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += args.ProcessName;
		needSpace = true;
	}

	if (args.FileName != NULL && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += CFile::getFilename(args.FileName);
		needSpace = true;
	}

	if (args.Line != -1 && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString(args.Line);
		needSpace = true;
	}

	if (args.FuncName != NULL && !_Raw)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += args.FuncName;
		needSpace = true;
	}

	if (needSpace) { str += " : "; needSpace = false; }

	str += message;

	if (_FilePointer > (FILE*)1)
	{
		// if the file is too big (>5mb), rename it and create another one (check only after 20 lines to speed up)
		if (_LastLogSizeChecked++ > 20)
		{
		  int res = ftell (_FilePointer);
		  if (res > 5*1024*1024)
		    {
			fclose (_FilePointer);
			rename (_FileName.c_str(), CFile::findNewFile (_FileName).c_str());
			_FilePointer = (FILE*) 1;
			_LastLogSizeChecked = 0;
		    }
		}
	}

	if (_FilePointer == (FILE*)1)
	{
		_FilePointer = fopen (_FileName.c_str(), "at");
		if (_FilePointer == NULL)
			printf ("Can't open log file '%s': %s\n", _FileName.c_str(), strerror (errno));
	}

	if (_FilePointer != 0)
	{
		if (_NeedHeader)
		{
			const char *hs = HeaderString();
			fwrite (hs, strlen (hs), 1, _FilePointer);
			_NeedHeader = false;
		}

		if(!str.empty())
			fwrite (str.c_str(), str.size(), 1, _FilePointer);

		if(!args.CallstackAndLog.empty())
			fwrite (args.CallstackAndLog.c_str(), args.CallstackAndLog.size (), 1, _FilePointer);

		fflush (_FilePointer);
	}
}

// Log format in clipboard: "2000/01/15 12:05:30 <LogType> <ProcessName> <FileName> <Line>: <Msg>"
// Log format on the screen: in debug   "<ProcessName> <FileName> <Line>: <Msg>"
//                           in release "<Msg>"
void CMsgBoxDisplayer::doDisplay ( const CLog::TDisplayInfo& args, const char *message)
{
#ifdef NL_OS_WINDOWS

	bool needSpace = false;
//	stringstream ss;
	string str;

	// create the string for the clipboard

	if (args.Date != 0)
	{
		str += dateToHumanString(args.Date);
		needSpace = true;
	}

	if (args.LogType != CLog::LOG_NO)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		str += logTypeToString(args.LogType);
		needSpace = true;
	}

	if (!args.ProcessName.empty())
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		str += args.ProcessName;
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		str += CFile::getFilename(args.FileName);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString(args.Line);
		needSpace = true;
	}

	if (args.FuncName != NULL)
	{
		//if (needSpace) { ss << " "; needSpace = false; }
		if (needSpace) { str += " "; needSpace = false; }
		str += args.FuncName;
		needSpace = true;
	}

	if (needSpace) { str += ": "; needSpace = false; }

	str += message;

	CSystemUtils::copyTextToClipboard(str);

	// create the string on the screen
	needSpace = false;
//	stringstream ss2;
	string str2;

#ifdef NL_DEBUG
	if (!args.ProcessName.empty())
	{
		if (needSpace) { str2 += " "; needSpace = false; }
		str2 += args.ProcessName;
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		if (needSpace) { str2 += " "; needSpace = false; }
		str2 += CFile::getFilename(args.FileName);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { str2 += " "; needSpace = false; }
		str2 += NLMISC::toString(args.Line);
		needSpace = true;
	}

	if (args.FuncName != NULL)
	{
		if (needSpace) { str2 += " "; needSpace = false; }
		str2 += args.FuncName;
		needSpace = true;
	}

	if (needSpace) { str2 += ": "; needSpace = false; }

#endif // NL_DEBUG

	str2 += message;
	str2 += "\n\n(this message was copied in the clipboard)";

/*	if (IsDebuggerPresent ())
	{
		// Must break in assert call
		DebugNeedAssert = true;
	}
	else
*/	{

		// Display the report

		string body;

		body += toString(LogTypeToString[2][args.LogType]) + "\n";
		body += "ProcName: " + args.ProcessName + "\n";
		body += "Date: " + string(dateToHumanString(args.Date)) + "\n";
		if(args.FileName == NULL)
			body += "File: <Unknown>\n";
		else
			body += "File: " + string(args.FileName) + "\n";
		body += "Line: " + toString(args.Line) + "\n";
		if (args.FuncName == NULL)
			body += "FuncName: <Unknown>\n";
		else
			body += "FuncName: " + string(args.FuncName) + "\n";
		body += "Reason: " + toString(message);

		body += args.CallstackAndLog;

		string subject;

		// procname is host/service_name-sid we only want the service_name to avoid redondant mail
		string procname;
		string::size_type pos = args.ProcessName.find ("/");
		if (pos == string::npos)
		{
			procname =  args.ProcessName;
		}
		else
		{
			string::size_type pos2 = args.ProcessName.find ("-", pos+1);
			if (pos2 == string::npos)
			{
				procname =  args.ProcessName.substr (pos+1);
			}
			else
			{
				procname =  args.ProcessName.substr (pos+1, pos2-pos-1);
			}
		}

		subject += procname + " NeL " + toString(LogTypeToString[0][args.LogType]) + " " + (args.FileName?string(args.FileName):"") + " " + toString(args.Line) + " " + (args.FuncName?string(args.FuncName):"");

		// Check the envvar NEL_IGNORE_ASSERT
		if (getenv ("NEL_IGNORE_ASSERT") == NULL)
		{
			// yoyo: allow only to send the crash report once. Because users usually click ignore,
			// which create noise into list of bugs (once a player crash, it will surely continues to do it).
			std::string filename = getLogDirectory() + NL_CRASH_DUMP_FILE;

			if  (ReportDebug == report (args.ProcessName + " NeL " + toString(logTypeToString(args.LogType, true)), "", subject, body, true, 2, true, 1, !isCrashAlreadyReported(), IgnoreNextTime, filename.c_str()))
			{
				INelContext::getInstance().setDebugNeedAssert(true);
			}

			// no more sent mail for crash
			setCrashAlreadyReported(true);
		}

/*		// Check the envvar NEL_IGNORE_ASSERT
		if (getenv ("NEL_IGNORE_ASSERT") == NULL)
		{
			// Ask the user to continue, debug or ignore
			int result = MessageBox (NULL, ss2.str().c_str (), logTypeToString(args.LogType, true), MB_ABORTRETRYIGNORE | MB_ICONSTOP);
			if (result == IDABORT)
			{
				// Exit the program now
				exit (EXIT_FAILURE);
			}
			else if (result == IDRETRY)
			{
				// Give the debugger a try
				DebugNeedAssert = true;
 			}
			else if (result == IDIGNORE)
			{
				// Continue, do nothing
			}
		}
*/	}

#endif
}



/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
void CStdDisplayer::display (const std::string& str)
{
//	printf("%s", str.c_str ());
	cout << str;

#ifdef NL_OS_WINDOWS
	// display the string in the debugger is the application is started with the debugger
	if (IsDebuggerPresent ())
		OutputDebugString(str.c_str ());
#endif
}


void CFileDisplayer::display (const std::string& str)
{
	if (_FileName.size () == 0) return;

	ofstream ofs (_FileName.c_str (), ios::out | ios::app);
	if (ofs.is_open ())
	{
		ofs << str;
		ofs.close();
	}


//	FILE *fp = fopen (_FileName.c_str (), "a");
//	if (fp == NULL) return;

//	fprintf (fp, "%s", str.c_str ());

//	fclose (fp);
}



void CMsgBoxDisplayer::display (const std::string& str)
{
#ifdef NL_OS_WINDOWS

	CSystemUtils::copyTextToClipboard(str);

	string strf = str;
	strf += "\n\n(this message was copied in the clipboard)";
	MessageBox (NULL, strf.c_str (), "", MB_OK | MB_ICONEXCLAMATION);
#endif
}
**************************************************************************/


} // NLMISC
