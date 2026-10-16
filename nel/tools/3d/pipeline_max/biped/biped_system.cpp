/**
 * \file biped_system.cpp
 * \brief CBipedSystem
 * \date 2026-07-08
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CBipedSystem
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
#include "biped_system.h"

// STL includes

// NeL includes

// Project includes
#include "../storage_object.h"
#include "biped.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BIPED {

using BUILTIN::CObject;

CBipedSystem::CBipedSystem(CScene *scene) : CObject(scene)
{
	for (int i = 0; i < CBipedAnimTrack::TrackCount; ++i)
		m_TrackLifted[i] = false;
}

CBipedSystem::~CBipedSystem()
{

}

const ucstring CBipedSystem::DisplayName = ucstring("Biped");
const char *CBipedSystem::InternalName = "BipedSystem";
const NLMISC::CClassId CBipedSystem::ClassId = NLMISC::CClassId(0x00009155, 0x00000000);
const TSClassId CBipedSystem::SuperClassId = 0x00000060; // Object
const CBipedSystemClassDesc BipedSystemClassDesc(&DllPluginDescBiped);

void CBipedSystem::parse(uint16 version, uint filter)
{
	CObject::parse(version);
	if (m_ChunksOwnsPointers) return; // parsing aborted upstream

	// Drain the remaining chunk list, in storage order, into the token vector. getChunk pops the
	// front orphan when asked for its exact id, so peek+get never reorders and never warns.
	m_Tokens.clear();
	for (;;)
	{
		uint16 id = peekChunk();
		IStorageObject *chunk = getChunk(id);
		if (!chunk) break;
		SChunkToken token;
		token.Id = id;
		token.Raw = chunk;
		token.TrackIdx = -1;
		token.IsTime = false;
		m_Tokens.push_back(token);
	}

	// Lift the keytrack pairs that decode consistently; keep raw pass-through otherwise.
	for (int i = 0; i < CBipedAnimTrack::TrackCount; ++i)
	{
		const CBipedAnimTrack::SDesc &desc = CBipedAnimTrack::Descs[i];
		size_t dTok = m_Tokens.size(), tTok = m_Tokens.size();
		for (size_t k = 0; k < m_Tokens.size(); ++k)
		{
			if (m_Tokens[k].Id == desc.DataId && dTok == m_Tokens.size()) dTok = k;
			if (m_Tokens[k].Id == desc.TimeId && tTok == m_Tokens.size()) tTok = k;
		}
		if (dTok == m_Tokens.size() || tTok == m_Tokens.size()) continue;
		CStorageRaw *dRaw = dynamic_cast<CStorageRaw *>(m_Tokens[dTok].Raw);
		CStorageRaw *tRaw = dynamic_cast<CStorageRaw *>(m_Tokens[tTok].Raw);
		if (!dRaw || !tRaw) continue;
		if (!m_Tracks[i].decode(desc, dRaw->Value, tRaw->Value)) continue;
		m_TrackLifted[i] = true;
		m_ArchivedChunks.push_back(dRaw);
		m_ArchivedChunks.push_back(tRaw);
		m_Tokens[dTok].Raw = nullptr; m_Tokens[dTok].TrackIdx = (sint8)i; m_Tokens[dTok].IsTime = false;
		m_Tokens[tTok].Raw = nullptr; m_Tokens[tTok].TrackIdx = (sint8)i; m_Tokens[tTok].IsTime = true;
	}
}

void CBipedSystem::clean()
{
	CObject::clean();
	// Mirror the base clean for the pass-through tokens the base no longer sees (they were
	// drained out of the orphan list during parse).
	for (size_t k = 0; k < m_Tokens.size(); ++k)
	{
		if (m_Tokens[k].Raw && m_Tokens[k].Raw->isContainer())
			static_cast<CStorageContainer *>(m_Tokens[k].Raw)->clean();
	}
}

void CBipedSystem::build(uint16 version, uint filter)
{
	CObject::build(version);
	// Re-emit the drained chunk list in original order; lifted tracks re-encode from the typed
	// containers (this is where keytrack edits reach the file).
	for (size_t k = 0; k < m_Tokens.size(); ++k)
	{
		SChunkToken &token = m_Tokens[k];
		if (token.Raw)
		{
			putChunk(token.Id, token.Raw);
			continue;
		}
		CStorageRaw *raw = new CStorageRaw();
		const CBipedAnimTrack &track = m_Tracks[token.TrackIdx];
		if (token.IsTime) track.encodeTime(raw->Value);
		else track.encodeData(raw->Value);
		m_ArchivedChunks.push_back(raw);
		putChunk(token.Id, raw);
	}
}

void CBipedSystem::disown()
{
	m_Tokens.clear();
	for (int i = 0; i < CBipedAnimTrack::TrackCount; ++i)
	{
		m_Tracks[i] = CBipedAnimTrack();
		m_TrackLifted[i] = false;
	}
	CObject::disown();
}

void CBipedSystem::init()
{
	CObject::init();
}

bool CBipedSystem::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CObject::inherits(classId);
}

const ISceneClassDesc *CBipedSystem::classDesc() const
{
	return &BipedSystemClassDesc;
}

void CBipedSystem::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CObject::toStringLocal(ostream, pad);
	for (int i = 0; i < CBipedAnimTrack::TrackCount; ++i)
	{
		if (!m_TrackLifted[i]) continue;
		ostream << "\n" << pad << "KeyTrack " << CBipedAnimTrack::Descs[i].Name << ": "
		        << m_Tracks[i].keyCount() << " keys";
	}
}

const CBipedAnimTrack *CBipedSystem::track(CBipedAnimTrack::ETrack t) const
{
	if (t < 0 || t >= CBipedAnimTrack::TrackCount || !m_TrackLifted[t]) return nullptr;
	return &m_Tracks[t];
}

CBipedAnimTrack *CBipedSystem::trackForEdit(CBipedAnimTrack::ETrack t)
{
	if (t < 0 || t >= CBipedAnimTrack::TrackCount || !m_TrackLifted[t]) return nullptr;
	return &m_Tracks[t];
}

IStorageObject *CBipedSystem::createChunkById(uint16 id, bool container)
{
	return CObject::createChunkById(id, container);
}

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
