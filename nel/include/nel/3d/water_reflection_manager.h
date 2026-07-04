/**
 * \file water_reflection_manager.h
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

#ifndef NL3D_WATER_REFLECTION_MANAGER_H
#define NL3D_WATER_REFLECTION_MANAGER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/matrix.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/uv.h>
#include <nel/misc/vector_2f.h>

#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>

#include <map>
#include <vector>

namespace NL3D {

class CScene;
class CCamera;
class ITexture;

/**
 * Realtime planar reflections for water surfaces (HL2-era).
 *
 * Owned by CScene. Each frame:
 * - During the main scene render, visible water models report their
 *   reflection plane (world z), projected screen area, and screen-space
 *   AABB via reportVisibleSurface().
 * - Before the next main render (the render loop's scene reflections
 *   pass), beginPasses() ranks the reported planes by screen area (with
 *   hysteresis in favor of already-active planes) and admits up to the
 *   configured budget, returning the number of reflection passes. The
 *   caller renders each pass through its own render logic — the render
 *   loop replays its scene rendering with the reflected camera the way
 *   the stereo pass system replicates passes — between beginPass() and
 *   endPass(), and finishes with endPasses(). beginPass() binds and
 *   clears the render target, restricts rendering to the active
 *   sub-region, sets the scene camera to the reflected camera and
 *   enables a clip plane slightly below the water surface. Water and
 *   flares exclude themselves at engine level while
 *   isRenderingReflection() is set.
 * - During the main render, water models query getActiveReflection() by
 *   plane height; when available (and allowed by the shape flag or the
 *   force-all override), the water render uses the reflection texture
 *   with projected UVs instead of the static envmap.
 *
 * Render targets use the "fixed mode" strategy validated by the
 * planar_reflection sample: allocation is derived from the window size
 * (stable across frames, no reallocation stutter; pow2 rounded down to
 * bound memory on large screens), while a per-frame active sub-region
 * sized to the water's screen AABB bounds fill rate. UVs are scaled by
 * (active / allocation) to address the sub-region.
 */
class CWaterReflectionManager
{
public:
	/// Published reflection state for one water plane, valid for one main render.
	struct CActiveReflection
	{
		NLMISC::CSmartPtr<ITexture>	Texture;
		NLMISC::CMatrix				ReflViewMatrix;	// world -> reflected camera space
		CFrustum					ReflFrustum;	// off-center sub-frustum used for the reflection render
		NLMISC::CUV					UVScale;		// maps sub-frustum [0,1] projection to the active RT sub-region
		float						PlaneZ;
	};

	CWaterReflectionManager();
	~CWaterReflectionManager();

	void			setScene(CScene *scene) { _Scene = scene; }

	/// \name Configuration
	// @{
	/// Maximum number of water planes with realtime reflection per frame. -1 = unlimited, 0 = disabled.
	void			setMaxReflections(sint maxReflections) { _MaxReflections = maxReflections; if (maxReflections == 0) _HadReflections = false; }
	sint			getMaxReflections() const { return _MaxReflections; }
	/// Enable realtime reflection on all water surfaces regardless of the per-shape artist flag.
	void			setForceReflections(bool force) { _ForceReflections = force; }
	bool			getForceReflections() const { return _ForceReflections; }
	/// Render reflections at half resolution (default true)
	void			setHalfRes(bool halfRes) { _HalfRes = halfRes; }
	bool			getHalfRes() const { return _HalfRes; }
	/// Round render target sizes down to powers of two, bounds memory (default true)
	void			setPow2(bool pow2) { _Pow2 = pow2; }
	bool			getPow2() const { return _Pow2; }
	/** Fixed render target allocation from the window size with an active
	  * sub-region (default true, avoids per-frame reallocation stutter).
	  * When false, targets are allocated at the active size (dynamic mode). */
	void			setFixedSize(bool fixedSize) { _FixedSize = fixedSize; }
	bool			getFixedSize() const { return _FixedSize; }
	/** Select the current view (eye), default 0. Reflections are rendered
	  * and published per view: stereo render loops set the view before each
	  * eye's reflection passes and before each eye's scene render, so that
	  * every eye reflects from its own camera. Plane selection and render
	  * target sub-region sizing are shared across views (stats are merged
	  * from all views' scene renders); only the mirrored camera and the
	  * render target contents are per-view. */
	void			setCurrentView(uint view) { _CurrentView = view; }
	uint			getCurrentView() const { return _CurrentView; }
	// @}

	/** Report a visible water surface. Called by CWaterModel during the
	 * main scene render. Screen coordinates are in [0,1] of the main
	 * camera viewport. */
	void			reportVisibleSurface(float planeZ, float screenArea,
						const NLMISC::CVector2f &screenMin, const NLMISC::CVector2f &screenMax,
						bool shapeAllows);

	/// \name Reflection render passes
	// @{
	/** Select the water planes to reflect this frame (consumes the merged
	  * stats from the last main render). Returns the number of reflection
	  * passes to render. Selection is camera-independent, so this may be
	  * called before the render loop starts and the count handed to the
	  * loop driver (IStereoDisplay::setSceneReflectionPasses); each pass
	  * may then be rendered once per view. For each pass, set the current
	  * view, call beginPass(), render the scene content through the
	  * caller's own render logic, then endPass(); finish the frame with
	  * endPasses() (safe to call when zero passes were returned). */
	uint			beginPasses();
	/** Set up reflection pass 'pass' for the current view: saves the scene
	  * camera and driver state, binds and clears the render target,
	  * restricts rendering to the active sub-region, sets the scene camera
	  * to the current scene camera mirrored across the water plane and
	  * enables the water clip plane. The pass state (reflected view matrix,
	  * sub-frustum, UV scale — the active viewport is
	  * (0, 0, UVScale.U, UVScale.V) of the render target) is returned in
	  * 'out' for the caller's own drawing. */
	void			beginPass(uint pass, CActiveReflection &out);
	/** Publish the reflection rendered by pass 'pass' for the current view
	  * and restore the scene camera and driver state. */
	void			endPass(uint pass);
	/// End the frame's reflection passes.
	void			endPasses();
	// @}

	/// \name Queries during the main render (all for the current view)
	// @{
	/// Returns the active reflection for the given plane height, or NULL.
	const CActiveReflection	*getActiveReflection(float planeZ) const;
	/// Returns the active reflection by index (0..getNumActiveReflections()-1), or NULL. For debug display.
	const CActiveReflection	*getActiveReflectionByIndex(uint index) const;
	bool			hasActiveReflections() const { const CView *v = currentView(); return v && !v->Active.empty(); }
	uint			getNumActiveReflections() const { const CView *v = currentView(); return v ? (uint)v->Active.size() : 0; }
	/// True while the manager is rendering a reflection pass (guards recursion and stat collection).
	bool			isRenderingReflection() const { return _InReflectionRender; }
	/** True while ANY scene's reflection pass is rendering. Content that
	  * lives in auxiliary scenes rendered into a reflection (e.g. the sun
	  * flare in the sky scene) must check this rather than its own scene's
	  * manager. */
	static bool		isAnyRenderingReflection() { return _AnyReflectionRenderCount != 0; }
	/** True when water models should compute and report visibility stats:
	  * the app has opted in by calling renderReflections() at least once,
	  * reflections aren't disabled, and no reflection pass is rendering. */
	bool			wantsSurfaceReports() const { return _CollectionArmed && _MaxReflections != 0 && !_InReflectionRender; }
	/** True once any reflection has been rendered. Used to keep the shared
	  * water vertex buffer format stable (with the reflection UV channel)
	  * instead of reallocating it whenever reflections toggle on and off. */
	bool			needsPlanarUVs() const { return _HadReflections; }
	// @}

	/// Release render targets and the internal camera (driver reset / scene teardown).
	void			release();

private:
	// Quantize a plane height to a bucket key (1/16 m resolution)
	static sint32	planeKey(float z) { return (sint32)floorf(z * 16.f + 0.5f); }

	// Per-plane visibility stats collected during the main render
	struct CPlaneStats
	{
		float				PlaneZ;
		float				Area;		// summed projected screen area
		NLMISC::CVector2f	Min, Max;	// united screen AABB, [0,1]
		bool				Allowed;	// at least one surface with the artist flag
	};

	// One reflection render target slot
	struct CSlot
	{
		NLMISC::CSmartPtr<ITexture>	Texture;
		uint						AllocW, AllocH;
		CSlot() : AllocW(0), AllocH(0) {}
	};

	// Per-view (per-eye) reflection state
	struct CView
	{
		std::map<sint32, CActiveReflection>	Active;	// published reflections for the current main render
		std::vector<CSlot>					Slots;
	};

	// Selected state for one reflection pass (one plane, rendered per view)
	struct CPassData
	{
		sint32				Key;
		CPlaneStats			Stats;
		CActiveReflection	Refl;	// filled by beginPass, published by endPass
	};

	CCamera			*getReflCamera();
	const CView		*currentView() const { return _CurrentView < _Views.size() ? &_Views[_CurrentView] : NULL; }
	CView			&ensureCurrentView();

	CScene								*_Scene;
	sint								_MaxReflections;
	bool								_ForceReflections;
	bool								_HalfRes;
	bool								_Pow2;
	bool								_FixedSize;
	bool								_InReflectionRender;
	bool								_CollectionArmed;
	bool								_HadReflections;
	uint								_CurrentView;

	std::map<sint32, CPlaneStats>		_Collected;	// stats from the current main render, merged across views
	std::vector<CView>					_Views;		// per-view published reflections and render targets
	std::vector<sint32>					_PrevAdmitted; // hysteresis: planes admitted last frame
	std::vector<CPassData>				_Passes;	// selected passes between beginPasses and endPasses
	CCamera								*_ReflCamera;

	static uint							_AnyReflectionRenderCount; // reflection passes rendering, across all scenes

	// state saved across one reflection pass (beginPass..endPass)
	CCamera								*_SaveCam;
	CViewport							_SaveSceneViewport;
	NLMISC::CSmartPtr<ITexture>			_SaveRenderTarget;
	CViewport							_SaveDrvViewport;

}; /* class CWaterReflectionManager */

} /* namespace NL3D */

#endif /* #ifndef NL3D_WATER_REFLECTION_MANAGER_H */

/* end of file */
