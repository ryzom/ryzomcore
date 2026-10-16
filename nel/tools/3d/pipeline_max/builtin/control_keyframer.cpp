/**
 * \file control_keyframer.cpp
 * \brief CControlKeyFramer
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * Typed classes for the builtin keyframe animation controllers.
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
#include "control_keyframer.h"

// STL includes
#include <cstring>

// NeL includes
#include <nel/misc/debug.h> // nlctassert

// Project includes

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

// The typed accessors reinterpret the raw chunk bytes; the record layouts must be packed.
nlctassert(sizeof(CStorageLinPoint3Key) == 20);
nlctassert(sizeof(CStorageLinRotKey) == 24);
nlctassert(sizeof(CStorageLinScaleKey) == 36);
nlctassert(sizeof(CStorageBezFloatKey) == 28);
nlctassert(sizeof(CStorageBezPoint3Key) == 80);
nlctassert(sizeof(CStorageBezScaleKey) == 148);
nlctassert(sizeof(CStorageTCBPoint3Key) == 64);
nlctassert(sizeof(CStorageTCBRotKey) == 92);
nlctassert(sizeof(CStorageTCBScaleKey) == 112);

// Common controller chunk ids (shared across all keyframe controller classes; the default
// value and key table ids are per-class and passed in by the concrete constructors).
#define PMB_CTRL_UNKNOWN2500_CHUNK_ID 0x2500
#define PMB_CTRL_UNKNOWN3002_CHUNK_ID 0x3002
#define PMB_CTRL_RANGE_CHUNK_ID 0x3003
#define PMB_CTRL_UNKNOWN3005_CHUNK_ID 0x3005
#define PMB_CTRL_UNKNOWN2532_CHUNK_ID 0x2532
#define PMB_CTRL_UNKNOWN2533_CHUNK_ID 0x2533
#define PMB_CTRL_UNKNOWN2534_CHUNK_ID 0x2534
// Leading empty marker on Bezier Point3 / Color controllers (before the default value).
// Must be known so the claim loop reaches the 0x2526 key table.
#define PMB_CTRL_POINT3_MARKER_CHUNK_ID 0x8499

CControlKeyFramerBase::CControlKeyFramerBase(CScene *scene, uint16 defaultChunkId, uint16 keyChunkId, uint keySize)
	: CReferenceTarget(scene)
	, m_DefaultChunkId(defaultChunkId)
	, m_KeyChunkId(keyChunkId)
	, m_KeySize(keySize)
	, m_KeyTable(nullptr)
	, m_Default(nullptr)
	, m_Range(nullptr)
{

}

CControlKeyFramerBase::~CControlKeyFramerBase()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
			delete it->second;
		m_Claimed.clear();
	}
}

bool CControlKeyFramerBase::isKnownChunkId(uint16 id) const
{
	switch (id)
	{
	case PMB_CTRL_UNKNOWN2500_CHUNK_ID:
	case PMB_CTRL_UNKNOWN3002_CHUNK_ID:
	case PMB_CTRL_RANGE_CHUNK_ID:
	case PMB_CTRL_UNKNOWN3005_CHUNK_ID:
	case PMB_CTRL_UNKNOWN2532_CHUNK_ID:
	case PMB_CTRL_UNKNOWN2533_CHUNK_ID:
	case PMB_CTRL_UNKNOWN2534_CHUNK_ID:
	case PMB_CTRL_POINT3_MARKER_CHUNK_ID: // 0x8499 — Point3/Color leading marker
		return true;
	}
	if (id == m_DefaultChunkId) return true;
	if (id == m_KeyChunkId) return true;
	return false;
}

void CControlKeyFramerBase::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		// Claim known chunks off the head of the orphan list, in file order, stopping at the
		// first unrecognized id. getChunk on the head never triggers the out-of-order warning,
		// and anything after an unknown id simply stays orphaned (verbatim pass-through).
		for (;;)
		{
			uint16 id = peekChunk();
			if (id == 0x0000) break;
			if (!isKnownChunkId(id)) break;
			IStorageObject *so = getChunk(id);
			if (!so) break;
			m_Claimed.push_back(TStorageObjectWithId(id, so));
			if (id == m_KeyChunkId) m_KeyTable = dynamic_cast<CStorageRaw *>(so);
			else if (id == m_DefaultChunkId) m_Default = dynamic_cast<CStorageRaw *>(so);
			else if (id == PMB_CTRL_RANGE_CHUNK_ID) m_Range = dynamic_cast<CStorageRaw *>(so);
		}
	}
}

void CControlKeyFramerBase::clean()
{
	CReferenceTarget::clean();
}

void CControlKeyFramerBase::build(uint16 version, uint filter)
{
	CReferenceTarget::build(version);
	for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
		putChunk(it->first, it->second);
}

void CControlKeyFramerBase::disown()
{
	m_Claimed.clear();
	m_KeyTable = nullptr;
	m_Default = nullptr;
	m_Range = nullptr;
	CReferenceTarget::disown();
}

void CControlKeyFramerBase::init()
{
	CReferenceTarget::init();
}

void CControlKeyFramerBase::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	uint nb = keyCount();
	if (nb)
	{
		ostream << "\n" << pad << "Keys: " << nb;
	}
	else if (m_KeyTable)
	{
		ostream << "\n" << pad << "Keys: UNALIGNED " << m_KeyTable->Value.size() << " bytes / " << m_KeySize;
	}
	sint32 rs, re;
	if (range(rs, re))
	{
		ostream << "\n" << pad << "Range: " << rs << " to " << re << " ticks";
	}
}

uint CControlKeyFramerBase::keyCount() const
{
	if (!m_KeyTable) return 0;
	if (m_KeySize == 0) return 0;
	if (m_KeyTable->Value.size() % m_KeySize) return 0;
	return (uint)(m_KeyTable->Value.size() / m_KeySize);
}

const void *CControlKeyFramerBase::keyData() const
{
	if (!keyCount()) return nullptr;
	return nlVectorData(m_KeyTable->Value);
}

const uint8 *CControlKeyFramerBase::defaultValue(uint &sizeOut) const
{
	if (!m_Default) { sizeOut = 0; return nullptr; }
	sizeOut = (uint)m_Default->Value.size();
	return nlVectorData(m_Default->Value);
}

bool CControlKeyFramerBase::range(sint32 &start, sint32 &end) const
{
	if (!m_Range) return false;
	if (m_Range->Value.size() != 8) return false;
	memcpy(&start, nlVectorData(m_Range->Value), 4);
	memcpy(&end, nlVectorData(m_Range->Value) + 4, 4);
	return true;
}

IStorageObject *CControlKeyFramerBase::createChunkById(uint16 id, bool container)
{
	// All leaf chunks on these controllers default to CStorageRaw already; containers
	// (0x2532/33/34) stay generic containers. Nothing to specialize.
	return CReferenceTarget::createChunkById(id, container);
}

// ---------------------------------------------------------------------------------------------
// Value at t=0

// Index of the key bracketing tick 0; sets lerpNext + lerpFactor when 0 falls strictly between
// two keys (used only by the Linear controllers, which interpolate).
template <typename TKey>
static uint kfKeyIndexAt0(const TKey *keys, uint numKeys, bool &lerpNext, float &lerpFactor)
{
	lerpNext = false;
	lerpFactor = 0.0f;
	if (keys[0].Time >= 0) return 0;
	if (keys[numKeys - 1].Time <= 0) return numKeys - 1;
	for (uint i = 0; i + 1 < numKeys; ++i)
	{
		if (keys[i].Time <= 0 && keys[i + 1].Time >= 0)
		{
			if (keys[i + 1].Time == 0) return i + 1;
			if (keys[i].Time == 0) return i;
			lerpNext = true;
			lerpFactor = (0.0f - (float)keys[i].Time) / ((float)keys[i + 1].Time - (float)keys[i].Time);
			return i;
		}
	}
	return 0;
}

bool CControlKeyFramerBase::floatValueAt0(float &out) const
{
	if (keyCount())
	{
		bool l; float f;
		if (const CControlFloatLinear *c = dynamic_cast<const CControlFloatLinear *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			out = c->keys()[i].Val;
			if (l) out += f * (c->keys()[i + 1].Val - c->keys()[i].Val);
			return true;
		}
		if (const CControlFloatBezier *c = dynamic_cast<const CControlFloatBezier *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			out = c->keys()[i].Val;
			return true;
		}
	}
	uint sz = 0;
	const uint8 *d = defaultValue(sz);
	if (d && sz >= 4) { memcpy(&out, d, 4); return true; }
	return false;
}

bool CControlKeyFramerBase::posValueAt0(float out[3]) const
{
	if (keyCount())
	{
		bool l; float f;
		if (const CControlPosLinear *c = dynamic_cast<const CControlPosLinear *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			const CStorageLinPoint3Key *k = c->keys();
			out[0] = k[i].Val[0]; out[1] = k[i].Val[1]; out[2] = k[i].Val[2];
			if (l)
			{
				out[0] += f * (k[i + 1].Val[0] - k[i].Val[0]);
				out[1] += f * (k[i + 1].Val[1] - k[i].Val[1]);
				out[2] += f * (k[i + 1].Val[2] - k[i].Val[2]);
			}
			return true;
		}
		if (const CControlPosBezier *c = dynamic_cast<const CControlPosBezier *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			out[0] = c->keys()[i].Val[0]; out[1] = c->keys()[i].Val[1]; out[2] = c->keys()[i].Val[2];
			return true;
		}
		if (const CControlPosTCB *c = dynamic_cast<const CControlPosTCB *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			out[0] = c->keys()[i].Val[0]; out[1] = c->keys()[i].Val[1]; out[2] = c->keys()[i].Val[2];
			return true;
		}
	}
	uint sz = 0;
	const uint8 *d = defaultValue(sz);
	if (d && sz >= 12) { memcpy(out, d, 12); return true; }
	return false;
}

bool CControlKeyFramerBase::rotValueAt0(float out[4]) const
{
	if (keyCount())
	{
		bool l; float f;
		if (const CControlRotLinear *c = dynamic_cast<const CControlRotLinear *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			memcpy(out, c->keys()[i].Quat, 16);
			return true;
		}
		if (const CControlRotTCB *c = dynamic_cast<const CControlRotTCB *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			memcpy(out, c->keys()[i].AbsQuat, 16);
			return true;
		}
	}
	uint sz = 0;
	const uint8 *d = defaultValue(sz);
	if (d && sz >= 16) { memcpy(out, d, 16); return true; }
	return false;
}

bool CControlKeyFramerBase::scaleValueAt0(float out[7]) const
{
	// identity default
	out[0] = out[1] = out[2] = 1.0f;
	out[3] = out[4] = out[5] = 0.0f; out[6] = 1.0f;
	if (keyCount())
	{
		bool l; float f;
		if (const CControlScaleLinear *c = dynamic_cast<const CControlScaleLinear *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			memcpy(out, c->keys()[i].S, 12); memcpy(out + 3, c->keys()[i].Q, 16);
			return true;
		}
		if (const CControlScaleBezier *c = dynamic_cast<const CControlScaleBezier *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			memcpy(out, c->keys()[i].S, 12); memcpy(out + 3, c->keys()[i].Q, 16);
			return true;
		}
		if (const CControlScaleTCB *c = dynamic_cast<const CControlScaleTCB *>(this))
		{
			uint i = kfKeyIndexAt0(c->keys(), keyCount(), l, f);
			memcpy(out, c->keys()[i].S, 12); memcpy(out + 3, c->keys()[i].Q, 16);
			return true;
		}
	}
	uint sz = 0;
	const uint8 *d = defaultValue(sz);
	if (d && sz >= 28) { memcpy(out, d, 12); memcpy(out + 3, d + 12, 16); return true; }
	if (d && sz >= 12) { memcpy(out, d, 12); return true; }
	return false;
}

////////////////////////////////////////////////////////////////////////
// Concrete classes
////////////////////////////////////////////////////////////////////////

// Default-value chunk ids per controller value type
#define PMB_CTRL_DEFAULT_FLOAT_CHUNK_ID 0x2501
// Point3 / Color controllers store their 12-byte default under 0x2501 (same id as the
// float default, but 12 bytes = RGB triple) — observed on every light-color Bezier Point3
// in stuff/animated_light/fyros_city_animated_lights.max.
#define PMB_CTRL_DEFAULT_POINT3_CHUNK_ID 0x2501
#define PMB_CTRL_DEFAULT_POS_CHUNK_ID 0x2503
#define PMB_CTRL_DEFAULT_ROT_CHUNK_ID 0x2504
#define PMB_CTRL_DEFAULT_SCALE_CHUNK_ID 0x2505

// Key-table chunk ids
#define PMB_CTRL_KEYS_LIN_FLOAT_CHUNK_ID 0x2511
#define PMB_CTRL_KEYS_LIN_POS_CHUNK_ID 0x2513
#define PMB_CTRL_KEYS_LIN_ROT_CHUNK_ID 0x2514
#define PMB_CTRL_KEYS_LIN_SCALE_CHUNK_ID 0x2515
#define PMB_CTRL_KEYS_BEZ_FLOAT_CHUNK_ID 0x2525
#define PMB_CTRL_KEYS_BEZ_POS_CHUNK_ID 0x2526
#define PMB_CTRL_KEYS_BEZ_SCALE_CHUNK_ID 0x2528
#define PMB_CTRL_KEYS_TCB_POS_CHUNK_ID 0x2521
#define PMB_CTRL_KEYS_TCB_ROT_CHUNK_ID 0x2522
#define PMB_CTRL_KEYS_TCB_SCALE_CHUNK_ID 0x2523

// Superclass ids
#define PMB_SCLASS_CONTROL_FLOAT 0x00009003
#define PMB_SCLASS_CONTROL_POINT3 0x00009005
#define PMB_SCLASS_CONTROL_COLOR 0x00009009
#define PMB_SCLASS_CONTROL_POS 0x0000900b
#define PMB_SCLASS_CONTROL_ROT 0x0000900c
#define PMB_SCLASS_CONTROL_SCALE 0x0000900d

#define PMB_DEFINE_CONTROL_KEYFRAMER(className, displayName, internalName, classIdA, sclassId, defaultChunk, keyChunk, keyType) \
	className::className(CScene *scene) : CControlKeyFramerBase(scene, defaultChunk, keyChunk, sizeof(keyType)) { } \
	className::~className() { } \
	const ucstring className::DisplayName = ucstring(displayName); \
	const char *className::InternalName = internalName; \
	const NLMISC::CClassId className::ClassId = NLMISC::CClassId(classIdA, 0x00000000); \
	const TSClassId className::SuperClassId = sclassId; \
	const className##ClassDesc className##Desc(&DllPluginDescBuiltin); \
	bool className::inherits(const NLMISC::CClassId classId) const \
	{ \
		if (classId == classDesc()->classId()) return true; \
		return CControlKeyFramerBase::inherits(classId); \
	} \
	const ISceneClassDesc *className::classDesc() const { return &className##Desc; }

PMB_DEFINE_CONTROL_KEYFRAMER(CControlPosLinear, "Linear Position", "ControlPosLinear", 0x00002002, PMB_SCLASS_CONTROL_POS, PMB_CTRL_DEFAULT_POS_CHUNK_ID, PMB_CTRL_KEYS_LIN_POS_CHUNK_ID, CStorageLinPoint3Key)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlRotLinear, "Linear Rotation", "ControlRotLinear", 0x00002003, PMB_SCLASS_CONTROL_ROT, PMB_CTRL_DEFAULT_ROT_CHUNK_ID, PMB_CTRL_KEYS_LIN_ROT_CHUNK_ID, CStorageLinRotKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlScaleLinear, "Linear Scale", "ControlScaleLinear", 0x00002004, PMB_SCLASS_CONTROL_SCALE, PMB_CTRL_DEFAULT_SCALE_CHUNK_ID, PMB_CTRL_KEYS_LIN_SCALE_CHUNK_ID, CStorageLinScaleKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlFloatLinear, "Linear Float", "ControlFloatLinear", 0x00002001, PMB_SCLASS_CONTROL_FLOAT, PMB_CTRL_DEFAULT_FLOAT_CHUNK_ID, PMB_CTRL_KEYS_LIN_FLOAT_CHUNK_ID, CStorageLinFloatKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlFloatBezier, "Bezier Float", "ControlFloatBezier", 0x00002007, PMB_SCLASS_CONTROL_FLOAT, PMB_CTRL_DEFAULT_FLOAT_CHUNK_ID, PMB_CTRL_KEYS_BEZ_FLOAT_CHUNK_ID, CStorageBezFloatKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlPosBezier, "Bezier Position", "ControlPosBezier", 0x00002008, PMB_SCLASS_CONTROL_POS, PMB_CTRL_DEFAULT_POS_CHUNK_ID, PMB_CTRL_KEYS_BEZ_POS_CHUNK_ID, CStorageBezPoint3Key)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlPoint3Bezier, "Bezier Point3", "ControlPoint3Bezier", 0x0000200a, PMB_SCLASS_CONTROL_POINT3, PMB_CTRL_DEFAULT_POINT3_CHUNK_ID, PMB_CTRL_KEYS_BEZ_POS_CHUNK_ID, CStorageBezPoint3Key)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlColorBezier, "Bezier Color", "ControlColorBezier", 0x00002011, PMB_SCLASS_CONTROL_COLOR, PMB_CTRL_DEFAULT_POINT3_CHUNK_ID, PMB_CTRL_KEYS_BEZ_POS_CHUNK_ID, CStorageBezPoint3Key)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlScaleBezier, "Bezier Scale", "ControlScaleBezier", 0x00002010, PMB_SCLASS_CONTROL_SCALE, PMB_CTRL_DEFAULT_SCALE_CHUNK_ID, PMB_CTRL_KEYS_BEZ_SCALE_CHUNK_ID, CStorageBezScaleKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlPosTCB, "TCB Position", "ControlPosTCB", 0x00442312, PMB_SCLASS_CONTROL_POS, PMB_CTRL_DEFAULT_POS_CHUNK_ID, PMB_CTRL_KEYS_TCB_POS_CHUNK_ID, CStorageTCBPoint3Key)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlRotTCB, "TCB Rotation", "ControlRotTCB", 0x00442313, PMB_SCLASS_CONTROL_ROT, PMB_CTRL_DEFAULT_ROT_CHUNK_ID, PMB_CTRL_KEYS_TCB_ROT_CHUNK_ID, CStorageTCBRotKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlScaleTCB, "TCB Scale", "ControlScaleTCB", 0x00442315, PMB_SCLASS_CONTROL_SCALE, PMB_CTRL_DEFAULT_SCALE_CHUNK_ID, PMB_CTRL_KEYS_TCB_SCALE_CHUNK_ID, CStorageTCBScaleKey)
PMB_DEFINE_CONTROL_KEYFRAMER(CControlPoint3TCB, "TCB Point3", "ControlPoint3TCB", 0x00442314, PMB_SCLASS_CONTROL_POINT3, PMB_CTRL_DEFAULT_POINT3_CHUNK_ID, PMB_CTRL_KEYS_TCB_POS_CHUNK_ID, CStorageTCBPoint3Key)

#undef PMB_DEFINE_CONTROL_KEYFRAMER

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
