/**
 * \file geom_object.cpp
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

#include <nel/misc/types_nl.h>
#include "geom_object.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

#define PMB_GEOM_UNKNOWN0900_CHUNK_ID 0x0900
#define PMB_GEOM_BUFFERS_CHUNK_ID 0x08fe

CGeomObject::CGeomObject(CScene *scene) : CObject(scene), m_Unknown0900(NULL), m_GeomBuffers(NULL)
{

}

CGeomObject::~CGeomObject()
{
	if (!m_ChunksOwnsPointers)
	{
		m_Unknown0900 = NULL;
		m_GeomBuffers = NULL;
	}
}

const ucstring CGeomObject::DisplayName = ucstring("GeomObject");
const char *CGeomObject::InternalName = "GeomObject";
const char *CGeomObject::InternalNameUnknown = "GeomObjectUnknown";
const NLMISC::CClassId CGeomObject::ClassId = NLMISC::CClassId(0x37097c44, 0x38aa3f24); /* Not official, please correct */
const TSClassId CGeomObject::SuperClassId = 0x00000010;
const CGeomObjectClassDesc GeomObjectClassDesc(&DllPluginDescBuiltin);
const CGeomObjectSuperClassDesc GeomObjectSuperClassDesc(&GeomObjectClassDesc);

void CGeomObject::parse(uint16 version, uint filter)
{
	if (filter == 0)
	{
		CObject::parse(version);
	}
	else if (filter == PMB_GEOM_OBJECT_PARSE_FILTER)
	{
		if (!m_ChunksOwnsPointers)
		{
			m_Unknown0900 = getChunk(PMB_GEOM_UNKNOWN0900_CHUNK_ID);
			m_GeomBuffers = static_cast<STORAGE::CGeomBuffers *>(getChunk(PMB_GEOM_BUFFERS_CHUNK_ID));
		}
	}
}

void CGeomObject::clean()
{
	CObject::clean();
}

void CGeomObject::build(uint16 version, uint filter)
{
	if (filter == 0)
	{
		CObject::build(version);
	}
	else if (filter == PMB_GEOM_OBJECT_PARSE_FILTER)
	{
		if (m_Unknown0900) putChunk(PMB_GEOM_UNKNOWN0900_CHUNK_ID, m_Unknown0900);
		if (m_GeomBuffers) putChunk(PMB_GEOM_BUFFERS_CHUNK_ID, m_GeomBuffers);
	}
}

void CGeomObject::disown()
{
	m_Unknown0900 = NULL;
	m_GeomBuffers = NULL;
	CObject::disown();
}

void CGeomObject::init()
{
	CObject::init();
}

bool CGeomObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CObject::inherits(classId);
}

const ISceneClassDesc *CGeomObject::classDesc() const
{
	return &GeomObjectClassDesc;
}

void CGeomObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	if (filter == 0)
	{
		CObject::toStringLocal(ostream, pad);
	}
	else if (filter == PMB_GEOM_OBJECT_PARSE_FILTER)
	{
		std::string padpad = pad + "\t";
		if (m_Unknown0900)
		{
			ostream << "\n" << pad << "GeomObject Unknown 0x0900: ";
			m_Unknown0900->toString(ostream, padpad);
		}
		if (m_GeomBuffers)
		{
			ostream << "\n" << pad << "GeomBuffers: ";
			m_GeomBuffers->toString(ostream, padpad);
		}
	}
}

inline uint32 rrsub(uint32 v, uint32 size)
{
	if (v) return v - 1;
	return size - 1;
}

inline uint32 rradd(uint32 v, uint32 size)
{
	uint32 vp = v + 1;
	if (vp != size) return vp;
	return 0;
}

void CGeomObject::triangulatePolyFace(std::vector<STORAGE::CGeomTriIndex> &triangles, const STORAGE::CGeomPolyFaceInfo &polyFace)
{
	nlassert(polyFace.Vertices.size() >= 3);
	nlassert(polyFace.Triangulation.size() == polyFace.Vertices.size() - 3);
	uint nbVert = polyFace.Vertices.size();
	uint nbCuts = polyFace.Triangulation.size();
	uint nbTriangles = 0;

	// This code creates a matrix, aka a table, of all possible paths
	// that can be traveled to get directly from one vertex to another
	// over an egde.
	// Outer edges of the polygon are one-way, backwards.
	// Cut edges can be traveled both ways.
	// Each edge direction can only be traveled by one triangle.
	// Ingenious, if I may say so myself.
	// Bad performance by std::vector, though.
	std::vector<std::vector<bool> > from_to;
	from_to.resize(nbVert);
	for (uint i = 0; i < nbVert; ++i)
	{
		from_to[i].resize(nbVert);
		for (uint j = 0; j < nbVert; ++j)
		{
			from_to[i][j] = false;
		}
		// Can travel backwards over the outer edge
		from_to[i][rrsub(i, nbVert)] = true;
	}
	for (uint i = 0; i < nbCuts; ++i)
	{
		// Can travel both ways over cuts, but the first direction is handled directly!
		// from_to[polyFace.Triangulation[i].first][polyFace.Triangulation[i].second] = true;
		from_to[polyFace.Triangulation[i].second][polyFace.Triangulation[i].first] = true;
	}
	// Triangulate all cuts, this assumes cuts are in the direction
	// of a triangle that is not already handled by another cut...
	for (uint i = 0; i < nbCuts; ++i)
	{
		uint32 a = polyFace.Triangulation[i].first;
		uint32 b = polyFace.Triangulation[i].second;
		// from_to[polyFace.Triangulation[i].first][polyFace.Triangulation[i].second] = false; // handled!
		// Try to find a path that works
		for (uint c = 0; c < nbVert; ++c)
		{
			// Can we make a triangle
			if (from_to[b][c] && from_to[c][a])
			{
				STORAGE::CGeomTriIndex tri;
				tri.a = polyFace.Vertices[c];
				tri.b = polyFace.Vertices[b];
				tri.c = polyFace.Vertices[a];
				triangles.push_back(tri);
				++nbTriangles;
				// nldebug("add tri from cut");
				from_to[b][c] = false;
				from_to[c][a] = false;
				break;
			}
		}
	}
	// Find... The Last Triangle
	for (uint a = 0; a < nbVert; ++a)
	{
		uint b = rrsub(a, nbVert);
		// Can we still travel backwards over the outer edge?
		if (from_to[a][b])
		{
			for (uint c = 0; c < nbVert; ++c)
			{
				// Can we make a triangle
				if (from_to[b][c] && from_to[c][a])
				{
					STORAGE::CGeomTriIndex tri;
					tri.a = polyFace.Vertices[c];
					tri.b = polyFace.Vertices[b];
					tri.c = polyFace.Vertices[a];
					triangles.push_back(tri);
					++nbTriangles;
					// nldebug("add final tri");
					from_to[b][c] = false;
					from_to[c][a] = false;
					break;
				}
			}
		}
	}
	// nldebug("triangles: %i", nbTriangles);
	// nldebug("cuts: %i", nbCuts);
	nlassert(nbTriangles == nbCuts + 1);
}

IStorageObject *CGeomObject::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_GEOM_UNKNOWN0900_CHUNK_ID:
		return new CStorageArray<sint32>();
	case PMB_GEOM_BUFFERS_CHUNK_ID:
		return new STORAGE::CGeomBuffers();
	}
	return CObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
