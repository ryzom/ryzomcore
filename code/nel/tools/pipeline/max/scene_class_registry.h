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

	void add(const NLMISC::CClassId, const ISceneClassDesc *desc);
	void remove(const NLMISC::CClassId);
	CSceneClass *create(const NLMISC::CClassId classid) const;
	void destroy(CSceneClass *sceneClass) const;
	const ISceneClassDesc *describe(const NLMISC::CClassId classid) const;

}; /* class CSceneClassRegistry */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_REGISTRY_H */

/* end of file */
