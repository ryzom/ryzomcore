/**
 * \file geom_buffers.cpp
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * \author Claude Fable 5
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
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>s

// Project includes
#include "../../storage_array.h"
#include "../../storage_value.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define PMBS_GEOM_BUFFERS_PARSE 1

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
#define PBMS_GEOM_BUFFERS_POLY_A_EDGE_CHUNK_ID 0x010a
#define PBMS_GEOM_BUFFERS_POLY_A_FACE_CHUNK_ID 0x011a
// Map-channel chunk family, repeated per stored channel in file order (corpus-established:
// 0x0959 always 4 bytes, 0x2398 always 4 bytes value 1, 0x2394/0x2396 always exactly
// count-prefixed at stride 12; EditableMesh only, stable from Max 3 through Max 2010).
#define PMBS_GEOM_BUFFERS_MAP_CHANNEL_CHUNK_ID 0x0959
#define PMBS_GEOM_BUFFERS_MAP_SUPPORT_CHUNK_ID 0x2398
#define PMBS_GEOM_BUFFERS_MAP_VERTEX_CHUNK_ID 0x2394
#define PMBS_GEOM_BUFFERS_MAP_FACE_CHUNK_ID 0x2396

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void CGeomTriIndex::serial(NLMISC::IStream &stream)
{
	stream.serial(a);
	stream.serial(b);
	stream.serial(c);
}

std::string CGeomTriIndex::toString() const
{
	std::stringstream ss;
	ss << a << " " << b << " " << c;
	return ss.str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void CGeomTriIndexInfo::serial(NLMISC::IStream &stream)
{
	stream.serial(a);
	stream.serial(b);
	stream.serial(c);
	stream.serial(alwaysOne);
	stream.serial(smoothingGroups);
}

std::string CGeomTriIndexInfo::toString() const
{
	std::stringstream ss;
	ss << a << " " << b << " " << c << ", " << alwaysOne << " " << smoothingGroups;
	return ss.str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void CGeomPolyVertexInfo::serial(NLMISC::IStream &stream)
{
	stream.serial(i1);
	stream.serial(v);
}

std::string CGeomPolyVertexInfo::toString() const
{
	std::stringstream ss;
	ss << "0x" << NLMISC::toString("%x", i1) << ", " << v.toString();
	return ss.str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void CGeomPolyEdgeInfo::serial(NLMISC::IStream &stream)
{
	stream.serial(i1);
	stream.serial(a);
	stream.serial(b);
}

std::string CGeomPolyEdgeInfo::toString() const
{
	std::stringstream ss;
	ss << "0x" << NLMISC::toString("%x", i1) << ", " << a << " " << b;
	return ss.str();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CGeomPolyFaceInfo::CGeomPolyFaceInfo() : I1(0), Material(0), SmoothingGroups(0)
{

}

void CGeomPolyFaceInfo::serial(NLMISC::IStream &stream)
{
	// nldebug("go");
	stream.serialCont(Vertices);
	// nldebug("%i vertices", Vertices.size());
	uint16 bitfield;
	if (!stream.isReading())
	{
		/*nldebug("writing");*/
		bitfield = 0x0000;
		if (I1) bitfield |= 0x0001;
		// bitfield |= 0x0002;
		// bitfield |= 0x0004;
		if (Material) bitfield |= 0x0008;
		if (SmoothingGroups) bitfield |= 0x0010;
		if (Triangulation.size()) bitfield |= 0x0020;
		// bitfield |= 0x0040;
		// bitfield |= 0x0080;
	}
	stream.serial(bitfield);
	// nldebug("bitfield 0x%x", (uint32)bitfield);
	if (bitfield & 0x0001) { /*nldebug("i1");*/ stream.serial(I1); nlassert(I1); bitfield &= ~0x0001; }
	else I1 = 0;
	if (bitfield & 0x0008) { /*nldebug("material");*/ stream.serial(Material); nlassert(Material); bitfield &= ~0x0008; }
	else Material = 0;
	if (bitfield & 0x0010) { /*nldebug("smoothing");*/ stream.serial(SmoothingGroups); nlassert(SmoothingGroups); bitfield &= ~0x0010; }
	else SmoothingGroups = 0;
	if (bitfield & 0x0020)
	{
		/*nldebug("triangles");*/
		if (stream.isReading()) Triangulation.resize(Vertices.size() - 3);
		else nlassert(Triangulation.size() == Vertices.size() - 3);
		for (std::vector<std::pair<uint32, uint32> >::size_type i = 0; i < Triangulation.size(); ++i)
		{
			stream.serial(Triangulation[i].first);
			/*nldebug("cut from %i", Triangulation[i].first);*/
			nlassert(Triangulation[i].first < Vertices.size());
			stream.serial(Triangulation[i].second);
			/*nldebug("to %i", Triangulation[i].second);*/
			nlassert(Triangulation[i].second < Vertices.size());
		}
		nlassert(Triangulation.size());
		bitfield &= ~0x0020;
	}
	if (bitfield) nlerror("Remaining bitfield value 0x%x, please debug and implement", (uint32)bitfield);
}

std::string CGeomPolyFaceInfo::toString() const
{
	std::stringstream ss;
	ss << "( ";
	for (std::vector<uint32>::size_type i = 0; i < Vertices.size(); ++i)
	{
		ss << Vertices[i] << " ";
	}
	ss << ")";
	if (I1) ss << ", I1: " << "0x" << NLMISC::toString("%x", I1);
	if (Material) ss << ", M: " << Material;
	if (SmoothingGroups) ss << ", S: " << SmoothingGroups;
	if (Triangulation.size())
	{
		ss << ", ( ";
		for (std::vector<std::pair<uint32, uint32> >::size_type i = 0; i < Triangulation.size(); ++i)
		{
			ss << Triangulation[i].first << ":" << Triangulation[i].second << " ";
		}
		ss << ")";
	}
	ss << " ";
	return ss.str();
}

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

void CGeomBuffers::parse(uint16 version, uint filter)
{
#if PMBS_GEOM_BUFFERS_PARSE
	CStorageContainer::parse(version);
#else
	CStorageContainer::parse(version);
#endif
}

void CGeomBuffers::clean()
{
#if PMBS_GEOM_BUFFERS_PARSE
	CStorageContainer::clean();
#else
	CStorageContainer::clean();
#endif
}

void CGeomBuffers::build(uint16 version, uint filter)
{
#if PMBS_GEOM_BUFFERS_PARSE
	CStorageContainer::build(version);
#else
	CStorageContainer::build(version);
#endif
}

void CGeomBuffers::disown()
{
#if PMBS_GEOM_BUFFERS_PARSE
	CStorageContainer::disown();
#else
	CStorageContainer::disown();
#endif
}

const std::vector<NLMISC::CVector> *CGeomBuffers::triVertices() const
{
	const CStorageArraySizePre<NLMISC::CVector> *a =
		dynamic_cast<const CStorageArraySizePre<NLMISC::CVector> *>(findStorageObject(0x0914));
	return a ? &a->Value : nullptr;
}

const std::vector<CGeomTriIndexInfo> *CGeomBuffers::triFaces() const
{
	const CStorageArraySizePre<CGeomTriIndexInfo> *a =
		dynamic_cast<const CStorageArraySizePre<CGeomTriIndexInfo> *>(findStorageObject(0x0912));
	return a ? &a->Value : nullptr;
}

const std::vector<CGeomPolyVertexInfo> *CGeomBuffers::polyVertices() const
{
	const CStorageArraySizePre<CGeomPolyVertexInfo> *a =
		dynamic_cast<const CStorageArraySizePre<CGeomPolyVertexInfo> *>(findStorageObject(PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID));
	return a ? &a->Value : nullptr;
}

const std::vector<CGeomPolyFaceInfo> *CGeomBuffers::polyFaces() const
{
	const CStorageArrayDynSize<CGeomPolyFaceInfo> *a =
		dynamic_cast<const CStorageArrayDynSize<CGeomPolyFaceInfo> *>(findStorageObject(PBMS_GEOM_BUFFERS_POLY_A_FACE_CHUNK_ID));
	return a ? &a->Value : nullptr;
}

void CGeomBuffers::mapChannels(std::vector<CMapChannelView> &out) const
{
	// Walk the chunk list in file order; 0x0959 opens a new group, the family leaves that follow
	// attach to it. A family leaf before any 0x0959 opens an announce-less group (Channel -1, one
	// Max 3 corpus witness) — the consumer decides what to do with it, matching the historical
	// raw read that skipped such channels.
	out.clear();
	bool haveGroup = false;
	for (TStorageObjectConstIt it = m_Chunks.begin(), end = m_Chunks.end(); it != end; ++it)
	{
		switch (it->first)
		{
		case PMBS_GEOM_BUFFERS_MAP_CHANNEL_CHUNK_ID:
			if (const CStorageValue<uint32> *v = dynamic_cast<const CStorageValue<uint32> *>(it->second))
			{
				out.resize(out.size() + 1);
				out.back().Channel = (sint32)v->Value;
				haveGroup = true;
			}
			break;
		case PMBS_GEOM_BUFFERS_MAP_SUPPORT_CHUNK_ID:
			if (const CStorageValue<uint32> *v = dynamic_cast<const CStorageValue<uint32> *>(it->second))
			{
				if (!haveGroup) { out.resize(out.size() + 1); haveGroup = true; }
				out.back().SupportFlag = v->Value;
				out.back().HasSupportFlag = true;
			}
			break;
		case PMBS_GEOM_BUFFERS_MAP_VERTEX_CHUNK_ID:
			if (const CStorageArraySizePre<NLMISC::CVector> *a = dynamic_cast<const CStorageArraySizePre<NLMISC::CVector> *>(it->second))
			{
				if (!haveGroup) { out.resize(out.size() + 1); haveGroup = true; }
				out.back().Verts = &a->Value;
			}
			break;
		case PMBS_GEOM_BUFFERS_MAP_FACE_CHUNK_ID:
			if (const CStorageArraySizePre<CGeomTriIndex> *a = dynamic_cast<const CStorageArraySizePre<CGeomTriIndex> *>(it->second))
			{
				if (!haveGroup) { out.resize(out.size() + 1); haveGroup = true; }
				out.back().Faces = &a->Value;
			}
			break;
		}
	}
}

IStorageObject *CGeomBuffers::createChunkById(uint16 id, bool container)
{
#if PMBS_GEOM_BUFFERS_PARSE
	switch (id)
	{
	//	nlassert(!container);
	//	return new CStorageArray<float>();
	//case PBMS_GEOM_BUFFERS_POLY_A_INDEX_B_CHUNK_ID:
	//	nlassert(!container);
	//	return new CStorageArray<uint32>();
	case PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID:
		nlassert(!container);
		return new CStorageArraySizePre<CGeomPolyVertexInfo>();
	case PBMS_GEOM_BUFFERS_POLY_A_EDGE_CHUNK_ID:
		nlassert(!container);
		return new CStorageArraySizePre<CGeomPolyEdgeInfo>();
	case PBMS_GEOM_BUFFERS_POLY_A_FACE_CHUNK_ID:
		nlassert(!container);
		return new CStorageArrayDynSize<CGeomPolyFaceInfo>();
	case PMBS_GEOM_BUFFERS_TRI_B_INDEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_C_INDEX_CHUNK_ID:
		nlassert(!container);
		return new CStorageArray<CGeomTriIndex>(); // nb: parse should check if the sizes match with tri_a size (which is the master index buffer)
	case PMBS_GEOM_BUFFERS_TRI_A_INDEX_CHUNK_ID:
		nlassert(!container);
		return new CStorageArraySizePre<CGeomTriIndexInfo>();
	case PMBS_GEOM_BUFFERS_TRI_A_VERTEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_B_VERTEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_TRI_C_VERTEX_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_MAP_VERTEX_CHUNK_ID:
		nlassert(!container);
		return new CStorageArraySizePre<NLMISC::CVector>();
	case PMBS_GEOM_BUFFERS_MAP_FACE_CHUNK_ID:
		nlassert(!container);
		return new CStorageArraySizePre<CGeomTriIndex>();
	case PMBS_GEOM_BUFFERS_MAP_CHANNEL_CHUNK_ID:
	case PMBS_GEOM_BUFFERS_MAP_SUPPORT_CHUNK_ID:
		nlassert(!container);
		return new CStorageValue<uint32>();
	}
#endif
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
