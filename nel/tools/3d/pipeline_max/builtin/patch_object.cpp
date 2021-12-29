/**
 * \file patch_object.cpp
 * \brief CPatchObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CPatchObject
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
#include "patch_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CPatchObject::CPatchObject(CScene *scene) : CGeomObject(scene)
{

}

CPatchObject::~CPatchObject()
{

}

const ucstring CPatchObject::DisplayName = ucstring("PatchObject");
const char *CPatchObject::InternalName = "PatchObject";
const NLMISC::CClassId CPatchObject::ClassId = NLMISC::CClassId(0xd0a6b36, 0x7dce4b64); /* Not official, please correct */
const TSClassId CPatchObject::SuperClassId = CGeomObject::SuperClassId;
const CPatchObjectClassDesc PatchObjectClassDesc(&DllPluginDescBuiltin);

void CPatchObject::parse(uint16 version, uint filter)
{
	CGeomObject::parse(version);
}

void CPatchObject::clean()
{
	CGeomObject::clean();
}

void CPatchObject::build(uint16 version, uint filter)
{
	CGeomObject::build(version);
}

void CPatchObject::disown()
{
	CGeomObject::disown();
}

void CPatchObject::init()
{
	CGeomObject::init();
}

bool CPatchObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CGeomObject::inherits(classId);
}

const ISceneClassDesc *CPatchObject::classDesc() const
{
	return &PatchObjectClassDesc;
}

void CPatchObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CGeomObject::toStringLocal(ostream, pad);
}

IStorageObject *CPatchObject::createChunkById(uint16 id, bool container)
{
	return CGeomObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
