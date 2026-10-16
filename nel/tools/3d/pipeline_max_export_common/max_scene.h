/**
 * \file max_scene.h
 * \brief Shared Max scene-graph transform helpers for the headless exporters: the node world-TM
 * accumulation the reference plugin gets from 3ds Max's GetNodeTM (PRS / LookAt controllers
 * evaluated at t=0, composed through the Max Matrix3 math in max_math.h), the getLocalMatrix /
 * decompMatrix / convertMatrix operations of CExportNel, and the controller value-at-t=0 readers.
 * These were duplicated across pipeline_max_export_ig / _shape (and now _skel); consolidated here
 * so the copies stop drifting — see pipeline_max_design.md's note on the max_math/db_path
 * consolidation. Governed by the same T3-epsilon bit-exactness contract as max_math.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_MAX_SCENE_H
#define PIPELINE_MAX_EXPORT_COMMON_MAX_SCENE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/matrix.h>
#include <nel/misc/class_id.h>

#include <map>

#include "max_math.h"

#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/builtin/i_node.h"

namespace MAXSCENE {

// The two transform-controller classes whose value we can evaluate at t=0 are the typed
// CControlPRS (0x2005) / CControlLookAt (0x2006) — see pipeline_max/builtin/control_transform.h;
// identity lives on the typed classes (dynamic_cast), not on exported ClassId constants.

// ---------------------------------------------------------------------------------------------
// Controller value at t=0.
//
// A transform sub-controller's value at tick 0 — the key table bracketed at tick 0 for the typed
// keyframers (CControlKeyFramerBase::{pos,rot,scale,float}ValueAt0), else the default-value chunk
// (0x2503 pos / 0x2504 rot / 0x2505 scale / 0x2501 float, claimed by the keyframer). A controller
// that is not a typed keyframer yields the identity/zero default: the corpus-wide 0x9008
// inventory (design doc §10j-dix) established that no non-keyframer sub-controller carries a
// default-value chunk anywhere, so the historical raw-chunk fallback never fired and is gone.
// The rotation is returned in the Max STORED convention
// (the inverse of the node-TM rotation); feed it straight to MAXMATH::composePRS, whose
// quatToMatrix3 (= Max Quat::MakeMatrix) already bakes in the transpose.

MAXMATH::Point3M     posValueAt0(PIPELINE::MAX::CSceneClass *ctrl);
MAXMATH::QuatM       rotValueAt0(PIPELINE::MAX::CSceneClass *ctrl);
MAXMATH::ScaleValueM scaleValueAt0(PIPELINE::MAX::CSceneClass *ctrl);
float                floatValueAt0(PIPELINE::MAX::CSceneClass *ctrl, float def);

// Read an object-offset PRS (chunks 0x096a pos / 0x096b rot / 0x096c scale on the node), the
// GetObjOffset{Pos,Rot,Scale} the reference applies to build objectTM = offsetTM * nodeTM.
// Defaults to identity when a chunk is absent. \a node may be NULL (→ identity). Returns true if
// any offset chunk was present (the caller uses this to skip the offset composition entirely).
bool readObjectOffset(PIPELINE::MAX::BUILTIN::INode *node,
                      MAXMATH::Point3M &pos, MAXMATH::QuatM &rot, MAXMATH::ScaleValueM &scale);

// ---------------------------------------------------------------------------------------------
// Node transforms.

// Per-run memoization of getNodeTM. Construct one per file; SceneRoot may be left NULL (a node
// whose class is not CNodeImpl — e.g. the scene root — already resolves to identity).
struct SNodeTMCache
{
	PIPELINE::MAX::BUILTIN::INode *SceneRoot;
	std::map<PIPELINE::MAX::BUILTIN::INode *, MAXMATH::Matrix3M> TM;
	SNodeTMCache() : SceneRoot(nullptr) { }
};

// The node's world TM at t=0 — Max's GetNodeTM(time) reproduced by composing the node's PRS
// controller (composePRS) with its parent's world TM (Max Matrix3 order: local * parent),
// recursively and memoized. Nodes that are not CNodeImpl (the scene root) and LookAt-only
// rotations return identity for the rotation part (LookAt supplies a position only). A non-PRS,
// non-LookAt controller warns once and yields an identity local TM.
MAXMATH::Matrix3M getNodeTM(PIPELINE::MAX::BUILTIN::INode *node, SNodeTMCache &cache);

// CExportNel::getLocalMatrix: nodeTM * Inverse(parentTM) in Max Matrix3 math.
MAXMATH::Matrix3M getLocalMatrix(PIPELINE::MAX::BUILTIN::INode &node, SNodeTMCache &cache);

// CExportNel::decompMatrix: decomp_affine (max_math) into the NeL default-track convention —
// nelPos = t, nelRot = (q.x,q.y,q.z,-q.w), nelScale = f·diag(Inverse(srtm)·stm·srtm).
void decompMatrix(NLMISC::CVector &nelScale, NLMISC::CQuat &nelRot, NLMISC::CVector &nelPos,
                  const MAXMATH::Matrix3M &maxMatrix);

// CExportNel::convertMatrix: a Max row-vector Matrix3 into a NeL column CMatrix, built exactly as
// the reference does — identity() then setRot(I,J,K) then setPos(P) — so the CMatrix carries the
// same state bits (MAT_ROT|MAT_SCALEANY, MAT_TRANS when |P|>0) the .skel/.shape serializers key on.
void convertMatrix(NLMISC::CMatrix &nelMatrix, const MAXMATH::Matrix3M &maxMatrix);

} /* namespace MAXSCENE */

#endif /* PIPELINE_MAX_EXPORT_COMMON_MAX_SCENE_H */

/* end of file */
