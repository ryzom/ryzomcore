/**
 * \file scene_class.cpp
 * \brief CSceneClass
 * \date 2012-08-20 09:07GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClass
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
#include "scene_class.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClass::CSceneClass()
{

}

CSceneClass::~CSceneClass()
{

}

std::string CSceneClass::getClassName()
{
	return getClassDesc()->internalName();
}

void CSceneClass::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CSceneClass::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CSceneClass::clean()
{
	CStorageContainer::clean();
}

void CSceneClass::build(uint16 version)
{
	CStorageContainer::build(version);
}

void CSceneClass::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CSceneClass::createChunkById(uint16 id, bool container)
{
	return CStorageContainer::createChunkById(id, container);
}

const ucchar *CSceneClass::DisplayName = ucstring("Scene Class").c_str();
const char *CSceneClass::InternalName = "SceneClass";
const NLMISC::CClassId CSceneClass::ClassId = NLMISC::CClassId::Null;
const TSClassId CSceneClass::SuperClassId = 0x0000;

namespace {
static const CSceneClassDesc<CSceneClass> SceneClassDesc;
} /* anonymous namespace */

const ISceneClassDesc *CSceneClass::getClassDesc()
{
	return static_cast<const ISceneClassDesc *>(&SceneClassDesc);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
