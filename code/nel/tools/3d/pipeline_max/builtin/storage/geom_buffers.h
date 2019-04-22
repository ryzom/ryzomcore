/**
 * \file geom_buffers.h
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * CGeomBuffers
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

#ifndef PIPELINE_GEOM_BUFFERS_H
#define PIPELINE_GEOM_BUFFERS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/vector.h>

// Project includes
#include "../../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

struct CGeomTriIndex
{
	uint32 a;
	uint32 b;
	uint32 c;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomTriIndexInfo
{
	uint32 a;
	uint32 b;
	uint32 c;
	uint32 alwaysOne;
	uint32 smoothingGroups;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyVertexInfo
{
	uint32 i1;
	NLMISC::CVector v;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyEdgeInfo
{
	uint32 i1;
	uint32 a;
	uint32 b;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyFaceInfo
{
	CGeomPolyFaceInfo();
	/// Vertex indices in the vertex buffer
	std::vector<uint32> Vertices;
	// Bitfield (implicitly stored)
	/// Unknown 01 00 01 00
	uint32 I1;
	// Unknown?
	// Unknown?
	/// Material index in multi-submat
	uint16 Material;
	/// Bitfield with smoothing groups
	uint32 SmoothingGroups;
	/// Cuts at local vertex index to local vertex index
	std::vector<std::pair<uint32, uint32> > Triangulation;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

/**
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * CGeomBuffers
 */
class CGeomBuffers : public CStorageContainer
{
public:
	CGeomBuffers();
	virtual ~CGeomBuffers();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CGeomBuffers */

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_GEOM_BUFFERS_H */

/* end of file */
