// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
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

#include "nel/misc/log.h"

#ifdef NL_OS_WINDOWS
#	include <process.h>
#else
#	include <unistd.h>
#endif

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

string *CLog::_ProcessName = NULL;

CLog::CLog( TLogType logType) : _LogType (logType), _FileName(NULL), _Line(-1), _FuncName(NULL), _Mutex("LOG"+toString((uint)logType)), _PosSet(false)
{
}

void CLog::setDefaultProcessName ()
{
	if (_ProcessName == NULL)
	{
		_ProcessName = (string *)INelContext::getInstance().getSingletonPointer("NLMISC::CLog::_ProcessName");
		if (_ProcessName == NULL)
		{
			_ProcessName = new string;
			INelContext::getInstance().setSingletonPointer("NLMISC::CLog::_ProcessName", _ProcessName);
		}
	}

#ifdef NL_OS_WINDOWS
	if ((*_ProcessName).empty())
	{
		wchar_t name[1024];
		GetModuleFileNameW(NULL, name, 1023);
		(*_ProcessName) = CFile::getFilename(wideToUtf8(name));
	}
#else
	if ((*_ProcessName).empty())
	{
		*_ProcessName = "<Unknown>";
	}
#endif
}

void CLog::setProcessName (const std::string &processName)
{
	if (_ProcessName == NULL)
	{
		_ProcessName = (string *)INelContext::getInstance().getSingletonPointer("NLMISC::CLog::_ProcessName");
		if (_ProcessName == NULL)
		{
			_ProcessName = new string;
			INelContext::getInstance().setSingletonPointer("NLMISC::CLog::_ProcessName", _ProcessName);
		}
	}

	// keep only filename without path
	*_ProcessName = CFile::getFilename(processName);
}

void CLog::setPosition (sint line, const char *fileName, const char *funcName)
{
	if ( !noDisplayer() )
	{
		_Mutex.enter();
		_PosSet++;
		_FileName = fileName;
		_Line = line;
		_FuncName = funcName;
	}
}

/// Symetric to setPosition(). Automatically called by display...(). Do not call if noDisplayer().
void CLog::unsetPosition()
{
	nlassert( !noDisplayer() );

	if ( _PosSet > 0 )
	{
		_FileName = NULL;
		_Line = -1;
		_FuncName = NULL;
		_PosSet--;
		_Mutex.leave(); // needs setPosition() to have been called
	}
}


void CLog::addDisplayer (IDisplayer *displayer, bool bypassFilter)
{
	if (displayer == NULL)
	{
		// Can't nlwarning because recursive call
		printf ("Trying to add a NULL displayer\n");
		return;
	}

	if (bypassFilter)
	{
		CDisplayers::iterator idi = std::find (_BypassFilterDisplayers.begin (), _BypassFilterDisplayers.end (), displayer);
		if (idi == _BypassFilterDisplayers.end ())
		{
			_BypassFilterDisplayers.push_back (displayer);
		}
		else
		{
			nlwarning ("LOG: Couldn't add the displayer, it was already added");
		}
	}
	else
	{
		CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
		if (idi == _Displayers.end ())
		{
			_Displayers.push_back (displayer);
		}
		else
		{
			nlwarning ("LOG: Couldn't add the displayer, it was already added");
		}
	}
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL)
	{
		nlwarning ("LOG: Trying to remove a NULL displayer");
		return;
	}

	CDisplayers::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi != _Displayers.end ())
	{
		_Displayers.erase (idi);
	}

	idi = std::find (_BypassFilterDisplayers.begin (), _BypassFilterDisplayers.end (), displayer);
	if (idi != _BypassFilterDisplayers.end ())
	{
		_BypassFilterDisplayers.erase (idi);
	}

}

void CLog::removeDisplayer (const char *displayerName)
{
	if (displayerName == NULL || displayerName[0] == '\0')
	{
		nlwarning ("LOG: Trying to remove an empty displayer name");
		return;
	}

	CDisplayers::iterator idi;
	for (idi = _Displayers.begin (); idi != _Displayers.end ();)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			idi = _Displayers.erase (idi);
		}
		else
		{
			idi++;
		}
	}

	for (idi = _BypassFilterDisplayers.begin (); idi != _BypassFilterDisplayers.end ();)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			idi = _BypassFilterDisplayers.erase (idi);
		}
		else
		{
			idi++;
		}
	}
}

IDisplayer *CLog::getDisplayer (const char *displayerName)
{
	if (displayerName == NULL || displayerName[0] == '\0')
	{
		nlwarning ("LOG: Trying to get an empty displayer name");
		return NULL;
	}

	CDisplayers::iterator idi;
	for (idi = _Displayers.begin (); idi != _Displayers.end (); idi++)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			return *idi;
		}
	}
	for (idi = _BypassFilterDisplayers.begin (); idi != _BypassFilterDisplayers.end (); idi++)
	{
		if ((*idi)->DisplayerName == displayerName)
		{
			return *idi;
		}
	}
	return NULL;
}

/*
 * Returns true if the specified displayer is attached to the log object
 */
bool CLog::attached(IDisplayer *displayer) const
{
	return (find( _Displayers.begin(), _Displayers.end(), displayer ) != _Displayers.end()) ||
			(find( _BypassFilterDisplayers.begin(), _BypassFilterDisplayers.end(), displayer ) != _BypassFilterDisplayers.end());
}


void CLog::displayString (const char *str)
{
	const char *disp = NULL;
	TDisplayInfo localargs, *args = NULL;

	setDefaultProcessName ();

	if(strchr(str,'\n') == NULL)
	{
		if (TempString.empty())
		{
			time (&TempArgs.Date);
			TempArgs.LogType = _LogType;
			TempArgs.ProcessName = *_ProcessName;
			TempArgs.ThreadId = getThreadId();
			TempArgs.FileName = _FileName;
			TempArgs.Line = _Line;
			TempArgs.FuncName = _FuncName;
			TempArgs.CallstackAndLog.clear();

			TempString = str;
		}
		else
		{
			TempString += str;
		}
		return;
	}
	else
	{
		if (TempString.empty())
		{
			time (&localargs.Date);
			localargs.LogType = _LogType;
			localargs.ProcessName = *_ProcessName;
			localargs.ThreadId = getThreadId();
			localargs.FileName = _FileName;
			localargs.Line = _Line;
			localargs.FuncName = _FuncName;
			localargs.CallstackAndLog.clear();

			disp = str;
			args = &localargs;
		}
		else
		{
			TempString += str;
			disp = TempString.c_str();
			args = &TempArgs;
		}
	}

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( *args, disp );
	}

	// get the log at the last minute to be sure to have everything
	if(args->LogType == LOG_ERROR || args->LogType == LOG_ASSERT)
	{
		getCallStackAndLog (args->CallstackAndLog, 4);
	}

	if (passFilter (disp))
	{
		// Send to the attached displayers
		for (CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			(*idi)->display( *args, disp );
		}
	}
	TempString.clear();
	unsetPosition();
}


/*
 * Display the string with decoration and final new line to all attached displayers
 */
#ifdef NL_OS_WINDOWS
void CLog::_displayNL (const char *format, ...)
#else
void CLog::displayNL (const char *format, ...)
#endif
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 1024/*NLMISC::MaxCStringSize*/);

	if (strlen(str)<1024/*NLMISC::MaxCStringSize*/-1)
		strcat (str, "\n");
	else
		str[1024/*NLMISC::MaxCStringSize*/-2] = '\n';

	displayString (str);
}

/*
 * Display the string with decoration to all attached displayers
 */
#ifdef NL_OS_WINDOWS
void CLog::_display (const char *format, ...)
#else
void CLog::display (const char *format, ...)
#endif
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 1024/*NLMISC::MaxCStringSize*/);

	displayString (str);
}

void CLog::displayRawString (const char *str)
{
	const char *disp = NULL;
	TDisplayInfo localargs, *args = NULL;

	setDefaultProcessName ();

	if(strchr(str,'\n') == NULL)
	{
		if (TempString.empty())
		{
			localargs.Date = 0;
			localargs.LogType = CLog::LOG_NO;
			localargs.ProcessName.clear();
			localargs.ThreadId = 0;
			localargs.FileName = NULL;
			localargs.Line = -1;
			localargs.CallstackAndLog.clear();

			TempString = str;
		}
		else
		{
			TempString += str;
		}
		return;
	}
	else
	{
		if (TempString.empty())
		{
			localargs.Date = 0;
			localargs.LogType = CLog::LOG_NO;
			localargs.ProcessName.clear();
			localargs.ThreadId = 0;
			localargs.FileName = NULL;
			localargs.Line = -1;
			localargs.CallstackAndLog.clear();

			disp = str;
			args = &localargs;
		}
		else
		{
			TempString += str;
			disp = TempString.c_str();
			args = &TempArgs;
		}
	}

	// send to all bypass filter displayers
	for (CDisplayers::iterator idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( *args, disp );
	}

	// get the log at the last minute to be sure to have everything
	if(args->LogType == LOG_ERROR || args->LogType == LOG_ASSERT)
	{
		getCallStackAndLog (args->CallstackAndLog, 4);
	}

	if ( passFilter( disp ) )
	{
		// Send to the attached displayers
		for ( CDisplayers::iterator idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
		{
			(*idi)->display( *args, disp );
		}
	}
	TempString.clear();
	unsetPosition();
}

/*
 * Display a string (and nothing more) to all attached displayers
 */
#ifdef NL_OS_WINDOWS
void CLog::_displayRawNL( const char *format, ... )
#else
void CLog::displayRawNL( const char *format, ... )
#endif
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 1024/*NLMISC::MaxCStringSize*/);

	if (strlen(str)<1024/*NLMISC::MaxCStringSize*/-1)
		strcat (str, "\n");
	else
		str[1024/*NLMISC::MaxCStringSize*/-2] = '\n';

	displayRawString(str);
}

/*
 * Display a string (and nothing more) to all attached displayers
 */
#ifdef NL_OS_WINDOWS
void CLog::_displayRaw( const char *format, ... )
#else
void CLog::displayRaw( const char *format, ... )
#endif
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 1024/*NLMISC::MaxCStringSize*/);

	displayRawString(str);
}


#ifdef NL_OS_WINDOWS
void CLog::_forceDisplayRaw (const char *format, ...)
#else
void CLog::forceDisplayRaw (const char *format, ...)
#endif
{
	if ( noDisplayer() )
	{
		return;
	}

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 1024/*NLMISC::MaxCStringSize*/);

	TDisplayInfo args;
	CDisplayers::iterator idi;

	// send to all bypass filter displayers
	for (idi=_BypassFilterDisplayers.begin(); idi!=_BypassFilterDisplayers.end(); idi++ )
	{
		(*idi)->display( args, str );
	}

	// Send to the attached displayers
	for ( idi=_Displayers.begin(); idi!=_Displayers.end(); idi++ )
	{
		(*idi)->display( args, str );
	}
}



/*
 * Returns true if the string must be logged, according to the current filter
 */
bool CLog::passFilter( const char *filter )
{
	bool yes = _PositiveFilter.empty();

	bool found;
	list<string>::iterator ilf;

	// 1. Positive filter
	for ( ilf=_PositiveFilter.begin(); ilf!=_PositiveFilter.end(); ++ilf )
	{
		found = ( strstr( filter, (*ilf).c_str() ) != NULL );
		if ( found )
		{
			yes = true; // positive filter passed (no need to check another one)
			break;
		}
		// else try the next one
	}
	if ( ! yes )
	{
		return false; // positive filter not passed
	}

	// 2. Negative filter
	for ( ilf=_NegativeFilter.begin(); ilf!=_NegativeFilter.end(); ++ilf )
	{
		found = ( strstr( filter, (*ilf).c_str() ) != NULL );
		if ( found )
		{
			return false; // negative filter not passed (no need to check another one)
		}
	}
	return true; // negative filter passed
}


/*
 * Removes a filter by name. Returns true if it was found.
 */
void CLog::removeFilter( const char *filterstr )
{
	if (filterstr == NULL)
	{
		_PositiveFilter.clear();
		_NegativeFilter.clear();
		//displayNL ("CLog::addNegativeFilter('%s')", filterstr);
	}
	else
	{
		_PositiveFilter.remove( filterstr );
		_NegativeFilter.remove( filterstr );
		//displayNL ("CLog::removeFilter('%s')", filterstr);
	}
}

void CLog::displayFilter( CLog &log )
{
	std::list<std::string>::iterator it;
	log.displayNL ("Positive Filter(s):");
	for (it = _PositiveFilter.begin (); it != _PositiveFilter.end (); it++)
	{
		log.displayNL ("'%s'", (*it).c_str());
	}
	log.displayNL ("Negative Filter(s):");
	for (it = _NegativeFilter.begin (); it != _NegativeFilter.end (); it++)
	{
		log.displayNL ("'%s'", (*it).c_str());
	}
}

void CLog::addPositiveFilter( const char *filterstr )
{
	//displayNL ("CLog::addPositiveFilter('%s')", filterstr);
	_PositiveFilter.push_back( filterstr );
}

void CLog::addNegativeFilter( const char *filterstr )
{
	//displayNL ("CLog::addNegativeFilter('%s')", filterstr);
	_NegativeFilter.push_back( filterstr );
}

void CLog::resetFilters()
{
	//displayNL ("CLog::resetFilter()");
	_PositiveFilter.clear();
	_NegativeFilter.clear();
}

/// Do not call this unless you know why you're doing it, it kills the debug/log system!
void CLog::releaseProcessName()
{
	if (INelContext::isContextInitialised())
	{
		INelContext::getInstance().releaseSingletonPointer("NLMISC::CLog::_ProcessName", _ProcessName);
	}

	if (_ProcessName)
	{
		delete _ProcessName;
		_ProcessName = NULL;
	}
}

} // NLMISC

