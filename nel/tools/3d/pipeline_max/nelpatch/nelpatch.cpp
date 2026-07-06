/**
 * \file nelpatch.cpp
 * \brief CNelPatch
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
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

#include <nel/misc/types_nl.h>
#include "nelpatch.h"

// STL includes

// NeL includes

// Project includes
#include "../scene_class_registry.h"

#include "rkl_patch_object.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

CNelPatch::CNelPatch()
{

}

CNelPatch::~CNelPatch()
{

}

void CNelPatch::registerClasses(CSceneClassRegistry *registry)
{
	registry->add(&RklPatchObjectClassDesc);
}

const ucchar *CDllPluginDescNelPatch::displayName() const
{
	static const ucstring value = ucstring("NeL Patch Mesh (Ryzom Core)");
	return value.c_str();
}

const ucchar *CDllPluginDescNelPatch::internalName() const
{
	static const ucstring value = ucstring("nelconvertpatch_r.dlm");
	return value.c_str();
}

const CDllPluginDescNelPatch DllPluginDescNelPatch;

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
