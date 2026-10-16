// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2025  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//
// Planar Reflection Demo
//
// Demonstrates HL2-era planar reflections on an arbitrary polygon.
// A spinning colored cube floats above a hexagonal reflective floor at Z=0,
// with a rolling checkerboard skybox for visual context.
//
// Technique:
//   1. Mirror the camera across the reflection plane (negate eye.z and target.z,
//      keep up=(0,0,1)) to create a reflected camera.
//   2. Compute the screen-space AABB of the floor polygon as seen by the
//      reflected camera. Add a configurable margin (for wobble in real water)
//      and snap the dimensions to a pixel grid (rtSnap=32) to avoid RT churn.
//   3. Build an off-center sub-frustum covering only the AABB region.
//   4. Compute the "active" RT dimensions from the snapped AABB (what dynamic
//      mode would allocate). In fixed mode, a larger RT is allocated once at
//      window size + margin, but a viewport and scissor restrict rendering to
//      the active sub-region. UVs are scaled by (activeSize / allocSize) to
//      address the correct sub-region of the texture.
//   5. Render the scene from the reflected camera into the RT, with a clip
//      plane at Z=0 to discard geometry below the reflection surface.
//   6. Render the scene normally from the real camera.
//   7. Tessellate the floor polygon using a screen-space grid (matching the NeL
//      water renderer): project the polygon to grid coordinates, rasterize with
//      computeInnerBorders/computeOuterBorders, clip border cells against the
//      polygon edges using Sutherland-Hodgman (CPolygon::clip). For each cell,
//      un-project grid vertices to Z=0 via ray-plane intersection, then compute
//      reflection UVs by projecting through the reflected view matrix and
//      sub-frustum (CFrustum::project), scaled by the UV scale factor.
//      Draw as batched CQuadColorUV quads.
//   8. Detach texture from material, recycle RT, swap buffers.
//
// Render target sizing:
//   Two modes are available (W key toggles, default is fixed):
//
//   Fixed mode (default): The RT is allocated at window size plus a small
//   margin (rtMargin). This avoids per-frame reallocation stutter on GPUs
//   that are slow to create/destroy render targets. A viewport and scissor
//   restrict rendering to the active sub-region (same size as dynamic mode
//   would allocate), so fill rate and texel density match the actual water
//   coverage on screen. When the water is far away, only a small corner of
//   the fixed RT is used, saving fill rate compared to rendering into the
//   full allocation.
//
//   Dynamic mode: The RT is sized to the reflected camera's screen-space AABB
//   of the floor polygon. Dimensions are snapped up to multiples of rtSnap
//   (default 32 pixels) with equal padding on all sides, preventing churn as
//   the AABB shifts from frame to frame. This uses less VRAM but may cause
//   stutter when the snapped size changes.
//
//   Both modes use an off-center (asymmetric) frustum so that
//   CFrustum::project() maps floor vertices to [0,1] UVs within the viewport.
//   Half-resolution (H, default on) and power-of-two sizing (P, default on)
//   can be combined with either mode.
//
// Debug overlays:
//   T key cycles the render target overlay through three states:
//     Off -> Fullscreen (RT stretched to screen) -> Corner thumbnail (default)
//   The thumbnail shows the full RT allocation including unused regions,
//   so you can verify the viewport sub-region optimization.
//
//   B key toggles boundary lines:
//     Yellow quadrilateral - RT coverage projected onto the normal screen
//       (un-project RT corners from reflected camera to Z=0, re-project
//       through the normal camera)
//     Cyan rectangle - floor polygon's screen footprint (normal camera AABB)
//
// Controls:
//   F - Toggle wireframe rendering
//   C - Toggle camera orbit
//   R - Toggle cube rotation
//   S - Toggle skybox roll
//   T - Cycle render target overlay: off / fullscreen / corner thumbnail
//   B - Toggle render target boundary overlay
//   H - Toggle half-resolution reflection (default on)
//   P - Toggle power-of-two render target sizing (default on)
//   W - Toggle fixed window-sized render target (default on, avoids stutter)
//   V - Toggle VSync (default on)
//   Up/Down - Move camera closer/farther
//

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/geom_ext.h>
#include <nel/misc/path.h>
#include <nel/misc/plane.h>
#include <nel/misc/polygon.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector_2f.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>
#include <nel/3d/scissor.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/texture_user.h>
#include <nel/3d/render_target_manager.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
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

// Build a view matrix from eye position, target, and up vector
static CMatrix buildViewMatrix(const CVector &eye, const CVector &target, const CVector &up)
{
	CVector jj = (target - eye).normed();
	CVector ii = (jj ^ up).normed();
	CVector kk = ii ^ jj;
	CMatrix camWorld;
	camWorld.setRot(ii, jj, kk, true);
	camWorld.setPos(eye);
	CMatrix viewMatrix = camWorld;
	viewMatrix.invert();
	return viewMatrix;
}

// Un-project a screen-space grid coordinate to a world position on the Z=0 plane,
// and compute the reflection texture UV.
// Returns false if the ray doesn't hit the floor (pointing above horizon).
static bool gridToWorldAndUV(float gx, float gy, int gridN,
	const CFrustum &frustum, const CMatrix &camWorld, const CVector &eye,
	const CFrustum &reflFrustum, const CMatrix &reflectedViewMatrix,
	const CUV &uvScale,
	CVector &worldPos, CUV &uv)
{
	float sx = gx / float(gridN);
	float sy = gy / float(gridN);

	CVector nearLocal = frustum.unProject(CVector(sx, sy, 0.f));
	CVector worldNear = camWorld * nearLocal;
	CVector dir = worldNear - eye;

	if (dir.z >= -0.0001f)
		return false;

	float t = -eye.z / dir.z;
	worldPos.x = eye.x + t * dir.x;
	worldPos.y = eye.y + t * dir.y;
	worldPos.z = 0.f;

	CVector reflLocal = reflectedViewMatrix * worldPos;
	CVector proj = reflFrustum.project(reflLocal);
	uv = CUV(proj.x * uvScale.U, proj.y * uvScale.V);
	return true;
}

// Emit a clipped polygon as triangle-fan quads.
// The polygon vertices are in grid coordinates; each is un-projected to Z=0
// and given a reflection UV. Quads are appended to the output vector.
static void emitClippedPoly(const CPolygon &clipPoly, int gridN,
	const CFrustum &frustum, const CMatrix &camWorld, const CVector &eye,
	const CFrustum &reflFrustum, const CMatrix &reflectedViewMatrix,
	const CUV &uvScale, CRGBA color,
	std::vector<CQuadColorUV> &outQuads)
{
	uint nv = (uint)clipPoly.Vertices.size();
	if (nv < 3) return;

	// Pre-compute world positions and UVs for all vertices
	std::vector<CVector> wp(nv);
	std::vector<CUV> uv(nv);
	for (uint i = 0; i < nv; ++i)
	{
		if (!gridToWorldAndUV(clipPoly.Vertices[i].x, clipPoly.Vertices[i].y, gridN,
			frustum, camWorld, eye, reflFrustum, reflectedViewMatrix, uvScale, wp[i], uv[i]))
			return;
	}

	// Convert triangle fan to quads: each quad covers 2 fan triangles
	for (uint k = 0; k < (nv - 1) / 2; ++k)
	{
		uint i0 = 0;
		uint i1 = 2 * k + 1;
		uint i2 = 2 * k + 2;
		uint i3 = (2 * k + 3 < nv) ? 2 * k + 3 : nv - 1;

		CQuadColorUV q;
		q.V0 = wp[i0]; q.V1 = wp[i1]; q.V2 = wp[i2]; q.V3 = wp[i3];
		q.Color0 = q.Color1 = q.Color2 = q.Color3 = color;
		q.Uv0 = uv[i0]; q.Uv1 = uv[i1]; q.Uv2 = uv[i2]; q.Uv3 = uv[i3];
		outQuads.push_back(q);
	}
}

// Draw a checkerboard skybox that rolls forward (rotates around X axis).
// Only the odd cells are drawn on top of the clear color.
static void drawSkybox(UDriver *driver, UMaterial &mat, const CVector &center, float angle)
{
	const float s = 50.f;
	const int grid = 16;
	float cs = 2.f * s / float(grid);
	CRGBA color(50, 50, 55);

	// Roll the skybox forward around the X axis
	CMatrix rot;
	rot.identity();
	rot.rotateX(angle);

	// Each face defined by origin corner (relative to center), U-axis, V-axis
	struct Face { CVector org, u, v; };
	Face faces[6] = {
		{ CVector(-s, -s,  s), CVector(1, 0, 0), CVector(0, 1, 0) }, // +Z top
		{ CVector(-s, -s, -s), CVector(1, 0, 0), CVector(0, 1, 0) }, // -Z bottom
		{ CVector( s, -s, -s), CVector(0, 1, 0), CVector(0, 0, 1) }, // +X right
		{ CVector(-s, -s, -s), CVector(0, 1, 0), CVector(0, 0, 1) }, // -X left
		{ CVector(-s,  s, -s), CVector(1, 0, 0), CVector(0, 0, 1) }, // +Y front
		{ CVector(-s, -s, -s), CVector(1, 0, 0), CVector(0, 0, 1) }, // -Y back
	};

	CQuadColor q;
	q.Color0 = q.Color1 = q.Color2 = q.Color3 = color;

	for (int f = 0; f < 6; ++f)
	{
		CVector org = center + rot * faces[f].org;
		CVector du = rot * (faces[f].u * cs);
		CVector dv = rot * (faces[f].v * cs);

		for (int j = 0; j < grid; ++j)
		{
			for (int i = 0; i < grid; ++i)
			{
				if ((i + j) % 2 == 0) continue;
				q.V0 = org + du * float(i)     + dv * float(j);
				q.V1 = org + du * float(i + 1) + dv * float(j);
				q.V2 = org + du * float(i + 1) + dv * float(j + 1);
				q.V3 = org + du * float(i)     + dv * float(j + 1);
				driver->drawQuad(q, mat);
			}
		}
	}
}

// Draw a colored quad (one face of the cube)
static void drawFace(UDriver *driver, UMaterial &mat,
	const CVector &v0, const CVector &v1, const CVector &v2, const CVector &v3,
	CRGBA color)
{
	CQuadColor quad;
	quad.V0 = v0;
	quad.V1 = v1;
	quad.V2 = v2;
	quad.V3 = v3;
	quad.Color0 = quad.Color1 = quad.Color2 = quad.Color3 = color;
	driver->drawQuad(quad, mat);
}

// Draw a cube centered at origin with given half-size, transformed by matrix
static void drawCube(UDriver *driver, UMaterial &mat, const CMatrix &transform, float s)
{
	// 8 vertices of the cube in NeL coords (X right, Y forward, Z up)
	CVector v[8] = {
		transform * CVector(-s, -s, -s), // 0: left  back  bottom
		transform * CVector( s, -s, -s), // 1: right back  bottom
		transform * CVector( s,  s, -s), // 2: right front bottom
		transform * CVector(-s,  s, -s), // 3: left  front bottom
		transform * CVector(-s, -s,  s), // 4: left  back  top
		transform * CVector( s, -s,  s), // 5: right back  top
		transform * CVector( s,  s,  s), // 6: right front top
		transform * CVector(-s,  s,  s), // 7: left  front top
	};

	// 6 faces with distinct colors
	drawFace(driver, mat, v[3], v[2], v[1], v[0], CRGBA(200,  50,  50)); // bottom (Z-) red
	drawFace(driver, mat, v[4], v[5], v[6], v[7], CRGBA( 50, 200,  50)); // top    (Z+) green
	drawFace(driver, mat, v[0], v[1], v[5], v[4], CRGBA( 50,  50, 200)); // back   (Y-) blue
	drawFace(driver, mat, v[2], v[3], v[7], v[6], CRGBA(200, 200,  50)); // front  (Y+) yellow
	drawFace(driver, mat, v[3], v[0], v[4], v[7], CRGBA(200,  50, 200)); // left   (X-) magenta
	drawFace(driver, mat, v[1], v[2], v[6], v[5], CRGBA( 50, 200, 200)); // right  (X+) cyan
}

// Find the project root by walking up from the current working directory
// looking for a ".nel" folder, then return the font path under graphics/.
static std::string findFontPath()
{
	std::string rootPath = CPath::standardizePath(CPath::getCurrentPath(), false);

	while (!rootPath.empty())
	{
		if (CFile::isDirectory(rootPath + "/.nel"))
		{
			std::string fontPath = rootPath + "/graphics/fonts/n019003l.pfb";
			if (CFile::fileExists(fontPath))
				return fontPath;
		}
		std::string::size_type sep = CFile::getLastSeparator(rootPath);
		if (sep == string::npos)
			break;
		rootPath = rootPath.substr(0, sep);
	}
	return std::string();
}

// Planar reflection demo application
class CPlanarReflectionDemo : public IEventListener
{
public:
	CPlanarReflectionDemo();
	~CPlanarReflectionDemo();
	void run();
	void renderOneFrame();

	virtual void operator()(const CEvent &event) NL_OVERRIDE;

private:
	bool m_CloseWindow;
	bool m_Wireframe;
	bool m_AnimCamera;
	bool m_AnimCube;
	bool m_AnimSkybox;
	bool m_ShowBounds;
	int m_ShowRT; // 0=off, 1=fullscreen, 2=corner thumbnail
	bool m_HalfRes;
	bool m_Pow2;
	bool m_FixedRT;
	bool m_VSync;
	bool m_KeyForward;
	bool m_KeyBackward;
	UDriver *m_Driver;
	UTextContext *m_TextContext;
	UMaterial m_SkyMat;
	UMaterial m_CubeMat;
	UMaterial m_FloorMat;
	float m_CamAngle;
	float m_CubeAngleZ;
	float m_CubeAngleX;
	float m_SkyAngle;
	float m_CamDist;
	double m_LastTime;
	float m_SmoothFps;
};

CPlanarReflectionDemo::CPlanarReflectionDemo()
	: m_CloseWindow(false)
	, m_Wireframe(false)
	, m_AnimCamera(true)
	, m_AnimCube(true)
	, m_AnimSkybox(true)
	, m_ShowBounds(false)
	, m_ShowRT(2)
	, m_HalfRes(true)
	, m_Pow2(true)
	, m_FixedRT(true)
	, m_VSync(true)
	, m_KeyForward(false)
	, m_KeyBackward(false)
	, m_TextContext(NULL)
	, m_CamAngle(0.f)
	, m_CubeAngleZ(0.f)
	, m_CubeAngleX(0.f)
	, m_SkyAngle(0.f)
	, m_CamDist(8.f)
	, m_LastTime(0.0)
	, m_SmoothFps(60.f)
{
#ifdef __EMSCRIPTEN__
	m_Driver = UDriver::createDriver(0, UDriver::OpenGlEs3);
#else
	m_Driver = UDriver::createDriver(0, UDriver::OpenGl3);
#endif
	if (!m_Driver)
	{
		nlerror("Failed to create driver");
		return;
	}

	m_Driver->EventServer.addListener(EventCloseWindowId, this);
	m_Driver->EventServer.addListener(EventKeyDownId, this);
	m_Driver->EventServer.addListener(EventKeyUpId, this);

	m_Driver->setDisplay(UDriver::CMode(800, 600, 32, true));
	m_Driver->setWindowTitle(ucstring("NeL Planar Reflection Demo"));
	m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0);

	// Initialize text renderer (optional, font may not be found)
	std::string fontPath = findFontPath();
	if (!fontPath.empty())
	{
		m_TextContext = m_Driver->createTextContext(fontPath);
		if (m_TextContext)
			m_TextContext->setFontSize(12);
	}

	// Skybox material: no depth write, always passes depth test, double-sided
	m_SkyMat = m_Driver->createMaterial();
	m_SkyMat.initUnlit();
	m_SkyMat.setZWrite(false);
	m_SkyMat.setZFunc(UMaterial::always);
	m_SkyMat.setDoubleSided(true);

	// Opaque material for the cube
	m_CubeMat = m_Driver->createMaterial();
	m_CubeMat.initUnlit();
	m_CubeMat.setZWrite(true);
	m_CubeMat.setZFunc(UMaterial::lessequal);

	// Textured material for the reflective floor
	m_FloorMat = m_Driver->createMaterial();
	m_FloorMat.initUnlit();
	m_FloorMat.setZWrite(true);
	m_FloorMat.setZFunc(UMaterial::lessequal);
	m_FloorMat.setDoubleSided(true);
	m_FloorMat.setBlend(true);
	m_FloorMat.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);
}

CPlanarReflectionDemo::~CPlanarReflectionDemo()
{
	m_FloorMat.setTexture(0, NULL);
	if (m_TextContext)
		m_Driver->deleteTextContext(m_TextContext);
	m_Driver->deleteMaterial(m_FloorMat);
	m_Driver->deleteMaterial(m_CubeMat);
	m_Driver->deleteMaterial(m_SkyMat);
	m_Driver->release();
	delete m_Driver;
}

void CPlanarReflectionDemo::operator()(const CEvent &event)
{
	if (event == EventCloseWindowId)
	{
		m_CloseWindow = true;
	}
	else if (event == EventKeyDownId)
	{
		CEventKeyDown &keyDown = (CEventKeyDown &)event;
		if (keyDown.FirstTime)
		{
			if (keyDown.Key == KeyF) m_Wireframe = !m_Wireframe;
			if (keyDown.Key == KeyC) m_AnimCamera = !m_AnimCamera;
			if (keyDown.Key == KeyR) m_AnimCube = !m_AnimCube;
			if (keyDown.Key == KeyS) m_AnimSkybox = !m_AnimSkybox;
			if (keyDown.Key == KeyB) m_ShowBounds = !m_ShowBounds;
			if (keyDown.Key == KeyT) m_ShowRT = (m_ShowRT + 1) % 3;
			if (keyDown.Key == KeyH) m_HalfRes = !m_HalfRes;
			if (keyDown.Key == KeyP) m_Pow2 = !m_Pow2;
			if (keyDown.Key == KeyW) m_FixedRT = !m_FixedRT;
			if (keyDown.Key == KeyV) { m_VSync = !m_VSync; m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0); }
		}
		if (keyDown.Key == KeyUP) m_KeyForward = true;
		if (keyDown.Key == KeyDOWN) m_KeyBackward = true;
	}
	else if (event == EventKeyUpId)
	{
		CEventKeyUp &keyUp = (CEventKeyUp &)event;
		if (keyUp.Key == KeyUP) m_KeyForward = false;
		if (keyUp.Key == KeyDOWN) m_KeyBackward = false;
	}
}

void CPlanarReflectionDemo::run()
{
	m_LastTime = CTime::ticksToSecond(CTime::getPerformanceTime());

#ifndef __EMSCRIPTEN__
	while (m_Driver->isActive() && !m_CloseWindow)
	{
		renderOneFrame();
	}
#endif
}

void CPlanarReflectionDemo::renderOneFrame()
{
	if (!m_Driver->isFrameReady())
		return; // GPU busy, skip frame to avoid blocking browser event loop

	CFrustum frustum;

	float camHeight = 4.f;
	float cubeHeight = 2.f;
	float cubeHalfSize = 1.0f;

	const int gridN = 32;
	const float rtMargin = 0.5f; // margin in grid cells for reflection wobble
	const uint rtSnap = 32; // RT size quantization in pixels

	// Floor polygon: hexagon at Z=0
	const int numPolyVerts = 6;
	CVector2f floorPolyWorld[6];
	float radius = 5.f;
	for (int i = 0; i < numPolyVerts; ++i)
	{
		float a = float(i) * float(2.0 * Pi) / float(numPolyVerts);
		floorPolyWorld[i] = CVector2f(cosf(a) * radius, sinf(a) * radius);
	}

	// CDriverUser provides setRenderTarget, which is not on the UDriver interface
	CDriverUser *dru = static_cast<CDriverUser *>(m_Driver);

	{
		m_Driver->EventServer.pump();

		uint32 screenW, screenH;
		m_Driver->getWindowSize(screenW, screenH);
		if (screenW == 0 || screenH == 0) { nlSleep(10); return; }
		frustum.initPerspective(float(Pi / 3.0), float(screenW) / float(screenH), 0.1f, 100.f);

		double now = CTime::ticksToSecond(CTime::getPerformanceTime());
		float dt = float(now - m_LastTime);
		m_LastTime = now;
		if (dt > 0.f) m_SmoothFps += (1.f / dt - m_SmoothFps) * min(1.f, dt * 5.f);

		if (m_AnimCamera) m_CamAngle += dt * 0.3f;
		if (m_AnimCube) { m_CubeAngleZ += dt * 0.5f; m_CubeAngleX += dt * 0.3f; }
		if (m_AnimSkybox) m_SkyAngle += dt * 0.1f;
		if (m_KeyForward) m_CamDist -= dt * 4.f;
		if (m_KeyBackward) m_CamDist += dt * 4.f;
		if (m_CamDist < 1.f) m_CamDist = 1.f;

		m_Driver->setPolygonMode(m_Wireframe ? UDriver::Line : UDriver::Filled);

		// --- Phase 1: Per-frame setup ---

		CVector eye(cosf(m_CamAngle) * m_CamDist, sinf(m_CamAngle) * m_CamDist, camHeight);
		CVector target(0.f, 0.f, cubeHeight * 0.5f);
		CVector up(0.f, 0.f, 1.f);

		CMatrix viewMatrix = buildViewMatrix(eye, target, up);

		// Cube transform: positioned at cubeHeight, spinning
		CMatrix cubeTransform;
		cubeTransform.identity();
		cubeTransform.setPos(CVector(0.f, 0.f, cubeHeight));
		cubeTransform.rotateZ(m_CubeAngleZ);
		cubeTransform.rotateX(m_CubeAngleX);

		CMatrix modelMatrix;
		modelMatrix.identity();

		CMatrix camWorld = viewMatrix;
		camWorld.invert();

		// --- Phase 2: Reflected camera ---

		CVector reflectedEye(eye.x, eye.y, -eye.z);
		CVector reflectedTarget(target.x, target.y, -target.z);
		CMatrix reflectedViewMatrix = buildViewMatrix(reflectedEye, reflectedTarget, up);

		// --- Compute reflection RT bounds ---
		// The sub-frustum must be computed from the *reflected* camera's projection
		// of the floor polygon, since the reflected camera has a different orientation.
		// A separate AABB from the normal camera is used for the debug overlay.

		float reflAabbMinX = 1.f, reflAabbMaxX = 0.f;
		float reflAabbMinY = 1.f, reflAabbMaxY = 0.f;
		float viewAabbMinX = 1.f, viewAabbMaxX = 0.f;
		float viewAabbMinY = 1.f, viewAabbMaxY = 0.f;
		for (int i = 0; i < numPolyVerts; ++i)
		{
			CVector wp(floorPolyWorld[i].x, floorPolyWorld[i].y, 0.f);

			CVector reflScr = frustum.project(reflectedViewMatrix * wp);
			if (reflScr.x < reflAabbMinX) reflAabbMinX = reflScr.x;
			if (reflScr.x > reflAabbMaxX) reflAabbMaxX = reflScr.x;
			if (reflScr.y < reflAabbMinY) reflAabbMinY = reflScr.y;
			if (reflScr.y > reflAabbMaxY) reflAabbMaxY = reflScr.y;

			CVector viewScr = frustum.project(viewMatrix * wp);
			if (viewScr.x < viewAabbMinX) viewAabbMinX = viewScr.x;
			if (viewScr.x > viewAabbMaxX) viewAabbMaxX = viewScr.x;
			if (viewScr.y < viewAabbMinY) viewAabbMinY = viewScr.y;
			if (viewScr.y > viewAabbMaxY) viewAabbMaxY = viewScr.y;
		}

		// Clamp to visible screen area before margin/snapping
		reflAabbMinX = max(0.f, reflAabbMinX);
		reflAabbMaxX = min(1.f, reflAabbMaxX);
		reflAabbMinY = max(0.f, reflAabbMinY);
		reflAabbMaxY = min(1.f, reflAabbMaxY);
		viewAabbMinX = max(0.f, viewAabbMinX);
		viewAabbMaxX = min(1.f, viewAabbMaxX);
		viewAabbMinY = max(0.f, viewAabbMinY);
		viewAabbMaxY = min(1.f, viewAabbMaxY);

		// Add margin (configurable, in grid cells)
		float marginScreen = rtMargin / float(gridN);
		reflAabbMinX -= marginScreen;
		reflAabbMaxX += marginScreen;
		reflAabbMinY -= marginScreen;
		reflAabbMaxY += marginScreen;

		// Compute active RT dimensions from AABB (what dynamic mode would use)
		uint activeW, activeH;
		{
			uint rawW = (uint)max(1.f, ceilf((reflAabbMaxX - reflAabbMinX) * screenW));
			uint rawH = (uint)max(1.f, ceilf((reflAabbMaxY - reflAabbMinY) * screenH));
			uint snappedW = ((rawW + rtSnap - 1) / rtSnap) * rtSnap;
			uint snappedH = ((rawH + rtSnap - 1) / rtSnap) * rtSnap;
			float padX = float(snappedW - rawW) / (2.f * screenW);
			float padY = float(snappedH - rawH) / (2.f * screenH);
			reflAabbMinX -= padX;
			reflAabbMaxX += padX;
			reflAabbMinY -= padY;
			reflAabbMaxY += padY;
			activeW = snappedW;
			activeH = snappedH;
		}

		// Apply half-res and pow2 to active dimensions
		activeW = ((activeW + rtSnap - 1) / rtSnap) * rtSnap;
		activeH = ((activeH + rtSnap - 1) / rtSnap) * rtSnap;
		if (m_HalfRes) { activeW = max(1u, activeW / 2); activeH = max(1u, activeH / 2); }
		if (m_Pow2)
		{
			uint pw = 1; while (pw * 2 <= activeW) pw *= 2; activeW = pw;
			uint ph = 1; while (ph * 2 <= activeH) ph *= 2; activeH = ph;
		}

		// Allocation size: in fixed mode, allocate at window size + margin (stable);
		// in dynamic mode, allocate at active size (may cause reallocation stutter).
		uint rtW, rtH;
		if (m_FixedRT)
		{
			uint marginPx = (uint)ceilf(marginScreen * max((float)screenW, (float)screenH));
			rtW = screenW + 2 * marginPx;
			rtH = screenH + 2 * marginPx;
			rtW = ((rtW + rtSnap - 1) / rtSnap) * rtSnap;
			rtH = ((rtH + rtSnap - 1) / rtSnap) * rtSnap;
			if (m_HalfRes) { rtW = max(1u, rtW / 2); rtH = max(1u, rtH / 2); }
			if (m_Pow2)
			{
				uint pw = 1; while (pw * 2 <= rtW) pw *= 2; rtW = pw;
				uint ph = 1; while (ph * 2 <= rtH) ph *= 2; rtH = ph;
			}
			// Clamp active to allocation (active can't exceed the RT)
			if (activeW > rtW) activeW = rtW;
			if (activeH > rtH) activeH = rtH;
		}
		else
		{
			rtW = activeW;
			rtH = activeH;
		}

		// UV scale: maps sub-frustum [0,1] output to the active sub-region of the RT
		CUV uvScale(float(activeW) / float(rtW), float(activeH) / float(rtH));

		// Build off-center sub-frustum covering only the reflected AABB region.
		// The sub-frustum limits rendering to the floor polygon's bounding box.
		CFrustum reflFrustum = frustum;
		float fw = frustum.Right - frustum.Left;
		float fh = frustum.Top - frustum.Bottom;
		reflFrustum.Left = frustum.Left + reflAabbMinX * fw;
		reflFrustum.Right = frustum.Left + reflAabbMaxX * fw;
		reflFrustum.Bottom = frustum.Bottom + reflAabbMinY * fh;
		reflFrustum.Top = frustum.Bottom + reflAabbMaxY * fh;

		// --- Phase 3: Render reflected scene to render target ---

		CRenderTargetManager &rtm = m_Driver->getRenderTargetManager();
		CTextureUser *reflectRT = rtm.getRenderTarget(rtW, rtH);

		dru->setRenderTarget(*reflectRT);

		// Set viewport and scissor to the active sub-region of the RT
		float vpW = float(activeW) / float(rtW);
		float vpH = float(activeH) / float(rtH);
		CViewport activeVP;
		activeVP.init(0.f, 0.f, vpW, vpH);
		m_Driver->setViewport(activeVP);
		m_Driver->setScissor(CScissor(0.f, 0.f, vpW, vpH));

		m_Driver->clearBuffers(CRGBA(40, 40, 40));

		m_Driver->setFrustum(reflFrustum);
		m_Driver->setViewMatrix(reflectedViewMatrix);
		m_Driver->setModelMatrix(modelMatrix);

		// Clip plane: keep geometry above Z=0
		m_Driver->setClipPlane(0, CPlane(0.f, 0.f, 1.f, 0.f));
		m_Driver->enableClipPlane(0, true);

		drawSkybox(m_Driver, m_SkyMat, reflectedEye, m_SkyAngle);
		drawCube(m_Driver, m_CubeMat, cubeTransform, cubeHalfSize);

		m_Driver->enableClipPlane(0, false);

		// --- Phase 4: Unbind render target, restore viewport/scissor ---

		CTextureUser texNull;
		dru->setRenderTarget(texNull);

		CViewport fullVP;
		fullVP.initFullScreen();
		m_Driver->setViewport(fullVP);
		CScissor fullScissor;
		fullScissor.initFullScreen();
		m_Driver->setScissor(fullScissor);

		// --- Phase 5: Render scene normally ---

		m_Driver->clearBuffers(CRGBA(40, 40, 40));

		m_Driver->setFrustum(frustum);
		m_Driver->setViewMatrix(viewMatrix);
		m_Driver->setModelMatrix(modelMatrix);

		drawSkybox(m_Driver, m_SkyMat, eye, m_SkyAngle);
		drawCube(m_Driver, m_CubeMat, cubeTransform, cubeHalfSize);

		// --- Phase 6: Render floor with reflection texture ---

		// Set reflection texture on floor material
		m_FloorMat.setTexture(0, reflectRT);

		// Screen-space grid tessellation with polygon clipping
		// (matching the NeL water rendering approach):
		// 1. Project the floor polygon to screen-space grid coordinates
		// 2. Rasterize to classify grid cells as inner, border, or outer
		// 3. Clip border cells against the polygon edges (Sutherland-Hodgman)
		// 4. Un-project grid vertices back to Z=0, compute reflection UVs
		{
			CRGBA floorColor(150, 180, 220, 180);

			// Project floor polygon to grid coordinates [0..gridN]
			CPolygon2D projPoly;
			projPoly.Vertices.resize(numPolyVerts);
			for (int i = 0; i < numPolyVerts; ++i)
			{
				CVector wp(floorPolyWorld[i].x, floorPolyWorld[i].y, 0.f);
				CVector scr = frustum.project(viewMatrix * wp);
				projPoly.Vertices[i] = CVector2f(scr.x * gridN, scr.y * gridN);
			}

			// Rasterize: find inner cells (fully inside) and outer cells (touched)
			CPolygon2D::TRasterVect inside;
			sint minYInside;
			projPoly.computeInnerBorders(inside, minYInside);

			CPolygon2D::TRasterVect border;
			sint minYBorder;
			projPoly.computeOuterBorders(border, minYBorder);

			// Build clip planes from projected polygon edges (in grid space, z=0)
			std::vector<CPlane> clipPlanes(numPolyVerts);
			bool ccw = projPoly.isCCWOriented();
			for (int i = 0; i < numPolyVerts; ++i)
			{
				int j = (i + 1) % numPolyVerts;
				CVector v0(projPoly.Vertices[i].x, projPoly.Vertices[i].y, 0.f);
				CVector v1(projPoly.Vertices[j].x, projPoly.Vertices[j].y, 0.f);
				CVector2f edge = projPoly.Vertices[j] - projPoly.Vertices[i];
				CVector v2(projPoly.Vertices[i].x, projPoly.Vertices[i].y, edge.norm());
				clipPlanes[i].make(v0, v1, v2);
				if (!ccw) clipPlanes[i].invert();
			}

			std::vector<CQuadColorUV> floorQuads;
			floorQuads.reserve(gridN * gridN);

			for (sint ky = 0; ky < (sint)border.size(); ++ky)
			{
				sint absY = minYBorder + ky;

				// Look up inner range for this scanline
				sint insideIdx = absY - minYInside;
				sint borderFirst = border[ky].first;
				sint borderLast = border[ky].second;
				sint insideFirst, insideLast;
				if (insideIdx >= 0 && insideIdx < (sint)inside.size()
					&& inside[insideIdx].first <= inside[insideIdx].second)
				{
					insideFirst = inside[insideIdx].first;
					insideLast = inside[insideIdx].second;
				}
				else
				{
					// No inside cells on this scanline — all cells are borders
					insideFirst = borderLast + 1;
					insideLast = borderLast;
				}

				// Left border cells: clip against polygon
				for (sint x = borderFirst; x < insideFirst; ++x)
				{
					CPolygon clipPoly;
					clipPoly.Vertices.resize(4);
					clipPoly.Vertices[0].set(float(x),     float(absY),     0.f);
					clipPoly.Vertices[1].set(float(x + 1), float(absY),     0.f);
					clipPoly.Vertices[2].set(float(x + 1), float(absY + 1), 0.f);
					clipPoly.Vertices[3].set(float(x),     float(absY + 1), 0.f);
					clipPoly.clip(clipPlanes);
					emitClippedPoly(clipPoly, gridN, frustum, camWorld, eye,
						reflFrustum, reflectedViewMatrix, uvScale, floorColor, floorQuads);
				}

				// Inner cells: no clipping needed
				for (sint x = insideFirst; x <= insideLast; ++x)
				{
					CVector wp[4];
					CUV uv[4];
					float coords[4][2] = {
						{ float(x),     float(absY) },
						{ float(x + 1), float(absY) },
						{ float(x + 1), float(absY + 1) },
						{ float(x),     float(absY + 1) }
					};
					bool valid = true;
					for (int c = 0; c < 4; ++c)
					{
						if (!gridToWorldAndUV(coords[c][0], coords[c][1], gridN,
							frustum, camWorld, eye, reflFrustum, reflectedViewMatrix, uvScale, wp[c], uv[c]))
						{
							valid = false;
							break;
						}
					}
					if (!valid) continue;

					CQuadColorUV q;
					q.V0 = wp[0]; q.V1 = wp[1]; q.V2 = wp[2]; q.V3 = wp[3];
					q.Color0 = q.Color1 = q.Color2 = q.Color3 = floorColor;
					q.Uv0 = uv[0]; q.Uv1 = uv[1]; q.Uv2 = uv[2]; q.Uv3 = uv[3];
					floorQuads.push_back(q);
				}

				// Right border cells: clip against polygon
				for (sint x = insideLast + 1; x <= borderLast; ++x)
				{
					CPolygon clipPoly;
					clipPoly.Vertices.resize(4);
					clipPoly.Vertices[0].set(float(x),     float(absY),     0.f);
					clipPoly.Vertices[1].set(float(x + 1), float(absY),     0.f);
					clipPoly.Vertices[2].set(float(x + 1), float(absY + 1), 0.f);
					clipPoly.Vertices[3].set(float(x),     float(absY + 1), 0.f);
					clipPoly.clip(clipPlanes);
					emitClippedPoly(clipPoly, gridN, frustum, camWorld, eye,
						reflFrustum, reflectedViewMatrix, uvScale, floorColor, floorQuads);
				}
			}

			if (!floorQuads.empty())
				m_Driver->drawQuads(floorQuads, m_FloorMat);
		}

		// --- Phase 6b: Draw full RT overlay ---

		if (m_ShowRT)
		{
			m_Driver->setMatrixMode2D11();

			float x0, y0, x1, y1;
			if (m_ShowRT == 1)
			{
				// Fullscreen
				x0 = 0.f; y0 = 0.f; x1 = 1.f; y1 = 1.f;
			}
			else
			{
				// Corner thumbnail: 1/4 screen in top-right
				float thumbW = 0.25f;
				float thumbH = 0.25f * float(rtW) / float(rtH); // preserve RT aspect ratio
				if (thumbH > 0.25f) { thumbW *= 0.25f / thumbH; thumbH = 0.25f; }
				x0 = 1.f - thumbW;
				y0 = 1.f - thumbH;
				x1 = 1.f;
				y1 = 1.f;
			}

			CQuadColorUV rtQuad;
			rtQuad.V0.set(x0, y0, 0.5f); rtQuad.Uv0 = CUV(0.f, 1.f);
			rtQuad.V1.set(x1, y0, 0.5f); rtQuad.Uv1 = CUV(1.f, 1.f);
			rtQuad.V2.set(x1, y1, 0.5f); rtQuad.Uv2 = CUV(1.f, 0.f);
			rtQuad.V3.set(x0, y1, 0.5f); rtQuad.Uv3 = CUV(0.f, 0.f);
			rtQuad.Color0 = rtQuad.Color1 = rtQuad.Color2 = rtQuad.Color3 = CRGBA::White;
			m_Driver->drawQuad(rtQuad, m_FloorMat);

			if (m_ShowRT == 2)
			{
				// White border around thumbnail
				CRGBA borderColor(255, 255, 255);
				m_Driver->drawLine(x0, y0, x1, y0, borderColor);
				m_Driver->drawLine(x1, y0, x1, y1, borderColor);
				m_Driver->drawLine(x1, y1, x0, y1, borderColor);
				m_Driver->drawLine(x0, y1, x0, y0, borderColor);
			}
		}

		// --- Phase 6c: Draw RT boundary overlay ---

		if (m_ShowBounds)
		{
			m_Driver->setMatrixMode2D11();

			// Yellow: render target boundary projected onto the normal screen
			// Un-project RT corners from reflected camera → world Z=0 → normal camera
			CMatrix reflCamWorld = reflectedViewMatrix;
			reflCamWorld.invert();

			float rtCornersRefl[4][2] = {
				{ reflAabbMinX, reflAabbMinY },
				{ reflAabbMaxX, reflAabbMinY },
				{ reflAabbMaxX, reflAabbMaxY },
				{ reflAabbMinX, reflAabbMaxY }
			};
			float rtCornersScreen[4][2];
			for (int i = 0; i < 4; ++i)
			{
				CVector nearLocal = frustum.unProject(CVector(rtCornersRefl[i][0], rtCornersRefl[i][1], 0.f));
				CVector worldNear = reflCamWorld * nearLocal;
				CVector dir = worldNear - reflectedEye;
				float t = (fabsf(dir.z) > 0.0001f) ? (-reflectedEye.z / dir.z) : 0.f;
				CVector wp(reflectedEye.x + t * dir.x, reflectedEye.y + t * dir.y, 0.f);
				CVector scr = frustum.project(viewMatrix * wp);
				rtCornersScreen[i][0] = scr.x;
				rtCornersScreen[i][1] = scr.y;
			}

			CRGBA rtColor(255, 255, 0);
			for (int i = 0; i < 4; ++i)
			{
				int j = (i + 1) % 4;
				m_Driver->drawLine(rtCornersScreen[i][0], rtCornersScreen[i][1],
					rtCornersScreen[j][0], rtCornersScreen[j][1], rtColor);
			}

			// Cyan: floor polygon screen footprint (normal camera AABB)
			CRGBA viewColor(0, 255, 255);
			m_Driver->drawLine(viewAabbMinX, viewAabbMinY, viewAabbMaxX, viewAabbMinY, viewColor);
			m_Driver->drawLine(viewAabbMaxX, viewAabbMinY, viewAabbMaxX, viewAabbMaxY, viewColor);
			m_Driver->drawLine(viewAabbMaxX, viewAabbMaxY, viewAabbMinX, viewAabbMaxY, viewColor);
			m_Driver->drawLine(viewAabbMinX, viewAabbMaxY, viewAabbMinX, viewAabbMinY, viewColor);
		}

		// --- Phase 6d: Draw HUD text ---

		if (m_TextContext)
		{
			m_TextContext->setHotSpot(UTextContext::TopLeft);
			m_TextContext->setColor(CRGBA::White);
			m_TextContext->setFontSize(12);

			float lineH = 0.025f;
			float x = 0.01f;
			float y = 1.f - 0.01f;

			const char *rtModeNames[] = { "off", "fullscreen", "thumbnail" };

			m_TextContext->printfAt(x, y, "[F] Wireframe: %s", m_Wireframe ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[C] Camera orbit: %s", m_AnimCamera ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[R] Cube rotation: %s", m_AnimCube ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[S] Skybox roll: %s", m_AnimSkybox ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[T] RT overlay: %s", rtModeNames[m_ShowRT]);
			y -= lineH;
			m_TextContext->printfAt(x, y, "[B] Boundary overlay: %s", m_ShowBounds ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[H] Half-res: %s", m_HalfRes ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[P] Pow2: %s", m_Pow2 ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[W] Fixed RT: %s", m_FixedRT ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[V] VSync: %s", m_VSync ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[Up/Down] Camera dist: %.1f", m_CamDist);
			y -= lineH * 1.5f;
			m_TextContext->printfAt(x, y, "RT: %ux%u  Active: %ux%u  UV scale: %.3f x %.3f",
				rtW, rtH, activeW, activeH, uvScale.U, uvScale.V);
			y -= lineH;
			m_TextContext->printfAt(x, y, "FPS: %.1f  (%.2f ms)", m_SmoothFps, dt * 1000.f);
		}

		// --- Phase 7: Cleanup and swap ---

		m_FloorMat.setTexture(0, NULL);
		rtm.recycleRenderTarget(reflectRT);

		m_Driver->swapBuffers();
	}
}

#ifdef __EMSCRIPTEN__
static CPlanarReflectionDemo *s_Demo = NULL;

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
	// Emscripten: demo must persist since emscripten_set_main_loop never returns
	static CPlanarReflectionDemo demo;
	s_Demo = &demo;
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
	CPlanarReflectionDemo demo;
	demo.run();
#endif

	return EXIT_SUCCESS;
}
