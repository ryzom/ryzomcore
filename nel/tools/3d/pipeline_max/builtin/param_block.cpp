/**
 * \file param_block.cpp
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

#include <nel/misc/types_nl.h>
#include "param_block.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "control_keyframer.h"

using namespace std;
// using namespace NLMISC;

// Old ParamBlock chunk stream (corpus-established, stable Max 3 .. Max 2010; see param_block.h):
// object-level 0x0001 (4 bytes) and 0x0005 (2 bytes) leaves stay raw/undecoded; one 0x0002
// container per parameter with 0x0003 index, optional empty 0x0004 marker, and one value leaf
// 0x0100+typecode — or an empty 0x0200 marker for an animated (controller-backed) parameter.
#define PMB_OLDPB_ENTRY_CHUNK_ID 0x0002
#define PMB_OLDPB_INDEX_CHUNK_ID 0x0003
#define PMB_OLDPB_MARKER_CHUNK_ID 0x0004
#define PMB_OLDPB_VALUE_FLOAT_CHUNK_ID 0x0100
#define PMB_OLDPB_VALUE_INT_CHUNK_ID 0x0101
#define PMB_OLDPB_VALUE_RGBA_CHUNK_ID 0x0102
#define PMB_OLDPB_VALUE_POINT3_CHUNK_ID 0x0103
#define PMB_OLDPB_VALUE_BOOL_CHUNK_ID 0x0104
#define PMB_OLDPB_ANIMATED_FLOAT_CHUNK_ID 0x0200
#define PMB_OLDPB_ANIMATED_INT_CHUNK_ID 0x0201
#define PMB_OLDPB_ANIMATED_POINT3_CHUNK_ID 0x0202

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CParamBlock::CParamBlock(CScene *scene) : CReferenceTarget(scene)
{

}

CParamBlock::~CParamBlock()
{

}

const ucstring CParamBlock::DisplayName = ucstring("ParamBlock");
const char *CParamBlock::InternalName = "ParamBlock";
const char *CParamBlock::InternalNameUnknown = "ParamBlockUnknown";
const NLMISC::CClassId CParamBlock::ClassId = NLMISC::CClassId(0x00000008, 0x00000000); /* Not official, please correct */
const TSClassId CParamBlock::SuperClassId = 0x00000008;
const CParamBlockClassDesc ParamBlockClassDesc(&DllPluginDescBuiltin);

void CParamBlock::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
		decodeModel();
}

void CParamBlock::clean()
{
	CReferenceTarget::clean();
}

void CParamBlock::build(uint16 version, uint filter)
{
	// Raw chunks stay authoritative: the base re-emits every (possibly modified in place)
	// orphaned chunk verbatim, so roundtrip is byte-exact for every unmodified parameter.
	CReferenceTarget::build(version);
}

void CParamBlock::disown()
{
	m_Params.clear();
	m_UnknownEntryChunkIds.clear();
	CReferenceTarget::disown();
}

void CParamBlock::init()
{
	CReferenceTarget::init();
}

bool CParamBlock::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CParamBlock::classDesc() const
{
	return &ParamBlockClassDesc;
}

// Decode the parameter entries from the orphaned chunks WITHOUT moving them (the raw chunks
// remain the serialization authority). Reference-slot counting is compact over the ANIMATED
// entries only, in entry order (the mapping anim_build's StdUVGen controller resolution
// established; the waterfall material-anim gate is byte-exact on it).
void CParamBlock::decodeModel()
{
	m_Params.clear();
	m_UnknownEntryChunkIds.clear();

	sint refSlot = 0;
	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != PMB_OLDPB_ENTRY_CHUNK_ID) continue;
		CStorageContainer *entry = dynamic_cast<CStorageContainer *>(it->second);
		if (!entry) continue;
		SParam p;
		for (TStorageObjectConstIt cit = entry->chunks().begin(); cit != entry->chunks().end(); ++cit)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(cit->second);
			if (!raw) { m_UnknownEntryChunkIds.push_back(cit->first); continue; }
			switch (cit->first)
			{
			case PMB_OLDPB_INDEX_CHUNK_ID:
				if (raw->Value.size() == 4) memcpy(&p.Index, nlVectorData(raw->Value), 4);
				break;
			case PMB_OLDPB_MARKER_CHUNK_ID:
				// Empty per-param marker, never a value.
				break;
			case PMB_OLDPB_VALUE_FLOAT_CHUNK_ID:
			case PMB_OLDPB_VALUE_INT_CHUNK_ID:
			case PMB_OLDPB_VALUE_BOOL_CHUNK_ID:
				if (raw->Value.size() == 4)
				{
					p.Kind = cit->first == PMB_OLDPB_VALUE_FLOAT_CHUNK_ID ? KindFloat
						: cit->first == PMB_OLDPB_VALUE_INT_CHUNK_ID ? KindInt : KindBool;
					p.ValueChunkId = cit->first;
					p.HasConstant = true;
					// Both views are bit-copies of the same stored dword (see SParam).
					memcpy(&p.F[0], nlVectorData(raw->Value), 4);
					memcpy(&p.I, nlVectorData(raw->Value), 4);
					p.Chunk = raw;
				}
				break;
			case PMB_OLDPB_VALUE_RGBA_CHUNK_ID:
			case PMB_OLDPB_VALUE_POINT3_CHUNK_ID:
				if (raw->Value.size() == 12)
				{
					p.Kind = KindPoint3;
					p.ValueChunkId = cit->first;
					p.HasConstant = true;
					memcpy(p.F, nlVectorData(raw->Value), 12);
					p.Chunk = raw;
				}
				break;
			case PMB_OLDPB_ANIMATED_FLOAT_CHUNK_ID:
			case PMB_OLDPB_ANIMATED_INT_CHUNK_ID:
			case PMB_OLDPB_ANIMATED_POINT3_CHUNK_ID:
				// The value leaf is replaced by this empty marker (0x0200 + the type code —
				// float/int/rgba-point3); the controller occupies the block's next compact
				// reference slot regardless of kind (verified corpus-wide: the 0x2034 reference
				// array length equals the total marker count of all three ids, and mixed blocks
				// order slots by entry order — the animated-light color (0x0202, entry 0) takes
				// slot 0 ahead of its two float params' controllers).
				p.Animated = true;
				p.ValueChunkId = cit->first;
				break;
			default:
				m_UnknownEntryChunkIds.push_back(cit->first);
				break;
			}
		}
		if (p.Animated)
			p.RefSlot = refSlot++;
		m_Params.push_back(p);
	}
}

const CParamBlock::SParam *CParamBlock::findParam(sint32 index) const
{
	for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
		if (it->Index == index) return &(*it);
	return nullptr;
}

CParamBlock::SParam *CParamBlock::findParamMutable(sint32 index)
{
	for (std::vector<SParam>::iterator it = m_Params.begin(); it != m_Params.end(); ++it)
		if (it->Index == index) return &(*it);
	return nullptr;
}

bool CParamBlock::getFloat(sint32 index, float &out) const
{
	const SParam *p = findParam(index);
	if (!p || !p->HasConstant || p->Kind != KindFloat) return false;
	out = p->F[0];
	return true;
}

bool CParamBlock::getInt(sint32 index, sint32 &out) const
{
	const SParam *p = findParam(index);
	if (!p || !p->HasConstant || (p->Kind != KindInt && p->Kind != KindBool)) return false;
	out = p->I;
	return true;
}

bool CParamBlock::getPoint3(sint32 index, float out[3]) const
{
	const SParam *p = findParam(index);
	if (!p || !p->HasConstant || p->Kind != KindPoint3) return false;
	out[0] = p->F[0];
	out[1] = p->F[1];
	out[2] = p->F[2];
	return true;
}

bool CParamBlock::getFloatAt0(sint32 index, float &out) const
{
	const SParam *p = findParam(index);
	if (!p) return false;
	if (p->HasConstant && p->Kind == KindFloat) { out = p->F[0]; return true; }
	// Animated — resolve the compact reference slot to its keyframer at tick 0.
	if (p->Animated && p->RefSlot >= 0)
		if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(getReference((uint)p->RefSlot)))
			return kf->floatValueAt0(out);
	return false;
}

CReferenceMaker *CParamBlock::controllerForParam(sint32 index) const
{
	// First ANIMATED entry with this declared index, in entry order (exactly the walk the
	// exporter's StdUVGen controller resolution used — not findParam, which would stop at a
	// same-index constant entry if a block ever carried duplicates).
	for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
		if (it->Animated && it->Index == index && it->RefSlot >= 0)
			return getReference((uint)it->RefSlot);
	return nullptr;
}

// ---------------------------------------------------------------------------------------------
// In-place modify: rewrite the bytes of the owning value leaf. Every other chunk is untouched,
// so a modified block still roundtrips byte-exactly except for the changed value.

bool CParamBlock::setFloat(sint32 index, float v)
{
	SParam *p = findParamMutable(index);
	if (!p || !p->HasConstant || !p->Chunk || p->Kind != KindFloat) return false;
	if (p->Chunk->Value.size() < 4) return false;
	memcpy(nlVectorData(p->Chunk->Value), &v, 4);
	memcpy(&p->F[0], &v, 4);
	memcpy(&p->I, &v, 4);
	return true;
}

bool CParamBlock::setInt(sint32 index, sint32 v)
{
	SParam *p = findParamMutable(index);
	if (!p || !p->HasConstant || !p->Chunk || (p->Kind != KindInt && p->Kind != KindBool)) return false;
	if (p->Chunk->Value.size() < 4) return false;
	memcpy(nlVectorData(p->Chunk->Value), &v, 4);
	memcpy(&p->F[0], &v, 4);
	memcpy(&p->I, &v, 4);
	return true;
}

bool CParamBlock::setPoint3(sint32 index, const float pt[3])
{
	SParam *p = findParamMutable(index);
	if (!p || !p->HasConstant || !p->Chunk || p->Kind != KindPoint3) return false;
	if (p->Chunk->Value.size() < 12) return false;
	memcpy(nlVectorData(p->Chunk->Value), pt, 12);
	p->F[0] = pt[0]; p->F[1] = pt[1]; p->F[2] = pt[2];
	return true;
}

bool CParamBlock::selfTestReencode(std::string &err) const
{
	for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
	{
		const SParam &p = *it;
		if (!p.HasConstant || !p.Chunk) continue;
		uint sz = 0;
		uint8 buf[12];
		switch (p.Kind)
		{
		case KindFloat:
		case KindInt:
		case KindBool:
			memcpy(buf, &p.I, 4); sz = 4; break;
		case KindPoint3:
			memcpy(buf, p.F, 12); sz = 12; break;
		default:
			continue;
		}
		if (p.Chunk->Value.size() != sz
			|| memcmp(buf, nlVectorData(p.Chunk->Value), sz) != 0)
		{
			std::stringstream ss;
			ss << "param " << p.Index << " value chunk 0x" << std::hex << p.ValueChunkId << " re-encode mismatch";
			err = ss.str();
			return false;
		}
	}
	return true;
}

void CParamBlock::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (!m_Params.empty())
	{
		uint nAnim = 0;
		for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
			if (it->Animated) ++nAnim;
		ostream << "\n" << pad << "ParamBlock: " << m_Params.size() << " params, " << nAnim << " animated";
	}
}

IStorageObject *CParamBlock::createChunkById(uint16 id, bool container)
{
	// The 0x0002 parameter entries default to CStorageContainer and every leaf (0x0001, 0x0005,
	// and the entry contents) to CStorageRaw already; the raw bytes stay authoritative.
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
