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
// Clip Plane Demo
//
// Draws a rotating cube with a clip plane that animates up and down,
// slicing through the cube to demonstrate hardware clip plane support.
//

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/geom_ext.h>
#include <nel/misc/plane.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_material.h>

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

// Clip plane demo application
class CClipPlaneDemo : public IEventListener
{
public:
	CClipPlaneDemo();
	~CClipPlaneDemo();
	void run();
	void renderOneFrame();

	virtual void operator()(const CEvent &event) NL_OVERRIDE;

private:
	bool m_CloseWindow;
	UDriver *m_Driver;
	UMaterial m_CubeMat;
	UMaterial m_PlaneMat;
	CFrustum m_Frustum;
	double m_StartTime;
};

CClipPlaneDemo::CClipPlaneDemo()
	: m_CloseWindow(false)
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

	m_Driver->setDisplay(UDriver::CMode(800, 600, 32, true));
	m_Driver->setWindowTitle(ucstring("NeL Clip Plane Demo"));

	// Opaque material for the cube
	m_CubeMat = m_Driver->createMaterial();
	m_CubeMat.initUnlit();
	m_CubeMat.setZWrite(true);
	m_CubeMat.setZFunc(UMaterial::lessequal);

	// Translucent material for the clip plane visualization
	m_PlaneMat = m_Driver->createMaterial();
	m_PlaneMat.initUnlit();
	m_PlaneMat.setZWrite(false);
	m_PlaneMat.setBlend(true);
	m_PlaneMat.setBlendFunc(UMaterial::srcalpha, UMaterial::invsrcalpha);

	m_Frustum.initPerspective(float(Pi / 3.0), 800.f / 600.f, 0.1f, 100.f);
	m_StartTime = CTime::ticksToSecond(CTime::getPerformanceTime());
}

CClipPlaneDemo::~CClipPlaneDemo()
{
	m_Driver->deleteMaterial(m_PlaneMat);
	m_Driver->deleteMaterial(m_CubeMat);
	m_Driver->release();
	delete m_Driver;
}

void CClipPlaneDemo::operator()(const CEvent &event)
{
	if (event == EventCloseWindowId)
	{
		m_CloseWindow = true;
	}
}

void CClipPlaneDemo::renderOneFrame()
{
	if (!m_Driver->isFrameReady())
		return; // GPU busy, skip frame to avoid blocking browser event loop

	m_Driver->EventServer.pump();

	double now = CTime::ticksToSecond(CTime::getPerformanceTime()) - m_StartTime;

	m_Driver->clearBuffers(CRGBA(40, 40, 40));

	float camDist = 8.f;
	float camHeight = 4.f;

	// Orbit camera around the scene
	float camAngle = float(now * 0.3);
	CVector eye(cosf(camAngle) * camDist, sinf(camAngle) * camDist, camHeight);
	CVector target(0.f, 0.f, 0.f);
	CVector up(0.f, 0.f, 1.f);
	CVector jj = (target - eye).normed();
	CVector ii = (jj ^ up).normed();
	CVector kk = ii ^ jj;
	CMatrix camWorld;
	camWorld.setRot(ii, jj, kk, true);
	camWorld.setPos(eye);
	CMatrix viewMatrix = camWorld;
	viewMatrix.invert();

	m_Driver->setFrustum(m_Frustum);
	m_Driver->setViewMatrix(viewMatrix);

	// Identity model matrix (cube is in world space)
	CMatrix modelMatrix;
	modelMatrix.identity();
	m_Driver->setModelMatrix(modelMatrix);

	// Rotate the cube
	CMatrix cubeTransform;
	cubeTransform.identity();
	cubeTransform.rotateZ(float(now * 0.5));
	cubeTransform.rotateX(float(now * 0.3));

	// Animate clip plane: wobble along both axes + oscillate height
	float tiltX = float(sin(now * 0.4) * 0.3);
	float tiltY = float(sin(now * 0.5) * 0.3);
	float clipHeight = float(sin(now * 0.7) * 1.2);
	CVector normal(tiltX, tiltY, 1.f);
	normal.normalize();
	CPlane clipPlane(normal.x, normal.y, normal.z, -clipHeight * normal.z);

	// Enable and set the clip plane
	m_Driver->setClipPlane(0, clipPlane);
	m_Driver->enableClipPlane(0, true);

	// Draw the cube (half-size = 1.5)
	drawCube(m_Driver, m_CubeMat, cubeTransform, 1.5f);

	// Disable clip plane
	m_Driver->enableClipPlane(0, false);

	// Draw a translucent quad to visualize the clip plane
	{
		// Find two tangent vectors to the plane normal
		CVector tangentU = (normal ^ CVector(0.f, 1.f, 0.f));
		if (tangentU.norm() < 0.001f)
			tangentU = normal ^ CVector(1.f, 0.f, 0.f);
		tangentU.normalize();
		CVector tangentV = normal ^ tangentU;
		tangentV.normalize();

		// (0, 0, clipHeight) always lies on our plane since
		// d = -clipHeight * normal.z
		CVector center(0.f, 0.f, clipHeight);
		float planeSize = 3.f;

		CQuadColor planeQuad;
		planeQuad.V0 = center - tangentU * planeSize - tangentV * planeSize;
		planeQuad.V1 = center + tangentU * planeSize - tangentV * planeSize;
		planeQuad.V2 = center + tangentU * planeSize + tangentV * planeSize;
		planeQuad.V3 = center - tangentU * planeSize + tangentV * planeSize;
		CRGBA planeColor(100, 200, 255, 80);
		planeQuad.Color0 = planeQuad.Color1 = planeQuad.Color2 = planeQuad.Color3 = planeColor;
		m_Driver->drawQuad(planeQuad, m_PlaneMat);
	}

	m_Driver->swapBuffers();
}

void CClipPlaneDemo::run()
{
#ifndef __EMSCRIPTEN__
	while (m_Driver->isActive() && !m_CloseWindow)
	{
		renderOneFrame();
	}
#endif
}

#ifdef __EMSCRIPTEN__
static CClipPlaneDemo *s_Demo = NULL;

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
	static CClipPlaneDemo demo;
	s_Demo = &demo;
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
	CClipPlaneDemo demo;
	demo.run();
#endif

	return EXIT_SUCCESS;
}
