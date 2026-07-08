/**
 * \file biped.cpp
 * \brief CBiped
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * CBiped
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
#include "biped.h"

// STL includes

// NeL includes

// Project includes
#include "../scene_class_registry.h"

#include "biped_driven.h"
#include "biped_system.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BIPED {

CBiped::CBiped()
{

}

CBiped::~CBiped()
{

}

void CBiped::registerClasses(CSceneClassRegistry *registry)
{
	registry->add(&BipedDrivenClassDesc);
	registry->add(&BipedSystemClassDesc);
}

const ucchar *CDllPluginDescBiped::displayName() const
{
	static const ucstring value = ucstring("Biped Controller (Ryzom Core)");
	return value.c_str();
}

const ucchar *CDllPluginDescBiped::internalName() const
{
	static const ucstring value = ucstring("biped.dlc");
	return value.c_str();
}

const CDllPluginDescBiped DllPluginDescBiped;

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
