/**
 * \file dll_plugin_desc.h
 * \brief CDllPluginDesc
 * \date 2012-08-20 09:59GMT
 * \author Jan Boon (Kaetemi)
 * CDllPluginDesc
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_DLL_PLUGIN_DESC_H
#define PIPELINE_DLL_PLUGIN_DESC_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/ucstring.h>

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief IDllPluginDescInternal
 * \date 2012-08-20 09:59GMT
 * \author Jan Boon (Kaetemi)
 * Used for internal plugin descriptions that have no actual plugin
 * associated with them.
 */
class IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const = 0;
	virtual const ucchar *internalName() const = 0;

}; /* class IDllPluginDescInternal */

/**
 * \brief IDllPluginDesc
 * \date 2012-08-20 09:59GMT
 * \author Jan Boon (Kaetemi)
 * Inherit from this when making a plugin
 */
class IDllPluginDesc : public IDllPluginDescInternal
{
public:
	// virtual const ucchar *displayName() const = 0;
	// virtual const ucchar *internalName() const = 0;
	// TODO: list scene classes

}; /* class IDllPluginDesc */

/**
 * \brief CDllPluginDescBuiltin
 * \date 2012-08-20 09:59GMT
 * \author Jan Boon (Kaetemi)
 * Use only internally for builtin classes
 * TODO: Move to separate header?
 */
class CDllPluginDescBuiltin : public IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const;
	virtual const ucchar *internalName() const;
}; /* class CDllPluginDescBuiltin */

extern const CDllPluginDescBuiltin DllPluginDescBuiltin;

/**
 * \brief CDllPluginDescScript
 * \date 2012-08-20 09:59GMT
 * \author Jan Boon (Kaetemi)
 * Use only internally for scripts
 * TODO: Move to separate header?
 */
class CDllPluginDescScript : public IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const;
	virtual const ucchar *internalName() const;
}; /* class CDllPluginDescScript */

extern const CDllPluginDescScript DllPluginDescScript;

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_DLL_PLUGIN_DESC_H */

/* end of file */
