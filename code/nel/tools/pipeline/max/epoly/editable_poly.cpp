/**
 * \file editable_poly.cpp
 * \brief CEditablePoly
 * \date 2012-08-26 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CEditablePoly
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
#include "editable_poly.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "epoly.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace EPOLY {

CEditablePoly::CEditablePoly(CScene *scene) : CPolyObject(scene)
{

}

CEditablePoly::~CEditablePoly()
{

}

const ucstring CEditablePoly::DisplayName = ucstring("EditablePoly");
const char *CEditablePoly::InternalName = "EditablePoly";
const NLMISC::CClassId CEditablePoly::ClassId = NLMISC::CClassId(0x1bf8338d, 0x192f6098);
const TSClassId CEditablePoly::SuperClassId = CPolyObject::SuperClassId;
const CEditablePolyClassDesc EditablePolyClassDesc(&DllPluginDescEPoly);

void CEditablePoly::parse(uint16 version)
{
	CPolyObject::parse(version);
}

void CEditablePoly::clean()
{
	CPolyObject::clean();
}

void CEditablePoly::build(uint16 version)
{
	CPolyObject::build(version);
}

void CEditablePoly::disown()
{
	CPolyObject::disown();
}

void CEditablePoly::init()
{
	CPolyObject::init();
}

bool CEditablePoly::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CPolyObject::inherits(classId);
}

const ISceneClassDesc *CEditablePoly::classDesc() const
{
	return &EditablePolyClassDesc;
}

void CEditablePoly::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CPolyObject::toStringLocal(ostream, pad);
}

IStorageObject *CEditablePoly::createChunkById(uint16 id, bool container)
{
	return CPolyObject::createChunkById(id, container);
}

} /* namespace EPOLY */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
