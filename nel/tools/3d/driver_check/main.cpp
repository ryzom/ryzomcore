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

// driver_check draws a lit test scene with a 3D driver that has no window: the driver draws
// into an off-screen surface. It prints what the driver says about itself, the time it took to
// set the display, which is when the driver makes its context and builds the programs of its own
// materials, and the time of each frame up to the end of its drawing, and can save the last frame.
//
// The scene is the one of the per-pixel lighting sample: a sphere over a floor, lit by eight
// point lights of the light table, first per vertex and then per pixel.
//
// Only the OpenGL ES 3.0 driver has an off-screen surface so far (through EGL).

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/matrix.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/driver.h>
#include <nel/3d/dru.h>
#include <nel/3d/frustum.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/light.h>
#include <nel/3d/material.h>
#include <nel/3d/scissor.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/viewport.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace {

const uint LightCount = 8;

// A view matrix from the position of the eye, the point it looks at and the up vector
CMatrix buildViewMatrix(const CVector &eye, const CVector &target, const CVector &up)
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

// A sphere around the origin, with normals and one vertex colour
void buildSphere(CVertexBuffer &vb, CIndexBuffer &ib, float radius, uint slices, uint stacks, CRGBA color)
{
	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::PrimaryColorFlag);
	vb.setBufferUsage(CVertexBuffer::Immutable, false);
	vb.setNumVertices((stacks + 1) * (slices + 1));

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setBufferUsage(CIndexBuffer::Immutable, false);
	ib.setNumIndexes(stacks * slices * 2 * 3);

	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);
		for (uint j = 0; j <= stacks; ++j)
		{
			float phi = (float)Pi * (float)j / (float)stacks;
			for (uint i = 0; i <= slices; ++i)
			{
				float theta = 2.f * (float)Pi * (float)i / (float)slices;
				CVector n(sinf(phi) * cosf(theta), sinf(phi) * sinf(theta), cosf(phi));
				uint idx = j * (slices + 1) + i;
				vba.setVertexCoord(idx, n * radius);
				vba.setNormalCoord(idx, n);
				vba.setColor(idx, color);
			}
		}
	}

	{
		CIndexBufferReadWrite iba;
		ib.lock(iba);
		uint tri = 0;
		for (uint j = 0; j < stacks; ++j)
		{
			for (uint i = 0; i < slices; ++i)
			{
				uint a = j * (slices + 1) + i;
				uint b = a + slices + 1;
				iba.setTri(tri * 3, a, b, a + 1);
				++tri;
				iba.setTri(tri * 3, a + 1, b, b + 1);
				++tri;
			}
		}
	}
}

// A square floor at height 0, with normals and one vertex colour
void buildFloor(CVertexBuffer &vb, CIndexBuffer &ib, float halfSize, CRGBA color)
{
	vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::PrimaryColorFlag);
	vb.setBufferUsage(CVertexBuffer::Immutable, false);
	vb.setNumVertices(4);

	ib.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	ib.setBufferUsage(CIndexBuffer::Immutable, false);
	ib.setNumIndexes(6);

	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);
		const CVector corners[4] = {
			CVector(-halfSize, -halfSize, 0.f), CVector(halfSize, -halfSize, 0.f),
			CVector(halfSize, halfSize, 0.f), CVector(-halfSize, halfSize, 0.f)
		};
		for (uint i = 0; i < 4; ++i)
		{
			vba.setVertexCoord(i, corners[i]);
			vba.setNormalCoord(i, CVector(0.f, 0.f, 1.f));
			vba.setColor(i, color);
		}
	}

	{
		CIndexBufferReadWrite iba;
		ib.lock(iba);
		iba.setTri(0, 0, 1, 2);
		iba.setTri(3, 0, 2, 3);
	}
}

struct CLightConfig
{
	CRGBA Color;
	float OrbitRadius;
	float OrbitHeight;
};

const CLightConfig LightConfigs[LightCount] = {
	{ CRGBA(255, 60, 60), 6.f, 3.0f },
	{ CRGBA(255, 160, 40), 7.f, 2.0f },
	{ CRGBA(255, 255, 60), 5.f, 4.0f },
	{ CRGBA(60, 255, 60), 8.f, 1.5f },
	{ CRGBA(60, 255, 255), 6.f, 3.5f },
	{ CRGBA(60, 60, 255), 7.f, 2.5f },
	{ CRGBA(200, 60, 255), 5.f, 4.5f },
	{ CRGBA(255, 255, 255), 9.f, 1.0f },
};

struct CScene
{
	CVertexBuffer SphereVB;
	CIndexBuffer SphereIB;
	CVertexBuffer FloorVB;
	CIndexBuffer FloorIB;
	CMaterial SphereMat;
	CMaterial FloorMat;
};

// One frame of the scene, with the first perPixelLights of the lights per pixel and the others per
// vertex. Returns the seconds it took, up to the end of its drawing.
double drawFrame(IDriver *drv, CScene &scene, uint width, uint height, uint perPixelLights)
{
	TTicks start = CTime::getPerformanceTime();

	drv->clear2D(CRGBA(20, 20, 30));
	drv->clearZBuffer();
	drv->clearStencilBuffer();

	CScissor scissor;
	scissor.initFullScreen();
	drv->setupScissor(scissor);
	CViewport viewport;
	viewport.initFullScreen();
	drv->setupViewport(viewport);
	CFrustum frustum;
	frustum.initPerspective(float(Pi / 3.0), float(width) / float(height), 0.1f, 200.f);
	drv->setFrustum(frustum.Left, frustum.Right, frustum.Bottom, frustum.Top, frustum.Near, frustum.Far, frustum.Perspective);
	drv->setupViewMatrix(buildViewMatrix(CVector(13.f, 7.5f, 8.f), CVector(0.f, 0.f, 1.5f), CVector(0.f, 0.f, 1.f)));
	drv->enableFog(false);

	drv->enableLightTableMode(true);
	drv->setLightTableSize(LightCount);
	sint16 indices[LightCount];
	uint8 factors[LightCount];
	for (uint i = 0; i < LightCount; ++i)
	{
		float angle = float(i) * 2.f * (float)Pi / float(LightCount);
		CVector pos(cosf(angle) * LightConfigs[i].OrbitRadius, sinf(angle) * LightConfigs[i].OrbitRadius, LightConfigs[i].OrbitHeight);
		CRGBA diffuse = LightConfigs[i].Color;
		CRGBA specular(diffuse.R / 2, diffuse.G / 2, diffuse.B / 2);
		CLight light;
		light.setupPointLight(CRGBA::Black, diffuse, specular, pos, CVector(0.f, 0.f, -1.f));
		light.setupAttenuation(2.f, 12.f);
		drv->setLightTableEntry(i, light);
		indices[i] = (sint16)i;
		factors[i] = 255;
	}
	drv->setLights(indices, factors, LightCount, perPixelLights, CRGBA(20, 20, 25));

	CMatrix sphereModel;
	sphereModel.identity();
	sphereModel.setPos(CVector(0.f, 0.f, 1.5f));
	drv->setupModelMatrix(sphereModel);
	drv->activeVertexBuffer(scene.SphereVB);
	drv->activeIndexBuffer(scene.SphereIB);
	drv->renderTriangles(scene.SphereMat, 0, scene.SphereIB.getNumIndexes() / 3);

	CMatrix floorModel;
	floorModel.identity();
	drv->setupModelMatrix(floorModel);
	drv->activeVertexBuffer(scene.FloorVB);
	drv->activeIndexBuffer(scene.FloorIB);
	drv->renderTriangles(scene.FloorMat, 0, 2);

	drv->enableLightTableMode(false);

	// the swap of an off-screen surface does nothing, the time is the one of the drawing
	drv->finish();
	double seconds = CTime::ticksToSecond(CTime::getPerformanceTime() - start);

	drv->swapBuffers();

	return seconds;
}

} // namespace

int main(int argc, char **argv)
{
	CApplicationContext appContext;

	CCmdArgs args;
	args.setDescription("Draws a lit test scene with the OpenGL ES 3.0 driver, without a window, and prints the time the driver took to set the display, which is when it makes its context and builds the programs of its own materials, and the time of each frame.");
	args.addArg("f", "frames", "count", "Frames to draw in each lighting mode (default: 4)");
	args.addArg("s", "size", "pixels", "Width and height of the surface (default: 512)");
	args.addArg("o", "output", "file", "Save the last frame to this file (.tga)");
	if (!args.parse(argc, argv))
		return 1;

	uint frames = 4;
	if (args.haveArg("f") && (!fromString(args.getArg("f").front(), frames) || frames == 0 || frames > 100000))
	{
		printf("--frames: not a count from 1 to 100000: '%s'\n", args.getArg("f").front().c_str());
		return 1;
	}
	uint size = 512;
	if (args.haveArg("s") && (!fromString(args.getArg("s").front(), size) || size < 16 || size > 8192))
	{
		printf("--size: not a size from 16 to 8192: '%s'\n", args.getArg("s").front().c_str());
		return 1;
	}

	IDriver *drv = nullptr;
	try
	{
		drv = CDRU::createGlEs3Driver();
	}
	catch (const Exception &e)
	{
		printf("The OpenGL ES 3.0 driver cannot be loaded: %s\n", e.what());
		return 1;
	}
	if (!drv)
	{
		printf("This build has no OpenGL ES 3.0 driver\n");
		return 1;
	}

	if (!drv->init())
	{
		printf("The driver refused to start\n");
		delete drv;
		return 1;
	}
	GfxMode mode((uint16)size, (uint16)size, 32, true, true);
	TTicks displayStart = CTime::getPerformanceTime();
	if (!drv->setDisplay(EmptyWindow, mode, false, false))
	{
		printf("The driver has no off-screen surface of %ux%u (see the log)\n", size, size);
		drv->release();
		delete drv;
		return 1;
	}
	double displaySeconds = CTime::ticksToSecond(CTime::getPerformanceTime() - displayStart);
	drv->activate();

	// the context, and the programs of the driver's own materials
	printf("display set in %.1f ms\n", displaySeconds * 1000.0);
	printf("driver: %s\n", drv->getDriverInformation());
	printf("card: %s\n", drv->getVideocardInformation());
	printf("light table: %u entries\n", drv->getMaxLightTableSize());

	int result = 0;
	{
		CScene scene;
		buildSphere(scene.SphereVB, scene.SphereIB, 1.5f, 32, 16, CRGBA(160, 160, 160));
		buildFloor(scene.FloorVB, scene.FloorIB, 20.f, CRGBA(100, 100, 100));
		scene.SphereMat.initLighted();
		scene.SphereMat.setDiffuse(CRGBA::White);
		scene.SphereMat.setAmbient(CRGBA(30, 30, 30));
		scene.SphereMat.setSpecular(CRGBA(200, 200, 200));
		scene.SphereMat.setShininess(32.f);
		scene.FloorMat.initLighted();
		scene.FloorMat.setDiffuse(CRGBA::White);
		scene.FloorMat.setAmbient(CRGBA(30, 30, 30));
		scene.FloorMat.setSpecular(CRGBA::Black);
		scene.FloorMat.setShininess(1.f);

		const uint perPixelLights[2] = { 0, LightCount };
		const char *const modeNames[2] = { "per vertex", "per pixel" };
		for (uint m = 0; m < 2; ++m)
		{
			for (uint f = 0; f < frames; ++f)
			{
				double seconds = drawFrame(drv, scene, size, size, perPixelLights[m]);
				printf("%s, frame %u: %.1f ms\n", modeNames[m], f, seconds * 1000.0);
			}
		}

		if (args.haveArg("o"))
		{
			// the picture of the frame that was drawn last: the swap of an off-screen surface keeps it
			const string fileName = args.getArg("o").front();
			CBitmap bitmap;
			drv->getBuffer(bitmap);
			COFile file;
			if (file.open(fileName) && bitmap.writeTGA(file, 32))
			{
				printf("saved %ux%u to '%s'\n", bitmap.getWidth(), bitmap.getHeight(), fileName.c_str());
			}
			else
			{
				printf("Cannot write '%s'\n", fileName.c_str());
				result = 1;
			}
		}

		// the buffers and materials of the scene go before the driver
	}

	drv->release();
	delete drv;
	return result;
}
