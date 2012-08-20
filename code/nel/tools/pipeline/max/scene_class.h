/**
 * \file scene_class.h
 * \brief CSceneClass
 * \date 2012-08-20 09:07GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClass
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

#ifndef PIPELINE_SCENE_CLASS_H
#define PIPELINE_SCENE_CLASS_H
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
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

	// static const
	static const ucchar *DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// virtual
	virtual const ISceneClassDesc *getClassDesc();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

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
	virtual const ucchar *displayName() const = 0;
	virtual const char *internalName() const = 0;
	virtual NLMISC::CClassId classId() const = 0;
	virtual TSClassId superClassId() const = 0;

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
	virtual const ucchar *displayName() const { return T::DisplayName; }
	virtual const char *internalName() const { return T::InternalName; }
	virtual NLMISC::CClassId classId() const { return T::ClassId; }
	virtual TSClassId superClassId() const { return T::SuperClassId; }

}; /* class CSceneClassDesc */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_H */

/* end of file */
