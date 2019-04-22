/**
 * \file base_object.cpp
 * \brief CBaseObject
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CBaseObject
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
#include "base_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CBaseObject::CBaseObject(CScene *scene) : CReferenceTarget(scene)
{

}

CBaseObject::~CBaseObject()
{

}

const ucstring CBaseObject::DisplayName = ucstring("BaseObject");
const char *CBaseObject::InternalName = "BaseObject";
// const char *CBaseObject::InternalNameUnknown = "BaseObjectUnknown";
const NLMISC::CClassId CBaseObject::ClassId = NLMISC::CClassId(0x71c8167a, 0x5a9f57b2); /* Not official, please correct */
const TSClassId CBaseObject::SuperClassId = CReferenceTarget::SuperClassId;
const CBaseObjectClassDesc BaseObjectClassDesc(&DllPluginDescBuiltin);
// const CBaseObjectSuperClassDesc BaseObjectSuperClassDesc(&BaseObjectClassDesc);

void CBaseObject::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
}

void CBaseObject::clean()
{
	CReferenceTarget::clean();
}

void CBaseObject::build(uint16 version, uint filter)
{
	CReferenceTarget::build(version);
}

void CBaseObject::disown()
{
	CReferenceTarget::disown();
}

void CBaseObject::init()
{
	CReferenceTarget::init();
}

bool CBaseObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CBaseObject::classDesc() const
{
	return &BaseObjectClassDesc;
}

void CBaseObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
}

IStorageObject *CBaseObject::createChunkById(uint16 id, bool container)
{
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
