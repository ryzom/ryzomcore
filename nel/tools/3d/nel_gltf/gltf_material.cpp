/**
 * \file gltf_material.cpp
 * \brief See gltf_material.h.
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
#include "gltf_material.h"

#include <cstdio>
#include <cstring>

#include <nel/misc/mem_stream.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/texture_cube.h>

using namespace NLMISC;
using namespace NL3D;

namespace NLGLTF {

// ---------------------------------------------------------------------------------------------
// Small helpers

static std::string rgbaToHex(CRGBA c)
{
	char buf[12];
	snprintf(buf, sizeof(buf), "%02x%02x%02x%02x", c.R, c.G, c.B, c.A);
	return buf;
}

static bool hexToRgba(const std::string &s, CRGBA &out)
{
	if (s.size() != 8) return false;
	unsigned v[4];
	for (int i = 0; i < 4; ++i)
	{
		unsigned x = 0;
		for (int k = 0; k < 2; ++k)
		{
			char h = s[i * 2 + k];
			x <<= 4;
			if (h >= '0' && h <= '9') x |= (unsigned)(h - '0');
			else if (h >= 'a' && h <= 'f') x |= (unsigned)(h - 'a' + 10);
			else if (h >= 'A' && h <= 'F') x |= (unsigned)(h - 'A' + 10);
			else return false;
		}
		v[i] = x;
	}
	out.set((uint8)v[0], (uint8)v[1], (uint8)v[2], (uint8)v[3]);
	return true;
}

static std::string bytesToHex(const uint8 *d, uint len)
{
	std::string out;
	out.reserve(len * 2);
	char buf[4];
	for (uint i = 0; i < len; ++i)
	{
		snprintf(buf, sizeof(buf), "%02x", d[i]);
		out += buf;
	}
	return out;
}

static bool hexToBytes(const std::string &s, std::vector<uint8> &out)
{
	if (s.size() % 2) return false;
	out.resize(s.size() / 2);
	for (size_t i = 0; i < out.size(); ++i)
	{
		unsigned x = 0;
		for (int k = 0; k < 2; ++k)
		{
			char h = s[i * 2 + k];
			x <<= 4;
			if (h >= '0' && h <= '9') x |= (unsigned)(h - '0');
			else if (h >= 'a' && h <= 'f') x |= (unsigned)(h - 'a' + 10);
			else if (h >= 'A' && h <= 'F') x |= (unsigned)(h - 'A' + 10);
			else return false;
		}
		out[i] = (uint8)x;
	}
	return true;
}

// Canonical CTexEnv packing, independent of the compiler's bitfield layout. LSB upward:
// OpRGB(4) SrcArg0RGB(2) OpArg0RGB(2) SrcArg1RGB(2) OpArg1RGB(2) SrcArg2RGB(2) OpArg2RGB(2)
// OpAlpha(4) SrcArg0Alpha(2) OpArg0Alpha(2) SrcArg1Alpha(2) OpArg1Alpha(2) SrcArg2Alpha(2)
// OpArg2Alpha(2) — 32 bits total.
static uint32 packTexEnv(const CMaterial::CTexEnv &e)
{
	uint32 v = 0;
	uint shift = 0;
	v |= (e.Env.OpRGB & 0xF) << shift; shift += 4;
	v |= (e.Env.SrcArg0RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg0RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.SrcArg1RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg1RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.SrcArg2RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg2RGB & 0x3) << shift; shift += 2;
	v |= (e.Env.OpAlpha & 0xF) << shift; shift += 4;
	v |= (e.Env.SrcArg0Alpha & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg0Alpha & 0x3) << shift; shift += 2;
	v |= (e.Env.SrcArg1Alpha & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg1Alpha & 0x3) << shift; shift += 2;
	v |= (e.Env.SrcArg2Alpha & 0x3) << shift; shift += 2;
	v |= (e.Env.OpArg2Alpha & 0x3) << shift; shift += 2;
	return v;
}

static void unpackTexEnv(uint32 v, CMaterial::CTexEnv &e)
{
	uint shift = 0;
	e.Env.OpRGB = (v >> shift) & 0xF; shift += 4;
	e.Env.SrcArg0RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg0RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.SrcArg1RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg1RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.SrcArg2RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg2RGB = (v >> shift) & 0x3; shift += 2;
	e.Env.OpAlpha = (v >> shift) & 0xF; shift += 4;
	e.Env.SrcArg0Alpha = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg0Alpha = (v >> shift) & 0x3; shift += 2;
	e.Env.SrcArg1Alpha = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg1Alpha = (v >> shift) & 0x3; shift += 2;
	e.Env.SrcArg2Alpha = (v >> shift) & 0x3; shift += 2;
	e.Env.OpArg2Alpha = (v >> shift) & 0x3; shift += 2;
}

static uint32 defaultTexEnvPacked()
{
	CMaterial::CTexEnv def; // ctor = setDefault()
	return packTexEnv(def);
}

// ---------------------------------------------------------------------------------------------
// Texture descriptors. Key prefix is "nel_tex<i>_" (or "nel_tex<i>_cube_" for the cube's inner
// texture descriptor).

static void textureBaseToExtras(const ITexture &tex, CJsonValue &extras, const std::string &prefix)
{
	// ITexture base state, serialized subset (upload format, wraps, filters, grayscale-as-alpha)
	extras.setInt((prefix + "upload").c_str(), (sint64)tex.getUploadFormat());
	extras.setInt((prefix + "wrap_s").c_str(), (sint64)tex.getWrapS());
	extras.setInt((prefix + "wrap_t").c_str(), (sint64)tex.getWrapT());
	extras.setInt((prefix + "min_filter").c_str(), (sint64)tex.getMinFilter());
	extras.setInt((prefix + "mag_filter").c_str(), (sint64)tex.getMagFilter());
	extras.setBool((prefix + "gray_alpha").c_str(), tex.isGrayscaleAsAlpha());
}

static void textureBaseFromExtras(ITexture &tex, const CJsonValue &extras, const std::string &prefix)
{
	tex.setUploadFormat((ITexture::TUploadFormat)extras.getInt((prefix + "upload").c_str(), ITexture::Auto));
	tex.setWrapS((ITexture::TWrapMode)extras.getInt((prefix + "wrap_s").c_str(), ITexture::Repeat));
	tex.setWrapT((ITexture::TWrapMode)extras.getInt((prefix + "wrap_t").c_str(), ITexture::Repeat));
	tex.setFilterMode((ITexture::TMagFilter)extras.getInt((prefix + "mag_filter").c_str(), ITexture::Linear),
	                  (ITexture::TMinFilter)extras.getInt((prefix + "min_filter").c_str(), ITexture::LinearMipMapLinear));
	tex.loadGrayscaleAsAlpha(extras.getBool((prefix + "gray_alpha").c_str(), true));
}

// The inner (non-cube) texture: CTextureFile or CTextureMultiFile.
static bool flatTextureToExtras(const ITexture *tex, CJsonValue &extras, const std::string &prefix,
                                std::string *err)
{
	if (const CTextureFile *tf = dynamic_cast<const CTextureFile *>(tex))
	{
		extras.setString((prefix + "class").c_str(), "file");
		extras.setString((prefix + "file").c_str(), tf->getFileName());
		extras.setBool((prefix + "degrade").c_str(), tf->allowDegradation());
		return true;
	}
	if (const CTextureMultiFile *tm = dynamic_cast<const CTextureMultiFile *>(tex))
	{
		extras.setString((prefix + "class").c_str(), "multi");
		CJsonValue *files = extras.setArray((prefix + "files").c_str());
		for (uint i = 0; i < tm->getNumFileName(); ++i)
			files->pushString(tm->getFileName(i));
		// _CurrSelectedTexture serializes; default ctor value 0
		extras.setInt((prefix + "sel").c_str(), 0);
		return true;
	}
	if (err) *err = "unsupported texture class " + std::string(const_cast<ITexture *>(tex)->getClassName());
	return false;
}

static ITexture *flatTextureFromExtras(const CJsonValue &extras, const std::string &prefix,
                                       std::string *err)
{
	std::string cls = extras.getString((prefix + "class").c_str(), "");
	if (cls == "file")
	{
		CTextureFile *tf = new CTextureFile;
		tf->setFileName(extras.getString((prefix + "file").c_str(), ""));
		tf->setAllowDegradation(extras.getBool((prefix + "degrade").c_str(), true));
		return tf;
	}
	if (cls == "multi")
	{
		const CJsonValue *files = extras.get((prefix + "files").c_str());
		uint n = files && files->isArray() ? (uint)files->size() : 0;
		CTextureMultiFile *tm = new CTextureMultiFile(n);
		for (uint i = 0; i < n; ++i)
		{
			const CJsonValue *f = files->at(i);
			if (f && f->isString())
				tm->setFileName(i, f->asString().c_str());
		}
		return tm;
	}
	if (err) *err = "unknown texture class '" + cls + "' at " + prefix;
	return NULL;
}

static bool textureToExtras(const ITexture *tex, CJsonValue &extras, const std::string &prefix,
                            std::string *err)
{
	if (const CTextureCube *tc = dynamic_cast<const CTextureCube *>(tex))
	{
		// Export cubes hold 6 fresh copies of one source texture (the specular-shader side-2
		// duplication in buildATexture). Verify that invariant and encode the single source.
		CTextureCube *ncc = const_cast<CTextureCube *>(tc);
		const ITexture *face0 = ncc->getTexture((CTextureCube::TFace)0);
		if (!face0)
		{
			if (err) *err = "cube texture with empty face";
			return false;
		}
		for (uint i = 1; i < 6; ++i)
		{
			ITexture *fi = ncc->getTexture((CTextureCube::TFace)i);
			if (!fi || fi->getClassName() != const_cast<ITexture *>(face0)->getClassName())
			{
				if (err) *err = "cube texture faces are not uniform";
				return false;
			}
		}
		extras.setString((prefix + "class").c_str(), "cube");
		if (!flatTextureToExtras(face0, extras, prefix + "cube_", err))
			return false;
		textureBaseToExtras(*face0, extras, prefix + "cube_");
		textureBaseToExtras(*tex, extras, prefix);
		return true;
	}
	if (!flatTextureToExtras(tex, extras, prefix, err))
		return false;
	textureBaseToExtras(*tex, extras, prefix);
	return true;
}

static ITexture *textureFromExtras(const CJsonValue &extras, const std::string &prefix,
                                   std::string *err)
{
	std::string cls = extras.getString((prefix + "class").c_str(), "");
	if (cls.empty())
		return NULL;
	if (cls == "cube")
	{
		// Rebuild the source texture, then duplicate into the 6 faces in the same face order
		// buildATexture uses (positive_z, negative_z, negative_x, positive_x, positive_y,
		// negative_y — but since all faces are copies of one texture, order only matters for
		// the serialPolyPtr id sequence, which follows the _Textures array order regardless).
		ITexture *src = flatTextureFromExtras(extras, prefix + "cube_", err);
		if (!src) return NULL;
		textureBaseFromExtras(*src, extras, prefix + "cube_");
		CTextureCube *cube = new CTextureCube;
		for (uint i = 0; i < 6; ++i)
		{
			ITexture *copy;
			if (CTextureMultiFile *tm = dynamic_cast<CTextureMultiFile *>(src))
				copy = new CTextureMultiFile(*tm);
			else
				copy = new CTextureFile(*static_cast<CTextureFile *>(src));
			cube->setTexture((CTextureCube::TFace)i, copy);
		}
		delete src;
		textureBaseFromExtras(*cube, extras, prefix);
		return cube;
	}
	ITexture *tex = flatTextureFromExtras(extras, prefix, err);
	if (!tex) return NULL;
	textureBaseFromExtras(*tex, extras, prefix);
	return tex;
}

// ---------------------------------------------------------------------------------------------

bool materialToExtras(const CMaterial &mat, CJsonValue &extras, std::string *err)
{
	CMaterial &m = const_cast<CMaterial &>(mat); // several getters are non-const

	if (!m._LightMaps.empty())
	{
		if (err) *err = "material carries lightmaps (out of codec scope; 1_export is unmapped)";
		return false;
	}

	extras.setInt("nel_mtl", 1); // codec schema version
	extras.setInt("nel_flags", (sint64)m.getFlags()); // verification dword
	extras.setBool("nel_lighted", m.isLighted());
	extras.setInt("nel_shader", (sint64)m.getShader());
	extras.setBool("nel_blend", m.getBlend());
	extras.setInt("nel_src_blend", (sint64)m.getSrcBlend());
	extras.setInt("nel_dst_blend", (sint64)m.getDstBlend());
	extras.setBool("nel_alpha_test", m.getAlphaTest());
	extras.setDouble("nel_alpha_test_thr", m.getAlphaTestThreshold());
	extras.setBool("nel_zwrite", m.getZWrite());
	extras.setInt("nel_zfunc", (sint64)m.getZFunc());
	extras.setDouble("nel_zbias", m.getZBias());
	extras.setString("nel_color", rgbaToHex(m.getColor()));
	extras.setString("nel_emissive", rgbaToHex(m.getEmissive()));
	extras.setString("nel_ambient", rgbaToHex(m.getAmbient()));
	extras.setString("nel_diffuse", rgbaToHex(m.getDiffuse()));
	extras.setString("nel_specular", rgbaToHex(m.getSpecular()));
	extras.setDouble("nel_shininess", m.getShininess());
	extras.setBool("nel_lighted_vcolor", m.isLightedVertexColor());
	extras.setBool("nel_stained_glass", m.getStainedGlassWindow());

	bool texAddr = false;
	for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		char prefixBuf[24];
		snprintf(prefixBuf, sizeof(prefixBuf), "nel_tex%u_", i);
		std::string prefix = prefixBuf;

		ITexture *tex = m.getTexture((uint8)i);
		if (tex)
		{
			if (!textureToExtras(tex, extras, prefix, err))
				return false;
		}

		// TexCoordGen + mode (only meaningful with a texture, but flags are per-stage)
		if (m.getTexCoordGen(i))
		{
			extras.setBool((prefix + "gen").c_str(), true);
			extras.setInt((prefix + "gen_mode").c_str(), (sint64)m.getTexCoordGenMode(i));
		}

		// TexEnv, when not default
		uint32 env = packTexEnv(m._TexEnvs[i]);
		CRGBA envColor = m._TexEnvs[i].ConstantColor;
		if (env != defaultTexEnvPacked() || envColor != CRGBA(255, 255, 255, 255))
		{
			extras.setInt((prefix + "env").c_str(), (sint64)env);
			extras.setString((prefix + "env_color").c_str(), rgbaToHex(envColor));
		}

		// User texture matrix: friendly floats + the exact CMatrix serial stream (StateBit-
		// conditional layout — the blob is what reproduces the .shape bytes; the floats are the
		// artist-visible view).
		if (m.isUserTexMatEnabled(i))
		{
			const NLMISC::CMatrix &um = m.getUserTexMat(i);
			CJsonValue *arr = extras.setArray((prefix + "usermat").c_str());
			const float *mm = um.get();
			for (uint k = 0; k < 16; ++k)
				arr->pushDouble(mm[k]);
			CMemStream ms;
			nlassert(!ms.isReading());
			const_cast<NLMISC::CMatrix &>(um).serial(ms);
			extras.setString((prefix + "usermat_blob").c_str(),
			                 bytesToHex(ms.buffer(), ms.length()));
		}

		if (m.getFlags() & IDRV_MAT_TEX_ADDR)
		{
			extras.setInt((prefix + "addr").c_str(), (sint64)m._TexAddrMode[i]);
			texAddr = true;
		}
	}
	(void)texAddr;

	return true;
}

bool materialFromExtras(const CJsonValue &extras, CMaterial &mat, std::string *err)
{
	if (extras.getInt("nel_mtl", 0) != 1)
	{
		if (err) *err = "missing or unsupported nel_mtl version";
		return false;
	}

	// The setter sequence mirrors material_build's reachable state space; order matters
	// (setShader resets textures, init* resets everything).
	bool lighted = extras.getBool("nel_lighted", false);
	if (lighted)
		mat.initLighted();
	else
		mat.initUnlit();

	mat.setShader((CMaterial::TShader)extras.getInt("nel_shader", CMaterial::Normal));

	// Textures
	for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		char prefixBuf[24];
		snprintf(prefixBuf, sizeof(prefixBuf), "nel_tex%u_", i);
		std::string prefix = prefixBuf;

		std::string cls = extras.getString((prefix + "class").c_str(), "");
		if (!cls.empty())
		{
			ITexture *tex = textureFromExtras(extras, prefix, err);
			if (!tex) return false;
			mat.setTexture((uint8)i, tex);
		}

		if (extras.getBool((prefix + "gen").c_str(), false))
		{
			mat.setTexCoordGen(i, true);
			mat.setTexCoordGenMode(i, (CMaterial::TTexCoordGenMode)extras.getInt((prefix + "gen_mode").c_str(), CMaterial::TexCoordGenReflect));
		}

		const CJsonValue *env = extras.get((prefix + "env").c_str());
		if (env && env->isNumber())
		{
			unpackTexEnv((uint32)env->asInt(), mat._TexEnvs[i]);
			CRGBA envColor(255, 255, 255, 255);
			std::string hex = extras.getString((prefix + "env_color").c_str(), "");
			if (!hex.empty() && !hexToRgba(hex, envColor))
			{
				if (err) *err = "bad " + prefix + "env_color";
				return false;
			}
			mat._TexEnvs[i].ConstantColor = envColor;
		}

		std::string blob = extras.getString((prefix + "usermat_blob").c_str(), "");
		if (!blob.empty())
		{
			std::vector<uint8> bytes;
			if (!hexToBytes(blob, bytes) || bytes.empty())
			{
				if (err) *err = "bad " + prefix + "usermat_blob";
				return false;
			}
			CMemStream ms;
			ms.fill(&bytes[0], (uint32)bytes.size());
			nlassert(ms.isReading());
			NLMISC::CMatrix um;
			um.serial(ms);
			mat.enableUserTexMat(i, true);
			mat.setUserTexMat(i, um);
		}

		const CJsonValue *addr = extras.get((prefix + "addr").c_str());
		if (addr && addr->isNumber())
		{
			if (!(mat.getFlags() & IDRV_MAT_TEX_ADDR))
				mat.enableTexAddrMode(true);
			mat.setTexAddressingMode((uint8)i, (CMaterial::TTexAddressingMode)addr->asInt());
		}
	}

	// Blend / alpha / z
	mat.setBlend(extras.getBool("nel_blend", false));
	mat.setSrcBlend((CMaterial::TBlend)extras.getInt("nel_src_blend", CMaterial::srcalpha));
	mat.setDstBlend((CMaterial::TBlend)extras.getInt("nel_dst_blend", CMaterial::invsrcalpha));
	mat.setAlphaTest(extras.getBool("nel_alpha_test", false));
	mat.setAlphaTestThreshold((float)extras.getDouble("nel_alpha_test_thr", 0.5));
	mat.setZWrite(extras.getBool("nel_zwrite", true));
	mat.setZFunc((CMaterial::ZFunc)extras.getInt("nel_zfunc", CMaterial::lessequal));
	mat.setZBias((float)extras.getDouble("nel_zbias", 0.0));

	// Colors
	CRGBA c;
	if (hexToRgba(extras.getString("nel_color", "ffffffff"), c)) mat.setColor(c);
	if (hexToRgba(extras.getString("nel_emissive", "000000ff"), c)) mat.setEmissive(c);
	if (hexToRgba(extras.getString("nel_ambient", "000000ff"), c)) mat.setAmbient(c);
	if (hexToRgba(extras.getString("nel_diffuse", "ffffffff"), c)) mat.setDiffuse(c);
	if (hexToRgba(extras.getString("nel_specular", "000000ff"), c)) mat.setSpecular(c);
	mat.setShininess((float)extras.getDouble("nel_shininess", 0.0));

	mat.setLightedVertexColor(extras.getBool("nel_lighted_vcolor", false));
	mat.setStainedGlassWindow(extras.getBool("nel_stained_glass", false));

	// Verification: the reconstructed flag dword must equal what the writer saw. A mismatch
	// means the codec's field set has a gap (or the extras were hand-edited inconsistently).
	sint64 expected = extras.getInt("nel_flags", -1);
	if (expected >= 0 && (uint32)expected != mat.getFlags())
	{
		if (err)
		{
			char buf[96];
			snprintf(buf, sizeof(buf), "flag mismatch: reconstructed 0x%08x, expected 0x%08x",
			         mat.getFlags(), (uint32)expected);
			*err = buf;
		}
		return false;
	}

	return true;
}

} /* namespace NLGLTF */

/* end of file */
