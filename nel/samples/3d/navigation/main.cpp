// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026 Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

//
// Navigation + move gizmo sample
//
// A minimum editor context for judging how NL3D::CNavMouseListener feels: a grid of
// cubes you can pick, and a move gizmo on the selection - three axis
// arrows, three plane handles, and a screen-space centre - so translation can be tried
// along an axis, in a plane, and parallel to the view.
//
// It exists because navigation cannot be judged by reading it, and because the same
// drags have to work on a mouse, on a laptop touchpad, and on a phone. It builds for
// WebGL, which is the only practical way to try the last two.
//
// Input, by design (see nav_mouse_listener.h for why):
// left pick a cube / drag a gizmo handle - never navigation
// middle pan; Alt+middle orbit; Ctrl+Alt+middle dolly
// Ctrl+mid fast pan; Shift+mid axis-locked pan
// wheel stepped zoom
// Z frame the selection (or everything, when nothing is selected)
//

#include <nel/misc/types_nl.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/events.h>
#include <nel/misc/geom_ext.h>
#include <nel/misc/matrix.h>
#include <nel/misc/plane.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector.h>

#include <nel/3d/nav_mouse_listener.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_material.h>
#include <nel/3d/viewport.h>

#include <algorithm>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#ifdef NL_OS_WINDOWS
#ifndef NL_COMP_MINGW
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace std;
using namespace NLMISC;
using namespace NL3D;

// ---------------------------------------------------------------------------------------------
// Scene

static const float kCubeHalf = 0.6f;

struct SCube
{
	CVector Pos;
	CRGBA Color;
	SCube() : Color(CRGBA::White) { }
	SCube(const CVector &p, CRGBA c) : Pos(p), Color(c) { }
};

// ---------------------------------------------------------------------------------------------
// Move gizmo
//
// Handles are hit-tested in SCREEN space against the projected geometry rather than by
// ray/solid intersection: it is simpler, it degrades gracefully when handles overlap, and
// - the reason that matters here - a fingertip is a large, imprecise pointer, so the pick
// radius wants to be a pixel budget rather than a world one.

enum TGizmoHandle
{
	GizmoNone = -1,
	GizmoAxisX = 0,
	GizmoAxisY,
	GizmoAxisZ,
	GizmoPlaneXY,
	GizmoPlaneYZ,
	GizmoPlaneZX,
	GizmoScreen,
	GizmoHandleCount
};

// Gizmo proportions, in gizmo-local units (the whole thing is scaled to a constant pixel
// size, so these are ratios rather than distances).
static const float kAxisLen = 1.f;
static const float kArrowLen = 0.22f;
static const float kArrowWide = 0.075f;
// Plane handles occupy the CORNER between their two axes, inner corner meeting the gizmo
// origin. They are not pushed out along the axes: the middle of the gizmo is where all
// three planes meet, and that meeting point is exactly what the screen handle means.
static const float kPlaneOff = 0.f;
static const float kPlaneSize = 0.28f;
// Cone tessellation for the arrowheads, and how far the inner cap is darkened.
static const int kConeSegments = 10;
static const uint8 kCapShade = 110;

// Constant on-screen size: the gizmo should not shrink as the camera pulls back, or it
// stops being clickable exactly when you most need it.
// Target on-screen height of the gizmo, in physical pixels. It is only ever APPLIED at rest
// - see updateGizmoFit - so the gizmo holds a fixed world size throughout any interaction.
static const float kGizmoPixels = 150.f;

// The screen-space handle has no geometry of its own: it is the empty middle of the gizmo,
// and it announces itself by lighting all three plane handles instead of adding a fourth
// shape to an already busy centre. This is its pick radius, in viewport-height fractions.
static const float kScreenPickRadius = 0.018f;

// Screen-space pick radius for the axis shafts, as a fraction of viewport height. Generous
// on purpose - this is the number to raise if the gizmo feels fiddly under a finger.
static const float kAxisPickRadius = 0.014f;

/** Unit vector for axis 0/1/2 (CVector has no operator[]). */
static CVector axisVec(int a)
{
	return a == 0 ? CVector::I : (a == 1 ? CVector::J : CVector::K);
}

static const CRGBA kAxisColor[3] = {
	CRGBA(230, 70, 70), CRGBA(90, 220, 90), CRGBA(90, 140, 240)
};
static const CRGBA kHotColor(255, 220, 60);

// ---------------------------------------------------------------------------------------------

class CNavigationDemo : public IEventListener
{
public:
	CNavigationDemo();
	~CNavigationDemo();

	void renderOneFrame();
	void run();

private:
	virtual void operator()(const CEvent &event) NL_OVERRIDE;

	// --- camera / projection helpers
	CMatrix camWorld() const { return m_Nav.getViewMatrix(); }
	/** World point -> viewport space (0..1, y up). Returns false when behind the camera. */
	bool project(const CVector &world, float &sx, float &sy) const;
	void ray(float x, float y, CVector &pos, CVector &dir) const;
	/** World units per gizmo unit, so the gizmo keeps a constant pixel size. */
	float gizmoScale() const;
	float fitGizmoScale() const;
	void updateGizmoFit();

	// --- picking
	int pickCube(float x, float y) const;
	TGizmoHandle pickGizmo(float x, float y) const;

	// --- gizmo drag
	/** Where a drag on `handle` currently points, in world space. */
	bool dragPoint(TGizmoHandle handle, float x, float y, CVector &out) const;
	void beginDrag(TGizmoHandle handle, float x, float y);
	void updateDrag(float x, float y);

	// --- draw
	void drawScene();
	void drawGizmo();
	void drawArrow(const CVector &origin, const CVector &axis, CRGBA col);
	void frameSelection();

	UDriver *m_Driver;
	UMaterial m_Mat;
// The gizmo draws over the scene: no depth test anywhere. What little
// self-occlusion it needs comes from culling and draw order - see drawGizmo.
UMaterial m_GizmoMat; // outline and plane handles: blended, both sides visible
UMaterial m_GizmoSolidMat; // shafts and cones: back faces culled
	CFrustum m_Frustum;
	CViewport m_Viewport;
	CNavMouseListener m_Nav;

	vector<SCube> m_Cubes;
	int m_Selected;

	TGizmoHandle m_Hover;
	TGizmoHandle m_Drag;
	CVector m_DragGrab; // world point the drag started from
	CVector m_DragOrigin; // cube position when the drag started

	// Deferred gizmo fit - see updateGizmoFit.
	float m_GizmoScale;
	bool m_GizmoFitDirty;
	uint32 m_FitSerial;
	uint32 m_FitW, m_FitH;
	float m_MouseX, m_MouseY;
	bool m_LeftDown;
	bool m_CloseWindow;
};

// ---------------------------------------------------------------------------------------------

CNavigationDemo::CNavigationDemo()
	: m_Driver(NULL), m_Selected(-1), m_Hover(GizmoNone), m_Drag(GizmoNone),
	  m_GizmoScale(1.f), m_GizmoFitDirty(true), m_FitSerial(0), m_FitW(0), m_FitH(0),
	  m_MouseX(0.5f), m_MouseY(0.5f), m_LeftDown(false), m_CloseWindow(false)
{
	// Emscripten has to be told which driver to take; the default resolution finds nothing
	// there and createDriver returns a shell whose _Driver asserts on first use.
#ifdef __EMSCRIPTEN__
	m_Driver = UDriver::createDriver(0, UDriver::OpenGlEs3);
#else
	m_Driver = UDriver::createDriver();
#endif
	if (!m_Driver)
	{
		nlerror("Failed to create driver");
		return;
	}
	m_Driver->setDisplay(UDriver::CMode(1024, 768, 32, true));
	m_Driver->setWindowTitle(ucstring("NeL navigation + move gizmo"));

	m_Mat = m_Driver->createMaterial();
	m_Mat.initUnlit();
	m_Mat.setZWrite(true);
	m_Mat.setZFunc(UMaterial::lessequal);
	m_Mat.setDoubleSided(true);

	// The gizmo sits at the object's centre, so with a depth test it would be buried inside
// the very thing it manipulates. Drawn over everything. The selection
// outline needs the same treatment for the same reason - it wraps a cube it must remain
// visible through.
	m_GizmoMat = m_Driver->createMaterial();
	m_GizmoMat.initUnlit();
	m_GizmoMat.setZWrite(false);
	m_GizmoMat.setZFunc(UMaterial::always);
	m_GizmoMat.setDoubleSided(true);
	m_GizmoMat.setBlend(true);
	m_GizmoMat.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);

	// Shafts and cones, same no-depth policy, but with back faces culled. That is the whole
	// fix for a cone fighting itself: a cone is convex, so once its far wall and its cap are
	// culled, exactly one surface covers each pixel and there is nothing left to order.
	m_GizmoSolidMat = m_Driver->createMaterial();
	m_GizmoSolidMat.initUnlit();
	m_GizmoSolidMat.setZWrite(false);
	m_GizmoSolidMat.setZFunc(UMaterial::always);
	m_GizmoSolidMat.setDoubleSided(false);

	m_Frustum.initPerspective(60.f * (float)Pi / 180.f, 4.f / 3.f, 0.1f, 1000.f);

	// A 4x4 field with varied heights, so axis moves have something to read against.
	static const CRGBA kPalette[4] = {
		CRGBA(200, 120, 90), CRGBA(120, 170, 200),
		CRGBA(160, 200, 120), CRGBA(190, 160, 210)
	};
	for (int y = 0; y < 4; ++y)
	for (int x = 0; x < 4; ++x)
	{
		const float h = ((x + y) & 1) ? 0.4f : 0.f;
		m_Cubes.push_back(SCube(CVector((x - 1.5f) * 2.f, (y - 1.5f) * 2.f, h),
		                        kPalette[(x + y * 4) & 3]));
	}

	// Three-quarter view of the field, and hand the nav listener a matching target so the
	// very first orbit pivots about what you are looking at.
	const CVector center(0.f, 0.f, 0.f);
	const CVector dir = CVector(-0.55f, -0.65f, 0.55f).normed();
	const float dist = 12.f;
	const CVector eye = center + dir * dist;
	const CVector jj = (center - eye).normed();
	const CVector ii = (jj ^ CVector::K).normed();
	const CVector kk = ii ^ jj;
	CMatrix cam;
	cam.identity();
	cam.setRot(ii, jj, kk, true);
	cam.setPos(eye);

	m_Nav.setMatrix(cam);
	m_Nav.setFrustrum(m_Frustum);
	m_Nav.setViewport(m_Viewport);
	m_Nav.setTarget(center);
	m_Nav.addToServer(m_Driver->EventServer);

	m_Driver->EventServer.addListener(EventCloseWindowId, this);
	m_Driver->EventServer.addListener(EventMouseDownId, this);
	m_Driver->EventServer.addListener(EventMouseUpId, this);
	m_Driver->EventServer.addListener(EventMouseMoveId, this);
	m_Driver->EventServer.addListener(EventKeyDownId, this);
}

CNavigationDemo::~CNavigationDemo()
{
	if (!m_Driver)
		return;
	m_Nav.removeFromServer(m_Driver->EventServer);
	m_Driver->EventServer.removeListener(EventCloseWindowId, this);
	m_Driver->EventServer.removeListener(EventMouseDownId, this);
	m_Driver->EventServer.removeListener(EventMouseUpId, this);
	m_Driver->EventServer.removeListener(EventMouseMoveId, this);
	m_Driver->EventServer.removeListener(EventKeyDownId, this);
	m_Driver->deleteMaterial(m_Mat);
	m_Driver->deleteMaterial(m_GizmoMat);
	m_Driver->deleteMaterial(m_GizmoSolidMat);
	m_Driver->release();
	delete m_Driver;
}

// ---------------------------------------------------------------------------------------------
// Camera helpers

void CNavigationDemo::ray(float x, float y, CVector &pos, CVector &dir) const
{
	m_Viewport.getRayWithPoint(x, y, pos, dir, camWorld(), m_Frustum);
}

bool CNavigationDemo::project(const CVector &world, float &sx, float &sy) const
{
	CMatrix view = camWorld();
	view.invert();
	const CVector eye = view * world;
	if (eye.y <= m_Frustum.Near * 0.5f)
		return false; // behind (or on) the camera plane: no meaningful screen position
	const CVector p = m_Frustum.project(eye);
	sx = p.x;
	sy = p.y;
	return true;
}

// Ratio between a physical (CSS) pixel and a backing-store pixel. Everything the driver
// reports is in backing pixels, so any size meant to feel the same across devices has to
// go through here.
static float uiScale()
{
#ifdef __EMSCRIPTEN__
	const double dpr = emscripten_get_device_pixel_ratio();
	return dpr > 0.0 ? (float)dpr : 1.f;
#else
	return 1.f;
#endif
}

// World size the gizmo would need RIGHT NOW to stand kGizmoPixels tall on screen. Only
// updateGizmoFit calls this, and only when the user is between interactions.
float CNavigationDemo::fitGizmoScale() const
{
	if (m_Selected < 0)
		return m_GizmoScale;
	uint32 w = 0, h = 0;
	m_Driver->getWindowSize(w, h);
	if (!h)
		return m_GizmoScale;
	// World size of one pixel at the gizmo's depth, from the frustum's vertical extent.
	const CVector camPos = camWorld().getPos();
	const float depth = (m_Cubes[m_Selected].Pos - camPos) * camWorld().getJ();
	const float d = depth > m_Frustum.Near ? depth : m_Frustum.Near;
	const float worldPerPixelAtNear = (m_Frustum.Top - m_Frustum.Bottom) / (float)h;
	// kGizmoPixels is a physical size, so it goes through the device pixel ratio: the canvas
	// backing store already runs at CSS x DPR, and a constant backing-pixel count would be a
	// gizmo that shrinks as density rises - a third of its size on a phone.
	return kGizmoPixels * uiScale() * worldPerPixelAtNear * d / m_Frustum.Near;
}

/*
 * The chosen behaviour, which is neither of the two obvious options.
 *
 * A gizmo re-fitted every frame holds a constant screen size, but then it grows relative to
 * its object as the object recedes - so dragging along an axis reads as the object shrinking
 * out from under a manipulator that stays put. A gizmo of fixed world size never does that,
 * but goes unusably small the moment you pull the camera back.
 *
 * So: fit to the screen, but only BETWEEN interactions. During a drag or a view move the
 * world size is frozen, and the gizmo behaves like part of the scene. When the interaction
 * ends it re-fits, once. And because a re-fit mid-navigation would be visible as a jump, the
 * gizmo simply isn't drawn while the view is moving - it comes back correctly sized.
 */
void CNavigationDemo::updateGizmoFit()
{
	uint32 w = 0, h = 0;
	m_Driver->getWindowSize(w, h);
	if (w != m_FitW || h != m_FitH)
	{
		m_FitW = w;
		m_FitH = h;
		m_GizmoFitDirty = true;
	}
	const uint32 serial = m_Nav.viewSerial();
	if (serial != m_FitSerial)
	{
		m_FitSerial = serial;
		m_GizmoFitDirty = true;
	}
	if (!m_GizmoFitDirty)
		return;
	// Hold the stale scale until the user's hands are off: mid-drag is exactly when a resize
	// would be felt.
	if (m_Nav.isNavigating() || m_Drag != GizmoNone)
		return;
	m_GizmoScale = fitGizmoScale();
	m_GizmoFitDirty = false;
}

float CNavigationDemo::gizmoScale() const
{
return m_GizmoScale;
}

// ---------------------------------------------------------------------------------------------
// Picking

int CNavigationDemo::pickCube(float x, float y) const
{
	CVector pos, dir;
	ray(x, y, pos, dir);
	dir.normalize();
	int best = -1;
	float bestDist = 1e30f;
	for (size_t i = 0; i < m_Cubes.size(); ++i)
	{
		CAABBox box;
		box.setCenter(m_Cubes[i].Pos);
		box.setHalfSize(CVector(kCubeHalf, kCubeHalf, kCubeHalf));
		if (!box.intersect(pos, pos + dir * 1000.f))
			continue;
		const float d = (m_Cubes[i].Pos - pos).norm();
		if (d < bestDist)
		{
			bestDist = d;
			best = (int)i;
		}
	}
	return best;
}

/** Distance from p to segment ab, all in viewport space. */
static float segDist(float px, float py, float ax, float ay, float bx, float by)
{
	const float vx = bx - ax, vy = by - ay;
	const float len2 = vx * vx + vy * vy;
	float t = 0.f;
	if (len2 > 1e-12f)
	{
		t = ((px - ax) * vx + (py - ay) * vy) / len2;
		t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
	}
	const float dx = px - (ax + vx * t), dy = py - (ay + vy * t);
	return sqrtf(dx * dx + dy * dy);
}

/** Point in the projected quad, by the same-side test on all four edges. */
static bool quadContains(float px, float py, const float qx[4], const float qy[4])
{
	int sign = 0;
	for (int i = 0; i < 4; ++i)
	{
		const int j = (i + 1) & 3;
		const float cross = (qx[j] - qx[i]) * (py - qy[i]) - (qy[j] - qy[i]) * (px - qx[i]);
		const int s = cross > 0.f ? 1 : (cross < 0.f ? -1 : 0);
		if (!s)
			continue;
		if (!sign)
			sign = s;
		else if (s != sign)
			return false;
	}
	return true;
}

TGizmoHandle CNavigationDemo::pickGizmo(float x, float y) const
{
	if (m_Selected < 0)
		return GizmoNone;
	const CVector o = m_Cubes[m_Selected].Pos;
	const float s = gizmoScale();

	float ox, oy;
	if (!project(o, ox, oy))
		return GizmoNone;

	// Pick distances are measured in units of viewport HEIGHT, so stretching x by the
	// aspect ratio first. Normalized coordinates are per-axis, so an untouched radius is an
	// ellipse on screen - barely noticeable on a 4:3 window, but on a portrait phone it
	// becomes a tall slot a thumb keeps missing sideways. Containment tests below need no
	// such correction: point-in-quad survives any affine squash.
	uint32 vw = 0, vh = 0;
	m_Driver->getWindowSize(vw, vh);
	const float ar = (vw && vh) ? (float)vw / (float)vh : 1.f;

	// Pick tolerances stay in screen space, because fingers and cursors are screen-space
	// things - but a world-sized gizmo can project down to a few pixels when the camera is
	// far away, and a fixed tolerance would then cover the whole thing and make every hit
	// the centre handle. Cap each tolerance against the gizmo's own projected extent so the
	// proportions hold at any distance; at normal working distances the caps never bind and
	// the constants apply as-is.
	float ex, ey;
	float ext = 0.f;
	for (int a = 0; a < 3; ++a)
	{
		if (!project(o + axisVec(a) * (kAxisLen * s), ex, ey))
			continue;
		const float d = segDist(ex * ar, ey, ox * ar, oy, ox * ar, oy);
		if (d > ext)
			ext = d;
	}
	const float screenR = std::min(kScreenPickRadius, 0.30f * ext);
	const float axisR = std::min(kAxisPickRadius, 0.25f * ext);

	// Centre first, then planes, then axes: smaller and more specific targets win, which
	// is also the order Max resolves overlaps in. The centre is invisible - see
	// kScreenPickRadius - so keep its target generous enough to find by feel. It is tested
	// FIRST, ahead of the planes whose inner corners it sits on top of: the very middle of
	// the gizmo means all axes at once, regardless of which plane's corner is under it.
	if (segDist(x * ar, y, ox * ar, oy, ox * ar, oy) < screenR)
		return GizmoScreen;

	static const int planeAxes[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };
	for (int p = 0; p < 3; ++p)
	{
		const CVector a = axisVec(planeAxes[p][0]);
		const CVector b = axisVec(planeAxes[p][1]);
		const CVector c0 = o + (a + b) * (kPlaneOff * s);
		const CVector c1 = c0 + a * (kPlaneSize * s);
		const CVector c2 = c0 + (a + b) * (kPlaneSize * s);
		const CVector c3 = c0 + b * (kPlaneSize * s);
		float qx[4], qy[4];
		if (!project(c0, qx[0], qy[0]) || !project(c1, qx[1], qy[1])
		    || !project(c2, qx[2], qy[2]) || !project(c3, qx[3], qy[3]))
			continue;
		if (quadContains(x, y, qx, qy))
			return (TGizmoHandle)(GizmoPlaneXY + p);
	}

	TGizmoHandle best = GizmoNone;
	float bestDist = axisR;
	for (int a = 0; a < 3; ++a)
	{
		const CVector axis = axisVec(a);
		float tx, ty;
		if (!project(o + axis * (kAxisLen * s), tx, ty))
			continue;
		const float d = segDist(x * ar, y, ox * ar, oy, tx * ar, ty);
		if (d < bestDist)
		{
			bestDist = d;
			best = (TGizmoHandle)(GizmoAxisX + a);
		}
	}
	return best;
}

// ---------------------------------------------------------------------------------------------
// Gizmo drag
//
// Axis drags resolve against the plane that contains the axis and faces the camera most
// squarely, then project onto the axis - the standard construction, and the one that stays
// stable when you look nearly straight down an axis.

bool CNavigationDemo::dragPoint(TGizmoHandle handle, float x, float y, CVector &out) const
{
	if (m_Selected < 0 || handle == GizmoNone)
		return false;
	const CVector o = m_DragOrigin;
	CVector pos, dir;
	ray(x, y, pos, dir);
	dir.normalize();

	if (handle == GizmoScreen)
	{
		CPlane pl;
		pl.make(camWorld().getJ(), o);
		out = pl.intersect(pos, pos + dir);
		return true;
	}
	if (handle >= GizmoPlaneXY && handle <= GizmoPlaneZX)
	{
		static const int planeNormal[3] = { 2, 0, 1 }; // XY->Z, YZ->X, ZX->Y
		const CVector n = axisVec(planeNormal[handle - GizmoPlaneXY]);
		if (fabsf(n * dir) < 1e-4f)
			return false; // ray parallel to the plane
		CPlane pl;
		pl.make(n, o);
		out = pl.intersect(pos, pos + dir);
		return true;
	}

	// Axis
	const CVector axis = axisVec(handle - GizmoAxisX);
	// Of the two planes containing the axis, take the one whose normal the view direction
	// meets least obliquely.
	const CVector viewDir = camWorld().getJ();
	CVector n1 = axis ^ viewDir;
	if (n1.norm() < 1e-4f)
		return false; // looking straight down the axis: nothing to resolve against
	n1.normalize();
	CVector n = axis ^ n1;
	n.normalize();
	if (fabsf(n * dir) < 1e-4f)
		return false;
	CPlane pl;
	pl.make(n, o);
	const CVector hit = pl.intersect(pos, pos + dir);
	out = o + axis * ((hit - o) * axis);
	return true;
}

void CNavigationDemo::beginDrag(TGizmoHandle handle, float x, float y)
{
	m_DragOrigin = m_Cubes[m_Selected].Pos;
	CVector grab;
	if (!dragPoint(handle, x, y, grab))
		return;
	m_Drag = handle;
	m_DragGrab = grab;
}

void CNavigationDemo::updateDrag(float x, float y)
{
	if (m_Drag == GizmoNone || m_Selected < 0)
		return;
	CVector now;
	if (!dragPoint(m_Drag, x, y, now))
		return;
	m_Cubes[m_Selected].Pos = m_DragOrigin + (now - m_DragGrab);
}

// ---------------------------------------------------------------------------------------------
// Events

void CNavigationDemo::operator()(const CEvent &event)
{
	if (event == EventCloseWindowId)
	{
		m_CloseWindow = true;
		return;
	}
	if (event == EventKeyDownId)
	{
		const CEventKeyDown *k = (const CEventKeyDown *)&event;
		if (k->Key == KeyZ)
			frameSelection();
		else if (k->Key == KeyESCAPE)
			m_Selected = -1;
		return;
	}

	const CEventMouse *m = (const CEventMouse *)&event;
	if (event == EventMouseMoveId)
	{
		m_MouseX = m->X;
		m_MouseY = m->Y;
		if (m_LeftDown && m_Drag != GizmoNone)
			updateDrag(m_MouseX, m_MouseY);
		else if (!m_LeftDown)
			m_Hover = pickGizmo(m_MouseX, m_MouseY);
		return;
	}
	if (event == EventMouseDownId)
	{
		m_MouseX = m->X;
		m_MouseY = m->Y;
		// Left ONLY: every modified left combination stays free for selection semantics,
		// and navigation never touches this button.
		if (m->Button != leftButton)
			return;
		m_LeftDown = true;
		const TGizmoHandle h = pickGizmo(m_MouseX, m_MouseY);
		if (h != GizmoNone)
		{
			beginDrag(h, m_MouseX, m_MouseY);
			return;
		}
		m_Selected = pickCube(m_MouseX, m_MouseY);
		m_GizmoFitDirty = true; // new object, new distance
		m_Hover = GizmoNone;
		return;
	}
	if (event == EventMouseUpId)
	{
		if (m->Button & leftButton)
		{
			m_LeftDown = false;
			m_Drag = GizmoNone;
			m_GizmoFitDirty = true; // the object moved; re-fit now that the hands are off
		}
		return;
	}
}

void CNavigationDemo::frameSelection()
{
	CAABBox box;
	if (m_Selected >= 0)
	{
		box.setCenter(m_Cubes[m_Selected].Pos);
		box.setHalfSize(CVector(kCubeHalf, kCubeHalf, kCubeHalf));
	}
	else
	{
		if (m_Cubes.empty())
			return;
		box.setCenter(m_Cubes[0].Pos);
		box.setHalfSize(CVector(kCubeHalf, kCubeHalf, kCubeHalf));
		for (size_t i = 1; i < m_Cubes.size(); ++i)
		{
			box.extend(m_Cubes[i].Pos - CVector(kCubeHalf, kCubeHalf, kCubeHalf));
			box.extend(m_Cubes[i].Pos + CVector(kCubeHalf, kCubeHalf, kCubeHalf));
		}
	}
	m_Nav.frameBox(box);
}

// ---------------------------------------------------------------------------------------------
// Draw

static void drawBox(UDriver *drv, UMaterial &mat, const CVector &c, float h, CRGBA col)
{
	static const int faces[6][4] = {
		{ 0, 1, 3, 2 }, { 4, 6, 7, 5 }, { 0, 4, 5, 1 },
		{ 2, 3, 7, 6 }, { 0, 2, 6, 4 }, { 1, 5, 7, 3 }
	};
	// Flat per-face shading so the cubes read as solids without a light setup.
	static const uint8 shade[6] = { 255, 150, 200, 175, 225, 190 };
	CVector v[8];
	for (int i = 0; i < 8; ++i)
		v[i] = c + CVector((i & 1) ? h : -h, (i & 2) ? h : -h, (i & 4) ? h : -h);
	for (int f = 0; f < 6; ++f)
	{
		CQuadColor q;
		q.V0 = v[faces[f][0]];
		q.V1 = v[faces[f][1]];
		q.V2 = v[faces[f][2]];
		q.V3 = v[faces[f][3]];
		CRGBA fc((uint8)(col.R * shade[f] / 255), (uint8)(col.G * shade[f] / 255),
		         (uint8)(col.B * shade[f] / 255), col.A);
		q.Color0 = q.Color1 = q.Color2 = q.Color3 = fc;
		drv->drawQuad(q, mat);
	}
}

void CNavigationDemo::drawScene()
{
	// Ground grid, so panning and orbiting have a frame of reference.
	const float g = 8.f;
	for (int i = -8; i <= 8; ++i)
	{
		const float t = (float)i;
		const CRGBA c = (i == 0) ? CRGBA(110, 110, 120) : CRGBA(70, 70, 78);
		CLineColor lx;
		lx.V0 = CVector(-g, t, -kCubeHalf);
		lx.V1 = CVector(g, t, -kCubeHalf);
		lx.Color0 = lx.Color1 = c;
		m_Driver->drawLine(lx, m_Mat);
		CLineColor ly;
		ly.V0 = CVector(t, -g, -kCubeHalf);
		ly.V1 = CVector(t, g, -kCubeHalf);
		ly.Color0 = ly.Color1 = c;
		m_Driver->drawLine(ly, m_Mat);
	}

	for (size_t i = 0; i < m_Cubes.size(); ++i)
		drawBox(m_Driver, m_Mat, m_Cubes[i].Pos, kCubeHalf, m_Cubes[i].Color);

	// Selection outline: a wire box slightly proud of the solid.
	if (m_Selected >= 0)
	{
		const CVector c = m_Cubes[m_Selected].Pos;
		const float h = kCubeHalf * 1.04f;
		CVector v[8];
		for (int i = 0; i < 8; ++i)
			v[i] = c + CVector((i & 1) ? h : -h, (i & 2) ? h : -h, (i & 4) ? h : -h);
		static const int edges[12][2] = {
			{ 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 0, 2 }, { 1, 3 },
			{ 4, 6 }, { 5, 7 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
		};
		for (int e = 0; e < 12; ++e)
		{
			CLineColor l;
			l.V0 = v[edges[e][0]];
			l.V1 = v[edges[e][1]];
			l.Color0 = l.Color1 = kHotColor;
			m_Driver->drawLine(l, m_GizmoMat);
		}
	}
}

void CNavigationDemo::drawArrow(const CVector &origin, const CVector &axis, CRGBA col)
{
	const CVector tip = origin + axis;
	CLineColor shaft;
	shaft.V0 = origin;
	shaft.V1 = tip;
	shaft.Color0 = shaft.Color1 = col;
	m_Driver->drawLine(shaft, m_GizmoSolidMat);


	// Solid cone arrowhead. Built against the axis rather than against the view, so it
	// reads the same from any angle, and capped so it stays solid when seen from behind.
	CVector n = axis;
	n.normalize();
	CVector u = n ^ CVector::K;
	if (u.norm() < 1e-3f)
		u = n ^ CVector::I;
	u.normalize();
	const CVector v = n ^ u;
	const float len = axis.norm();
	const float r = kArrowWide * len;
	const CVector base = tip - n * (kArrowLen * len);
	const CRGBA capCol((uint8)(col.R * kCapShade / 255), (uint8)(col.G * kCapShade / 255),
	                   (uint8)(col.B * kCapShade / 255), col.A);

	CVector rim[kConeSegments];
	for (int i = 0; i < kConeSegments; ++i)
	{
		const float a = 2.f * (float)Pi * (float)i / (float)kConeSegments;
		rim[i] = base + u * (cosf(a) * r) + v * (sinf(a) * r);
	}
	for (int i = 0; i < kConeSegments; ++i)
	{
		const int j = (i + 1) % kConeSegments;
		CTriangleColor side;
		side.V0 = tip;
		side.V1 = rim[i];
		side.V2 = rim[j];
		side.Color0 = side.Color1 = side.Color2 = col;
		m_Driver->drawTriangle(side, m_GizmoSolidMat);

		// Darker underside, so the cone reads as a solid with a lit side and a shaded
		// cap rather than as a flat silhouette when seen from below.
		CTriangleColor cap;
		cap.V0 = base;
		cap.V1 = rim[j];
		cap.V2 = rim[i];
		cap.Color0 = cap.Color1 = cap.Color2 = capCol;
		m_Driver->drawTriangle(cap, m_GizmoSolidMat);
	}
}

// Draw order for a three-piece set, farthest first. The gizmo's pieces are small, convex
// and mostly disjoint, so a single key each is enough; the cases a per-piece key cannot
// express (a shaft whose depth range straddles a cone's) are a few pixels at glancing
// angles, and not worth a depth buffer to fix.
static void depthOrder(const float key[3], int order[3])
{
	order[0] = 0;
	order[1] = 1;
	order[2] = 2;
	for (int i = 0; i < 2; ++i)
		for (int j = i + 1; j < 3; ++j)
			if (key[order[j]] > key[order[i]])
			{
				const int t = order[i];
				order[i] = order[j];
				order[j] = t;
			}
}

void CNavigationDemo::drawGizmo()
{
	if (m_Selected < 0)
		return;
	// Not drawn while the view is moving: the scale it would be drawn at is stale by
	// definition, and hiding it is both cleaner and cheaper than showing a wrong size.
	if (m_Nav.isNavigating())
		return;
	const CVector o = m_Cubes[m_Selected].Pos;
	const float s = gizmoScale();
	const TGizmoHandle hot = (m_Drag != GizmoNone) ? m_Drag : m_Hover;

	// View depth of each axis' far end and each plane's centre, for the painter order below.
	const CVector camPos = camWorld().getPos();
	const CVector fwd = camWorld().getJ();
	float axisKey[3], planeKey[3];
	static const int planeAxes[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };
	for (int i = 0; i < 3; ++i)
	{
		axisKey[i] = (o + axisVec(i) * (kAxisLen * s) - camPos) * fwd;
		const CVector pc = axisVec(planeAxes[i][0]) + axisVec(planeAxes[i][1]);
		planeKey[i] = (o + pc * (kPlaneOff * s + kPlaneSize * s * 0.5f) - camPos) * fwd;
	}
	int axisOrder[3], planeOrder[3];
	depthOrder(axisKey, axisOrder);
	depthOrder(planeKey, planeOrder);

	// Plane handles first: they sit under the axes and should not overdraw them. Among
	// themselves they are blended and meet at the origin, so back-to-front is the best
	// available - they interpenetrate, which no ordering fully resolves.
	for (int pi = 0; pi < 3; ++pi)
	{
		const int p = planeOrder[pi];
		const CVector a = axisVec(planeAxes[p][0]);
		const CVector b = axisVec(planeAxes[p][1]);
		const CVector c0 = o + (a + b) * (kPlaneOff * s);
		CQuadColor q;
		q.V0 = c0;
		q.V1 = c0 + a * (kPlaneSize * s);
		q.V2 = c0 + (a + b) * (kPlaneSize * s);
		q.V3 = c0 + b * (kPlaneSize * s);
		// The screen handle owns no shape of its own: it lights ALL THREE planes, which
		// reads as "the whole plane set is live" - which is exactly what a view-parallel
		// move is.
		const bool isHot = (hot == GizmoPlaneXY + p) || (hot == GizmoScreen);
		CRGBA c = isHot ? kHotColor : kAxisColor[planeAxes[p][0]];
		c.A = isHot ? 190 : 110;
		q.Color0 = q.Color1 = q.Color2 = q.Color3 = c;
		m_Driver->drawQuad(q, m_GizmoMat);
	}

	// Arrows back to front, so an arrow in front of another axis' shaft wins the overlap.
	for (int ai = 0; ai < 3; ++ai)
	{
		const int a = axisOrder[ai];
		drawArrow(o, axisVec(a) * (kAxisLen * s),
		          (hot == GizmoAxisX + a) ? kHotColor : kAxisColor[a]);
	}

}

void CNavigationDemo::renderOneFrame()
{
	if (!m_Driver->isFrameReady())
		return; // GPU busy: skip rather than block the browser event loop

	m_Driver->EventServer.pump();

	m_Driver->clearBuffers(CRGBA(48, 50, 56));

	uint32 w = 0, h = 0;
	m_Driver->getWindowSize(w, h);
	if (w && h)
		m_Frustum.initPerspective(60.f * (float)Pi / 180.f, (float)w / (float)h, 0.1f, 1000.f);
	m_Nav.setFrustrum(m_Frustum);

	// After the frustum is current (fitGizmoScale reads it) and before anything picks or
	// draws with the scale.
	updateGizmoFit();

	CMatrix view = camWorld();
	view.invert();
	m_Driver->setFrustum(m_Frustum);
	m_Driver->setViewMatrix(view);
	CMatrix model;
	model.identity();
	m_Driver->setModelMatrix(model);

	drawScene();
	drawGizmo();

	m_Driver->swapBuffers();
}

void CNavigationDemo::run()
{
#ifndef __EMSCRIPTEN__
	while (m_Driver->isActive() && !m_CloseWindow)
		renderOneFrame();
#endif
}

#ifdef __EMSCRIPTEN__
static CNavigationDemo *s_Demo = NULL;

static void emscriptenMainLoop()
{
	if (s_Demo)
		s_Demo->renderOneFrame();
}
#endif

#ifdef NL_OS_WINDOWS
sint WINAPI WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR /* cmdline */, int /* nCmdShow */)
#else
sint main(int /* argc */, char ** /* argv */)
#endif
{
	CApplicationContext applicationContext;

#ifdef __EMSCRIPTEN__
	static CNavigationDemo demo;
	s_Demo = &demo;
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
	CNavigationDemo demo;
	demo.run();
#endif

	return EXIT_SUCCESS;
}
