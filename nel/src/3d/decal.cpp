/** \file decal.cpp
 * Projected texture decal system for NeL 3D.
 *
 * Based on the design from the intern report by Christopher Tarento (2007).
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

#include "std3d.h"
#include "nel/3d/decal.h"

#include "nel/3d/texture_file.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/landscape_model.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***************************************************************************
CDecalContext::CDecalContext() : ClipMode(DecalClipGeometry), DestTris(NULL), ClipDownFacing(false) {}

// ***************************************************************************
// Static mask texture (shared across all decals)
NLMISC::CSmartPtr<ITexture> CDecal::_MaskTexture;


// ***************************************************************************
CDecal::CDecal() :
_MaterialId(0),
_Touched(true),
_FirstFrame(true),
_StableFrameCount(0),
_IsStatic(false),
_Priority(0),
_ClipDownFacing(false),
_UV1(CUV(0, 0)),
_UV2(CUV(1, 1)),
_Diffuse(CRGBA::White),
_Emissive(CRGBA::Black),
_BottomBlendZMin(-1e10f),
_BottomBlendZMax(-1e10f),
_TopBlendZMin(1e10f),
_TopBlendZMax(1e10f),
_CustomUVMatrixEnabled(false)
{
setOpacity(true);
setTransparency(false);
setIsRenderable(true);

// Material setup: unlit, alpha-blended, double-sided, no z-write, with z-bias
_Mat.initUnlit();
_Mat.setBlend(true);
_Mat.setSrcBlend(CMaterial::srcalpha);
_Mat.setDstBlend(CMaterial::invsrcalpha);
_Mat.setZWrite(false);
_Mat.setDoubleSided(true);
_Mat.setZBias(-0.06f);
_Mat.setAlphaTest(true);
_Mat.setAlphaTestThreshold(1.f / 255.f);

// Stage 0: diffuse color applied to texture
// RGB = Texture * Diffuse, Alpha = Diffuse * Texture
_Mat.texEnvOpRGB(0, CMaterial::Modulate);
_Mat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
_Mat.texEnvArg1RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
_Mat.texEnvOpAlpha(0, CMaterial::Modulate);
_Mat.texEnvArg0Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
_Mat.texEnvArg1Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

// Stage 1: add emissive color
// RGB = Previous + Constant, Alpha = Previous * Constant
_Mat.texEnvOpRGB(1, CMaterial::Add);
_Mat.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
_Mat.texEnvArg1RGB(1, CMaterial::Constant, CMaterial::SrcColor);
_Mat.texEnvOpAlpha(1, CMaterial::Modulate);
_Mat.texEnvArg0Alpha(1, CMaterial::Previous, CMaterial::SrcAlpha);
_Mat.texEnvArg1Alpha(1, CMaterial::Constant, CMaterial::SrcAlpha);

setEmissive(CRGBA::Black);

// Default clipping mode: geometry clipping (best fillrate, see PDF 4.5.2)
_DecalContext.ClipMode = DecalClipGeometry;
}


// ***************************************************************************
void CDecal::initModel()
{
_LastCamPos = getOwnerScene()->getCam()->getMatrix().getPos();
}


// ***************************************************************************
CDecal::~CDecal()
{
}


// ***************************************************************************
void CDecal::registerBasic()
{
CScene::registerModel(DecalId, TransformId, CDecal::creator);
}


// ***************************************************************************
void CDecal::setTexture(const std::string &filename)
{
CTextureFile *tex = new CTextureFile(filename);
tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapLinear);
tex->setWrapS(ITexture::Clamp);
tex->setWrapT(ITexture::Clamp);
_Mat.setTexture(0, tex);
// Stage 1 also needs the texture for the emissive add operation to work correctly
_Mat.setTexture(1, tex);
}


// ***************************************************************************
void CDecal::setEmissive(NLMISC::CRGBA emissive)
{
_Emissive = emissive;
// Set the stage 1 constant color to the emissive value
_Mat.texConstantColor(1, CRGBA(emissive.R, emissive.G, emissive.B, 255));
}


// ***************************************************************************
void CDecal::setBottomBlend(float zMin, float zMax)
{
if (zMin > zMax) std::swap(zMin, zMax);
_BottomBlendZMin = zMin;
_BottomBlendZMax = zMax;
}


// ***************************************************************************
void CDecal::setTopBlend(float zMin, float zMax)
{
if (zMin > zMax) std::swap(zMin, zMax);
_TopBlendZMin = zMin;
_TopBlendZMax = zMax;
}


// ***************************************************************************
// Clip method: bounding sphere vs frustum test (see PDF .6.1)
bool CDecal::clip()
{
CScene *scene = getOwnerScene();
CClipTrav &clipTrav = scene->getClipTrav();

// Compute bounding sphere from decal's world matrix
// The decal is a unit cube [0,1]^3, so center is at (0.5, 0.5, 0.5) in local space
CVector localCenter(0.5f, 0.5f, 0.5f);
CVector worldCenter = getWorldMatrix() * localCenter;

// Radius: half-diagonal of the unit cube, scaled by the largest scale factor
// For a unit cube, half-diagonal = sqrt(3)/2 ~ 0.866
float scaleI = (getWorldMatrix().getI()).norm();
float scaleJ = (getWorldMatrix().getJ()).norm();
float scaleK = (getWorldMatrix().getK()).norm();
float maxScale = std::max(scaleI, std::max(scaleJ, scaleK));
float radius = 0.866f * maxScale;

// Test against camera frustum planes
const std::vector<CPlane> &pyramid = clipTrav.WorldPyramid;
for (uint i = 0; i < pyramid.size(); ++i)
{
float d = pyramid[i] * worldCenter;
if (d > radius)
{
return false;
}
}

return true;
}


// ***************************************************************************
// Render traversal: just register with the decal manager for batched rendering
void CDecal::traverseRender()
{
getOwnerScene()->getRenderTrav().getDecalManager().addDecal(this, _MaterialId);
}


// ***************************************************************************
std::vector<CVector> &CDecal::getVertices(const bool useVertexProgram)
{
const NLMISC::CVector &camPos = getOwnerScene()->getCam()->getMatrix().getPos();

// First-frame skip: matrices are incorrect on the first traversal (PDF §4.6.4).
// Return empty vertices and defer computation to the next frame.
if (_FirstFrame)
{
	_FirstFrame = false;
	_LastCamPos = camPos;
	_Touched = true;
	_Vertices.clear();
	_UVs.clear();
	_Colors.clear();
	return _Vertices;
}

if (_IsStatic)
{
// Static decal: only recompute on first touch.
// After that, use frame-count heuristic based on camera movement (4.5.4).
if (_Touched)
{
_LastCamPos = camPos;
computeDecal(useVertexProgram);
_Touched = false;
_StableFrameCount = 0;
}
}
else
{
// Dynamic decal: recompute when camera moves significantly (4.6.1).
// Use visibility distance as threshold for recalculation.
if ((camPos - _LastCamPos).norm() >= 4.f)
{
_Touched = true;
}

if (_Touched)
{
_LastCamPos = camPos;
computeDecal(useVertexProgram);
_Touched = false;
_StableFrameCount = 0;
}
else
{
// Increment stable frame count. If a dynamic decal has been stable
// for many frames, it effectively becomes static (4.5.4 heuristic).
_StableFrameCount++;
}
}

return _Vertices;
}


// ***************************************************************************
// Face selection and clipping (see PDF .6.2 and 4.5.1/4.5.2)
void CDecal::computeDecal(const bool useVertexProgram)
{
CScene *sc = getOwnerScene();

CVisualCollisionManager *vcm = sc->getVisualCollisionManagerForShadow();
if (!vcm)
{
nlwarning("CDecal::computeDecal: VisualCollisionManager not available");
return;
}

CDecalContext &context = _DecalContext;

// Build clip planes from the unit cube corners transformed to world space
float decalSize = 1.0f;
_ClipCorners[0] = getWorldMatrix() * (CVector(0.f, 1.f, 0.f) * decalSize);
_ClipCorners[1] = getWorldMatrix() * (CVector(1.f, 1.f, 0.f) * decalSize);
_ClipCorners[2] = getWorldMatrix() * (CVector(1.f, 0.f, 0.f) * decalSize);
_ClipCorners[3] = getWorldMatrix() * (CVector(0.f, 0.f, 0.f) * decalSize);

// Build 4 side clip planes from the corners (4.5.1)
context.WorldClipPlanes.resize(4);
context.WorldBBox.setMinMax(
getWorldMatrix() * (CVector(0.f, 0.f, 0.f) * decalSize),
getWorldMatrix() * (CVector(1.f, 1.f, 1.f) * decalSize));

for (uint i = 0; i < 4; ++i)
{
context.WorldClipPlanes[i].make(
_ClipCorners[i],
_ClipCorners[(i + 1) & 3],
_ClipCorners[i] + (_ClipCorners[(i + 1) & 3] - _ClipCorners[i]).norm() * CVector::K);
context.WorldClipPlanes[i].invert();
}

context.WorldMatrix = getWorldMatrix();
context.ClipDownFacing = _ClipDownFacing;

// Clear and collect triangles via visual collision (objects/meshes)
_Vertices.clear();
context.DestTris = &_Vertices;
vcm->receiveDecal(context);

// Also collect triangles from landscape's shadow poly receiver (terrain)
CRenderTrav &renderTrav = sc->getRenderTrav();
const std::vector<CLandscapeModel*> &landscapes = renderTrav.getLandscapeRenderList();
for (uint i = 0; i < landscapes.size(); ++i)
{
CLandscapeModel *lm = landscapes[i];
if (!lm) continue;
CVector vertDelta = -lm->Landscape.getPZBModelPosition();
lm->Landscape.getShadowPolyReceiver().receiveDecal(context, vertDelta);
}

// Generate UV coordinates from collected vertices
generateUVs();

// Compute per-vertex colors for CPU fallback path
if (!useVertexProgram)
{
CDecalManager &mgr = sc->getRenderTrav().getDecalManager();
(void)mgr; // distScale/distBias set externally
computeColors(0.f, 1.f);
}
}


// ***************************************************************************
// UV coordinate generation (see PDF 4.5.3)
// Uses inverse world matrix to project world-space vertices back to unit-cube local space.
// Supports custom UV matrix and texture matrix overrides.
// Also builds the worldToUV matrix for the VP path.
void CDecal::generateUVs()
{
_UVs.resize(_Vertices.size());

if (_Vertices.empty())
return;

// Compute worldToUV matrix: maps world position to [0,1] UV in local decal space
CMatrix invWorld = getWorldMatrix().inverted();

if (_CustomUVMatrixEnabled)
{
	// Custom UV matrix overrides the entire UV generation pipeline
	_WorldToUVMatrix = _CustomUVMatrix;
}
else
{
	// Default: texture matrix × reverse UV matrix × inverse world
	CMatrix reverseUV = getReverseUVMatrix();
	_WorldToUVMatrix = _TextureMatrix * reverseUV * invWorld;
}

// Use the worldToUV matrix to generate UVs, matching the VP path.
// Row 0 of the matrix gives U, Row 1 gives V.
const float *m = _WorldToUVMatrix.get();
for (uint i = 0; i < _Vertices.size(); ++i)
{
	const CVector &vtx = _Vertices[i];
	// DP4 equivalent: dot product of matrix row with (vx, vy, vz, 1)
	float u = m[0] * vtx.x + m[4] * vtx.y + m[8] * vtx.z + m[12];
	float v = m[1] * vtx.x + m[5] * vtx.y + m[9] * vtx.z + m[13];

	// Apply UV sub-region mapping (for texture atlases)
	_UVs[i].U = _UV1.U + u * (_UV2.U - _UV1.U);
	_UVs[i].V = _UV1.V + v * (_UV2.V - _UV1.V);
}
}


// ***************************************************************************
// Compute per-vertex colors for CPU fallback path
// Applies diffuse color, distance attenuation, and bottom/top Z blending.
void CDecal::computeColors(float distScale, float distBias)
{
_Colors.resize(_Vertices.size());

if (_Vertices.empty())
return;

const CVector camPos = getOwnerScene()->getCam()->getMatrix().getPos();

float bottomBlendScale = 1.f / NLMISC::favoid0(_BottomBlendZMax - _BottomBlendZMin);
float topBlendScale = 1.f / NLMISC::favoid0(_TopBlendZMin - _TopBlendZMax);

for (uint i = 0; i < _Vertices.size(); ++i)
{
const CVector &v = _Vertices[i];

// Distance attenuation
float dist = (camPos - v).norm();
float intensity = dist * distScale + distBias;
clamp(intensity, 0.f, 1.f);

// Bottom blend
float bottomBlend = (v.z - _BottomBlendZMin) * bottomBlendScale;
clamp(bottomBlend, 0.f, 1.f);
intensity *= bottomBlend;

// Top blend
float topBlend = (v.z - _TopBlendZMax) * topBlendScale;
clamp(topBlend, 0.f, 1.f);
intensity *= topBlend;

// Apply to diffuse color
_Colors[i].R = _Diffuse.R;
_Colors[i].G = _Diffuse.G;
_Colors[i].B = _Diffuse.B;
_Colors[i].A = (uint8)((float)_Diffuse.A * intensity);
}
}


// ***************************************************************************
void CDecal::setUVCoord(const CUV uv1, const CUV uv2)
{
_UV1 = uv1;
_UV2 = uv2;
_Touched = true;

// Mipmap limiting for texture atlases (PDF §4.6.2):
// When using a sub-region, mipmaps can bleed into neighboring portions.
// Disable mipmaps on the texture to prevent this.
ITexture *tex = _Mat.getTexture(0);
if (tex)
{
	bool isSubRegion = (fabsf(uv1.U) > 1e-6f || fabsf(uv1.V) > 1e-6f || fabsf(uv2.U - 1.f) > 1e-6f || fabsf(uv2.V - 1.f) > 1e-6f);
	if (isSubRegion)
	{
		tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	}
	else
	{
		tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapLinear);
	}
}
}


// ***************************************************************************
void CDecal::setCustomUVMatrix(bool on, const CMatrix &matrix)
{
_CustomUVMatrixEnabled = on;
_CustomUVMatrix = matrix;
_Touched = true;
}


// ***************************************************************************
void CDecal::setTextureMatrix(const CMatrix &matrix)
{
_TextureMatrix = matrix;
_Touched = true;
}


// ***************************************************************************
void CDecal::setWorldMatrixForArrow(const NLMISC::CVector2f &start, const NLMISC::CVector2f &end, float halfWidth)
{
CMatrix matrix;
CVector I = CVector(end.x, end.y, 0.f) - CVector(start.x, start.y, 0.f);
CVector J = 2.f * halfWidth * CVector::K ^ I.normed();
matrix.setRot(I, J, CVector::K);
matrix.setPos(CVector(start.x, start.y, 0.f) - 0.5f * J);
setMatrix(matrix);
_Touched = true;
}


// ***************************************************************************
void CDecal::setWorldMatrixForSpot(const NLMISC::CVector2f &pos, float radius, float angleInRadians)
{
CMatrix matrix;
matrix.rotateZ(angleInRadians);
matrix.setScale(2.f * radius);
matrix.setPos(CVector(pos.x - radius, pos.y - radius, 0.f));
setMatrix(matrix);
_Touched = true;
}


// ***************************************************************************
ITexture *CDecal::getMaskTexture()
{
if (_MaskTexture != NULL)
	return _MaskTexture;

// Generate a 4×4 RGBA texture: opaque white in the 2×2 center, transparent black at edges
const uint32 maskSize = 4;
const uint32 maskCenterMin = 1; // inclusive: center region starts at pixel 1
const uint32 maskCenterMax = 2; // inclusive: center region ends at pixel 2
uint32 dataSize = maskSize * maskSize * 4; // RGBA
uint8 *data = new uint8[dataSize];
memset(data, 0, dataSize);
for (uint y = 0; y < maskSize; ++y)
{
	for (uint x = 0; x < maskSize; ++x)
	{
		uint idx = (y * maskSize + x) * 4;
		// Center pixels are opaque white
		if (x >= maskCenterMin && x <= maskCenterMax && y >= maskCenterMin && y <= maskCenterMax)
		{
			data[idx + 0] = 255; // R
			data[idx + 1] = 255; // G
			data[idx + 2] = 255; // B
			data[idx + 3] = 255; // A
		}
		// Edge pixels are transparent (already 0)
	}
}

CTextureMem *tex = new CTextureMem(data, dataSize, true, false, maskSize, maskSize, CBitmap::RGBA);
tex->setWrapS(ITexture::Clamp);
tex->setWrapT(ITexture::Clamp);
tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
tex->setUploadFormat(ITexture::RGBA8888);
_MaskTexture = tex;
return _MaskTexture;
}


// ***************************************************************************
bool CDecal::contains(const NLMISC::CVector2f &pos) const
{
	CMatrix invMat = getWorldMatrix();
	invMat.invert();
	CVector posIn = invMat * CVector(pos.x, pos.y, 0.f);
	return posIn.x >= 0.f && posIn.x <= 1.f && posIn.y >= 0.f && posIn.y <= 1.f;
}
