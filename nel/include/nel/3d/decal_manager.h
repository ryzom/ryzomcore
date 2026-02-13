/** \file decal_manager.h
 * Batched decal rendering manager for NeL 3D.
 *
 * Collects decals sorted by material and renders them in batched draw calls
 * using a fixed-size AGP volatile vertex buffer. See PDF §4.5.4 and §.6.3.
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_DECAL_MANAGER_H
#define NL_DECAL_MANAGER_H

#include "nel/3d/transform.h"

#include "nel/3d/decal.h"
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/vertex_program.h"
#include "nel/3d/driver.h"


namespace NL3D
{

class CScene;


/// Maximum number of vertices in the batching array buffer (see PDF §4.5.4).
/// When exceeded, the buffer is flushed and refilled.
const uint32 NL3D_DECAL_VB_MAX_VERTICES = 4096 * 3;


// ***************************************************************************
/**
 * Vertex program for decal distance attenuation, bottom/top Z blending,
 * and diffuse color modulation.
 *
 * Ported from the legacy CLegacyDecal system's DecalAttenuationVertexProgram.
 *
 * Constants:
 *   c[0-3]: ModelViewProjection matrix
 *   c[4]:   WorldToUV row 0 (world X,Y,Z,W → U)
 *   c[5]:   WorldToUV row 1 (world X,Y,Z,W → V)
 *   c[6]:   Camera position (world space, relative to model origin)
 *   c[7]:   DistScaleBias (x=scale, y=bias, z=0.0, w=1.0)
 *   c[8]:   Diffuse color (RGB normalized to [0,1])
 *   c[11]:  BlendScale (x=bottomScale, y=bottomBias, z=topScale, w=topBias)
 */
class CVertexProgramDecalAttenuation : public CVertexProgram
{
public:
	struct CIdx
	{
		uint WorldToUV0;
		uint WorldToUV1;
		uint RefCamDist;
		uint DistScaleBias;
		uint Diffuse;
		uint BlendScale;
	};

	CVertexProgramDecalAttenuation();
	~CVertexProgramDecalAttenuation();
	virtual void buildInfo();
	inline const CIdx &idx() const { return m_Idx; }

private:
	CIdx m_Idx;
};


// ***************************************************************************
/**
 * Decal Manager: collects decals per material and renders them in batched draw calls.
 *
 * Each frame, CDecal::traverseRender() registers decals here. At flush time,
 * the manager iterates per-material groups, fills a fixed-size AGP volatile
 * vertex buffer (position + UV + color), and issues renderRawTriangles() calls.
 * If the buffer overflows mid-decal, it is flushed and refilled (see PDF §.6.3).
 *
 * Supports a vertex program path (attenuation, Z blending, diffuse) and a CPU
 * fallback path using per-vertex colors.
 *
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecalManager
{

public:
	CDecalManager();
	~CDecalManager();

	/** Render all registered decals in batched draw calls.
	  * Iterates per-material, fills the VB, and renders. See PDF §.6.3.
	  * \param sc Owner scene (for driver access)
	  */
	void flush(CScene *sc);

	/** Register a decal for rendering this frame.
	  * \param decal The decal to add
	  * \param materialId Material ID for batching (decals with same ID are grouped)
	  */
	void addDecal(CDecal *decal, uint32 materialId);

	/** Register a material for use by decals.
	  * \param mat The material to register
	  * \return The material ID to use when creating decals
	  */
	uint32 registerMaterial(const CMaterial &mat);

	/// Clear all registered decals (called at start of each frame).
	void clearAllDecals();

	/** Set whether vertex programs should be used.
	  * \param b true to use vertex programs
	  */
	void setVertexProgram(const bool b) { _UseVertexProgram = b; }

	/** Set distance attenuation parameters (affects all decals).
	  * At distance d from camera: alpha *= d * scale + bias.
	  * Typically scale = -factor/maxDist, bias = factor.
	  * \param scale Distance scale factor
	  * \param bias Distance bias
	  */
	void setDistAttenuation(float scale, float bias) { _DistScale = scale; _DistBias = bias; }

private:
	/// A registered material with its ID
	struct CRegisteredMaterial
	{
		CMaterial	Mat;
		uint32		Id;
	};

	/// Decals grouped by material ID
	typedef std::map<uint32, std::vector<CDecal*> > TDecalMap;
	TDecalMap								_Decals;

	/// Registered materials by ID
	std::vector<CRegisteredMaterial>		_Materials;
	uint32									_NextMaterialId;

	bool									_UseVertexProgram;
	float									_DistScale;
	float									_DistBias;

	/// Fixed-size AGP volatile vertex buffer (Position + TexCoord0 + PrimaryColor)
	CVertexBuffer							_VB;

	/// The decal attenuation vertex program (shared instance)
	NLMISC::CSmartPtr<CVertexProgramDecalAttenuation>	_VertexProgram;
};



}//NL3D
#endif
