/**
 * \file poly_object.cpp
 * \brief CPolyObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CPolyObject
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
#include "poly_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CPolyObject::CPolyObject(CScene *scene) : CGeomObject(scene)
{

}

CPolyObject::~CPolyObject()
{

}

const ucstring CPolyObject::DisplayName = ucstring("PolyObject");
const char *CPolyObject::InternalName = "PolyObject";
const NLMISC::CClassId CPolyObject::ClassId = NLMISC::CClassId(0x59772461, 0x6e1141e8); /* Not official, please correct */
const TSClassId CPolyObject::SuperClassId = CGeomObject::SuperClassId;
const CPolyObjectClassDesc PolyObjectClassDesc(&DllPluginDescBuiltin);

void CPolyObject::parse(uint16 version)
{
	CGeomObject::parse(version);
}

void CPolyObject::clean()
{
	CGeomObject::clean();
}

void CPolyObject::build(uint16 version)
{
	CGeomObject::build(version);
}

void CPolyObject::disown()
{
	CGeomObject::disown();
}

void CPolyObject::init()
{
	CGeomObject::init();
}

bool CPolyObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CGeomObject::inherits(classId);
}

const ISceneClassDesc *CPolyObject::classDesc() const
{
	return &PolyObjectClassDesc;
}

void CPolyObject::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CGeomObject::toStringLocal(ostream, pad);
}

IStorageObject *CPolyObject::createChunkById(uint16 id, bool container)
{
	return CGeomObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
