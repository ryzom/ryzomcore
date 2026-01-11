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

#ifndef NL_DISPLAYER_H
#define NL_DISPLAYER_H

#include "types_nl.h"

#include <string>

#include "log.h"

namespace NLMISC
{


class CMutex;


/**
 * Displayer interface. Used to specialize a displayer to display a string.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IDisplayer
{
public:

	/// Constructor
	IDisplayer(const char *displayerName = "");

	/// Destructor
	virtual ~IDisplayer();

	/// Display the string where it does.
	void display( const CLog::TDisplayInfo& args, const char *message );

	/// This is the identifier for a displayer, it is used to find or remove a displayer
	std::string DisplayerName;

protected:

	/// Method to implement in the derived class
	virtual void doDisplay( const CLog::TDisplayInfo& args, const char *message) = 0;


	// Return the header string with date (for the first line of the log)
	static const char *HeaderString ();

public:

	/// Convert log type to string
	static const char *logTypeToString (CLog::TLogType logType, bool longFormat = false);

	/// Convert the current date to human string
	static const char *dateToHumanString ();

	/// Convert date to "2000/01/14 10:05:17" string
	static const char *dateToHumanString (time_t date);

	/// Convert date to "784551148" string (time in second from 1975)
	static const char *dateToComputerString (time_t date);

private:

	CMutex	*_Mutex;
};



/**
 * Std displayer. Put string to stdout.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CStdDisplayer : virtual public IDisplayer
{
public:
	CStdDisplayer (const char *displayerName = "") : IDisplayer (displayerName) {}

protected:

	/// Display the string to stdout and OutputDebugString on Windows
	virtual void doDisplay ( const CLog::TDisplayInfo& args, const char *message );

};


/**
 * File displayer. Put string into a file.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CFileDisplayer : virtual public IDisplayer
{
public:

	/// Constructor
	CFileDisplayer (const std::string &filename, bool eraseLastLog = false, const char *displayerName = "", bool raw = false);

	CFileDisplayer ();

	~CFileDisplayer ();

	/// Set Parameter of the displayer if not set at the ctor time
	void setParam (const std::string &filename, bool eraseLastLog = false);

protected:
	/// Put the string into the file.
    virtual void doDisplay ( const CLog::TDisplayInfo& args, const char *message );

private:
	std::string _FileName;

	FILE		*_FilePointer;

	bool		_NeedHeader;

	uint		_LastLogSizeChecked;

	bool		_Raw;
};

/**
 * Message Box displayer. Put string into a message box.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CMsgBoxDisplayer : virtual public IDisplayer
{
public:
	CMsgBoxDisplayer (const char *displayerName = "") : IDisplayer (displayerName), IgnoreNextTime(false) {}

	bool IgnoreNextTime;

protected:
	/// Put the string into the file.
    virtual void doDisplay ( const CLog::TDisplayInfo& args, const char *message );
};


}

#endif // NL_DISPLAYER_H

/* End of displayer.h */
