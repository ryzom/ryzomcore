/**
 * \file nelpatch.h
 * \brief CNelPatch
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CNelPatch
 */

/*
 * Copyright (C) 2026  by authors
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

#ifndef PIPELINE_NELPATCH_H
#define PIPELINE_NELPATCH_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../dll_plugin_desc.h"

namespace PIPELINE {
namespace MAX {

class CSceneClassRegistry;

namespace NELPATCH {

/**
 * \brief CNelPatch
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * Registers the typed classes for the NeL patch mesh converter plugin (nelconvertpatch_r.dlm,
 * "Rykol Patchmesh Converter") — the in-tree plugin_max/nel_patch_lib plugin that stores the
 * NeL landscape patch grids (RklPatch objects) inside .max files.
 */
class CNelPatch
{
public:
	static void registerClasses(CSceneClassRegistry *registry);

public:
	CNelPatch();
	virtual ~CNelPatch();

}; /* class CNelPatch */

/**
 * \brief CDllPluginDescNelPatch
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CDllPluginDescNelPatch
 */
class CDllPluginDescNelPatch : public IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const;
	virtual const ucchar *internalName() const;
}; /* class CDllPluginDescNelPatch */

extern const CDllPluginDescNelPatch DllPluginDescNelPatch;

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_NELPATCH_H */

/* end of file */
