/**
 * \file object.h
 * \brief CObject
 * \date 2012-08-22 09:13GMT
 * \author Jan Boon (Kaetemi)
 * CObject
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

#ifndef PIPELINE_OBJECT_H
#define PIPELINE_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "base_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CObject
 * \date 2012-08-22 09:13GMT
 * \author Jan Boon (Kaetemi)
 * CObject
 */
class CObject : public CBaseObject
{
public:
	CObject(CScene *scene);
	virtual ~CObject();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();
	virtual void init();
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CObject */

typedef CSceneClassDesc<CObject> CObjectClassDesc;
extern const CObjectClassDesc ObjectClassDesc;
typedef CSuperClassDesc<CObject> CObjectSuperClassDesc;
extern const CObjectSuperClassDesc ObjectSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_OBJECT_H */

/* end of file */
