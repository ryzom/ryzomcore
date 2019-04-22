/**
 * \file editable_patch.cpp
 * \brief CEditablePatch
 * \date 2012-08-26 12:12GMT
 * \author Jan Boon (Kaetemi)
 * CEditablePatch
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
#include "editable_patch.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CEditablePatch::CEditablePatch(CScene *scene) : CPatchObject(scene)
{

}

CEditablePatch::~CEditablePatch()
{

}

const ucstring CEditablePatch::DisplayName = ucstring("EditablePatch");
const char *CEditablePatch::InternalName = "EditablePatch";
const NLMISC::CClassId CEditablePatch::ClassId = NLMISC::CClassId(0x00001030, 0x00000000);
const TSClassId CEditablePatch::SuperClassId = CPatchObject::SuperClassId;
const CEditablePatchClassDesc EditablePatchClassDesc(&DllPluginDescBuiltin);

void CEditablePatch::parse(uint16 version, uint filter)
{
	CPatchObject::parse(version);
}

void CEditablePatch::clean()
{
	CPatchObject::clean();
}

void CEditablePatch::build(uint16 version, uint filter)
{
	CPatchObject::build(version);
}

void CEditablePatch::disown()
{
	CPatchObject::disown();
}

void CEditablePatch::init()
{
	CPatchObject::init();
}

bool CEditablePatch::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CPatchObject::inherits(classId);
}

const ISceneClassDesc *CEditablePatch::classDesc() const
{
	return &EditablePatchClassDesc;
}

void CEditablePatch::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CPatchObject::toStringLocal(ostream, pad);
}

IStorageObject *CEditablePatch::createChunkById(uint16 id, bool container)
{
	return CPatchObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
