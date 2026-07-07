/**
 * \file param_block_2.cpp
 * \brief CParamBlock2
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
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

#include <nel/misc/types_nl.h>
#include "param_block_2.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes

// Project includes
#include "control_keyframer.h"

using namespace std;
// using namespace NLMISC;

#define PMB_PB2_HEADER_CHUNK_ID 0x0009
#define PMB_PB2_PARAM_CHUNK_ID 0x000e

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

bool CParamBlock2::SParam::typeIsRefKind(uint16 type)
{
	switch (type & 0x07ff)
	{
	case TYPE_MTL:
	case TYPE_TEXMAP:
	case TYPE_NODE:
	case TYPE_REFTARG:
		return true;
	}
	return false;
}

CParamBlock2::CParamBlock2(CScene *scene) : CReferenceTarget(scene),
	m_HasHeader(false), m_ScriptVersion(0), m_BlockId(0), m_ParamCount(0)
{

}

CParamBlock2::~CParamBlock2()
{

}

const ucstring CParamBlock2::DisplayName = ucstring("ParamBlock2");
const char *CParamBlock2::InternalName = "ParamBlock2";
const char *CParamBlock2::InternalNameUnknown = "ParamBlock2Unknown";
const NLMISC::CClassId CParamBlock2::ClassId = NLMISC::CClassId(0x00000082, 0x00000000); /* Not official, please correct */
const TSClassId CParamBlock2::SuperClassId = 0x00000082;
const CParamBlock2ClassDesc ParamBlock2ClassDesc(&DllPluginDescBuiltin);

void CParamBlock2::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
		decodeModel();
}

void CParamBlock2::clean()
{
	CReferenceTarget::clean();
}

void CParamBlock2::build(uint16 version, uint filter)
{
	// Raw chunks stay authoritative: the base re-emits every (possibly modified in place)
	// orphaned chunk verbatim, so roundtrip is byte-exact for every unmodified parameter.
	CReferenceTarget::build(version);
}

void CParamBlock2::disown()
{
	m_Params.clear();
	m_HasHeader = false;
	m_ScriptVersion = 0;
	m_BlockId = 0;
	m_ParamCount = 0;
	CReferenceTarget::disown();
}

void CParamBlock2::init()
{
	CReferenceTarget::init();
}

bool CParamBlock2::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CParamBlock2::classDesc() const
{
	return &ParamBlock2ClassDesc;
}

// Decode the header + parameter records from the orphaned chunks WITHOUT moving them (the raw
// chunks remain the serialization authority). Reference-slot counting mirrors the record order
// exactly so refValue resolves the same reference the original param block would.
void CParamBlock2::decodeModel()
{
	m_Params.clear();
	m_HasHeader = false;
	m_ScriptVersion = 0;
	m_BlockId = 0;
	m_ParamCount = 0;

	sint refSlot = 0;
	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) continue;
		if (it->first == PMB_PB2_HEADER_CHUNK_ID && raw->Value.size() >= 16)
		{
			memcpy(&m_ScriptVersion, raw->Value.data(), 4);
			memcpy(&m_BlockId, raw->Value.data() + 4, 2);
			memcpy(&m_ParamCount, raw->Value.data() + 10, 2);
			m_HasHeader = true;
		}
		else if (it->first == PMB_PB2_PARAM_CHUNK_ID && raw->Value.size() >= 15)
		{
			SParam p;
			memcpy(&p.Id, raw->Value.data(), 2);
			memcpy(&p.Type, raw->Value.data() + 2, 2);
			p.Chunk = raw;
			p.IsTab = (p.Type & TYPE_TAB_FLAG) != 0;
			uint8 flagByte = raw->Value[14];
			bool refKind = SParam::typeIsRefKind(p.baseType());
			bool isConstant = (flagByte & 0x40) != 0;
			const uint8 *payload = raw->Value.data() + 15;
			size_t payloadSize = raw->Value.size() - 15;

			if (p.IsTab)
			{
				// Tab record: flag byte at [14] (0x00), then u32 count, then per element a u8
				// flag (0x40 = inline) + value by base type. Reference-kind element values are
				// scene storage indices (resolved through the scene container by the consumer),
				// not PB2 reference slots — a tab record does NOT own a PB2 reference slot.
				const uint8 *q = raw->Value.data() + 14 + 1;
				const uint8 *end = raw->Value.data() + raw->Value.size();
				if (q + 4 <= end)
				{
					uint32 count;
					memcpy(&count, q, 4);
					q += 4;
					uint16 base = p.baseType();
					uint elemSize = (base == TYPE_RGBA || base == TYPE_POINT3 || base == TYPE_HSV) ? 12 : 4;
					for (uint32 e = 0; e < count && q + 1 + elemSize <= end; ++e)
					{
						++q; // element flag byte
						if (elemSize == 4)
						{
							sint32 iv; float fv;
							memcpy(&iv, q, 4);
							memcpy(&fv, q, 4);
							p.TabI.push_back(iv);
							p.TabF.push_back(fv);
						}
						else
						{
							float fv[3];
							memcpy(fv, q, 12);
							p.TabF.push_back(fv[0]);
							p.TabF.push_back(fv[1]);
							p.TabF.push_back(fv[2]);
							p.TabI.push_back(0);
						}
						q += elemSize;
					}
					p.HasConstant = true;
				}
				m_Params.push_back(p);
				continue;
			}

			if (refKind || !isConstant)
			{
				// reftarget-kind params and controller-backed value params own the PB2's
				// reference slots in record order
				p.RefBacked = true;
				p.RefSlot = refSlot++;
			}
			if (isConstant && !refKind && payloadSize > 0)
			{
				p.HasConstant = true;
				p.PayloadOff = 15;
				switch (p.baseType())
				{
				case TYPE_FLOAT:
				case TYPE_ANGLE:
				case TYPE_PCNT_FRAC:
				case TYPE_WORLD:
				case TYPE_COLOR_CHANNEL:
					if (payloadSize >= 4) memcpy(&p.F[0], payload, 4);
					else p.HasConstant = false;
					break;
				case TYPE_INT:
				case TYPE_BOOL:
				case TYPE_TIMEVALUE:
				case TYPE_RADIOBTN_INDEX:
					if (payloadSize >= 4) { memcpy(&p.I, payload, 4); p.F[0] = (float)p.I; }
					else p.HasConstant = false;
					break;
				case TYPE_RGBA:
				case TYPE_POINT3:
				case TYPE_HSV:
					if (payloadSize >= 12) memcpy(p.F, payload, 12);
					else p.HasConstant = false;
					break;
				case TYPE_STRING:
				case TYPE_FILENAME:
					if (payloadSize >= 4)
					{
						uint32 len;
						memcpy(&len, payload, 4);
						if (len > payloadSize - 4) len = (uint32)(payloadSize - 4);
						std::string s((const char *)payload + 4, len);
						while (!s.empty() && s[s.size() - 1] == '\0') s.resize(s.size() - 1);
						p.S = s;
					}
					break;
				default:
					// unknown scalar types: keep the record id/type, no decoded value
					p.HasConstant = false;
					break;
				}
			}
			m_Params.push_back(p);
		}
	}
}

const CParamBlock2::SParam *CParamBlock2::findParam(uint16 id) const
{
	for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
		if (it->Id == id) return &(*it);
	return NULL;
}

CParamBlock2::SParam *CParamBlock2::findParamMutable(uint16 id)
{
	for (std::vector<SParam>::iterator it = m_Params.begin(); it != m_Params.end(); ++it)
		if (it->Id == id) return &(*it);
	return NULL;
}

bool CParamBlock2::getFloat(uint16 id, float &out) const
{
	const SParam *p = findParam(id);
	if (!p || !p->HasConstant || p->IsTab) return false;
	out = p->F[0];
	return true;
}

bool CParamBlock2::getInt(uint16 id, sint32 &out) const
{
	const SParam *p = findParam(id);
	if (!p || !p->HasConstant || p->IsTab) return false;
	out = p->I;
	return true;
}

bool CParamBlock2::getBool(uint16 id, bool &out) const
{
	const SParam *p = findParam(id);
	if (!p || !p->HasConstant || p->IsTab) return false;
	out = p->I != 0;
	return true;
}

bool CParamBlock2::getColor(uint16 id, float out[3]) const
{
	const SParam *p = findParam(id);
	if (!p || !p->HasConstant || p->IsTab) return false;
	out[0] = p->F[0];
	out[1] = p->F[1];
	out[2] = p->F[2];
	return true;
}

bool CParamBlock2::getString(uint16 id, std::string &out) const
{
	const SParam *p = findParam(id);
	if (!p || !p->HasConstant || p->IsTab) return false;
	out = p->S;
	return true;
}

CReferenceMaker *CParamBlock2::refValue(const SParam &param) const
{
	if (!param.RefBacked || param.RefSlot < 0) return NULL;
	return getReference((uint)param.RefSlot);
}

bool CParamBlock2::getFloatAt0(uint16 id, float &out) const
{
	const SParam *p = findParam(id);
	if (!p || p->IsTab) return false;
	if (p->HasConstant) { out = p->F[0]; return true; }
	// Controller-backed (animated) — resolve the reference slot to its keyframer at tick 0.
	if (p->RefBacked)
		if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(refValue(*p)))
			return kf->floatValueAt0(out);
	return false;
}

bool CParamBlock2::getColorAt0(uint16 id, float out[3]) const
{
	const SParam *p = findParam(id);
	if (!p || p->IsTab) return false;
	if (p->HasConstant) { out[0] = p->F[0]; out[1] = p->F[1]; out[2] = p->F[2]; return true; }
	if (p->RefBacked)
		if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(refValue(*p)))
			return kf->posValueAt0(out); // point3-kind controller (RGBA/Point3 driven by a pos controller)
	return false;
}

CReferenceMaker *CParamBlock2::refValue(uint16 id) const
{
	const SParam *p = findParam(id);
	if (!p) return NULL;
	return refValue(*p);
}

// ---------------------------------------------------------------------------------------------
// In-place modify: rewrite the value bytes inside the owning 0x000e chunk. The record's opaque
// header bytes and everything else are untouched, so a modified block still roundtrips through
// build byte-exactly except for the changed value.

bool CParamBlock2::setFloat(uint16 id, float v)
{
	SParam *p = findParamMutable(id);
	if (!p || p->IsTab || !p->HasConstant || !p->Chunk) return false;
	switch (p->baseType())
	{
	case TYPE_FLOAT:
	case TYPE_ANGLE:
	case TYPE_PCNT_FRAC:
	case TYPE_WORLD:
	case TYPE_COLOR_CHANNEL:
		break;
	default:
		return false;
	}
	if (p->PayloadOff + 4 > p->Chunk->Value.size()) return false;
	memcpy(p->Chunk->Value.data() + p->PayloadOff, &v, 4);
	p->F[0] = v;
	return true;
}

bool CParamBlock2::setInt(uint16 id, sint32 v)
{
	SParam *p = findParamMutable(id);
	if (!p || p->IsTab || !p->HasConstant || !p->Chunk) return false;
	switch (p->baseType())
	{
	case TYPE_INT:
	case TYPE_TIMEVALUE:
	case TYPE_RADIOBTN_INDEX:
		break;
	default:
		return false;
	}
	if (p->PayloadOff + 4 > p->Chunk->Value.size()) return false;
	memcpy(p->Chunk->Value.data() + p->PayloadOff, &v, 4);
	p->I = v;
	p->F[0] = (float)v;
	return true;
}

bool CParamBlock2::setBool(uint16 id, bool v)
{
	SParam *p = findParamMutable(id);
	if (!p || p->IsTab || !p->HasConstant || !p->Chunk) return false;
	if (p->baseType() != TYPE_BOOL) return false;
	if (p->PayloadOff + 4 > p->Chunk->Value.size()) return false;
	sint32 iv = v ? 1 : 0;
	memcpy(p->Chunk->Value.data() + p->PayloadOff, &iv, 4);
	p->I = iv;
	p->F[0] = (float)iv;
	return true;
}

bool CParamBlock2::setColor(uint16 id, const float c[3])
{
	SParam *p = findParamMutable(id);
	if (!p || p->IsTab || !p->HasConstant || !p->Chunk) return false;
	switch (p->baseType())
	{
	case TYPE_RGBA:
	case TYPE_POINT3:
	case TYPE_HSV:
		break;
	default:
		return false;
	}
	if (p->PayloadOff + 12 > p->Chunk->Value.size()) return false;
	memcpy(p->Chunk->Value.data() + p->PayloadOff, c, 12);
	p->F[0] = c[0]; p->F[1] = c[1]; p->F[2] = c[2];
	return true;
}

bool CParamBlock2::selfTestReencode(std::string &err) const
{
	for (std::vector<SParam>::const_iterator it = m_Params.begin(); it != m_Params.end(); ++it)
	{
		const SParam &p = *it;
		if (p.IsTab || !p.HasConstant || !p.Chunk) continue;
		uint sz = 0;
		uint8 buf[12];
		switch (p.baseType())
		{
		case TYPE_FLOAT:
		case TYPE_ANGLE:
		case TYPE_PCNT_FRAC:
		case TYPE_WORLD:
		case TYPE_COLOR_CHANNEL:
			memcpy(buf, &p.F[0], 4); sz = 4; break;
		case TYPE_INT:
		case TYPE_BOOL:
		case TYPE_TIMEVALUE:
		case TYPE_RADIOBTN_INDEX:
			memcpy(buf, &p.I, 4); sz = 4; break;
		case TYPE_RGBA:
		case TYPE_POINT3:
		case TYPE_HSV:
			memcpy(buf, p.F, 12); sz = 12; break;
		default:
			continue; // string/other: not re-encoded here
		}
		if (p.PayloadOff + sz > p.Chunk->Value.size()
			|| memcmp(buf, p.Chunk->Value.data() + p.PayloadOff, sz) != 0)
		{
			std::stringstream ss;
			ss << "param 0x" << std::hex << p.Id << " type 0x" << p.Type << " re-encode mismatch";
			err = ss.str();
			return false;
		}
	}
	return true;
}

void CParamBlock2::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (m_HasHeader)
		ostream << "\n" << pad << "ParamBlock2: block " << m_BlockId << ", version " << m_ScriptVersion
		        << ", " << m_Params.size() << "/" << m_ParamCount << " params";
}

IStorageObject *CParamBlock2::createChunkById(uint16 id, bool container)
{
	// Leaves default to CStorageRaw (0x0009 header, 0x000e records) and the bitmap-info sibling
	// containers (0x0003) to CStorageContainer already; the raw bytes stay authoritative.
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
