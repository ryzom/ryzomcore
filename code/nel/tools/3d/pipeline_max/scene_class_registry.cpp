/**
 * \file scene_class_registry.cpp
 * \brief CSceneClassRegistry
 * \date 2012-08-20 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassRegistry
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
#include "scene_class_registry.h"

// STL includes
#include <iostream>

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

// Elevate warnings to errors in this file for stricter reading
#undef nlwarning
#define nlwarning nlerror

// Elevate debug to error in this file for debugging
// #undef nldebug
// #define nldebug nlerror

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassRegistry::CSceneClassRegistry()
{

}

CSceneClassRegistry::~CSceneClassRegistry()
{

}

void CSceneClassRegistry::add(const ISceneClassDesc *desc)
{
	TKey key(desc->superClassId(), desc->classId());
	if (m_ClassDescriptions.find(key) != m_ClassDescriptions.end()) { nlerror("Already added this class to the registry"); return; }
	m_ClassDescriptions[key] = desc;
}

void CSceneClassRegistry::remove(const TSClassId superClassId, const NLMISC::CClassId classId)
{
	TKey key(superClassId, classId);
	if (m_ClassDescriptions.find(key) == m_ClassDescriptions.end()) { nlwarning("Try to remove class that is not found"); return; }
	m_ClassDescriptions.erase(key);
}

/// Add a superclass to the registry
void CSceneClassRegistry::add(const ISuperClassDesc *desc)
{
	// nldebug("Register superclass 0x%x", desc->superClassId());
	if (m_SuperClassDescriptions.find(desc->superClassId()) != m_SuperClassDescriptions.end()) { nlerror("Already added this superclass to the registry"); return; }
	m_SuperClassDescriptions[desc->superClassId()] = desc;
}

/// Remove a superclass from the registry
void CSceneClassRegistry::remove(const TSClassId superClassId)
{
	if (m_SuperClassDescriptions.find(superClassId) == m_SuperClassDescriptions.end()) { nlwarning("Try to remove superclass that is not found"); return; }
	m_SuperClassDescriptions.erase(superClassId);
}

/// Create a class by class id
CSceneClass *CSceneClassRegistry::create(CScene *scene, const TSClassId superClassId, const NLMISC::CClassId classId) const
{
	TKey key(superClassId, classId);
	if (m_ClassDescriptions.find(key) == m_ClassDescriptions.end()) { /* nldebug("Try to create class that does not exist"); */ return NULL; }
	return m_ClassDescriptions.find(key)->second->create(scene);
}

/// Create an unknown class by superclass id
CSceneClass *CSceneClassRegistry::createUnknown(CScene *scene, TSClassId superClassId, const NLMISC::CClassId classId, const ucstring &displayName, const ucstring &dllFilename, const ucstring &dllDescription) const
{
	if (m_SuperClassDescriptions.find(superClassId) == m_SuperClassDescriptions.end()) { nlwarning("Creating superclass 0x%x (%s) %s that does not exist", superClassId, displayName.toUtf8().c_str(), classId.toString().c_str()); return NULL; }
	return m_SuperClassDescriptions.find(superClassId)->second->createUnknown(scene, classId, displayName, dllFilename, dllDescription);
}

/// Destroy a class by pointer
void CSceneClassRegistry::destroy(CSceneClass *sceneClass) const
{
	sceneClass->classDesc()->destroy(sceneClass);
}

/// Return the description of a class by class id
const ISceneClassDesc *CSceneClassRegistry::describe(const TSClassId superClassId, const NLMISC::CClassId classId) const
{
	TKey key(superClassId, classId);
	if (m_ClassDescriptions.find(key) == m_ClassDescriptions.end()) { nldebug("Try to describe class that does not exist"); return NULL; }
	return m_ClassDescriptions.find(key)->second;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassRegistry::TKey::TKey(TSClassId superClassId, NLMISC::CClassId classId) : SuperClassId(superClassId), ClassId(classId)
{

}

bool CSceneClassRegistry::TKey::operator<(const CSceneClassRegistry::TKey &right) const
{
	if (SuperClassId < right.SuperClassId)
		return true;
	if (SuperClassId > right.SuperClassId)
		return false;
	if (ClassId < right.ClassId)
		return true;
	if (ClassId > right.ClassId)
		return false;
	return false;
}

bool CSceneClassRegistry::TKey::operator>(const CSceneClassRegistry::TKey &right) const
{
	if (SuperClassId > right.SuperClassId)
		return true;
	if (SuperClassId < right.SuperClassId)
		return false;
	if (ClassId > right.ClassId)
		return true;
	if (ClassId < right.ClassId)
		return false;
	return false;
}

bool CSceneClassRegistry::TKey::operator==(const CSceneClassRegistry::TKey &right) const
{
	return ClassId == right.ClassId && SuperClassId == right.SuperClassId;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
