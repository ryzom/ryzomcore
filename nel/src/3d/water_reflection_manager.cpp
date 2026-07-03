/**
 * \file water_reflection_manager.cpp
 * \brief CWaterReflectionManager
 * \date 2026-07-03
 * \author Jan Boon (Kaetemi)
 * CWaterReflectionManager
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std3d.h"
#include "nel/3d/water_reflection_manager.h"

#include "nel/misc/plane.h"

#include "nel/3d/camera.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene.h"
#include "nel/3d/scissor.h"
#include "nel/3d/texture_offscreen.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/viewport.h"

#include <algorithm>

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

// Keep geometry slightly below the surface in the reflection so that
// perturbed UVs near the waterline don't sample clip-void (halo artifacts)
static const float WATER_REFLECTION_CLIP_BIAS = 0.25f;
// Screen-space margin around the water AABB for UV wobble, fraction of screen
static const float WATER_REFLECTION_MARGIN = 0.02f;
// RT dimension snap in pixels, avoids active-region churn
static const uint WATER_REFLECTION_SNAP = 32;
// Minimum height of the camera above the plane for a reflection
static const float WATER_REFLECTION_MIN_CAM_HEIGHT = 0.05f;
// Hysteresis: a challenger plane must beat an incumbent by this factor
static const float WATER_REFLECTION_HYSTERESIS = 1.25f;

// Round down to a power of two (bounds RT memory on large screens; deliberate)
static uint pow2Down(uint v)
{
	uint p = 1;
	while (p * 2 <= v) p *= 2;
	return p;
}

static uint snapUp(uint v, uint snap)
{
	return ((v + snap - 1) / snap) * snap;
}

// ***************************************************************************
CWaterReflectionManager::CWaterReflectionManager()
	: _Scene(NULL)
	, _MaxReflections(-1)
	, _ForceReflections(false)
	, _HalfRes(true)
	, _Pow2(true)
	, _FixedSize(true)
	, _InReflectionRender(false)
	, _CollectionArmed(false)
	, _HadReflections(false)
	, _ReflCamera(NULL)
	, _SaveCam(NULL)
{
}

// ***************************************************************************
CWaterReflectionManager::~CWaterReflectionManager()
{
	release();
}

// ***************************************************************************
void CWaterReflectionManager::release()
{
	nlassert(!_InReflectionRender);
	_Active.clear();
	_Collected.clear();
	_PrevAdmitted.clear();
	_Slots.clear();
	_Passes.clear();
	_SaveRenderTarget = NULL;
	_SaveCam = NULL;
	_CollectionArmed = false;
	_HadReflections = false;
	if (_ReflCamera)
	{
		if (_Scene)
			_Scene->deleteModel(_ReflCamera);
		_ReflCamera = NULL;
	}
}

// ***************************************************************************
CCamera *CWaterReflectionManager::getReflCamera()
{
	if (!_ReflCamera)
	{
		_ReflCamera = static_cast<CCamera *>(_Scene->createModel(CameraId));
		_ReflCamera->setTransformMode(ITransformable::DirectMatrix);
	}
	return _ReflCamera;
}

// ***************************************************************************
void CWaterReflectionManager::reportVisibleSurface(float planeZ, float screenArea,
	const CVector2f &screenMin, const CVector2f &screenMax, bool shapeAllows)
{
	if (!wantsSurfaceReports()) return; // stats only from the main render, when the app renders reflections
	if (screenArea <= 0.f) return;

	sint32 key = planeKey(planeZ);
	std::map<sint32, CPlaneStats>::iterator it = _Collected.find(key);
	if (it == _Collected.end())
	{
		CPlaneStats st;
		st.PlaneZ = planeZ;
		st.Area = screenArea;
		st.Min = screenMin;
		st.Max = screenMax;
		st.Allowed = shapeAllows;
		_Collected[key] = st;
	}
	else
	{
		CPlaneStats &st = it->second;
		st.Area += screenArea;
		st.Min.x = std::min(st.Min.x, screenMin.x);
		st.Min.y = std::min(st.Min.y, screenMin.y);
		st.Max.x = std::max(st.Max.x, screenMax.x);
		st.Max.y = std::max(st.Max.y, screenMax.y);
		st.Allowed = st.Allowed || shapeAllows;
	}
}

// ***************************************************************************
const CWaterReflectionManager::CActiveReflection *CWaterReflectionManager::getActiveReflection(float planeZ) const
{
	std::map<sint32, CActiveReflection>::const_iterator it = _Active.find(planeKey(planeZ));
	if (it == _Active.end()) return NULL;
	return &it->second;
}

// ***************************************************************************
const CWaterReflectionManager::CActiveReflection *CWaterReflectionManager::getActiveReflectionByIndex(uint index) const
{
	if (index >= _Active.size()) return NULL;
	std::map<sint32, CActiveReflection>::const_iterator it = _Active.begin();
	std::advance(it, index);
	return &it->second;
}

// ***************************************************************************
uint CWaterReflectionManager::beginPasses()
{
	nlassert(_Scene);
	nlassert(!_InReflectionRender); // missing endPasses()?

	// Water models only report visibility stats once this has been called;
	// applications that never render reflections must not accumulate stats
	_CollectionArmed = true;

	// Take the stats from the last main render
	std::map<sint32, CPlaneStats> collected;
	collected.swap(_Collected);

	// Multi-pass render loops (stereo) may reach the reflections pass more
	// than once per frame; later calls find no stats and must keep the
	// reflections rendered by the first call instead of discarding them
	if (collected.empty() && !_Active.empty() && _MaxReflections != 0)
		return 0;

	_Active.clear();
	_Passes.clear();

	if (_MaxReflections == 0 || collected.empty())
	{
		_PrevAdmitted.clear();
		return 0;
	}

	CCamera *mainCam = _Scene->getCam();
	if (!mainCam)
	{
		_PrevAdmitted.clear();
		return 0;
	}
	float camZ = mainCam->getWorldMatrix().getPos().z;

	// Build candidate list: allowed planes with the camera above them.
	// Hysteresis: planes admitted last frame get a sticky area bonus so a
	// challenger must clearly out-rank them before stealing a slot.
	std::vector<std::pair<float, sint32> > candidates; // (-effective area, key) for ascending sort
	for (std::map<sint32, CPlaneStats>::iterator it = collected.begin(); it != collected.end(); ++it)
	{
		const CPlaneStats &st = it->second;
		if (!(st.Allowed || _ForceReflections)) continue;
		if (camZ <= st.PlaneZ + WATER_REFLECTION_MIN_CAM_HEIGHT) continue; // underwater / grazing: envmap fallback
		float effArea = st.Area;
		if (std::find(_PrevAdmitted.begin(), _PrevAdmitted.end(), it->first) != _PrevAdmitted.end())
			effArea *= WATER_REFLECTION_HYSTERESIS;
		candidates.push_back(std::make_pair(-effArea, it->first));
	}
	std::sort(candidates.begin(), candidates.end());

	uint numAdmitted = (uint)candidates.size();
	if (_MaxReflections >= 0)
		numAdmitted = std::min(numAdmitted, (uint)_MaxReflections);

	_PrevAdmitted.clear(); // repopulated by endPass
	if (!numAdmitted)
		return 0;

	if (_Slots.size() < numAdmitted)
		_Slots.resize(numAdmitted);

	// Prepare the per-pass reflected cameras, sub-frusta and target sizes
	_Passes.resize(numAdmitted);
	for (uint i = 0; i < numAdmitted; ++i)
	{
		_Passes[i].Key = candidates[i].second;
		preparePass(mainCam, collected[_Passes[i].Key], _Passes[i]);
	}

	// Save scene and driver state. The render target must be restored to
	// the previously bound one, not to NULL: the caller may already be
	// rendering into an effects render target (bloom/FXAA pipeline).
	IDriver *drv = _Scene->getDriver();
	_SaveCam = mainCam;
	_SaveSceneViewport = _Scene->getViewport();
	_SaveRenderTarget = drv->getRenderTarget();
	drv->getViewport(_SaveDrvViewport);

	_InReflectionRender = true;
	_HadReflections = true;

	// Water must not reflect itself; flares use per-frame occlusion queries
	_Scene->enableElementRender(UScene::FilterWater, false);
	_Scene->enableElementRender(UScene::FilterFlare, false);

	return numAdmitted;
}

// ***************************************************************************
void CWaterReflectionManager::preparePass(CCamera *mainCam, const CPlaneStats &stats, CPassData &pass)
{
	IDriver *drv = _Scene->getDriver();

	const float planeZ = stats.PlaneZ;

	// --- Reflected camera: mirror the main camera across the plane ---
	// Basis vectors are mirrored in z; I is negated afterwards to restore
	// right-handedness (the horizontal flip is absorbed by projecting the
	// UVs through this same camera). No world-up derivation, so steep
	// look-down camera angles stay robust.
	// NB: the camera world matrix is the last committed one (previous
	// frame's traversal), so the reflection image lags the viewpoint by one
	// frame. The published ReflViewMatrix is the same matrix, so the UV
	// projection stays exactly consistent with the rendered content.
	const CMatrix &camWorld = mainCam->getWorldMatrix();
	CVector I = camWorld.getI(), J = camWorld.getJ(), K = camWorld.getK(), P = camWorld.getPos();
	CVector Im(-I.x, -I.y, I.z);
	CVector Jm(J.x, J.y, -J.z);
	CVector Km(K.x, K.y, -K.z);
	CVector Pm(P.x, P.y, 2.f * planeZ - P.z);
	pass.ReflCamWorld.identity();
	pass.ReflCamWorld.setRot(Im, Jm, Km);
	pass.ReflCamWorld.setPos(Pm);

	// --- Screen AABB of the water in the reflected view ---
	// For points on the mirror plane, the reflected-camera projection is
	// the main-camera projection mirrored in x.
	float minX = 1.f - stats.Max.x;
	float maxX = 1.f - stats.Min.x;
	float minY = stats.Min.y;
	float maxY = stats.Max.y;
	clamp(minX, 0.f, 1.f); clamp(maxX, 0.f, 1.f);
	clamp(minY, 0.f, 1.f); clamp(maxY, 0.f, 1.f);
	minX -= WATER_REFLECTION_MARGIN; maxX += WATER_REFLECTION_MARGIN;
	minY -= WATER_REFLECTION_MARGIN; maxY += WATER_REFLECTION_MARGIN;

	// --- RT sizing (fixed/dynamic mode; see planar_reflection sample) ---
	uint32 winW, winH;
	drv->getWindowSize(winW, winH);
	if (winW == 0 || winH == 0) { winW = 128; winH = 128; }

	// Active region: snapped AABB dimensions, padded symmetrically
	uint rawW = (uint)std::max(1.f, ceilf((maxX - minX) * (float)winW));
	uint rawH = (uint)std::max(1.f, ceilf((maxY - minY) * (float)winH));
	uint snappedW = snapUp(rawW, WATER_REFLECTION_SNAP);
	uint snappedH = snapUp(rawH, WATER_REFLECTION_SNAP);
	float padX = (float)(snappedW - rawW) / (2.f * (float)winW);
	float padY = (float)(snappedH - rawH) / (2.f * (float)winH);
	minX -= padX; maxX += padX;
	minY -= padY; maxY += padY;
	uint activeW = snappedW;
	uint activeH = snappedH;
	if (_HalfRes) { activeW = std::max(1u, activeW / 2); activeH = std::max(1u, activeH / 2); }
	if (_Pow2) { activeW = pow2Down(activeW); activeH = pow2Down(activeH); }

	// Allocation: fixed mode allocates from the window size + margin (stable
	// across frames); dynamic mode allocates at the active size
	uint allocW, allocH;
	if (_FixedSize)
	{
		uint marginPx = (uint)ceilf(WATER_REFLECTION_MARGIN * (float)std::max(winW, winH));
		allocW = snapUp(winW + 2 * marginPx, WATER_REFLECTION_SNAP);
		allocH = snapUp(winH + 2 * marginPx, WATER_REFLECTION_SNAP);
		if (_HalfRes) { allocW = std::max(1u, allocW / 2); allocH = std::max(1u, allocH / 2); }
		if (_Pow2) { allocW = pow2Down(allocW); allocH = pow2Down(allocH); }
		activeW = std::min(activeW, allocW);
		activeH = std::min(activeH, allocH);
	}
	else
	{
		allocW = activeW;
		allocH = activeH;
	}

	// --- Off-center sub-frustum over the reflected AABB ---
	float fl, fr, fb, ft, fnear, ffar;
	mainCam->getFrustum(fl, fr, fb, ft, fnear, ffar);
	CFrustum subFrustum;
	float fw = fr - fl;
	float fh = ft - fb;
	subFrustum.init(
		fl + minX * fw, fl + maxX * fw,
		fb + minY * fh, fb + maxY * fh,
		fnear, ffar, true);

	pass.AllocW = allocW;
	pass.AllocH = allocH;
	pass.ActiveW = activeW;
	pass.ActiveH = activeH;
	pass.Refl.Texture = NULL; // set at beginPass, once the slot is ensured
	pass.Refl.ReflViewMatrix = pass.ReflCamWorld;
	pass.Refl.ReflViewMatrix.invert();
	pass.Refl.ReflFrustum = subFrustum;
	pass.Refl.UVScale.U = (float)activeW / (float)allocW;
	pass.Refl.UVScale.V = (float)activeH / (float)allocH;
	pass.Refl.PlaneZ = planeZ;
}

// ***************************************************************************
void CWaterReflectionManager::beginPass(uint pass, CActiveReflection &out)
{
	nlassert(_InReflectionRender);
	nlassert(pass < _Passes.size());
	CPassData &pd = _Passes[pass];
	IDriver *drv = _Scene->getDriver();

	// --- Render target ---
	CSlot &slot = _Slots[pass];
	if (!slot.Texture || slot.AllocW != pd.AllocW || slot.AllocH != pd.AllocH)
	{
		CTextureOffscreen *tex = new CTextureOffscreen();
		tex->setNeedsDepthStencil(true);
		tex->setRenderTarget(true);
		tex->setReleasable(false);
		tex->resize(pd.AllocW, pd.AllocH);
		tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
		tex->setWrapS(ITexture::Clamp);
		tex->setWrapT(ITexture::Clamp);
		drv->setupTexture(*tex);
		slot.Texture = tex;
		slot.AllocW = pd.AllocW;
		slot.AllocH = pd.AllocH;
	}
	pd.Refl.Texture = slot.Texture;

	// --- Bind the render target ---
	drv->setRenderTarget(slot.Texture, 0, 0, pd.AllocW, pd.AllocH);

	float vpW = pd.Refl.UVScale.U;
	float vpH = pd.Refl.UVScale.V;

	// Clear the full allocation to the fog color so out-of-frustum samples
	// and the wobble margin around the active region blend in
	CViewport fullVP;
	fullVP.initFullScreen();
	drv->setupViewport(fullVP);
	CScissor fullScissor;
	fullScissor.initFullScreen();
	drv->setupScissor(fullScissor);
	CRGBA clearColor = drv->fogEnabled() ? drv->getFogColor() : CRGBA(0, 0, 0, 255);
	drv->clear2D(clearColor);
	drv->clearZBuffer();

	// Restrict rendering to the active sub-region
	CViewport activeVP;
	activeVP.init(0.f, 0.f, vpW, vpH);
	drv->setupViewport(activeVP);
	CScissor activeScissor;
	activeScissor.init(0.f, 0.f, vpW, vpH);
	drv->setupScissor(activeScissor);

	// Clip plane, biased slightly below the surface. The driver transforms
	// it to eye space using the current view matrix, so set up the same
	// view state the caller's scene render will use (including the PZB
	// camera pos).
	const CFrustum &f = pd.Refl.ReflFrustum;
	drv->setFrustum(f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, true);
	drv->setupViewMatrixEx(pd.Refl.ReflViewMatrix, pd.ReflCamWorld.getPos());
	drv->setClipPlane(0, CPlane(0.f, 0.f, 1.f, -(pd.Refl.PlaneZ - WATER_REFLECTION_CLIP_BIAS)));
	drv->enableClipPlane(0, true);

	// --- Scene camera for the caller's scene render ---
	CCamera *reflCam = getReflCamera();
	reflCam->setMatrix(pd.ReflCamWorld);
	reflCam->setFrustum(f);
	_Scene->setCam(reflCam);
	CViewport sceneVP;
	sceneVP.init(0.f, 0.f, vpW, vpH);
	_Scene->setViewport(sceneVP);

	out = pd.Refl;
}

// ***************************************************************************
void CWaterReflectionManager::endPass(uint pass)
{
	nlassert(_InReflectionRender);
	nlassert(pass < _Passes.size());
	IDriver *drv = _Scene->getDriver();
	drv->enableClipPlane(0, false);

	// Publish for the coming main render
	_Active[_Passes[pass].Key] = _Passes[pass].Refl;
	_PrevAdmitted.push_back(_Passes[pass].Key);
}

// ***************************************************************************
void CWaterReflectionManager::endPasses()
{
	if (!_InReflectionRender)
		return; // no passes were begun
	nlassert(_Scene);
	IDriver *drv = _Scene->getDriver();

	_Scene->enableElementRender(UScene::FilterWater, true);
	_Scene->enableElementRender(UScene::FilterFlare, true);
	_InReflectionRender = false;

	// Restore scene and driver state
	drv->setRenderTarget(_SaveRenderTarget);
	drv->setupViewport(_SaveDrvViewport);
	CScissor fullScissor;
	fullScissor.initFullScreen();
	drv->setupScissor(fullScissor);
	drv->enableClipPlane(0, false);
	_Scene->setCam(_SaveCam);
	_Scene->setViewport(_SaveSceneViewport);
	_SaveRenderTarget = NULL;
	_SaveCam = NULL;
	_Passes.clear();
}

} /* namespace NL3D */

/* end of file */
