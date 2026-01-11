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
#include <iostream>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/ucstring.h>

// Project includes
#include "dll_directory.h"
#include "class_directory_3.h"
#include "scene_class_registry.h"
#include "scene_class_unknown.h"
#include "builtin/scene_impl.h"

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

std::string CScene::className() const
{
	return "Scene";
}

void CScene::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CScene::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
	nlassert(m_Chunks.size() == 1);
}

void CScene::clean()
{
	CStorageContainer::clean();
}

void CScene::build(uint16 version, uint filter)
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
		return new CSceneClassContainer(this, m_SceneClassRegistry, m_DllDirectory, m_ClassDirectory3);
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassContainer::CSceneClassContainer(CScene *scene, const CSceneClassRegistry *sceneClassRegistry, CDllDirectory *dllDirectory, CClassDirectory3 *classDirectory3) : m_Scene(scene), m_SceneClassRegistry(sceneClassRegistry), m_DllDirectory(dllDirectory), m_ClassDirectory3(classDirectory3), m_BuiltinScene(NULL)
{

}

CSceneClassContainer::~CSceneClassContainer()
{

}

std::string CSceneClassContainer::className() const
{
	return "SceneClassContainer";
}

void CSceneClassContainer::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CSceneClassContainer::parse(uint16 version, uint filter)
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	m_StorageObjectByIndex.resize(m_Chunks.size());
	TStorageObjectIterator it = m_Chunks.begin();
	for (std::vector<CSceneClass *>::size_type i = 0; i < m_StorageObjectByIndex.size(); ++i)
	{
		m_StorageObjectByIndex[i] = static_cast<CSceneClass *>(it->second);
		++it;
	}
	CSceneClass *builtinScene = m_StorageObjectByIndex[m_StorageObjectByIndex.size() - 1];
	nlassert(builtinScene);
	m_BuiltinScene = dynamic_cast<BUILTIN::CSceneImpl *>(builtinScene);
	nlassert(m_BuiltinScene);
	CStorageContainer::parse(version);
}

void CSceneClassContainer::clean()
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	CStorageContainer::clean();
}

void CSceneClassContainer::build(uint16 version, uint filter)
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	for (std::vector<CSceneClass *>::size_type i = 0; i < m_StorageObjectByIndex.size(); ++i)
	{
		m_StorageObjectToIndex[m_StorageObjectByIndex[i]] = i;
	}
	CStorageContainer::build(version);
}

void CSceneClassContainer::disown()
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	CStorageContainer::disown();
}

CSceneClass *CSceneClassContainer::getByStorageIndex(uint32 index) const
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	if (index >= m_StorageObjectByIndex.size()) { nlerror("Index %i is outside size %i", index, m_StorageObjectByIndex.size()); return NULL; }
	return m_StorageObjectByIndex[index];
}

uint32 CSceneClassContainer::getOrCreateStorageIndex(CSceneClass *storageObject)
{
	// Temporary 'readonly' implementation, not modifying m_Chunks!
	return m_StorageObjectToIndex[storageObject];
}

IStorageObject *CSceneClassContainer::createChunkById(uint16 id, bool container)
{
	// nldebug("Scene class id %x (%i)", (uint32)id, (uint32)id);
	switch (id)
	{
		// Known unknown special identifiers...
	case 0x2032:
		return m_SceneClassRegistry->createUnknown(m_Scene, 0x0, NLMISC::CClassId(0x29263a68, 0x405f22f5), ucstring("OSM Derived"), ucstring("Internal"), ucstring("Internal"));
	case 0x2033:
		return m_SceneClassRegistry->createUnknown(m_Scene, 0x0, NLMISC::CClassId(0x4ec13906, 0x5578130e), ucstring("WSM Derived"), ucstring("Internal"), ucstring("Internal"));
		// return new CSceneClass(m_Scene); // TODO: Make dummy dllentry and classentry for these...
		// return static_cast<IStorageObject *>(new CSceneClassUnknown<CSceneClass>(dllEntry, classEntry));
	}
	const CClassEntry *classEntry = m_ClassDirectory3->get(id);
	CSceneClass *sceneClass = m_SceneClassRegistry->create(m_Scene, classEntry->superClassId(), classEntry->classId());
	if (sceneClass)
	{
		return static_cast<IStorageObject *>(sceneClass);
	}
	else
	{
		const CDllEntry *dllEntry = m_DllDirectory->get(classEntry->dllIndex());
		sceneClass = m_SceneClassRegistry->createUnknown(m_Scene, classEntry->superClassId(), classEntry->classId(), classEntry->displayName(), dllEntry->dllFilename(), dllEntry->dllDescription());
		if (sceneClass)
		{
			return static_cast<IStorageObject *>(sceneClass);
		}
		else
		{
			// Create an invalid unknown scene class
			return static_cast<IStorageObject *>(new CSceneClassUnknown<CSceneClass>(m_Scene,classEntry->classId(), classEntry->superClassId(), classEntry->displayName(), "SceneClassUnknown", dllEntry->dllFilename(), dllEntry->dllDescription()));
		}
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
