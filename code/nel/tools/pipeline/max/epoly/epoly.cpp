/**
 * \file epoly.cpp
 * \brief CEPoly
 * \date 2012-08-26 12:03GMT
 * \author Jan Boon (Kaetemi)
 * CEPoly
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
#include "epoly.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "../scene_class_registry.h"

#include "editable_poly.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace EPOLY {

CEPoly::CEPoly()
{

}

CEPoly::~CEPoly()
{

}

void CEPoly::registerClasses(CSceneClassRegistry *registry)
{
	registry->add(&EditablePolyClassDesc);
}

const ucchar *CDllPluginDescEPoly::displayName() const
{
	static const ucstring value = ucstring("Editable Poly Object (Ryzom Core)");
	return value.c_str();
}

const ucchar *CDllPluginDescEPoly::internalName() const
{
	static const ucstring value = ucstring("EPoly.dlo");
	return value.c_str();
}

const CDllPluginDescEPoly DllPluginDescEPoly;

} /* namespace EPOLY */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
