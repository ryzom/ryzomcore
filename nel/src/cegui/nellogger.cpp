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

#include <nel/misc/types_nl.h>
#include <nel/cegui/nellogger.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace CEGUI {

NeLLogger::NeLLogger()
{
	
}

NeLLogger::~NeLLogger()
{
	
}

void NeLLogger::logEvent(const String& message, LoggingLevel level)
{
	if (getLoggingLevel() >= level) switch (level)
	{
	case Insane:
		nldebug("%s", message.c_str());
		break;
	case Informative:
	case Standard:
		nlinfo("%s", message.c_str());
		break;
#if ((CEGUI_VERSION_MAJOR > 0) || ((CEGUI_VERSION_MAJOR >= 0) && (CEGUI_VERSION_MINOR >= 6)))
	case Warnings:
#endif
	case Errors:
		nlwarning("%s", message.c_str());
		break;
	}
}

void NeLLogger::setLogFilename(const String& filename, bool append)
{
	// do nothing
}

} /* namespace CEGUI */

/* end of file */
