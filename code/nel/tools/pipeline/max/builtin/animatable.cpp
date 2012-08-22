/**
 * \file animatable.cpp
 * \brief CAnimatable
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * CAnimatable
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
#include "animatable.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CAnimatable::CAnimatable()
{

}

CAnimatable::~CAnimatable()
{

}

const ucchar *CAnimatable::DisplayName = ucstring("Animatable").c_str();
const char *CAnimatable::InternalName = "Animatable";
const char *CAnimatable::InternalNameUnknown = "AnimatableUnknown";
const NLMISC::CClassId CAnimatable::ClassId = NLMISC::CClassId(0x3101497b, 0x24af711b); /* Not official, please correct */
const TSClassId CAnimatable::SuperClassId = 0x77a60fbd;/* Not official, please correct */
CAnimatableClassDesc AnimatableClassDesc(&DllPluginDescBuiltin);
CAnimatableSuperClassDesc AnimatableSuperClassDesc(&AnimatableClassDesc);

void CAnimatable::parse(uint16 version, TParseLevel level)
{

}

void CAnimatable::clean()
{

}

void CAnimatable::build(uint16 version)
{

}

void CAnimatable::disown()
{

}

void CAnimatable::init()
{

}

// inherited
const ISceneClassDesc *CAnimatable::classDesc()
{
	return &AnimatableClassDesc;
}

void CAnimatable::toStringLocal(std::ostream &ostream, const std::string &pad) const
{

}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
