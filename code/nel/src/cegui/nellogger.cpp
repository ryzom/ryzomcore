/**
 * \file nellogger.cpp
 * \brief NeLLogger
 * \date 2008-11-08 16:16GMT
 * \author Jan Boon (Kaetemi)
 * NeLLogger
 * 
 * $Id: nellogger.cpp 1099 2009-02-28 18:16:17Z kaetemi $
 */

/* 
 * Copyright (C) 2008  by authors
 * 
 * This file is part of NeL CEGUI Renderer.
 * NeL CEGUI Renderer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * NeL CEGUI Renderer is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NeL CEGUI Renderer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

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
