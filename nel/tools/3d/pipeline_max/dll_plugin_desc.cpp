/**
 * \file dll_plugin_desc.cpp
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

#include <nel/misc/types_nl.h>
#include "dll_plugin_desc.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

const ucchar *CDllPluginDescBuiltin::displayName() const
{
	static const ucstring value = ucstring("Builtin");
	return value.c_str();
}

const ucchar *CDllPluginDescBuiltin::internalName() const
{
	static const ucstring value = ucstring("Builtin");
	return value.c_str();
}

const CDllPluginDescBuiltin DllPluginDescBuiltin;

const ucchar *CDllPluginDescScript::displayName() const
{
	static const ucstring value = ucstring("Script");
	return value.c_str();
}

const ucchar *CDllPluginDescScript::internalName() const
{
	static const ucstring value = ucstring("Script");
	return value.c_str();
}

const CDllPluginDescScript DllPluginDescScript;

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
