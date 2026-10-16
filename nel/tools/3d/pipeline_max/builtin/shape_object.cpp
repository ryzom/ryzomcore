/**
 * \file shape_object.cpp
 * \brief CShapeObject
 * \date 2026-07-16 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CShapeObject
 */

/*
 * Copyright (C) 2026  by authors
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
#include "shape_object.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

// Shape-superclass spline chunk stream (corpus-established; see shape_object.h):
// Spline3D container children, canonical order.
#define PMB_SPLINE_NUM_KNOTS_CHUNK_ID 0x2900
#define PMB_SPLINE_WORD2904_CHUNK_ID 0x2904
#define PMB_SPLINE_KNOTS_CHUNK_ID 0x290a
#define PMB_SPLINE_CLOSED_CHUNK_ID 0x290d
// BezierShape container children. The Spline3D containers carry id 0x1010; note the BezierShape
// container id itself is context-dependent (0x1010 under SplineShape/Line, 0x1020 under a Text
// glyph's 0x1030), which is why structures are identified by content (see isSplineContainer).
#define PMB_BEZIER_SPLINE3D_CHUNK_ID 0x1010
#define PMB_BEZIER_SEL_VERT_CHUNK_ID 0x1020
#define PMB_BEZIER_SEL_SEG_CHUNK_ID 0x1030
#define PMB_BEZIER_SEL_POLY_CHUNK_ID 0x1040
#define PMB_BEZIER_STEPS_CHUNK_ID 0x1050
#define PMB_BEZIER_UNKNOWN1060_CHUNK_ID 0x1060
#define PMB_BEZIER_UNKNOWN1070_CHUNK_ID 0x1070
#define PMB_BEZIER_UNKNOWN1080_CHUNK_ID 0x1080
#define PMB_BEZIER_UNKNOWN1090_CHUNK_ID 0x1090
#define PMB_BEZIER_UNKNOWN1100_CHUNK_ID 0x1100
#define PMB_BEZIER_UNKNOWN2000_CHUNK_ID 0x2000
// Compact on-disk knot record size (corpus-verified: every Spline3D in the corpus uses 52).
#define PMB_SPLINE_KNOT_BYTES 52

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CShapeObject::CShapeObject(CScene *scene) : CObject(scene),
	m_Steps(0), m_HasSteps(false), m_NumBezierShapes(0)
{

}

CShapeObject::~CShapeObject()
{

}

const ucstring CShapeObject::DisplayName = ucstring("ShapeObject");
const char *CShapeObject::InternalName = "ShapeObject";
const char *CShapeObject::InternalNameUnknown = "ShapeObjectUnknown";
const NLMISC::CClassId CShapeObject::ClassId = NLMISC::CClassId(0x2ea1743f, 0x5c60b192); /* Not official, please correct */
const TSClassId CShapeObject::SuperClassId = 0x00000040; // Shape; literal to avoid cross-TU static-init-order dependency
const CShapeObjectClassDesc ShapeObjectClassDesc(&DllPluginDescBuiltin);

void CShapeObject::parse(uint16 version, uint filter)
{
	CObject::parse(version);
	if (!m_ChunksOwnsPointers)
		decodeShapeModel();
}

void CShapeObject::clean()
{
	CObject::clean();
}

void CShapeObject::build(uint16 version, uint filter)
{
	// Raw chunks stay authoritative: the base re-emits every orphaned chunk verbatim, so
	// roundtrip is byte-exact by construction.
	CObject::build(version);
}

void CShapeObject::disown()
{
	m_Splines.clear();
	m_Steps = 0;
	m_HasSteps = false;
	m_NumBezierShapes = 0;
	m_UnknownSiblingIds.clear();
	CObject::disown();
}

void CShapeObject::init()
{
	CObject::init();
}

bool CShapeObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CObject::inherits(classId);
}

const ISceneClassDesc *CShapeObject::classDesc() const
{
	return &ShapeObjectClassDesc;
}

// True when the container's direct children include both 0x2900 and 0x290a — the Spline3D
// signature. (The BezierShape container id is context-dependent — 0x1010 under SplineShape/
// Line, 0x1020 under a Text glyph's 0x1030 — so structures are identified by content.)
bool CShapeObject::isSplineContainer(CStorageContainer *container)
{
	bool haveNum = false, haveKnots = false;
	const CStorageContainer::TStorageObjectContainer &chunks = container->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first == PMB_SPLINE_NUM_KNOTS_CHUNK_ID) haveNum = true;
		else if (it->first == PMB_SPLINE_KNOTS_CHUNK_ID) haveKnots = true;
	}
	return haveNum && haveKnots;
}

void CShapeObject::noteUnknownSiblingId(uint16 id)
{
	for (std::vector<uint16>::const_iterator it = m_UnknownSiblingIds.begin(); it != m_UnknownSiblingIds.end(); ++it)
		if (*it == id) return;
	m_UnknownSiblingIds.push_back(id);
}

// Decode the spline model from the orphaned chunks WITHOUT moving them (the raw chunks remain
// the serialization authority). Preorder walk, so splines land in document order across all
// BezierShapes (Text carries one BezierShape per glyph).
void CShapeObject::decodeShapeModel()
{
	m_Splines.clear();
	m_Steps = 0;
	m_HasSteps = false;
	m_NumBezierShapes = 0;
	m_UnknownSiblingIds.clear();

	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
		if (sub) walkContainer(sub);
	}
}

void CShapeObject::walkContainer(CStorageContainer *container)
{
	if (isSplineContainer(container))
	{
		decodeSpline(container, m_NumBezierShapes ? m_NumBezierShapes - 1 : 0);
		return;
	}

	// A BezierShape holds the Spline3D containers and the 0x1050 steps leaf as direct children.
	const CStorageContainer::TStorageObjectContainer &chunks = container->chunks();
	bool isBezierShape = false;
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first == PMB_BEZIER_STEPS_CHUNK_ID && !it->second->isContainer())
		{
			isBezierShape = true;
			break;
		}
		CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
		if (sub && isSplineContainer(sub))
		{
			isBezierShape = true;
			break;
		}
	}

	if (isBezierShape)
	{
		++m_NumBezierShapes;
		for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
		{
			CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			switch (it->first)
			{
			case PMB_BEZIER_SPLINE3D_CHUNK_ID: // Spline3D container (content re-checked — id alone does not decide)
				if (sub && isSplineContainer(sub)) { decodeSpline(sub, m_NumBezierShapes - 1); break; }
				noteUnknownSiblingId(it->first);
				break;
			case PMB_BEZIER_SEL_VERT_CHUNK_ID:
			case PMB_BEZIER_SEL_SEG_CHUNK_ID:
			case PMB_BEZIER_SEL_POLY_CHUNK_ID:
				// Per-spline selection sets (0x2700 bit arrays behind a 0x1000 count) — kept raw.
				if (!sub) noteUnknownSiblingId(it->first);
				break;
			case PMB_BEZIER_STEPS_CHUNK_ID:
				// Interpolation steps; last in document order wins on multi-BezierShape shapes
				// (Text), matching the historical export-side walk.
				if (raw && raw->Value.size() == 4)
				{
					sint32 steps = 0;
					memcpy(&steps, nlVectorData(raw->Value), 4);
					m_Steps = steps;
					m_HasSteps = true;
				}
				else noteUnknownSiblingId(it->first);
				break;
			case PMB_BEZIER_UNKNOWN1060_CHUNK_ID:
			case PMB_BEZIER_UNKNOWN1070_CHUNK_ID:
			case PMB_BEZIER_UNKNOWN1080_CHUNK_ID:
			case PMB_BEZIER_UNKNOWN1090_CHUNK_ID:
			case PMB_BEZIER_UNKNOWN1100_CHUNK_ID:
			case PMB_BEZIER_UNKNOWN2000_CHUNK_ID:
				// Known-unknown BezierShape leaves, kept raw.
				if (!raw) noteUnknownSiblingId(it->first);
				break;
			default:
				noteUnknownSiblingId(it->first);
				break;
			}
		}
		return;
	}

	// Plain container — recurse looking for nested BezierShapes (Text glyph containers).
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		CStorageContainer *sub = dynamic_cast<CStorageContainer *>(it->second);
		if (sub) walkContainer(sub);
	}
}

void CShapeObject::decodeSpline(CStorageContainer *container, uint bezierShape)
{
	SSpline spline;
	spline.BezierShape = bezierShape;

	// Canonical child sequence: 0x2900(4), 0x2904(4), 0x290a(NumKnots*52), 0x290d(4).
	static const uint16 canonicalIds[4] = {
		PMB_SPLINE_NUM_KNOTS_CHUNK_ID, PMB_SPLINE_WORD2904_CHUNK_ID,
		PMB_SPLINE_KNOTS_CHUNK_ID, PMB_SPLINE_CLOSED_CHUNK_ID
	};
	uint pos = 0;
	bool canonical = true;

	const CStorageContainer::TStorageObjectContainer &chunks = container->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (pos < 4 && it->first == canonicalIds[pos]) ++pos;
		else canonical = false;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		switch (it->first)
		{
		case PMB_SPLINE_NUM_KNOTS_CHUNK_ID:
			if (raw && raw->Value.size() == 4) memcpy(&spline.NumKnots, nlVectorData(raw->Value), 4);
			else canonical = false;
			break;
		case PMB_SPLINE_WORD2904_CHUNK_ID:
			if (raw && raw->Value.size() == 4) memcpy(&spline.Word2904, nlVectorData(raw->Value), 4);
			else canonical = false;
			break;
		case PMB_SPLINE_KNOTS_CHUNK_ID:
			if (raw) spline.KnotsChunk = raw;
			else canonical = false;
			break;
		case PMB_SPLINE_CLOSED_CHUNK_ID:
			if (raw && raw->Value.size() == 4) memcpy(&spline.ClosedWord, nlVectorData(raw->Value), 4);
			else canonical = false;
			break;
		default:
			canonical = false;
			noteUnknownSiblingId(it->first);
			break;
		}
	}
	if (pos != 4) canonical = false;
	if (spline.KnotsChunk
		&& (uint64)spline.KnotsChunk->Value.size() != (uint64)spline.NumKnots * PMB_SPLINE_KNOT_BYTES)
		canonical = false;
	spline.Canonical = canonical;

	// Decode the compact knot records. Tolerate an oversized payload (decode NumKnots records)
	// but never read past the end; the canonical flag records exact-size conformance.
	if (spline.KnotsChunk
		&& (uint64)spline.NumKnots * PMB_SPLINE_KNOT_BYTES <= (uint64)spline.KnotsChunk->Value.size())
	{
		spline.Knots.reserve(spline.NumKnots);
		const uint8 *data = nlVectorData(spline.KnotsChunk->Value);
		for (uint32 i = 0; i < spline.NumKnots; ++i)
		{
			const uint8 *p = data + (size_t)i * PMB_SPLINE_KNOT_BYTES;
			SKnot knot;
			memcpy(&knot.KType, p + 0, 4);
			memcpy(&knot.LType, p + 4, 4);
			memcpy(&knot.Du, p + 8, 4);
			memcpy(&knot.Knot.x, p + 12, 4); memcpy(&knot.Knot.y, p + 16, 4); memcpy(&knot.Knot.z, p + 20, 4);
			memcpy(&knot.InVec.x, p + 24, 4); memcpy(&knot.InVec.y, p + 28, 4); memcpy(&knot.InVec.z, p + 32, 4);
			memcpy(&knot.OutVec.x, p + 36, 4); memcpy(&knot.OutVec.y, p + 40, 4); memcpy(&knot.OutVec.z, p + 44, 4);
			memcpy(&knot.Flags, p + 48, 4);
			spline.Knots.push_back(knot);
		}
	}

	m_Splines.push_back(spline);
}

bool CShapeObject::selfTestReencode(std::string &err) const
{
	for (uint s = 0; s < m_Splines.size(); ++s)
	{
		const SSpline &spline = m_Splines[s];
		if (!spline.Canonical)
		{
			std::stringstream ss;
			ss << "spline " << s << " container is not structurally canonical";
			err = ss.str();
			return false;
		}
		if (!spline.KnotsChunk || spline.Knots.size() != spline.NumKnots)
		{
			std::stringstream ss;
			ss << "spline " << s << " decoded " << spline.Knots.size() << " of " << spline.NumKnots << " knots";
			err = ss.str();
			return false;
		}
		const uint8 *data = nlVectorData(spline.KnotsChunk->Value);
		for (uint32 i = 0; i < spline.NumKnots; ++i)
		{
			const SKnot &knot = spline.Knots[i];
			uint8 buf[PMB_SPLINE_KNOT_BYTES];
			memcpy(buf + 0, &knot.KType, 4);
			memcpy(buf + 4, &knot.LType, 4);
			memcpy(buf + 8, &knot.Du, 4);
			memcpy(buf + 12, &knot.Knot.x, 4); memcpy(buf + 16, &knot.Knot.y, 4); memcpy(buf + 20, &knot.Knot.z, 4);
			memcpy(buf + 24, &knot.InVec.x, 4); memcpy(buf + 28, &knot.InVec.y, 4); memcpy(buf + 32, &knot.InVec.z, 4);
			memcpy(buf + 36, &knot.OutVec.x, 4); memcpy(buf + 40, &knot.OutVec.y, 4); memcpy(buf + 44, &knot.OutVec.z, 4);
			memcpy(buf + 48, &knot.Flags, 4);
			if (memcmp(buf, data + (size_t)i * PMB_SPLINE_KNOT_BYTES, PMB_SPLINE_KNOT_BYTES) != 0)
			{
				std::stringstream ss;
				ss << "spline " << s << " knot " << i << " re-encode mismatch";
				err = ss.str();
				return false;
			}
		}
	}
	return true;
}

void CShapeObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CObject::toStringLocal(ostream, pad);
	if (!m_Splines.empty() || m_HasSteps)
	{
		uint nKnots = 0, nClosed = 0;
		for (std::vector<SSpline>::const_iterator it = m_Splines.begin(); it != m_Splines.end(); ++it)
		{
			nKnots += (uint)it->Knots.size();
			if (it->closed()) ++nClosed;
		}
		ostream << "\n" << pad << "ShapeObject: " << m_NumBezierShapes << " beziershapes, "
			<< m_Splines.size() << " splines (" << nClosed << " closed), " << nKnots << " knots";
		if (m_HasSteps) ostream << ", steps " << m_Steps;
	}
}

IStorageObject *CShapeObject::createChunkById(uint16 id, bool container)
{
	// The BezierShape tree defaults to CStorageContainer for containers and CStorageRaw for
	// every leaf already; the raw bytes stay authoritative.
	return CObject::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
