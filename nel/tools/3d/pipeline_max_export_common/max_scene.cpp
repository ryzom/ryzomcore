/**
 * \file max_scene.cpp
 * \brief See max_scene.h.
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

#include <nel/misc/types_nl.h>
#include "max_scene.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

namespace MAXSCENE {

// ---------------------------------------------------------------------------------------------
// Controller value at t=0. The evaluation lives in the library (CControlKeyFramerBase::
// {pos,rot,scale,float}ValueAt0 — key-bracket at tick 0 else default-value chunk); these wrap it
// into the MAXMATH value types. There is deliberately NO raw-chunk fallback for non-keyframer
// controllers: the corpus-wide 0x9008 inventory (design doc §10j-dix) established that no
// non-keyframer sub-controller carries a default-value chunk (0x2501/0x2503/0x2504/0x2505)
// anywhere, Max 3 included — the historical fallback never fired, and a non-keyframer resolves
// to the caller-side identity/zero default exactly as before.
// The composite controllers that split a value into one float controller per component are
// evaluated through those: Position XYZ and Scale XYZ (references 0, 1, 2 are x, y, z) and Euler XYZ
// (three angles in radians, the axis order in chunk 0x1003). Any other controller that is not a
// keyframer warns once per class and gives the default.

static const NLMISC::CClassId CLASSID_POSITION_XYZ(0x118f7e02, 0xffee238a);
static const NLMISC::CClassId CLASSID_SCALE_XYZ(0x118f7c01, 0xfeee238b);
static const NLMISC::CClassId CLASSID_EULER_XYZ(0x00002012, 0x00000000);

static float componentAt0(CSceneClass *ctrl, uint i, float def)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(ctrl);
	CSceneClass *sub = rm && i < rm->nbReferences() ? dynamic_cast<CSceneClass *>(rm->getReference(i)) : nullptr;
	if (sub && !dynamic_cast<CControlKeyFramerBase *>(sub))
	{
		fprintf(stderr, "WARNING: controller (0x%08x, 0x%08x) under a component controller not supported; default "
		        "used\n", sub->classDesc()->classId().a(), sub->classDesc()->classId().b());
		return def;
	}
	return sub ? floatValueAt0(sub, def) : def;
}

static void warnController(CSceneClass *ctrl, const char *what)
{
	static std::set<NLMISC::CClassId> s_Warned;
	const NLMISC::CClassId cid = ctrl->classDesc()->classId();
	if (s_Warned.insert(cid).second)
		fprintf(stderr, "WARNING: %s controller (0x%08x, 0x%08x) not supported; default used\n", what, cid.a(), cid.b());
}

// The quaternion whose quatToMatrix3 is the rotation part of m.
static QuatM rotationToQuat(const Matrix3M &m)
{
	QuatM q;
	const float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
	if (trace > 0.0f)
	{
		const float s = sqrtf(trace + 1.0f) * 2.0f;
		q.w = 0.25f * s;
		q.x = (m.m[2][1] - m.m[1][2]) / s;
		q.y = (m.m[0][2] - m.m[2][0]) / s;
		q.z = (m.m[1][0] - m.m[0][1]) / s;
	}
	else if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2])
	{
		const float s = sqrtf(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0f;
		q.w = (m.m[2][1] - m.m[1][2]) / s;
		q.x = 0.25f * s;
		q.y = (m.m[0][1] + m.m[1][0]) / s;
		q.z = (m.m[0][2] + m.m[2][0]) / s;
	}
	else if (m.m[1][1] > m.m[2][2])
	{
		const float s = sqrtf(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0f;
		q.w = (m.m[0][2] - m.m[2][0]) / s;
		q.x = (m.m[0][1] + m.m[1][0]) / s;
		q.y = 0.25f * s;
		q.z = (m.m[1][2] + m.m[2][1]) / s;
	}
	else
	{
		const float s = sqrtf(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0f;
		q.w = (m.m[1][0] - m.m[0][1]) / s;
		q.x = (m.m[0][2] + m.m[2][0]) / s;
		q.y = (m.m[1][2] + m.m[2][1]) / s;
		q.z = 0.25f * s;
	}
	return q;
}

Point3M posValueAt0(CSceneClass *ctrl)
{
	Point3M p = { 0.0f, 0.0f, 0.0f };
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float v[3];
		if (kf->posValueAt0(v)) { p.x = v[0]; p.y = v[1]; p.z = v[2]; }
	}
	else if (ctrl && ctrl->classDesc()->classId() == CLASSID_POSITION_XYZ)
	{
		p.x = componentAt0(ctrl, 0, 0.0f);
		p.y = componentAt0(ctrl, 1, 0.0f);
		p.z = componentAt0(ctrl, 2, 0.0f);
	}
	else if (ctrl)
	{
		warnController(ctrl, "position");
	}
	return p;
}

QuatM rotValueAt0(CSceneClass *ctrl)
{
	QuatM q = { 0.0f, 0.0f, 0.0f, 1.0f };
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float v[4];
		if (kf->rotValueAt0(v)) { q.x = v[0]; q.y = v[1]; q.z = v[2]; q.w = v[3]; }
	}
	else if (ctrl && ctrl->classDesc()->classId() == CLASSID_EULER_XYZ)
	{
		// The rotation axes of the first, second and third angle per axis order. The order is stored plus
		// 100 (except order 0) in newer files, where references 0, 1, 2 are the x, y and z angles and the
		// six orders of three different axes take their angles per axis; older files store the plain order
		// with the references in angle order. The repeating orders (XYX, YZY, ZXZ) use reference order
		// in both.
		static const int s_Orders[9][3] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 2, 0 }, { 1, 0, 2 }, { 2, 0, 1 },
		                                    { 2, 1, 0 }, { 0, 1, 0 }, { 1, 2, 1 }, { 2, 0, 2 } };
		sint32 stored = 0;
		const CStorageContainer::TStorageObjectContainer &orphans = ctrl->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		{
			const CStorageRaw *raw = it->first == 0x1003 ? dynamic_cast<const CStorageRaw *>(it->second) : nullptr;
			if (raw && raw->Value.size() >= 4)
				memcpy(&stored, &raw->Value[0], 4);
		}
		const bool perAxis = stored >= 100;
		const sint32 order = stored % 100;
		if (stored < 0 || order > 8)
		{
			fprintf(stderr, "WARNING: Euler rotation controller with axis order %i not supported; no rotation used\n",
			        (int)stored);
			return q;
		}
		const float ang[3] = { componentAt0(ctrl, 0, 0.0f), componentAt0(ctrl, 1, 0.0f), componentAt0(ctrl, 2, 0.0f) };
		Matrix3M m = Matrix3M::identity();
		for (int i = 0; i < 3; ++i)
			m = rotateAxisM3(m, s_Orders[order][i], perAxis && order < 6 ? ang[s_Orders[order][i]] : ang[i]);
		q = rotationToQuat(m);
	}
	else if (ctrl)
	{
		warnController(ctrl, "rotation");
	}
	return q;
}

ScaleValueM scaleValueAt0(CSceneClass *ctrl)
{
	ScaleValueM s;
	s.s.x = s.s.y = s.s.z = 1.0f;
	s.q.x = s.q.y = s.q.z = 0.0f;
	s.q.w = 1.0f;
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float v[7];
		if (kf->scaleValueAt0(v))
		{
			s.s.x = v[0]; s.s.y = v[1]; s.s.z = v[2];
			s.q.x = v[3]; s.q.y = v[4]; s.q.z = v[5]; s.q.w = v[6];
		}
	}
	else if (ctrl && ctrl->classDesc()->classId() == CLASSID_SCALE_XYZ)
	{
		// the scale axis stays the identity
		s.s.x = componentAt0(ctrl, 0, 1.0f);
		s.s.y = componentAt0(ctrl, 1, 1.0f);
		s.s.z = componentAt0(ctrl, 2, 1.0f);
	}
	else if (ctrl)
	{
		warnController(ctrl, "scale");
	}
	return s;
}

float floatValueAt0(CSceneClass *ctrl, float def)
{
	float v = def;
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float fv;
		if (kf->floatValueAt0(fv)) v = fv;
	}
	return v;
}

bool readObjectOffset(INode *node, Point3M &pos, QuatM &rot, ScaleValueM &scale)
{
	pos.x = pos.y = pos.z = 0.0f;
	rot.x = rot.y = rot.z = 0.0f;
	rot.w = 1.0f;
	scale.s.x = scale.s.y = scale.s.z = 1.0f;
	scale.q.x = scale.q.y = scale.q.z = 0.0f;
	scale.q.w = 1.0f;
	// The typed CNodeImpl overlay decodes the offset chunks (0x096a/0x096b/0x096c) at parse —
	// one decode path, in the library (formerly a findRawChunk walk here).
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(node);
	if (!n) return false;
	bool any = false;
	float p3[3], q4[4], s3[3];
	if (n->objectOffsetPos(p3)) { pos.x = p3[0]; pos.y = p3[1]; pos.z = p3[2]; any = true; }
	if (n->objectOffsetRot(q4)) { rot.x = q4[0]; rot.y = q4[1]; rot.z = q4[2]; rot.w = q4[3]; any = true; }
	if (n->objectOffsetScale(s3, q4))
	{
		scale.s.x = s3[0]; scale.s.y = s3[1]; scale.s.z = s3[2];
		scale.q.x = q4[0]; scale.q.y = q4[1]; scale.q.z = q4[2]; scale.q.w = q4[3];
		any = true;
	}
	return any;
}

// ---------------------------------------------------------------------------------------------
// Node transforms.

Matrix3M getNodeTM(INode *node, SNodeTMCache &cache)
{
	if (!node || node == cache.SceneRoot) return Matrix3M::identity();
	if (!dynamic_cast<CNodeImpl *>(node)) return Matrix3M::identity(); // scene root / unknown node class
	std::map<INode *, Matrix3M>::iterator it = cache.TM.find(node);
	if (it != cache.TM.end()) return it->second;

	Point3M pos = { 0.0f, 0.0f, 0.0f };
	QuatM rot = { 0.0f, 0.0f, 0.0f, 1.0f };
	ScaleValueM scale;
	scale.s.x = scale.s.y = scale.s.z = 1.0f;
	scale.q.x = scale.q.y = scale.q.z = 0.0f;
	scale.q.w = 1.0f;

	CReferenceMaker *tm = dynamic_cast<CReferenceMaker *>(node->getReference(0));
	if (CControlPRS *prs = dynamic_cast<CControlPRS *>(tm))
	{
		pos = posValueAt0(dynamic_cast<CSceneClass *>(prs->positionController()));
		rot = rotValueAt0(dynamic_cast<CSceneClass *>(prs->rotationController()));
		scale = scaleValueAt0(dynamic_cast<CSceneClass *>(prs->scaleController()));
	}
	else if (CControlLookAt *la = dynamic_cast<CControlLookAt *>(tm))
	{
		// LookAt (target lights/cameras): position from ref 1; rotation is target-computed and not
		// needed for the current consumers (identity rotation part).
		pos = posValueAt0(dynamic_cast<CSceneClass *>(la->positionController()));
	}
	else if (CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm))
	{
		fprintf(stderr, "WARNING: node '%s' TM controller %s is not PRS; identity local TM used\n",
		        ucstring(node->userName()).toUtf8().c_str(), tmsc->classDesc()->classId().toString().c_str());
	}

	Matrix3M local = composePRS(pos, rot, scale);
	Matrix3M world = local * getNodeTM(node->parent(), cache);
	cache.TM[node] = world;
	return world;
}

Matrix3M getLocalMatrix(INode &node, SNodeTMCache &cache)
{
	Matrix3M nodeTM = getNodeTM(&node, cache);
	Matrix3M parentTM = getNodeTM(node.parent(), cache);
	return nodeTM * inverseM3(parentTM);
}

void decompMatrix(NLMISC::CVector &nelScale, NLMISC::CQuat &nelRot, NLMISC::CVector &nelPos,
                  const Matrix3M &maxMatrix)
{
	AffinePartsM parts;
	decompAffine(maxMatrix, parts);
	nelPos.set(parts.t.x, parts.t.y, parts.t.z);
	nelRot.set(parts.q.x, parts.q.y, parts.q.z, -parts.q.w);
	Matrix3M srtm = quatToMatrix3(parts.u);
	Matrix3M stm = Matrix3M::identity();
	stm.m[0][0] = parts.k.x;
	stm.m[1][1] = parts.k.y;
	stm.m[2][2] = parts.k.z;
	Matrix3M smat = inverseM3(srtm) * stm * srtm;
	nelScale.set(parts.f * smat.m[0][0], parts.f * smat.m[1][1], parts.f * smat.m[2][2]);
}

void convertMatrix(NLMISC::CMatrix &nelMatrix, const Matrix3M &maxMatrix)
{
	// Exactly CExportNel::convertMatrix: identity() then setRot(I,J,K) then setPos(P), so the
	// CMatrix carries the same state bits the serializers rely on.
	NLMISC::CVector I(maxMatrix.m[0][0], maxMatrix.m[0][1], maxMatrix.m[0][2]);
	NLMISC::CVector J(maxMatrix.m[1][0], maxMatrix.m[1][1], maxMatrix.m[1][2]);
	NLMISC::CVector K(maxMatrix.m[2][0], maxMatrix.m[2][1], maxMatrix.m[2][2]);
	NLMISC::CVector P(maxMatrix.m[3][0], maxMatrix.m[3][1], maxMatrix.m[3][2]);
	nelMatrix.identity();
	nelMatrix.setRot(I, J, K);
	nelMatrix.setPos(P);
}

} /* namespace MAXSCENE */

/* end of file */
