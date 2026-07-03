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
	, _CurrentView(0)
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
	_Views.clear();
	_Collected.clear();
	_PrevAdmitted.clear();
	_Passes.clear();
	_SaveRenderTarget = NULL;
	_SaveCam = NULL;
	_CollectionArmed = false;
	_HadReflections = false;
	_CurrentView = 0;
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
	const CView *view = currentView();
	if (!view) return NULL;
	std::map<sint32, CActiveReflection>::const_iterator it = view->Active.find(planeKey(planeZ));
	if (it == view->Active.end()) return NULL;
	return &it->second;
}

// ***************************************************************************
const CWaterReflectionManager::CActiveReflection *CWaterReflectionManager::getActiveReflectionByIndex(uint index) const
{
	const CView *view = currentView();
	if (!view || index >= view->Active.size()) return NULL;
	std::map<sint32, CActiveReflection>::const_iterator it = view->Active.begin();
	std::advance(it, index);
	return &it->second;
}


// ***************************************************************************
CWaterReflectionManager::CView &CWaterReflectionManager::ensureCurrentView()
{
	if (_CurrentView >= _Views.size())
		_Views.resize(_CurrentView + 1);
	return _Views[_CurrentView];
}

// ***************************************************************************
uint CWaterReflectionManager::beginPasses()
{
	nlassert(_Scene);
	nlassert(!_InReflectionRender); // missing endPass()?

	// Water models only report visibility stats once this has been called;
	// applications that never render reflections must not accumulate stats
	_CollectionArmed = true;

	// Take the merged stats from the last main render
	std::map<sint32, CPlaneStats> collected;
	collected.swap(_Collected);

	for (uint i = 0; i < _Views.size(); ++i)
		_Views[i].Active.clear();
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
	float camZ = mainCam->getMatrix().getPos().z; // local matrix: see beginPass

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

	_PrevAdmitted.clear();
	if (!numAdmitted)
		return 0;

	// Stash the selected passes until render time
	_Passes.resize(numAdmitted);
	for (uint i = 0; i < numAdmitted; ++i)
	{
		_Passes[i].Key = candidates[i].second;
		_Passes[i].Stats = collected[candidates[i].second];
		_PrevAdmitted.push_back(candidates[i].second);
	}

	return numAdmitted;
}

// ***************************************************************************
void CWaterReflectionManager::beginPass(uint pass, CActiveReflection &out)
{
	nlassert(_Scene);
	nlassert(!_InReflectionRender); // missing endPass()?
	nlassert(pass < _Passes.size());
	if (_InReflectionRender || pass >= _Passes.size())
	{
		// Misuse (unbalanced bracketing or a stale pass index from the
		// render loop): publish nothing rather than capturing the
		// manager's own state as the scene state to restore
		out = CActiveReflection();
		out.PlaneZ = 0.f;
		return;
	}
	CPassData &pd = _Passes[pass];
	const CPlaneStats &stats = pd.Stats;
	IDriver *drv = _Scene->getDriver();

	const float planeZ = stats.PlaneZ;

	// Save scene and driver state. The render target must be restored to
	// the previously bound one, not to NULL: the caller may already be
	// rendering into an effects render target (bloom/FXAA pipeline).
	CCamera *mainCam = _Scene->getCam();
	nlassert(mainCam);
	_SaveCam = mainCam;
	_SaveSceneViewport = _Scene->getViewport();
	_SaveRenderTarget = drv->getRenderTarget();
	drv->getViewport(_SaveDrvViewport);

	// --- Reflected camera: mirror the current (per-view) camera across the
	// plane ---
	// Basis vectors are mirrored in z; I is negated afterwards to restore
	// right-handedness (the horizontal flip is absorbed by projecting the
	// UVs through this same camera). No world-up derivation, so steep
	// look-down camera angles stay robust.
	// The camera's LOCAL matrix is mirrored, not its cached world matrix:
	// the world matrix is refreshed by the HRC pass of each replicated
	// scene render, so under a moving camera it holds the previous frame's
	// value for the frame's first reflection pass and the current frame's
	// for later ones — per-eye reflections would mirror different cameras
	// (found with the stereo debugger's comparison composite). The local
	// matrix is the currently committed camera, immediately, for every
	// pass. Cameras are root-level transforms, so local == world.
	const CMatrix &camWorld = mainCam->getMatrix();
	CVector I = camWorld.getI(), J = camWorld.getJ(), K = camWorld.getK(), P = camWorld.getPos();
	CVector Im(-I.x, -I.y, I.z);
	CVector Jm(J.x, J.y, -J.z);
	CVector Km(K.x, K.y, -K.z);
	CVector Pm(P.x, P.y, 2.f * planeZ - P.z);
	CMatrix reflCamWorld;
	reflCamWorld.identity();
	reflCamWorld.setRot(Im, Jm, Km);
	reflCamWorld.setPos(Pm);

	// --- Screen AABB of the water in the reflected view ---
	// For points on the mirror plane, the reflected-camera projection is
	// the main-camera projection mirrored across the optical axis:
	// u' = -u - 2*Left/(Right-Left), which reduces to 1-u for x-symmetric
	// frusta but stays correct for off-center (per-eye HMD) frusta. The
	// AABB is merged across views, so all views share the sub-region
	// sizing.
	float fl, fr, fb, ft, fnear, ffar;
	mainCam->getFrustum(fl, fr, fb, ft, fnear, ffar);
	const float mirrorOfs = -2.f * fl / (fr - fl);
	float minX = -stats.Max.x + mirrorOfs;
	float maxX = -stats.Min.x + mirrorOfs;
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

	// The reported AABB is relative to the camera viewport, which may be a
	// sub-region of the window (per-eye side-by-side viewports)
	const float vpPixW = (float)winW * _SaveSceneViewport.getWidth();
	const float vpPixH = (float)winH * _SaveSceneViewport.getHeight();

	// Active region: snapped AABB dimensions, padded symmetrically
	uint rawW = (uint)std::max(1.f, ceilf((maxX - minX) * vpPixW));
	uint rawH = (uint)std::max(1.f, ceilf((maxY - minY) * vpPixH));
	uint snappedW = snapUp(rawW, WATER_REFLECTION_SNAP);
	uint snappedH = snapUp(rawH, WATER_REFLECTION_SNAP);
	float padX = (float)(snappedW - rawW) / (2.f * std::max(1.f, vpPixW));
	float padY = (float)(snappedH - rawH) / (2.f * std::max(1.f, vpPixH));
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
	CFrustum subFrustum;
	float fw = fr - fl;
	float fh = ft - fb;
	subFrustum.init(
		fl + minX * fw, fl + maxX * fw,
		fb + minY * fh, fb + maxY * fh,
		fnear, ffar, true);

	// --- Render target for this pass, in the current view's slot ---
	CView &view = ensureCurrentView();
	if (view.Slots.size() <= pass)
		view.Slots.resize(pass + 1);
	CSlot &slot = view.Slots[pass];
	if (!slot.Texture || slot.AllocW != allocW || slot.AllocH != allocH)
	{
		CTextureOffscreen *tex = new CTextureOffscreen();
		tex->setNeedsDepthStencil(true);
		tex->setRenderTarget(true);
		tex->setReleasable(false);
		tex->resize(allocW, allocH);
		tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
		tex->setWrapS(ITexture::Clamp);
		tex->setWrapT(ITexture::Clamp);
		drv->setupTexture(*tex);
		slot.Texture = tex;
		slot.AllocW = allocW;
		slot.AllocH = allocH;
	}

	// --- Bind the render target ---
	drv->setRenderTarget(slot.Texture, 0, 0, allocW, allocH);

	float vpW = (float)activeW / (float)allocW;
	float vpH = (float)activeH / (float)allocH;

	// Clear the full allocation to the fog color so out-of-frustum samples
	// and the wobble margin around the active region blend in. The pass
	// owns its write state: glClearBuffer honors the color write mask, so
	// a mask left disabled by earlier rendering (occlusion tests, special
	// multipass) must not turn this clear (and the whole pass) into a no-op.
	drv->setColorMask(true, true, true, true);
	CViewport fullVP;
	fullVP.initFullScreen();
	drv->setupViewport(fullVP);
	CScissor fullScissor;
	fullScissor.initFullScreen();
	drv->setupScissor(fullScissor);
	CRGBA clearColor = drv->fogEnabled() ? drv->getFogColor() : CRGBA(0, 0, 0, 255);
	clearColor.A = 255;
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
	pd.Refl.Texture = slot.Texture;
	pd.Refl.ReflViewMatrix = reflCamWorld;
	pd.Refl.ReflViewMatrix.invert();
	pd.Refl.ReflFrustum = subFrustum;
	pd.Refl.UVScale.U = vpW;
	pd.Refl.UVScale.V = vpH;
	pd.Refl.PlaneZ = planeZ;
	drv->setFrustum(subFrustum.Left, subFrustum.Right, subFrustum.Bottom, subFrustum.Top, subFrustum.Near, subFrustum.Far, true);
	drv->setupViewMatrixEx(pd.Refl.ReflViewMatrix, Pm);
	drv->setClipPlane(0, CPlane(0.f, 0.f, 1.f, -(planeZ - WATER_REFLECTION_CLIP_BIAS)));
	drv->enableClipPlane(0, true);

	// --- Scene camera for the caller's scene render ---
	// The mirrored camera sits below the surface, outside any cluster. By
	// mirror symmetry the reflection sees the clusters visible from the
	// real eye: inherit its cluster system and resolve the camera clusters
	// from its unmirrored position.
	CCamera *reflCam = getReflCamera();
	reflCam->setMatrix(reflCamWorld);
	reflCam->setFrustum(subFrustum);
	reflCam->setClusterSystem(mainCam->getClusterSystem());
	_Scene->getClipTrav().setClusterVisibilityPosOverride(true, P);
	_Scene->setCam(reflCam);
	CViewport sceneVP;
	sceneVP.init(0.f, 0.f, vpW, vpH);
	_Scene->setViewport(sceneVP);

	// While set, water and flares exclude themselves from the render (at
	// engine level, since render loops re-apply their own scene filters
	// inside the pass) and water models don't report visibility stats
	_InReflectionRender = true;
	_HadReflections = true;

	out = pd.Refl;
}

// ***************************************************************************
void CWaterReflectionManager::endPass(uint pass)
{
	nlassert(_Scene);
	nlassert(_InReflectionRender);
	nlassert(pass < _Passes.size());
	if (!_InReflectionRender || pass >= _Passes.size())
		return;
	IDriver *drv = _Scene->getDriver();

	_InReflectionRender = false;

	// Publish for the coming main render of the current view
	ensureCurrentView().Active[_Passes[pass].Key] = _Passes[pass].Refl;

	// Restore scene and driver state
	_Scene->getClipTrav().setClusterVisibilityPosOverride(false);
	drv->enableClipPlane(0, false);
	drv->setRenderTarget(_SaveRenderTarget);
	drv->setupViewport(_SaveDrvViewport);
	CScissor fullScissor;
	fullScissor.initFullScreen();
	drv->setupScissor(fullScissor);
	_Scene->setCam(_SaveCam);
	_Scene->setViewport(_SaveSceneViewport);
	_SaveRenderTarget = NULL;
	_SaveCam = NULL;
}

// ***************************************************************************
void CWaterReflectionManager::endPasses()
{
	nlassert(!_InReflectionRender); // missing endPass()?
	_Passes.clear();
}

} /* namespace NL3D */

/* end of file */
