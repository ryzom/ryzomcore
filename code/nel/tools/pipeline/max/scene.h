/**
 * \file scene.h
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

#ifndef PIPELINE_SCENE_H
#define PIPELINE_SCENE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>

// Project includes
#include "storage_object.h"
#include "storage_value.h"

namespace PIPELINE {
namespace MAX {

// Don't really care about superclass IDs right now, but we have to.
typedef uint32 TSClassId;

/**
 * \brief CScene
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CScene
 */
class CScene : public CStorageContainer
{
public:
	CScene();
	virtual ~CScene();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	virtual void serialized(TStorageObjectContainer::iterator soit, bool container);

}; /* class CScene */

/**
 * \brief CSceneClassContainer
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassContainer
 */
class CSceneClassContainer : public CStorageContainer
{
public:
	CSceneClassContainer();
	virtual ~CSceneClassContainer();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	virtual void serialized(TStorageObjectContainer::iterator soit, bool container);

}; /* class CSceneClassContainer */

class ISceneClassDesc;

/**
 * \brief CSceneClass
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClass
 */
class CSceneClass : public CStorageContainer
{
public:
	CSceneClass();
	virtual ~CSceneClass();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");

	// static const
	static const ucchar *DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// virtual
	virtual const ISceneClassDesc *getClassDesc();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	virtual void serialized(TStorageObjectContainer::iterator soit, bool container);

}; /* class CSceneClass */

/**
 * \brief ISceneClassDesc
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * ISceneClassDesc
 */
class ISceneClassDesc
{
public:
	virtual CSceneClass *create() const = 0;
	virtual void destroy(CSceneClass *sc) const = 0;
	virtual const ucchar *getDisplayName() const = 0;
	virtual const char *getInternalName() const = 0;
	virtual NLMISC::CClassId getClassId() const = 0;
	virtual TSClassId getSuperClassId() const = 0;

}; /* class ISceneClassDesc */

/**
 * \brief CSceneClassDesc
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassDesc
 * Use in a cpp when registering the CClassId.
 */
template <typename T>
class CSceneClassDesc : public ISceneClassDesc
{
public:
	virtual CSceneClass *create() const { return static_cast<CSceneClass *>(new T()); }
	virtual void destroy(CSceneClass *sc) const { delete static_cast<T *>(sc); }
	virtual const ucchar *getDisplayName() const { return T::DisplayName; }
	virtual const char *getInternalName() const { return T::InternalName; }
	virtual NLMISC::CClassId getClassId() const { return T::ClassId; }
	virtual TSClassId getSuperClassId() const { return T::SuperClassId; }

}; /* class CSceneClassDesc */

/**
 * \brief CSceneClassRegistry
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassRegistry
 */
class CSceneClassRegistry
{
public:
	void add(const NLMISC::CClassId, const ISceneClassDesc *desc);
	void remove(const NLMISC::CClassId);
	CSceneClass *create(const NLMISC::CClassId classid) const;
	void destroy(CSceneClass *sceneClass) const;
	const ISceneClassDesc *describe(const NLMISC::CClassId classid) const;

}; /* class ISceneClassConstructor */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_H */

/* end of file */
