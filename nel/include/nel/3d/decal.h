/** \file decal.h
 * Projected texture decal system for NeL 3D.
 *
 * Based on the design from the intern report by Christopher Tarento (2007).
 * Decals project textures onto scene geometry using a unit-cube bounding box,
 * quad-grid face selection, and batched rendering through CDecalManager.
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

#ifndef NL_DECAL_H
#define NL_DECAL_H


#include "nel/3d/transform.h"

#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/shadow_poly_receiver.h"
#include "nel/misc/polygon.h"


namespace NLMISC
{

class CPlane;

}


namespace NL3D
{

class CScene;
class CDecalManager;

class UScene;
class UDriver;


// ***************************************************************************
const NLMISC::CClassId		DecalId=NLMISC::CClassId(0x6a570fe6, 0x32323e16);


// ***************************************************************************
/// Clipping mode for decal face selection (see PDF §4.5.2)
enum TDecalClipMode
{
	/// No clipping: selected faces are used as-is
	DecalClipNone = 0,
	/// Mask clipping: use a second texture stage as a mask to delimit edges (preserves geometry, saves CPU)
	DecalClipMask,
	/// Geometry clipping: clip vertices against bounding box planes (saves fillrate, costs CPU)
	DecalClipGeometry
};


// ***************************************************************************
/**
 * Context for decal face selection through visual collision.
 * Carries clip planes, bounding box, and destination triangle list.
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecalContext
{
public:
	CDecalContext();

public:
	std::vector<CPlane>		WorldClipPlanes;
	CAABBox					WorldBBox;
	NLMISC::CPolygon2D		Poly2D;
	CMatrix					WorldMatrix;
	std::vector<CVector>	*DestTris;
	TDecalClipMode			ClipMode;
	bool					ClipDownFacing;
};


// ***************************************************************************
/**
 * A projected texture decal in the scene graph.
 *
 * The decal is a CTransform-based model with a unit-cube projection volume.
 * It overrides traverseClip() for bounding-sphere frustum culling (§.6.1)
 * and traverseRender() to register with CDecalManager for batched rendering.
 *
 * Face selection uses quad-grid + clip-plane refinement via CVisualCollisionMesh.
 * UV coordinates are generated from the inverse world matrix of the unit cube (§4.5.3).
 *
 * Supports a vertex program for distance-based attenuation, bottom/top Z blending,
 * and per-vertex diffuse color (ported from the legacy CLegacyDecal system).
 *
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecal : public CTransform
{
public:
	/// Constructor
	CDecal();

	/// Destructor
	~CDecal();

	/// Initialization after insertion in scene graph.
	void initModel();

	/// Register CDecal as a valid model type for scene auto-registration.
	static void registerBasic();

	/** Clip method override.
	  * Uses bounding sphere vs frustum test for fast culling (see PDF §.6.1).
	  * \return true if the decal is visible
	  */
	bool clip();

	/** Render traversal.
	  * Registers this decal with the CDecalManager for batched rendering.
	  */
	void traverseRender();

	/// Get the decal's material (for texture/blend setup).
	CMaterial &getMaterial() { return _Mat; }

	/** Get the decal's material ID for batching.
	  * Decals sharing the same material ID are batched together in the manager.
	  */
	uint32 getMaterialId() const { return _MaterialId; }

	/** Set the decal's material ID.
	  * This ID is assigned when registering a material with CDecalManager.
	  */
	void setMaterialId(uint32 id) { _MaterialId = id; }

	/** Set the decal texture from a filename.
	  * \param filename Path to the texture file.
	  */
	void setTexture(const std::string &filename);

	/** Get vertices and UVs for rendering.
	  * Recomputes if the decal is touched (moved or camera moved beyond threshold).
	  * \return vector of vertices (position interleaved, 3 per triangle)
	  */
	std::vector<CVector> &getVertices(const bool useVertexProgram);

	/** Get UV coordinates corresponding to the vertices.
	  * Valid after calling getVertices().
	  * \return vector of UV coordinates (one per vertex)
	  */
	const std::vector<CUV> &getUVs() const { return _UVs; }

	/** Get per-vertex RGBA colors corresponding to the vertices.
	  * Valid after calling getVertices(). Contains diffuse color + computed alpha
	  * (distance attenuation × bottom blend × top blend).
	  * Used by the CPU fallback path when vertex programs are not available.
	  * \return vector of RGBA colors (one per vertex)
	  */
	const std::vector<NLMISC::CRGBA> &getColors() const { return _Colors; }

	/** Set UV sub-region within a texture atlas.
	  * \param uv1 Top-left UV coordinate
	  * \param uv2 Bottom-right UV coordinate
	  */
	void setUVCoord(const CUV uv1, const CUV uv2);

	/** Set the clipping mode for face selection.
	  * \param mode One of DecalClipNone, DecalClipMask, DecalClipGeometry
	  */
	void setClippingMode(TDecalClipMode mode) { _DecalContext.ClipMode = mode; }

	/// Get the current clipping mode.
	TDecalClipMode getClippingMode() const { return _DecalContext.ClipMode; }

	/** Mark this decal as static for caching optimization.
	  * Static decals only recompute geometry when first created or when
	  * the camera moves beyond the visibility distance threshold (see PDF §4.5.4).
	  * \param isStatic true for static decals
	  */
	void setStatic(const bool isStatic) { _IsStatic = isStatic; }

	/// Return whether this decal is marked static.
	bool isStatic() const { return _IsStatic; }

	/** Set the render priority (0 = highest, 7 = lowest).
	  * Decals with lower priority values are rendered first within their material group.
	  * \param priority Value in [0, 7]
	  */
	void setPriority(uint8 priority) { _Priority = (priority < 8) ? priority : 7; }

	/// Get the current render priority.
	uint8 getPriority() const { return _Priority; }

	/** Set whether downward-facing surfaces should be clipped.
	  * When enabled, triangles whose normal has a negative Z component
	  * are excluded from the decal projection.
	  * \param clipDownFacing true to clip down-facing surfaces
	  */
	void setClipDownFacing(bool clipDownFacing) { _ClipDownFacing = clipDownFacing; }

	/// Get whether down-facing clip is enabled.
	bool getClipDownFacing() const { return _ClipDownFacing; }

	/** Set a custom UV matrix (world → UV transform).
	  * When enabled, this matrix replaces the default inverse-world UV generation.
	  * \param on true to enable, false to return to default UV generation
	  * \param matrix The world-to-UV matrix (only used when on=true)
	  */
	void setCustomUVMatrix(bool on, const CMatrix &matrix = CMatrix::Identity);

	/** Set the texture coordinate transform matrix.
	  * Applied to the UV generation pipeline (multiplied with the inverse-world matrix).
	  * \param matrix The texture transform matrix
	  */
	void setTextureMatrix(const CMatrix &matrix);

	/** Set the world matrix for an arrow-shaped decal.
	  * Convenience method that computes a world matrix for a decal stretched
	  * from start to end with the given half-width.
	  * \param start 2D start position
	  * \param end 2D end position
	  * \param halfWidth Half-width of the arrow
	  */
	void setWorldMatrixForArrow(const NLMISC::CVector2f &start, const NLMISC::CVector2f &end, float halfWidth);

	/** Set the world matrix for a spot-shaped decal.
	  * Convenience method that computes a world matrix for a circular decal
	  * centered at pos with given radius and optional rotation.
	  * \param pos 2D center position
	  * \param radius Radius of the spot
	  * \param angleInRadians Optional rotation angle
	  */
	void setWorldMatrixForSpot(const NLMISC::CVector2f &pos, float radius, float angleInRadians = 0.f);

	/** Test if a 2D point is contained within this decal's projection area.
	  * Used by R2 editor tools for hit-testing.
	  * \param pos 2D point to test (world XY)
	  * \return true if the point falls within the unit-cube projection
	  */
	bool contains(const NLMISC::CVector2f &pos) const;

	/** Set the diffuse color applied to the decal.
	  * The RGB components tint the texture, alpha is a base opacity.
	  * \param diffuse RGBA diffuse color
	  */
	void setDiffuse(NLMISC::CRGBA diffuse) { _Diffuse = diffuse; }

	/// Get the current diffuse color.
	NLMISC::CRGBA getDiffuse() const { return _Diffuse; }

	/** Set the emissive color added to the decal.
	  * Added on top of the texture × diffuse result.
	  * \param emissive RGBA emissive color
	  */
	void setEmissive(NLMISC::CRGBA emissive);

	/// Get the current emissive color.
	NLMISC::CRGBA getEmissive() const { return _Emissive; }

	/** Set the bottom Z-blend region.
	  * Decal alpha fades from 0 at zMin to 1 at zMax (bottom edge).
	  * \param zMin Altitude below which the decal is fully transparent
	  * \param zMax Altitude above which bottom blend is fully opaque
	  */
	void setBottomBlend(float zMin, float zMax);

	/** Set the top Z-blend region.
	  * Decal alpha fades from 1 at zMin to 0 at zMax (top edge).
	  * \param zMin Altitude below which top blend is fully opaque
	  * \param zMax Altitude above which the decal is fully transparent
	  */
	void setTopBlend(float zMin, float zMax);

	/// Get bottom blend zMin.
	float getBottomBlendZMin() const { return _BottomBlendZMin; }
	/// Get bottom blend zMax.
	float getBottomBlendZMax() const { return _BottomBlendZMax; }
	/// Get top blend zMin.
	float getTopBlendZMin() const { return _TopBlendZMin; }
	/// Get top blend zMax.
	float getTopBlendZMax() const { return _TopBlendZMax; }

	/** Get the world-to-UV matrix rows for the vertex program.
	  * Row 0 maps world X to U, Row 1 maps world Y to V.
	  * Set up during generateUVs() for the VP path.
	  */
	const CMatrix &getWorldToUVMatrix() const { return _WorldToUVMatrix; }

	/** Get the Matrix that transforms local coordinates to UV coordinates.
	  * Maps (x,y)=(0,0) to (u,v)=(0,1) and (x,y)=(0,1) to (u,v)=(0,0)
	  * in local decal space (unit cube). See PDF §4.5.3.
	  */
	static CMatrix getReverseUVMatrix()
	{
		CMatrix m;
		m.setRot(CVector::I, -CVector::J, CVector::K);
		m.setPos(CVector::J);
		return m;
	}

private:
	/// Creator function for scene model registration.
	static CTransform *creator() { return new CDecal(); }

	/** Compute decal geometry: face selection, clipping, and UV generation.
	  * Called by getVertices() when the decal needs recomputation.
	  */
	void computeDecal(const bool useVertexProgram);

	/** Generate UV coordinates for the collected vertices.
	  * Uses the inverse world matrix to project back to unit-cube local space,
	  * then maps to UV sub-region defined by _UV1/_UV2. See PDF §4.5.3.
	  * Camera position is subtracted for numerical stability (§4.6.1).
	  */
	void generateUVs();

	/** Compute per-vertex colors for the CPU fallback path.
	  * Applies diffuse color, distance attenuation, and bottom/top Z blending.
	  * \param distScale Linear distance attenuation scale factor
	  * \param distBias Linear distance attenuation bias
	  */
	void computeColors(float distScale, float distBias);

private:
	CMaterial					_Mat;
	uint32						_MaterialId;

	bool						_Touched;
	bool						_FirstFrame;	///< True until the first frame has been traversed (matrices invalid on frame 0, see PDF §4.6.4)
	uint32						_StableFrameCount;

	CVector						_LastCamPos;
	CVector						_ClipCorners[4];

	std::vector<CVector>		_Vertices;
	std::vector<CUV>			_UVs;
	std::vector<NLMISC::CRGBA>	_Colors;
	bool						_IsStatic;
	uint8						_Priority;
	bool						_ClipDownFacing;

	CUV							_UV1;
	CUV							_UV2;
	CDecalContext				_DecalContext;

	NLMISC::CRGBA				_Diffuse;
	NLMISC::CRGBA				_Emissive;
	float						_BottomBlendZMin;
	float						_BottomBlendZMax;
	float						_TopBlendZMin;
	float						_TopBlendZMax;

	CMatrix						_WorldToUVMatrix;
	CMatrix						_TextureMatrix;
	bool						_CustomUVMatrixEnabled;
	CMatrix						_CustomUVMatrix;

	/// Static mask texture for DecalClipMask mode (generated once, shared)
	static NLMISC::CSmartPtr<ITexture>	_MaskTexture;
	static ITexture *getMaskTexture();
};

}//NL3D
#endif
