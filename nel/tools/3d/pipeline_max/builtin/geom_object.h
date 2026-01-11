/**
 * \file geom_object.h
 * \brief CGeomGeomObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CGeomGeomObject
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

#ifndef PIPELINE_GEOM_OBJECT_H
#define PIPELINE_GEOM_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "object.h"
#include "storage/geom_buffers.h"

/// Must be passed to the parse and build functions by
/// inheriting classes to parse the actual geom object.
#define PMB_GEOM_OBJECT_PARSE_FILTER 0x432a4da6

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CGeomGeomObject
 * \date 2012-08-22 08:58GMT
 * \author Jan Boon (Kaetemi)
 * CGeomGeomObject
 */
class CGeomObject : public CObject
{
public:
	CGeomObject(CScene *scene);
	virtual ~CGeomObject();

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

	static void triangulatePolyFace(std::vector<STORAGE::CGeomTriIndex> &triangles, const STORAGE::CGeomPolyFaceInfo &polyFace);

	// read access
	inline STORAGE::CGeomBuffers *geomBuffers() const { return m_GeomBuffers; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	IStorageObject *m_Unknown0900;
	STORAGE::CGeomBuffers *m_GeomBuffers;

}; /* class CGeomObject */

typedef CSceneClassDesc<CGeomObject> CGeomObjectClassDesc;
extern const CGeomObjectClassDesc GeomObjectClassDesc;
typedef CSuperClassDesc<CGeomObject> CGeomObjectSuperClassDesc;
extern const CGeomObjectSuperClassDesc GeomObjectSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_GEOM_OBJECT_H */

/* end of file */
