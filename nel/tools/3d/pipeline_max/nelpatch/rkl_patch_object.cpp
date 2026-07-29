/**
 * \file rkl_patch_object.cpp
 * \brief CRklPatchObject
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CRklPatchObject
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
#include "rkl_patch_object.h"

// STL includes
#include <cstdlib>

// NeL includes

// Project includes
#include "nelpatch.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

CRklPatchObject::CRklPatchObject(CScene *scene) : BUILTIN::CPatchObject(scene)
{

}

CRklPatchObject::~CRklPatchObject()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
			delete it->second;
		m_Claimed.clear();
	}
}

const ucstring CRklPatchObject::DisplayName = ucstring("RklPatch");
const char *CRklPatchObject::InternalName = "RklPatchObject";
const NLMISC::CClassId CRklPatchObject::ClassId = NLMISC::CClassId(0x368c679f, 0x711c22ee);
const TSClassId CRklPatchObject::SuperClassId = 0x00000010; // GeomObject (== CPatchObject::SuperClassId); literal, not a cross-TU read, to avoid the static-init-order fiasco (MSVC/VS2008 initialized this before CPatchObject's and got 0)
const CRklPatchObjectClassDesc RklPatchObjectClassDesc(&DllPluginDescNelPatch);

bool CRklPatchObject::isKnownChunkId(uint16 id) const
{
	if (id == 0x08fd) return true;
	for (const uint16 *p = PatchMeshChunkIds; *p; ++p)
		if (id == *p) return true;
	for (const uint16 *p = MeshChunkIds; *p; ++p)
		if (id == *p) return true;
	return false;
}

void CRklPatchObject::parse(uint16 version, uint filter)
{
	BUILTIN::CPatchObject::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		// Claim known chunks off the head of the orphan list, in file order, stopping at the
		// first unrecognized id (anything after it stays orphaned verbatim pass-through, same
		// discipline as CControlKeyFramerBase).
		for (;;)
		{
			uint16 id = peekChunk();
			if (id == 0x0000) break;
			if (!isKnownChunkId(id)) break;
			IStorageObject *so = getChunk(id);
			if (!so) break;
			m_Claimed.push_back(TStorageObjectWithId(id, so));
		}
	}
}

void CRklPatchObject::clean()
{
	BUILTIN::CPatchObject::clean();
}

void CRklPatchObject::build(uint16 version, uint filter)
{
	BUILTIN::CPatchObject::build(version);
	for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
		putChunk(it->first, it->second);
}

void CRklPatchObject::disown()
{
	m_Claimed.clear();
	BUILTIN::CPatchObject::disown();
}

void CRklPatchObject::init()
{
	BUILTIN::CPatchObject::init();
}

bool CRklPatchObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return BUILTIN::CPatchObject::inherits(classId);
}

const ISceneClassDesc *CRklPatchObject::classDesc() const
{
	return &RklPatchObjectClassDesc;
}

void CRklPatchObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	BUILTIN::CPatchObject::toStringLocal(ostream, pad);
	SRPatchMesh rp;
	SPatchMesh pm;
	std::string err;
	if (decodeRPatch(rp, err))
	{
		ostream << "\n" << pad << "RPatchMesh: v" << rp.Version << ", " << rp.Patches.size() << " ui patches, " << rp.Verts.size() << " ui verts";
		ostream << "\n" << pad << "TileTessLevel: " << rp.TileTessLevel << " ModeTile: " << (int)rp.ModeTile;
	}
	else
	{
		ostream << "\n" << pad << "RPatchMesh: UNPARSED (" << err << ")";
	}
	err.clear();
	if (decodePatch(pm, err))
	{
		ostream << "\n" << pad << "PatchMesh: " << pm.Verts.size() << " verts, " << pm.Vecs.size() << " vecs, "
			<< pm.Edges.size() << " edges, " << pm.Patches.size() << " patches";
	}
	else
	{
		ostream << "\n" << pad << "PatchMesh: UNPARSED (" << err << ")";
	}
	// The claimed run, id(size) in file order. The element streams are decoded above; this
	// line exists for the OTHER ids - the header/trailer chunks an encoder must regenerate or
	// provably preserve, whose size behaviour against the element counts is the evidence.
	// Small raws also print their bytes: the header/trailer singles are 4-8 byte values whose
	// MEANING (a count, a mode, a flag) only shows by varying across the corpus.
	ostream << "\n" << pad << "Claimed:" << std::hex;
	for (TStorageObjectContainer::const_iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
	{
		const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
		ostream << " " << it->first << "(" << std::dec;
		if (raw)
		{
			// Dev hook: PMAX_DUMP_CLAIMED_HEX=1 hex-dumps every claimed raw whole, for
			// decoding the header/trailer chunks whose layout is still being established.
			const char *full = getenv("PMAX_DUMP_CLAIMED_HEX");
			ostream << (uint)raw->Value.size();
			if (!raw->Value.empty() && (raw->Value.size() <= 24 || (full && *full && *full != '0')))
			{
				ostream << "=" << std::hex;
				for (size_t b = 0; b < raw->Value.size(); ++b)
				{
					if (b) ostream << ".";
					ostream << (uint)raw->Value[b];
				}
				ostream << std::dec;
			}
		}
		else
		{
			// Containers: the summed sizes of their raw children (the selection bitarrays
			// and caches are single-0x2700 wrappers, so this IS their payload size).
			// Dev hook: PMAX_DUMP_CLAIMED_TREE=1 prints the children instead - the id ORDER
			// inside the element containers is what an encoder must reproduce.
			const CStorageContainer *cont = dynamic_cast<const CStorageContainer *>(it->second);
			const char *tree = getenv("PMAX_DUMP_CLAIMED_TREE");
			if (cont && tree && *tree && *tree != '0')
			{
				ostream << "C:" << std::hex;
				for (TStorageObjectContainer::const_iterator c = cont->chunks().begin();
				     c != cont->chunks().end(); ++c)
				{
					if (c != cont->chunks().begin()) ostream << ".";
					ostream << c->first;
					if (const CStorageRaw *cr = dynamic_cast<const CStorageRaw *>(c->second))
						ostream << ":" << std::dec << (uint)cr->Value.size() << std::hex;
					else
						ostream << ":C";
				}
				ostream << std::dec;
			}
			else
			{
				uint sum = 0;
				if (cont)
					for (TStorageObjectContainer::const_iterator c = cont->chunks().begin();
					     c != cont->chunks().end(); ++c)
						if (const CStorageRaw *cr = dynamic_cast<const CStorageRaw *>(c->second))
							sum += (uint)cr->Value.size();
				ostream << "C" << sum;
			}
		}
		ostream << ")" << std::hex;
	}
	ostream << std::dec;
}

const CStorageRaw *CRklPatchObject::rpoChunk() const
{
	for (TStorageObjectContainer::const_iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
		if (it->first == 0x08fd) return dynamic_cast<const CStorageRaw *>(it->second);
	return NULL;
}

bool CRklPatchObject::decodeRPatch(SRPatchMesh &out, std::string &err) const
{
	const CStorageRaw *raw = rpoChunk();
	if (!raw) { err = "no 0x08fd chunk"; return false; }
	return decodeRpoChunk(nlVectorData(raw->Value), raw->Value.size(), out, err);
}

bool CRklPatchObject::decodePatch(SPatchMesh &out, std::string &err) const
{
	return decodePatchMesh(m_Claimed, out, err);
}

bool CRklPatchObject::setRPatch(const SRPatchMesh &in)
{
	for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
	{
		if (it->first != 0x08fd) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) return false;
		encodeRpoChunk(in, raw->Value);
		return true;
	}
	return false;
}

bool CRklPatchObject::setPatchMesh(const SPatchMesh &in, std::string &err)
{
	if (!encodePatchMesh(in, m_Claimed, err))
		return false;
	// The claimed run ALIASES entries of the source chunk list (parse copies orphan
	// pointers into m_Claimed; the source list keeps its own aliases until the save's
	// clean/build cycle). A structural edit above - element containers inserted or erased
	// as counts changed - only touched m_Claimed, which would leave the source list
	// missing new containers (a later geometry write cannot find the vertex) or holding
	// DANGLING pointers to erased ones. Rebuild the aliased run in place: drop every
	// claimed-id entry (ids only, stale pointers are never dereferenced) and splice the
	// current claimed run back at the same position.
	// Only the CONTIGUOUS known-id run is the claimed alias region - the claim itself
	// stops at the first unrecognized id, so any known-looking chunk after a gap was
	// never claimed and must stay untouched.
	TStorageObjectContainer &chunks = chunksMut();
	TStorageObjectContainer::iterator insertPos = chunks.end();
	bool haveIns = false;
	for (TStorageObjectContainer::iterator it = chunks.begin(); it != chunks.end();)
	{
		if (isKnownChunkId(it->first))
		{
			// Track the position AFTER every erase: an iterator captured at the first
			// erase would be invalidated when the run's later nodes (which it points
			// into) are erased too.
			it = chunks.erase(it);
			insertPos = it;
			haveIns = true;
		}
		else
		{
			if (haveIns)
				break; // the run ended; later known-looking ids were never claimed
			++it;
		}
	}
	for (TStorageObjectContainer::iterator it = m_Claimed.begin(); it != m_Claimed.end(); ++it)
		chunks.insert(insertPos, *it);
	return true;
}

IStorageObject *CRklPatchObject::createChunkById(uint16 id, bool container)
{
	// All leaf chunks default to CStorageRaw and containers to CStorageContainer already;
	// nothing to specialize (the raw bytes stay authoritative).
	return BUILTIN::CPatchObject::createChunkById(id, container);
}

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
