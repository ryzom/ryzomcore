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
const TSClassId CRklPatchObject::SuperClassId = BUILTIN::CPatchObject::SuperClassId;
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
	return decodeRpoChunk(raw->Value.data(), raw->Value.size(), out, err);
}

bool CRklPatchObject::decodePatch(SPatchMesh &out, std::string &err) const
{
	return decodePatchMesh(m_Claimed, out, err);
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
