/**
 * \file nellogger.h
 * \brief NeLLogger
 * \date 2008-11-08 16:16GMT
 * \author Jan Boon (Kaetemi)
 * NeLLogger
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

#ifndef CEGUI_NELLOGGER_H
#define CEGUI_NELLOGGER_H
#include <nel/misc/types_nl.h>

// STL includes

// CEGUI includes
#include <CEGUIBase.h>
#include <CEGUILogger.h>

// NeL includes

// Project includes

#ifdef NL_OS_WINDOWS
#ifdef NEL_CEGUIRENDERER_EXPORTS
#define DLLSPEC __declspec(dllexport)
#else //NEL_CEGUI_RENDERER_EXPORTS
#define DLLSPEC __declspec(dllimport)
#endif // NEL_CEGUI_RENDERER_EXPORTS
#else // NL_OS_WINDOWS
#define DLLSPEC 
#endif // NL_OS_WINDOWS

namespace CEGUI {

/**
 * \brief NeLLogger
 * \date 2008-11-08 16:16GMT
 * \author Jan Boon (Kaetemi)
 * NeLLogger
 */
class DLLSPEC NeLLogger : public Logger
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	NeLLogger();
	virtual ~NeLLogger();

	virtual void logEvent(const String& message, LoggingLevel level = Standard);
    virtual void setLogFilename(const String& filename, bool append = false);
}; /* class NeLLogger */

} /* namespace CEGUI */

#endif /* #ifndef CEGUI_NELLOGGER_H */

/* end of file */
