/**
 * \file object.cpp
 * \brief CObject
 * \date 2012-08-22 09:13GMT
 * \author Jan Boon (Kaetemi)
 * CObject
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
#include "object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CObject::CObject(CScene *scene) : CBaseObject(scene)
{

}

CObject::~CObject()
{

}

const ucstring CObject::DisplayName = ucstring("Object");
const char *CObject::InternalName = "Object";
const char *CObject::InternalNameUnknown = "ObjectUnknown";
const NLMISC::CClassId CObject::ClassId = NLMISC::CClassId(0x14021ed1, 0x33837a80); /* Not official, please correct */
const TSClassId CObject::SuperClassId = 0x00000060;
const CObjectClassDesc ObjectClassDesc(&DllPluginDescBuiltin);
const CObjectSuperClassDesc ObjectSuperClassDesc(&ObjectClassDesc);

void CObject::parse(uint16 version, uint filter)
{
	CBaseObject::parse(version);
}

void CObject::clean()
{
	CBaseObject::clean();
}

void CObject::build(uint16 version, uint filter)
{
	CBaseObject::build(version);
}

void CObject::disown()
{
	CBaseObject::disown();
}

void CObject::init()
{
	CBaseObject::init();
}

bool CObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CBaseObject::inherits(classId);
}

const ISceneClassDesc *CObject::classDesc() const
{
	return &ObjectClassDesc;
}

void CObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CBaseObject::toStringLocal(ostream, pad);
}

IStorageObject *CObject::createChunkById(uint16 id, bool container)
{
	return CBaseObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
