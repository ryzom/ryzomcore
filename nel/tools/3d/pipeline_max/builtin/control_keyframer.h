/**
 * \file control_keyframer.h
 * \brief CControlKeyFramer
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * Typed classes for the builtin keyframe animation controllers (Linear / Bezier / TCB
 * position, rotation, scale and float variants).
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

#ifndef PIPELINE_CONTROL_KEYFRAMER_H
#define PIPELINE_CONTROL_KEYFRAMER_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * Key record layouts as stored in the .max key-table chunks: little-endian dwords, no
 * padding (corpus-verified: every table's byte size is an exact multiple of the record size).
 *
 * Common fields: Time is in Max ticks (4800/s, 160/frame); Flags is the Max key-flags dword
 * (Bezier: tangent types at bits 7..9 in / 10..12 out, BEZKEY_STEP = 2).
 *
 * "Extra" fields are interpolation caches rebuilt by Max at load time (cached quats/tangent
 * data); they are preserved verbatim for roundtrip but their semantics are not decoded.
 */

/// Linear Position (0x2002, 0) — chunk 0x2513
struct CStorageLinPoint3Key
{
	sint32 Time;
	uint32 Flags;
	float Val[3];
};

/// Linear Rotation (0x2003, 0) — chunk 0x2514. Quat is the ABSOLUTE key rotation (x,y,z,w);
/// the reference exporter's ILinRotKey conversion negates w for NeL.
struct CStorageLinRotKey
{
	sint32 Time;
	uint32 Flags;
	float Quat[4];
};

/// Linear Scale (0x2004, 0) — chunk 0x2515. Max ScaleValue: per-axis scale S plus the scale
/// axis system quat Q.
struct CStorageLinScaleKey
{
	sint32 Time;
	uint32 Flags;
	float S[3];
	float Q[4];
};

/// Linear Float (0x2001, 0) — chunk 0x2511
struct CStorageLinFloatKey
{
	sint32 Time;
	uint32 Flags;
	float Val;
};

/// Bezier Float (0x2007, 0) — chunk 0x2525
struct CStorageBezFloatKey
{
	sint32 Time;
	uint32 Flags;
	float Val;
	float InTan;
	float OutTan;
	float Extra[2];
};

/// Bezier Position / Point3 (0x2008, 0) — chunk 0x2526. InTan/OutTan are in value units per
/// tick (the reference exporter multiplies by 4800 for NeL's per-second convention).
struct CStorageBezPoint3Key
{
	sint32 Time;
	uint32 Flags;
	float Val[3];
	float InTan[3];
	float OutTan[3];
	float Extra[9];
};

/// Bezier Scale (0x2010, 0), chunk 0x2528. Layout resolved against the fauna direct-reference
/// anims (plante_carnivore family carries the first corpus keys with nonzero tangents;
/// character corpus is zero-tangents throughout). After S[3]+Q[4], four 7-float blocks at
/// stride 7 = {InTan, OutTan, InLen, OutLen}, each block = vec3 data + 3 zero floats + a
/// constant 1.0 tail. OutTan sits at floats [14..16]; InLen carries -1 sentinels (default 1/3)
/// on first keys.
struct CStorageBezScaleKey
{
	sint32 Time;
	uint32 Flags;
	float S[3];
	float Q[4];
	float InTan[3];
	float InTanPad[4]; // 3 zeros + 1.0 block tail
	float OutTan[3];
	float OutTanPad[4]; // 3 zeros + 1.0 block tail
	float InLen[3]; // -1 sentinels = default 1/3
	float InLenPad[4]; // 3 zeros + 1.0 block tail
	float OutLen[3];
	float OutLenPad[4]; // 3 zeros + 1.0 block tail
};

/// TCB Position / Point3 (0x442312, 0) — chunk 0x2521
struct CStorageTCBPoint3Key
{
	sint32 Time;
	uint32 Flags;
	float Val[3];
	float Tens, Cont, Bias, EaseIn, EaseOut;
	float Extra[6];
};

/// TCB Rotation (0x442313, 0) — chunk 0x2522. AbsQuat is the cumulative absolute rotation at
/// this key; Axis+Angle is the RELATIVE angle-axis from the previous key (axis in world space)
/// — this is what Max's GetKey returns in ITCBRotKey.val and what the reference exporter
/// converts to NeL's CKeyTCBQuat (with the angle negated).
struct CStorageTCBRotKey
{
	sint32 Time;
	uint32 Flags;
	float AbsQuat[4];
	float Tens, Cont, Bias, EaseIn, EaseOut;
	float Axis[3];
	float Angle;
	float Extra[8];
};

/// TCB Scale (0x442315, 0) — chunk 0x2523. Max ScaleValue (per-axis S + axis-system quat Q)
/// followed by the TCB params, same field order as the other TCB records; decoded against
/// weapon-box scale tracks in the character anim corpus (fy_hof_a_stun_end Ma_Epee2M et al).
struct CStorageTCBScaleKey
{
	sint32 Time;
	uint32 Flags;
	float S[3];
	float Q[4];
	float Tens, Cont, Bias, EaseIn, EaseOut;
	float Extra[14];
};

/**
 * \brief CControlKeyFramerBase
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * Shared machinery for the typed keyframe controllers. Claims the controller's known chunks
 * head-first in file order (default value, 0x2500, 0x3002, 0x3003 range, 0x2532/33/34,
 * key table, 0x3005), stopping at the first unrecognized id so unknown chunks stay orphaned
 * pass-through. All claimed chunks are re-emitted verbatim in original order by build();
 * the key table and range are additionally exposed through typed read accessors. The raw
 * bytes remain authoritative (no authoring direction yet), so roundtrip is byte-exact by
 * construction.
 */
class CControlKeyFramerBase : public CReferenceTarget
{
public:
	CControlKeyFramerBase(CScene *scene, uint16 defaultChunkId, uint16 keyChunkId, uint keySize);
	virtual ~CControlKeyFramerBase() NL_OVERRIDE;

	// inherited
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;
	virtual void init() NL_OVERRIDE;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const NL_OVERRIDE;

	// read access
	/// Number of keys in the key table (0 when absent or when the table size is not an exact
	/// multiple of the record size — the latter never happens in the corpus, but a variant
	/// layout degrades to "no typed keys" rather than misreading).
	uint keyCount() const;
	/// Raw pointer to the first key record, NULL when keyCount() == 0.
	const void *keyData() const;
	/// Default value bytes (chunk 0x2503/0x2504/0x2505/0x2501 depending on the controller
	/// type), NULL when absent. Size returned in sizeOut.
	const uint8 *defaultValue(uint &sizeOut) const;
	/// Controller time range from chunk 0x3003 (Max Interval, 2 x sint32 ticks).
	bool range(sint32 &start, sint32 &end) const;

	//! \name Value at t=0 (tick 0)
	//! The value this controller drives at time 0 — the key table bracketed at tick 0 (linear
	//! interpolation between the surrounding keys for the Linear controllers), else the
	//! default-value chunk. Raw component form (no external math types). Returns false when no
	//! value can be produced. This is the same "eval at t=0" the export samples: a property
	//! driven by a controller that happens to be correct at frame 0 resolves here, rather than
	//! being read as a static field. `out` widths: float[1] / pos float[3] / rot quat float[4] /
	//! scale float[7] (per-axis scale s[0..2] + axis-system quat q[3..6]).
	//@{
	bool floatValueAt0(float &out) const;
	bool posValueAt0(float out[3]) const;
	bool rotValueAt0(float out[4]) const;
	bool scaleValueAt0(float out[7]) const;
	//@}

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	bool isKnownChunkId(uint16 id) const;

	uint16 m_DefaultChunkId;
	uint16 m_KeyChunkId;
	uint m_KeySize;

	/// Chunks claimed off the orphan list head, in original file order; re-emitted verbatim.
	TStorageObjectContainer m_Claimed;

	// Views into m_Claimed (not owned separately)
	CStorageRaw *m_KeyTable;
	CStorageRaw *m_Default;
	CStorageRaw *m_Range;

}; /* class CControlKeyFramerBase */

/// Declare one concrete typed controller class. They differ only in identity and key layout.
#define PMB_DECLARE_CONTROL_KEYFRAMER(className, keyType)                                         \
	class className : public CControlKeyFramerBase                                                \
	{                                                                                             \
	public:                                                                                       \
		className(CScene *scene);                                                                 \
		virtual ~className() NL_OVERRIDE;                                                         \
		static const ucstring DisplayName;                                                        \
		static const char *InternalName;                                                          \
		static const NLMISC::CClassId ClassId;                                                    \
		static const TSClassId SuperClassId;                                                      \
		virtual bool inherits(const NLMISC::CClassId classId) const NL_OVERRIDE;                  \
		virtual const ISceneClassDesc *classDesc() const NL_OVERRIDE;                             \
		inline const keyType *keys() const { return (const keyType *)keyData(); }                 \
	};                                                                                            \
	typedef CSceneClassDesc<className> className##ClassDesc;                                      \
	extern const className##ClassDesc className##Desc;

PMB_DECLARE_CONTROL_KEYFRAMER(CControlPosLinear, CStorageLinPoint3Key)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlRotLinear, CStorageLinRotKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlScaleLinear, CStorageLinScaleKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlFloatLinear, CStorageLinFloatKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlFloatBezier, CStorageBezFloatKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlPosBezier, CStorageBezPoint3Key)
// Bezier Point3 (0x200A, CTRL_POINT3 0x9005) and Bezier Color (0x2011, CTRL_COLOR 0x9009)
// share the Position Bezier key table layout (chunk 0x2526): the color / light-group anim
// path. Same storage as CControlPosBezier; distinct class ids so the ClassDirectory3 lookup
// instantiates the typed keyframer instead of a raw unknown.
PMB_DECLARE_CONTROL_KEYFRAMER(CControlPoint3Bezier, CStorageBezPoint3Key)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlColorBezier, CStorageBezPoint3Key)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlScaleBezier, CStorageBezScaleKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlPosTCB, CStorageTCBPoint3Key)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlRotTCB, CStorageTCBRotKey)
PMB_DECLARE_CONTROL_KEYFRAMER(CControlScaleTCB, CStorageTCBScaleKey)
// TCB Point3 (0x442314) — same keys as TCB Position, color/point3 role.
PMB_DECLARE_CONTROL_KEYFRAMER(CControlPoint3TCB, CStorageTCBPoint3Key)

#undef PMB_DECLARE_CONTROL_KEYFRAMER

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CONTROL_KEYFRAMER_H */

/* end of file */
