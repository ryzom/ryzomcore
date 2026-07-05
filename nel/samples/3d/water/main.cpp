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
//   N - Toggle the procedural pool field: a deterministic set of small
//       runtime-built water shapes (CWaterShape created in code, no assets),
//       each pool on its own plane at a different height, so every pool is
//       a separate reflection pass. Stresses the multi-plane budget and is
//       the testbed for reflection render target packing.
//   K/L - Fewer/more procedural pools
//   O - Cycle the reflection texture budget (unlimited, 1..4): passes pack
//       their active regions as tiles into shared render target textures;
//       when the budget is full, tiles shrink instead of dropping planes.
//       Watch the RT overlay [T] to see the pools tile into one texture.
//   B - Toggle the sky flare: a runtime-built CFlareShape with a procedural
//       blob texture. Bright reference point in reflections, and exercises
//       the flare occlusion query/fade machinery (per-context, including the
//       reflection contexts) in a controlled scene.
//   Up/Down - Move camera closer/farther
//

#include <nel/misc/types_nl.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/path.h>
#include <nel/misc/polygon.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>
#include <nel/3d/stereo_debugger.h>
#include <nel/3d/stereo_passthrough.h>

// Internal engine headers for the runtime-built shapes (procedural pools and
// flare): shapes are created in code and registered in the scene's shape
// bank under synthetic names, then instanced through the normal UScene path
#include <nel/3d/scene_user.h>
#include <nel/3d/shape_bank.h>
#include <nel/3d/water_shape.h>
#include <nel/3d/flare_shape.h>
#include <nel/3d/texture_mem.h>
#include <nel/3d/texture_file.h>

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

// Procedural pool field parameters (meters). Deterministic: the same count
// always produces the same pools.
static const float POOL_FIELD_RADIUS = 16.f;
static const float POOL_RADIUS = 3.5f;
static const float POOL_MIN_Z = 0.4f;
static const float POOL_MAX_Z = 3.4f;
static const uint POOL_MAX_COUNT = 12;
static const float POOL_CUBE_HALF_SIZE = 0.8f;

// Primary color palette per pool index: each pool's bobbing cube renders in
// shades of one hue, so a pool's reflection is attributable at a glance
static const CRGBA POOL_CUBE_COLORS[6] = {
	CRGBA(220,  40,  40), // red
	CRGBA( 40, 200,  40), // green
	CRGBA( 60, 100, 235), // blue
	CRGBA(235, 210,  40), // yellow
	CRGBA(210,  60, 210), // magenta
	CRGBA( 40, 205, 215), // cyan
};

// Deterministic integer hash (SplitMix-style avalanche) and derived [0,1[
// float; used instead of any random source so pool layouts are reproducible
static uint32 hash32(uint32 x)
{
	x ^= x >> 16; x *= 0x7feb352dU;
	x ^= x >> 15; x *= 0x846ca68bU;
	x ^= x >> 16;
	return x;
}

static float hash01(uint32 x)
{
	return float(hash32(x) & 0xffffff) / float(1 << 24);
}

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

// Shade a base color: f <= 1 darkens toward black, f > 1 lightens toward white
static CRGBA shadeColor(CRGBA c, float f)
{
	if (f <= 1.f)
		return CRGBA((uint8)(c.R * f), (uint8)(c.G * f), (uint8)(c.B * f));
	float t = f - 1.f;
	return CRGBA((uint8)(c.R + (255.f - c.R) * t),
		(uint8)(c.G + (255.f - c.G) * t),
		(uint8)(c.B + (255.f - c.B) * t));
}

// Draw a cube in shades of one base color: every face distinct (dark bottom,
// light top), the whole cube reads as one hue
static void drawCubePalette(UDriver *driver, UMaterial &mat, const CMatrix &transform, float s, CRGBA base)
{
	CVector v[8] = {
		transform * CVector(-s, -s, -s),
		transform * CVector( s, -s, -s),
		transform * CVector( s,  s, -s),
		transform * CVector(-s,  s, -s),
		transform * CVector(-s, -s,  s),
		transform * CVector( s, -s,  s),
		transform * CVector( s,  s,  s),
		transform * CVector(-s,  s,  s),
	};

	drawFace(driver, mat, v[3], v[2], v[1], v[0], shadeColor(base, 0.30f)); // bottom (Z-) darkest
	drawFace(driver, mat, v[4], v[5], v[6], v[7], shadeColor(base, 1.60f)); // top    (Z+) lightest
	drawFace(driver, mat, v[0], v[1], v[5], v[4], shadeColor(base, 0.55f)); // back   (Y-)
	drawFace(driver, mat, v[2], v[3], v[7], v[6], shadeColor(base, 1.00f)); // front  (Y+) base
	drawFace(driver, mat, v[3], v[0], v[4], v[7], shadeColor(base, 0.75f)); // left   (X-)
	drawFace(driver, mat, v[1], v[2], v[6], v[5], shadeColor(base, 1.30f)); // right  (X+)
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
		OptCamHeight = 11,
		OptStereo = 12,
		OptPools = 13,
		OptPoolCount = 14,
		OptFlare = 15,
		OptMaxTextures = 16
	};
	void setOption(int opt, int value);

private:
	void drawWorldContent(UDriver &driver, const NLMISC::CVector &eye);
	void renderScenePart(bool traverse, bool keep);
	void applyFootprint(const NLMISC::CVector &center, float radius);
	void applyMode();
	void updateReflectionBudget();
	void createProceduralPools();
	void destroyProceduralPools();
	void createFlare();
	void updateFlare();
	NL3D::IStereoDisplay *stereoDisplay() { return m_Stereo && m_StereoDebugger ? (NL3D::IStereoDisplay *)m_StereoDebugger : (NL3D::IStereoDisplay *)m_StereoPassthrough; }

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

	// Render loop manager: passthrough normally, stereo debugger when
	// toggled (per-eye replicated passes with a side-by-side composite)
	NL3D::CStereoPassthrough *m_StereoPassthrough;
	NL3D::CStereoDebugger *m_StereoDebugger;
	bool m_Stereo;
	UInstance m_Water;
	// Procedural pool field (runtime-built water shapes) and sky flare
	bool m_PoolsMode;
	uint m_PoolCount;
	bool m_FlareOn;
	std::vector<UInstance> m_Pools;
	std::vector<CVector> m_PoolPositions;
	UInstance m_Flare;
	CVector m_LakeCenter;
	float m_LakeRadius;
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
	, m_StereoPassthrough(NULL)
	, m_StereoDebugger(NULL)
	, m_Stereo(false)
	, m_Scene(NULL)
	, m_TextContext(NULL)
	, m_PoolsMode(false)
	, m_PoolCount(5)
	, m_FlareOn(true)
	, m_LakeCenter(0.f, 0.f, 0.f)
	, m_LakeRadius(10.f)
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

	m_StereoPassthrough = new CStereoPassthrough();
	m_StereoPassthrough->setDriver(m_Driver);
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
	m_PoolsMode = queryFlag("pools", m_PoolsMode ? 1 : 0) != 0;
	m_PoolCount = (uint)queryFlag("poolcount", (int)m_PoolCount);
	clamp(m_PoolCount, 1u, POOL_MAX_COUNT);
	m_FlareOn = queryFlag("flare", m_FlareOn ? 1 : 0) != 0;
	setOption(OptMaxTextures, queryFlag("maxrt", 0));
	if (queryFlag("stereo", 0) != 0)
		setOption(OptStereo, 1);
#endif
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
		m_LakeCenter = bbox.getCenter();
		CVector halfSize = bbox.getHalfSize();
		m_LakeRadius = max(1.f, max(halfSize.x, halfSize.y));
		nlinfo("Water shape loaded: center (%.1f %.1f %.1f), radius %.1f",
			m_LakeCenter.x, m_LakeCenter.y, m_LakeCenter.z, m_LakeRadius);
	}
	else
	{
		nlwarning("Failed to load waterbassina01.shape; check assets");
	}

	createFlare();
	applyMode(); // builds the pool field when requested; footprint + budget

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
			['Skybox roll', 9, $9],
			['Stereo debugger', 12, $12],
			['Procedural pools', 13, $13],
			['Sky flare', 15, $15]
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
		html += '<label style="display:block;margin-top:2px">Pool count <input type="range" data-opt="14" min="1" max="12" value="' + ($14 & 255) + '" style="width:90px;vertical-align:middle"></label>';
		html += '<label style="display:block;margin-top:2px">Max RT tex (0=any) <input type="range" data-opt="16" min="0" max="4" value="' + ($14 >> 8) + '" style="width:90px;vertical-align:middle"></label>';
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
		(int)m_CamHeight,
		m_Stereo ? 1 : 0,
		m_PoolsMode ? 1 : 0,
		// EM_ASM allows 16 args ($0..$15): pool count and the reflection
		// texture budget share one packed argument
		(int)m_PoolCount | ((m_Scene->getWaterReflectionMaxTextures() < 0 ? 0 : (int)m_Scene->getWaterReflectionMaxTextures()) << 8),
		m_FlareOn ? 1 : 0);
#endif
}

CWaterDemo::~CWaterDemo()
{
	delete m_StereoDebugger;
	m_StereoDebugger = NULL;
	delete m_StereoPassthrough;
	m_StereoPassthrough = NULL;
	m_Driver->deleteMaterial(m_MirrorMat);
	destroyProceduralPools();
	if (!m_Flare.empty())
		m_Scene->deleteInstance(m_Flare);
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
			if (keyDown.Key == KeyX) setOption(OptStereo, !m_Stereo);
			if (keyDown.Key == KeyV) { m_VSync = !m_VSync; m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0); }
			if (keyDown.Key == KeyN) setOption(OptPools, !m_PoolsMode);
			if (keyDown.Key == KeyK) setOption(OptPoolCount, (int)m_PoolCount - 1);
			if (keyDown.Key == KeyL) setOption(OptPoolCount, (int)m_PoolCount + 1);
			if (keyDown.Key == KeyB) setOption(OptFlare, !m_FlareOn);
			if (keyDown.Key == KeyO)
			{
				sint cur = m_Scene->getWaterReflectionMaxTextures();
				setOption(OptMaxTextures, cur < 0 ? 1 : (cur >= 4 ? 0 : (int)cur + 1));
			}
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
	case OptPlanar: m_Planar = value != 0; updateReflectionBudget(); break;
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
	case OptStereo:
		m_Stereo = value != 0;
		if (m_Stereo && !m_StereoDebugger)
		{
			// Created once and kept: its render targets live in the
			// driver's render target pool
			m_StereoDebugger = new CStereoDebugger();
			m_StereoDebugger->setDriver(m_Driver);
		}
		break;
	case OptPools:
		m_PoolsMode = value != 0;
		applyMode();
		break;
	case OptPoolCount:
	{
		uint count = (uint)max(1, min((int)POOL_MAX_COUNT, value));
		if (count != m_PoolCount)
		{
			m_PoolCount = count;
			if (m_PoolsMode)
				applyMode();
		}
		break;
	}
	case OptFlare:
		m_FlareOn = value != 0;
		updateFlare();
		break;
	case OptMaxTextures:
		// 0 = as many textures as needed; 1..4 = tile the passes into at
		// most that many shared textures per view
		m_Scene->setWaterReflectionMaxTextures(value <= 0 ? -1 : value);
		break;
	}
}

// Scale demo proportions (cube, camera limits) to the viewed water footprint
void CWaterDemo::applyFootprint(const CVector &center, float radius)
{
	m_WaterCenter = center;
	m_WaterRadius = radius;
	m_CubeHalfSize = radius * 0.08f;
	clamp(m_CubeHalfSize, 0.5f, 3.f);
	m_CamDist = radius * 1.1f;
	clamp(m_CamDist, 8.f, 80.f);
	m_CamHeight = radius * 0.3f;
	clamp(m_CamHeight, 3.f, 25.f);
	if (m_QueryCamDist > 0) m_CamDist = (float)m_QueryCamDist;
	if (m_QueryCamHeight > 0) m_CamHeight = (float)m_QueryCamHeight;
}

// Switch between the lake shape and the procedural pool field
void CWaterDemo::applyMode()
{
	if (m_PoolsMode)
	{
		if (!m_Water.empty())
			m_Water.hide();
		createProceduralPools();
		applyFootprint(CVector(0.f, 0.f, 0.5f * (POOL_MIN_Z + POOL_MAX_Z)), POOL_FIELD_RADIUS);
	}
	else
	{
		destroyProceduralPools();
		if (!m_Water.empty())
			m_Water.show();
		applyFootprint(m_LakeCenter, m_LakeRadius);
	}
	updateFlare();
	updateReflectionBudget();
}

// The budget covers every visible plane: 1 for the lake, one per pool in
// pools mode (each pool sits on its own plane)
void CWaterDemo::updateReflectionBudget()
{
	m_Scene->setMaxRealtimeWaterReflections(m_Planar ? (m_PoolsMode ? (sint)m_PoolCount : 1) : 0);
}

// Build the deterministic pool field: runtime-created CWaterShape objects
// registered in the scene's shape bank under synthetic names
void CWaterDemo::createProceduralPools()
{
	destroyProceduralPools();

	CScene &scene = static_cast<CSceneUser *>(m_Scene)->getScene();
	CShapeBank *bank = scene.getShapeBank();

	// Texture setup cloned from the lake shape when it is loaded; the same
	// named file textures otherwise (same files, already on the search path)
	CWaterShape *ref = dynamic_cast<CWaterShape *>(bank->getShape("waterbassina01.shape"));

	for (uint i = 0; i < m_PoolCount; ++i)
	{
		std::string name = toString("procpool%02u.shape", i);
		if (!bank->getShape(name))
		{
			CWaterShape *ws = new CWaterShape;

			// Weird-but-convex deterministic outline: vertices at jittered
			// angles on the unit circle (a polygon inscribed in a circle
			// with increasing angles is convex by construction), squashed
			// and rotated by a per-pool affine transform. Vertex count,
			// elongation and orientation all vary with the pool index.
			uint numVerts = 5 + hash32(i * 97 + 11) % 4;
			float sx = POOL_RADIUS * (0.55f + 0.45f * hash01(i * 97 + 23));
			float sy = POOL_RADIUS * (0.30f + 0.35f * hash01(i * 97 + 37));
			float rot = 2.f * float(Pi) * hash01(i * 97 + 51);
			float cr = cosf(rot), sr = sinf(rot);
			CPolygon2D poly;
			poly.Vertices.resize(numVerts);
			for (uint k = 0; k < numVerts; ++k)
			{
				// jitter strictly below half the angular step keeps the
				// angles increasing (and therefore the outline convex)
				float jitter = 0.8f * (hash01(i * 331 + k * 17 + 5) - 0.5f);
				float a = 2.f * float(Pi) * (float(k) + jitter) / float(numVerts);
				float px = sx * cosf(a);
				float py = sy * sinf(a);
				poly.Vertices[k].set(cr * px - sr * py, sr * px + cr * py);
			}
			ws->setShape(poly);

			if (ref)
			{
				for (uint e = 0; e < 2; ++e)
				{
					ws->setEnvMap(e, ref->getEnvMap(e));
					ws->setHeightMap(e, ref->getHeightMap(e));
					ws->setHeightMapScale(e, ref->getHeightMapScale(e));
					ws->setHeightMapSpeed(e, ref->getHeightMapSpeed(e));
				}
				ws->setWaveHeightFactor(ref->getWaveHeightFactor());
				ws->setTransitionRatio(ref->getTransitionRatio());
				ws->setWaterPoolID(ref->getWaterPoolID());
			}
			else
			{
				ws->setEnvMap(0, new CTextureFile("waterenvmap.tga"));
				ws->setEnvMap(1, new CTextureFile("waterunderenvmap.tga"));
				ws->setHeightMap(0, new CTextureFile("waterdisplace.tga"));
				ws->setHeightMap(1, new CTextureFile("waterbump.tga"));
				ws->setHeightMapScale(0, CVector2f(0.005f, 0.005f));
				ws->setHeightMapScale(1, CVector2f(0.01f, 0.01f));
				ws->setHeightMapSpeed(0, CVector2f(0.005f, 0.005f));
				ws->setHeightMapSpeed(1, CVector2f(0.01f, 0.01f));
			}
			ws->enableRealtimeReflection(true);
			bank->add(name, ws);
		}

		UInstance pool = m_Scene->createInstance(name);
		if (pool.empty())
			continue;
		// Golden-angle spiral placement so any count spreads evenly in view;
		// stepped heights put every pool on its own plane, so every pool is
		// its own reflection pass
		float ang = float(i) * 2.39996f;
		float rad = m_PoolCount > 1 ? POOL_FIELD_RADIUS * 0.78f * sqrtf((float(i) + 0.5f) / float(m_PoolCount)) : 0.f;
		float z = m_PoolCount > 1 ? POOL_MIN_Z + (POOL_MAX_Z - POOL_MIN_Z) * float(i) / float(m_PoolCount - 1) : 0.5f * (POOL_MIN_Z + POOL_MAX_Z);
		CVector pos(cosf(ang) * rad, sinf(ang) * rad, z);
		pool.setPos(pos);
		m_Pools.push_back(pool);
		m_PoolPositions.push_back(pos);
	}
}

void CWaterDemo::destroyProceduralPools()
{
	for (uint i = 0; i < m_Pools.size(); ++i)
	{
		if (!m_Pools[i].empty())
			m_Scene->deleteInstance(m_Pools[i]);
	}
	m_Pools.clear();
	m_PoolPositions.clear();
}

// Runtime-built sky flare: CFlareShape with a procedural radial blob
// texture, additive. Bright reference point in the reflections, and
// exercises the flare occlusion/fade machinery per context.
void CWaterDemo::createFlare()
{
	CScene &scene = static_cast<CSceneUser *>(m_Scene)->getScene();
	CShapeBank *bank = scene.getShapeBank();
	if (!bank->getShape("procflare.shape"))
	{
		// Radial blob: smooth falloff plus a hot core
		const uint ts = 64;
		uint8 *data = new uint8[ts * ts * 4];
		for (uint y = 0; y < ts; ++y)
		for (uint x = 0; x < ts; ++x)
		{
			float dx = (float(x) + 0.5f) / float(ts) - 0.5f;
			float dy = (float(y) + 0.5f) / float(ts) - 0.5f;
			float d = 2.f * sqrtf(dx * dx + dy * dy);
			float v = std::max(0.f, 1.f - d);
			float lum = std::min(1.f, 0.6f * v * v + powf(v, 8.f));
			uint8 c = (uint8)(255.f * lum + 0.5f);
			uint8 *px = data + 4 * (y * ts + x);
			px[0] = c; px[1] = c; px[2] = c; px[3] = c;
		}
		CTextureMem *tex = new CTextureMem(data, ts * ts * 4, true, false, ts, ts, CBitmap::RGBA);

		CFlareShape *fs = new CFlareShape;
		fs->setTexture(0, tex);
		fs->setSize(0, 5.f); // world units at the flare position
		fs->setColor(CRGBA(255, 235, 190));
		fs->setPersistence(1.f); // 1s fade, easy to eyeball
		fs->setMaxViewDist(2000.f);
		bank->add("procflare.shape", fs);
	}
	m_Flare = m_Scene->createInstance("procflare.shape");
	if (m_Flare.empty())
		nlwarning("Failed to create the flare instance");
	updateFlare();
}

// Place the flare high over the water footprint and apply its visibility
void CWaterDemo::updateFlare()
{
	if (m_Flare.empty())
		return;
	// Opposite the camera's start side (the orbit begins on +X looking -X)
	// and low enough to sit inside the default frustum, so the flare is in
	// view immediately even with the orbit off (the flare model clips as a
	// point: off-frustum means gone entirely, not partially)
	m_Flare.setPos(CVector(m_WaterCenter.x - m_WaterRadius * 1.1f,
		m_WaterCenter.y + m_WaterRadius * 0.6f,
		m_WaterCenter.z + m_WaterRadius * 0.55f));
	if (m_FlareOn)
		m_Flare.show();
	else
		m_Flare.hide();
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

	// Pools mode: one bobbing cube above each pool, each in shades of its
	// own primary color, so each pool's reflected content is attributable
	for (uint i = 0; i < m_PoolPositions.size(); ++i)
	{
		float bob = sinf(m_BobPhase * 1.3f + float(i) * 1.7f) * POOL_CUBE_HALF_SIZE * 1.2f + POOL_CUBE_HALF_SIZE * 1.1f;
		CMatrix t;
		t.identity();
		t.setPos(m_PoolPositions[i] + CVector(0.f, 0.f, bob));
		t.rotateZ(m_CubeAngle + float(i) * 0.8f + float(Pi / 4.0));
		t.rotateX(0.9553f); // corner down, like the reference cube
		drawCubePalette(&driver, m_CubeMat, t, POOL_CUBE_HALF_SIZE, POOL_CUBE_COLORS[i % 6]);
	}
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

// Replicated scene render for one pass of the render loop: traverse on the
// first pass of a group, re-render the kept traversal on the others.
// The HRC pass always runs, matching the client's render loop: water
// rendering diverges between replicated renders without it (verified with
// the stereo debugger's comparison composite).
void CWaterDemo::renderScenePart(bool traverse, bool keep)
{
	m_Scene->beginPartRender();
	m_Scene->renderPart(UScene::RenderAll, true, traverse, keep);
	m_Scene->endPartRender(true, true, keep);
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

	// --- Render loop: driven by the render loop manager (stereo display),
	// which replicates passes — per water reflection pass, and per eye when
	// the stereo debugger is enabled. Same structure as the client's
	// main loop. ---

	IStereoDisplay *display = stereoDisplay();
	display->setSceneReflectionPasses(m_Scene->beginWaterReflectionPasses());

	while (display->nextPass())
	{
		const CViewport &vp = display->getCurrentViewport();
		m_Driver->setViewport(vp);
		m_Scene->setViewport(vp);

		// Per-pass flare occlusion context, like the client's render loop:
		// reflection passes must not share the main view's context — their
		// occlusion queries test the mirrored view's depth (a flare that is
		// off the mirrored frustum would pin the shared fade to zero)
		m_Scene->setFlareContext(display->getFlareContext());

		display->beginRenderTarget();

		if (display->wantClear())
		{
			m_Driver->clearBuffers(CRGBA(75, 95, 125));
		}

		if (display->wantSceneReflections())
		{
			// One water reflection pass (one plane, one eye). The eye
			// stages of a pass are adjacent: the second eye re-renders the
			// first eye's traversal. Reflections are never the frame's
			// last render, so traversals (and the frame's ellapsed time)
			// are always kept.
			m_Scene->setWaterReflectionView(display->getSceneView());
			uint reflPass = display->getSceneReflectionPass();
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
			activeVP.init(passInfo.UBias, passInfo.VBias, passInfo.UScale, passInfo.VScale);
			m_Driver->setViewport(activeVP);
			CScissor activeScissor;
			activeScissor.init(passInfo.UBias, passInfo.VBias, passInfo.UScale, passInfo.VScale);
			m_Driver->setScissor(activeScissor);
			m_Driver->setFrustum(reflFrustum);
			m_Driver->setViewMatrix(passInfo.ReflViewMatrix);
			m_Driver->setModelMatrix(identity);

			// Same content as the main pass (water itself is filtered out
			// of the scene render)
			drawWorldContent(*m_Driver, reflCamWorld.getPos());
			renderScenePart(display->isSceneFirst(), true);

			m_Scene->endWaterReflectionPass(reflPass);

			// Restore our driver context for the next pass
			m_Driver->setViewport(vp);
			CScissor fullScissor;
			fullScissor.initFullScreen();
			m_Driver->setScissor(fullScissor);
		}

		if (display->wantScene())
		{
			m_Scene->setWaterReflectionView(display->getSceneView());

			m_Driver->setFrustum(frustum);
			m_Driver->setViewMatrix(viewMatrix);
			m_Driver->setModelMatrix(identity);

			drawWorldContent(*m_Driver, eye);

			// The water surface (blends over the cube and skybox), sampling
			// this eye's reflections
			renderScenePart(display->isSceneFirst(), !display->isSceneLast());

			// --- Debug: mirror floor (world-space, per eye) ---
			UWaterReflectionInfo reflInfo;
			if (m_Mirror && m_Scene->getActiveWaterReflectionInfo(0, reflInfo))
			{
				// Draw the raw reflection as an opaque mirror floor over the
				// water: a world-space grid across the water footprint, each
				// vertex projected through the engine's reflected camera and
				// sub-frustum. This replicates the planar_reflection
				// sample's floor using the engine-provided data, bypassing
				// the water shader entirely.
				m_Driver->enableFog(false);
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
						CVector pv = reflInfo.ReflViewMatrix * world;
						float invDepth = 1.f / max(pv.y, reflInfo.FrustumNear);
						uvs[c]->U = (reflInfo.FrustumNear * pv.x * invDepth - reflInfo.FrustumLeft) * ooW * reflInfo.UScale + reflInfo.UBias;
						uvs[c]->V = (reflInfo.FrustumNear * pv.z * invDepth - reflInfo.FrustumBottom) * ooH * reflInfo.VScale + reflInfo.VBias;
					}
					q.Color0 = q.Color1 = q.Color2 = q.Color3 = CRGBA::White;
					quads.push_back(q);
				}
				m_MirrorMat.setTexture(0, reflInfo.Texture);
				m_Driver->drawQuads(quads, m_MirrorMat);
				m_MirrorMat.setTexture(0, NULL);
				if (m_Fog)
					m_Driver->enableFog(true);
			}
		}

		if (display->wantInterface2D())
		{
			m_Driver->enableFog(false);

			// --- Debug: reflection RT overlay ---
			UWaterReflectionInfo reflInfo;
			if (m_ShowRT && m_Scene->getActiveWaterReflectionInfo(0, reflInfo))
			{
				// Reflection RT overlay, matching the planar_reflection
				// sample: fullscreen or corner thumbnail, full allocation
				// shown (the current view's — the last eye in stereo)
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
				m_MirrorMat.setTexture(0, NULL);

				if (m_ShowRT == 2)
				{
					CRGBA borderColor(255, 255, 255);
					m_Driver->drawLine(ox0, oy0, ox1, oy0, borderColor);
					m_Driver->drawLine(ox1, oy0, ox1, oy1, borderColor);
					m_Driver->drawLine(ox1, oy1, ox0, oy1, borderColor);
					m_Driver->drawLine(ox0, oy1, ox0, oy0, borderColor);
				}
			}

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
				m_TextContext->printfAt(x, y, "[X] Stereo debugger: %s  [V] VSync: %s",
					m_Stereo ? "ON" : "OFF", m_VSync ? "ON" : "OFF");
				y -= lineH;
				m_TextContext->printfAt(x, y, "[N] Procedural pools: %s  [K/L] Pool count: %u  [B] Sky flare: %s",
					m_PoolsMode ? "ON" : "OFF", m_PoolCount, m_FlareOn ? "ON" : "OFF");
				y -= lineH;
				if (m_Scene->getWaterReflectionMaxTextures() < 0)
					m_TextContext->printfAt(x, y, "[O] Max reflection textures: unlimited");
				else
					m_TextContext->printfAt(x, y, "[O] Max reflection textures: %d", (int)m_Scene->getWaterReflectionMaxTextures());
				y -= lineH;
				m_TextContext->printfAt(x, y, "[Up/Down] Camera dist: %.1f", m_CamDist);
				y -= lineH * 1.5f;
				if (m_PoolsMode)
				{
					m_TextContext->printfAt(x, y, "%u runtime-built pools, each on its own plane: %u active reflections",
						(uint)m_Pools.size(), m_Scene->getNumActiveWaterReflections());
				}
				else if (m_Water.empty())
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
				y -= lineH;
				m_TextContext->printfAt(x, y, "Build: %s %s", __DATE__, __TIME__);
			}
		}

		display->endRenderTarget();
	}
	m_Scene->endWaterReflectionPasses();
	m_Driver->enableFog(false);

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
