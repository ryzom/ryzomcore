/**
 * \file scene_class_unknown.cpp
 * \brief CSceneClassUnknown
 * \date 2012-08-20 13:23GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassUnknown
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
#include "scene_class_unknown.h"

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

CSceneClassUnknownDllPluginDesc::CSceneClassUnknownDllPluginDesc(const CDllEntry *dllEntry) : m_DisplayName(dllEntry->dllDescription()), m_InternalName(dllEntry->dllFilename())
{

}

const ucchar *CSceneClassUnknownDllPluginDesc::displayName() const
{
	return m_DisplayName.c_str();
}

const ucchar *CSceneClassUnknownDllPluginDesc::internalName() const
{
	return m_InternalName.c_str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassUnknownDesc::CSceneClassUnknownDesc(const CDllEntry *dllEntry, const CClassEntry *classEntry) : m_DisplayName(classEntry->displayName()), m_ClassId(classEntry->classId()), m_SuperClassId(classEntry->superClassId()), m_DllPluginDesc(dllEntry)
{

}

CSceneClass *CSceneClassUnknownDesc::create() const
{
	nlassert(false);
}

void CSceneClassUnknownDesc::destroy(CSceneClass *sc) const
{
	nlassert(false);
}

const ucchar *CSceneClassUnknownDesc::displayName() const
{
	return m_DisplayName.c_str();
}

const char *CSceneClassUnknownDesc::internalName() const
{
	return "SceneClassUnknown";
}

NLMISC::CClassId CSceneClassUnknownDesc::classId() const
{
	return m_ClassId;
}

TSClassId CSceneClassUnknownDesc::superClassId() const
{
	return m_SuperClassId;
}

const IDllPluginDescInternal *CSceneClassUnknownDesc::dllPluginDesc() const
{
	return &m_DllPluginDesc;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassUnknown::CSceneClassUnknown(const CDllEntry *dllEntry, const CClassEntry *classEntry) : m_Desc(dllEntry, classEntry)
{

}

CSceneClassUnknown::~CSceneClassUnknown()
{

}

void CSceneClassUnknown::toString(std::ostream &ostream, const std::string &pad)
{
	CSceneClass::toString(ostream, pad);
}

void CSceneClassUnknown::parse(uint16 version, TParseLevel level)
{
	CSceneClass::parse(version, level);
}

void CSceneClassUnknown::clean()
{
	CSceneClass::clean();
}

void CSceneClassUnknown::build(uint16 version)
{
	CSceneClass::build(version);
}

void CSceneClassUnknown::disown()
{
	CSceneClass::disown();
}

IStorageObject *CSceneClassUnknown::createChunkById(uint16 id, bool container)
{
	return CSceneClass::createChunkById(id, container);
}

const ISceneClassDesc *CSceneClassUnknown::getClassDesc()
{
	return &m_Desc;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
