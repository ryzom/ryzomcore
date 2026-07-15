/**
 * \file param_block_2.h
 * \brief CParamBlock2
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * CParamBlock2
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

#ifndef PIPELINE_PARAM_BLOCK_2_H
#define PIPELINE_PARAM_BLOCK_2_H
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
 * \brief CParamBlock2
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 *
 * The Max ParamBlock2 (superclass 0x82). A ParamBlock2 scene object stores a single parameter
 * block: a header chunk 0x0009 = { u32 scriptVersion, u16 blockId, u16 (owner class marker),
 * u16 0x2328, u16 paramCount, u32 ownerSceneIndex }, followed by one 0x000e chunk per
 * parameter = { u16 paramId, u16 type, 10 opaque bytes, u8 flagByte, payload }. flagByte bit
 * 0x40 = an inline constant value follows (except reference-kind types, whose value is a
 * reference slot on the PB2 object). Reference-kind params (MTL/TEXMAP/NODE/REFTARG) and
 * controller-backed params own the PB2's reference slots in record order. Tab (array) params
 * (type bit 0x800) carry a u32 count then per-element flag+value.
 *
 * This class keeps the raw chunks authoritative (roundtrip is byte-exact by construction, the
 * design-doc §5/§12.2 discipline shared with CControlKeyFramerBase / CRklPatchObject): parse
 * decodes a typed model over the orphaned chunks WITHOUT moving them, build re-emits them
 * verbatim. On top of that it exposes typed read access to every parameter and an in-place
 * modify API (setFloat/setInt/setBool/setColor rewrite the owning record's payload bytes) —
 * the read+modify+save foundation for programmatic .max editing and the standalone NeL
 * material editor. See max_geometry_formats.md Part I and pipeline_max_design.md §10i/§12.5.
 */
class CParamBlock2 : public CReferenceTarget
{
public:
	// Low bits of the parameter type word (type & 0x07ff); bit 0x0800 = Tab (array).
	enum TType
	{
		TYPE_FLOAT = 0x0,
		TYPE_INT = 0x1,
		TYPE_RGBA = 0x2,
		TYPE_POINT3 = 0x3,
		TYPE_BOOL = 0x4,
		TYPE_ANGLE = 0x5,
		TYPE_PCNT_FRAC = 0x6,
		TYPE_WORLD = 0x7,
		TYPE_STRING = 0x8,
		TYPE_FILENAME = 0x9,
		TYPE_HSV = 0xa,
		TYPE_COLOR_CHANNEL = 0xb,
		TYPE_TIMEVALUE = 0xc,
		TYPE_RADIOBTN_INDEX = 0xd,
		TYPE_MTL = 0xe,
		TYPE_TEXMAP = 0xf,
		TYPE_BITMAP = 0x10,
		TYPE_NODE = 0x11,
		TYPE_REFTARG = 0x12,
		TYPE_TAB_FLAG = 0x0800
	};

	/// One decoded parameter record (a claimed view over its 0x000e chunk; the chunk stays in
	/// the orphan list and remains the serialization authority).
	struct SParam
	{
		uint16 Id;
		uint16 Type;
		bool HasConstant;   // an inline constant payload was decoded
		bool RefBacked;     // owns a reference slot on the PB2 object
		sint RefSlot;       // reference slot index, -1 when not ref-backed
		bool IsTab;         // Tab (array) parameter

		// Scalar constant payloads by kind (F[0] for float scalars, F[0..2] for color/point3)
		float F[4];
		sint32 I;
		std::string S;      // string / filename

		// Tab element values (reference-kind element types put the per-element value — a scene
		// storage index, -1 = none — in TabI; 12-byte element types fill three TabF per element)
		std::vector<sint32> TabI;
		std::vector<float> TabF;

		// Internal: the owning raw chunk (0x000e) and the byte offset of the payload within it,
		// for in-place modification. Not owned (the base container owns the chunk).
		CStorageRaw *Chunk;
		uint PayloadOff;

		SParam() : Id(0), Type(0), HasConstant(false), RefBacked(false), RefSlot(-1), IsTab(false),
			I(0), Chunk(NULL), PayloadOff(0) { F[0] = F[1] = F[2] = F[3] = 0.0f; }

		inline uint16 baseType() const { return (uint16)(Type & 0x07ff); }
		static bool typeIsRefKind(uint16 type);
	};

	CParamBlock2(CScene *scene);
	virtual ~CParamBlock2();

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
	/// True when a 0x0009 header chunk was found.
	inline bool hasHeader() const { return m_HasHeader; }
	inline uint32 scriptVersion() const { return m_ScriptVersion; }
	inline uint16 blockId() const { return m_BlockId; }
	/// Parameter count declared in the header (may exceed the number of stored records — Max
	/// only writes records for parameters that carry a value or a reference).
	inline uint16 declaredParamCount() const { return m_ParamCount; }
	/// Decoded parameter records, in file order.
	inline const std::vector<SParam> &params() const { return m_Params; }
	/// Find a parameter by id, NULL when absent.
	const SParam *findParam(uint16 id) const;

	bool getFloat(uint16 id, float &out) const;
	bool getInt(uint16 id, sint32 &out) const;
	bool getBool(uint16 id, bool &out) const;
	bool getColor(uint16 id, float out[3]) const;
	bool getString(uint16 id, std::string &out) const;

	/// Value at t=0 — like getFloat/getColor, but a ParamBlock2 parameter may be an inline
	/// constant OR controller-backed (animated). These return the inline constant when present,
	/// and otherwise resolve the parameter's reference slot to its keyframe controller and
	/// evaluate it at tick 0 (the value the exporter/editor actually wants — a param animated by
	/// a controller that happens to be correct at frame 0). Only float parameters (Bezier Float
	/// controller) and point3/color parameters carried by a pos-kind controller resolve; a color
	/// backed by an untyped Point3 controller falls through to false. See
	/// CControlKeyFramerBase::floatValueAt0 / posValueAt0.
	bool getFloatAt0(uint16 id, float &out) const;
	bool getColorAt0(uint16 id, float out[3]) const;

	/// The scene object referenced by a ref-backed parameter (texmap/material/node), NULL when
	/// absent. Resolves through the PB2 object's own reference slots.
	CReferenceMaker *refValue(const SParam &param) const;
	CReferenceMaker *refValue(uint16 id) const;
	//@}

	//! \name In-place modify (rewrites the owning record's payload bytes; returns false on a
	//! type mismatch or an unsupported/absent parameter). Only fixed-size scalar/color kinds are
	//! writable so far; the chunk stays byte-exact for every unmodified parameter.
	//@{
	bool setFloat(uint16 id, float v);
	bool setInt(uint16 id, sint32 v);
	bool setBool(uint16 id, bool v);
	bool setColor(uint16 id, const float c[3]);
	//@}

	/// Write-direction self-check: re-encode every fixed-size scalar/color parameter from its
	/// decoded value and verify the bytes match the stored payload (proves the modify path
	/// reproduces the original layout). Returns false and fills err on the first mismatch.
	bool selfTestReencode(std::string &err) const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	void decodeModel();
	SParam *findParamMutable(uint16 id);

	bool m_HasHeader;
	uint32 m_ScriptVersion;
	uint16 m_BlockId;
	uint16 m_ParamCount;
	std::vector<SParam> m_Params;

}; /* class CParamBlock2 */

typedef CSceneClassDesc<CParamBlock2> CParamBlock2ClassDesc;
extern const CParamBlock2ClassDesc ParamBlock2ClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PARAM_BLOCK_2_H */

/* end of file */
