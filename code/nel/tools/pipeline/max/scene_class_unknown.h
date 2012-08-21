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
#include "dll_directory.h"
#include "class_directory_3.h"

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
	CSceneClassUnknownDllPluginDesc(const CDllEntry *dllEntry);
	virtual const ucchar *displayName() const;
	virtual const ucchar *internalName() const;

private:
	ucstring m_DisplayName;
	ucstring m_InternalName;

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
	CSceneClassUnknownDesc(const CDllEntry *dllEntry, const CClassEntry *classEntry);
	virtual CSceneClass *create() const;
	virtual void destroy(CSceneClass *sc) const;
	virtual const ucchar *displayName() const;
	virtual const char *internalName() const;
	virtual NLMISC::CClassId classId() const;
	virtual TSClassId superClassId() const;
	virtual const IDllPluginDescInternal *dllPluginDesc() const;

private:
	ucstring m_DisplayName;
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
	CSceneClassUnknown(const CDllEntry *dllEntry, const CClassEntry *classEntry) : m_Desc(dllEntry, classEntry) { }
	virtual ~CSceneClassUnknown() { }

	// inherited
	virtual const ISceneClassDesc *getClassDesc() { return &m_Desc; }

private:
	CSceneClassUnknownDesc m_Desc;

}; /* class CSceneClass */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_UNKNOWN_H */

/* end of file */
