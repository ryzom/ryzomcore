/**
 * \file wsm_derived_object.cpp
 * \brief CWSMDerivedObject
 * \date 2026-07-16 16:30GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CWSMDerivedObject
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
#include "wsm_derived_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CWSMDerivedObject::CWSMDerivedObject(CScene *scene) : CDerivedObject(scene)
{

}

CWSMDerivedObject::~CWSMDerivedObject()
{

}

const ucstring CWSMDerivedObject::DisplayName = ucstring("WSM Derived");
const char *CWSMDerivedObject::InternalName = "WSMDerivedObject";
const char *CWSMDerivedObject::InternalNameUnknown = "WSMDerivedObjectUnknown";
const NLMISC::CClassId CWSMDerivedObject::ClassId = NLMISC::CClassId(0x4ec13906, 0x5578130e);
// No superclass in the file — see CDerivedObject::SuperClassId. Literal to avoid the cross-TU
// static-init-order dependency (design-doc §2b).
const TSClassId CWSMDerivedObject::SuperClassId = 0x00000000;
const CWSMDerivedObjectClassDesc WSMDerivedObjectClassDesc(&DllPluginDescBuiltin);

void CWSMDerivedObject::parse(uint16 version, uint filter)
{
	CDerivedObject::parse(version);
}

void CWSMDerivedObject::clean()
{
	CDerivedObject::clean();
}

void CWSMDerivedObject::build(uint16 version, uint filter)
{
	CDerivedObject::build(version);
}

void CWSMDerivedObject::disown()
{
	CDerivedObject::disown();
}

void CWSMDerivedObject::init()
{
	CDerivedObject::init();
}

bool CWSMDerivedObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CDerivedObject::inherits(classId);
}

const ISceneClassDesc *CWSMDerivedObject::classDesc() const
{
	return &WSMDerivedObjectClassDesc;
}

void CWSMDerivedObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CDerivedObject::toStringLocal(ostream, pad);
}

IStorageObject *CWSMDerivedObject::createChunkById(uint16 id, bool container)
{
	return CDerivedObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
