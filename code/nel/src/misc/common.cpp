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
#include "nel/misc/common.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <io.h>
#	include <tchar.h>
#elif defined NL_OS_UNIX
#	include <unistd.h>
#	include <cerrno>
#	include <pthread.h>
#	include <sched.h>
#endif

#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"

using namespace std;

#ifndef NL_COMP_MINGW
#ifdef NL_OS_WINDOWS
#	pragma message( " " )

#	if FINAL_VERSION
#		pragma message( "************************" )
#		pragma message( "**** FINAL_VERSION *****" )
#		pragma message( "************************" )
#	else
#		pragma message( "Not using FINAL_VERSION")
#	endif // FINAL_VERSION

#	ifdef ASSERT_THROW_EXCEPTION
#		pragma message( "nlassert throws exceptions" )
#	else
#		pragma message( "nlassert does not throw exceptions" )
#	endif // ASSERT_THROW_EXCEPTION

#	ifdef _STLPORT_VERSION
#		pragma message( "Using STLport" )
#	else
#		pragma message( "Using standard STL" )
#	endif // _STLPORT_VERSION

#	pragma message( " " )

#	if (_MSC_VER >= 1200) && (_MSC_VER < 1400) && (WINVER < 0x0500)
//Using VC7 and later lib, need this to compile on VC6
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#	endif


#endif // NL_OS_WINDOWS
#endif // !NL_COMP_MINGW


#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{

/*
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms )
{
#ifdef NL_OS_WINDOWS

#ifdef NL_DEBUG
	// a Sleep(0) "block" the other thread in DEBUG/_CONSOLE, so we clamp
	ms = max(ms, (uint32)1);
#endif

	Sleep( ms );

#elif defined NL_OS_UNIX
	//usleep( ms*1000 ); // resolution: 20 ms!

	timespec ts;
	ts.tv_sec = ms/1000;
	ts.tv_nsec = (ms%1000)*1000000;
	int res;
	do
	{
		res = nanosleep( &ts, &ts ); // resolution: 10 ms (with common scheduling policy)
	}
	while ( (res != 0) && (errno==EINTR) );
#endif
}


/*
 * Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
 */
size_t getThreadId()
{
#ifdef NL_OS_WINDOWS
	return GetCurrentThreadId();
#elif defined NL_OS_UNIX
	return size_t(pthread_self());
	// doesnt work on linux kernel 2.6	return getpid();
#endif

}


/*
 * Returns a readable string from a vector of bytes. '\0' are replaced by ' '
 */
string stringFromVector( const vector<uint8>& v, bool limited )
{
	string s;

	if (!v.empty())
	{
		int size = (int)v.size ();
		if (limited && size > 1000)
		{
			string middle = "...<buf too big,skip middle part>...";
			s.resize (1000 + middle.size());
			memcpy (&*s.begin(), &*v.begin(), 500);
			memcpy (&*s.begin()+500, &*middle.begin(), middle.size());
			memcpy (&*s.begin()+500+middle.size(), &*v.begin()+size-500, 500);
		}
		else
		{
			s.resize (size);
			memcpy( &*s.begin(), &*v.begin(), v.size() );
		}

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			// remplace non printable char and % with '?' chat
			if ( ! isprint((uint8)(*is)) || (*is) == '%')
			{
				(*is) = '?';
			}
		}
	}
/*
	if ( ! v.empty() )
	{
		// Copy contents
		s.resize( v.size() );
		memcpy( &*s.begin(), &*v.begin(), v.size() );

		// Replace '\0' characters
		string::iterator is;
		for ( is=s.begin(); is!=s.end(); ++is )
		{
			// remplace non printable char and % with '?' chat
			if ( ! isprint((*is)) || (*is) == '%')
			{
				(*is) = '?';
			}
		}
	}
*/	return s;
}


sint smprintf( char *buffer, size_t count, const char *format, ... )
{
	sint ret;

	va_list args;
	va_start( args, format );
	ret = vsnprintf( buffer, count, format, args );
	if ( ret == -1 )
	{
		buffer[count-1] = '\0';
	}
	va_end( args );

	return( ret );
}


sint64 atoiInt64 (const char *ident, sint64 base)
{
	sint64 number = 0;
	bool neg = false;

	// NULL string
	nlassert (ident != NULL);

	// empty string
	if (*ident == '\0') goto end;

	// + sign
	if (*ident == '+') ident++;

	// - sign
	if (*ident == '-') { neg = true; ident++; }

	while (*ident != '\0')
	{
		if (isdigit((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'0';
		}
		else if (base > 10 && islower((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'a'+10;
		}
		else if (base > 10 && isupper((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'A'+10;
		}
		else
		{
			goto end;
		}
		ident++;
	}
end:
	if (neg) number = -number;
	return number;
}

void itoaInt64 (sint64 number, char *str, sint64 base)
{
	str[0] = '\0';
	char b[256];
	if(!number)
	{
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	memset(b,'\0',255);
	memset(b,'0',64);
	sint n;
	sint64 x = number;
	if (x < 0) x = -x;
	char baseTable[] = "0123456789abcdefghijklmnopqrstuvwyz";
	for(n = 0; n < 64; n ++)
	{
		sint num = (sint)(x % base);
		b[64 - n] = baseTable[num];
		if(!x)
		{
			int k;
			int j = 0;

			if (number < 0)
			{
				str[j++] = '-';
			}

			for(k = 64 - n + 1; k <= 64; k++)
			{
				str[j ++] = b[k];
			}
			str[j] = '\0';
			break;
		}
		x /= base;
	}
}

uint raiseToNextPowerOf2(uint v)
{
	uint	res=1;
	while(res<v)
		res<<=1;

	return res;
}

uint	getPowerOf2(uint v)
{
	uint	res=1;
	uint	ret=0;
	while(res<v)
	{
		ret++;
		res<<=1;
	}

	return ret;
}

bool isPowerOf2(sint32 v)
{
	while(v)
	{
		if(v&1)
		{
			v>>=1;
			if(v)
				return false;
		}
		else
			v>>=1;
	}

	return true;
}

string bytesToHumanReadable (const std::string &bytes)
{
	return bytesToHumanReadable (atoiInt64(bytes.c_str()));
}

string bytesToHumanReadable (uint64 bytes)
{
	static const char *divTable[]= { "B", "KB", "MB", "GB", "TB" };
	uint div = 0;
	uint64 res = bytes;
	uint64 newres = res;
	for(;;)
	{
		newres /= 1024;
		if(newres < 8 || div > 3)
			break;
		div++;
		res = newres;
	}
	return toString ("%"NL_I64"u%s", res, divTable[div]);
}

uint32 humanReadableToBytes (const string &str)
{
	uint32 res;

	if(str.empty())
		return 0;

	// not a number
	if(str[0]<'0' || str[0]>'9')
		return 0;

	res = atoi (str.c_str());

	if(str[str.size()-1] == 'B')
	{
		if (str.size()<3)
			return res;

		// there's no break and it's **normal**
		switch (str[str.size()-2])
		{
		case 'G': res *= 1024;
		case 'M': res *= 1024;
		case 'K': res *= 1024;
		default: ;
		}
	}

	return res;
}


NLMISC_CATEGORISED_COMMAND(nel,btohr, "Convert a bytes number into an human readable number", "<int>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if (args.size() != 1)
		return false;

	log.displayNL("%s -> %s", args[0].c_str(), bytesToHumanReadable(args[0]).c_str());

	return true;
}


NLMISC_CATEGORISED_COMMAND(nel,hrtob, "Convert a human readable number into a bytes number", "<hr>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if (args.size() != 1)
		return false;

	log.displayNL("%s -> %u", args[0].c_str(), humanReadableToBytes(args[0]));

	return true;
}


string secondsToHumanReadable (uint32 time)
{
	static const char *divTable[] = { "s", "mn", "h", "d" };
	static uint  divCoef[]  = { 60, 60, 24 };
	uint div = 0;
	uint32 res = time;
	uint32 newres = res;
	for(;;)
	{
		if(div > 2)
			break;

		newres /= divCoef[div];

		if(newres < 3)
			break;

		div++;
		res = newres;
	}
	return toString ("%u%s", res, divTable[div]);
}

uint32 fromHumanReadable (const std::string &str)
{
	if (str.size() == 0)
		return 0;

	uint32 val;
	fromString(str, val);

	switch (str[str.size()-1])
	{
	case 's': return val;			// second
	case 'n': return val*60;		// minutes (mn)
	case 'h': return val*60*60;		// hour
	case 'd': return val*60*60*24;	// day
	case 'b':	// bytes
		switch (str[str.size()-2])
		{
		case 'k': return val*1024;
		case 'm': return val*1024*1024;
		case 'g': return val*1024*1024*1024;
		default : return val;
		}
	default: return val;
	}
	return 0;
}


NLMISC_CATEGORISED_COMMAND(nel,stohr, "Convert a second number into an human readable time", "<int>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if (args.size() != 1)
		return false;

	uint32 seconds;
	fromString(args[0], seconds);
	log.displayNL("%s -> %s", args[0].c_str(), secondsToHumanReadable(seconds).c_str());

	return true;
}


std::string	toLower(const std::string &str)
{
	string res;
	res.reserve(str.size());
	for(uint i = 0; i < str.size(); i++)
	{
		if( (str[i] >= 'A') && (str[i] <= 'Z') )
			res += str[i] - 'A' + 'a';
		else
			res += str[i];
	}
	return res;
}

char toLower(const char ch)
{
	if( (ch >= 'A') && (ch <= 'Z') )
	{
		return ch - 'A' + 'a';
	}
	else
	{
		return ch;
	}
}

void toLower(char *str)
{
	if (str == 0)
		return;

	while(*str != '\0')
	{
		if( (*str >= 'A') && (*str <= 'Z') )
		{
			*str = *str - 'A' + 'a';
		}
		str++;
	}
}

std::string	toUpper(const std::string &str)
{
	string res;
	res.reserve(str.size());
	for(uint i = 0; i < str.size(); i++)
	{
		if( (str[i] >= 'a') && (str[i] <= 'z') )
			res += str[i] - 'a' + 'A';
		else
			res += str[i];
	}
	return res;
}

void		toUpper(char *str)
{
	if (str == 0)
		return;

	while(*str != '\0')
	{
		if( (*str >= 'a') && (*str <= 'z') )
		{
			*str = *str - 'a' + 'A';
		}
		str++;
	}
}

std::string formatThousands(const std::string& s)
{
	sint i, k;
	sint remaining = (sint)s.length() - 1;
	static std::string separator = NLMISC::CI18N::get("uiThousandsSeparator").toUtf8();

	// Don't add separator if the number is < 10k
	if (remaining < 4) return s;

	std::string ns;

	do
	{
		for (i = remaining, k = 0; i >= 0 && k < 3; --i, ++k )
		{
			ns = s[i] + ns; // New char is added to front of ns
			if ( i > 0 && k == 2) ns = separator + ns; // j > 0 means still more digits
		}

		remaining -= 3;
	}
	while (remaining >= 0);

	return ns;
}

//
// Exceptions
//

Exception::Exception() : _Reason("Unknown Exception")
{
//	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const std::string &reason) : _Reason(reason)
{
	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

Exception::Exception(const char *format, ...)
{
	NLMISC_CONVERT_VARGS (_Reason, format, NLMISC::MaxCStringSize);
	nlinfo("Exception will be launched: %s", _Reason.c_str());
}

const char	*Exception::what() const throw()
{
	return _Reason.c_str();
}

bool killProgram(uint32 pid)
{
#ifdef NL_OS_UNIX
	int res = kill(pid, SIGKILL);
	if(res == -1)
	{
		char *err = strerror (errno);
		nlwarning("Failed to kill '%d' err %d: '%s'", pid, errno, err);
	}
	return res == 0;
/*#elif defined(NL_OS_WINDOWS)
	// it doesn't work because pid != handle and i don't know how to kill a pid or know the real handle of another service (not -1)
	int res = TerminateProcess((HANDLE)pid, 888);
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	nlwarning("Failed to kill '%d' err %d: '%s'", pid, GetLastError (), lpMsgBuf);
	LocalFree(lpMsgBuf);
	return res != 0;
*/
#else
	nlwarning("kill not implemented on this OS");
	return false;
#endif
}

bool abortProgram(uint32 pid)
{
#ifdef NL_OS_UNIX
	int res = kill(pid, SIGABRT);
	if(res == -1)
	{
		char *err = strerror (errno);
		nlwarning("Failed to abort '%d' err %d: '%s'", pid, errno, err);
	}
	return res == 0;
#else
	nlwarning("abort not implemented on this OS");
	return false;
#endif
}

bool launchProgram (const std::string &programName, const std::string &arguments)
{

#ifdef NL_OS_WINDOWS
	STARTUPINFOA         si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si.cb = sizeof(si);

/*	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	STARTUPINFO si;
	si.cb = sizeof (si);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.cbReserved2 = 0;
	si.wShowWindow = SW_MINIMIZE;
	si.lpReserved2 = NULL;

	PROCESS_INFORMATION pi;
*/

	// Enable nlassert/nlstop to display the error reason & callstack
	const TCHAR *SE_TRANSLATOR_IN_MAIN_MODULE = _T("NEL_SE_TRANS");
	TCHAR envBuf [2];
	if ( GetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, envBuf, 2 ) != 0)
	{
		SetEnvironmentVariable( SE_TRANSLATOR_IN_MAIN_MODULE, NULL );
	}

	string arg = " " + arguments;
	BOOL res = CreateProcessA(programName.c_str(), (char*)arg.c_str(), 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW, 0, 0, &si, &pi);

	if (res)
	{
		//nldebug("LAUNCH: Successful launch '%s' with arg '%s'", programName.c_str(), arguments.c_str());
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		return true;
	}
	else
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
		nlwarning("LAUNCH: Failed launched '%s' with arg '%s' err %d: '%s'", programName.c_str(), arguments.c_str(), GetLastError (), lpMsgBuf);
		LocalFree(lpMsgBuf);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}

#elif defined(NL_OS_UNIX)

	static bool firstLaunchProgram = true;
	if (firstLaunchProgram)
	{
		// The aim of this is to avoid defunct process.
		//
		// From "man signal":
		//------
		// According to POSIX (3.3.1.3) it is unspecified what happens when SIGCHLD is set to SIG_IGN.   Here
		// the  BSD  and  SYSV  behaviours  differ,  causing BSD software that sets the action for SIGCHLD to
		// SIG_IGN to fail on Linux.
		//------
		//
		// But it works fine on my GNU/Linux so I do this because it's easier :) and I don't know exactly
		// what to do to be portable.
		signal(SIGCHLD,SIG_IGN);

		firstLaunchProgram = false;
	}

	// convert one arg into several args
	vector<string> args;
	string::size_type pos1 = 0, pos2 = 0;
	do
	{
		pos1 = arguments.find_first_not_of (" ", pos2);
		if (pos1 == string::npos) break;
		pos2 = arguments.find_first_of (" ", pos1);
		args.push_back (arguments.substr (pos1, pos2-pos1));
	}
	while (pos2 != string::npos);

	// Store the size of each arg
	vector<char *> argv(args.size()+2);
	uint i = 0;
	argv[i] = (char *)programName.c_str();
	for (; i < args.size(); i++)
	{
		argv[i+1] = (char *) args[i].c_str();
	}
	argv[i+1] = NULL;

	int status = vfork ();
	/////////////////////////////////////////////////////////
	/// WARNING : NO MORE INSTRCUTION AFTER VFORK !
	/// READ VFORK manual
	/////////////////////////////////////////////////////////
	if (status == -1)
	{
		char *err = strerror (errno);
		nlwarning("LAUNCH: Failed launched '%s' with arg '%s' err %d: '%s'", programName.c_str(), arguments.c_str(), errno, err);
	}
	else if (status == 0)
    {

		// Exec (the only allowed instruction after vfork)
		status = execvp(programName.c_str(), &argv.front());

		if (status == -1)
		{
			perror("Failed launched");
			_exit(EXIT_FAILURE);
		}
	}
	else
	{
		//nldebug("LAUNCH: Successful launch '%s' with arg '%s'", programName.c_str(), arguments.c_str());
		return true;
	}
#else
	nlwarning ("LAUNCH: launchProgram() not implemented");
#endif

	return false;

}

/*
 * Display the bits (with 0 and 1) composing a byte (from right to left)
 */
void displayByteBits( uint8 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? '1' : '0';
	}
	log->displayRawNL( "%s", s1.c_str() );
	if ( displayBegin )
	{
		for ( i=nbits; i>beginpos+1; --i )
		{
			s2 += " ";
		}
		s2 += "^";
		log->displayRawNL( "%s beginpos=%u", s2.c_str(), beginpos );
	}
}


//#define displayDwordBits(a,b,c)

/*
 * Display the bits (with 0 and 1) composing a number (uint32) (from right to left)
 */
void displayDwordBits( uint32 b, uint nbits, sint beginpos, bool displayBegin, NLMISC::CLog *log )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? '1' : '0';
	}
	log->displayRawNL( "%s", s1.c_str() );
	if ( displayBegin )
	{
		for ( i=nbits; i>beginpos+1; --i )
		{
			s2 += " ";
		}
		s2 += "^";
		log->displayRawNL( "%s beginpos=%u", s2.c_str(), beginpos );
	}
}


int	nlfseek64( FILE *stream, sint64 offset, int origin )
{
#ifdef NL_OS_WINDOWS

	//
	fpos_t pos64 = 0;
	switch (origin)
	{
	case SEEK_CUR:
		if (fgetpos(stream, &pos64) != 0)
			return -1;
	case SEEK_END:
		pos64 = _filelengthi64(_fileno(stream));
		if (pos64 == -1L)
			return -1;
	};

	// Seek
	pos64 += offset;

	// Set the final position
	return fsetpos (stream, &pos64);

#else // NL_OS_WINDOWS
	// TODO: to fix for Linux and Mac OS X

	// This code doesn't work under windows : fseek() implementation uses a signed 32 bits offset. What ever we do, it can't seek more than 2 Go.
	// For the moment, i don't know if it works under linux for seek of more than 2 Go.

	nlassert ((offset < SINT64_CONSTANT(2147483647)) && (offset > SINT64_CONSTANT(-2147483648)));

	bool first = true;
	do
	{
		// Get the size of the next fseek
		sint nextSeek;
		if (offset > 0)
			nextSeek = (sint)std::min ((sint64)SINT64_CONSTANT(2147483647), offset);
		else
			nextSeek = (sint)std::max ((sint64)-SINT64_CONSTANT(2147483648), offset);

		// Make a seek
		int result = fseek ( stream, nextSeek, first?origin:SEEK_CUR );
		if (result != 0)
			return result;

		// Remaining
		offset -= nextSeek;
		first = false;
	}
	while (offset);

	return 0;

#endif // NL_OS_WINDOWS
}

sint64 nlftell64(FILE *stream)
{
#ifdef NL_OS_WINDOWS
	fpos_t pos64 = 0;
	if (fgetpos(stream, &pos64) == 0)
	{
		return (sint64) pos64;
	}
	else return -1;
#else
	nlunreferenced(stream);

	// TODO: implement for Linux and Mac OS X
	nlerror("Not implemented");
	return -1;
#endif
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// Commands
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

NLMISC_CATEGORISED_COMMAND(nel, sleep, "Freeze the service for N seconds (for debug purpose)", "<N>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 1) return false;

	sint32 n;
	fromString(args[0], n);

	log.displayNL ("Sleeping during %d seconds", n);

	nlSleep(n * 1000);
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, system, "Execute the command line using system() function call (wait until the end of the command)", "<commandline>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 1) return false;

	string cmd = args[0];
	log.displayNL ("Executing '%s'", cmd.c_str());
	sint error = system(cmd.c_str());
	if (error)
	{
		log.displayNL ("Execution of '%s' failed with error code %d", cmd.c_str(), error);
	}
	else
	{
		log.displayNL ("End of Execution of '%s'", cmd.c_str());
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, launchProgram, "Execute the command line using launcProgram() function call (launch in background task without waiting the end of the execution)", "<programName> <arguments>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 2) return false;

	string cmd = args[0];
	string arg = args[1];
	log.displayNL ("Executing '%s' with argument '%s'", cmd.c_str(), arg.c_str());
	launchProgram(cmd, arg);
	log.displayNL ("End of Execution of '%s' with argument '%s'", cmd.c_str(), arg.c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, killProgram, "kill a program given the pid", "<pid>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 1) return false;
	uint32 pid;
	fromString(args[0], pid);
	killProgram(pid);
	return true;
}

#ifdef NL_OS_WINDOWS
LONG GetRegKey(HKEY key, LPCSTR subkey, LPSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyExA(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS)
	{
        long datasize = MAX_PATH;
        char data[MAX_PATH];
        RegQueryValueA(hkey, NULL, data, &datasize);
        lstrcpyA(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}
#endif // NL_OS_WINDOWS

bool openURL (const char *url)
{
#ifdef NL_OS_WINDOWS
    char key[1024];
    if (GetRegKey(HKEY_CLASSES_ROOT, ".html", key) == ERROR_SUCCESS)
	{
        lstrcatA(key, "\\shell\\open\\command");

        if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS)
		{
            char *pos;
            pos = strstr(key, "\"%1\"");
            if (pos == NULL) {                     // No quotes found
                pos = strstr(key, "%1");       // Check for %1, without quotes
                if (pos == NULL)                   // No parameter at all...
                    pos = key+lstrlenA(key)-1;
                else
                    *pos = '\0';                   // Remove the parameter
            }
            else
                *pos = '\0';                       // Remove the parameter

            lstrcatA(pos, " ");
            lstrcatA(pos, url);
            int res = WinExec(key,SW_SHOWDEFAULT);
			return (res>31);
		}
	}
#elif defined(NL_OS_MAC)
	return launchProgram("open", url);
#elif defined(NL_OS_UNIX)
	return launchProgram("/etc/alternatives/x-www-browser", url);
#else
	nlwarning("openURL() is not implemented for this OS");
#endif // NL_OS_WINDOWS
	return false;
}

bool openDoc (const char *document)
{
#ifdef NL_OS_WINDOWS
	string ext = CFile::getExtension (document);
    char key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecuteA(NULL, "open", document, NULL,NULL, SW_SHOWDEFAULT);

    // If it failed, get the .htm regkey and lookup the program
    if ((uintptr_t)result <= HINSTANCE_ERROR)
	{
        if (GetRegKey(HKEY_CLASSES_ROOT, ext.c_str(), key) == ERROR_SUCCESS)
		{
            lstrcatA(key, "\\shell\\open\\command");

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS)
			{
                char *pos;
                pos = strstr(key, "\"%1\"");
                if (pos == NULL) {                     // No quotes found
                    pos = strstr(key, "%1");       // Check for %1, without quotes
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlenA(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcatA(pos, " ");
                lstrcatA(pos, document);
                int res = WinExec(key,SW_SHOWDEFAULT);
				return (res>31);
            }
        }
    }
	else
		return true;
#else
	// TODO: implement for Linux and Mac OS X
	nlunreferenced(document);
#endif // NL_OS_WINDOWS
	return false;
}

} // NLMISC
