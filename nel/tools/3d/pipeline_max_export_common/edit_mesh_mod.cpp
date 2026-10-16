/**
 * \file edit_mesh_mod.cpp
 * \brief See edit_mesh_mod.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#include "edit_mesh_mod.h"

#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/storage/mesh_delta.h"

using namespace PIPELINE::MAX;
using PIPELINE::MAX::BUILTIN::CDerivedObject;
using PIPELINE::MAX::BUILTIN::STORAGE::CMeshDelta;

namespace EDITMESH {

// Thin copy from the typed library decode (BUILTIN::STORAGE::CMeshDelta, design-doc §10j-sept)
// into the SEdits evaluation record. The chunk-level format knowledge lives on CMeshDelta now
// (bit-exact rows, corpus-selftested); this wrapper only converts the raw float-bit words into
// the CVector shapes applyEdits consumes. Public API and semantics unchanged: true iff the
// 0x2512 → 0x4000 mesh-delta subtree exists, decoded records appended to \a out.
bool readModApp(CStorageContainer *c2500, SEdits &out)
{
	CMeshDelta md;
	if (!md.decode(CDerivedObject::modAppLocalModData(c2500)))
		return false;
	out.Moves.reserve(out.Moves.size() + md.moves().size());
	for (uint i = 0; i < md.moves().size(); ++i)
	{
		const CMeshDelta::SMove &m = md.moves()[i];
		out.Moves.push_back(std::make_pair(m.Index, NLMISC::CVector(
			CMeshDelta::asF(m.P[0]), CMeshDelta::asF(m.P[1]), CMeshDelta::asF(m.P[2]))));
	}
	out.CreatedVerts.reserve(out.CreatedVerts.size() + md.createdVerts().size());
	for (uint i = 0; i < md.createdVerts().size(); ++i)
	{
		const CMeshDelta::SCreatedVert &v = md.createdVerts()[i];
		SCreatedVert cv;
		cv.SrcTag = v.SrcTag;
		cv.Pos = NLMISC::CVector(CMeshDelta::asF(v.P[0]), CMeshDelta::asF(v.P[1]), CMeshDelta::asF(v.P[2]));
		out.CreatedVerts.push_back(cv);
	}
	out.CreatedFacesA.reserve(out.CreatedFacesA.size() + md.createdFaces().size());
	for (uint i = 0; i < md.createdFaces().size(); ++i)
	{
		const CMeshDelta::SCreatedFace &cf = md.createdFaces()[i];
		SFace f;
		f.V[0] = cf.V[0]; f.V[1] = cf.V[1]; f.V[2] = cf.V[2];
		f.SmGroup = cf.SmGroup;
		f.FaceFlags = cf.FaceFlags;
		out.CreatedFacesA.push_back(f);
	}
	out.FaceRemap.reserve(out.FaceRemap.size() + md.faceRemap().size());
	for (uint i = 0; i < md.faceRemap().size(); ++i)
	{
		const CMeshDelta::SFaceVertRemap &mr = md.faceRemap()[i];
		SFaceVertRemap r;
		r.Index = mr.Index;
		r.ApplyMask = mr.ApplyMask;
		r.V[0] = mr.V[0]; r.V[1] = mr.V[1]; r.V[2] = mr.V[2];
		out.FaceRemap.push_back(r);
	}
	out.FaceAttribs.reserve(out.FaceAttribs.size() + md.faceAttribs().size());
	for (uint i = 0; i < md.faceAttribs().size(); ++i)
	{
		const CMeshDelta::SFaceAttrib &ma = md.faceAttribs()[i];
		SFaceAttribChange fa;
		fa.Index = ma.Index;
		fa.ApplyMask = ma.ApplyMask;
		fa.Values = ma.Values;
		out.FaceAttribs.push_back(fa);
	}
	if (md.delVerts().Present) md.delVerts().bits(out.DelVerts);
	if (md.delFaces().Present) md.delFaces().bits(out.DelFaces);
	return true;
}

} /* namespace EDITMESH */

/* end of file */
