/**
 * \file super_class_desc.h
 * \brief CSuperClassDesc
 * \date 2012-08-22 11:19GMT
 * \author Jan Boon (Kaetemi)
 * CSuperClassDesc
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

#ifndef PIPELINE_SUPER_CLASS_DESC_H
#define PIPELINE_SUPER_CLASS_DESC_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "scene_class_unknown.h"

namespace PIPELINE {
namespace MAX {

/**
 * \brief ISuperClassDesc
 * \date 2012-08-22 09:42GMT
 * \author Jan Boon (Kaetemi)
 * ISuperClassDesc
 */
class ISuperClassDesc
{
public:
	/// Create an unknown class that inherits from this superclass
	virtual CSceneClass *createUnknown(CScene *scene, const NLMISC::CClassId classId, const ucstring &displayName, const ucstring &dllFilename, const ucstring &dllDescription) const = 0;
	/// Get an internal name associated with unknown classes of this superclass
	virtual const char *internalNameUnknown() const = 0;
	/// Gets the associated super class id, may be different from classDesc()->superClassId() for non-implemented superclasses
	virtual TSClassId superClassId() const = 0;
	/// Return the class description that directly implements this superclass
	virtual const ISceneClassDesc *classDesc() const = 0;

}; /* class ISceneClassDesc */

/**
 * \brief CSuperClassDesc
 * \date 2012-08-22 09:42GMT
 * \author Jan Boon (Kaetemi)
 * ISuperClassDesc
 */
template <typename T>
class CSuperClassDesc : public ISuperClassDesc
{
public:
	CSuperClassDesc(const ISceneClassDesc *classDesc) : m_ClassDesc(classDesc) { }
	virtual CSceneClass *createUnknown(CScene *scene, const NLMISC::CClassId classId, const ucstring &displayName, const ucstring &dllFilename, const ucstring &dllDescription) const { return static_cast<CSceneClass *>(new CSceneClassUnknown<T>(scene, classId, m_ClassDesc->superClassId(), displayName,internalNameUnknown(), dllFilename, dllDescription)); }
	virtual const char *internalNameUnknown() const { return T::InternalNameUnknown; }
	virtual TSClassId superClassId() const { return m_ClassDesc->superClassId(); }
	virtual const ISceneClassDesc *classDesc() const { return m_ClassDesc; }
private:
	const ISceneClassDesc *m_ClassDesc;
}; /* class ISceneClassDesc */

/**
 * \brief CSuperClassDescUnknown
 * \date 2012-08-22 09:42GMT
 * \author Jan Boon (Kaetemi)
 * Template for non-implemented superclass descriptions
 */
template <typename T, TSClassId SuperClassId>
class CSuperClassDescUnknown : public ISuperClassDesc
{
public:
	CSuperClassDescUnknown(const ISceneClassDesc *classDesc, const char *internalNameUnknown) : m_ClassDesc(classDesc), m_InternalNameUnknown(internalNameUnknown) { }
	virtual CSceneClass *createUnknown(CScene *scene, const NLMISC::CClassId classId, const ucstring &displayName, const ucstring &dllFilename, const ucstring &dllDescription) const { return static_cast<CSceneClass *>(new CSceneClassUnknown<T>(scene, classId, SuperClassId, displayName,internalNameUnknown(), dllFilename, dllDescription)); }
	virtual const char *internalNameUnknown() const { return m_InternalNameUnknown; }
	virtual TSClassId superClassId() const { return SuperClassId; }
	virtual const ISceneClassDesc *classDesc() const { return m_ClassDesc; }
private:
	const ISceneClassDesc *m_ClassDesc;
	const char *m_InternalNameUnknown;

}; /* class ISceneClassDesc */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SUPER_CLASS_DESC_H */

/* end of file */
