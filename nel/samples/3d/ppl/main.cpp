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
// Per-Pixel Lighting Demo
//
// Demonstrates the light table API with per-pixel lighting (PPL).
// A gray sphere (vertex-colored) and a light gray cylinder (material-colored)
// sit on a large floor plane, illuminated by 8 colored point lights orbiting
// at different heights and speeds.
//
// Each light can be toggled between PPL and vertex-lit evaluation using
// keys 1-8. Up/Down arrows scale global light brightness.
//
// Controls:
//   1-8   Toggle light N between PPL and vertex-lit
//   Up    Increase global light brightness
//   Down  Decrease global light brightness
//   G     Toggle fog
//   F     Toggle wireframe
//   C     Toggle camera orbit
//   V     Toggle VSync
//

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/driver.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/material.h>
#include <nel/3d/light.h>
#include <nel/3d/frustum.h>

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

// Generate a UV sphere into the given vertex buffer and index buffer.
// Sphere is centered at origin with the given radius.
// VB format: Position + Normal + PrimaryColor
static void buildSphere(CVertexBuffer &vb, CIndexBuffer &ib, float radius, uint slices, uint stacks, CRGBA color)
{
	uint numVerts = (stacks + 1) * (slices + 1);
	uint numTris = stacks * slices * 2;

	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::PrimaryColorFlag);
	vb.setPreferredMemory(CVertexBuffer::StaticPreferred, false);
	vb.setNumVertices(numVerts);

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setPreferredMemory(CIndexBuffer::StaticPreferred, false);
	ib.setNumIndexes(numTris * 3);

	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);

		for (uint j = 0; j <= stacks; ++j)
		{
			float phi = (float)Pi * (float)j / (float)stacks;
			float sp = sinf(phi);
			float cp = cosf(phi);

			for (uint i = 0; i <= slices; ++i)
			{
				float theta = 2.f * (float)Pi * (float)i / (float)slices;
				float st = sinf(theta);
				float ct = cosf(theta);

				// NeL: X right, Y forward, Z up
				CVector n(sp * ct, sp * st, cp);
				CVector p = n * radius;

				uint idx = j * (slices + 1) + i;
				vba.setVertexCoord(idx, p);
				vba.setNormalCoord(idx, n);
				vba.setColor(idx, color);
			}
		}
	}

	{
		CIndexBufferReadWrite iba;
		ib.lock(iba);

		uint triIdx = 0;
		for (uint j = 0; j < stacks; ++j)
		{
			for (uint i = 0; i < slices; ++i)
			{
				uint a = j * (slices + 1) + i;
				uint b = a + slices + 1;

				iba.setTri(triIdx * 3, a, b, a + 1);
				triIdx++;
				iba.setTri(triIdx * 3, a + 1, b, b + 1);
				triIdx++;
			}
		}
	}
}

// Generate a cylinder along Z axis, centered at origin, with given radius and height.
// VB format: Position + Normal (no vertex color)
static void buildCylinder(CVertexBuffer &vb, CIndexBuffer &ib, float radius, float height, uint segments)
{
	// Vertices: 2 center + 2 * (segments+1) rim
	uint numRim = segments + 1;
	uint numVerts = 2 + 2 * numRim + 2 * numRim; // top center + bottom center + top rim + bottom rim + top side + bottom side
	// Simplify: separate cap vertices (with cap normals) from side vertices (with outward normals)
	// Cap top: center + ring = 1 + numRim
	// Cap bottom: center + ring = 1 + numRim
	// Side: top ring + bottom ring = 2 * numRim
	numVerts = 2 * (1 + numRim) + 2 * numRim;

	// Triangles: top cap fan + bottom cap fan + side quads
	uint numTris = segments + segments + segments * 2;

	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag);
	vb.setPreferredMemory(CVertexBuffer::StaticPreferred, false);
	vb.setNumVertices(numVerts);

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setPreferredMemory(CIndexBuffer::StaticPreferred, false);
	ib.setNumIndexes(numTris * 3);

	float halfH = height * 0.5f;

	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);

		uint v = 0;

		// --- Top cap ---
		uint topCenter = v;
		vba.setVertexCoord(v, CVector(0.f, 0.f, halfH));
		vba.setNormalCoord(v, CVector(0.f, 0.f, 1.f));
		v++;

		uint topRimStart = v;
		for (uint i = 0; i <= segments; ++i)
		{
			float a = 2.f * (float)Pi * (float)i / (float)segments;
			float ca = cosf(a), sa = sinf(a);
			vba.setVertexCoord(v, CVector(ca * radius, sa * radius, halfH));
			vba.setNormalCoord(v, CVector(0.f, 0.f, 1.f));
			v++;
		}

		// --- Bottom cap ---
		uint bottomCenter = v;
		vba.setVertexCoord(v, CVector(0.f, 0.f, -halfH));
		vba.setNormalCoord(v, CVector(0.f, 0.f, -1.f));
		v++;

		uint bottomRimStart = v;
		for (uint i = 0; i <= segments; ++i)
		{
			float a = 2.f * (float)Pi * (float)i / (float)segments;
			float ca = cosf(a), sa = sinf(a);
			vba.setVertexCoord(v, CVector(ca * radius, sa * radius, -halfH));
			vba.setNormalCoord(v, CVector(0.f, 0.f, -1.f));
			v++;
		}

		// --- Side ---
		uint sideTopStart = v;
		for (uint i = 0; i <= segments; ++i)
		{
			float a = 2.f * (float)Pi * (float)i / (float)segments;
			float ca = cosf(a), sa = sinf(a);
			CVector normal(ca, sa, 0.f);
			vba.setVertexCoord(v, CVector(ca * radius, sa * radius, halfH));
			vba.setNormalCoord(v, normal);
			v++;
		}

		uint sideBottomStart = v;
		for (uint i = 0; i <= segments; ++i)
		{
			float a = 2.f * (float)Pi * (float)i / (float)segments;
			float ca = cosf(a), sa = sinf(a);
			CVector normal(ca, sa, 0.f);
			vba.setVertexCoord(v, CVector(ca * radius, sa * radius, -halfH));
			vba.setNormalCoord(v, normal);
			v++;
		}

		// --- Index buffer ---
		CIndexBufferReadWrite iba;
		ib.lock(iba);

		uint tri = 0;

		// Top cap fan (CCW when viewed from above = +Z)
		for (uint i = 0; i < segments; ++i)
		{
			iba.setTri(tri * 3, topCenter, topRimStart + i, topRimStart + i + 1);
			tri++;
		}

		// Bottom cap fan (reversed winding for -Z face)
		for (uint i = 0; i < segments; ++i)
		{
			iba.setTri(tri * 3, bottomCenter, bottomRimStart + i + 1, bottomRimStart + i);
			tri++;
		}

		// Side quads
		for (uint i = 0; i < segments; ++i)
		{
			uint st = sideTopStart + i;
			uint sb = sideBottomStart + i;
			iba.setTri(tri * 3, st, sb, st + 1);
			tri++;
			iba.setTri(tri * 3, st + 1, sb, sb + 1);
			tri++;
		}
	}
}

// Generate a floor quad at Z=0 with normals pointing up.
// VB format: Position + Normal + PrimaryColor
static void buildFloor(CVertexBuffer &vb, CIndexBuffer &ib, float halfSize, CRGBA color)
{
	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::PrimaryColorFlag);
	vb.setPreferredMemory(CVertexBuffer::StaticPreferred, false);
	vb.setNumVertices(4);

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setPreferredMemory(CIndexBuffer::StaticPreferred, false);
	ib.setNumIndexes(6);

	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);

		CVector n(0.f, 0.f, 1.f);
		vba.setVertexCoord(0, CVector(-halfSize, -halfSize, 0.f));
		vba.setNormalCoord(0, n);
		vba.setColor(0, color);

		vba.setVertexCoord(1, CVector( halfSize, -halfSize, 0.f));
		vba.setNormalCoord(1, n);
		vba.setColor(1, color);

		vba.setVertexCoord(2, CVector( halfSize,  halfSize, 0.f));
		vba.setNormalCoord(2, n);
		vba.setColor(2, color);

		vba.setVertexCoord(3, CVector(-halfSize,  halfSize, 0.f));
		vba.setNormalCoord(3, n);
		vba.setColor(3, color);
	}

	{
		CIndexBufferReadWrite iba;
		ib.lock(iba);

		iba.setTri(0, 0, 1, 2);
		iba.setTri(3, 0, 2, 3);
	}
}

// Light configuration: color, orbit radius, orbit height, orbit speed
struct SLightConfig
{
	CRGBA Color;
	float OrbitRadius;
	float OrbitHeight;
	float OrbitSpeed;
};

static const SLightConfig LightConfigs[8] = {
	{ CRGBA(255,  60,  60), 6.f, 3.0f,  0.5f },  // Red
	{ CRGBA(255, 160,  40), 7.f, 2.0f,  0.4f },  // Orange
	{ CRGBA(255, 255,  60), 5.f, 4.0f,  0.6f },  // Yellow
	{ CRGBA( 60, 255,  60), 8.f, 1.5f,  0.35f }, // Green
	{ CRGBA( 60, 255, 255), 6.f, 3.5f,  0.55f }, // Cyan
	{ CRGBA( 60,  60, 255), 7.f, 2.5f,  0.45f }, // Blue
	{ CRGBA(200,  60, 255), 5.f, 4.5f,  0.65f }, // Purple
	{ CRGBA(255, 255, 255), 9.f, 1.0f,  0.3f },  // White
};

// PPL demo application
class CPPLDemo : public IEventListener
{
public:
	CPPLDemo();
	~CPPLDemo();
	void run();

	virtual void operator()(const CEvent &event) NL_OVERRIDE;

private:
	bool m_CloseWindow;
	bool m_Wireframe;
	bool m_AnimCamera;
	bool m_FogEnabled;
	bool m_VSync;
	bool m_KeyUp;
	bool m_KeyDown;
	bool m_LightPPL[8]; // true = PPL, false = vertex-lit
	bool m_LightEnabled[8]; // true = on, false = off
	float m_Brightness;

	UDriver *m_Driver;
	UTextContext *m_TextContext;

	CVertexBuffer m_SphereVB;
	CIndexBuffer m_SphereIB;
	CVertexBuffer m_CylVB;
	CIndexBuffer m_CylIB;
	CVertexBuffer m_FloorVB;
	CIndexBuffer m_FloorIB;

	CMaterial m_SphereMat;
	CMaterial m_CylMat;
	CMaterial m_FloorMat;
};

CPPLDemo::CPPLDemo()
	: m_CloseWindow(false)
	, m_Wireframe(false)
	, m_AnimCamera(true)
	, m_FogEnabled(true)
	, m_VSync(true)
	, m_KeyUp(false)
	, m_KeyDown(false)
	, m_Brightness(1.f)
	, m_TextContext(NULL)
{
	for (uint i = 0; i < 8; ++i)
	{
		m_LightPPL[i] = true;
		m_LightEnabled[i] = true;
	}

	m_Driver = UDriver::createDriver(0, UDriver::OpenGl3);
	if (!m_Driver)
	{
		nlerror("Failed to create driver");
		return;
	}

	m_Driver->EventServer.addListener(EventCloseWindowId, this);
	m_Driver->EventServer.addListener(EventKeyDownId, this);
	m_Driver->EventServer.addListener(EventKeyUpId, this);

	m_Driver->setDisplay(UDriver::CMode(1024, 768, 32, true));
	m_Driver->setWindowTitle(ucstring("NeL Per-Pixel Lighting Demo"));
	m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0);

	// Initialize text renderer
	std::string fontPath = findFontPath();
	if (!fontPath.empty())
	{
		m_TextContext = m_Driver->createTextContext(fontPath);
		if (m_TextContext)
			m_TextContext->setFontSize(12);
	}

	// Build geometry
	buildSphere(m_SphereVB, m_SphereIB, 1.5f, 32, 16, CRGBA(160, 160, 160));
	buildCylinder(m_CylVB, m_CylIB, 0.8f, 3.f, 32);
	buildFloor(m_FloorVB, m_FloorIB, 20.f, CRGBA(100, 100, 100));

	// Sphere material: lit, vertex color provides diffuse
	m_SphereMat.initLighted();
	m_SphereMat.setDiffuse(CRGBA::White);
	m_SphereMat.setAmbient(CRGBA(30, 30, 30));
	m_SphereMat.setSpecular(CRGBA(200, 200, 200));
	m_SphereMat.setShininess(32.f);

	// Cylinder material: lit, material diffuse provides color
	m_CylMat.initLighted();
	m_CylMat.setDiffuse(CRGBA(200, 200, 200));
	m_CylMat.setAmbient(CRGBA(30, 30, 30));
	m_CylMat.setSpecular(CRGBA(200, 200, 200));
	m_CylMat.setShininess(32.f);
	m_CylMat.setLightedVertexColor(true);

	// Floor material: lit, vertex color provides diffuse
	m_FloorMat.initLighted();
	m_FloorMat.setDiffuse(CRGBA::White);
	m_FloorMat.setAmbient(CRGBA(30, 30, 30));
	m_FloorMat.setSpecular(CRGBA::Black);
	m_FloorMat.setShininess(1.f);
}

CPPLDemo::~CPPLDemo()
{
	if (m_TextContext)
		m_Driver->deleteTextContext(m_TextContext);
	m_Driver->release();
	delete m_Driver;
}

void CPPLDemo::operator()(const CEvent &event)
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
			if (keyDown.Key == KeyG) m_FogEnabled = !m_FogEnabled;
			if (keyDown.Key == KeyV) { m_VSync = !m_VSync; m_Driver->setSwapVBLInterval(m_VSync ? 1 : 0); }

			// Shift+1-8 toggle light on/off, 1-8 toggle PPL/vertex
			// 0 toggles all PPL/vertex, Shift+0 toggles all on/off
			if (keyDown.Key >= Key1 && keyDown.Key <= Key8)
			{
				uint li = keyDown.Key - Key1;
				if (keyDown.Button & shiftKeyButton)
					m_LightEnabled[li] = !m_LightEnabled[li];
				else
					m_LightPPL[li] = !m_LightPPL[li];
			}
			if (keyDown.Key == Key0)
			{
				if (keyDown.Button & shiftKeyButton)
				{
					// Toggle all on/off: if any are on, turn all off; else turn all on
					bool anyOn = false;
					for (uint i = 0; i < 8; ++i) if (m_LightEnabled[i]) { anyOn = true; break; }
					for (uint i = 0; i < 8; ++i) m_LightEnabled[i] = !anyOn;
				}
				else
				{
					// Toggle all PPL/vertex: if any are PPL, set all vertex; else set all PPL
					bool anyPPL = false;
					for (uint i = 0; i < 8; ++i) if (m_LightPPL[i]) { anyPPL = true; break; }
					for (uint i = 0; i < 8; ++i) m_LightPPL[i] = !anyPPL;
				}
			}
		}
		if (keyDown.Key == KeyUP) m_KeyUp = true;
		if (keyDown.Key == KeyDOWN) m_KeyDown = true;
	}
	else if (event == EventKeyUpId)
	{
		CEventKeyUp &keyUp = (CEventKeyUp &)event;
		if (keyUp.Key == KeyUP) m_KeyUp = false;
		if (keyUp.Key == KeyDOWN) m_KeyDown = false;
	}
}

void CPPLDemo::run()
{
	IDriver *drv = static_cast<CDriverUser *>(m_Driver)->getDriver();

	CFrustum frustum;

	float camAngle = 0.f;
	float camDist = 15.f;
	float camHeight = 8.f;
	float lightTime = 0.f;

	double lastTime = CTime::ticksToSecond(CTime::getPerformanceTime());
	float smoothFps = 60.f;

	while (m_Driver->isActive() && !m_CloseWindow)
	{
		m_Driver->EventServer.pump();

		uint32 screenW, screenH;
		m_Driver->getWindowSize(screenW, screenH);
		if (screenW == 0 || screenH == 0) { nlSleep(10); continue; }
		frustum.initPerspective(float(Pi / 3.0), float(screenW) / float(screenH), 0.1f, 200.f);

		double now = CTime::ticksToSecond(CTime::getPerformanceTime());
		float dt = float(now - lastTime);
		lastTime = now;
		if (dt > 0.f) smoothFps += (1.f / dt - smoothFps) * min(1.f, dt * 5.f);

		if (m_AnimCamera) camAngle += dt * 0.3f;
		lightTime += dt;

		// Brightness control
		if (m_KeyUp) m_Brightness += dt * 1.f;
		if (m_KeyDown) m_Brightness -= dt * 1.f;
		if (m_Brightness < 0.f) m_Brightness = 0.f;
		if (m_Brightness > 5.f) m_Brightness = 5.f;

		m_Driver->setPolygonMode(m_Wireframe ? UDriver::Line : UDriver::Filled);

		// --- Camera setup ---

		CVector eye(cosf(camAngle) * camDist, sinf(camAngle) * camDist, camHeight);
		CVector target(0.f, 0.f, 1.5f);
		CVector up(0.f, 0.f, 1.f);
		CMatrix viewMatrix = buildViewMatrix(eye, target, up);

		CMatrix modelIdentity;
		modelIdentity.identity();

		// --- Fog setup ---

		if (m_FogEnabled)
		{
			drv->enableFog(true);
			drv->setupFog(10.f, 80.f, CRGBA(80, 20, 120));
			drv->setupFogMode(IDriver::FogLinear);
		}
		else
		{
			drv->enableFog(false);
		}

		// --- Clear and set matrices ---

		CRGBA clearColor = m_FogEnabled ? CRGBA(80, 20, 120) : CRGBA(20, 20, 30);
		m_Driver->clearBuffers(clearColor);

		m_Driver->setFrustum(frustum);
		m_Driver->setViewMatrix(viewMatrix);
		m_Driver->setModelMatrix(modelIdentity);

		// --- Setup light table ---

		drv->enableLightTableMode(true);
		drv->setLightTableSize(8);

		CLight lights[8];
		for (uint i = 0; i < 8; ++i)
		{
			float angle = lightTime * LightConfigs[i].OrbitSpeed + float(i) * 2.f * (float)Pi / 8.f;
			float r = LightConfigs[i].OrbitRadius;
			float h = LightConfigs[i].OrbitHeight;
			CVector pos(cosf(angle) * r, sinf(angle) * r, h);

			// Scale color by brightness
			CRGBA baseColor = LightConfigs[i].Color;
			uint8 dr = (uint8)min(255.f, float(baseColor.R) * m_Brightness);
			uint8 dg = (uint8)min(255.f, float(baseColor.G) * m_Brightness);
			uint8 db = (uint8)min(255.f, float(baseColor.B) * m_Brightness);
			CRGBA diffuse(dr, dg, db);

			// Specular at half brightness
			uint8 sr = (uint8)min(255.f, float(baseColor.R) * m_Brightness * 0.5f);
			uint8 sg = (uint8)min(255.f, float(baseColor.G) * m_Brightness * 0.5f);
			uint8 sb = (uint8)min(255.f, float(baseColor.B) * m_Brightness * 0.5f);
			CRGBA specular(sr, sg, sb);

			lights[i].setupPointLight(CRGBA::Black, diffuse, specular, pos, CVector(0.f, 0.f, -1.f));
			lights[i].setupAttenuation(2.f, 12.f);

			drv->setLightTableEntry(i, lights[i]);
		}

		// --- Render objects ---

		// Sort lights: enabled PPL first, then enabled vertex, skip disabled
		sint16 indices[8];
		uint8 factors[8];
		uint numPPL = 0;
		uint numTotal = 0;

		// PPL lights first
		for (uint i = 0; i < 8; ++i)
		{
			if (m_LightEnabled[i] && m_LightPPL[i])
			{
				indices[numTotal++] = (sint16)i;
				numPPL++;
			}
		}
		// Then vertex lights
		for (uint i = 0; i < 8; ++i)
		{
			if (m_LightEnabled[i] && !m_LightPPL[i])
				indices[numTotal++] = (sint16)i;
		}

		for (uint i = 0; i < numTotal; ++i)
			factors[i] = 255;

		CRGBA ambient(20, 20, 25);

		drv->setLights(indices, factors, numTotal, numPPL, ambient);

		// Render sphere at position (-3, 0, 1.5)
		{
			CMatrix sphereModel;
			sphereModel.identity();
			sphereModel.setPos(CVector(-3.f, 0.f, 1.5f));
			m_Driver->setModelMatrix(sphereModel);

			drv->activeVertexBuffer(m_SphereVB);
			drv->activeIndexBuffer(m_SphereIB);
			drv->renderTriangles(m_SphereMat, 0, m_SphereIB.getNumIndexes() / 3);
		}

		// Render cylinder at position (3, 0, 0)
		{
			CMatrix cylModel;
			cylModel.identity();
			cylModel.setPos(CVector(3.f, 0.f, 1.5f));
			m_Driver->setModelMatrix(cylModel);

			drv->activeVertexBuffer(m_CylVB);
			drv->activeIndexBuffer(m_CylIB);
			drv->renderTriangles(m_CylMat, 0, m_CylIB.getNumIndexes() / 3);
		}

		// Render floor
		{
			m_Driver->setModelMatrix(modelIdentity);

			drv->activeVertexBuffer(m_FloorVB);
			drv->activeIndexBuffer(m_FloorIB);
			drv->renderTriangles(m_FloorMat, 0, 2);
		}

		// Disable light table mode
		drv->enableLightTableMode(false);

		// --- Draw light position indicators (small unlit quads) ---
		{
			m_Driver->setModelMatrix(modelIdentity);
			for (uint i = 0; i < 8; ++i)
			{
				float angle = lightTime * LightConfigs[i].OrbitSpeed + float(i) * 2.f * (float)Pi / 8.f;
				float r = LightConfigs[i].OrbitRadius;
				float h = LightConfigs[i].OrbitHeight;
				CVector pos(cosf(angle) * r, sinf(angle) * r, h);

				float s = 0.15f;
				CRGBA c = LightConfigs[i].Color;
				if (!m_LightEnabled[i])
				{
					// Very dim for disabled lights
					c.R /= 6; c.G /= 6; c.B /= 6;
				}
				else if (!m_LightPPL[i])
				{
					// Dim vertex-lit indicators
					c.R /= 2; c.G /= 2; c.B /= 2;
				}

				CQuadColor q;
				q.V0 = pos + CVector(-s, 0, -s);
				q.V1 = pos + CVector( s, 0, -s);
				q.V2 = pos + CVector( s, 0,  s);
				q.V3 = pos + CVector(-s, 0,  s);
				q.Color0 = q.Color1 = q.Color2 = q.Color3 = c;

				UMaterial mat = m_Driver->createMaterial();
				mat.initUnlit();
				mat.setZWrite(false);
				mat.setDoubleSided(true);
				m_Driver->drawQuad(q, mat);
				m_Driver->deleteMaterial(mat);
			}
		}

		// --- HUD ---

		if (m_TextContext)
		{
			m_TextContext->setHotSpot(UTextContext::TopLeft);
			m_TextContext->setColor(CRGBA::White);
			m_TextContext->setFontSize(12);

			float lineH = 0.025f;
			float x = 0.01f;
			float y = 1.f - 0.01f;

			m_TextContext->printfAt(x, y, "Per-Pixel Lighting Demo");
			y -= lineH * 1.5f;

			// Light status
			for (uint i = 0; i < 8; ++i)
			{
				const char *names[] = { "Red", "Orange", "Yellow", "Green", "Cyan", "Blue", "Purple", "White" };
				if (m_LightEnabled[i])
					m_TextContext->printfAt(x, y, "[%d] %s: %s  [Shift+%d] ON", i + 1, names[i], m_LightPPL[i] ? "PPL" : "Vertex", i + 1);
				else
					m_TextContext->printfAt(x, y, "[%d] %s: ---  [Shift+%d] OFF", i + 1, names[i], i + 1);
				y -= lineH;
			}

			y -= lineH * 0.5f;
			m_TextContext->printfAt(x, y, "[Up/Down] Brightness: %.2f", m_Brightness);
			y -= lineH;
			m_TextContext->printfAt(x, y, "[G] Fog: %s", m_FogEnabled ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[F] Wireframe: %s", m_Wireframe ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[C] Camera orbit: %s", m_AnimCamera ? "ON" : "OFF");
			y -= lineH;
			m_TextContext->printfAt(x, y, "[V] VSync: %s", m_VSync ? "ON" : "OFF");
			y -= lineH * 1.5f;

			uint pplCount = 0, vertCount = 0, offCount = 0;
			for (uint i = 0; i < 8; ++i)
			{
				if (!m_LightEnabled[i]) offCount++;
				else if (m_LightPPL[i]) pplCount++;
				else vertCount++;
			}
			m_TextContext->printfAt(x, y, "PPL: %u  Vertex: %u  Off: %u", pplCount, vertCount, offCount);
			y -= lineH;
			m_TextContext->printfAt(x, y, "FPS: %.1f  (%.2f ms)", smoothFps, dt * 1000.f);
		}

		m_Driver->swapBuffers();
	}
}

#ifdef NL_OS_WINDOWS
sint WINAPI WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR /* cmdline */, int /* nCmdShow */)
#else
sint main(int /* argc */, char ** /* argv */)
#endif
{
	CApplicationContext applicationContext;

	CPPLDemo demo;
	demo.run();

	return EXIT_SUCCESS;
}
