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
#include "dll_directory.h"
#include "class_directory_3.h"
#include "scene_class_registry.h"
#include "scene_class_unknown.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CScene::CScene(const CSceneClassRegistry *sceneClassRegistry, CDllDirectory *dllDirectory, CClassDirectory3 *classDirectory3) : m_SceneClassRegistry(sceneClassRegistry), m_DllDirectory(dllDirectory), m_ClassDirectory3(classDirectory3)
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

void CScene::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
	nlassert(m_Chunks.size() == 1);
}

void CScene::clean()
{
	CStorageContainer::clean();
}

void CScene::build(uint16 version)
{
	nlassert(m_Chunks.size() == 1);
	CStorageContainer::build(this->version());
}

void CScene::disown()
{
	CStorageContainer::disown();
}

uint16 CScene::version()
{
	nlassert(m_Chunks.size() == 1);
	return m_Chunks.begin()->first;
}

CSceneClassContainer *CScene::container()
{
	nlassert(m_Chunks.size() == 1);
	return static_cast<CSceneClassContainer *>(m_Chunks.begin()->second);
}

IStorageObject *CScene::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		// Return the scene class container. There can be only one.
		return new CSceneClassContainer(m_SceneClassRegistry, m_DllDirectory, m_ClassDirectory3);
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassContainer::CSceneClassContainer(const CSceneClassRegistry *sceneClassRegistry, CDllDirectory *dllDirectory, CClassDirectory3 *classDirectory3) : m_SceneClassRegistry(sceneClassRegistry), m_DllDirectory(dllDirectory), m_ClassDirectory3(classDirectory3)
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

void CSceneClassContainer::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CSceneClassContainer::clean()
{
	CStorageContainer::clean();
}

void CSceneClassContainer::build(uint16 version)
{
	CStorageContainer::build(version);
}

void CSceneClassContainer::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CSceneClassContainer::createChunkById(uint16 id, bool container)
{
	// nldebug("Scene class id %x (%i)", (uint32)id, (uint32)id);
	switch (id)
	{
		// Known unknown special identifiers...
	case 0x2032:
	case 0x2033:
		return new CSceneClass(); // TODO: Make dummy dllentry and classentry for these...
		// return static_cast<IStorageObject *>(new CSceneClassUnknown<CSceneClass>(dllEntry, classEntry));
	}
	const CClassEntry *classEntry = m_ClassDirectory3->get(id);
	CSceneClass *sceneClass = m_SceneClassRegistry->create(classEntry->classId());
	if (sceneClass)
	{
		return static_cast<IStorageObject *>(sceneClass);
	}
	else
	{
		// Create an unknown scene class; TODO: By TSClassId, maybe the registry should have a createUnknown(TSuperClassId)
		const CDllEntry *dllEntry = m_DllDirectory->get(classEntry->dllIndex());
		return static_cast<IStorageObject *>(new CSceneClassUnknown<CSceneClass>(dllEntry, classEntry));
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
