/**
 * \file geom_buffers.cpp
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

#include <nel/misc/types_nl.h>
#include "geom_buffers.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "../../storage_array.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// Elevate warnings to errors in this file for stricter reading
#undef nlwarning
#define nlwarning nlerror

// Elevate debug to error in this file for debugging
// #undef nldebug
// #define nldebug nlerror

// Chunk identifiers
#define PMBS_GEOM_BUFFERS_TRI_A_VERTEX_CHUNK_ID 0x0914
#define PMBS_GEOM_BUFFERS_TRI_A_INDEX_CHUNK_ID 0x0912
#define PMBS_GEOM_BUFFERS_TRI_B_VERTEX_CHUNK_ID 0x0916
#define PMBS_GEOM_BUFFERS_TRI_B_INDEX_CHUNK_ID 0x0918
#define PMBS_GEOM_BUFFERS_TRI_C_VERTEX_CHUNK_ID 0x0938
#define PMBS_GEOM_BUFFERS_TRI_C_INDEX_CHUNK_ID 0x0942
#define PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID 0x0100
#define PBMS_GEOM_BUFFERS_POLY_A_INDEX_A_CHUNK_ID 0x010a
#define PBMS_GEOM_BUFFERS_POLY_A_INDEX_B_CHUNK_ID 0x011a

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CGeomBuffers::CGeomBuffers()
{

}

CGeomBuffers::~CGeomBuffers()
{

}

std::string CGeomBuffers::className() const
{
	return "GeomBuffers";
}

void CGeomBuffers::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CGeomBuffers::parse(uint16 version)
{
	CStorageContainer::parse(version);
}

void CGeomBuffers::clean()
{
	CStorageContainer::clean();
}

void CGeomBuffers::build(uint16 version)
{
	CStorageContainer::build(version);
}

void CGeomBuffers::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CGeomBuffers::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMBS_GEOM_BUFFERS_TRI_A_VERTEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_B_VERTEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_C_VERTEX_CHUNK_ID:
	case PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID:
		nlassert(!container);
		return new CStorageArray<float>();
	case PMBS_GEOM_BUFFERS_TRI_A_INDEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_B_INDEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_C_INDEX_CHUNK_ID:
	case PBMS_GEOM_BUFFERS_POLY_A_INDEX_A_CHUNK_ID:
	case PBMS_GEOM_BUFFERS_POLY_A_INDEX_B_CHUNK_ID:
		nlassert(!container);
		return new CStorageArray<uint32>();
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
