/**
 * \file editable_mesh.cpp
 * \brief CEditableMesh
 * \date 2012-08-26 12:11GMT
 * \author Jan Boon (Kaetemi)
 * CEditableMesh
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
#include "editable_mesh.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "update1.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace UPDATE1 {

CEditableMesh::CEditableMesh(CScene *scene) : CTriObject(scene)
{

}

CEditableMesh::~CEditableMesh()
{

}

const ucstring CEditableMesh::DisplayName = ucstring("EditableMesh");
const char *CEditableMesh::InternalName = "EditableMesh";
const NLMISC::CClassId CEditableMesh::ClassId = NLMISC::CClassId(0xe44f10b3, 0x00000000);
const TSClassId CEditableMesh::SuperClassId = CTriObject::SuperClassId;
const CEditableMeshClassDesc EditableMeshClassDesc(&DllPluginDescUpdate1);

void CEditableMesh::parse(uint16 version)
{
	CTriObject::parse(version);
}

void CEditableMesh::clean()
{
	CTriObject::clean();
}

void CEditableMesh::build(uint16 version)
{
	CTriObject::build(version);
}

void CEditableMesh::disown()
{
	CTriObject::disown();
}

void CEditableMesh::init()
{
	CTriObject::init();
}

bool CEditableMesh::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CTriObject::inherits(classId);
}

const ISceneClassDesc *CEditableMesh::classDesc() const
{
	return &EditableMeshClassDesc;
}

void CEditableMesh::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CTriObject::toStringLocal(ostream, pad);
}

IStorageObject *CEditableMesh::createChunkById(uint16 id, bool container)
{
	return CTriObject::createChunkById(id, container);
}

} /* namespace UPDATE1 */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
