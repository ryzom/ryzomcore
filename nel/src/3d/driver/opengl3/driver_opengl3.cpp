// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl3.h"
#include "driver_opengl3.h"
#include "driver_opengl3_extension.h"

// by default, we disable the windows menu keys (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU

#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/dynloadlib.h"
#include "driver_opengl3_vertex_buffer.h"
#include "driver_opengl3_uniform_buffer.h"


using namespace std;
using namespace NLMISC;


#define NL3D_GL3_FRAME_IN_FLIGHT_DEBUG 0


// ***************************************************************************
// try to allocate 16Mo by default of AGP Ram.
#define	NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE		(16384*1024)



// ***************************************************************************
#ifndef NL_STATIC

#ifdef NL_OS_WINDOWS
// dllmain::
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// Yoyo: Vianney change: don't need to call initDebug() anymore.
		// initDebug();
	}
	return true;
}

#endif /* NL_OS_WINDOWS */

class CDriverGLNelLibrary : public INelLibrary {
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CDriverGLNelLibrary)

#endif /* #ifndef NL_STATIC */

namespace NL3D {

#ifdef NL_STATIC

#ifdef USE_OPENGLES3
IDriver* createGlEs3DriverInstance ()
{
	return new NLDRIVERGL3::CDriverGL3;
}
#else
IDriver* createGl3DriverInstance ()
{
	return new NLDRIVERGL3::CDriverGL3;
}
#endif

#else

#ifdef NL_OS_WINDOWS

#ifdef NL_COMP_MINGW
extern "C"
{
#endif

__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new NLDRIVERGL3::CDriverGL3;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

#ifdef NL_COMP_MINGW
}
#endif

#elif defined (NL_OS_UNIX)

extern "C"
{
	IDriver* NL3D_createIDriverInstance ()
	{
		return new NLDRIVERGL3::CDriverGL3;
	}

	uint32 NL3D_interfaceVersion ()
	{
		return IDriver::InterfaceVersion;
	}
}

#endif // NL_OS_WINDOWS

#endif // NL_STATIC

namespace NLDRIVERGL3 {

CMaterial::CTexEnv CDriverGL3::_TexEnvReplace;


#ifdef NL_OS_WINDOWS
uint CDriverGL3::_Registered=0;
#endif // NL_OS_WINDOWS

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32 CDriverGL3::ReleaseVersion = 0x11;

GLenum CDriverGL3::NLCubeFaceToGLCubeFace[6] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
};

// ***************************************************************************
CDriverGL3::CDriverGL3()
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

#if NL3D_GL3_UNIFORM_BUFFER_DEBUG
	// Test UniformBufferFormat
	{
		CUniformBufferFormat ubf;
		testUniformBufferFormat(ubf);
		std::stringstream ss;
		generateUniformBufferGLSL(ss, ubf, NL_BUILTIN_MATERIAL_BINDING);
		nlinfo("%s", ss.str().c_str());
	}
#endif

#if defined(NL_OS_WINDOWS)

	_PBuffer = NULL;
	_hRC = NULL;
	_hDC = NULL;

#elif defined(NL_OS_MAC)

	_ctx                = nil;
	_glView             = nil;
	_backBufferHeight   = 0;
	_backBufferWidth    = 0;

	// autorelease pool for memory management
	_autoreleasePool = [[NSAutoreleasePool alloc] init];

	// init the application object
	[NSApplication sharedApplication];

	// create the menu in the top screen bar
	setupApplicationMenu();

	// finish the application launching
	[NSApp finishLaunching];

#elif defined (NL_OS_UNIX) && !defined(__EMSCRIPTEN__)

	_dpy = 0;
	_visual_info = NULL;

#	ifdef XF86VIDMODE
	// zero the old screen mode
	memset(&_OldScreenMode, 0, sizeof(_OldScreenMode));
#	endif //XF86VIDMODE

#endif // NL_OS_UNIX

	_ColorDepth = ColorDepth32;

	_DefaultCursor = EmptyCursor;
	_BlankCursor = EmptyCursor;

	// Create change basis matrix matrix
	{
		CVector I(1, 0, 0);
		CVector J(0, 0, -1);
		CVector K(0, 1, 0);
		_ChangeBasis.identity();
		_ChangeBasis.setRot(I, J, K, true);
	}

	_AlphaBlendedCursorSupported = false;
	_AlphaBlendedCursorSupportRetrieved = false;
	_CurrCol = CRGBA::White;
	_CurrRot = 0;
	_CurrHotSpotX = 0;
	_CurrHotSpotY = 0;
	_CursorScale = 1.f;
	_MouseCaptured = false;

	_win = EmptyWindow;
	_WindowX = 0;
	_WindowY = 0;
	_WindowVisible = true;
	_DestroyWindow = false;
	_Maximized = false;

	_CurrentMode.Width = 0;
	_CurrentMode.Height = 0;
	_CurrentMode.Depth = 0;
	_CurrentMode.OffScreen = false;
	_CurrentMode.Windowed = true;
	_CurrentMode.AntiAlias = -1;

	_Interval = 1;
	_Resizable = false;

	_DecorationWidth = 0;
	_DecorationHeight = 0;

	_CurrentMaterial=NULL;
	_Initialized = false;

	_FogEnabled= false;
	_FogEnd = _FogStart = 0.f;
	_FogMode = FogLinear;
	_FogDensity = 1.f;
	_CurrentFogColor[0]= 0;
	_CurrentFogColor[1]= 0;
	_CurrentFogColor[2]= 0;
	_CurrentFogColor[3]= 0;
	_FogColorOverrideBlack = false;

	_RenderTargetFBO = NULL;

	uint i;

	_ForceNormalize= false;

	/*_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;
	_CurrentVertexArrayRange= NULL;*/
	_CurrentVertexBufferGL= NULL;
	/*_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;*/

	_AllocatedTextureMemory= 0;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;

	_SumTextureMemoryUsed = false;

	_AnisotropicFilter = 0.f;

	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	for (i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
	}

	// _UserTexMat removed — texture matrices read directly from material

	// reserve enough space to never reallocate, nor test for reallocation.
	_LightMapLUT.resize(NL3D_DRV_MAX_LIGHTMAP);

	_PolygonSmooth= false;

	_VBHardProfiling= false;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;

	_TexEnvReplace.setDefault();
	_TexEnvReplace.Env.OpAlpha = CMaterial::Previous;
	_TexEnvReplace.Env.OpRGB = CMaterial::Previous;

	_WndActive = false;
	//
	_CurrentOcclusionQuery = NULL;
	_SwapBufferCounter = 0;
	_SwapBufferInFlight = 0;
	for (size_t i = 0; i < NL3D_GL3_FRAME_QUEUE_MAX; ++i)
		_SwapBufferSync[i] = 0;
	_PixelUploadPBO = 0;

#ifdef USE_OPENGLES3
	_ScratchElementBuffer = 0;
	_ScratchElementBufferSize = 0;
#endif

	_LightMapDynamicLightEnabled = false;
	_LightMapDynamicLightDirty= false;
	_LightMapDynUBOId = 0;
	memset(&_LightMapDynUBOEntry, 0, sizeof(_LightMapDynUBOEntry));
	_LightMapDynUBODirty = false;
	_UseLightMapDynUBO = false;
	_LightTableMode= false;
	_LightTableUBOId = 0;
	_LightTableDirty = false;
	_UserLightUBODirty = true;
	_LightTableUBOCapacity = 0;
	_MaxLightTableSize = NL_OPENGL3_MAX_LIGHT_TABLE_CAPACITY;
	_CameraUBOId = 0;
	_CameraUBODirty = true;
	_CameraUBOUploadDirty = false;
	_CameraUBOCapacity = 0;
	_LightTableObjCount = 0;
	_NumPerPixelLights = 0;
	for (uint i = 0; i < MaxLight; ++i)
	{
		_LightTableObjIndices[i] = -1;
		_LightTableObjFactors[i] = 0.0f;
	}

	_CurrentMaterialSupportedShader= CMaterial::Normal;

	// to avoid any problem if light0 never setted up, and ligthmap rendered
	_UserLight0.setupDirectional(CRGBA::Black, CRGBA::White, CRGBA::White, CVector::K);

	_TextureTargetCubeFace = 0;
	_TextureTargetUpload = false;

	m_UserVertexProgram = NULL;
	m_UserGeometryProgram = NULL;
	m_UserPixelProgram = NULL;
	m_DriverVertexProgram = NULL;
	m_DriverGeometryProgram = NULL;
	m_DriverPixelProgram = NULL;

	m_VPBuiltinTouched = true;
	
	// for GL ES 3.0 compatibility
#ifdef USE_OPENGLES3
	m_PPClipPlanes = true; // GL ES 3.0: use PP-based clip plane discard, no native gl_ClipDistance
	m_LinkedMegaShaders = true; // GL ES 3.0: always use linked VP+PP programs, SSO not available
	m_SupportSSO = false; // GL ES 3.0: SSO not available (no GL_ARB_separate_shader_objects)
#else
	m_PPClipPlanes = false; // GL 3.3: use native gl_ClipDistance
	m_LinkedMegaShaders = true; // GL 3.3: also useful, set true for both
	m_SupportSSO = true; // GL 3.3: separable shader objects supported
#endif
	_DriverGLStates.setPPClipPlanes(m_PPClipPlanes);
	// m_SupportNonUBOs = false; // testing strict mode, linked-only always implies ubo-only // TODO

#if !FINAL_VERSION && defined(NL_DEBUG)
	m_BuildUnusedPrograms = false; // set to test compilation of all possible mega shader variants
#else
	m_BuildUnusedPrograms = false;
#endif
	m_UseMegaShaders = true;
	m_UseMegaLightTableUBO = true;  // implied by m_UseMegaObjectUBO
	m_UseMegaCameraUBO = true;      // implied by m_UseMegaObjectUBO
	m_UseMegaObjectUBO = true;
	m_UseMegaMaterialUBO = true;
	m_PPOBound = true; // PPO is bound after initProgramPipeline()
	m_VPSpecularOutput = true;
	m_VPNormalOutput = false;
	m_VPWorldSpacePositionOutput = false;
	memset(m_ProgramNoUniforms, 0, sizeof(m_ProgramNoUniforms));
	memset(m_ProgramNoBuiltinUniforms, 0, sizeof(m_ProgramNoBuiltinUniforms));
	memset(m_ProgramOnlyUBOs, 0, sizeof(m_ProgramOnlyUBOs));
	memset(m_ProgramUsesLightTableUBO, 0, sizeof(m_ProgramUsesLightTableUBO));
	memset(m_ProgramUsesCameraUBO, 0, sizeof(m_ProgramUsesCameraUBO));
	memset(m_ProgramUsesObjectUBO, 0, sizeof(m_ProgramUsesObjectUBO));
	memset(m_ProgramUsesMaterialUBO, 0, sizeof(m_ProgramUsesMaterialUBO));
	m_NelvpActiveUB = NULL;
	_ObjectUBOId = 0;
	_ObjectUBOCapacity = 0;
	// _OverrideMaterialUBOId = 0; // Replaced by per-material UBO slots
	for (sint i = 0; i < UBBindingCount; ++i)
	{
		_BoundUserUB[i] = NULL;
		_UserUBBoundId[i] = 0;
	}
	memset(&_LightMapUBOOverride, 0, sizeof(_LightMapUBOOverride));
}

// ***************************************************************************
CDriverGL3::~CDriverGL3()
{
	H_AUTO_OGL(CDriverGL3_CDriverGLDtor)

	release();

#if defined(NL_OS_MAC)
	[_autoreleasePool release];
#endif
}

// --------------------------------------------------
bool CDriverGL3::setupDisplay()
{
	H_AUTO_OGL(CDriverGL3_setupDisplay)

	// Driver caps.
	//=============
	// Retrieve the extensions for the current context.
	registerGlExtensions (_Extensions);
	vector<string> lines;
	explode(_Extensions.toString(), string("\n"), lines);
	for (uint i = 0; i < lines.size(); i++)
		nlinfo("3D: %s", lines[i].c_str());

#if defined(NL_OS_WINDOWS)
	registerWGlExtensions(_Extensions, _hDC);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
	registerGlXExtensions(_Extensions, _dpy, DefaultScreen(_dpy));
#endif // NL_OS_WINDOWS

	// Determine runtime light table size based on ANGLE/platform detection
	_MaxLightTableSize = NL_OPENGL3_MAX_LIGHT_TABLE_CAPACITY;
#ifdef __EMSCRIPTEN__
	if (_Extensions.IsWindowsPlatform || _Extensions.IsAndroidPlatform) // WebGL on Windows/Android: assume poor GLES translation
		_MaxLightTableSize = NL_OPENGL3_ANGLE_MAX_LIGHT_TABLE;
#else
	if (_Extensions.IsANGLE) // Desktop: only if ANGLE detected via GL_RENDERER
		_MaxLightTableSize = NL_OPENGL3_ANGLE_MAX_LIGHT_TABLE;
#endif
	nlinfo("3D: Light table size: %d (ANGLE: %s, Windows: %s, Android: %s)",
		_MaxLightTableSize,
		_Extensions.IsANGLE ? "yes" : "no",
		_Extensions.IsWindowsPlatform ? "yes" : "no",
		_Extensions.IsAndroidPlatform ? "yes" : "no");

	// Check required extensions!!
	if (!_Extensions.GLCore)
	{
		nlwarning("Missing required GL 3.30 Core features. Update your driver");
		throw EBadDisplay("Missing required GL 3.30 Core features. Update your driver");
	}
	if (m_SupportSSO)
	{
		if (!_Extensions.ARBSeparateShaderObjects)
		{
			m_SupportSSO = false;
			if (!m_UseMegaShaders || !m_LinkedMegaShaders)
			{
				nlwarning("Missing required GL extension: GL_ARB_separate_shader_objects. Update your driver");
				throw EBadDisplay("Missing required GL extension: GL_ARB_separate_shader_objects. Update your driver");
			}
			else
			{
				nlwarning("Missing recommended GL extension: GL_ARB_separate_shader_objects. Update your driver");
			}
		}
	}

	// All User Light are disabled by Default
	for (uint i = 0; i < MaxLight; ++i)
	{
		_UserLightEnable[i] = false;
		_LightEnable[i] = false;
		touchLightVP(i);
	}

	// Default global ambient matches OpenGL default (0.2, 0.2, 0.2)
	_AmbientGlobal = NLMISC::CRGBA(51, 51, 51);

	// All clip planes disabled by default
	for (uint i = 0; i < MaxClipPlanes; ++i)
	{
		_ClipPlaneEnabled[i] = false;
		_ClipPlaneEye[i][0] = 0.f;
		_ClipPlaneEye[i][1] = 0.f;
		_ClipPlaneEye[i][2] = 0.f;
		_ClipPlaneEye[i][3] = 0.f;
	}

	// Init OpenGL/Driver defaults.
	//=============================
	_DriverGLStates.init();
	_DriverGLStates.viewport(0, 0, _CurrentMode.Width, _CurrentMode.Height);
	glEnable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	_DriverGLStates.polygonMode(GL_FILL);
	//glDisable(GL_NORMALIZE);
	//glDisable(GL_COLOR_SUM_EXT); // FIXME GL3

	_CurrViewport.init(0.f, 0.f, 1.f, 1.f);
	_CurrScissor.initFullScreen();
	_ForceNormalize = false;
	// Setup defaults for blend, lighting ...
	_DriverGLStates.forceDefaults();
	// Default delta camera pos.
	_PZBCameraPos = CVector::Null;

	// Init VertexArrayRange according to supported extenstion.
	/*_AGPVertexArrayRange = new CVertexArrayRange(this);
	_VRAMVertexArrayRange = new CVertexArrayRange(this);

	// Reset VertexArrayRange.
	_CurrentVertexArrayRange = NULL;
	_CurrentVertexBufferGL = NULL;
	_NVCurrentVARPtr = NULL;
	_NVCurrentVARSize = 0;*/

	initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

	// Init embm if present
	//===========================================================
	initEMBM();

	// Activate the default texture environnments for all stages.
	//===========================================================
	for (uint stage = 0; stage < IDRV_PROGRAM_MAXSAMPLERS; ++stage)
	{
		// init no texture.
		_CurrentTexture[stage] = NULL;
		_CurrentTextureInfoGL[stage] = NULL;
		// init no texture.
	}
	for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
	{
		// init default env.
		CMaterial::CTexEnv env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);

		// set All TexGen by default to identity matrix (prefer use the textureMatrix scheme)
		setTexGenModeVP(stage, TexGenDisabled);
	}

	if (!initProgramPipeline())
		nlerror("Failed to create Pipeline Object");

	// Create UBOs
	nglGenBuffers(1, &_LightTableUBOId);
	nglGenBuffers(1, &_LightMapDynUBOId);
	nglGenBuffers(1, &_CameraUBOId);
	nglGenBuffers(1, &_ObjectUBOId);
	// nglGenBuffers(1, &_OverrideMaterialUBOId); // Replaced by per-material UBO slots
	nglGenBuffers(1, &_PixelUploadPBO);

#ifdef USE_OPENGLES3
	// WebGL 2.0 does not support client-side index arrays.
	// Create a scratch element buffer for uploading index data before draw calls.
	nglGenBuffers(1, &_ScratchElementBuffer);
#endif

	// Pre-allocate UBOs to their full declared sizes.
	// WebGL 2.0 requires bound UBOs to be at least as large as the
	// corresponding uniform block in the shader, even before first use.
	{
		// NlLightTable: _MaxLightTableSize × NlLightInfo (96 bytes each)
		_DriverGLStates.forceBindUniformBuffer(_LightTableUBOId);
		nglBufferData(GL_UNIFORM_BUFFER, _MaxLightTableSize * sizeof(CLightTableUBOEntry), NULL, GL_STREAM_DRAW);
		_LightTableUBOCapacity = _MaxLightTableSize;

		// NlLightTable binding for lightmap dynamic UBO (1 entry)
		_DriverGLStates.forceBindUniformBuffer(_LightMapDynUBOId);
		nglBufferData(GL_UNIFORM_BUFFER, sizeof(CLightTableUBOEntry), NULL, GL_STREAM_DRAW);

		// NlCamera UBO
		_DriverGLStates.forceBindUniformBuffer(_CameraUBOId);
		nglBufferData(GL_UNIFORM_BUFFER, sizeof(CCameraUBOData), NULL, GL_STREAM_DRAW);
		_CameraUBOCapacity = sizeof(CCameraUBOData);

		// NlModel (object) UBO
		_DriverGLStates.forceBindUniformBuffer(_ObjectUBOId);
		nglBufferData(GL_UNIFORM_BUFFER, sizeof(CObjectUBOData), NULL, GL_STREAM_DRAW);

		_DriverGLStates.forceBindUniformBuffer(0);
	}

	if (m_UseMegaShaders)
	{
		if (!initMegaVertexPrograms())
		{
			nlwarning("GL3: Failed to init mega vertex programs, falling back to per-material shaders");
#ifdef NL_DEBUG
			nlerror("GL3: Mega vertex program init failed");
#endif
			m_UseMegaShaders = false;
		}
		else if (!initMegaPixelPrograms())
		{
			nlwarning("GL3: Failed to init mega pixel programs, falling back to per-material shaders");
#ifdef NL_DEBUG
			nlerror("GL3: Mega pixel program init failed");
#endif
			m_UseMegaShaders = false;
		}
		else
			nlinfo("GL3: Mega shaders initialized");
	}

	if (m_LinkedMegaShaders && m_UseMegaShaders)
	{
		if (!initMegaLinkedPrograms())
		{
			nlwarning("GL3: Failed to link mega programs, falling back to SSO");
#ifdef NL_DEBUG
			nlerror("GL3: Mega linked program init failed");
#endif
			m_LinkedMegaShaders = false;
		}
		else
			nlinfo("GL3: Linked mega shaders initialized");
	}

	_PPLExponent = 1.f;
	_PPLightDiffuseColor = NLMISC::CRGBA::White;
	_PPLightSpecularColor = NLMISC::CRGBA::White;

	// Backward compatibility: default lighting is Light0 default openGL
	// meaning that light direction is always (0,1,0) in eye-space
	// use enableLighting(0....), to get normal behaviour
	// _DriverGLStates.enableLight(0, true); // FIXME GL3 VERTEX PROGRAM
	_LightMode[0] = CLight::DirectionalLight;
	_WorldLightDirection[0] = CVector::Null;

	_Initialized = true;

	_ForceDXTCCompression = false;
	_ForceTextureResizePower = 0;

	// Reset profiling.
	_AllocatedTextureMemory = 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall = 0;
	_NbSetupModelMatrixCall = 0;

	// Reset the vbl interval
	setSwapVBLInterval(_Interval);

	return true;
}

// ***************************************************************************
// Hardware-accelerated texture blit with scaling (used by bloom to downscale
// the scene into the blur texture). Returning false is fine: CDriverUser
// falls back to rendering a quad into the destination render target, which
// achieves the same result. The old GL driver also returns false here;
// only D3D has a native implementation via IDirect3DDevice9::StretchRect.
bool CDriverGL3::stretchRect(ITexture * /* srcText */, NLMISC::CRect &/* srcRect */, ITexture * /* destText */, NLMISC::CRect &/* destRect */)
{
	H_AUTO_OGL(CDriverGL3_stretchRect)

	return false;
}

// ***************************************************************************
bool CDriverGL3::supportBloomEffect() const
{
	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportNonPowerOfTwoTextures() const
{
	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::isTextureRectangle(ITexture * tex) const
{
	return (supportTextureRectangle() && tex->isOffscreenTexture() && tex->mipMapOff()
			&& (!isPowerOf2(tex->getWidth()) || !isPowerOf2(tex->getHeight())));
}

// ***************************************************************************
bool CDriverGL3::activeFrameBufferObject(ITexture * tex)
{
	if (supportFrameBufferObject())
	{
		if (tex)
		{
			// Ensure the texture is set up in the driver before accessing its driver info
			if (!tex->TextureDrvShare || !tex->TextureDrvShare->DrvTexture)
				setupTexture(*tex);
			CTextureDrvInfosGL3*	gltext = (CTextureDrvInfosGL3*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
			return gltext->activeFrameBufferObject(tex);
		}
		else
		{
			_DriverGLStates.forceBindFramebuffer(0);
			return true;
		}
	}

	return false;
}

// --------------------------------------------------
bool CDriverGL3::isTextureExist(const ITexture&tex)
{
	H_AUTO_OGL(CDriverGL3_isTextureExist)
	bool result;

	// Create the shared Name.
	std::string	name;
	getTextureShareName (tex, name);

	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		result = (rTexDrvInfos.find(name) != rTexDrvInfos.end());
	}
	return result;
}

// --------------------------------------------------
bool CDriverGL3::clear2D(CRGBA rgba)
{
	H_AUTO_OGL(CDriverGL3_clear2D)

	NLMISC::CRGBAF rgbaf(rgba);
	GLfloat fv[] = { rgbaf.R, rgbaf.G, rgbaf.B, rgbaf.A };
	nglClearBufferfv(GL_COLOR, 0, fv);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearZBuffer(float zval)
{
	H_AUTO_OGL(CDriverGL3_clearZBuffer);

	_DriverGLStates.enableZWrite(true);
	nglClearBufferfv(GL_DEPTH, 0, &zval);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearStencilBuffer(sint stencilval)
{
	H_AUTO_OGL(CDriverGL3_clearStencilBuffer)

	nglClearBufferiv(GL_STENCIL, 0, &stencilval);

	return true;
}

// --------------------------------------------------
void CDriverGL3::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	H_AUTO_OGL(CDriverGL3_setColorMask)
	_DriverGLStates.colorMask(bRed, bGreen, bBlue, bAlpha);
}

// --------------------------------------------------
bool CDriverGL3::isFrameReady()
{
	size_t syncI = _SwapBufferCounter % NL3D_GL3_FRAME_QUEUE_MAX;
	if (_SwapBufferSync[syncI])
	{
		// Non-blocking check: is the oldest fence signaled?
		GLint status = GL_UNSIGNALED;
		GLsizei len = 0;
		nglGetSynciv(_SwapBufferSync[syncI], GL_SYNC_STATUS, 1, &len, &status);
		if (len == 0 || status != GL_SIGNALED)
			return false; // GPU still processing or query failed, skip this frame
	}
	return true;
}

// --------------------------------------------------
bool CDriverGL3::swapBuffers()
{
	H_AUTO_OGL(CDriverGL3_swapBuffers);

	// Set fence
	size_t syncI = _SwapBufferCounter % NL3D_GL3_FRAME_QUEUE_MAX;
	if (_SwapBufferSync[syncI]) // Wait for oldest fence, if this is still in flight
	{
#if NL3D_GL3_FRAME_IN_FLIGHT_DEBUG
		nldebug("Wait for oldest fence");
#endif
#ifdef __EMSCRIPTEN__
		// On Emscripten, we cannot block. Callers should check isFrameReady() first.
		// If the sync is still unsignaled (caller didn't check), log a warning.
		{
			GLint status = GL_UNSIGNALED;
			GLsizei len = 0;
			nglGetSynciv(_SwapBufferSync[syncI], GL_SYNC_STATUS, 1, &len, &status);
			if (status != GL_SIGNALED)
				nldebug("GL3: swapBuffers called while GPU sync not yet signaled (caller should check isFrameReady())");
		}
#else
		GLenum syncR = nglClientWaitSync(_SwapBufferSync[syncI], 0, 1000000000ULL);
		switch (syncR)
		{
		case GL_ALREADY_SIGNALED:
		case GL_CONDITION_SATISFIED:
			break;
		case GL_TIMEOUT_EXPIRED:
			nlwarning("Timeout expired for glClientWaitSync");
			break;
		case GL_WAIT_FAILED:
			nlwarning("Wait failed for glClientWaitSync");
			break;
		default:
			nlwarning("Unknown glClientWaitSync result");
		}
#endif
		nglDeleteSync(_SwapBufferSync[syncI]);
		++_SwapBufferInFlight;
	}
	// Fence occurs before the frame swap, because it only is relevant for user supplied buffers. The framebuffer is not our concern
	_SwapBufferSync[syncI] = nglFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	nlassert(_SwapBufferSync[syncI]);

	++_SwapBufferCounter;

	if (!_WndActive)
	{
		updateLostBuffers();
	}

#if defined(NL_OS_WINDOWS)

	SwapBuffers(_hDC);

#elif defined(NL_OS_MAC)

	// TODO: maybe do this somewhere else?
	if (_DestroyWindow)
	{
		[_autoreleasePool release];
		_autoreleasePool = [[NSAutoreleasePool alloc] init];
	}

	[_ctx flushBuffer];

#elif defined (NL_OS_UNIX)

#ifndef __EMSCRIPTEN__
	glXSwapBuffers(_dpy, _win);
#endif

#endif // NL_OS_WINDOWS

	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o)).
	for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// init no texture.
		setTexGenModeVP(stage, TexGenDisabled);
	}

	// Activate the default material.
	//===========================================================
	// Same reasoning as textures :)
	_DriverGLStates.forceDefaults();

	_CurrentMaterial= NULL;

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset Profile VBHardLock
	if (_VBHardProfiling)
	{
		_CurVBHardLockCount= 0;
		_NumVBHardProfileFrame++;
	}

	// Check all vertex buffer to see which one are lost
	updateLostBuffers();

#if NL3D_GL3_SYNC_IMMEDIATE
	// Sync on the current buffer
	nlassert(_SwapBufferSync[syncI]);
	nglClientWaitSync(_SwapBufferSync[syncI], 0, 1000000000ULL);
#endif
	
	// Check in flight buffers, also checks the current one
	for (size_t i = 0; i < NL3D_GL3_FRAME_QUEUE_MAX; ++i)
	{
		size_t syncJ = (syncI + 1 + i) % NL3D_GL3_FRAME_QUEUE_MAX;
		if (_SwapBufferSync[syncJ]) // If there's a frame in flight
		{
			GLint status = 0;
			nglGetSynciv(_SwapBufferSync[syncJ], GL_SYNC_STATUS, 1, NULL, &status);
			if (status == GL_SIGNALED)
			{
				// Frame is no longer in flight
				nglDeleteSync(_SwapBufferSync[syncJ]);
				_SwapBufferSync[syncJ] = 0;
#if NL3D_GL3_FRAME_IN_FLIGHT_DEBUG
				nldebug("Frame %u no longer in flight", (unsigned int)_SwapBufferInFlight);
#endif
				++_SwapBufferInFlight;
				nlassert(_SwapBufferInFlight <= _SwapBufferCounter);
			}
			else
			{
				// Following frames are still in flight
				break;
			}
		}
	}

	nlassert(_SwapBufferInFlight <= _SwapBufferCounter);
#if NL3D_GL3_FRAME_IN_FLIGHT_DEBUG
	nldebug("Swap buffer: %u, in flight: %u", (unsigned int)_SwapBufferCounter, (unsigned int)_SwapBufferInFlight);
#endif

	return true;
}

// --------------------------------------------------
bool CDriverGL3::release()
{
	H_AUTO_OGL(CDriverGL3_release)

	// release only if the driver was initialized
	if (!_Initialized) return true;

	// hide window
	showWindow(false);

	m_DriverVertexProgram = NULL;
	m_DriverGeometryProgram = NULL;
	m_DriverPixelProgram = NULL;

	m_UserVertexProgram = NULL;
	m_UserGeometryProgram = NULL;
	m_UserPixelProgram = NULL;

	// Delete all cached programs
	for (CHashSet<CVPBuiltin, CVPBuiltinHashTraits>::iterator it(m_VPBuiltinCache.begin()), end(m_VPBuiltinCache.end()); it != end; ++it)
		delete it->VertexProgram;
	m_VPBuiltinCache.clear();
	for (CHashSet<CPPBuiltin, CPPBuiltinHashTraits>::iterator it(m_PPBuiltinCache.begin()), end(m_PPBuiltinCache.end()); it != end; ++it)
		delete it->PixelProgram;
	m_PPBuiltinCache.clear();

	// Delete UBOs
	if (_LightTableUBOId)
	{
		nglDeleteBuffers(1, &_LightTableUBOId);
		_LightTableUBOId = 0;
	}
	if (_LightMapDynUBOId)
	{
		nglDeleteBuffers(1, &_LightMapDynUBOId);
		_LightMapDynUBOId = 0;
	}
	if (_CameraUBOId)
	{
		nglDeleteBuffers(1, &_CameraUBOId);
		_CameraUBOId = 0;
	}
	if (_ObjectUBOId)
	{
		nglDeleteBuffers(1, &_ObjectUBOId);
		_ObjectUBOId = 0;
	}
	// Replaced by per-material UBO slots
	// if (_OverrideMaterialUBOId)
	// {
	// 	nglDeleteBuffers(1, &_OverrideMaterialUBOId);
	// 	_OverrideMaterialUBOId = 0;
	// }
	if (_PixelUploadPBO)
	{
		nglDeleteBuffers(1, &_PixelUploadPBO);
		_PixelUploadPBO = 0;
	}

#ifdef USE_OPENGLES3
	if (_ScratchElementBuffer)
	{
		nglDeleteBuffers(1, &_ScratchElementBuffer);
		_ScratchElementBuffer = 0;
	}
#endif

	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	nlassert(_DepthStencilFBOs.empty());

	_SwapBufferCounter = 0;
	_SwapBufferInFlight = 0;
	for (size_t i = 0; i < NL3D_GL3_FRAME_QUEUE_MAX; ++i)
	{
		if (_SwapBufferSync[i])
		{
			nglDeleteSync(_SwapBufferSync[i]);
			_SwapBufferSync[i] = 0;
		}
	}

	// delete querries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	// release caustic cube map
//	_CauticCubeMap = NULL;

	// Make sure vertex buffers are really all gone
	// FIXME VERTEXBUFFER

	// Delete default VAO
	_DriverGLStates.release();

	// destroy window and associated ressources
	destroyWindow();

	// other uninitializations
	unInit();

	// released
	_Initialized= false;

	return true;
}

// --------------------------------------------------
void CDriverGL3::setupViewport (const class CViewport& viewport)
{
	H_AUTO_OGL(CDriverGL3_setupViewport)

	if (_win == EmptyWindow) return;

	// Setup gl viewport
	uint32 clientWidth, clientHeight;
	getRenderTargetSize(clientWidth, clientHeight);

	// Backup the viewport
	_CurrViewport = viewport;

	// Get viewport
	float x;
	float y;
	float width;
	float height;
	viewport.getValues (x, y, width, height);

	// Render to texture : adjuste the viewport
	if (_TextureTarget)
	{
		float factorX = 1;
		float factorY = 1;
		if (clientWidth)
			factorX = (float)_TextureTarget->getWidth() / (float)clientWidth;
		if (clientHeight)
			factorY = (float)_TextureTarget->getHeight() / (float)clientHeight;
		x *= factorX;
		y *= factorY;
		width *= factorX;
		height *= factorY;
	}

	// Setup gl viewport
	sint ix=(sint)((float)clientWidth*x+0.5f);
	clamp (ix, 0, (sint)clientWidth);
	sint iy=(sint)((float)clientHeight*y+0.5f);
	clamp (iy, 0, (sint)clientHeight);
	sint iwidth=(sint)((float)clientWidth*width+0.5f);
	clamp (iwidth, 0, (sint)clientWidth-ix);
	sint iheight=(sint)((float)clientHeight*height+0.5f);
	clamp (iheight, 0, (sint)clientHeight-iy);
	_DriverGLStates.viewport(ix, iy, iwidth, iheight);
}

// --------------------------------------------------
void CDriverGL3::getViewport(CViewport &viewport)
{
	H_AUTO_OGL(CDriverGL3_getViewport)
	viewport = _CurrViewport;
}

// --------------------------------------------------
void CDriverGL3::setupScissor (const class CScissor& scissor)
{
	H_AUTO_OGL(CDriverGL3_setupScissor)

	if (_win == EmptyWindow) return;

	// Setup gl viewport
	uint32 clientWidth, clientHeight;
	getRenderTargetSize(clientWidth, clientHeight);

	// Backup the scissor
	_CurrScissor= scissor;

	// Get scissor
	float x= scissor.X;
	float y= scissor.Y;
	float width= scissor.Width;
	float height= scissor.Height;

	// Render to texture : adjuste the scissor
	if (_TextureTarget)
	{
		float factorX = 1;
		float factorY = 1;
		if (clientWidth)
			factorX = (float) _TextureTarget->getWidth() / (float)clientWidth;
		if (clientHeight)
			factorY = (float) _TextureTarget->getHeight() / (float)clientHeight;
		x *= factorX;
		y *= factorY;
		width *= factorX;
		height *= factorY;
	}

	// enable or disable Scissor, but AFTER textureTarget adjust
	if (x==0.f && y==0.f && width>=1.f && height>=1.f)
	{
		_DriverGLStates.enableScissorTest(false);
	}
	else
	{
		// Setup gl scissor
		sint ix0=(sint)floor((float)clientWidth * x + 0.5f);
		clamp (ix0, 0, (sint)clientWidth);
		sint iy0=(sint)floor((float)clientHeight* y + 0.5f);
		clamp (iy0, 0, (sint)clientHeight);

		sint ix1=(sint)floor((float)clientWidth * (x+width) + 0.5f);
		clamp (ix1, 0, (sint)clientWidth);
		sint iy1=(sint)floor((float)clientHeight* (y+height) + 0.5f);
		clamp (iy1, 0, (sint)clientHeight);

		sint iwidth= ix1 - ix0;
		clamp (iwidth, 0, (sint)clientWidth);
		sint iheight= iy1 - iy0;
		clamp (iheight, 0, (sint)clientHeight);

		_DriverGLStates.scissor(ix0, iy0, iwidth, iheight);
		_DriverGLStates.enableScissorTest(true);
	}
}

uint8 CDriverGL3::getBitPerPixel ()
{
	H_AUTO_OGL(CDriverGL3_getBitPerPixel)
	return _CurrentMode.Depth;
}

const char *CDriverGL3::getVideocardInformation ()
{
	H_AUTO_OGL(CDriverGL3_getVideocardInformation)
	static char name[1024];

	if (!_Initialized) return "OpenGL isn't initialized";

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);
	const char *version = (const char *) glGetString (GL_VERSION);

	smprintf(name, 1024, "OpenGL / %s / %s / %s", vendor, renderer, version);
	return name;
}

bool CDriverGL3::clipRect(NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_clipRect)
	// Clip the wanted rectangle with window.
	uint32 width, height;
	getWindowSize(width, height);

	sint32	xr=rect.right() ,yr=rect.bottom();

	clamp((sint32&)rect.X, (sint32)0, (sint32)width);
	clamp((sint32&)rect.Y, (sint32)0, (sint32)height);
	clamp((sint32&)xr, (sint32)rect.X, (sint32)width);
	clamp((sint32&)yr, (sint32)rect.Y, (sint32)height);
	rect.Width= xr-rect.X;
	rect.Height= yr-rect.Y;

	return rect.Width>0 && rect.Height>0;
}

void CDriverGL3::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_getBufferPart)
	bitmap.reset();

	if (clipRect(rect))
	{
		bitmap.resize(rect.Width, rect.Height, CBitmap::RGBA);
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels ().getPtr());
	}
}

void CDriverGL3::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_getZBufferPart)
	zbuffer.clear();

	if (clipRect(rect))
	{
		zbuffer.resize(rect.Width*rect.Height);

		// glPixelTransferf(GL_DEPTH_SCALE/GL_DEPTH_BIAS) removed: does not exist
		// in GL 3.3 core profile. Depth readback returns raw values without transfer.
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_DEPTH_COMPONENT , GL_FLOAT, &(zbuffer[0]));
	}
}

void CDriverGL3::getZBuffer (std::vector<float>  &zbuffer)
{
	H_AUTO_OGL(CDriverGL3_getZBuffer)
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getZBufferPart(zbuffer, rect);
}

void CDriverGL3::getBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL3_getBuffer)
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getBufferPart(bitmap, rect);
	bitmap.flipV();
}

bool CDriverGL3::fillBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL3_fillBuffer)
	// fillBuffer is deprecated (movie shooter). glDrawPixels does not exist in GL 3.3 Core.
	return false;
}

// ***************************************************************************
void CDriverGL3::copyFrameBufferToTexture(ITexture *tex,
										 uint32 level,
										 uint32 offsetx,
										 uint32 offsety,
										 uint32 x,
										 uint32 y,
										 uint32 width,
										 uint32 height,
										 uint cubeFace /*= 0*/
										)
{
	H_AUTO_OGL(CDriverGL3_copyFrameBufferToTexture)
	bool compressed = false;
	getGlTextureFormat(*tex, compressed);
	nlassert(!compressed);
	// first, mark the texture as valid, and make sure there is a corresponding texture in the device memory
	setupTexture(*tex);
	CTextureDrvInfosGL3*	gltext = (CTextureDrvInfosGL3*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(NULL);
	_DriverGLStates.activeTexture(0);
	// setup texture mode, after activeTexture()
	// FIXME GL3 TEXTUREMODE CDriverGLStates3::TTextureMode textureMode= CDriverGLStates3::Texture2D;

	// FIXME GL3 TEXTUREMODE if (gltext->TextureMode == GL_TEXTURE_RECTANGLE)
	// FIXME GL3 TEXTUREMODE 	textureMode = CDriverGLStates3::TextureRect;

	// FIXME GL3 TEXTUREMODE _DriverGLStates.setTextureMode(textureMode);
	if (tex->isTextureCube())
	{
		_DriverGLStates.forceBindTexture(GL_TEXTURE_CUBE_MAP, gltext->ID);
		glCopyTexSubImage2D(NLCubeFaceToGLCubeFace[cubeFace], level, offsetx, offsety, x, y, width, height);
	}
	else
	{
		_DriverGLStates.forceBindTexture(gltext->TextureMode, gltext->ID);
		glCopyTexSubImage2D(gltext->TextureMode, level, offsetx, offsety, x, y, width, height);
	}
	// disable texturing.
	// FIXME GL3 TEXTUREMODE _DriverGLStates.setTextureMode(CDriverGLStates3::TextureDisabled);
	_CurrentTexture[0] = NULL;
	_CurrentTextureInfoGL[0] = NULL;
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(tex);
}

// ***************************************************************************
void			CDriverGL3::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{
	H_AUTO_OGL(CDriverGL3_profileRenderedPrimitives)
	pIn= _PrimitiveProfileIn;
	pOut= _PrimitiveProfileOut;
}


// ***************************************************************************
uint32			CDriverGL3::profileAllocatedTextureMemory()
{
	H_AUTO_OGL(CDriverGL3_profileAllocatedTextureMemory)
	return _AllocatedTextureMemory;
}


// ***************************************************************************
uint32			CDriverGL3::profileSetupedMaterials() const
{
	H_AUTO_OGL(CDriverGL3_profileSetupedMaterials)
	return _NbSetupMaterialCall;
}


// ***************************************************************************
uint32			CDriverGL3::profileSetupedModelMatrix() const
{
	H_AUTO_OGL(CDriverGL3_profileSetupedModelMatrix)

	return _NbSetupModelMatrixCall;
}


// ***************************************************************************
void			CDriverGL3::enableUsedTextureMemorySum (bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableUsedTextureMemorySum)

	if (enable)
		nlinfo ("3D: PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverGL3");
	_SumTextureMemoryUsed=enable;
}


// ***************************************************************************
uint32			CDriverGL3::getUsedTextureMemory() const
{
	H_AUTO_OGL(CDriverGL3_getUsedTextureMemory)

	// Sum memory used
	uint32 memory=0;

	// For each texture used
	set<CTextureDrvInfosGL3*>::const_iterator ite=_TextureUsed.begin();
	while (ite!=_TextureUsed.end())
	{
		// Get the gl texture
		CTextureDrvInfosGL3*	gltext;
		gltext= (*ite);

		// Sum the memory used by this texture
		memory+=gltext->TextureMemory;

		// Next texture
		ite++;
	}

	// Return the count
	return memory;
}


// ***************************************************************************
void CDriverGL3::setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4])
{
	H_AUTO_OGL(CDriverGL3_setMatrix2DForTextureOffsetAddrMode)

	if (!supportTextureShaders()) return;
	//nlassert(supportTextureShaders());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	_DriverGLStates.activeTexture(stage);

	//glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat);
}

// ***************************************************************************
bool CDriverGL3::supportPerPixelLighting(bool specular) const
{
	H_AUTO_OGL(CDriverGL3_supportPerPixelLighting)

	// Per-pixel lighting is deprecated. The replacement is supportWorldSpacePPL().
	return false;
}

// ***************************************************************************
void CDriverGL3::setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess)
{
	H_AUTO_OGL(CDriverGL3_setPerPixelLightingLight)

	_PPLExponent = shininess;
	_PPLightDiffuseColor = diffuse;
	_PPLightSpecularColor = specular;
}

// ***************************************************************************
bool CDriverGL3::supportWorldSpacePPL() const
{
	return true;
}

// ***************************************************************************
uint			CDriverGL3::getNbTextureStages() const
{
	H_AUTO_OGL(CDriverGL3_getNbTextureStages)

	return IDRV_MAT_MAXTEXTURES; // Must return this for compatibility
}

// ***************************************************************************
bool CDriverGL3::supportEMBM() const
{
	H_AUTO_OGL(CDriverGL3_supportEMBM);

	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::isEMBMSupportedAtStage(uint stage) const
{
	H_AUTO_OGL(CDriverGL3_isEMBMSupportedAtStage)

	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return true;
}

// ***************************************************************************
void CDriverGL3::setEMBMMatrix(const uint stage,const float mat[4])
{
	H_AUTO_OGL(CDriverGL3_setEMBMMatrix)

	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	memcpy(_EMBMMatrix[stage], mat, sizeof(float) * 4);
}

// ***************************************************************************
void CDriverGL3::initEMBM()
{
	H_AUTO_OGL(CDriverGL3_initEMBM);

	for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		_EMBMMatrix[i][0] = 0.0f;
		_EMBMMatrix[i][1] = 0.0f;
		_EMBMMatrix[i][2] = 0.0f;
		_EMBMMatrix[i][3] = 0.0f;
	}
}

// ***************************************************************************
void CDriverGL3::forceNativeFragmentPrograms(bool nativeOnly)
{
}

// ***************************************************************************
void CDriverGL3::finish()
{
	H_AUTO_OGL(CDriverGL3_finish)
	glFinish();
}

// ***************************************************************************
void CDriverGL3::flush()
{
	H_AUTO_OGL(CDriverGL3_flush)
	glFlush();
}

// ***************************************************************************
void	CDriverGL3::setSwapVBLInterval(uint interval)
{
	H_AUTO_OGL(CDriverGL3_setSwapVBLInterval)

	if (!_Initialized)
		return;

	bool res = true;

#if defined(NL_OS_WINDOWS)
	if (_Extensions.WGLEXTSwapControl)
	{
		res = nwglSwapIntervalEXT(interval) == TRUE;
	}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
	if (_win && _Extensions.GLXEXTSwapControl)
	{
		res = nglXSwapIntervalEXT(_dpy, _win, interval) == 0;
	}
	else if (_Extensions.GLXSGISwapControl)
	{
		res = nglXSwapIntervalSGI(interval) == 0;
	}
	else if (_Extensions.GLXMESASwapControl)
	{
		res = nglXSwapIntervalMESA(interval) == 0;
	}
#endif

	if (res)
	{
		_Interval = interval;
	}
	else
	{
		nlwarning("Could not set swap interval");
	}
}

// ***************************************************************************
uint	CDriverGL3::getSwapVBLInterval()
{
	H_AUTO_OGL(CDriverGL3_getSwapVBLInterval)

#if defined(NL_OS_WINDOWS)
	if (_Extensions.WGLEXTSwapControl)
	{
		return nwglGetSwapIntervalEXT();
	}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX) && !defined(__EMSCRIPTEN__)
	if (_win && _Extensions.GLXEXTSwapControl)
	{
		uint swap, maxSwap;
		glXQueryDrawable(_dpy, _win, GLX_SWAP_INTERVAL_EXT, &swap);
		glXQueryDrawable(_dpy, _win, GLX_MAX_SWAP_INTERVAL_EXT, &maxSwap);
		nlwarning("The swap interval is %u and the max swap interval is %u", swap, maxSwap);
		return swap;
	}
	else if (_Extensions.GLXMESASwapControl)
	{
		return nglXGetSwapIntervalMESA();
	}
#endif

	return _Interval;
}

// ***************************************************************************
void	CDriverGL3::startProfileVBHardLock()
{
	if (_VBHardProfiling)
		return;

	// start
	_VBHardProfiles.clear();
	_VBHardProfiles.reserve(50);
	_VBHardProfiling= true;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}

// ***************************************************************************
void	CDriverGL3::endProfileVBHardLock(vector<std::string> &result)
{
	if (!_VBHardProfiling)
		return;

	// Fill infos.
	result.clear();
	result.resize(_VBHardProfiles.size() + 1);
	float	total= 0;
	for (uint i=0;i<_VBHardProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CVBHardProfile	&vbProf= _VBHardProfiles[i];
		const char	*vbName;
		if (vbProf.VBHard && !vbProf.VBHard->getName().empty())
		{
			vbName= vbProf.VBHard->getName().c_str();
		}
		else
		{
			vbName= "????";
		}
		// Display in ms.
		float	timeLock= (float)CTime::ticksToSecond(vbProf.AccumTime)*1000 / max(_NumVBHardProfileFrame,1U);
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", vbName, vbProf.Change?'*':' ', timeLock);
		total+= timeLock;

		result[i]= tmp;
	}
	result[_VBHardProfiles.size()]= toString("Total: %2.3f", total);

	// clear.
	_VBHardProfiling= false;
	_VBHardProfiles.clear();
	std::vector<CVBHardProfile>().swap(_VBHardProfiles);
}

// ***************************************************************************
void	CDriverGL3::appendVBHardLockProfile(NLMISC::TTicks time, CVertexBuffer *vb)
{
	// must allocate a new place?
	if (_CurVBHardLockCount>=_VBHardProfiles.size())
	{
		_VBHardProfiles.resize(_VBHardProfiles.size()+1);
		// set the original VBHard
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
	}

	// Accumulate.
	_VBHardProfiles[_CurVBHardLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if (_VBHardProfiles[_CurVBHardLockCount].VBHard != vb)
	{
		// flag, and set new
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
		_VBHardProfiles[_CurVBHardLockCount].Change= true;
	}

	// next!
	_CurVBHardLockCount++;
}

// ***************************************************************************
void CDriverGL3::startProfileIBLock()
{
	// not implemented
}

// ***************************************************************************
void CDriverGL3::endProfileIBLock(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void CDriverGL3::profileIBAllocation(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void	CDriverGL3::profileVBHardAllocation(std::vector<std::string> &result)
{
	result.clear();
	result.reserve(1000);
	result.push_back(toString("Memory Allocated: %4d Ko in AGP / %4d Ko in VRAM",
		getAvailableVertexAGPMemory()/1000, getAvailableVertexVRAMMemory()/1000));
	result.push_back(toString("Num VBHard: %d", _VertexBufferGLSet.Set.size()));

	uint	totalMemUsed= 0;
	set<IVertexBufferGL3*>::iterator	it;
	for (it= _VertexBufferGLSet.Set.begin(); it!=_VertexBufferGLSet.Set.end(); it++)
	{
		IVertexBufferGL3	*vbHard= *it;
		if (vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			totalMemUsed+= vSize*numVerts;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000));

	for (it= _VertexBufferGLSet.Set.begin(); it!=_VertexBufferGLSet.Set.end(); it++)
	{
		IVertexBufferGL3	*vbHard= *it;
		if (vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			result.push_back(toString("  %16s: %4d ko (format: %d / numVerts: %d)",
				vbHard->VB->getName().c_str(), vSize*numVerts/1000, vSize, numVerts));
		}
	}
}

// ***************************************************************************
// GL_NVX_gpu_memory_info constants (not in all glext.h versions)
#ifndef GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 0x9047
#endif

sint CDriverGL3::getTotalVideoMemory() const
{
	H_AUTO_OGL(CDriverGL3_getTotalVideoMemory);

	if (_Extensions.NVXGPUMemoryInfo)
	{
		GLint memoryInKiB = 0;
		glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &memoryInKiB);

		nlinfo("3D: GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX returned %d KiB", memoryInKiB);
		return memoryInKiB;
	}

	if (_Extensions.ATIMeminfo)
	{
		GLint params[4];
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, params);

		nlinfo("3D: GL_TEXTURE_FREE_MEMORY_ATI returned %d KiB", params[0]);
		return params[0];
	}

	return -1;
}

// ***************************************************************************
bool CDriverGL3::supportCloudRenderSinglePass() const
{
	H_AUTO_OGL(CDriverGL3_supportCloudRenderSinglePass)

	// Cloud shader (CCloud) is deprecated and will not be supported in the GL3 driver.
	// It was only used in the Snowballs demo and was removed from Ryzom.
	return false;
}

// ***************************************************************************
bool CDriverGL3::supportMADOperator() const
{
	H_AUTO_OGL(CDriverGL3_supportMADOperator)

	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportLargeUBOArrays() const
{
	return _MaxLightTableSize >= NL_OPENGL3_MAX_LIGHT_TABLE_CAPACITY;
}

// ***************************************************************************
uint CDriverGL3::getNumAdapter() const
{
	H_AUTO_OGL(CDriverGL3_getNumAdapter)

	return 1;
}

// ***************************************************************************
bool CDriverGL3::getAdapter(uint adapter, CAdapter &desc) const
{
	H_AUTO_OGL(CDriverGL3_getAdapter)

	if (adapter == 0)
	{
		desc.DeviceName = (const char *) glGetString (GL_RENDERER);
		desc.Driver = (const char *) glGetString (GL_VERSION);
		desc.Vendor= (const char *) glGetString (GL_VENDOR);

		desc.Description = "Default OpenGL adapter";
		desc.DeviceId = 0;
		desc.DriverVersion = 0;
		desc.Revision = 0;
		desc.SubSysId = 0;
		desc.VendorId = 0;
		return true;
	}
	return false;
}

// ***************************************************************************
bool CDriverGL3::setAdapter(uint adapter)
{
	H_AUTO_OGL(CDriverGL3_setAdapter)

	return adapter == 0;
}

// ***************************************************************************
CVertexBuffer::TVertexColorType CDriverGL3::getVertexColorFormat() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	return CVertexBuffer::TRGBA;
}

// ***************************************************************************
void CDriverGL3::startBench (bool wantStandardDeviation, bool quick, bool reset)
{
	CHTimer::startBench (wantStandardDeviation, quick, reset);
}

// ***************************************************************************
void CDriverGL3::endBench ()
{
	CHTimer::endBench ();
}

// ***************************************************************************
void CDriverGL3::displayBench (class NLMISC::CLog *log)
{
	// diplay
	CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	CHTimer::displayHierarchical(log, true, 48, 2);
	CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTimeWithoutSons);
}

#ifdef NL_DEBUG
void CDriverGL3::dumpMappedBuffers()
{
	// Vertex array ranges removed in GL3 driver
}
#endif

// ***************************************************************************
bool CDriverGL3::supportOcclusionQuery() const
{
	H_AUTO_OGL(CDriverGL3_supportOcclusionQuery)

	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportTextureRectangle() const
{
	H_AUTO_OGL(CDriverGL3_supportTextureRectangle);

	// This is deprecated in favour of NPOT, 
	// see supportNonPowerOfTwoTextures
	return false;
}

// ***************************************************************************
bool CDriverGL3::supportPackedDepthStencil() const
{
	H_AUTO_OGL(CDriverGL3_supportPackedDepthStencil);

	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportFrameBufferObject() const
{
	H_AUTO_OGL(CDriverGL3_supportFrameBufferObject);

	return _Extensions.GLCore;
}

// ***************************************************************************
IOcclusionQuery *CDriverGL3::createOcclusionQuery()
{
	H_AUTO_OGL(CDriverGL3_createOcclusionQuery)

	GLuint id;
	nglGenQueries(1, &id);
	if (id == 0) return NULL;
	COcclusionQueryGL3 *oqgl = new COcclusionQueryGL3;
	oqgl->Driver = this;
	oqgl->ID = id;
	oqgl->OcclusionType = IOcclusionQuery::NotAvailable;
	_OcclusionQueryList.push_front(oqgl);
	oqgl->Iterator = _OcclusionQueryList.begin();
	oqgl->VisibleCount = 0;
	return oqgl;

}

// ***************************************************************************
void CDriverGL3::deleteOcclusionQuery(IOcclusionQuery *oq)
{
	H_AUTO_OGL(CDriverGL3_deleteOcclusionQuery);

	if (!oq) return;
	COcclusionQueryGL3 *oqgl = NLMISC::safe_cast<COcclusionQueryGL3 *>(oq);
	nlassert((CDriverGL3 *) oqgl->Driver == this); // should come from the same driver
	oqgl->Driver = NULL;
	nlassert(oqgl->ID != 0);
	GLuint id = oqgl->ID;
	nglDeleteQueries(1, &id);
	_OcclusionQueryList.erase(oqgl->Iterator);
	if (oqgl == _CurrentOcclusionQuery)
	{
		_CurrentOcclusionQuery = NULL;
	}
	delete oqgl;

}

// ***************************************************************************
void COcclusionQueryGL3::begin()
{
	H_AUTO_OGL(COcclusionQueryGL3_begin);

	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == NULL); // only one query at a time
	nlassert(ID);
	nglBeginQuery(GL_SAMPLES_PASSED, ID);
	Driver->_CurrentOcclusionQuery = this;
	OcclusionType = NotAvailable;
	VisibleCount = 0;

}

// ***************************************************************************
void COcclusionQueryGL3::end()
{
	H_AUTO_OGL(COcclusionQueryGL3_end);

	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == this); // only one query at a time
	nlassert(ID);
	nglEndQuery(GL_SAMPLES_PASSED);
	Driver->_CurrentOcclusionQuery = NULL;

}

// ***************************************************************************
IOcclusionQuery::TOcclusionType COcclusionQueryGL3::getOcclusionType()
{
	H_AUTO_OGL(COcclusionQueryGL3_getOcclusionType);

	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!

	if (OcclusionType == NotAvailable)
	{
		GLuint result;
		nglGetQueryObjectuiv(ID, GL_QUERY_RESULT_AVAILABLE, &result);
		if (result != GL_FALSE)
		{
			nglGetQueryObjectuiv(ID, GL_QUERY_RESULT, &result);
			OcclusionType = result != 0 ? NotOccluded : Occluded;
			VisibleCount = (uint)result;
		}
	}

	return OcclusionType;
}

// ***************************************************************************
uint COcclusionQueryGL3::getVisibleCount()
{
	H_AUTO_OGL(COcclusionQueryGL3_getVisibleCount)
	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (getOcclusionType() == NotAvailable) return 0;
	return VisibleCount;
}

// ***************************************************************************
void CDriverGL3::getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const
{
	lightedMaterial = IDRV_MAT_MAXTEXTURES;
	unlightedMaterial = IDRV_MAT_MAXTEXTURES;
}

// ***************************************************************************
void CDriverGL3::beginDialogMode()
{
}

// ***************************************************************************
void CDriverGL3::endDialogMode()
{
}

// ***************************************************************************
void displayGLError(GLenum error)
{
	switch(error)
	{
	case GL_NO_ERROR: nlwarning("GL_NO_ERROR"); break;
	case GL_INVALID_ENUM: nlwarning("GL_INVALID_ENUM"); break;
	case GL_INVALID_VALUE: nlwarning("GL_INVALID_VALUE"); break;
	case GL_INVALID_OPERATION: nlwarning("GL_INVALID_OPERATION"); break;
	case GL_STACK_OVERFLOW: nlwarning("GL_STACK_OVERFLOW"); break;
	case GL_STACK_UNDERFLOW: nlwarning("GL_STACK_UNDERFLOW"); break;
	case GL_OUT_OF_MEMORY: nlwarning("GL_OUT_OF_MEMORY"); break;
	default:
		nlwarning("GL_ERROR");
		break;
	}
}

} // NLDRIVERGL3

} // NL3D
