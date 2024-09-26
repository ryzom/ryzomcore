/**
 * \file update1.cpp
 * \brief CUpdate1
 * \date 2012-08-26 12:11GMT
 * \author Jan Boon (Kaetemi)
 * CUpdate1
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
#include "update1.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "../scene_class_registry.h"

#include "editable_mesh.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace UPDATE1 {

CUpdate1::CUpdate1()
{

}

CUpdate1::~CUpdate1()
{

}

void CUpdate1::registerClasses(CSceneClassRegistry *registry)
{
	registry->add(&EditableMeshClassDesc);
}

const ucchar *CDllPluginDescUpdate1::displayName() const
{
	static const ucstring value = ucstring("Editable Mesh Object (Ryzom Core)");
	return value.c_str();
}

const ucchar *CDllPluginDescUpdate1::internalName() const
{
	static const ucstring value = ucstring("update1.dlo");
	return value.c_str();
}

const CDllPluginDescUpdate1 DllPluginDescUpdate1;

} /* namespace UPDATE1 */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
