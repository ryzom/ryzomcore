/**
 * \file scene.cpp
 * \brief CScene
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CScene
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
#include "scene.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/ucstring.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CScene::CScene()
{

}

CScene::~CScene()
{

}

std::string CScene::getClassName()
{
	return "Scene";
}

void CScene::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CScene::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		// Return the scene class container. There can be only one.
		return new CSceneClassContainer();
	}
	return CStorageContainer::createChunkById(id, container);
}

void CScene::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassContainer::CSceneClassContainer()
{

}

CSceneClassContainer::~CSceneClassContainer()
{

}

std::string CSceneClassContainer::getClassName()
{
	return "SceneClassContainer";
}

void CSceneClassContainer::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CSceneClassContainer::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		// TODO: Check the class registry.
		// Return default unknown scene class.
		return new CSceneClass();
	}
	return CStorageContainer::createChunkById(id, container);
}

void CSceneClassContainer::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

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
	return getClassDesc()->getInternalName();
}

void CSceneClass::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

IStorageObject *CSceneClass::createChunkById(uint16 id, bool container)
{
	return CStorageContainer::createChunkById(id, container);
}

void CSceneClass::serialized(TStorageObjectContainer::iterator soit, bool container)
{
	CStorageContainer::serialized(soit, container);
}

const ucchar *CSceneClass::DisplayName = ucstring("Unknown Scene Class").c_str();
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
