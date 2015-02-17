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

#include "nel/misc/common.h"
#include "nel/misc/ucstring.h"

#include "nel/misc/report.h"
#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <windowsx.h>
#	include <winuser.h>
#endif // NL_OS_WINDOWS

#define NL_NO_DEBUG_FILES 1

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

//old doesn't work on visual c++ 7.1 due to default parameter typedef bool (*TEmailFunction) (const std::string &smtpServer, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachedFile = "", bool onlyCheck = false);
typedef bool (*TEmailFunction) (const std::string &smtpServer, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachedFile, bool onlyCheck);

static TEmailFunction EmailFunction = NULL;

void setReportEmailFunction (void *emailFunction)
{
	EmailFunction = (TEmailFunction)emailFunction;
}

#ifndef NL_OS_WINDOWS

// GNU/Linux, do nothing

void report ()
{
}

#else

TReportResult report (const std::string &title, const std::string &header, const std::string &subject, const std::string &body, bool enableCheckIgnore, uint debugButton, bool ignoreButton, sint quitButton, bool sendReportButton, bool &ignoreNextTime, const string &attachedFile)
{
	std::string fname;

	time_t s = time( NULL );
	fname = std::string( "log_" ) + toString( s ) + ".txt";

	std::ofstream f;
	f.open( fname.c_str() );
	if( f.good() )
	{
		f << body;
		f.close();

		NLMISC::launchProgram( "rcerror", fname );
	}

	NLMISC::CFile::deleteFile( fname );
	

#ifdef NL_OS_WINDOWS
#ifndef NL_COMP_MINGW
				// disable the Windows popup telling that the application aborted and disable the dr watson report.
				_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#endif
				// quit without calling atexit or static object dtors.
				abort();

	return ReportQuit;
}

#endif


} // NLMISC
