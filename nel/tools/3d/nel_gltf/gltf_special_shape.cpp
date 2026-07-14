/**
 * \file gltf_special_shape.cpp
 * \brief Structural special-shape codec — see gltf_special_shape.h. Field sets mirror the
 * builders (pipeline_max_export_shape/{water,remanence,flare}_build.cpp): the decompose side
 * reads every serialized field through the shape's getters, the rebuild side sets every field
 * through the same setters the builders call, so byte-identity of the reserialized shape is
 * the completeness proof.
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

#include <nel/misc/types_nl.h>
#include "gltf_special_shape.h"

#include <cstdio>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/3d/shape.h>
#include <nel/3d/mesh.h>
#include <nel/3d/water_shape.h>
#include <nel/3d/seg_remanence_shape.h>
#include <nel/3d/flare_shape.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_blend.h>
#include <nel/3d/animated_material.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>

#include "gltf_material.h"

#ifdef NL_OS_WINDOWS
#include <process.h>
#define NLGLTF_SS_GETPID _getpid
#else
#include <unistd.h>
#define NLGLTF_SS_GETPID getpid
#endif

using namespace NLMISC;
using namespace NL3D;

namespace NLGLTF {

// ---------------------------------------------------------------------------------------------
// Small helpers

static std::string colorToHex(const CRGBA &c)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "%02x%02x%02x%02x", c.R, c.G, c.B, c.A);
	return buf;
}

static bool hexToColor(const std::string &s, CRGBA &c)
{
	if (s.size() != 8) return false;
	uint v[4];
	for (uint i = 0; i < 4; ++i)
	{
		if (sscanf(s.c_str() + i * 2, "%2x", &v[i]) != 1) return false;
	}
	c.set((uint8)v[0], (uint8)v[1], (uint8)v[2], (uint8)v[3]);
	return true;
}

static void putVector(CJsonValue &e, const char *key, const CVector &v)
{
	CJsonValue *a = e.setArray(key);
	a->pushDouble(v.x);
	a->pushDouble(v.y);
	a->pushDouble(v.z);
}

static bool getVector(const CJsonValue &e, const char *key, CVector &v)
{
	const CJsonValue *a = e.get(key);
	if (!a || !a->isArray() || a->size() != 3) return false;
	v.set(a->at(0)->asFloat(), a->at(1)->asFloat(), a->at(2)->asFloat());
	return true;
}

static void putQuat(CJsonValue &e, const char *key, const CQuat &q)
{
	CJsonValue *a = e.setArray(key);
	a->pushDouble(q.x);
	a->pushDouble(q.y);
	a->pushDouble(q.z);
	a->pushDouble(q.w);
}

static bool getQuat(const CJsonValue &e, const char *key, CQuat &q)
{
	const CJsonValue *a = e.get(key);
	if (!a || !a->isArray() || a->size() != 4) return false;
	q.set(a->at(0)->asFloat(), a->at(1)->asFloat(), a->at(2)->asFloat(), a->at(3)->asFloat());
	return true;
}

// Plain-file or day/night-blend texture -> 1 or 2 file names (the only texture classes the
// builders construct: CTextureFile with just a file name, or CTextureBlend of two of them).
static bool texToFiles(const ITexture *t, std::vector<std::string> &files, std::string *err)
{
	files.clear();
	if (!t) return true;
	if (const CTextureFile *tf = dynamic_cast<const CTextureFile *>(t))
	{
		files.push_back(tf->getFileName());
		return true;
	}
	if (const CTextureBlend *tb = dynamic_cast<const CTextureBlend *>(t))
	{
		const CTextureFile *t0 = dynamic_cast<const CTextureFile *>(tb->getBlendtexture(0));
		const CTextureFile *t1 = dynamic_cast<const CTextureFile *>(tb->getBlendtexture(1));
		if (!t0 || !t1)
		{
			if (err) *err = "blend texture with non-file components";
			return false;
		}
		files.push_back(t0->getFileName());
		files.push_back(t1->getFileName());
		return true;
	}
	if (err) *err = "texture class not carried by the structural codec";
	return false;
}

// Rebuild the exact texture the water builder constructs: plain CTextureFile, or
// CTextureBlend(day, night).
static ITexture *texFromFiles(const std::vector<std::string> &files)
{
	if (files.empty()) return NULL;
	if (files.size() == 1)
	{
		CTextureFile *tf = new CTextureFile;
		tf->setFileName(files[0]);
		return tf;
	}
	CTextureBlend *b = new CTextureBlend;
	CTextureFile *t0 = new CTextureFile;
	t0->setFileName(files[0]);
	CTextureFile *t1 = new CTextureFile;
	t1->setFileName(files[1]);
	b->setBlendTexture(0, t0);
	b->setBlendTexture(1, t1);
	return b;
}

static void putTexFiles(CJsonValue &e, const char *key, const std::vector<std::string> &files)
{
	if (files.empty()) return;
	CJsonValue *a = e.setArray(key);
	for (size_t i = 0; i < files.size(); ++i)
		a->pushString(files[i]);
}

static bool getTexFiles(const CJsonValue &e, const char *key, std::vector<std::string> &files)
{
	files.clear();
	const CJsonValue *a = e.get(key);
	if (!a) return true; // absent = no texture
	if (!a->isArray() || a->size() < 1 || a->size() > 2) return false;
	for (size_t i = 0; i < a->size(); ++i)
		files.push_back(a->at(i)->asString());
	return true;
}

// ---------------------------------------------------------------------------------------------
// Water

static bool waterToExtras(CWaterShape *ws, CJsonValue &e, std::string *err)
{
	std::vector<std::string> files;
	if (!texToFiles(ws->getEnvMap(0), files, err)) return false;
	putTexFiles(e, "nel_water_env", files);
	if (!texToFiles(ws->getEnvMap(1), files, err)) return false;
	putTexFiles(e, "nel_water_env_under", files);
	if (!texToFiles(ws->getHeightMap(0), files, err)) return false;
	if (files.size() != 1)
	{
		if (err) *err = "water displace map not a single file";
		return false;
	}
	e.setString("nel_water_displace_file", files[0]);
	if (!texToFiles(ws->getHeightMap(1), files, err)) return false;
	if (files.size() != 1)
	{
		if (err) *err = "water bump map not a single file";
		return false;
	}
	e.setString("nel_water_bump_file", files[0]);
	if (!texToFiles(ws->getColorMap(), files, err)) return false;
	if (files.size() > 1)
	{
		if (err) *err = "water color map not a single file";
		return false;
	}
	if (!files.empty())
		e.setString("nel_water_color_file", files[0]);

	CVector2f v;
	v = ws->getHeightMapScale(0);
	e.setDouble("nel_water_displace_scale_u", v.x);
	e.setDouble("nel_water_displace_scale_v", v.y);
	v = ws->getHeightMapSpeed(0);
	e.setDouble("nel_water_displace_speed_u", v.x);
	e.setDouble("nel_water_displace_speed_v", v.y);
	v = ws->getHeightMapScale(1);
	e.setDouble("nel_water_bump_scale_u", v.x);
	e.setDouble("nel_water_bump_scale_v", v.y);
	v = ws->getHeightMapSpeed(1);
	e.setDouble("nel_water_bump_speed_u", v.x);
	e.setDouble("nel_water_bump_speed_v", v.y);

	{
		NLMISC::CVector2f c0, c1, pos;
		ws->getColorMapMat(c0, c1, pos);
		CJsonValue *a = e.setArray("nel_water_color_mat");
		a->pushDouble(c0.x);
		a->pushDouble(c0.y);
		a->pushDouble(c1.x);
		a->pushDouble(c1.y);
		a->pushDouble(pos.x);
		a->pushDouble(pos.y);
	}

	const NLMISC::CPolygon2D &poly = ws->getShape();
	{
		CJsonValue *a = e.setArray("nel_water_poly");
		for (size_t i = 0; i < poly.Vertices.size(); ++i)
		{
			a->pushDouble(poly.Vertices[i].x);
			a->pushDouble(poly.Vertices[i].y);
		}
	}

	e.setDouble("nel_water_height_factor", ws->getWaveHeightFactor());
	e.setInt("nel_water_pool_id", (sint64)ws->getWaterPoolID());
	e.setDouble("nel_water_transition_ratio", ws->getTransitionRatio());
	e.setBool("nel_water_splash", ws->isSplashEnabled());
	e.setBool("nel_water_scene_env_above", ws->getUseSceneWaterEnvMap(0));
	e.setBool("nel_water_scene_env_under", ws->getUseSceneWaterEnvMap(1));

	putVector(e, "nel_water_def_pos", ws->getDefaultPos()->getDefaultValue());
	putVector(e, "nel_water_def_scale", ws->getDefaultScale()->getDefaultValue());
	putQuat(e, "nel_water_def_rot", ws->getDefaultRotQuat()->getDefaultValue());
	return true;
}

static IShape *waterFromExtras(const CJsonValue &e, std::string *err)
{
	CWaterShape *ws = new CWaterShape;
	std::vector<std::string> files;
	bool ok = true;
	ok = ok && getTexFiles(e, "nel_water_env", files);
	if (ok && !files.empty())
		ws->setEnvMap(0, texFromFiles(files));
	ok = ok && getTexFiles(e, "nel_water_env_under", files);
	if (ok && !files.empty())
		ws->setEnvMap(1, texFromFiles(files));
	if (!ok)
	{
		if (err) *err = "bad water env map extras";
		delete ws;
		return NULL;
	}
	{
		std::vector<std::string> one(1);
		one[0] = e.getString("nel_water_displace_file", "");
		ws->setHeightMap(0, texFromFiles(one));
		one[0] = e.getString("nel_water_bump_file", "");
		ws->setHeightMap(1, texFromFiles(one));
	}
	{
		std::string cf = e.getString("nel_water_color_file", "");
		if (!cf.empty())
		{
			std::vector<std::string> one(1, cf);
			ws->setColorMap(texFromFiles(one));
		}
	}

	ws->setHeightMapScale(0, CVector2f((float)e.getDouble("nel_water_displace_scale_u", 1.0),
	                                   (float)e.getDouble("nel_water_displace_scale_v", 1.0)));
	ws->setHeightMapSpeed(0, CVector2f((float)e.getDouble("nel_water_displace_speed_u", 0.0),
	                                   (float)e.getDouble("nel_water_displace_speed_v", 0.0)));
	ws->setHeightMapScale(1, CVector2f((float)e.getDouble("nel_water_bump_scale_u", 1.0),
	                                   (float)e.getDouble("nel_water_bump_scale_v", 1.0)));
	ws->setHeightMapSpeed(1, CVector2f((float)e.getDouble("nel_water_bump_speed_u", 0.0),
	                                   (float)e.getDouble("nel_water_bump_speed_v", 0.0)));

	{
		const CJsonValue *a = e.get("nel_water_color_mat");
		if (a && a->isArray() && a->size() == 6)
			ws->setColorMapMat(CVector2f(a->at(0)->asFloat(), a->at(1)->asFloat()),
			                   CVector2f(a->at(2)->asFloat(), a->at(3)->asFloat()),
			                   CVector2f(a->at(4)->asFloat(), a->at(5)->asFloat()));
	}

	{
		const CJsonValue *a = e.get("nel_water_poly");
		if (!a || !a->isArray() || a->size() < 6 || a->size() % 2)
		{
			if (err) *err = "bad water polygon extras";
			delete ws;
			return NULL;
		}
		NLMISC::CPolygon2D poly;
		poly.Vertices.resize(a->size() / 2);
		for (size_t i = 0; i < poly.Vertices.size(); ++i)
			poly.Vertices[i].set(a->at(i * 2)->asFloat(), a->at(i * 2 + 1)->asFloat());
		ws->setShape(poly);
	}

	ws->setWaveHeightFactor((float)e.getDouble("nel_water_height_factor", 1.0));
	ws->setWaterPoolID((uint32)e.getInt("nel_water_pool_id", 0));
	ws->setTransitionRatio((float)e.getDouble("nel_water_transition_ratio",
		ws->getTransitionRatio()));
	ws->enableSplash(e.getBool("nel_water_splash", true));
	ws->setUseSceneWaterEnvMap(0, e.getBool("nel_water_scene_env_above", false));
	ws->setUseSceneWaterEnvMap(1, e.getBool("nel_water_scene_env_under", false));

	CVector pos, scale;
	CQuat rot;
	if (getVector(e, "nel_water_def_pos", pos))
		ws->getDefaultPos()->setDefaultValue(pos);
	if (getVector(e, "nel_water_def_scale", scale))
		ws->getDefaultScale()->setDefaultValue(scale);
	if (getQuat(e, "nel_water_def_rot", rot))
		ws->getDefaultRotQuat()->setDefaultValue(rot);
	return ws;
}

// ---------------------------------------------------------------------------------------------
// Remanence

static bool remanenceToExtras(CSegRemanenceShape *srs, CJsonValue &e, std::string *err)
{
	e.setInt("nel_rem_num_slices", (sint64)srs->getNumSlices());
	e.setDouble("nel_rem_slice_time", srs->getSliceTime());
	e.setDouble("nel_rem_rollup_ratio", srs->getRollupRatio());
	e.setBool("nel_rem_texture_shifting", srs->getTextureShifting());
	{
		CJsonValue *m = e.setObject("nel_rem_material");
		if (!materialToExtras(srs->getMaterial(), *m, err))
			return false;
	}
	{
		CJsonValue *a = e.setArray("nel_rem_corners");
		for (uint k = 0; k < srs->getNumCorners(); ++k)
		{
			CVector c = srs->getCorner(k);
			a->pushDouble(c.x);
			a->pushDouble(c.y);
			a->pushDouble(c.z);
		}
	}
	if (srs->getAnimatedMaterial())
		e.setString("nel_rem_anim_material", srs->getAnimatedMaterial()->Name);
	putVector(e, "nel_rem_def_pos", srs->getDefaultPos()->getDefaultValue());
	putVector(e, "nel_rem_def_scale", srs->getDefaultScale()->getDefaultValue());
	putQuat(e, "nel_rem_def_rot", srs->getDefaultRotQuat()->getDefaultValue());
	return true;
}

static IShape *remanenceFromExtras(const CJsonValue &e, std::string *err)
{
	CSegRemanenceShape *srs = new CSegRemanenceShape;
	srs->setNumSlices((uint32)e.getInt("nel_rem_num_slices", 2));
	srs->setSliceTime((float)e.getDouble("nel_rem_slice_time", 0.05));
	srs->setRollupRatio((float)e.getDouble("nel_rem_rollup_ratio", 1.0));
	{
		const CJsonValue *m = e.get("nel_rem_material");
		NL3D::CMaterial mat;
		if (!m || !materialFromExtras(*m, mat, err))
		{
			if (err && err->empty()) *err = "missing remanence material extras";
			delete srs;
			return NULL;
		}
		srs->setMaterial(mat);
	}
	{
		const CJsonValue *a = e.get("nel_rem_corners");
		if (!a || !a->isArray() || a->size() < 6 || a->size() % 3)
		{
			if (err) *err = "bad remanence corners extras";
			delete srs;
			return NULL;
		}
		srs->setNumCorners((uint)(a->size() / 3));
		for (size_t k = 0; k < a->size() / 3; ++k)
			srs->setCorner((uint)k, CVector(a->at(k * 3)->asFloat(), a->at(k * 3 + 1)->asFloat(),
			                                a->at(k * 3 + 2)->asFloat()));
	}
	srs->setTextureShifting(e.getBool("nel_rem_texture_shifting", false));
	{
		std::string am = e.getString("nel_rem_anim_material", "");
		if (!am.empty())
			srs->setAnimatedMaterial(am);
	}
	CVector pos, scale;
	CQuat rot;
	if (getVector(e, "nel_rem_def_pos", pos))
		srs->getDefaultPos()->setDefaultValue(pos);
	if (getVector(e, "nel_rem_def_scale", scale))
		srs->getDefaultScale()->setDefaultValue(scale);
	if (getQuat(e, "nel_rem_def_rot", rot))
		srs->getDefaultRotQuat()->setDefaultValue(rot);
	return srs;
}

// ---------------------------------------------------------------------------------------------
// Flare

static bool flareToExtras(CFlareShape *fs, CJsonValue &e, std::string *err)
{
	e.setString("nel_flare_color", colorToHex(fs->getColor()));
	e.setDouble("nel_flare_persistence", fs->getPersistence());
	e.setDouble("nel_flare_spacing", fs->getFlareSpacing());
	e.setBool("nel_flare_attenuable", fs->getAttenuable());
	e.setDouble("nel_flare_atten_range", fs->getAttenuationRange());
	e.setBool("nel_flare_first_keep_size", fs->getFirstFlareKeepSize());
	e.setString("nel_flare_dazzle_color", colorToHex(fs->getDazzleColor()));
	e.setDouble("nel_flare_dazzle_atten_range", fs->getDazzleAttenuationRange());
	e.setDouble("nel_flare_max_view_dist", fs->getMaxViewDist());
	e.setDouble("nel_flare_max_view_dist_ratio", fs->getMaxViewDistRatio());
	e.setBool("nel_flare_infinite_dist", fs->getFlareAtInfiniteDist());
	e.setBool("nel_flare_scale_disappear", fs->getScaleWhenDisappear());
	e.setDouble("nel_flare_size_disappear", fs->getSizeDisappear());
	e.setDouble("nel_flare_angle_disappear", fs->getAngleDisappear());
	e.setBool("nel_flare_look_at", fs->getLookAtMode());
	if (!fs->getOcclusionTestMeshName().empty())
		e.setString("nel_flare_occlusion_mesh", fs->getOcclusionTestMeshName());
	e.setBool("nel_flare_occlusion_inherit_scale_rot", fs->getOcclusionTestMeshInheritScaleRot());
	{
		CJsonValue *sizes = e.setArray("nel_flare_sizes");
		CJsonValue *rpos = e.setArray("nel_flare_rel_pos");
		CJsonValue *texs = e.setArray("nel_flare_textures");
		for (uint k = 0; k < MaxFlareNum; ++k)
		{
			sizes->pushDouble(fs->getSize(k));
			rpos->pushDouble(fs->getRelativePos(k));
			const ITexture *t = fs->getTexture(k);
			if (!t)
				texs->pushString("");
			else if (const CTextureFile *tf = dynamic_cast<const CTextureFile *>(t))
				texs->pushString(tf->getFileName());
			else
			{
				if (err) *err = "flare texture class not carried";
				return false;
			}
		}
	}
	putVector(e, "nel_flare_def_pos", fs->getDefaultPos()->getDefaultValue());
	return true;
}

static IShape *flareFromExtras(const CJsonValue &e, std::string *err)
{
	CFlareShape *fs = new CFlareShape;
	CRGBA c;
	if (hexToColor(e.getString("nel_flare_color", ""), c))
		fs->setColor(c);
	fs->setPersistence((float)e.getDouble("nel_flare_persistence", fs->getPersistence()));
	fs->setFlareSpacing((float)e.getDouble("nel_flare_spacing", fs->getFlareSpacing()));
	fs->setAttenuable(e.getBool("nel_flare_attenuable", false));
	fs->setAttenuationRange((float)e.getDouble("nel_flare_atten_range", fs->getAttenuationRange()));
	fs->setFirstFlareKeepSize(e.getBool("nel_flare_first_keep_size", false));
	if (hexToColor(e.getString("nel_flare_dazzle_color", ""), c))
		fs->setDazzleColor(c);
	fs->setDazzleAttenuationRange((float)e.getDouble("nel_flare_dazzle_atten_range",
		fs->getDazzleAttenuationRange()));
	fs->setMaxViewDist((float)e.getDouble("nel_flare_max_view_dist", fs->getMaxViewDist()));
	fs->setMaxViewDistRatio((float)e.getDouble("nel_flare_max_view_dist_ratio",
		fs->getMaxViewDistRatio()));
	fs->setFlareAtInfiniteDist(e.getBool("nel_flare_infinite_dist", false));
	fs->setScaleWhenDisappear(e.getBool("nel_flare_scale_disappear", false));
	fs->setSizeDisappear((float)e.getDouble("nel_flare_size_disappear", fs->getSizeDisappear()));
	fs->setAngleDisappear((float)e.getDouble("nel_flare_angle_disappear", fs->getAngleDisappear()));
	fs->setLookAtMode(e.getBool("nel_flare_look_at", false));
	{
		std::string om = e.getString("nel_flare_occlusion_mesh", "");
		if (!om.empty())
			fs->setOcclusionTestMeshName(om);
	}
	fs->setOcclusionTestMeshInheritScaleRot(e.getBool("nel_flare_occlusion_inherit_scale_rot", false));
	{
		const CJsonValue *sizes = e.get("nel_flare_sizes");
		const CJsonValue *rpos = e.get("nel_flare_rel_pos");
		const CJsonValue *texs = e.get("nel_flare_textures");
		if (!sizes || sizes->size() != MaxFlareNum || !rpos || rpos->size() != MaxFlareNum
			|| !texs || texs->size() != MaxFlareNum)
		{
			if (err) *err = "bad flare per-slot extras";
			delete fs;
			return NULL;
		}
		for (uint k = 0; k < MaxFlareNum; ++k)
		{
			fs->setSize(k, sizes->at(k)->asFloat());
			fs->setRelativePos(k, rpos->at(k)->asFloat());
			const std::string &tn = texs->at(k)->asString();
			if (!tn.empty())
				fs->setTexture(k, new CTextureFile(tn));
			else
				fs->setTexture(k, NULL);
		}
	}
	CVector pos;
	if (getVector(e, "nel_flare_def_pos", pos))
		fs->getDefaultPos()->setDefaultValue(pos);
	return fs;
}

// ---------------------------------------------------------------------------------------------
// Dispatch + serialization

bool specialShapeToExtras(IShape *shape, CJsonValue &extras, std::string *classOut,
                          std::string *err)
{
	if (err) err->clear();
	if (CWaterShape *ws = dynamic_cast<CWaterShape *>(shape))
	{
		if (classOut) *classOut = "water";
		return waterToExtras(ws, extras, err);
	}
	if (CSegRemanenceShape *srs = dynamic_cast<CSegRemanenceShape *>(shape))
	{
		if (classOut) *classOut = "remanence";
		return remanenceToExtras(srs, extras, err);
	}
	if (CFlareShape *fs = dynamic_cast<CFlareShape *>(shape))
	{
		if (classOut) *classOut = "flare";
		return flareToExtras(fs, extras, err);
	}
	return false;
}

IShape *specialShapeFromExtras(const CJsonValue &extras, const std::string &shapeClass,
                               std::string *err)
{
	if (shapeClass == "water")
		return waterFromExtras(extras, err);
	if (shapeClass == "remanence")
		return remanenceFromExtras(extras, err);
	if (shapeClass == "flare")
		return flareFromExtras(extras, err);
	if (err) *err = "unknown structural shape class " + shapeClass;
	return NULL;
}

bool specialShapeToFileBytes(IShape *shape, std::vector<uint8> &out, std::string *err)
{
	// Same route as the direct exporter (pipeline_max_export_shape/main.cpp): export-era
	// stream flags, temp COFile, then the CWaterShape v7->v4 patch + 14-byte truncation.
	bool oldVB = CVertexBuffer::SerialOldPreferredMemory;
	bool oldIB = CIndexBuffer::SerialOldPreferredMemory;
	CVertexBuffer::SerialOldPreferredMemory = true;
	CIndexBuffer::SerialOldPreferredMemory = true;
	char tmpPath[256];
	snprintf(tmpPath, sizeof(tmpPath), "/tmp/nel_gltf_special_shape.%d.tmp", (int)NLGLTF_SS_GETPID());
	bool ok = false;
	try
	{
		{
			COFile ofile;
			if (!ofile.open(tmpPath))
			{
				if (err) *err = "cannot open temp file";
				throw NLMISC::Exception("temp open");
			}
			CShapeStream shapeStream(shape);
			shapeStream.serial(ofile);
			ofile.close();
		}
		{
			CIFile ifile;
			if (!ifile.open(tmpPath))
			{
				if (err) *err = "cannot reopen temp file";
				throw NLMISC::Exception("temp reopen");
			}
			out.resize(ifile.getFileSize());
			if (!out.empty())
				ifile.serialBuffer(&out[0], (uint)out.size());
			ifile.close();
		}
		CFile::deleteFile(tmpPath);
		std::string className = shape->getClassName();
		uint32 len = (uint32)out.size();
		uint32 waterVerOff = 4 + 8 + 4 + (uint32)className.size();
		if (className == "CWaterShape" && waterVerOff < len && out[waterVerOff] == 7 && len > 14)
		{
			out[waterVerOff] = 4;
			out.resize(len - 14);
		}
		ok = true;
	}
	catch (const NLMISC::Exception &e)
	{
		if (err && err->empty()) *err = e.what();
	}
	CVertexBuffer::SerialOldPreferredMemory = oldVB;
	CIndexBuffer::SerialOldPreferredMemory = oldIB;
	return ok;
}

} /* namespace NLGLTF */

/* end of file */
