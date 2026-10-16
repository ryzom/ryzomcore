/**
 * \file nav_mouse_listener.h
 * \brief Viewport navigation on the view-target model.
 * \author Jan Boon (Kaetemi)
 *
 * A drop-in alternative to CEvent3dMouseListener's `edit3d` mode. Two reasons that mode
 * could not be kept:
 *
 * 1. edit3d aliases navigation onto MODIFIED LEFT drags (Alt+Left orbit, Ctrl+Left pan,
 *    Shift+Left zoom, Ctrl+Shift+Left dolly). Those modifiers are needed for sub-object
 *    selection (Ctrl+Left = add, Alt+Left = subtract), so the left button has to belong
 *    entirely to select/paint/transform. Navigation is middle-button only: MMB pan,
 *    Alt+MMB orbit, Ctrl+Alt+MMB dolly; Ctrl+MMB is accelerated pan (edit3d used it for
 *    dolly) and Shift+MMB is constrained pan.
 *
 * 2. edit3d orbits about a FIXED world point (_HotSpot) that nothing ever moves. The pivot
 *    here is the VIEW TARGET, a camera-relative point at
 *    `camPos + forward * targetDistance`, so panning carries the pivot along. Each
 *    navigation op touches a different half of that pair:
 *
 *      pan    camera translates, distance unchanged  -> target rides along with the camera
 *      dolly  camera moves along the view axis       -> target stays put, distance shrinks
 *      orbit  rigid rotation about the pivot         -> both unchanged
 *      frame  Zoom Extents (Selected)                -> both re-derived from the framed box
 *
 *    Wheel zoom is a dolly, so it also converges on the target rather than on a stale point.
 *
 * Orbit pivot policy is not a separate mechanism: it only chooses WHERE the pivot comes
 * from for the duration of the orbit. Plain orbit derives it from the camera (the
 * maintained target); the other mode substitutes a selection centre. That is the whole of
 * TOrbitPivot here.
 *
 * The pan / orbit / dolly arithmetic is kept line-for-line with the edit3d implementation
 * (nel/src/3d/event_mouse_listener.cpp) so the drag FEEL is unchanged - including its
 * `(vect.x+vect.y+vect.z)` dolly scalarization, which is odd but is what the tool has always
 * felt like. Only the target bookkeeping and the button map are new.
 *
 * The public shape mirrors CEvent3dMouseListener (getViewMatrix / setMatrix / setFrustrum /
 * setViewport / addToServer / removeFromServer) so the viewer swap is mechanical.
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

#ifndef NL_NAV_MOUSE_LISTENER_H
#define NL_NAV_MOUSE_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"

#include "nel/3d/frustum.h"
#include "nel/3d/viewport.h"

#include <deque>

namespace NLMISC {
class CEventServer;
}

namespace NL3D {

/// Where an orbit drag pivots.
enum TOrbitPivot
{
	/// Maintained view target (camera-derived, so it follows panning).
	PivotViewTarget = 0,
	/// Caller-supplied selection centre.
	PivotSelection
};

class CNavMouseListener : public NLMISC::IEventListener
{
public:
	CNavMouseListener();
	virtual ~CNavMouseListener() NL_OVERRIDE { }

	// --- host wiring (same shape as CEvent3dMouseListener; the viewer swap is mechanical)
	void addToServer(NLMISC::CEventServer &server);
	void removeFromServer(NLMISC::CEventServer &server);
	const NLMISC::CMatrix &getViewMatrix() const { return m_Matrix; }
	void setMatrix(const NLMISC::CMatrix &matrix) { m_Matrix = matrix; }
	void setFrustrum(const CFrustum &frustum) { m_Frustum = frustum; }
	void setViewport(const CViewport &viewport) { m_Viewport = viewport; }

	// --- view target (the orbit / dolly / wheel pivot)
	/** Place the target at a world point; the distance re-derives from the current camera. */
	void setTarget(const NLMISC::CVector &worldPos);
	const NLMISC::CVector &target() const { return m_Target; }
	float targetDistance() const { return m_TargetDist; }

	/**
	 * Zoom Extents (Selected): frame `box` keeping the current view DIRECTION (no reorient),
	 * and re-derive both halves of the target pair from it. Pushes a view-history entry so
	 * Shift+Z steps back over it. Degenerate boxes get a floor radius.
	 */
	void frameBox(const NLMISC::CAABBox &box);

	// --- orbit pivot policy
	void setOrbitPivot(TOrbitPivot pivot) { m_OrbitPivot = pivot; }
	TOrbitPivot orbitPivot() const { return m_OrbitPivot; }
	/** Selection centre for PivotSelection; `valid` false falls back to the view target. */
	void setSelectionPivot(const NLMISC::CVector &worldPos, bool valid);

// --- interaction state, for hosts that key overlays off the view
/**
 * True while a middle-button view drag is in flight. Editors hide their manipulators for
 * the duration and re-fit them when it ends, so a screen-sized gizmo is never seen
 * resizing under a drag - which is why it reads as stable.
 */
bool isNavigating() const { return m_Drag != DragNone || m_HostGesture; }

// --- host-driven gestures
/**
 * For hosts that own an input policy this listener cannot see - a one-finger drag that
 * only counts as navigation because it started on empty space, a touchpad convention, an
 * on-screen control. Bracket the gesture with begin/end so it snapshots the view for
 * Shift+Z and reports through isNavigating() like any other drag, then feed it motion.
 */
void beginHostGesture();
void endHostGesture() { m_HostGesture = false; }
/** Pan so the world point under (fromX, fromY) ends up under (toX, toY). Viewport coords. */
void panBetween(float fromX, float fromY, float toX, float toY);
/**
 * Bumped whenever the camera moves, by any route: drag, wheel, framing, view undo/redo.
 * Hosts caching something derived from the view - a screen-constant gizmo scale, a LOD
 * choice - compare serials instead of re-deriving every frame.
 */
uint32 viewSerial() const { return m_ViewSerial; }

// --- view history (Shift+Z / Shift+Y)
	/** Snapshot the current view; drag starts and frameBox do this for you. */
	void pushViewState();
	bool viewUndo();
	bool viewRedo();

	// inherited
	virtual void operator()(const NLMISC::CEvent &event) NL_OVERRIDE;

private:
	/// What a middle-button drag is doing, decided per move event from the button+modifier mask.
	enum TDrag
	{
		DragNone = 0,
		DragPan,            ///< MMB
		DragPanFast,        ///< Ctrl+MMB (accelerated pan; edit3d spent this on dolly)
		DragPanConstrained, ///< Shift+MMB (locked to the drag's dominant screen axis)
		DragOrbit,          ///< Alt+MMB
		DragDolly           ///< Ctrl+Alt+MMB
	};

	struct SViewState
	{
		NLMISC::CMatrix Matrix;
		NLMISC::CVector Target;
		float TargetDist;
		SViewState() : TargetDist(0.f) { }
	};

	static TDrag classify(uint32 button);
	/** Unproject both mouse points onto the camera-facing plane through the target. */
	void dragPlaneDelta(float x0, float y0, float x1, float y1, NLMISC::CVector &out) const;
	void applyPan(const NLMISC::CVector &delta);
	void applyDolly(float amount);
	void applyOrbit(float dx, float dy);
	/** Keep the target on the view axis after a rigid camera rotation. */
	void reprojectTargetFromCamera();
	/// Single choke point for "the camera moved" - every mutation of m_Matrix goes through it.
	void noteViewChanged() { ++m_ViewSerial; }
	/** Recompute the distance after the camera moved along the view axis (target fixed). */
	void reprojectDistanceFromCamera();
	void captureCurrent(SViewState &out) const;
	void restore(const SViewState &in);

	NLMISC::CMatrix m_Matrix;
	CFrustum m_Frustum;
	CViewport m_Viewport;

	NLMISC::CVector m_Target;
	float m_TargetDist;

	TOrbitPivot m_OrbitPivot;
	NLMISC::CVector m_SelectionPivot;
	bool m_HaveSelectionPivot;

	// drag state
	float m_X, m_Y;             ///< last mouse position (viewport space)
	/// True between beginHostGesture/endHostGesture; folds into isNavigating().
	bool m_HostGesture;
	/// Bumped by noteViewChanged() on every camera move; see viewSerial().
	uint32 m_ViewSerial;
	TDrag m_Drag;               ///< kind of the drag in flight
	float m_DragStartX, m_DragStartY;
	int m_ConstrainAxis;        ///< constrained pan: 0 = undecided, 1 = horizontal, 2 = vertical

	std::deque<SViewState> m_Undo;
	std::deque<SViewState> m_Redo;

}; /* class CNavMouseListener */

} /* namespace NL3D */

#endif /* NL_NAV_MOUSE_LISTENER_H */

/* end of file */
