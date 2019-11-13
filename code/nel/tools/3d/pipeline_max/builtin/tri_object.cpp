/**
 * \file tri_object.cpp
 * \brief CTriObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CTriObject
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
#include "tri_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CTriObject::CTriObject(CScene *scene) : CGeomObject(scene)
{

}

CTriObject::~CTriObject()
{

}

const ucstring CTriObject::DisplayName = ucstring("TriObject");
const char *CTriObject::InternalName = "TriObject";
const NLMISC::CClassId CTriObject::ClassId = NLMISC::CClassId(0x4553fa6, 0x30f8421e); /* Not official, please correct */
const TSClassId CTriObject::SuperClassId = CGeomObject::SuperClassId;
const CTriObjectClassDesc TriObjectClassDesc(&DllPluginDescBuiltin);

void CTriObject::parse(uint16 version, uint filter)
{
	if (filter == 0)
	{
		CGeomObject::parse(version);
	}
	else if (filter == PMB_TRI_OBJECT_PARSE_FILTER)
	{
		if (!m_ChunksOwnsPointers)
		{
			CGeomObject::parse(version, PMB_GEOM_OBJECT_PARSE_FILTER);
			// 0x0901
			// 0x0902
			// 0x0904
			// 0x0903
		}
	}
}

void CTriObject::clean()
{
	CGeomObject::clean();
}

void CTriObject::build(uint16 version, uint filter)
{
	if (filter == 0)
	{
		CGeomObject::build(version);
	}
	else if (filter == PMB_TRI_OBJECT_PARSE_FILTER)
	{
		CGeomObject::build(version, PMB_GEOM_OBJECT_PARSE_FILTER);
		// 0x0901
		// 0x0902
		// 0x0904
		// 0x0903
	}
}

void CTriObject::disown()
{
	CGeomObject::disown();
}

void CTriObject::init()
{
	CGeomObject::init();
}

bool CTriObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CGeomObject::inherits(classId);
}

const ISceneClassDesc *CTriObject::classDesc() const
{
	return &TriObjectClassDesc;
}

void CTriObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	if (filter == 0)
	{
		CGeomObject::toStringLocal(ostream, pad);
	}
	else if (filter == PMB_TRI_OBJECT_PARSE_FILTER)
	{
		CGeomObject::toStringLocal(ostream, pad, PMB_GEOM_OBJECT_PARSE_FILTER);
		// 0x0901
		// 0x0902
		// 0x0904
		// 0x0903
	}
}

IStorageObject *CTriObject::createChunkById(uint16 id, bool container)
{
	return CGeomObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
