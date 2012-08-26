/**
 * \file scene_class_unknown.h
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

#ifndef PIPELINE_SCENE_CLASS_UNKNOWN_H
#define PIPELINE_SCENE_CLASS_UNKNOWN_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "scene_class.h"

namespace PIPELINE {
namespace MAX {

/**
 * \brief CSceneClassUnknownDllPluginDesc
 * \date 2012-08-20 13:23GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassUnknownDllPluginDesc
 */
class CSceneClassUnknownDllPluginDesc : public IDllPluginDescInternal
{
public:
	CSceneClassUnknownDllPluginDesc(const ucstring &dllFilename, const ucstring &dllDescription);
	virtual const ucchar *internalName() const;
	virtual const ucchar *displayName() const;

private:
	ucstring m_InternalName;
	ucstring m_DisplayName;

};

/**
 * \brief CSceneClassUnknownDesc
 * \date 2012-08-20 13:23GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassUnknownDesc
 */
class CSceneClassUnknownDesc : public ISceneClassDesc
{
public:
	CSceneClassUnknownDesc(const NLMISC::CClassId classId, const TSClassId superClassId, const ucstring &displayName, const std::string &internalName, const ucstring &dllFilename, const ucstring &dllDescription);
	virtual CSceneClass *create(CScene *scene) const;
	virtual void destroy(CSceneClass *sc) const;
	virtual const ucchar *displayName() const;
	virtual const char *internalName() const;
	virtual NLMISC::CClassId classId() const;
	virtual TSClassId superClassId() const;
	virtual const IDllPluginDescInternal *dllPluginDesc() const;

private:
	ucstring m_DisplayName;
	std::string m_InternalName;
	NLMISC::CClassId m_ClassId;
	TSClassId m_SuperClassId;
	CSceneClassUnknownDllPluginDesc m_DllPluginDesc;

}; /* class ISceneClassDesc */

/**
 * \brief CSceneClassUnknown
 * \date 2012-08-20 13:23GMT
 * \author Jan Boon (Kaetemi)
 * Utility class for parsing unknown scene classes and keeping them
 * in storage as is.
 */
template <typename TSuperClass>
class CSceneClassUnknown : public TSuperClass
{
public:
	CSceneClassUnknown(CScene *scene, const NLMISC::CClassId classId, const TSClassId superClassId, const ucstring &displayName, const std::string &internalName, const ucstring &dllFilename, const ucstring &dllDescription) : TSuperClass(scene), m_Desc(classId, superClassId, displayName, internalName, dllFilename, dllDescription) { }
	virtual ~CSceneClassUnknown() { }

	// inherited
	virtual const ISceneClassDesc *classDesc() const { return &m_Desc; }

private:
	CSceneClassUnknownDesc m_Desc;

}; /* class CSceneClass */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_UNKNOWN_H */

/* end of file */
