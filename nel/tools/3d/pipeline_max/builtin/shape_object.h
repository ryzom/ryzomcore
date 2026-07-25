/**
 * \file shape_object.h
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

#ifndef PIPELINE_SHAPE_OBJECT_H
#define PIPELINE_SHAPE_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>

// NeL includes
#include <nel/misc/vector.h>

// Project includes
#include "object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CShapeObject
 * \date 2026-07-16 10:00GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The Shape-superclass object (superclass 0x40) — spline geometry: SplineShape, Line, Text,
 * and the parametric spline primitives (Rectangle, Circle, Helix).
 *
 * Chunk stream (corpus-established over every 0x40 object in the corpus — 5922 objects, all in
 * Scene-version 0x200e files; the Max 3 corpus carries none):
 *
 * - The parametric classes (Rectangle 0x1065, Circle 0x1999, Helix 0x1994) store NO spline
 *   chunks at all — their geometry regenerates from their old-style ParamBlock (CParamBlock).
 * - SplineShape (0x0a) carries one BezierShape container at chunk id 0x1010 among its object
 *   chunks; Line (0x1040) wraps the identical SplineShape body (including that 0x1010) in one
 *   0x2000 container; Text (0x1993) carries one per-glyph 0x1030 container each holding its
 *   BezierShape as a 0x1020 child. Because the BezierShape container id is context-dependent,
 *   the typed decode identifies structures by CONTENT, not by absolute path: any container
 *   whose direct children include both 0x2900 and 0x290a is a Spline3D; any container with a
 *   Spline3D child or a 0x1050 leaf is a BezierShape.
 * - BezierShape container, in canonical order: zero or more Spline3D containers (id 0x1010),
 *   then selection-set containers 0x1020/0x1030/0x1040 (per-spline 0x2700 bit arrays behind a
 *   0x1000 count — kept raw), then leaves 0x1050 (uint32 interpolation steps: 5 on the Text
 *   glyph outlines, 6 default, 0 = knots only), 0x1060, 0x1070, 0x1080, 0x1090, 0x1100 (empty),
 *   0x2000 (24 bytes).
 * - Spline3D container, in canonical order: 0x2900 uint32 numKnots, 0x2904 uint32 (0 corpus-
 *   wide, semantics unknown), 0x290a numKnots x 52-byte compact knot records, 0x290d uint32 =
 *   the CLOSED flag (0 open / 1 closed; the flag lives at 0x290d, not 0x2904).
 * - Compact knot record (52 bytes): ktype sint32 (KTYPE_AUTO=0/CORNER=1/BEZIER=2/
 *   BEZIER_CORNER=3), ltype sint32 (LTYPE_CURVE=0/LTYPE_LINE=1), du float, then three Point3:
 *   the KNOT POINT first (matches ShapeObject::InterpPiece3D endpoints), then the in and out
 *   handle vectors, then flags uint32 (0 corpus-wide).
 *
 * This class keeps the raw chunks authoritative (the CParamBlock/CParamBlock2 overlay-codec
 * discipline): parse decodes a typed model over the orphaned chunks WITHOUT moving them,
 * build re-emits them verbatim, so roundtrip is byte-exact by construction. On
 * top of that it exposes the decoded splines (per-Spline3D closed flag and knot records, in
 * document order across all BezierShapes) and the interpolation steps.
 */
class CShapeObject : public CObject
{
public:
	/// One knot of a Spline3D, decoded from its 52-byte compact record.
	struct SKnot
	{
		sint32 KType;  // KTYPE_AUTO=0, CORNER=1, BEZIER=2, BEZIER_CORNER=3
		sint32 LType;  // LTYPE_CURVE=0, LTYPE_LINE=1
		float Du;      // parameter value
		NLMISC::CVector Knot;   // the knot point (InterpPiece3D endpoints)
		NLMISC::CVector InVec;  // in handle (second Point3 on disk)
		NLMISC::CVector OutVec; // out handle (third Point3 on disk)
		uint32 Flags;  // raw flags word (0 corpus-wide)
	};

	/// One Spline3D (a decoded view over its container; the chunks stay orphaned and remain
	/// the serialization authority).
	struct SSpline
	{
		uint32 NumKnots;    // 0x2900
		uint32 Word2904;    // 0x2904 (0 corpus-wide, semantics unknown)
		uint32 ClosedWord;  // 0x290d — the closed flag
		std::vector<SKnot> Knots; // decoded 0x290a records (empty when NumKnots*52 exceeds the payload)
		uint BezierShape;   // index of the owning BezierShape, document order
		bool Canonical;     // sibling set is exactly {0x2900(4), 0x2904(4), 0x290a(NumKnots*52),
		                    // 0x290d(4)} in that order (true corpus-wide)

		// Internal: the raw 0x290a leaf (not owned; the base container owns the chunk).
		CStorageRaw *KnotsChunk;

		SSpline() : NumKnots(0), Word2904(0), ClosedWord(0), BezierShape(0),
			Canonical(false), KnotsChunk(NULL) { }

		inline bool closed() const { return ClosedWord != 0; }
	};

	CShapeObject(CScene *scene);
	virtual ~CShapeObject() NL_OVERRIDE;

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;
	virtual void init() NL_OVERRIDE;
	virtual bool inherits(const NLMISC::CClassId classId) const NL_OVERRIDE;
	virtual const ISceneClassDesc *classDesc() const NL_OVERRIDE;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const NL_OVERRIDE;

	//! \name Typed read access (valid between parse and clean/disown)
	//@{
	/// Decoded Spline3D records, in document order across all BezierShapes. Empty for the
	/// parametric shape classes (Rectangle, Circle, Helix), which store no spline chunks.
	inline const std::vector<SSpline> &splines() const { return m_Splines; }
	/// BezierShape interpolation steps (0x1050) — the last one in document order when a shape
	/// carries several BezierShapes (Text), matching the historical walk.
	inline sint32 steps() const { return m_Steps; }
	inline bool hasSteps() const { return m_HasSteps; }
	/// Number of BezierShape containers found (Text carries one per glyph; SplineShape/Line one).
	inline uint numBezierShapes() const { return m_NumBezierShapes; }
	//@}

	/// Write-direction self-check: re-encode every decoded knot record from its typed fields and
	/// verify the bytes match the stored 0x290a payload, and verify every Spline3D container is
	/// structurally canonical (proves the decode offsets over the corpus). Returns false and
	/// fills err on the first mismatch.
	bool selfTestReencode(std::string &err) const;

	/// Chunk ids the decode did not recognize among Spline3D or BezierShape container children
	/// (diagnostics; such chunks pass through untouched). Empty across the corpus.
	inline const std::vector<uint16> &unknownSiblingIds() const { return m_UnknownSiblingIds; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	void decodeShapeModel();
	void walkContainer(CStorageContainer *container);
	void decodeSpline(CStorageContainer *container, uint bezierShape);
	void noteUnknownSiblingId(uint16 id);
	static bool isSplineContainer(CStorageContainer *container);

	std::vector<SSpline> m_Splines;
	sint32 m_Steps;
	bool m_HasSteps;
	uint m_NumBezierShapes;
	std::vector<uint16> m_UnknownSiblingIds;

}; /* class CShapeObject */

typedef CSceneClassDesc<CShapeObject> CShapeObjectClassDesc;
extern const CShapeObjectClassDesc ShapeObjectClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SHAPE_OBJECT_H */

/* end of file */
