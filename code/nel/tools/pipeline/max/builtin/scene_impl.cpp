/**
 * \file scene_impl.cpp
 * \brief CSceneImpl
 * \date 2012-08-24 12:33GMT
 * \author Jan Boon (Kaetemi)
 * CSceneImpl
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
#include "scene_impl.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CSceneImpl::CSceneImpl(CScene *scene) : CReferenceMaker(scene)
{

}

CSceneImpl::~CSceneImpl()
{

}

const ucstring CSceneImpl::DisplayName = ucstring("Scene");
const char *CSceneImpl::InternalName = "SceneImpl";
const NLMISC::CClassId CSceneImpl::ClassId = NLMISC::CClassId(0x00002222, 0x00000000);
const TSClassId CSceneImpl::SuperClassId = CReferenceMaker::SuperClassId;
const CSceneImplClassDesc SceneImplClassDesc(&DllPluginDescBuiltin);

void CSceneImpl::parse(uint16 version, TParseLevel level)
{
	CReferenceMaker::parse(version, level);
}

void CSceneImpl::clean()
{
	CReferenceMaker::clean();
}

void CSceneImpl::build(uint16 version)
{
	CReferenceMaker::build(version);
}

void CSceneImpl::disown()
{
	CReferenceMaker::disown();
}

void CSceneImpl::init()
{
	CReferenceMaker::init();
}

bool CSceneImpl::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceMaker::inherits(classId);
}

const ISceneClassDesc *CSceneImpl::classDesc() const
{
	return &SceneImplClassDesc;
}

void CSceneImpl::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CReferenceMaker::toStringLocal(ostream, pad);
}

IStorageObject *CSceneImpl::createChunkById(uint16 id, bool container)
{
	return CReferenceMaker::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
