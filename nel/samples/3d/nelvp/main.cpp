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
// nelvp Vertex Program Demo
//
// Demonstrates nelvp, the NeL cross-platform assembly vertex program language.
// A sphere is rendered with a nelvp program that performs:
//   - Wobble morphing animation (displaces vertices along their normals
//     using a Taylor-series cosine approximation driven by time)
//   - Per-vertex two-point-light diffuse+specular Blinn-Phong lighting
//   - ModelViewProjection transform to clip space
//
// The VP uses constant registers for:
//   c[0]-c[3]   ModelViewProjection matrix
//   c[4]-c[7]   ModelView matrix (for eye-space position)
//   c[8]-c[11]  ModelView inverse-transpose (for normal transform)
//   c[12]       Wobble params: (time, amplitude, frequency1, phase_scale)
//   c[13]       Taylor coefficients: (1, -1/2, 1/24, -1/720)
//   c[14]       Trig constants: (2*Pi, 1/(2*Pi), Pi, 1/40320)
//   c[15]       Utility: (0, 0.5, 1, 2)
//   c[16]       Light 0 position (eye space)
//   c[17]       Light 0 diffuse color
//   c[18]       Light 0 specular color
//   c[19]       Light 1 position (eye space)
//   c[20]       Light 1 diffuse color
//   c[21]       Light 1 specular color
//   c[22]       Ambient color
//   c[23]       (specular_power, frequency2, 0, 0)
//
// Vertex inputs:
//   v[OPOS]     Object-space position
//   v[NRML]     Object-space normal
//
// Controls:
//   Up/Down     Increase/decrease wobble amplitude
//   Left/Right  Increase/decrease wobble frequency
//   W           Toggle wireframe
//   C           Toggle camera orbit
//

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/driver.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/material.h>
#include <nel/3d/vertex_program.h>

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

// ---------------------------------------------------------------------------
// nelvp vertex program source
//
// This program displaces sphere vertices along their normals using a
// two-wave cosine wobble (8th-order Taylor series), then computes
// Blinn-Phong lighting for two colored point lights in eye space.
//
// Register constraints honored throughout:
// - No instruction reads two different constant registers
// - No instruction reads two different input registers
// - All temp register components are written before read
// ---------------------------------------------------------------------------
static const char *s_NelvpSource =
"!!VP1.0\n"

// ---------------------------------------------------------------
// Wave 1: spatial pattern = pos.x + pos.y + pos.z
// ---------------------------------------------------------------
// c[12] = (time, amplitude, frequency1, phase_scale)
// c[15] = (0, 0.5, 1, 2)
"DP3 R0.x, v[OPOS], c[15].zzzz;\n"       // R0.x = pos.x + pos.y + pos.z
"MAD R0.x, R0.x, c[12].w, c[12].x;\n"    // R0.x = sum * phase_scale + time
"MUL R0.x, R0.x, c[12].z;\n"             // R0.x *= frequency1

// Wrap angle to [-Pi, Pi] for Taylor series accuracy
// c[14] = (2*Pi, 1/(2*Pi), Pi, 1/40320)
"MUL R0.x, R0.x, c[14].y;\n"             // angle / (2*Pi)
"EXP R1, R0.x;\n"                         // R1.y = fract(angle/(2*Pi))
"MAD R0.x, R1.y, c[14].x, -c[14].z;\n"   // fract * 2*Pi - Pi

// cos(x) via 8th-order Taylor: 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
// c[13] = (1, -1/2, 1/24, -1/720)
"MUL R1.x, R0.x, R0.x;\n"                // x^2
"MUL R1.y, R1.x, R1.x;\n"               // x^4
"MUL R1.z, R1.y, R1.x;\n"               // x^6
"MUL R1.w, R1.z, R1.x;\n"               // x^8
"MAD R0.y, R1.x, c[13].y, c[13].x;\n"   // 1 - x^2/2
"MAD R0.y, R1.y, c[13].z, R0.y;\n"       // + x^4/24
"MAD R0.y, R1.z, c[13].w, R0.y;\n"       // - x^6/720
"MAD R0.y, R1.w, c[14].w, R0.y;\n"       // + x^8/40320

// ---------------------------------------------------------------
// Wave 2: spatial pattern = pos.x - pos.z (orthogonal variation)
// Uses frequency2 from c[23].y for temporal decoherence
// ---------------------------------------------------------------
"ADD R0.z, v[OPOS].x, -v[OPOS].z;\n"     // pos.x - pos.z
"MAD R0.z, R0.z, c[12].w, c[12].x;\n"    // * phase_scale + time
"MUL R0.z, R0.z, c[23].y;\n"             // * frequency2

// Wrap to [-Pi, Pi]
"MUL R0.z, R0.z, c[14].y;\n"             // / (2*Pi)
"EXP R2, R0.z;\n"                         // R2.y = fract
"MAD R0.z, R2.y, c[14].x, -c[14].z;\n"   // fract * 2*Pi - Pi

// cos(x) Taylor 8th order
"MUL R2.x, R0.z, R0.z;\n"                // x^2
"MUL R2.y, R2.x, R2.x;\n"               // x^4
"MUL R2.z, R2.y, R2.x;\n"               // x^6
"MUL R2.w, R2.z, R2.x;\n"               // x^8
"MAD R0.z, R2.x, c[13].y, c[13].x;\n"   // cos approx
"MAD R0.z, R2.y, c[13].z, R0.z;\n"
"MAD R0.z, R2.z, c[13].w, R0.z;\n"
"MAD R0.z, R2.w, c[14].w, R0.z;\n"       // + x^8/40320

// ---------------------------------------------------------------
// Combine waves and displace vertex along normal
// ---------------------------------------------------------------
// displacement = amplitude * (cos1 + 0.5 * cos2)
"MAD R0.y, R0.z, c[15].y, R0.y;\n"       // cos1 + 0.5 * cos2
"MUL R0.y, R0.y, c[12].y;\n"             // * amplitude

"MOV R4, v[OPOS];\n"                      // R4 = original position (w=1)
"MOV R3, v[NRML];\n"                      // R3 = normal
"MAD R4.xyz, R3, R0.y, R4;\n"            // displace xyz, preserve w=1

// ---------------------------------------------------------------
// Transform displaced position to clip space: c[0]-c[3] = MVP
// ---------------------------------------------------------------
"DP4 o[HPOS].x, c[0], R4;\n"
"DP4 o[HPOS].y, c[1], R4;\n"
"DP4 o[HPOS].z, c[2], R4;\n"
"DP4 o[HPOS].w, c[3], R4;\n"

// ---------------------------------------------------------------
// Eye-space transforms for lighting
// ---------------------------------------------------------------
// Position to eye space: c[4]-c[7] = ModelView
"DP4 R5.x, c[4], R4;\n"
"DP4 R5.y, c[5], R4;\n"
"DP4 R5.z, c[6], R4;\n"
"MOV R5.w, c[15].z;\n"                   // R5.w = 1.0 (initialize for full-register reads)

// Normal to eye space: c[8]-c[10] = ModelView inverse-transpose
"DP3 R6, c[8], R3;\n"                     // write all 4 components (initializes .w for later full-register read)
"DP3 R6.y, c[9], R3;\n"
"DP3 R6.z, c[10], R3;\n"
// Normalize eye-space normal
"DP3 R6.w, R6, R6;\n"
"RSQ R6.w, R6.w;\n"
"MUL R6.xyz, R6, R6.w;\n"

// ---------------------------------------------------------------
// Compute view direction (shared by both lights)
// In eye space, camera is at origin: V = normalize(-eyePos)
// ---------------------------------------------------------------
"DP3 R9.w, R5, R5;\n"
"RSQ R9.w, R9.w;\n"
"MUL R9.xyz, -R5, R9.w;\n"               // R9 = normalized view dir

// ---------------------------------------------------------------
// Light 0
// c[16] = position, c[17] = diffuse, c[18] = specular
// ---------------------------------------------------------------
"ADD R7, c[16], -R5;\n"                   // L = lightPos - vertexEyePos
"DP3 R7.w, R7, R7;\n"
"RSQ R7.w, R7.w;\n"
"MUL R7.xyz, R7, R7.w;\n"                // normalize L

// NdotL (replicate to all components so R8.z is initialized for LIT)
"DP3 R8, R6, R7;\n"                       // R8.xyzw = N.L
"MAX R8.x, R8.x, c[15].x;\n"            // clamp NdotL >= 0

// Half vector: H = normalize(L + V)
"ADD R10, R7, R9;\n"
"DP3 R10.w, R10, R10;\n"
"RSQ R10.w, R10.w;\n"
"MUL R10.xyz, R10, R10.w;\n"

"DP3 R8.y, R6, R10;\n"                   // NdotH
"MOV R8.w, c[23].x;\n"                   // specular power
"LIT R11, R8;\n"                          // R11 = (1, diff, spec, 1)

// Accumulate: start with ambient
"MOV R0, c[22];\n"                        // R0 = ambient
"MAD R0.xyz, R11.y, c[17], R0;\n"        // += diff0 * light0 diffuse
"MAD R0.xyz, R11.z, c[18], R0;\n"        // += spec0 * light0 specular

// ---------------------------------------------------------------
// Light 1
// c[19] = position, c[20] = diffuse, c[21] = specular
// ---------------------------------------------------------------
"ADD R7, c[19], -R5;\n"
"DP3 R7.w, R7, R7;\n"
"RSQ R7.w, R7.w;\n"
"MUL R7.xyz, R7, R7.w;\n"

"DP3 R8, R6, R7;\n"                       // NdotL (all components)
"MAX R8.x, R8.x, c[15].x;\n"

"ADD R10, R7, R9;\n"                      // H = L + V
"DP3 R10.w, R10, R10;\n"
"RSQ R10.w, R10.w;\n"
"MUL R10.xyz, R10, R10.w;\n"

"DP3 R8.y, R6, R10;\n"                   // NdotH
"MOV R8.w, c[23].x;\n"                   // specular power
"LIT R11, R8;\n"

"MAD R0.xyz, R11.y, c[20], R0;\n"        // += diff1 * light1 diffuse
"MAD R0.xyz, R11.z, c[21], R0;\n"        // += spec1 * light1 specular

// ---------------------------------------------------------------
// Clamp and output final color
// ---------------------------------------------------------------
"MIN R0, R0, c[15].z;\n"                 // clamp <= 1.0
"MAX R0, R0, c[15].x;\n"                 // clamp >= 0.0
"MOV R0.w, c[15].z;\n"                   // alpha = 1
"MOV o[COL0], R0;\n"
"END\n";

// ---------------------------------------------------------------------------
// Build a UV sphere (position + normal format)
// ---------------------------------------------------------------------------
static void buildSphere(CVertexBuffer &vb, CIndexBuffer &ib,
	float radius, uint slices, uint stacks)
{
	uint numVerts = (stacks + 1) * (slices + 1);
	uint numTris = stacks * slices * 2;

	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag);
	vb.setBufferUsage(CVertexBuffer::Immutable, false);
	vb.setNumVertices(numVerts);

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setBufferUsage(CIndexBuffer::Immutable, false);
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

// ---------------------------------------------------------------------------
// Build a view matrix from eye position, target, and up vector
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Demo application
// ---------------------------------------------------------------------------
class CNelvpDemo : public IEventListener
{
public:
	CNelvpDemo();
	~CNelvpDemo();
	void run();
	void renderOneFrame();

	virtual void operator()(const CEvent &event) NL_OVERRIDE;

private:
	bool m_CloseWindow;
	bool m_Wireframe;
	bool m_AnimCamera;
	float m_Amplitude;
	float m_Frequency;

	UDriver *m_Driver;

	CVertexBuffer m_SphereVB;
	CIndexBuffer m_SphereIB;

	CMaterial m_Mat;
	CVertexProgram *m_VP;

	CFrustum m_Frustum;
	float m_CamDist;
	float m_CamHeight;
	float m_CamAngle;
	double m_StartTime;
	double m_LastTime;
};

CNelvpDemo::CNelvpDemo()
	: m_CloseWindow(false)
	, m_Wireframe(false)
	, m_AnimCamera(true)
	, m_Amplitude(0.15f)
	, m_Frequency(3.f)
	, m_VP(NULL)
	, m_CamDist(7.f)
	, m_CamHeight(3.f)
	, m_CamAngle(0.f)
	, m_StartTime(0.0)
	, m_LastTime(0.0)
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

	m_Driver->setDisplay(UDriver::CMode(800, 600, 32, true));
	m_Driver->setWindowTitle(ucstring("NeL nelvp Vertex Program Demo"));

	// Build sphere geometry (high poly for visible wobble)
	buildSphere(m_SphereVB, m_SphereIB, 2.f, 48, 32);

	// Unlit material -- the VP handles all lighting via o[COL0]
	m_Mat.initUnlit();
	m_Mat.setZWrite(true);
	m_Mat.setZFunc(CMaterial::lessequal);

	// Create vertex program from nelvp source
	m_VP = new CVertexProgram(s_NelvpSource);

	m_Frustum.initPerspective(float(Pi / 3.0), 800.f / 600.f, 0.1f, 100.f);
	m_StartTime = CTime::ticksToSecond(CTime::getPerformanceTime());
	m_LastTime = m_StartTime;
}

CNelvpDemo::~CNelvpDemo()
{
	delete m_VP;
	m_Driver->release();
	delete m_Driver;
}

void CNelvpDemo::operator()(const CEvent &event)
{
	if (event == EventCloseWindowId)
	{
		m_CloseWindow = true;
	}
	else if (event == EventKeyDownId)
	{
		CEventKeyDown &keyEvent = (CEventKeyDown &)event;
		switch (keyEvent.Key)
		{
		case KeyUP:    m_Amplitude = min(m_Amplitude + 0.02f, 1.f); break;
		case KeyDOWN:  m_Amplitude = max(m_Amplitude - 0.02f, 0.f); break;
		case KeyRIGHT: m_Frequency = min(m_Frequency + 0.5f, 20.f); break;
		case KeyLEFT:  m_Frequency = max(m_Frequency - 0.5f, 0.5f); break;
		case KeyW:     m_Wireframe = !m_Wireframe; break;
		case KeyC:     m_AnimCamera = !m_AnimCamera; break;
		default: break;
		}
	}
}

void CNelvpDemo::renderOneFrame()
{
	if (!m_Driver->isFrameReady())
		return; // GPU busy, skip frame to avoid blocking browser event loop

	IDriver *drv = static_cast<CDriverUser *>(m_Driver)->getDriver();

	m_Driver->EventServer.pump();

	double now = CTime::ticksToSecond(CTime::getPerformanceTime()) - m_StartTime;
	double dt = now - (m_LastTime - m_StartTime);
	m_LastTime = now + m_StartTime;

	// Camera orbit
	if (m_AnimCamera)
		m_CamAngle += float(dt * 0.4);

	CVector eye(cosf(m_CamAngle) * m_CamDist, sinf(m_CamAngle) * m_CamDist, m_CamHeight);
	CVector target(0.f, 0.f, 0.f);
	CVector up(0.f, 0.f, 1.f);
	CMatrix viewMatrix = buildViewMatrix(eye, target, up);

	m_Driver->clearBuffers(CRGBA(25, 25, 30));

	m_Driver->setFrustum(m_Frustum);
	m_Driver->setViewMatrix(viewMatrix);

	// Model matrix: identity (sphere at origin)
	CMatrix modelMatrix;
	modelMatrix.identity();
	m_Driver->setModelMatrix(modelMatrix);

	// Wireframe toggle
	if (m_Wireframe)
		m_Driver->setPolygonMode(UDriver::Line);
	else
		m_Driver->setPolygonMode(UDriver::Filled);

	// Activate vertex program BEFORE vertex buffer (driver requirement)
	nlverify(drv->activeVertexProgram(m_VP));

	// --- Set VP constant registers ---

	// c[0]-c[3]: ModelViewProjection matrix
	drv->setUniformMatrix(IDriver::VertexProgram, 0,
		IDriver::ModelViewProjection, IDriver::Identity);

	// c[4]-c[7]: ModelView matrix (for eye-space position)
	drv->setUniformMatrix(IDriver::VertexProgram, 4,
		IDriver::ModelView, IDriver::Identity);

	// c[8]-c[11]: ModelView inverse-transpose (for normal transform)
	drv->setUniformMatrix(IDriver::VertexProgram, 8,
		IDriver::ModelView, IDriver::InverseTranspose);

	// c[12]: Wobble parameters (time, amplitude, frequency1, phase_scale)
	float phaseScale = 1.5f;
	drv->setUniform4f(IDriver::VertexProgram, 12,
		(float)now, m_Amplitude, m_Frequency, phaseScale);

	// c[13]: Taylor coefficients for cos: (1, -1/2!, 1/4!, -1/6!)
	drv->setUniform4f(IDriver::VertexProgram, 13,
		1.f, -0.5f, 1.f / 24.f, -1.f / 720.f);

	// c[14]: Trig constants: (2*Pi, 1/(2*Pi), Pi, 1/8!)
	drv->setUniform4f(IDriver::VertexProgram, 14,
		2.f * (float)Pi, 1.f / (2.f * (float)Pi),
		(float)Pi, 1.f / 40320.f);

	// c[15]: Utility constants (0, 0.5, 1, 2)
	drv->setUniform4f(IDriver::VertexProgram, 15,
		0.f, 0.5f, 1.f, 2.f);

	// Orbiting lights
	float light0Angle = float(now * 0.7);
	float light1Angle = float(now * 0.5) + (float)Pi;

	CVector light0World(
		cosf(light0Angle) * 5.f,
		sinf(light0Angle) * 5.f,
		2.f + sinf(float(now * 0.3f)) * 2.f);
	CVector light1World(
		cosf(light1Angle) * 6.f,
		sinf(light1Angle) * 6.f,
		1.f + cosf(float(now * 0.4f)) * 2.f);

	// Transform light positions to eye space
	CVector light0Eye = viewMatrix * light0World;
	CVector light1Eye = viewMatrix * light1World;

	// c[16]: Light 0 position (eye space)
	drv->setUniform4f(IDriver::VertexProgram, 16,
		light0Eye.x, light0Eye.y, light0Eye.z, 1.f);

	// c[17]: Light 0 diffuse color (warm orange-red)
	drv->setUniform4f(IDriver::VertexProgram, 17,
		0.9f, 0.4f, 0.15f, 1.f);

	// c[18]: Light 0 specular color (bright white-orange)
	drv->setUniform4f(IDriver::VertexProgram, 18,
		1.0f, 0.7f, 0.4f, 1.f);

	// c[19]: Light 1 position (eye space)
	drv->setUniform4f(IDriver::VertexProgram, 19,
		light1Eye.x, light1Eye.y, light1Eye.z, 1.f);

	// c[20]: Light 1 diffuse color (cool blue-cyan)
	drv->setUniform4f(IDriver::VertexProgram, 20,
		0.15f, 0.4f, 0.9f, 1.f);

	// c[21]: Light 1 specular color (bright white-blue)
	drv->setUniform4f(IDriver::VertexProgram, 21,
		0.4f, 0.7f, 1.0f, 1.f);

	// c[22]: Ambient color (dim cool gray)
	drv->setUniform4f(IDriver::VertexProgram, 22,
		0.05f, 0.05f, 0.08f, 1.f);

	// c[23]: (specular_power, frequency2, 0, 0)
	// frequency2 = frequency1 * 1.3 for temporal decoherence between waves
	drv->setUniform4f(IDriver::VertexProgram, 23,
		32.f, m_Frequency * 1.3f, 0.f, 0.f);

	// Activate buffers and render
	drv->activeVertexBuffer(m_SphereVB);
	drv->activeIndexBuffer(m_SphereIB);
	drv->renderTriangles(m_Mat, 0, m_SphereIB.getNumIndexes() / 3);

	// Deactivate vertex program
	drv->activeVertexProgram(NULL);

	m_Driver->swapBuffers();
}

void CNelvpDemo::run()
{
#ifndef __EMSCRIPTEN__
	while (m_Driver->isActive() && !m_CloseWindow)
	{
		renderOneFrame();
	}
#endif
}

#ifdef __EMSCRIPTEN__
static CNelvpDemo *s_Demo = NULL;

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
	static CNelvpDemo demo;
	s_Demo = &demo;
	EM_ASM({ if (window.nlLoadingComplete) window.nlLoadingComplete(); });
	emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
	CNelvpDemo demo;
	demo.run();
#endif

	return EXIT_SUCCESS;
}
