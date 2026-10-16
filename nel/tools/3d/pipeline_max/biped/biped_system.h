/**
 * \file biped_system.h
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

#ifndef PIPELINE_BIPED_SYSTEM_H
#define PIPELINE_BIPED_SYSTEM_H
#include <nel/misc/types_nl.h>

// STL includes
#include <vector>

// Project includes
#include "../builtin/object.h"
#include "biped_anim_track.h"

namespace PIPELINE {
namespace MAX {
namespace BIPED {

/**
 * \brief CBipedSystem
 * \date 2026-07-08
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * The Biped system object, ClassId {0x9155, 0}, superclass 0x60 (Object). Per-rig store of
 * Character Studio's figure parameters plus every animation keytrack. This typed class lifts
 * the 13 animation keytrack chunk pairs (0x012c..0x014a) into CBipedAnimTrack objects and
 * passes every other chunk through verbatim IN ORIGINAL ORDER (the parse drains the whole
 * chunk list into an ordered token vector; build re-emits it, re-encoding the lifted tracks
 * in place). That makes the keytracks EDITABLE: replace a track's keys through
 * track()/trackForEdit() and the next clean/build/write emits the new animation. This is the
 * mechanism behind programmatic .max animation authoring (pipeline_max_export_anim --author-*).
 *
 * Read-path note: exporters (biped_rig.cpp, biped_anim.cpp) read rig/keytrack chunks through
 * findChunkAnywhere, which also scans chunks(), the pre-clean container that keeps holding
 * every original chunk after parse, so lifting the keytracks out of the orphan list does not
 * affect them. But those reads see the ORIGINAL bytes: after editing a track, write the file
 * and reload it for verification instead of re-reading the same in-memory scene.
 *
 * Byte-identity of the lift/re-emit roundtrip over every biped corpus file is gated by the
 * pipeline_max_anim_corpus T1/T2 tests. A track pair that fails the size-consistency decode is
 * left as raw pass-through (byte-identity preserved; track() returns NULL for it).
 */
class CBipedSystem : public BUILTIN::CObject
{
public:
	CBipedSystem(CScene *scene);
	virtual ~CBipedSystem() NL_OVERRIDE;

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
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

	// keytrack access
	/// Typed track, NULL when the pair wasn't lifted (chunks absent or size-inconsistent).
	const CBipedAnimTrack *track(CBipedAnimTrack::ETrack t) const;
	/// Mutable typed track for editing; the next build() emits the edited content.
	CBipedAnimTrack *trackForEdit(CBipedAnimTrack::ETrack t);

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	// One entry per chunk drained from the orphan list after the CObject chain's parse, in
	// original storage order. Lifted keytrack chunks keep their slot with Raw == NULL and are
	// re-encoded from m_Tracks at build; everything else passes through as-is.
	struct SChunkToken
	{
		uint16 Id;
		IStorageObject *Raw; // NULL = lifted keytrack chunk (see TrackIdx/IsTime)
		sint8 TrackIdx;
		bool IsTime;
	};
	std::vector<SChunkToken> m_Tokens;
	CBipedAnimTrack m_Tracks[CBipedAnimTrack::TrackCount];
	bool m_TrackLifted[CBipedAnimTrack::TrackCount];

}; /* class CBipedSystem */

typedef CSceneClassDesc<CBipedSystem> CBipedSystemClassDesc;
extern const CBipedSystemClassDesc BipedSystemClassDesc;

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_BIPED_SYSTEM_H */

/* end of file */
