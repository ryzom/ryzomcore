/**
 * \file zp_nav.cpp
 * \brief See zp_nav.h.
 * \author Jan Boon (Kaetemi)
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
#include "zp_nav.h"

#include <nel/misc/common.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/plane.h>

#include <cmath>

namespace ZPNAV {

// Ctrl+MMB accelerated pan multiplier. Tunable by feel; the ratio that reads as
// "clearly faster, still controllable" over a 160 m ligo cell.
static const float kPanAccel = 3.f;

// Wheel step, as a fraction of the camera->target distance (edit3d's constant, kept so the
// stepped zoom feels the same; only its convergence point changes - the maintained target
// rather than a hotspot fixed at session start).
static const float kWheelStep = 0.1f;

// The camera may not sit on top of its own target: dollying through it inverts orbit.
// Rather than block the zoom, clamp the distance and let the target ride forward, so orbit
// keeps working and the next frame/Z resets it cleanly.
static const float kMinTargetDist = 0.5f;

// Framing distance for a degenerate (single point / zero extent) box.
static const float kMinFrameRadius = 2.f;

static const size_t kViewHistoryMax = 32;

CNavListener::CNavListener()
	: m_TargetDist(1.f), m_OrbitPivot(PivotViewTarget), m_HaveSelectionPivot(false),
	  m_X(0.f), m_Y(0.f), m_Drag(DragNone), m_DragStartX(0.f), m_DragStartY(0.f),
	  m_ConstrainAxis(0)
{
	m_Matrix.identity();
	m_Target = NLMISC::CVector::Null;
	m_SelectionPivot = NLMISC::CVector::Null;
}

void CNavListener::addToServer(NLMISC::CEventServer &server)
{
	server.addListener(NLMISC::EventMouseMoveId, this);
	server.addListener(NLMISC::EventMouseDownId, this);
	server.addListener(NLMISC::EventMouseUpId, this);
	server.addListener(NLMISC::EventMouseWheelId, this);
}

void CNavListener::removeFromServer(NLMISC::CEventServer &server)
{
	server.removeListener(NLMISC::EventMouseMoveId, this);
	server.removeListener(NLMISC::EventMouseDownId, this);
	server.removeListener(NLMISC::EventMouseUpId, this);
	server.removeListener(NLMISC::EventMouseWheelId, this);
}

// ---------------------------------------------------------------------------------------------
// View target

void CNavListener::setTarget(const NLMISC::CVector &worldPos)
{
	m_Target = worldPos;
	reprojectDistanceFromCamera();
}

void CNavListener::setSelectionPivot(const NLMISC::CVector &worldPos, bool valid)
{
	m_SelectionPivot = worldPos;
	m_HaveSelectionPivot = valid;
}

void CNavListener::reprojectDistanceFromCamera()
{
	// Distance along the view axis, not the raw euclidean one: after a dolly the target is
	// still on the axis, and projecting keeps a numerically clean pair.
	const float d = (m_Target - m_Matrix.getPos()) * m_Matrix.getJ();
	m_TargetDist = d > kMinTargetDist ? d : kMinTargetDist;
	// Clamped: the camera dollied onto (or through) the target - carry the target forward so
	// the next orbit still has something sane to pivot about.
	if (d <= kMinTargetDist)
		m_Target = m_Matrix.getPos() + m_Matrix.getJ() * kMinTargetDist;
}

void CNavListener::reprojectTargetFromCamera()
{
	m_Target = m_Matrix.getPos() + m_Matrix.getJ() * m_TargetDist;
}

void CNavListener::frameBox(const NLMISC::CAABBox &box)
{
	pushViewState();
	const NLMISC::CVector center = box.getCenter();
	float radius = box.getRadius();
	if (radius < kMinFrameRadius)
		radius = kMinFrameRadius;
	// Same framing rule as the session's initial camera placement: back off twice the radius
	// along the CURRENT view direction (Zoom Extents does not reorient the viewport).
	const float dist = radius * 2.f;
	m_Matrix.setPos(center - m_Matrix.getJ() * dist);
	m_Target = center;
	m_TargetDist = dist > kMinTargetDist ? dist : kMinTargetDist;
}

// ---------------------------------------------------------------------------------------------
// View history

void CNavListener::captureCurrent(SViewState &out) const
{
	out.Matrix = m_Matrix;
	out.Target = m_Target;
	out.TargetDist = m_TargetDist;
}

void CNavListener::restore(const SViewState &in)
{
	m_Matrix = in.Matrix;
	m_Target = in.Target;
	m_TargetDist = in.TargetDist;
}

void CNavListener::pushViewState()
{
	SViewState s;
	captureCurrent(s);
	m_Undo.push_back(s);
	while (m_Undo.size() > kViewHistoryMax)
		m_Undo.pop_front();
	// A new view change invalidates the redo chain, exactly like the paint undo stacks.
	m_Redo.clear();
}

bool CNavListener::viewUndo()
{
	if (m_Undo.empty())
		return false;
	SViewState cur;
	captureCurrent(cur);
	m_Redo.push_back(cur);
	restore(m_Undo.back());
	m_Undo.pop_back();
	return true;
}

bool CNavListener::viewRedo()
{
	if (m_Redo.empty())
		return false;
	SViewState cur;
	captureCurrent(cur);
	m_Undo.push_back(cur);
	restore(m_Redo.back());
	m_Redo.pop_back();
	return true;
}

// ---------------------------------------------------------------------------------------------
// Drag classification: MIDDLE BUTTON ONLY. Every left/right combination resolves to DragNone
// so the left button belongs entirely to select / paint / transform (see the header).

CNavListener::TDrag CNavListener::classify(uint32 button)
{
	if (button == NLMISC::middleButton) return DragPan;
	if (button == (uint32)(NLMISC::ctrlButton | NLMISC::middleButton)) return DragPanFast;
	if (button == (uint32)(NLMISC::shiftButton | NLMISC::middleButton)) return DragPanConstrained;
	if (button == (uint32)(NLMISC::altButton | NLMISC::middleButton)) return DragOrbit;
	if (button == (uint32)(NLMISC::ctrlButton | NLMISC::altButton | NLMISC::middleButton)) return DragDolly;
	return DragNone;
}

// ---------------------------------------------------------------------------------------------
// Drag maths (edit3d port; see the header note on keeping the feel identical)

void CNavListener::dragPlaneDelta(float x0, float y0, float x1, float y1, NLMISC::CVector &out) const
{
	// Camera-facing plane through the target. Everything is computed target-local for the
	// same precision reason edit3d does it: world coordinates on a ligo board are large.
	NLMISC::CPlane plane;
	plane.make(m_Matrix.getJ(), NLMISC::CVector::Null);

	NLMISC::CMatrix localViewMatrix = m_Matrix;
	localViewMatrix.setPos(m_Matrix.getPos() - m_Target);

	NLMISC::CVector pos, dir;
	m_Viewport.getRayWithPoint(x0, y0, pos, dir, localViewMatrix, m_Frustum);
	const NLMISC::CVector p0 = plane.intersect(pos, pos + dir);
	m_Viewport.getRayWithPoint(x1, y1, pos, dir, localViewMatrix, m_Frustum);
	const NLMISC::CVector p1 = plane.intersect(pos, pos + dir);
	out = p0 - p1;
}

void CNavListener::applyPan(const NLMISC::CVector &delta)
{
	// Camera translates, distance unchanged -> the target rides along.
	m_Matrix.setPos(m_Matrix.getPos() + delta);
	m_Target += delta;
}

void CNavListener::applyDolly(float amount)
{
	NLMISC::CVector direc = m_Target - m_Matrix.getPos();
	if (direc.norm() < 0.0001f)
		return;
	direc.normalize();
	m_Matrix.setPos(m_Matrix.getPos() + direc * amount);
	// Target stays put in world space; the distance is what shrinks.
	reprojectDistanceFromCamera();
}

void CNavListener::applyOrbit(float dx, float dy)
{
	// Pivot: the maintained view target, or the selection centre when the caller asked
	// for PivotSelection and gave us one.
	const NLMISC::CVector axis = (m_OrbitPivot == PivotSelection && m_HaveSelectionPivot)
		? m_SelectionPivot : m_Target;

	NLMISC::CMatrix comeFromHotSpot = m_Matrix;
	comeFromHotSpot.setPos(axis);

	NLMISC::CMatrix turnZ;
	turnZ.identity();
	turnZ.rotateZ((float)NLMISC::Pi * 2.f * dx);

	NLMISC::CMatrix turnX;
	turnX.identity();
	turnX.rotateX((float)NLMISC::Pi * 2.f * dy);

	NLMISC::CMatrix goToHotSpot = comeFromHotSpot;
	goToHotSpot.invert();

	NLMISC::CMatrix negPivot, pivot;
	negPivot.identity();
	negPivot.setPos(-axis);
	pivot.identity();
	pivot.setPos(axis);

	pivot *= turnZ;
	pivot *= negPivot;
	pivot *= comeFromHotSpot;
	pivot *= turnX;
	pivot *= goToHotSpot;
	pivot *= m_Matrix;
	m_Matrix = pivot;
	m_Matrix.normalize(NLMISC::CMatrix::XYZ);

	// A rigid rotation about the target leaves it on the view axis, so this is a no-op for
	// PivotViewTarget. Under PivotSelection the camera moved relative to the target, and
	// re-deriving keeps the pair consistent (target back in front of the camera).
	reprojectTargetFromCamera();
}

// ---------------------------------------------------------------------------------------------

void CNavListener::operator()(const NLMISC::CEvent &event)
{
	if (event == NLMISC::EventMouseWheelId)
	{
		// Stepped zoom: a dolly toward the target, so it converges on the same point Alt+MMB
		// pivots about.
		const NLMISC::CEventMouseWheel *wheel = (const NLMISC::CEventMouseWheel *)&event;
		const NLMISC::CVector direc = m_Target - m_Matrix.getPos();
		m_Matrix.setPos(m_Matrix.getPos() + direc * (wheel->Direction ? kWheelStep : -kWheelStep));
		reprojectDistanceFromCamera();
		return;
	}

	const NLMISC::CEventMouse *mouse = (const NLMISC::CEventMouse *)&event;

	if (event == NLMISC::EventMouseDownId || event == NLMISC::EventMouseUpId)
	{
		// Position only; the drag kind is decided per move from the live button mask (same as
		// edit3d, and robust against a down event lost to a focus change).
		m_X = mouse->X;
		m_Y = mouse->Y;
		if (event == NLMISC::EventMouseUpId)
		{
			m_Drag = DragNone;
			m_ConstrainAxis = 0;
		}
		return;
	}

	if (event != NLMISC::EventMouseMoveId)
		return;

	const TDrag drag = classify((uint32)mouse->Button);
	if (drag == DragNone)
	{
		m_Drag = DragNone;
		m_ConstrainAxis = 0;
		m_X = mouse->X;
		m_Y = mouse->Y;
		return;
	}

	if (drag != m_Drag)
	{
		// Drag start (or a modifier changed mid-drag, which restarts the constraint latch):
		// snapshot the view so Shift+Z steps back over the whole gesture.
		if (m_Drag == DragNone)
			pushViewState();
		m_Drag = drag;
		m_DragStartX = m_X;
		m_DragStartY = m_Y;
		m_ConstrainAxis = 0;
	}

	switch (drag)
	{
	case DragPan:
	case DragPanFast:
	{
		NLMISC::CVector delta;
		dragPlaneDelta(m_X, m_Y, mouse->X, mouse->Y, delta);
		applyPan(drag == DragPanFast ? delta * kPanAccel : delta);
		break;
	}
	case DragPanConstrained:
	{
		// Latch the dominant screen axis once per drag (do not re-decide as you wander),
		// then project the world delta onto the matching camera axis.
		if (m_ConstrainAxis == 0)
		{
			const float ax = (float)fabs(mouse->X - m_DragStartX);
			const float ay = (float)fabs(mouse->Y - m_DragStartY);
			const float kDeadZone = 0.004f; // viewport units; avoids latching on jitter
			if (ax > kDeadZone || ay > kDeadZone)
				m_ConstrainAxis = (ax >= ay) ? 1 : 2;
		}
		if (m_ConstrainAxis != 0)
		{
			NLMISC::CVector delta;
			dragPlaneDelta(m_X, m_Y, mouse->X, mouse->Y, delta);
			const NLMISC::CVector axis = (m_ConstrainAxis == 1) ? m_Matrix.getI() : m_Matrix.getK();
			applyPan(axis * (delta * axis));
		}
		break;
	}
	case DragOrbit:
		applyOrbit(m_X - mouse->X, mouse->Y - m_Y);
		break;
	case DragDolly:
	{
		NLMISC::CVector delta;
		dragPlaneDelta(m_X, m_Y, mouse->X, mouse->Y, delta);
		applyDolly(delta.x + delta.y + delta.z);
		break;
	}
	default:
		break;
	}

	m_X = mouse->X;
	m_Y = mouse->Y;
}

} /* namespace ZPNAV */

/* end of file */
