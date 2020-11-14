/**
 * \file reference_target.cpp
 * \brief CReferenceTarget
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CReferenceTarget
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
#include "reference_target.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CReferenceTarget::CReferenceTarget(CScene *scene) : CReferenceMaker(scene)
{

}

CReferenceTarget::~CReferenceTarget()
{

}

const ucstring CReferenceTarget::DisplayName = ucstring("ReferenceTarget");
const char *CReferenceTarget::InternalName = "ReferenceTarget";
const char *CReferenceTarget::InternalNameUnknown = "ReferenceTargetUnknown";
const NLMISC::CClassId CReferenceTarget::ClassId = NLMISC::CClassId(0x5d545dd9, 0xa422e4); /* Not official, please correct */
const TSClassId CReferenceTarget::SuperClassId = 0x00000200;
const CReferenceTargetClassDesc ReferenceTargetClassDesc(&DllPluginDescBuiltin);
const CReferenceTargetSuperClassDesc ReferenceTargetSuperClassDesc(&ReferenceTargetClassDesc);

void CReferenceTarget::parse(uint16 version, uint filter)
{
	CReferenceMaker::parse(version);
}

void CReferenceTarget::clean()
{
	CReferenceMaker::clean();
}

void CReferenceTarget::build(uint16 version, uint filter)
{
	CReferenceMaker::build(version);
}

void CReferenceTarget::disown()
{
	CReferenceMaker::disown();
}

void CReferenceTarget::init()
{
	CReferenceMaker::init();
}

bool CReferenceTarget::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceMaker::inherits(classId);
}

const ISceneClassDesc *CReferenceTarget::classDesc() const
{
	return &ReferenceTargetClassDesc;
}

void CReferenceTarget::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceMaker::toStringLocal(ostream, pad);
}

IStorageObject *CReferenceTarget::createChunkById(uint16 id, bool container)
{
	return CReferenceMaker::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
