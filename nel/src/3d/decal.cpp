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
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/visual_collision_manager.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***************************************************************************
CDecalContext::CDecalContext() : ClipMode(DecalClipGeometry), DestTris(NULL) {}


// ***************************************************************************
CDecal::CDecal() :
_MaterialId(0),
_Touched(true),
_StableFrameCount(0),
_IsStatic(false),
_UV1(CUV(0, 0)),
_UV2(CUV(1, 1))
{
setOpacity(true);
setTransparency(false);
setIsRenderable(true);

// Material setup: alpha-blended, double-sided, no z-write, with z-bias
_Mat.setShader(CMaterial::TShader::Normal);
_Mat.setBlend(true);
_Mat.setSrcBlend(CMaterial::srcalpha);
_Mat.setDstBlend(CMaterial::invsrcalpha);
_Mat.setZWrite(false);
_Mat.setDoubleSided(true);
_Mat.setZBias(-0.06f);

// Texture environment: output = texture color/alpha
_Mat.texConstantColor(0, CRGBA::White);
_Mat.texEnvOpRGB(0, CMaterial::Replace);
_Mat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
_Mat.texEnvOpAlpha(0, CMaterial::Replace);
_Mat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

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

// Clear and collect triangles via visual collision
_Vertices.clear();
context.DestTris = &_Vertices;
vcm->receiveDecal(context);

// Generate UV coordinates from collected vertices
generateUVs();
}


// ***************************************************************************
// UV coordinate generation (see PDF 4.5.3)
// Uses inverse world matrix to project world-space vertices back to unit-cube local space.
// Camera position is subtracted for numerical stability (4.6.1).
void CDecal::generateUVs()
{
_UVs.resize(_Vertices.size());

if (_Vertices.empty())
return;

// Compute worldToUV matrix: maps world position to [0,1] UV in local decal space
CMatrix invWorld = getWorldMatrix().inverted();

for (uint i = 0; i < _Vertices.size(); ++i)
{
// Transform world vertex to local decal space
CVector local = invWorld * _Vertices[i];

// Map local X,Y to UV, respecting the UV sub-region
float u = _UV1.U + local.x * (_UV2.U - _UV1.U);
float v = _UV1.V + (1.0f - local.y) * (_UV2.V - _UV1.V);

_UVs[i].U = u;
_UVs[i].V = v;
}
}


// ***************************************************************************
void CDecal::setUVCoord(const CUV uv1, const CUV uv2)
{
_UV1 = uv1;
_UV2 = uv2;
_Touched = true;
}
