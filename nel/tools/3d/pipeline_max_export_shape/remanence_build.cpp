/**
 * \file remanence_build.cpp
 * \brief See remanence_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
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
#include "remanence_build.h"

#include <cstdio>
#include <vector>

#include <nel/3d/seg_remanence_shape.h>

#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max_export_common/spline_shape.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/max_math.h"
#include "material_build.h"
#include "scene_lib.h"

#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace NLMISC;
using namespace NL3D;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MATBUILD;
using MAXSCENE::decompMatrix;

namespace REMANENCEBUILD {

// AppData sub-ids (export_appdata.h)

IShape *buildRemanenceShape(INode &node, SNodeTMCache &tmCache, bool exportLighting)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	std::string name = nodeName(node);

	// AppData defaults match export_remanence.cpp
	uint numSlices = (uint)getScriptAppDataInt(n, NEL3D_APPDATA_REMANENCE_SLICE_NUMBER, 2);
	float samplingPeriod = getScriptAppDataFloat(n, NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD, 0.02f);
	float rollupRatio = getScriptAppDataFloat(n, NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO, 1.f);
	if (samplingPeriod <= 0.f) samplingPeriod = 0.02f;
	if (numSlices <= 2) numSlices = 2;
	if (rollupRatio <= 0.f) rollupRatio = 1.f;

	// Material — remanence requires exactly one
	std::vector<CMaterial> materials;
	SMaxMeshBaseBuild mmbb;
	buildMaterials(materials, mmbb, node, exportLighting);
	if (materials.size() != 1)
	{
		fprintf(stderr, "SKIP remanence '%s': expected a single material, got %u\n",
		        name.c_str(), (uint)materials.size());
		return NULL;
	}

	// Base shape object
	std::vector<CSceneClass *> mods;
	std::vector<CStorageContainer *> modApps;
	CSceneClass *base = baseObjectOf(node, &mods, &modApps);
	if (!base || !SPLINESHAPE::isShapeObject(base))
	{
		fprintf(stderr, "SKIP remanence '%s': base object is not a Shape (can't get curves)\n",
		        name.c_str());
		return NULL;
	}

	SPLINESHAPE::SShape shape;
	if (!SPLINESHAPE::decodeShapeObject(base, shape))
	{
		fprintf(stderr, "SKIP remanence '%s': no spline data decoded on shape object\n",
		        name.c_str());
		return NULL;
	}
	if (shape.Curves.size() != 1)
	{
		fprintf(stderr, "SKIP remanence '%s': expected 1 curve, got %u\n",
		        name.c_str(), (uint)shape.Curves.size());
		return NULL;
	}

	std::vector<CVector> ends;
	if (!SPLINESHAPE::pieceEndpoints(shape, 0, ends) || ends.size() < 2)
	{
		fprintf(stderr, "SKIP remanence '%s': curve needs at least one segment (2 knots)\n",
		        name.c_str());
		return NULL;
	}

	// objectToLocal = objectTM * inverse(nodeTM), same as water/mesh paths and the reference
	// export_remanence.cpp: objectToLocal = objectTM * invNodeTM; convertVector(nelPos, objectToLocal * pos)
	Matrix3M nodeTM = MAXSCENE::getNodeTM(&node, tmCache);
	Point3M opos;
	QuatM orot;
	ScaleValueM oscale;
	MAXSCENE::readObjectOffset(n, opos, orot, oscale);
	Matrix3M offsetTM = composePRS(opos, orot, oscale);
	Matrix3M objectTM = offsetTM * nodeTM;
	Matrix3M objectToLocal = objectTM * inverseM3(nodeTM);

	CSegRemanenceShape *srs = new CSegRemanenceShape;
	srs->setNumSlices((uint32)numSlices);
	srs->setSliceTime(samplingPeriod);
	srs->setRollupRatio(rollupRatio);
	srs->setMaterial(materials[0]);
	srs->setNumCorners((uint)ends.size());
	for (uint k = 0; k < ends.size(); ++k)
	{
		Point3M p;
		p.x = ends[k].x;
		p.y = ends[k].y;
		p.z = ends[k].z;
		Point3M lp = transformPoint(p, objectToLocal);
		srs->setCorner(k, CVector(lp.x, lp.y, lp.z));
	}

	srs->setTextureShifting(
		getScriptAppDataInt(n, NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE, 0) != 0);

	if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0
	    && !mmbb.MaterialInfo.empty())
	{
		srs->setAnimatedMaterial(mmbb.MaterialInfo[0].MaterialName);
	}

	// Default transform from local matrix (same as flare/mesh)
	{
		Matrix3M localTM = MAXSCENE::getLocalMatrix(node, tmCache);
		CVector pos, scale;
		CQuat rot;
		decompMatrix(scale, rot, pos, localTM);
		srs->getDefaultPos()->setDefaultValue(pos);
		srs->getDefaultScale()->setDefaultValue(scale);
		srs->getDefaultRotQuat()->setDefaultValue(rot);
	}

	return srs;
}

} // namespace REMANENCEBUILD

/* end of file */
