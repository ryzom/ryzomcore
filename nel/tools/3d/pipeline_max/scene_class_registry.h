/**
 * \file scene_class_registry.h
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

#ifndef PIPELINE_SCENE_CLASS_REGISTRY_H
#define PIPELINE_SCENE_CLASS_REGISTRY_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "scene_class.h"
#include "super_class_desc.h"

namespace PIPELINE {
namespace MAX {

/**
 * \brief CSceneClassRegistry
 * \date 2012-08-20 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassRegistry
 */
class CSceneClassRegistry
{
public:
	CSceneClassRegistry();
	virtual ~CSceneClassRegistry();

	/// Add a class to the registry
	void add(const ISceneClassDesc *desc);
	/// Remove a class from the registry
	void remove(TSClassId superClassId, const NLMISC::CClassId classId);

	/// Add a superclass to the registry
	void add(const ISuperClassDesc *desc);
	/// Remove a superclass from the registry
	void remove(const TSClassId superClassId);

	/// Create a class by class id
	CSceneClass *create(CScene *scene, TSClassId superClassId, const NLMISC::CClassId classId) const;

	/// Create an unknown class by superclass id
	CSceneClass *createUnknown(CScene *scene, TSClassId superClassId, const NLMISC::CClassId classId, const ucstring &displayName, const ucstring &dllFilename, const ucstring &dllDescription) const;

	/// Destroy a class by pointer
	void destroy(CSceneClass *sceneClass) const;

	/// Return the description of a class by class id
	const ISceneClassDesc *describe(TSClassId superClassId, const NLMISC::CClassId classId) const;

private:
	struct TKey
	{
		TKey(TSClassId superClassId, NLMISC::CClassId classId);
		TSClassId SuperClassId;
		NLMISC::CClassId ClassId;
		bool operator<(const TKey &right) const;
		bool operator>(const TKey &right) const;
		bool operator==(const TKey &right) const;
	};

	std::map<TKey, const ISceneClassDesc *> m_ClassDescriptions;
	std::map<TSClassId, const ISuperClassDesc *> m_SuperClassDescriptions;

}; /* class CSceneClassRegistry */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_REGISTRY_H */

/* end of file */
