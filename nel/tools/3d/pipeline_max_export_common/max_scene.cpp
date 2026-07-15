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

#include <cstdio>
#include <cstring>

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/control_keyframer.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;

namespace MAXSCENE {

const NLMISC::CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);
const NLMISC::CClassId CLASSID_LOOKAT_CTRL(0x00002006, 0x00000000);

// PRS sub-controller default-value chunk ids.
#define CHUNK_CTRL_POS_VALUE 0x2503
#define CHUNK_CTRL_ROT_VALUE 0x2504
#define CHUNK_CTRL_SCALE_VALUE 0x2505
#define CHUNK_CTRL_FLOAT_VALUE 0x2501

// ---------------------------------------------------------------------------------------------

static CStorageRaw *findRawChunk(CSceneClass *sc, uint16 id)
{
	if (!sc) return NULL;
	for (CStorageContainer::TStorageObjectConstIt it = sc->orphanedChunks().begin(); it != sc->orphanedChunks().end(); ++it)
		if (it->first == id) return dynamic_cast<CStorageRaw *>(it->second);
	for (CStorageContainer::TStorageObjectConstIt it = sc->chunks().begin(); it != sc->chunks().end(); ++it)
		if (it->first == id) return dynamic_cast<CStorageRaw *>(it->second);
	return NULL;
}

// Read a controller's default-value chunk: the typed keyframer's claimed default, else the raw
// orphan chunk (for a controller that is still an unknown pass-through).
static bool readCtrlDefaultBytes(CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes)
{
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(sc);
	if (kf)
	{
		uint size = 0;
		const uint8 *data = kf->defaultValue(size);
		if (data && size >= nBytes)
		{
			memcpy(dst, data, nBytes);
			return true;
		}
	}
	CStorageRaw *raw = findRawChunk(sc, chunkId);
	if (raw && raw->Value.size() >= nBytes)
	{
		memcpy(dst, nlVectorData(raw->Value), nBytes);
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------------------------
// Controller value at t=0. The evaluation lives in the library (CControlKeyFramerBase::
// {pos,rot,scale,float}ValueAt0 — key-bracket at tick 0 else default-value chunk); these wrap it
// into the MAXMATH value types, keeping the raw-chunk fallback for controllers that are not typed
// keyframers.

Point3M posValueAt0(CSceneClass *ctrl)
{
	Point3M p = { 0.0f, 0.0f, 0.0f };
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float v[3];
		if (kf->posValueAt0(v)) { p.x = v[0]; p.y = v[1]; p.z = v[2]; }
		return p;
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_POS_VALUE, &p, 12);
	return p;
}

QuatM rotValueAt0(CSceneClass *ctrl)
{
	QuatM q = { 0.0f, 0.0f, 0.0f, 1.0f };
	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl))
	{
		float v[4];
		if (kf->rotValueAt0(v)) { q.x = v[0]; q.y = v[1]; q.z = v[2]; q.w = v[3]; }
		return q;
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_ROT_VALUE, &q, 16);
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
		return s;
	}
	// Default chunk 0x2505: CVector scale + CQuat axis system (28 bytes) on a non-keyframer.
	uint8 buf[28];
	if (readCtrlDefaultBytes(ctrl, CHUNK_CTRL_SCALE_VALUE, buf, 28))
	{
		memcpy(&s.s, buf, 12);
		memcpy(&s.q, buf + 12, 16);
	}
	else if (readCtrlDefaultBytes(ctrl, CHUNK_CTRL_SCALE_VALUE, buf, 12))
	{
		memcpy(&s.s, buf, 12);
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
		return v;
	}
	readCtrlDefaultBytes(ctrl, CHUNK_CTRL_FLOAT_VALUE, &v, 4);
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
	CSceneClass *sc = dynamic_cast<CSceneClass *>(node);
	if (!sc) return false;
	bool any = false;
	CStorageRaw *raw = findRawChunk(sc, 0x096a);
	if (raw && raw->Value.size() >= 12) { memcpy(&pos, nlVectorData(raw->Value), 12); any = true; }
	raw = findRawChunk(sc, 0x096b);
	if (raw && raw->Value.size() >= 16) { memcpy(&rot, nlVectorData(raw->Value), 16); any = true; }
	raw = findRawChunk(sc, 0x096c);
	if (raw && raw->Value.size() >= 28) { memcpy(&scale, nlVectorData(raw->Value), 28); any = true; }
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
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm);
	if (tmsc && tmsc->classDesc()->classId() == CLASSID_PRS_CTRL && tm->nbReferences() >= 3)
	{
		pos = posValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(0)));
		rot = rotValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(1)));
		scale = scaleValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(2)));
	}
	else if (tmsc && tmsc->classDesc()->classId() == CLASSID_LOOKAT_CTRL && tm->nbReferences() >= 2)
	{
		// LookAt (target lights/cameras): position from ref 1; rotation is target-computed and not
		// needed for the current consumers (identity rotation part).
		pos = posValueAt0(dynamic_cast<CSceneClass *>(tm->getReference(1)));
	}
	else if (tmsc)
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
