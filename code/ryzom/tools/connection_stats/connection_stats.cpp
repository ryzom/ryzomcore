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

// connection_stats.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#define LOG_ANALYSER_PLUGIN_EXPORTS
#include "connection_stats.h"

#include <windows.h>


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


#include <nel/misc/debug.h>
using namespace NLMISC;


#include <time.h>
#include <map>
using namespace std;


time_t				LogBeginTime = 0, LogEndTime = 0;
bool				ContinuationOfStatInPreviousFile = true;


//
string timeToStr( const time_t& ts )
{
	//return string(ctime( &ts )).substr( 0, 24 );
	return IDisplayer::dateToHumanString( ts );
}


//
string toHourStr( uint totalSec )
{
	uint hour = totalSec / 3600;
	uint remMin = totalSec % 3600;
	string res = toString( "%uh%02u'%02u\"", hour, remMin / 60, remMin % 60 );
	return res;
}


/*
 *
 */
struct TSession
{
	uint		ClientId;
	time_t		BeginTime;
	time_t		EndTime;
	time_t		Duration;
	bool		Closed;
};


/*
 *
 */
struct TPlayerStat
{
	uint				UserId;
	string				Name;
	vector<TSession>	Sessions;
	uint				Average;
	uint				Sum;
	uint				Min;
	uint				Max;

	///
	TPlayerStat() : UserId(0), Name("?"), Average(0), Sum(0), Min(0), Max(0) {}

	///
	bool				beginSession( const time_t& ts, uint clientId, uint userId, const string& name )
	{
		if ( Sessions.empty() || (Name == "?") )
		{
			init( userId, name );
		}

		if ( Sessions.empty() || (Sessions.back().EndTime != 0) )
		{
			// Open a new session
			TSession s;
			s.ClientId = clientId;
			s.BeginTime = ts;
			s.EndTime = 0;
			s.Closed = false;
			Sessions.push_back( s );
			return true;
		}
		else
		{
			// Occurs if two clients have the same userId
			nlwarning( "Opening a session for user %u (%s) at %s while previous session at %s not closed", UserId, Name.c_str(), timeToStr( ts ).c_str(), timeToStr( Sessions.back().BeginTime ).c_str() );
			//Sessions.back().ClientId = clientId; // cumulate times
			//return false;

			// Close the previous session
			bool userMissing;
			endSession( ts, Sessions.back().ClientId, userId, name, &userMissing, false );
			
			// Open a new session
			TSession s;
			s.ClientId = clientId;
			s.BeginTime = ts;
			s.EndTime = 0;
			s.Closed = false;
			Sessions.push_back( s );
			return false;

		}
	}

	/**
	 * Return true if the disconnection is valid, false if ignored. If true, set userMissing if connection of user not found in the log
	 * but was done before writing into this stat file.
	 */
	bool				endSession( const time_t& ts, uint clientId, uint userId, const string& name, bool *userMissing, bool closed=true )
	{
		if ( Sessions.empty() || (Name == "?") )
		{
			init( userId, name );
		}

		if ( Sessions.empty() )
		{
			// User was already connected at beginning of log
			if ( ContinuationOfStatInPreviousFile )
			{
				nldebug( "User %u (%s): disconnection at %s: connection before beginning of stat detected", userId, name.c_str(), timeToStr( ts ).c_str() );
				TSession s;
				s.ClientId = clientId;
				s.BeginTime = 0;
				s.EndTime = ts;
				s.Closed = closed;
				Sessions.push_back( s );
				*userMissing = true;
				return true;
			}
			else
			{
				nldebug( "User %u (%s): ignoring disconnection at %s (could not be connected before stat)", userId, name.c_str(), timeToStr( ts ).c_str() );
				return false;
			}
		}
		else
		{
			// Close the current session
			if ( clientId == Sessions.back().ClientId )
			{
				if ( Sessions.back().EndTime == 0 )
				{
					Sessions.back().EndTime = ts;
					Sessions.back().Closed = closed;
					*userMissing = false;
					return true;
				}
				else
				{
					nlwarning( "Detected two successive disconnections of user %u (%s) without reconnection (second ignored) at %s", userId, name.c_str(), timeToStr( ts ).c_str() );
					return false;
				}
			}
			else
			{
				// Occurs if two clients have the same userId
				nlwarning( "Closing a session for user %u (%s) with invalid client (ignored)", userId, name.c_str() );
				return false;
			}
		}
	}

	///
	sint				calcSessionTime( uint numSession, const time_t& testEndTime )
	{
		if ( numSession < Sessions.size() )
		{
			if ( Sessions[numSession].BeginTime == 0 )
			{
				Sessions[numSession].BeginTime = LogBeginTime;
				nlinfo( "User %u %s already connected at beginning of log (session end at %s)", UserId, Name.c_str(), timeToStr( Sessions[numSession].EndTime ).c_str() );
			}
			if ( Sessions[numSession].EndTime == 0 )
			{
				Sessions[numSession].EndTime = LogEndTime;
				nlinfo( "User %u %s still connected at end of log (session begin at %s)", UserId, Name.c_str(), timeToStr( Sessions[numSession].BeginTime ).c_str() );
			}
				
			Sessions[numSession].Duration = (int)difftime( Sessions[numSession].EndTime, Sessions[numSession].BeginTime );
			return Sessions[numSession].Duration;
		}
		else
			return 0;
	}

private:

	///
	void				init( uint userId, const string& name )
	{
		UserId = userId;
		Name = name;
	}

};


/*
 *
 */
struct TInstantNbPlayers
{
	uint			Nb;
	time_t			Timestamp;
	uint			UserId;
	string			Event;
};


/*
 *
 */
typedef std::map< uint, TPlayerStat > TPlayerMap;
typedef std::deque< TInstantNbPlayers > TNbPlayersSeries;
TPlayerMap			PlayerMap;
TNbPlayersSeries	NbPlayersSeries;
uint				NbPlayers;
string				MainStats;
float				TotalTimeInDays;


///
void		resetAll()
{
	LogBeginTime = 0;
	LogEndTime = 0;
	PlayerMap.clear();
	NbPlayersSeries.clear();
	NbPlayers = 0;
	MainStats = "";
}


///
void		addConnectionEvent( const time_t& ts, uint userId )
{
	++NbPlayers;
	TInstantNbPlayers inp;
	inp.UserId = userId;
	inp.Event = "+";
	if ( ts != 0 )
	{
		inp.Nb = NbPlayers;
		inp.Timestamp = ts;
		NbPlayersSeries.push_back( inp );
	}
	else
	{
		nldebug( "Inserting connection of user %u at beginning", userId );
		// Insert at front and increment every other number
		for ( TNbPlayersSeries::iterator iv=NbPlayersSeries.begin(); iv!=NbPlayersSeries.end(); ++iv )
			++(*iv).Nb;
		inp.Nb = 1;
		inp.Timestamp = LogBeginTime;
		NbPlayersSeries.push_front( inp );
	}
}


///
void		addDisconnectionEvent( const time_t& ts, uint userId )
{
	--NbPlayers;
	TInstantNbPlayers inp;
	inp.Nb = NbPlayers;
	inp.Timestamp = ts;
	inp.UserId = userId;
	inp.Event = "-";
	NbPlayersSeries.push_back( inp );
}


///
void		addConnection( const time_t& ts, uint clientId, uint userId, const string& name )
{
	if ( PlayerMap[userId].beginSession( ts, clientId, userId, name ) )
	{
		addConnectionEvent( ts, userId );
	}
}


///
void		addDisconnection( const time_t& ts, uint clientId, uint userId )
{
	bool userMissing;
	if ( PlayerMap[userId].endSession( ts, clientId, userId, "?", &userMissing ) )
	{
		if ( userMissing )
		{
			// Add connection at beginning if the server was started at a date anterior to the beginning of this stat file
			// (otherwise, just discard the disconnection, it could be a stat file corruption transformed
			// into server reset)
			addConnectionEvent( 0, userId );
		}

		addDisconnectionEvent( ts, userId );
	}
}


///
void	resetConnections( const time_t& shutdownTs, const time_t& restartTs )
{
	ContinuationOfStatInPreviousFile = false;

	TPlayerMap::iterator ipm;
	for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
	{
		if ( ! (*ipm).second.Sessions.empty() )
		{
			if ( (*ipm).second.Sessions.back().EndTime == 0 )
			{
				addDisconnection( shutdownTs, (*ipm).second.Sessions.back().ClientId, (*ipm).second.UserId );
				nlwarning( "Resetting connection of user %u because of server shutdown at %s (restart at %s)", (*ipm).second.UserId, timeToStr( shutdownTs ).c_str(), timeToStr( restartTs ).c_str() );
			}
		}
	}
}


///
void		fillUserNamesInEvents()
{
	TNbPlayersSeries::iterator iv;
	for ( iv=NbPlayersSeries.begin(); iv!=NbPlayersSeries.end(); ++iv )
	{
		(*iv).Event += PlayerMap[(*iv).UserId].Name;
	}
}


///
void		extractTime( const string& line, time_t& ts )
{
	struct tm t;
	t.tm_isdst = -1; // auto-detect Daylight Saving Time
	t.tm_wday = 0;
	t.tm_yday = 0;
	sscanf( line.c_str(), "%d/%d/%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec );
	t.tm_year -= 1900;
	t.tm_mon -= 1; // 0..11

	ts = mktime( &t );
	if ( ts == (time_t)-1 )
	{
		/*CString s;
		s.Format( "%d/%d/%d %d:%d:%d (%d)", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, ts );
		AfxMessageBox( s );*
		exit(-1);*/
		ts = 0;
	}
}


///
void			calcStats( string& res )
{
	TPlayerMap::iterator ipm;
	for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
	{
		uint sum = 0, themax = 0, themin = ~0;
		for ( uint i = 0; i!=(*ipm).second.Sessions.size(); ++i )
		{
			sum += (*ipm).second.Sessions[i].Duration;
			if ( (uint)(*ipm).second.Sessions[i].Duration < themin )
				themin = (*ipm).second.Sessions[i].Duration;
			if ( (uint)(*ipm).second.Sessions[i].Duration > themax )
				themax = (*ipm).second.Sessions[i].Duration;
		}
		(*ipm).second.Sum = sum;
		(*ipm).second.Average = sum / (*ipm).second.Sessions.size();
		(*ipm).second.Min = themin;
		(*ipm).second.Max = themax;
	}
}


// Return date for filename such as 2003-06-24
string extractDateFilename( time_t date )
{
	string dateStr = timeToStr( date );
	string::size_type pos;
	for ( pos=0; pos!=dateStr.size(); ++pos )
	{
		if ( dateStr[pos] == '/' )
			dateStr[pos] = '-';
		if ( dateStr[pos] == ' ' )
		{
			dateStr = dateStr.substr( 0, pos );
			break;
		}
	}
	return dateStr;

	/*// Revert date
	string::size_type slashPos = dateStr.rfind( '/' );
	if ( slashPos == string::npos )
		return "";
	string year = dateStr.substr( slashPos+1, slashPos+5 );
	slashPos = dateStr.rfind( '/', slashPos-1 );
	if ( slashPos == string::npos )
		return "";
	string month = dateStr.substr( slashPos+1, slashPos+3 );
	string day = dateStr.substr( 0, 2 );
	return year + "-" + month + "-" + day;*/
}


enum TMainStatEnum { MSNb, MSAverage, MSSum, MSMin, MSMax };


/// Return stats in float 'days'
void			getValuesStatsAndClearValues( vector<float>& values, string& res, bool isTimeInMinute, TMainStatEnum msEnum )
{
	float sum = 0.0f, themax = 0.0f, themin = 60.0f*24.0f*365.25f*100.0f; // 1 century should be enough
	vector<float>::const_iterator iv;
	for ( iv=values.begin(); iv!=values.end(); ++iv )
	{
		sum += (*iv);
		if ( (*iv) < themin )
			themin = (*iv);
		if ( (*iv) > themax )
			themax = (*iv);
	}
	if ( isTimeInMinute )
	{
		res += toString( "\t%g", sum / (float)values.size() / (24.0f*60.0f) ) +
			   toString( "\t%g", sum / (24.0f*60.0f) ) +
			   toString( "\t%g", themin / (24.0f*60.0f) ) +
			   toString( "\t%g", themax / (24.0f*60.0f) );
		switch( msEnum )
		{
		case MSAverage:
			break;
		case MSSum:
			MainStats += toString( "\t%g", sum / (float)values.size() / (24.0f*60.0f) / TotalTimeInDays ) +
						 toString( "\t%g", sum / (24.0f*60.0f) / TotalTimeInDays ) +
						 toString( "\t%g", themax / (24.0f*60.0f) / TotalTimeInDays );
			break;
		case MSMax:
			break;
		default:
			break;
		}

	}
	else
	{
		res += "\t" + toString( "%g", sum / (float)values.size() ) +
			   "\t" + toString( "%g", sum ) +
			   "\t" + toString( "%g", themin ) +
			   "\t" + toString( "%g", themax );
	}
	values.clear();
}


/// Return stats in float 'days'
void			getValuesStatsAndClearValues( vector<float>& values, vector< vector<string> >& table, bool isTimeInMinute, TMainStatEnum msEnum )
{
	float sum = 0.0f, themax = 0.0f, themin = 60.0f*24.0f*365.25f*100.0f; // 1 century should be enough
	vector<float>::const_iterator iv;
	for ( iv=values.begin(); iv!=values.end(); ++iv )
	{
		sum += (*iv);
		if ( (*iv) < themin )
			themin = (*iv);
		if ( (*iv) > themax )
			themax = (*iv);
	}
	if ( isTimeInMinute )
	{
		table.back().push_back( toString( "%g", sum / (float)values.size() / (24.0f*60.0f) ) );
		table.back().push_back( toString( "%g", sum / (24.0f*60.0f) ) );
		table.back().push_back( toString( "%g", themin / (24.0f*60.0f) ) );
		table.back().push_back( toString( "%g", themax / (24.0f*60.0f) ) );
		switch( msEnum )
		{
		case MSAverage:
			break;
		case MSSum:
			MainStats += toString( "\t%g", sum / (float)values.size() / (24.0f*60.0f) / TotalTimeInDays ) +
						 toString( "\t%g", sum / (24.0f*60.0f) / TotalTimeInDays ) +
						 toString( "\t%g", themax / (24.0f*60.0f) / TotalTimeInDays );
			break;
		case MSMax:
			break;
		default:
			break;
		}

	}
	else
	{
		table.back().push_back( toString( "%g", sum / (float)values.size() ) );
		table.back().push_back( toString( "%g", sum ) );
		table.back().push_back( toString( "%g", themin ) );
		table.back().push_back( toString( "%g", themax ) );
	}
	values.clear();
}


/// (Note: main stats only for minutes)
uint			getSessionDurations( string& res, time_t endTime, bool convertToMinutes, bool inColumnsWithDetail )
{
	uint sessionNum = 0;
	if ( inColumnsWithDetail )
	{
		string s1, s2;
		TPlayerMap::iterator ipm;
		for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
		{
			s1 += toString( "%u", (*ipm).second.UserId ) + "\t";
			s2 += (*ipm).second.Name + "\t";
		}
		res += s1 + "\r\n" + s2 + "\r\n";
		sint timeSum;
		do
		{
			timeSum = 0;
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				sint duration = (*ipm).second.calcSessionTime( sessionNum, endTime );
				if ( duration != 0 )
				{
					res += (convertToMinutes ? toString( "%.2f", (float)duration/60.0f ) : toString( "%d", duration )) + "\t";
				}
				else
				{
					res += string(convertToMinutes ? "0" : "") + "\t";
				}
				timeSum += duration;
			}
			res += "\r\n";

			++sessionNum;
		}
		while ( timeSum != 0 );
		res += "\r\n";

		calcStats( res );
		
		if ( ! convertToMinutes)
		{
			// Stats
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				res += toString( "%u\t", (*ipm).second.Sessions.size() );
			}
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				res += toString( "%u\t", (*ipm).second.Average );
			}
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				res += toString( "%u\t", (*ipm).second.Sum );
			}
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				res += toString( "%u\t", (*ipm).second.Min );
			}
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				res += toString( "%u\t", (*ipm).second.Max );
			}
			res += "\r\n";
		}
		else
		{
			// Stats
			vector<float> values;
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Sessions.size()) );
				res += toString( "%u\t", (*ipm).second.Sessions.size() );
			}
			getValuesStatsAndClearValues( values, res, false, MSNb );
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Average)/60.0f );
				res += toString( "%.2f\t", values.back() );
			}
			getValuesStatsAndClearValues( values, res, true, MSAverage );
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Sum)/60.0f );
				res += toString( "%.2f\t", values.back() );
			}
			getValuesStatsAndClearValues( values, res, true, MSSum );
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Min)/60.0f );
				res += toString( "%.2f\t", values.back() );
			}
			getValuesStatsAndClearValues( values, res, true, MSMin );
			res += "\r\n";
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Max)/60.0f );
				res += toString( "%.2f\t", values.back() );
			}
			getValuesStatsAndClearValues( values, res, true, MSMax );
			res += "\r\n";
		}
	}
	else
	{
		vector< vector<string> >	table;

		string s1, s2;
		table.push_back();
		table.push_back();
		TPlayerMap::iterator ipm;
		for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
		{
			table[0].push_back( toString( "%u", (*ipm).second.UserId ) ); //+ "\t";
			table[1].push_back( (*ipm).second.Name ); //+ "\t";
		}
		//res += s1 + "\r\n" + s2 + "\r\n";
		sint timeSum;
		do
		{
			timeSum = 0;
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				sint duration = (*ipm).second.calcSessionTime( sessionNum, endTime );
				timeSum += duration;
			}
			++sessionNum;
		}
		while ( timeSum != 0 );
		table.push_back();
		//res += "\r\n";

		calcStats( res );
		
		if ( ! convertToMinutes)
		{
			// Stats
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				table.back().push_back( toString( "%u", (*ipm).second.Sessions.size() ) );
			}
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				table.back().push_back( toString( "%u", (*ipm).second.Average ) );
			}
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				table.back().push_back( toString( "%u", (*ipm).second.Sum ) );
			}
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				table.back().push_back( toString( "%u", (*ipm).second.Min ) );
			}
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				table.back().push_back( toString( "%u", (*ipm).second.Max ) );
			}
			//res += "\r\n";
		}
		else
		{
			// Stats
			vector<float> values;
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Sessions.size()) );
				table.back().push_back( toString( "%u", (*ipm).second.Sessions.size() ) );
			}
			getValuesStatsAndClearValues( values, table, false, MSNb );
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Average)/60.0f );
				table.back().push_back( toString( "%.2f", values.back() ) );
			}
			getValuesStatsAndClearValues( values, table, true, MSAverage );
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Sum)/60.0f );
				table.back().push_back( toString( "%.2f", values.back() ) );
			}
			getValuesStatsAndClearValues( values, table, true, MSSum );
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Min)/60.0f );
				table.back().push_back( toString( "%.2f", values.back() ) );
			}
			getValuesStatsAndClearValues( values, table, true, MSMin );
			//res += "\r\n";
			table.push_back();
			for ( ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
			{
				values.push_back( (float)((*ipm).second.Max)/60.0f );
				table.back().push_back( toString( "%.2f", values.back() ) );
			}
			getValuesStatsAndClearValues( values, table, true, MSMax );
			//res += "\r\n";
		}

		// Print in column
		/*for ( uint j=0; j!=table.size(); ++j )
		{
			for ( uint i=0; i!=table[j].size(); ++i )
			{
				res += table[j][i] + "\t";
			}
			res += "\r\n";
		}*/

		// Print in row
		uint maxI = 0;
		for ( uint j=0; j!=table.size(); ++j )
		{
			if ( table[j].size() > maxI )
				maxI = table[j].size();
		}
		for ( uint i=0; i!=maxI; ++i )
		{
			for ( uint j=0; j!=table.size(); ++j )
			{
				if ( i < table[j].size() )
					res += table[j][i] + "\t";
				else
					res += "\t";
			}
			res += "\r\n";
		}
	}

	res += "\r\n";

	return sessionNum;
}


/*
 *
 */
LOG_ANALYSER_PLUGIN_API std::string getInfoString()
{
	return "Input log: connection.stat or frontend_service.log.\n\nProduces tab-separated connection stats for Excel.";
}


/*
 *
 */
LOG_ANALYSER_PLUGIN_API bool doAnalyse( const std::vector<const char *>& vec, std::string& res, std::string& log )
{
	NLMISC::createDebug();
	CMemDisplayer memdisp;
	NLMISC::DebugLog->addDisplayer( &memdisp, true );
	NLMISC::InfoLog->addDisplayer( &memdisp, true );
	NLMISC::WarningLog->addDisplayer( &memdisp, true );
	NLMISC::ErrorLog->addDisplayer( &memdisp, true );
	NLMISC::AssertLog->addDisplayer( &memdisp, true );

	resetAll();
	
	// Begin and end time
	if ( ! vec.empty() )
	{
		sint l = 0;
		sint quicklines = min( vec.size(), 100 );
		while ( (l < quicklines) && ((string(vec[l]).size()<5) || (vec[l][4] != '/')) )
			++l;
		if ( l < quicklines )
			extractTime( string(vec[l]), LogBeginTime );

		l = ((sint)vec.size())-1;
		quicklines = max( ((sint)vec.size())-100, 0 );
		while ( (l >= quicklines) && ((string(vec[l]).size()<5) || (vec[l][4] != '/')) )
			--l;
		if ( l >= quicklines )
			extractTime( string(vec[l]), LogEndTime );
	}
	res += "Log begin time\t\t" + timeToStr( LogBeginTime ) + "\r\n";
	res += "Log end time\t\t" + timeToStr( LogEndTime ) + "\r\n";
	MainStats += timeToStr( LogEndTime );
	TotalTimeInDays = ((float)(LogEndTime-LogBeginTime)) / 3600.0f / 24.0f;

	// Scan sessions
	uint nbPossibleCorruptions = 0;
	uint i;
	for ( i=0; i!=vec.size(); ++i )
	{
		string line = string(vec[i]);
		time_t ts;
		uint clientId;
		uint userId;
		string::size_type p;

		// Auto-detect file corruption (version for connections.stat)
		if ( !line.empty() )
		{
			bool corrupted = false;

			// Search for beginning not being a date or 'Log Starting"
			if ( (line.size() < 20) || (line[10]!=' ') || (line[13]!=':')
				|| (line[16]!=':') || (line[19]!=' ') || (line.substr( 20 ).find( " : ") == string::npos ) )
			{
				if ( line.find( "Log Starting [" ) != 0 )
					corrupted = true;
			}
			else
			{
				// Search for year not at beginning. Ex: "2003/" (it does not work when the year changes in the log!)
				p = line.substr( 1 ).find( timeToStr( LogBeginTime ).substr( 0, 5 ) );
				if ( p != string::npos )
				{
					++p; // because searched from pos 1

					// Search for date/time
					if ( (line.size()>p+20) && (line[p+10]==' ') && (line[p+13]==':')
						 && (line[p+16]==':') && (line[p+19]==' ') )
					{
						// Search for the two next blank characters. The second is followed by ": ".
						// (Date Time ThreadId Machine/Service : User-defined log line)
						uint nbBlank = 0;
						string::size_type sp;
						for ( sp=p+20; sp!=line.size(); ++sp )
						{
							if ( line[sp]==' ')
								++nbBlank;
							if ( nbBlank==2 )
								break;
						}
						if ( (nbBlank==2) && (line[sp+1]==':') && (line[sp+2]==' ') )
						{
							corrupted = true;
						}
					}
				}
			}
			if ( corrupted )
			{
				++nbPossibleCorruptions;
				nlwarning( "Found possible file corruption at line %u: %s", i, line.c_str() );
			}
		}

		// Detect connections/disconnections
		p = line.find( "Adding client" );
		if ( p != string::npos )
		{
			extractTime( line, ts );
			char name [200];
			sscanf( line.substr( p ).c_str(), "Adding client %u (uid %u name %s", &clientId, &userId, &name );
			string sname = name;
			addConnection( ts, clientId, userId, sname /*sname.substr( 0, sname.size()-1 )*/ ); // now name format is "name priv ''".
			continue;
		}
		p = line.find( "Sent CL_DISCONNECT for client" );
		if ( p != string::npos )
		{
			extractTime( line, ts );
			sscanf( line.substr( p ).c_str(), "Sent CL_DISCONNECT for client %u (uid %u)", &clientId, &userId );
			addDisconnection( ts, clientId, userId );
			continue;
		}
		p = line.find( "Log Starting [" );
		if ( p != string::npos )
		{
			uint hs = string("Log Starting [").size();
			line = line.substr( hs, line.size() - hs - 1 ); // remove ] to get the timestamp
			time_t restartTs;
			extractTime( line, restartTs );
			// Go back to find the last time of log
			sint quicklines = max( ((sint)i)-10, 0 );
			sint l = ((sint)i)-1;
			while ( (l >= quicklines) && ((string(vec[l]).size()<5) || (vec[l][4] != '/')) )
				l--;
			if ( l >= quicklines )
				extractTime( vec[l], ts );
			else
				ts = restartTs;
			resetConnections( ts, restartTs );
		}
	}
	fillUserNamesInEvents();

	// Session durations
	string sd;
	uint maxNbSession = getSessionDurations( sd, LogEndTime, true, false );
	res += "Number of accounts\t" + toString( "%u", PlayerMap.size() ) + "\r\n";
	res += "Max number of session\t" + toString( "%u", maxNbSession ) + "\r\n";
	res += "\r\nTime of sessions\r\n";
	res += sd;
	res += "Connection events\r\n";
	MainStats += toString( "\t%u", PlayerMap.size() );
	
	// Timetable
	time_t prevTimestamp = 0;
	sint durationSum = 0;
	int prevPlayerNb = -1;
	uint maxPlayerNb = 0;
	for ( i=0; i!=NbPlayersSeries.size(); ++i )
	{
		sint duration = (prevTimestamp!=0) ? (int)difftime( NbPlayersSeries[i].Timestamp, prevTimestamp ) : 0;
		prevTimestamp = NbPlayersSeries[i].Timestamp;
		durationSum += duration;
		if ( prevPlayerNb != -1 )
			res += "\t" + toString( "%d", durationSum ) + "\t" + timeToStr( NbPlayersSeries[i].Timestamp ) + "\t" + toString( "%u", prevPlayerNb ) + "\t" + "\r\n";
		res += toString( "%d", duration ) + "\t" + toString( "%d", durationSum ) + "\t" + timeToStr( NbPlayersSeries[i].Timestamp ) + "\t" + toString( "%u", NbPlayersSeries[i].Nb ) + "\t" + NbPlayersSeries[i].Event + "\r\n";
		prevPlayerNb = NbPlayersSeries[i].Nb;
		if ( NbPlayersSeries[i].Nb > maxPlayerNb )
			maxPlayerNb = NbPlayersSeries[i].Nb;
	}
	MainStats += toString( "\t%u", maxPlayerNb ) + toString( "\t\t(%g days)", TotalTimeInDays );
	if ( nbPossibleCorruptions == 0 )
		MainStats += toString( "\t\tStat file OK" );
	else
		MainStats += toString( "\t\tFound %u possible stat file corruptions (edit the stat file to replace them with server reset if time too long, see log.log)", nbPossibleCorruptions );

	// Stats per user
	res += "\r\n\nStats per user (hrs)\r\n\nName\tUserId\tSessions\tCumulated\tAverage\tMin\tMax";
	for ( TPlayerMap::const_iterator ipm=PlayerMap.begin(); ipm!=PlayerMap.end(); ++ipm )
	{
		res += "\r\n\n";
		const TPlayerStat& playerStat = (*ipm).second;
		res += toString( "%s\tUser %u\t%u\t%s\t%s\t%s\t%s",
			playerStat.Name.c_str(), playerStat.UserId,	playerStat.Sessions.size(),
			toHourStr(playerStat.Sum).c_str(), toHourStr(playerStat.Average).c_str(), toHourStr(playerStat.Min).c_str(), toHourStr(playerStat.Max).c_str() );
		for ( uint i=0; i!=playerStat.Sessions.size(); ++i )
		{
			res += "\r\n";
			const TSession& sess = playerStat.Sessions[i];
			string status = sess.Closed ? "OK" : "Not closed";
			res += timeToStr( sess.BeginTime ) + "\t" + timeToStr( sess.EndTime ) + "\t" + status + "\t" + toHourStr( sess.Duration );
		}
	}

	string dateStr = " " + extractDateFilename( LogEndTime );
	res = dateStr + "\r\nDate\tAvg per player\tTotal time\tMax per player\tNb Players\tSimult. Pl.\r\n" + MainStats + "\r\n" + res;

	memdisp.write( log );
	return true;
}



/*CString s;
s.Format( "Found C=%u U=%u N=%s in %s", clientId, userId, name, line.substr( p ).c_str() );
AfxMessageBox( s );*/