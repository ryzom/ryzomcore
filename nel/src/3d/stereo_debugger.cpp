/**
 * \file stereo_debugger.cpp
 * \brief CStereoDebugger
 * \date 2013-07-03 20:17GMT
 * \author Jan Boon (Kaetemi)
 * CStereoDebugger
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#if !FINAL_VERSION
#include "std3d.h"
#include "nel/3d/stereo_debugger.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "nel/3d/u_camera.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/material.h"
#include "nel/3d/texture_offscreen.h"
#include "nel/3d/texture_user.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/render_target_manager.h"
#include "nel/misc/command.h"

using namespace std;
// using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

namespace {

// Stereo comparison shader (compiled from shaders/stereo_debug_pp.cg):
//   Identical pixels:           normal average image
//   Small diff (< ~1%):         blue-tinted (darkened R and G)
//   Significant diff (>= ~1%):  red-tinted (darkened G and B)
const char *a_arbfp1 =
	"!!ARBfp1.0\n"
	"OPTION ARB_precision_hint_fastest;\n"
	"# cgc version 3.1.0013, build date Apr 18 2012\n"
	"# command line args: -profile arbfp1 -O3 -fastmath -fastprecision\n"
	"# source file: stereo_debug_pp.cg\n"
	"#vendor NVIDIA Corporation\n"
	"#version 3.1.0.13\n"
	"#profile arbfp1\n"
	"#program stereo_debug_pp\n"
	"#semantic stereo_debug_pp.cTex0 : TEX0\n"
	"#semantic stereo_debug_pp.cTex1 : TEX1\n"
	"#var float2 texCoord : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
	"#var sampler2D cTex0 : TEX0 : texunit 0 : 1 : 1\n"
	"#var sampler2D cTex1 : TEX1 : texunit 1 : 2 : 1\n"
	"#var float4 oCol : $vout.COLOR : COL : 3 : 1\n"
	"#const c[0] = 1 0 0.0099999998 0.5\n"
	"PARAM c[1] = { { 1, 0, 0.0099999998, 0.5 } };\n"
	"TEMP R0;\n"
	"TEMP R1;\n"
	"TEMP R2;\n"
	"TEX R1, fragment.texcoord[0], texture[1], 2D;\n"
	"TEX R0, fragment.texcoord[0], texture[0], 2D;\n"
	"ADD R2.xyz, R0, -R1;\n"
	"ADD R0, R0, R1;\n"
	"MUL R0, R0, c[0].w;\n"
	"ABS R2.xyz, R2;\n"
	"MAX R1.x, R2, R2.y;\n"
	"MAX R1.x, R1, R2.z;\n"
	"SLT R1.y, c[0], R1.x;\n"
	"SGE R1.w, R1.x, c[0].z;\n"
	"ABS R1.w, R1;\n"
	"ABS R2.x, R1.y;\n"
	"MAD R1.z, R0.y, c[0].w, c[0].w;\n"
	"CMP R1.w, -R1, c[0].y, c[0].x;\n"
	"CMP R2.x, -R2, c[0].y, c[0];\n"
	"MUL R2.x, R2, R1.w;\n"
	"CMP result.color.y, -R2.x, R1.z, R0;\n"
	"MAD R1.z, R0, c[0].w, c[0].w;\n"
	"MUL R0.y, R1, R1.w;\n"
	"CMP result.color.z, -R0.y, R1, R0;\n"
	"MAD R0.z, R0.x, c[0].w, c[0].w;\n"
	"ADD R0.y, R1.x, -c[0].z;\n"
	"CMP result.color.x, R0.y, R0, R0.z;\n"
	"MOV result.color.w, R0;\n"
	"END\n";
	// 24 instructions, 3 R-regs

// GLSL 330 version of the same shader for the GL3 driver.
// Sampler names match CProgramIndex convention for automatic texture unit binding.
// texCoord0 at location 8 matches TAttribOffset::TexCoord0 from the builtin VP.
const char *a_glsl330f =
	"#version 330\n"
	"#extension GL_ARB_separate_shader_objects : enable\n"
	"\n"
	"out vec4 fragColor;\n"
	"\n"
	"layout(location = 8) smooth in vec4 texCoord0;\n"
	"\n"
	"uniform sampler2D sampler0;\n"
	"uniform sampler2D sampler1;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  vec4 left = texture(sampler0, texCoord0.xy);\n"
	"  vec4 right = texture(sampler1, texCoord0.xy);\n"
	"  vec4 avg = (left + right) * 0.5;\n"
	"\n"
	"  vec3 d = abs(left.rgb - right.rgb);\n"
	"  float md = max(max(d.r, d.g), d.b);\n"
	"\n"
	"  fragColor = avg;\n"
	"  if (md >= 0.01)\n"
	"    fragColor.r = 0.5 + (fragColor.r * 0.5);\n"
	"  else if (md > 0.0)\n"
	"    fragColor.b = 0.5 + (fragColor.b * 0.5);\n"
	"  else\n"
	"    fragColor.g = 0.5 + (fragColor.g * 0.5);\n"
	"}\n";

const char *a_ps_2_0 =
	"ps_2_0\n"
	"// cgc version 3.1.0013, build date Apr 18 2012\n"
	"// command line args: -profile ps_2_0 -O3 -fastmath -fastprecision\n"
	"// source file: stereo_debug_pp.cg\n"
	"//vendor NVIDIA Corporation\n"
	"//version 3.1.0.13\n"
	"//profile ps_2_0\n"
	"//program stereo_debug_pp\n"
	"//semantic stereo_debug_pp.cTex0 : TEX0\n"
	"//semantic stereo_debug_pp.cTex1 : TEX1\n"
	"//var float2 texCoord : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
	"//var sampler2D cTex0 : TEX0 : texunit 0 : 1 : 1\n"
	"//var sampler2D cTex1 : TEX1 : texunit 1 : 2 : 1\n"
	"//var float4 oCol : $vout.COLOR : COL : 3 : 1\n"
	"//const c[0] = 0.5 -0.0099999998 1 0\n"
	"dcl_2d s0\n"
	"dcl_2d s1\n"
	"def c0, 0.50000000, -0.01000000, 1.00000000, 0.00000000\n"
	"dcl t0.xy\n"
	"texld r0, t0, s1\n"
	"texld r1, t0, s0\n"
	"add r2.xyz, r1, -r0\n"
	"add r1, r1, r0\n"
	"mul r5, r1, c0.x\n"
	"abs r2.xyz, r2\n"
	"max r0.x, r2, r2.y\n"
	"max r0.x, r0, r2.z\n"
	"cmp r1.x, -r0, c0.w, c0.z\n"
	"add r3.x, r0, c0.y\n"
	"abs_pp r4.x, r1\n"
	"cmp r3.x, r3, c0.z, c0.w\n"
	"abs_pp r3.x, r3\n"
	"cmp_pp r3.x, -r3, c0.z, c0.w\n"
	"cmp_pp r4.x, -r4, c0.z, c0.w\n"
	"mad r2.x, r5.y, c0, c0\n"
	"mul_pp r4.x, r3, r4\n"
	"cmp r0.y, -r4.x, r5, r2.x\n"
	"mul_pp r1.x, r3, r1\n"
	"mad r2.x, r5.z, c0, c0\n"
	"cmp r0.z, -r1.x, r5, r2.x\n"
	"mov r0.w, r5\n"
	"mad r1.x, r5, c0, c0\n"
	"add r0.x, r0, c0.y\n"
	"cmp r0.x, r0, r1, r5\n"
	"mov oC0, r0\n";

class CStereoDebuggerFactory : public IStereoDeviceFactory
{
public:
	IStereoDisplay *createDevice() const
	{
		return new CStereoDebugger();
	}
};

} /* anonymous namespace */

// 0 = comparison (default), 1 = left only, 2 = right only
static int s_StereoDisplayMode = 0;

NLMISC_CATEGORISED_COMMAND(nel, stereoDisplayMode, "Set stereo debugger display mode (0=compare, 1=left, 2=right)", "<mode>")
{
	if (args.size() != 1) return false;
	NLMISC::fromString(args[0], s_StereoDisplayMode);
	if (s_StereoDisplayMode < 0 || s_StereoDisplayMode > 2) s_StereoDisplayMode = 0;
	return true;
}

CStereoDebugger::CStereoDebugger() : m_Driver(NULL), m_Stage(0), m_SubStage(0), m_LeftTexU(NULL), m_RightTexU(NULL), m_PixelProgram(NULL)
{

}

CStereoDebugger::~CStereoDebugger()
{
	if (!m_Mat.empty())
	{
		m_Driver->deleteMaterial(m_Mat);
	}

	delete m_PixelProgram;
	m_PixelProgram = NULL;

	m_Driver = NULL;
}

/// Sets driver and generates necessary render targets
void CStereoDebugger::setDriver(NL3D::UDriver *driver)
{
	nlassert(!m_PixelProgram);
	
	m_Driver = driver;
	NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(driver))->getDriver();

	if (drvInternal->supportBloomEffect() && drvInternal->supportNonPowerOfTwoTextures())
	{
		m_PixelProgram = new CPixelProgram();
		// glsl330f
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::glsl330f;
			source->setSourcePtr(a_glsl330f);
			m_PixelProgram->addSource(source);
		}
		// arbfp1
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::arbfp1;
			source->setSourcePtr(a_arbfp1);
			m_PixelProgram->addSource(source);
		}
		// ps_2_0
		{
			IProgram::CSource *source = new IProgram::CSource();
			source->Features.MaterialFlags = CProgramFeatures::TextureStages;
			source->Profile = IProgram::ps_2_0;
			source->setSourcePtr(a_ps_2_0);
			m_PixelProgram->addSource(source);
		}
		if (!drvInternal->compilePixelProgram(m_PixelProgram))
		{
			nlwarning("STEREO: No supported pixel program for stereo debugger");

			delete m_PixelProgram;
			m_PixelProgram = NULL;
		}
		else
		{
			nlinfo("STEREO: Pixel program compiled successfully for stereo debugger");
		}
	}

	if (m_PixelProgram)
	{
		m_Mat = m_Driver->createMaterial();
		m_Mat.initUnlit();
		m_Mat.setColor(CRGBA::White);
		m_Mat.setBlend (false);
		m_Mat.setAlphaTest (false);
		NL3D::CMaterial *mat = m_Mat.getObjectPtr();
		mat->setShader(NL3D::CMaterial::Normal);
		mat->setBlendFunc(CMaterial::one, CMaterial::zero);
		mat->setZWrite(false);
		mat->setZFunc(CMaterial::always);
		mat->setDoubleSided(true);

		m_QuadUV.V0 = CVector(0.f, 0.f, 0.5f);
		m_QuadUV.V1 = CVector(1.f, 0.f, 0.5f);
		m_QuadUV.V2 = CVector(1.f, 1.f, 0.5f);
		m_QuadUV.V3 = CVector(0.f, 1.f, 0.5f);

		m_QuadUV.Uv0 = CUV(0.f,  0.f);
		m_QuadUV.Uv1 = CUV(1.f, 0.f);
		m_QuadUV.Uv2 = CUV(1.f, 1.f);
		m_QuadUV.Uv3 = CUV(0.f,  1.f);
	}
}

bool CStereoDebugger::attachToDisplay()
{
	return false;
}

void CStereoDebugger::detachFromDisplay()
{

}

void CStereoDebugger::getTextures()
{
	nlassert(!m_LeftTexU);
	nlassert(!m_RightTexU);
	uint32 width, height;
	m_Driver->getWindowSize(width, height);
	m_LeftTexU = m_Driver->getRenderTargetManager().getRenderTarget(width, height);
	m_RightTexU = m_Driver->getRenderTargetManager().getRenderTarget(width, height);
	NL3D::CMaterial *mat = m_Mat.getObjectPtr();
	mat->setTexture(0, m_LeftTexU->getITexture());
	mat->setTexture(1, m_RightTexU->getITexture());
}

void CStereoDebugger::recycleTextures()
{
	nlassert(m_LeftTexU);
	nlassert(m_RightTexU);
	m_Mat.getObjectPtr()->setTexture(0, NULL);
	m_Mat.getObjectPtr()->setTexture(1, NULL);
	m_Driver->getRenderTargetManager().recycleRenderTarget(m_LeftTexU);
	m_Driver->getRenderTargetManager().recycleRenderTarget(m_RightTexU);
	m_LeftTexU = NULL;
	m_RightTexU = NULL;
}

/*
void CStereoDebugger::releaseTextures()
{
	if (!m_Mat.empty())
	{
		m_Mat.getObjectPtr()->setTexture(0, NULL);
		m_Mat.getObjectPtr()->setTexture(1, NULL);
		m_Driver->deleteMaterial(m_Mat);
	}

	delete m_LeftTexU;
	m_LeftTexU = NULL;
	m_LeftTex = NULL; // CSmartPtr

	delete m_RightTexU;
	m_RightTexU = NULL;
	m_RightTex = NULL; // CSmartPtr
}
*//*
void CStereoDebugger::initTextures()
{
	uint32 width, height;
	m_Driver->getWindowSize(width, height);
	NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(m_Driver))->getDriver();	

	m_LeftTex = new CTextureOffscreen();
	m_LeftTex->setRenderTarget(true);
	m_LeftTex->setReleasable(false);
	m_LeftTex->resize(width, height);
	m_LeftTex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	m_LeftTex->setWrapS(ITexture::Clamp);
	m_LeftTex->setWrapT(ITexture::Clamp);
	drvInternal->setupTexture(*m_LeftTex);
	m_LeftTexU = new CTextureUser(m_LeftTex);
	nlassert(!drvInternal->isTextureRectangle(m_LeftTex)); // not allowed

	m_RightTex = new CTextureOffscreen();
	m_RightTex->setRenderTarget(true);
	m_RightTex->setReleasable(false);
	m_RightTex->resize(width, height);
	m_RightTex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	m_RightTex->setWrapS(ITexture::Clamp);
	m_RightTex->setWrapT(ITexture::Clamp);
	drvInternal->setupTexture(*m_RightTex);
	m_RightTexU = new CTextureUser(m_RightTex);
	nlassert(!drvInternal->isTextureRectangle(m_RightTex)); // not allowed
}*/
/*
void CStereoDebugger::setTextures()
{
	NL3D::CMaterial *mat = m_Mat.getObjectPtr();
	mat->setTexture(0, m_LeftTex);
	mat->setTexture(1, m_RightTex);
}*/
/*
void CStereoDebugger::verifyTextures()
{
	if (m_Driver)
	{
		uint32 width, height;
		m_Driver->getWindowSize(width, height);
		if (m_LeftTex->getWidth() != width 
			|| m_RightTex->getWidth() != width 
			|| m_LeftTex->getHeight() != height 
			|| m_RightTex->getHeight() != height)
		{
			nldebug("Rebuild textures");
			releaseTextures();
			initTextures();
			setTextures();
		}
	}
}*/

/// Gets the required screen resolution for this device
bool CStereoDebugger::getScreenResolution(uint &width, uint &height)
{
	return false;
}

/// Set latest camera position etcetera
void CStereoDebugger::updateCamera(uint cid, const NL3D::UCamera *camera)
{
	m_Frustum[cid] = camera->getFrustum();
}

/// Get the frustum to use for clipping
void CStereoDebugger::getClippingFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

/// Get the original frustum of the camera
void CStereoDebugger::getOriginalFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing, as we never modified it
}

/// Is there a next pass
/// Filled mode stages: 1=L reflect, 2=R reflect, 3=L scene, 4=R scene, 5=composite
/// Non-filled mode stages: 1=reflect, 3=scene (skips 2 so want* conditions are shared)
bool CStereoDebugger::nextPass()
{
	if (m_Driver->getPolygonMode() == UDriver::Filled)
	{
		++m_Stage;
		m_SubStage = 0;
		if (m_Stage > 5)
		{
			m_Stage = 0;
			return false;
		}
		return true;
	}
	else
	{
		switch (m_Stage)
		{
		case 0:
			m_Stage = 1;
			m_SubStage = 0;
			return true;
		case 1:
			m_Stage = 3;
			m_SubStage = 0;
			return true;
		case 3:
			m_Stage = 0;
			m_SubStage = 0;
			return false;
		}
	}
	return false;
}

/// Gets the current viewport
const NL3D::CViewport &CStereoDebugger::getCurrentViewport() const
{
	if (m_Stage % 2) return m_LeftViewport;
	else return m_RightViewport;
}

/// Gets the current camera frustum
const NL3D::CFrustum &CStereoDebugger::getCurrentFrustum(uint cid) const
{
	return m_Frustum[cid];
}

/// Gets the current camera frustum
void CStereoDebugger::getCurrentFrustum(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

/// Gets the current camera matrix
void CStereoDebugger::getCurrentMatrix(uint cid, NL3D::UCamera *camera) const
{
	// do nothing
}

/// At the start of a new render target
bool CStereoDebugger::wantClear()
{
	m_SubStage = 1;
	return m_Stage >= 3 && m_Stage <= 4;
}

/// Render scene reflections
bool CStereoDebugger::wantSceneReflections()
{
	return m_Stage >= 1 && m_Stage <= 2;
}

/// The 3D scene
bool CStereoDebugger::wantScene()
{
	m_SubStage = 2;
	return m_Stage >= 3 && m_Stage <= 4;
}

/// The 3D scene end (after multiple wantScene)
bool CStereoDebugger::wantSceneEffects()
{
	return m_Stage >= 3 && m_Stage <= 4;
}

/// Interface within the 3D scene
bool CStereoDebugger::wantInterface3D()
{
	m_SubStage = 3;
	return m_Stage == 5;
}

/// 2D Interface
bool CStereoDebugger::wantInterface2D()
{
	m_SubStage = 4;
	return m_Stage == 5;
}

bool CStereoDebugger::isSceneFirst()
{
	// Odd stages are left eye (1=L reflect, 3=L scene): run traversals
	return m_Stage % 2 != 0;
}

bool CStereoDebugger::isSceneLast()
{
	// Even stages are right eye (2=R reflect, 4=R scene): release traversals
	if (m_Driver->getPolygonMode() == UDriver::Filled)
		return m_Stage % 2 == 0;
	else
		return m_Stage % 2 != 0; // wireframe: single pass per group, first=last
}

uint CStereoDebugger::getFlareContext()
{
	// Odd stages (1,3,5) = left eye → context 0, even stages (2,4) = right eye → context 2
	return (m_Stage % 2) ? 0 : 2;
}

/// Returns true if a new render target was set, always fase if not using render targets
bool CStereoDebugger::beginRenderTarget()
{
	if (m_PixelProgram && m_Stage >= 3 && m_Stage <= 4 && m_Driver && (m_Driver->getPolygonMode() == UDriver::Filled))
	{
		if (!m_LeftTexU) getTextures();
		if (m_Stage == 3) static_cast<CDriverUser *>(m_Driver)->setRenderTarget(*m_LeftTexU, 0, 0, 0, 0);
		else static_cast<CDriverUser *>(m_Driver)->setRenderTarget(*m_RightTexU, 0, 0, 0, 0);
		return true;
	}
	return false;
}

/// Returns true if a render target was fully drawn, always false if not using render targets
bool CStereoDebugger::endRenderTarget()
{
	if (m_PixelProgram && m_Stage >= 3 && m_Stage <= 4 && m_Driver && (m_Driver->getPolygonMode() == UDriver::Filled))
	{
		CTextureUser cu;
		(static_cast<CDriverUser *>(m_Driver))->setRenderTarget(cu);

		// Only draw composite after both eyes are rendered
		if (m_Stage == 4)
		{
			bool fogEnabled = m_Driver->fogEnabled();
			m_Driver->enableFog(false);

			m_Driver->setMatrixMode2D11();
			CViewport vp = CViewport();
			m_Driver->setViewport(vp);
			NL3D::IDriver *drvInternal = (static_cast<CDriverUser *>(m_Driver))->getDriver();
			NL3D::CMaterial *mat = m_Mat.getObjectPtr();
			if (s_StereoDisplayMode == 1)
			{
				// Left only
				mat->setTexture(0, m_LeftTexU->getITexture());
				mat->setTexture(1, NULL);
				drvInternal->activePixelProgram(NULL);
			}
			else if (s_StereoDisplayMode == 2)
			{
				// Right only
				mat->setTexture(0, m_RightTexU->getITexture());
				mat->setTexture(1, NULL);
				drvInternal->activePixelProgram(NULL);
			}
			else
			{
				// Comparison
				mat->setTexture(0, m_LeftTexU->getITexture());
				mat->setTexture(1, m_RightTexU->getITexture());
				drvInternal->activePixelProgram(m_PixelProgram);
			}

			m_Driver->drawQuad(m_QuadUV, m_Mat);

			drvInternal->activePixelProgram(NULL);
			m_Driver->enableFog(fogEnabled);
			recycleTextures();
		}

		return true;
	}
	return false;
}

void CStereoDebugger::listDevices(std::vector<CStereoDeviceInfo> &devicesOut)
{
	CStereoDeviceInfo devInfo;
	devInfo.Factory = new CStereoDebuggerFactory();
	devInfo.Library = CStereoDeviceInfo::NeL3D;
	devInfo.Class = CStereoDeviceInfo::StereoDisplay;
	devInfo.Manufacturer = "NeL";
	devInfo.ProductName = "Stereo Debugger";
	devInfo.Serial = "NL-3D-DEBUG";
	devInfo.AllowAuto = false;
	devicesOut.push_back(devInfo);
}

} /* namespace NL3D */

#endif /* #if !FINAL_VERSION */

/* end of file */
