/**
 * \file patch_edit_internal.h
 * \brief Shared drag state and preview helpers for the two patch-edit TUs.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Not a public header: consumers are patch_edit_ops.cpp and patch_edit_ui.cpp. The gizmo
 * drag is one interaction spread over both TUs - ops owns its lifecycle and state, ui reads
 * that state to draw the preview - so the state lives here rather than being duplicated.
 * Requires zp_state.h (SPaintZone, SPatchXform) and max_math.h (Matrix3M), which the TUs'
 * shared include prologue provides.
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

#ifndef ZONE_PAINTER_PATCH_EDIT_INTERNAL_H
#define ZONE_PAINTER_PATCH_EDIT_INTERNAL_H

// ---------------------------------------------------------------------------------------------
// Defined in patch_edit_ops.cpp

// The live gizmo drag. ops arms and resolves it; ui draws the preview from it (and the
// ZONE_PAINTER_GIZMO_DRAG dev hook in ui pins it, which is why ui also writes it).
extern bool s_Dragging;
extern int s_DragHandle;
extern NLMISC::CVector s_DragDelta;
extern SPatchXform s_DragXform;

/** The offset this drag gives a point at `p`. Null when nothing is being dragged. */
NLMISC::CVector zpDragOffsetAt(const NLMISC::CVector &p);
/** Are handles the transform target right now? (Both selections are non-empty by design.) */
bool zpHandleMode();
/** The object this node shows; several nodes may return the SAME pointer. */
const void *zpZoneNode(uint zoneId);
/** Linear part only: the image of a delta, with the matrix's translation dropped. */
NLMISC::CVector zpXformDelta(const NLMISC::CVector &d, const MAXMATH::Matrix3M &m);

// ---------------------------------------------------------------------------------------------
// Defined in patch_edit_ui.cpp

extern const NLMISC::CVector kNoOffset;
// Fit-at-rest world length of the gizmo. ui fits it between interactions; ops reads it as the
// scale drag's unit ("one gizmo length of drag doubles the size").
extern float s_GizmoWorldLen;
// Which two axes each plane handle spans; ops derives the drag plane's normal from it.
extern const int kGizmoPlaneAxes[3][2];

/**
 * Preview offset for a corner / a handle. ui draws the cage with them; ops' live push runs
 * the SAME functions per frame, which is what keeps the pushed surface and the drawn cage
 * from ever disagreeing about a preview.
 */
NLMISC::CVector zpVertOffset(const SPaintZone &pz, uint16 vi);
NLMISC::CVector zpTanOffset(const SPaintZone &pz, uint16 vecIdx);

#endif /* ZONE_PAINTER_PATCH_EDIT_INTERNAL_H */
