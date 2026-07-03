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
// Water Rendering Baseline Demo
//
// Shows the current NeL water rendering as-is: the authentic in-game
// lacustre water shape (waterbassina01.shape) instanced in a UScene, using the
// classic skymap/envmap reflection path. A colored cube bobs through the
// water surface and a rolling checkerboard skybox provides visual context.
//
// The point of this demo is what it does NOT do: the water reflection is a
// static environment map, so neither the cube nor the skybox appear in the
// reflection. This is the reference baseline for the upcoming planar
// reflection work (see the planar_reflection sample for the technique).
//
// Asset policy: no graphics asset copies live in the code repo. Assets come
// from a checkout of the public repository
//   https://github.com/ryzom/ryzomcore_graphics
// - Native builds locate the checkout at runtime by walking up from the
//   current working directory looking for a "ryzomcore_graphics" directory
//   (override with the NL_GRAPHICS_DIR environment variable).
// - Emscripten builds preload only the specific files listed in this
//   sample's CMakeLists.txt, taken from the NL_GRAPHICS_DIR CMake cache
//   variable (defaults to a sibling of the source tree).
// The water shapes reference .tga texture names; the graphics repo stores
// .png sources, handled via CPath::remapExtension.
//
// Controls:
//   F - Toggle wireframe rendering
//   C - Toggle camera orbit
//   R - Toggle cube bobbing/rotation
//   S - Toggle skybox roll
//   G - Toggle fog (default on, matches the in-game look; the skybox is
//       drawn fog-free like the game sky)
//   P - Toggle realtime planar reflection (default on): the water plane
//       reflects the actual scene (cube and skybox) via a render target
//       instead of the static envmap. Uses the scene water reflection
//       manager (CWaterReflectionManager) with the force-all flag. The
//       reflection is rendered as replicated passes through this demo's
//       own render logic (beginWaterReflectionPasses / beginWaterReflectionPass /
//       endWaterReflectionPass / endWaterReflectionPasses), the same way
//       the client's render loop replicates passes for stereo.
//   V - Toggle VSync (default on)
//   Up/Down - Move camera closer/farther
//

#include <nel/misc/types_nl.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// Read an integer flag from the URL query string, e.g. ?planar=0&mirror=1.
// Key input does not reach NeL on the web (no input callbacks in the
// Emscripten driver backend yet), so this is the way to drive the toggles.
static int queryFlag(const char *name, int defaultValue)
{
	return EM_ASM_INT({
		var m = new RegExp('[?&]' + UTF8ToString($0) + '=([^&]*)').exec(window.location.search);
		return m ? (parseInt(m[1]) | 0) : $1;
	}, name, defaultValue);
}
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
static CMatrix buildCamWorldMatrix(const CVector &eye, const CVector &target, const CVector &up)
{
	CVector jj = (target - eye).normed();
	CVector ii = (jj ^ up).normed();
	CVector kk = ii ^ jj;
	CMatrix camWorld;
	camWorld.setRot(ii, jj, kk, true);
	camWorld.setPos(eye);
	return camWorld;
}

// Draw a checkerboard skybox that rolls forward (rotates around X axis).
// Only the odd cells are drawn on top of the clear color.
static void drawSkybox(UDriver *driver, UMaterial &mat, const CVector &center, float angle)
{
	const float s = 400.f;
	const int grid = 16;
	float cs = 2.f * s / float(grid);
	CRGBA color(100, 125, 160); // bright sky cells so the water reflection reads clearly

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

// Find a ryzomcore_graphics checkout by walking up from the current working
// directory, or from the NL_GRAPHICS_DIR environment variable.
static std::string findGraphicsDir()
{
	const char *env = getenv("NL_GRAPHICS_DIR");
	if (env && CFile::isDirectory(std::string(env) + "/landscape/water"))
		return CPath::standardizePath(env, false);

	std::string rootPath = CPath::standardizePath(CPath::getCurrentPath(), false);
	while (!rootPath.empty())
	{
		std::string candidate = rootPath + "/ryzomcore_graphics";
		if (CFile::isDirectory(candidate + "/landscape/water"))
			return candidate;
		std::string::size_type sep = CFile::getLastSeparator(rootPath);
		if (sep == string::npos)
			break;
		rootPath = rootPath.substr(0, sep);
	}
	return std::string();
}

// Find the project root by walking up from the current working directory
// looking for a ".nel" folder, then return the font path under graphics/.
static std::string findFontPath()
{
	// Emscripten preload / explicit search path
	std::string lookup = CPath::lookup("beteckna.ttf", false, false);
	if (!lookup.empty())
		return lookup;

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

// Water rendering baseline demo application
class CWaterDemo : public IEventListener
{
public:
	CWaterDemo();
	~CWaterDemo();
	void run();
	void renderOneFrame();

	virtual void operator()(const CEvent &event) NL_OVERRIDE;

	// Option ids shared with the injected HTML control panel (Emscripten)
	enum TOption
	{
		OptPlanar = 0,
		OptMirror = 1,
		OptOverlay = 2,
		OptHalfRes = 3,
		OptPow2 = 4,
		OptFixedSize = 5,
		OptFog = 6,
		OptOrbit = 7,
		OptBob = 8,
		OptSkyRoll = 9,
		OptCamDist = 10,
		OptCamHeight = 11
	};
	void setOption(int opt, int value);

private:
	void drawWorldContent(UDriver &driver, const NLMISC::CVector &eye);

public:

private:
	bool m_CloseWindow;
	bool m_Wireframe;
	bool m_AnimCamera;
	bool m_AnimCube;
	bool m_AnimSkybox;
	bool m_Fog;
	bool m_Planar;
	bool m_Mirror;  // debug: draw the raw reflection as an opaque mirror floor
	int m_ShowRT;   // debug: reflection RT overlay: 0=off, 1=fullscreen, 2=corner thumbnail
	bool m_VSync;
	bool m_KeyForward;
	bool m_KeyBackward;
	int m_QueryCamDist;
	int m_QueryCamHeight;
	UDriver *m_Driver;
	UScene *m_Scene;
	UTextContext *m_TextContext;
	UMaterial m_SkyMat;
	UMaterial m_CubeMat;
	UMaterial m_MirrorMat;
	UInstance m_Water;
	CVector m_WaterCenter;
	float m_WaterRadius;
	float m_CubeHalfSize;
	CMatrix m_CubeTransform;
	float m_CamAngle;
	float m_CubeAngle;
	float m_BobPhase;
	float m_SkyAngle;
	float m_CamDist;
	float m_CamHeight;
	double m_StartTime;
	double m_LastTime;
	float m_SmoothFps;
};

CWaterDemo::CWaterDemo()
	: m_CloseWindow(false)
	, m_Wireframe(false)
	, m_AnimCamera(true)
	, m_AnimCube(true)
	, m_AnimSkybox(true)
	, m_Fog(true)
	, m_Planar(true)
	, m_Mirror(false)
	, m_ShowRT(2)
	, m_VSync(true)
	, m_KeyForward(false)
	, m_KeyBackward(false)
	, m_QueryCamDist(0)
	, m_QueryCamHeight(0)
	, m_Scene(NULL)
	, m_TextContext(NULL)
	, m_WaterCenter(0.f, 0.f, 0.f)
	, m_WaterRadius(10.f)
	, m_CubeHalfSize(1.f)
	, m_CamAngle(0.f)
	, m_CubeAngle(0.f)
	, m_BobPhase(0.f)
	, m_SkyAngle(0.f)
	, m_CamDist(15.f)
	, m_CamHeight(5.f)
	, m_StartTime(0.0)
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
	m_Driver->setWindowTitle(ucstring("NeL Water Rendering Baseline Demo"));
	m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0);

	// Asset search paths; no asset copies live in the code repo (see header)
#ifdef __EMSCRIPTEN__
	CPath::addSearchPath("/share/nl_sample_water/", true, false);
#else
	std::string gfx = findGraphicsDir();
	if (!gfx.empty())
	{
		nlinfo("Using graphics assets from '%s'", gfx.c_str());
		CPath::addSearchPath(gfx + "/landscape/water/shapes", false, false);
		CPath::addSearchPath(gfx + "/landscape/water/meshes/lacustre", false, false);
	}
	else
	{
		nlwarning("No ryzomcore_graphics checkout found; water shape will not load");
	}
#endif
	// Shapes reference .tga texture names; the graphics repo stores .png
	// sources. remapExtension(png, tga) = "a .png file satisfies a .tga lookup".
	CPath::remapExtension("png", "tga", true);

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

	// Textured material for the mirror-floor debug mode and the RT overlay
	m_MirrorMat = m_Driver->createMaterial();
	m_MirrorMat.initUnlit();
	m_MirrorMat.setZWrite(true);
	m_MirrorMat.setZFunc(UMaterial::lessequal);
	m_MirrorMat.setDoubleSided(true);

	// Scene with the water shape; the actual in-game lacustre lake water
	m_Scene = m_Driver->createScene(true);

	// Realtime planar reflections: one plane budget, force-enabled on all
	// water (the shape predates the artist flag), demo draws the skybox and
	// cube into the reflection via the content callback
#ifdef __EMSCRIPTEN__
	// URL query flags, e.g. ?planar=0&mirror=1&overlay=1&halfres=0&pow2=0&fixed=0&fog=0&orbit=0&bob=0
	m_Planar = queryFlag("planar", m_Planar ? 1 : 0) != 0;
	m_Mirror = queryFlag("mirror", m_Mirror ? 1 : 0) != 0;
	m_ShowRT = queryFlag("overlay", m_ShowRT);
	m_Fog = queryFlag("fog", m_Fog ? 1 : 0) != 0;
	m_AnimCamera = queryFlag("orbit", m_AnimCamera ? 1 : 0) != 0;
	m_AnimCube = queryFlag("bob", m_AnimCube ? 1 : 0) != 0;
	m_Scene->setWaterReflectionHalfRes(queryFlag("halfres", m_Scene->getWaterReflectionHalfRes() ? 1 : 0) != 0);
	m_Scene->setWaterReflectionPow2(queryFlag("pow2", m_Scene->getWaterReflectionPow2() ? 1 : 0) != 0);
	m_Scene->setWaterReflectionFixedSize(queryFlag("fixed", m_Scene->getWaterReflectionFixedSize() ? 1 : 0) != 0);
	m_QueryCamDist = queryFlag("dist", 0);
	m_QueryCamHeight = queryFlag("height", 0);
#endif
	m_Scene->setMaxRealtimeWaterReflections(m_Planar ? 1 : 0);
	m_Scene->setForceRealtimeWaterReflections(true);

	m_Water = m_Scene->createInstance("waterbassina01.shape");
	if (!m_Water.empty())
	{
		// The shape's default position track carries its in-game world
		// coordinates (~17km away); place the instance at the origin instead.
		// The polygon itself is in shape-local space (bbox center below).
		m_Water.setPos(CVector(0.f, 0.f, 0.f));

		CAABBox bbox;
		m_Water.getShapeAABBox(bbox);
		m_WaterCenter = bbox.getCenter();
		CVector halfSize = bbox.getHalfSize();
		m_WaterRadius = max(1.f, max(halfSize.x, halfSize.y));
		nlinfo("Water shape loaded: center (%.1f %.1f %.1f), radius %.1f",
			m_WaterCenter.x, m_WaterCenter.y, m_WaterCenter.z, m_WaterRadius);

		// Scale demo proportions to the water footprint
		m_CubeHalfSize = m_WaterRadius * 0.08f;
		clamp(m_CubeHalfSize, 0.5f, 3.f);
		m_CamDist = m_WaterRadius * 1.1f;
		clamp(m_CamDist, 8.f, 80.f);
		m_CamHeight = m_WaterRadius * 0.3f;
		clamp(m_CamHeight, 3.f, 25.f);
		if (m_QueryCamDist > 0) m_CamDist = (float)m_QueryCamDist;
		if (m_QueryCamHeight > 0) m_CamHeight = (float)m_QueryCamHeight;
	}
	else
	{
		nlwarning("Failed to load waterbassina01.shape; check assets");
	}

	m_StartTime = CTime::ticksToSecond(CTime::getPerformanceTime());

#ifdef __EMSCRIPTEN__
	// Inject an HTML control panel into the host page. Key input does not
	// reach NeL in browsers, so the page UI drives the demo through the
	// exported nlwater_option() function.
	EM_ASM({
		// NB: the array literal is wrapped in parens: the C preprocessor
		// splits EM_ASM arguments on commas that are not inside parentheses
		var opts = ([
			['Planar reflection', 0, $0],
			['Mirror floor debug', 1, $1],
			['Half-res RT', 3, $3],
			['Pow2 RT', 4, $4],
			['Fixed-size RT', 5, $5],
			['Fog', 6, $6],
			['Camera orbit', 7, $7],
			['Cube bobbing', 8, $8],
			['Skybox roll', 9, $9]
		]);
		var panel = document.createElement('div');
		panel.style.cssText = 'position:fixed;left:10px;bottom:10px;background:rgba(10,15,30,0.85);color:#dde;font:12px sans-serif;padding:10px 12px;border-radius:8px;z-index:20;user-select:none';
		var html = '<b>Water Demo</b>';
		opts.forEach(function(o) {
			html += '<label style="display:block;margin-top:4px;cursor:pointer"><input type="checkbox" data-opt="' + o[1] + '"' + (o[2] ? ' checked' : '') + '> ' + o[0] + '</label>';
		});
		html += '<label style="display:block;margin-top:4px">RT overlay <select data-opt="2">'
			+ '<option value="0"' + ($2 == 0 ? ' selected' : '') + '>off</option>'
			+ '<option value="1"' + ($2 == 1 ? ' selected' : '') + '>fullscreen</option>'
			+ '<option value="2"' + ($2 == 2 ? ' selected' : '') + '>thumbnail</option>'
			+ '</select></label>';
		html += '<label style="display:block;margin-top:6px">Cam dist <input type="range" data-opt="10" min="8" max="100" value="' + $10 + '" style="width:90px;vertical-align:middle"></label>';
		html += '<label style="display:block;margin-top:2px">Cam height <input type="range" data-opt="11" min="3" max="60" value="' + $11 + '" style="width:90px;vertical-align:middle"></label>';
		panel.innerHTML = html;
		function send(t) {
			var opt = parseInt(t.getAttribute('data-opt'));
			var val = (t.type === 'checkbox') ? (t.checked ? 1 : 0) : parseInt(t.value);
			Module._nlwater_option(opt, val);
		}
		panel.addEventListener('change', function(e) { send(e.target); });
		panel.addEventListener('input', function(e) { if (e.target.type === 'range') send(e.target); });
		document.body.appendChild(panel);
	},
		m_Planar ? 1 : 0,
		m_Mirror ? 1 : 0,
		m_ShowRT,
		m_Scene->getWaterReflectionHalfRes() ? 1 : 0,
		m_Scene->getWaterReflectionPow2() ? 1 : 0,
		m_Scene->getWaterReflectionFixedSize() ? 1 : 0,
		m_Fog ? 1 : 0,
		m_AnimCamera ? 1 : 0,
		m_AnimCube ? 1 : 0,
		m_AnimSkybox ? 1 : 0,
		(int)m_CamDist,
		(int)m_CamHeight);
#endif
}

CWaterDemo::~CWaterDemo()
{
	m_Driver->deleteMaterial(m_MirrorMat);
	if (!m_Water.empty())
		m_Scene->deleteInstance(m_Water);
	if (m_Scene)
		m_Driver->deleteScene(m_Scene);
	if (m_TextContext)
		m_Driver->deleteTextContext(m_TextContext);
	m_Driver->deleteMaterial(m_CubeMat);
	m_Driver->deleteMaterial(m_SkyMat);
	m_Driver->release();
	delete m_Driver;
}

void CWaterDemo::operator()(const CEvent &event)
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
			if (keyDown.Key == KeyS) setOption(OptSkyRoll, !m_AnimSkybox);
			if (keyDown.Key == KeyG) setOption(OptFog, !m_Fog);
			if (keyDown.Key == KeyP) setOption(OptPlanar, !m_Planar);
			if (keyDown.Key == KeyM) setOption(OptMirror, !m_Mirror);
			if (keyDown.Key == KeyT) setOption(OptOverlay, (m_ShowRT + 1) % 3);
			if (keyDown.Key == KeyH) setOption(OptHalfRes, !m_Scene->getWaterReflectionHalfRes());
			if (keyDown.Key == KeyY) setOption(OptPow2, !m_Scene->getWaterReflectionPow2());
			if (keyDown.Key == KeyW) setOption(OptFixedSize, !m_Scene->getWaterReflectionFixedSize());
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

void CWaterDemo::setOption(int opt, int value)
{
	switch (opt)
	{
	case OptPlanar: m_Planar = value != 0; m_Scene->setMaxRealtimeWaterReflections(m_Planar ? 1 : 0); break;
	case OptMirror: m_Mirror = value != 0; break;
	case OptOverlay: m_ShowRT = value; break;
	case OptHalfRes: m_Scene->setWaterReflectionHalfRes(value != 0); break;
	case OptPow2: m_Scene->setWaterReflectionPow2(value != 0); break;
	case OptFixedSize: m_Scene->setWaterReflectionFixedSize(value != 0); break;
	case OptFog: m_Fog = value != 0; break;
	case OptOrbit: m_AnimCamera = value != 0; break;
	case OptBob: m_AnimCube = value != 0; break;
	case OptSkyRoll: m_AnimSkybox = value != 0; break;
	case OptCamDist: m_CamDist = (float)value; break;
	case OptCamHeight: m_CamHeight = (float)value; break;
	}
}

// Draw the demo's world content (used by both the main pass and the
// reflection pass, so the reflection always matches the scene):
// fog-free skybox like the game sky, then the fogged cube
void CWaterDemo::drawWorldContent(UDriver &driver, const CVector &eye)
{
	driver.enableFog(false);
	drawSkybox(&driver, m_SkyMat, eye, m_SkyAngle);
	if (m_Fog)
		driver.enableFog(true);
	drawCube(&driver, m_CubeMat, m_CubeTransform, m_CubeHalfSize);
}


void CWaterDemo::run()
{
	m_LastTime = CTime::ticksToSecond(CTime::getPerformanceTime());

#ifndef __EMSCRIPTEN__
	while (m_Driver->isActive() && !m_CloseWindow)
	{
		renderOneFrame();
	}
#endif
}

void CWaterDemo::renderOneFrame()
{
	if (!m_Driver->isFrameReady())
		return; // GPU busy, skip frame to avoid blocking browser event loop

	m_Driver->EventServer.pump();

	uint32 screenW, screenH;
	m_Driver->getWindowSize(screenW, screenH);
	if (screenW == 0 || screenH == 0) { nlSleep(10); return; }

	double now = CTime::ticksToSecond(CTime::getPerformanceTime());
	float dt = float(now - m_LastTime);
	m_LastTime = now;
	if (dt > 0.f) m_SmoothFps += (1.f / dt - m_SmoothFps) * min(1.f, dt * 5.f);

	if (m_AnimCamera) m_CamAngle += dt * 0.15f;
	if (m_AnimCube) { m_CubeAngle += dt * 0.3f; m_BobPhase += dt * 0.4f; }
	if (m_AnimSkybox) m_SkyAngle += dt * 0.1f;
	if (m_KeyForward) m_CamDist -= dt * (m_WaterRadius * 0.5f);
	if (m_KeyBackward) m_CamDist += dt * (m_WaterRadius * 0.5f);
	if (m_CamDist < 2.f) m_CamDist = 2.f;

	m_Driver->setPolygonMode(m_Wireframe ? UDriver::Line : UDriver::Filled);

	// --- Camera setup (shared between scene render and direct driver draws) ---

	float waterZ = m_WaterCenter.z;
	CVector eye(m_WaterCenter.x + cosf(m_CamAngle) * m_CamDist,
		m_WaterCenter.y + sinf(m_CamAngle) * m_CamDist,
		waterZ + m_CamHeight);
	CVector target(m_WaterCenter.x, m_WaterCenter.y, waterZ);
	CVector up(0.f, 0.f, 1.f);

	CMatrix camWorld = buildCamWorldMatrix(eye, target, up);
	CMatrix viewMatrix = camWorld;
	viewMatrix.invert();

	float fov = float(Pi / 3.0);
	float aspect = float(screenW) / float(screenH);
	float nearZ = 0.1f;
	float farZ = 2000.f;

	CFrustum frustum;
	frustum.initPerspective(fov, aspect, nearZ, farZ);

	UCamera camera = m_Scene->getCam();
	camera.setTransformMode(UTransformable::DirectMatrix);
	camera.setPerspective(fov, aspect, nearZ, farZ);
	camera.setMatrix(camWorld);

	// Cube bobbing through the water surface, standing on a corner so its
	// silhouette (and its reflection) is unmistakable
	m_CubeTransform.identity();
	float bob = sinf(m_BobPhase) * m_CubeHalfSize * 1.5f + m_CubeHalfSize * 0.9f;
	m_CubeTransform.setPos(CVector(m_WaterCenter.x, m_WaterCenter.y, waterZ + bob));
	m_CubeTransform.rotateZ(m_CubeAngle + float(Pi / 4.0));
	m_CubeTransform.rotateX(0.9553f); // atan(sqrt(2)): body diagonal vertical, corner down

	// Fog matching the in-game look; scaled to the water footprint.
	// Enabled before the reflection pass so reflections are fogged too.
	if (m_Fog)
	{
		m_Driver->setupFog(m_WaterRadius * 0.4f, m_WaterRadius * 2.5f, CRGBA(75, 95, 125));
		m_Driver->enableFog(true);
	}
	else
	{
		m_Driver->enableFog(false);
	}

	m_Scene->animate(now - m_StartTime);

	CMatrix identity;
	identity.identity();

	// --- Reflection passes: replicate our render logic once per admitted
	// water plane, with the reflected camera the pass sets up ---

	uint numReflPasses = m_Scene->beginWaterReflectionPasses();
	for (uint reflPass = 0; reflPass < numReflPasses; ++reflPass)
	{
		UWaterReflectionInfo passInfo;
		m_Scene->beginWaterReflectionPass(reflPass, passInfo);

		// Set our driver context to the reflected camera and the render
		// target's active sub-region for the direct draws
		CMatrix reflCamWorld = passInfo.ReflViewMatrix;
		reflCamWorld.invert();
		CFrustum reflFrustum(passInfo.FrustumLeft, passInfo.FrustumRight,
			passInfo.FrustumBottom, passInfo.FrustumTop,
			passInfo.FrustumNear, passInfo.FrustumFar, true);
		CViewport activeVP;
		activeVP.init(0.f, 0.f, passInfo.UScale, passInfo.VScale);
		m_Driver->setViewport(activeVP);
		m_Driver->setScissor(CScissor(0.f, 0.f, passInfo.UScale, passInfo.VScale));
		m_Driver->setFrustum(reflFrustum);
		m_Driver->setViewMatrix(passInfo.ReflViewMatrix);
		m_Driver->setModelMatrix(identity);

		// Same content as the main pass (water itself is filtered out of
		// the scene render); keepTrav like any replicated pass
		drawWorldContent(*m_Driver, reflCamWorld.getPos());
		m_Scene->beginPartRender();
		m_Scene->renderPart(UScene::RenderAll, true, true, true);
		m_Scene->endPartRender(true, true, true);

		m_Scene->endWaterReflectionPass(reflPass);
	}
	m_Scene->endWaterReflectionPasses();

	// --- Main pass ---

	CViewport fullVP;
	fullVP.initFullScreen();
	m_Driver->setViewport(fullVP);
	CScissor fullScissor;
	fullScissor.initFullScreen();
	m_Driver->setScissor(fullScissor);

	m_Driver->clearBuffers(CRGBA(75, 95, 125));

	m_Driver->setFrustum(frustum);
	m_Driver->setViewMatrix(viewMatrix);
	m_Driver->setModelMatrix(identity);

	drawWorldContent(*m_Driver, eye);

	// --- Scene render: the water surface (blends over the cube and skybox) ---

	m_Scene->render();

	m_Driver->enableFog(false);

	// --- Debug: mirror floor and reflection RT overlay ---

	UWaterReflectionInfo reflInfo;
	bool haveRefl = m_Scene->getActiveWaterReflectionInfo(0, reflInfo);

	if (m_Mirror && haveRefl)
	{
		// Draw the raw reflection as an opaque mirror floor over the water:
		// a world-space grid across the water footprint, each vertex
		// projected through the engine's reflected camera and sub-frustum.
		// This replicates the planar_reflection sample's floor using the
		// engine-provided data, bypassing the water shader entirely.
		m_Driver->setFrustum(frustum);
		m_Driver->setViewMatrix(viewMatrix);
		m_Driver->setModelMatrix(identity);

		const int gridN = 24;
		const float x0 = m_WaterCenter.x - m_WaterRadius;
		const float y0 = m_WaterCenter.y - m_WaterRadius;
		const float step = 2.f * m_WaterRadius / (float)gridN;
		const float z = reflInfo.PlaneZ + 0.05f; // slight offset over the water surface
		const float ooW = 1.f / (reflInfo.FrustumRight - reflInfo.FrustumLeft);
		const float ooH = 1.f / (reflInfo.FrustumTop - reflInfo.FrustumBottom);

		static std::vector<CQuadColorUV> quads;
		quads.clear();
		for (int j = 0; j < gridN; ++j)
		for (int i = 0; i < gridN; ++i)
		{
			CQuadColorUV q;
			CVector *vs[4] = { &q.V0, &q.V1, &q.V2, &q.V3 };
			CUV *uvs[4] = { &q.Uv0, &q.Uv1, &q.Uv2, &q.Uv3 };
			float cx[4] = { (float)i, (float)(i + 1), (float)(i + 1), (float)i };
			float cy[4] = { (float)j, (float)j, (float)(j + 1), (float)(j + 1) };
			for (int c = 0; c < 4; ++c)
			{
				CVector world(x0 + cx[c] * step, y0 + cy[c] * step, z);
				*vs[c] = world;
				CVector p = reflInfo.ReflViewMatrix * world;
				float invDepth = 1.f / max(p.y, reflInfo.FrustumNear);
				uvs[c]->U = (reflInfo.FrustumNear * p.x * invDepth - reflInfo.FrustumLeft) * ooW * reflInfo.UScale;
				uvs[c]->V = (reflInfo.FrustumNear * p.z * invDepth - reflInfo.FrustumBottom) * ooH * reflInfo.VScale;
			}
			q.Color0 = q.Color1 = q.Color2 = q.Color3 = CRGBA::White;
			quads.push_back(q);
		}
		m_MirrorMat.setTexture(0, reflInfo.Texture);
		m_Driver->drawQuads(quads, m_MirrorMat);
	}

	if (m_ShowRT && haveRefl)
	{
		// Reflection RT overlay, matching the planar_reflection sample:
		// fullscreen or corner thumbnail, full allocation shown
		m_Driver->setMatrixMode2D11();

		float ox0, oy0, ox1, oy1;
		if (m_ShowRT == 1) { ox0 = 0.f; oy0 = 0.f; ox1 = 1.f; oy1 = 1.f; }
		else { ox0 = 0.65f; oy0 = 0.65f; ox1 = 1.f; oy1 = 1.f; }

		CQuadColorUV rtQuad;
		rtQuad.V0.set(ox0, oy0, 0.5f); rtQuad.Uv0 = CUV(0.f, 1.f);
		rtQuad.V1.set(ox1, oy0, 0.5f); rtQuad.Uv1 = CUV(1.f, 1.f);
		rtQuad.V2.set(ox1, oy1, 0.5f); rtQuad.Uv2 = CUV(1.f, 0.f);
		rtQuad.V3.set(ox0, oy1, 0.5f); rtQuad.Uv3 = CUV(0.f, 0.f);
		rtQuad.Color0 = rtQuad.Color1 = rtQuad.Color2 = rtQuad.Color3 = CRGBA::White;
		m_MirrorMat.setTexture(0, reflInfo.Texture);
		m_Driver->drawQuad(rtQuad, m_MirrorMat);

		if (m_ShowRT == 2)
		{
			CRGBA borderColor(255, 255, 255);
			m_Driver->drawLine(ox0, oy0, ox1, oy0, borderColor);
			m_Driver->drawLine(ox1, oy0, ox1, oy1, borderColor);
			m_Driver->drawLine(ox1, oy1, ox0, oy1, borderColor);
			m_Driver->drawLine(ox0, oy1, ox0, oy0, borderColor);
		}
	}

	m_MirrorMat.setTexture(0, NULL);

	// --- HUD ---

	if (m_TextContext)
	{
		m_Driver->setMatrixMode2D11();

		m_TextContext->setHotSpot(UTextContext::TopLeft);
		m_TextContext->setColor(CRGBA::White);
		m_TextContext->setFontSize(12);

		float lineH = 0.025f;
		float x = 0.01f;
		float y = 1.f - 0.01f;

		m_TextContext->printfAt(x, y, "[F] Wireframe: %s", m_Wireframe ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[C] Camera orbit: %s", m_AnimCamera ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[R] Cube bobbing: %s", m_AnimCube ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[S] Skybox roll: %s", m_AnimSkybox ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[G] Fog: %s", m_Fog ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[P] Planar reflection: %s (%u active)", m_Planar ? "ON" : "OFF",
			m_Scene->getNumActiveWaterReflections());
		y -= lineH;
		m_TextContext->printfAt(x, y, "[M] Mirror floor debug: %s  [T] RT overlay: %s",
			m_Mirror ? "ON" : "OFF",
			m_ShowRT == 0 ? "off" : (m_ShowRT == 1 ? "fullscreen" : "thumbnail"));
		y -= lineH;
		m_TextContext->printfAt(x, y, "[H] Half-res: %s  [Y] Pow2: %s  [W] Fixed RT: %s",
			m_Scene->getWaterReflectionHalfRes() ? "ON" : "OFF",
			m_Scene->getWaterReflectionPow2() ? "ON" : "OFF",
			m_Scene->getWaterReflectionFixedSize() ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[V] VSync: %s", m_VSync ? "ON" : "OFF");
		y -= lineH;
		m_TextContext->printfAt(x, y, "[Up/Down] Camera dist: %.1f", m_CamDist);
		y -= lineH * 1.5f;
		if (m_Water.empty())
		{
			m_TextContext->setColor(CRGBA(255, 80, 80));
			m_TextContext->printfAt(x, y, "waterbassina01.shape NOT LOADED - check ryzomcore_graphics assets");
			m_TextContext->setColor(CRGBA::White);
		}
		else
		{
			m_TextContext->printfAt(x, y, "waterbassina01.shape (lacustre basin): radius %.1fm, %s",
				m_WaterRadius,
				m_Scene->getNumActiveWaterReflections() ? "REALTIME planar reflection" : "envmap reflection (static skymap)");
		}
		y -= lineH;
		if (m_Scene->getNumActiveWaterReflections())
			m_TextContext->printfAt(x, y, "Cube and skybox ARE in the reflection now - toggle [P] to compare");
		else
			m_TextContext->printfAt(x, y, "Note: cube and skybox are NOT in the reflection - that is the point");
		y -= lineH;
		m_TextContext->printfAt(x, y, "FPS: %.1f  (%.2f ms)", m_SmoothFps, dt * 1000.f);
	}

	m_Driver->swapBuffers();
}

#ifdef __EMSCRIPTEN__
static CWaterDemo *s_Demo = NULL;

static void emscriptenMainLoop()
{
	if (s_Demo)
		s_Demo->renderOneFrame();
}

// Called by the injected HTML control panel
extern "C" EMSCRIPTEN_KEEPALIVE void nlwater_option(int opt, int value)
{
	if (s_Demo)
		s_Demo->setOption(opt, value);
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
	static CWaterDemo demo;
	s_Demo = &demo;
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
	CWaterDemo demo;
	demo.run();
#endif

	return EXIT_SUCCESS;
}
