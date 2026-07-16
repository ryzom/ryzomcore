/**
 * \file param_block.h
 * \brief CParamBlock
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CParamBlock
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

#ifndef PIPELINE_PARAM_BLOCK_H
#define PIPELINE_PARAM_BLOCK_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CParamBlock
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The old-style (pre-ParamBlock2) parameter block (superclass 0x8) — the format the builtin
 * parametric primitives (Box, Cylinder, Sphere, ...), the builtin lights, the UVW-mapping and
 * Mirror modifiers, and StdUVGen still use for their parameters.
 *
 * Chunk stream (corpus-established, identical from Max 3 (Scene 0x2004) through Max 2010):
 * two object-level leaves 0x0001 (4 bytes) and 0x0005 (2 bytes, kept raw/undecoded here), then
 * one 0x0002 container per parameter holding, in order: 0x0003 = sint32 declared param index
 * (4 bytes); optionally 0x0004 = an EMPTY marker leaf (never a value); then ONE value leaf
 * whose id is 0x0100 + the old ParamBlock type code — 0x0100 float(4) / 0x0101 int(4) /
 * 0x0102 rgba(12) / 0x0103 point3(12) / 0x0104 bool(4, int 0 or 1). An ANIMATED parameter
 * replaces its value leaf with an EMPTY marker at 0x0200 + the type code (0x0200 float /
 * 0x0201 int / 0x0202 rgba-point3 observed) and its controller occupies the block's reference
 * slots compactly, one per animated parameter of ANY kind, in entry order (§10k; the float
 * mapping is validated byte-exact by the waterfall material-anim gate, the mixed-kind ordering
 * by the animated-light blocks whose 0x0202 color at entry 0 resolves slot 0 to a Bezier
 * Point3 ahead of the float params' Bezier Float controllers).
 *
 * This class keeps the raw chunks authoritative (the CParamBlock2 discipline, design-doc
 * §5/§10j/§12.2): parse decodes a typed model over the orphaned chunks WITHOUT moving them,
 * build re-emits them verbatim, so roundtrip is byte-exact by construction. On top of that it
 * exposes typed read access per declared index, controller resolution for animated params
 * (t=0 evaluation through the typed keyframers), and an in-place modify API (setFloat/setInt/
 * setPoint3 rewrite only the owning value leaf's bytes).
 */
class CParamBlock : public CReferenceTarget
{
public:
	/// Value kind, derived from the value leaf's chunk id (0x0100 + old ParamBlock type code).
	enum TKind
	{
		KindNone = 0,   // no value decoded (animated, or an unrecognized leaf id)
		KindFloat,      // 0x0100
		KindInt,        // 0x0101
		KindPoint3,     // 0x0102 (rgba/point3, 3 floats)
		KindBool        // 0x0104 (sint32 0/1)
	};

	/// One decoded parameter entry (a claimed view over its 0x0002 container; the chunks stay
	/// in the orphan list and remain the serialization authority).
	struct SParam
	{
		sint32 Index;        // declared param index (0x0003)
		TKind Kind;          // value kind when HasConstant
		uint16 ValueChunkId; // the value leaf's chunk id as stored (the 0x020x marker id for
		                     // animated params, 0 when the entry carried neither)
		bool HasConstant;    // a recognized value leaf was decoded
		bool Animated;       // 0x0200 marker: the value is a controller on RefSlot
		sint RefSlot;        // compact reference slot (animated entries in entry order), -1 otherwise

		// Constant payload. Both views are bit-copies of the same stored dwords (F[0] holds the
		// float reinterpretation even for int/bool kinds — consumers pick the right view per
		// Kind); Point3 fills F[0..2].
		float F[3];
		sint32 I;

		// Internal: the owning raw value leaf, for in-place modification. Not owned (the base
		// container owns the chunk). NULL when there is no value leaf (animated params).
		CStorageRaw *Chunk;

		SParam() : Index(-1), Kind(KindNone), ValueChunkId(0), HasConstant(false),
			Animated(false), RefSlot(-1), I(0), Chunk(NULL) { F[0] = F[1] = F[2] = 0.0f; }
	};

	CParamBlock(CScene *scene);
	virtual ~CParamBlock();

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

	//! \name Typed read access (valid between parse and clean/disown)
	//@{
	/// Decoded parameter entries, in file order.
	inline const std::vector<SParam> &params() const { return m_Params; }
	/// Find a parameter by declared index, NULL when absent (first entry wins on duplicates).
	const SParam *findParam(sint32 index) const;

	bool getFloat(sint32 index, float &out) const;
	bool getInt(sint32 index, sint32 &out) const;
	bool getPoint3(sint32 index, float out[3]) const;

	/// Value at t=0 — the inline constant when present, else (animated param) the reference
	/// slot's keyframe controller evaluated at tick 0 (CControlKeyFramerBase::floatValueAt0).
	bool getFloatAt0(sint32 index, float &out) const;

	/// The controller of an ANIMATED parameter (resolves the compact reference slot), NULL for
	/// constant/absent parameters. anim_build's StdUVGen U/V-Offset track walk rides this.
	CReferenceMaker *controllerForParam(sint32 index) const;
	//@}

	//! \name In-place modify (rewrites the owning value leaf's bytes; returns false on a kind
	//! mismatch or an absent/animated parameter). The chunk stream stays byte-exact for every
	//! unmodified parameter.
	//@{
	bool setFloat(sint32 index, float v);
	bool setInt(sint32 index, sint32 v);   // int and bool kinds
	bool setPoint3(sint32 index, const float p[3]);
	//@}

	/// Write-direction self-check: re-encode every decoded constant from its typed value and
	/// verify the bytes match the stored value leaf (proves the decode offsets and the modify
	/// path). Returns false and fills err on the first mismatch.
	bool selfTestReencode(std::string &err) const;

	/// Entry-leaf chunk ids the decode did not recognize (diagnostics; such entries decode with
	/// Kind == KindNone and their raw chunks pass through untouched). Empty across the corpus.
	inline const std::vector<uint16> &unknownEntryChunkIds() const { return m_UnknownEntryChunkIds; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	void decodeModel();
	SParam *findParamMutable(sint32 index);

	std::vector<SParam> m_Params;
	std::vector<uint16> m_UnknownEntryChunkIds;

}; /* class CParamBlock */

typedef CSceneClassDesc<CParamBlock> CParamBlockClassDesc;
extern const CParamBlockClassDesc ParamBlockClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PARAM_BLOCK_H */

/* end of file */
