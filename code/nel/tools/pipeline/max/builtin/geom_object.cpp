/**
 * \file geom_object.cpp
 * \brief CGeomGeomObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CGeomGeomObject
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
#include "geom_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "storage/geom_buffers.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

#define PMB_GEOM_BUFFERS_CHUNK_ID 0x08fe

CGeomObject::CGeomObject(CScene *scene) : CObject(scene)
{

}

CGeomObject::~CGeomObject()
{

}

const ucstring CGeomObject::DisplayName = ucstring("GeomObject");
const char *CGeomObject::InternalName = "GeomObject";
const char *CGeomObject::InternalNameUnknown = "GeomObjectUnknown";
const NLMISC::CClassId CGeomObject::ClassId = NLMISC::CClassId(0x37097c44, 0x38aa3f24); /* Not official, please correct */
const TSClassId CGeomObject::SuperClassId = 0x00000010;
const CGeomObjectClassDesc GeomObjectClassDesc(&DllPluginDescBuiltin);
const CGeomObjectSuperClassDesc GeomObjectSuperClassDesc(&GeomObjectClassDesc);

void CGeomObject::parse(uint16 version)
{
	CObject::parse(version);
}

void CGeomObject::clean()
{
	CObject::clean();
}

void CGeomObject::build(uint16 version)
{
	CObject::build(version);
}

void CGeomObject::disown()
{
	CObject::disown();
}

void CGeomObject::init()
{
	CObject::init();
}

bool CGeomObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CObject::inherits(classId);
}

const ISceneClassDesc *CGeomObject::classDesc() const
{
	return &GeomObjectClassDesc;
}

void CGeomObject::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CObject::toStringLocal(ostream, pad);
}

IStorageObject *CGeomObject::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_GEOM_BUFFERS_CHUNK_ID:
		return new STORAGE::CGeomBuffers();
	}
	return CObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
