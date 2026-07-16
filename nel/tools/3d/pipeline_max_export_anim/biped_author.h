/**
 * \file biped_author.h
 * \brief Programmatic biped animation authoring (jump emote generator).
 *
 * Synthesizes a complete biped animation from scratch onto a skeleton .max file through the
 * typed CBipedSystem keytrack editing surface (pipeline_max/biped), using the decoded storage
 * conversions of §10c IN REVERSE (world-space authoring targets -> stored record fields) and the
 * CBipedAnimEval forward math for attach frames and verification. See pipeline_max_design.md
 * §10t/§10u.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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

#ifndef PIPELINE_MAX_EXPORT_ANIM_BIPED_AUTHOR_H
#define PIPELINE_MAX_EXPORT_ANIM_BIPED_AUTHOR_H

namespace BIPAUTHOR {

// Author the asymmetric jump emote: skelMax = the rig container (its keytracks are filled),
// idleMax = an existing animation on the same rig whose FIRST KEY defines the common idle pose
// (semantic fields are re-authored; unidentified cache slots are carried from these records),
// outMax = the .max written (skel streams verbatim except the rebuilt Scene).
int runAuthorJump(const char *skelMax, const char *idleMax, const char *outMax);

} /* namespace BIPAUTHOR */

#endif /* PIPELINE_MAX_EXPORT_ANIM_BIPED_AUTHOR_H */

/* end of file */
