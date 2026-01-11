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

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassUnknownDllPluginDesc::CSceneClassUnknownDllPluginDesc(const ucstring &dllFilename, const ucstring &dllDescription) : m_InternalName(dllFilename), m_DisplayName(dllDescription)
{

}

const ucchar *CSceneClassUnknownDllPluginDesc::internalName() const
{
	return m_InternalName.c_str();
}

const ucchar *CSceneClassUnknownDllPluginDesc::displayName() const
{
	return m_DisplayName.c_str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClassUnknownDesc::CSceneClassUnknownDesc(const NLMISC::CClassId classId, const TSClassId superClassId, const ucstring &displayName, const std::string &internalName, const ucstring &dllFilename, const ucstring &dllDescription) : m_DisplayName(displayName), m_InternalName(internalName), m_ClassId(classId), m_SuperClassId(superClassId), m_DllPluginDesc(dllFilename, dllDescription)
{

}

CSceneClass *CSceneClassUnknownDesc::create(CScene *scene) const
{
	nlassert(false);
}

void CSceneClassUnknownDesc::destroy(CSceneClass *sc) const
{
	delete sc;
}

const ucchar *CSceneClassUnknownDesc::displayName() const
{
	return m_DisplayName.c_str();
}

const char *CSceneClassUnknownDesc::internalName() const
{
	return m_InternalName.c_str();
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

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
