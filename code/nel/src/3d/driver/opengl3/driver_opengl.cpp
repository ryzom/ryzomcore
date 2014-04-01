// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdopengl.h"
#include "driver_opengl.h"
#include "driver_opengl_extension.h"

// by default, we disable the windows menu keys (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU

#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_loader.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/dynloadlib.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "driver_glsl_shader_generator.h"


using namespace std;
using namespace NLMISC;





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

IDriver* createGl3DriverInstance ()
{
	return new NLDRIVERGL::CDriverGL3;
}

#else

#ifdef NL_OS_WINDOWS

__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverGL3;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

#elif defined (NL_OS_UNIX)

extern "C"
{
	IDriver* NL3D_createIDriverInstance ()
	{
		return new CDriverGL3;
	}

	uint32 NL3D_interfaceVersion ()
	{
		return IDriver::InterfaceVersion;
	}
}

#endif // NL_OS_WINDOWS

#endif // NL_STATIC

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

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

#elif defined (NL_OS_UNIX)

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

	_AlphaBlendedCursorSupported = false;
	_AlphaBlendedCursorSupportRetrieved = false;
	_CurrCol = CRGBA::White;
	_CurrRot = 0;
	_CurrHotSpotX = 0;
	_CurrHotSpotY = 0;
	_CursorScale = 1.f;
	_MouseCaptured = false;

	_NeedToRestaureGammaRamp = false;

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
	_CurrentFogColor[0]= 0;
	_CurrentFogColor[1]= 0;
	_CurrentFogColor[2]= 0;
	_CurrentFogColor[3]= 0;

	_RenderTargetFBO = false;

	uint i;

	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;
	_SlowUnlockVBHard= false;

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
#ifdef GL_NONE
		_CurrentTexAddrMode[i] = GL_NONE;
#else
		_CurrentTexAddrMode[i] = 0;
#endif
	}

	for (i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
		_UserTexMat[ i ].identity();

	_UserTexMatEnabled = 0;

	// reserve enough space to never reallocate, nor test for reallocation.
	_LightMapLUT.resize(NL3D_DRV_MAX_LIGHTMAP);
	// must set replace for alpha part.
	_LightMapLastStageEnv.Env.OpAlpha= CMaterial::Replace;
	_LightMapLastStageEnv.Env.SrcArg0Alpha= CMaterial::Texture;
	_LightMapLastStageEnv.Env.OpArg0Alpha= CMaterial::SrcAlpha;

///	buildCausticCubeMapTex();

	_SpecularBatchOn= false;

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

	_LightMapDynamicLightEnabled = false;
	_LightMapDynamicLightDirty= false;

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

	shaderGenerator = new CGLSLShaderGenerator();
	usrShaderManager = new CUsrShaderManager();

	CUsrShaderLoader loader;
	loader.setManager(usrShaderManager);
	loader.loadShaders("./shaders");
}

// ***************************************************************************
CDriverGL3::~CDriverGL3()
{
	H_AUTO_OGL(CDriverGL3_CDriverGLDtor)

	release();

	delete shaderGenerator;
	shaderGenerator = NULL;
	delete usrShaderManager;
	usrShaderManager = NULL;

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
#elif defined(NL_OS_UNIX)
	registerGlXExtensions(_Extensions, _dpy, DefaultScreen(_dpy));
#endif // NL_OS_WINDOWS

	// Check required extensions!!
	if (!_Extensions.GLCore)
	{
		nlwarning("Missing required GL 3.30 Core features. Update your driver");
		throw EBadDisplay("Missing required GL 3.30 Core features. Update your driver");
	}
	if (!_Extensions.ARBSeparateShaderObjects)
	{
		nlwarning("Missing required GL extension: GL_ARB_separate_shader_objects. Update your driver");
		throw EBadDisplay("Missing required GL extension: GL_ARB_separate_shader_objects. Update your driver");
	}

	// All User Light are disabled by Default
	for (uint i = 0; i < MaxLight; ++i)
	{
		_UserLightEnable[i] = false;
		touchLightVP(i);
	}

	// init _DriverGLStates
	_DriverGLStates.init();

	// Init OpenGL/Driver defaults.
	//=============================
	glViewport(0,0,_CurrentMode.Width,_CurrentMode.Height);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_SUM_EXT);

	_CurrViewport.init(0.f, 0.f, 1.f, 1.f);
	_CurrScissor.initFullScreen();
	_CurrentGlNormalize = false;
	_ForceNormalize = false;
	// Setup defaults for blend, lighting ...
	_DriverGLStates.forceDefaults(inlGetNumTextStages());
	// Default delta camera pos.
	_PZBCameraPos = CVector::Null;

	// Init VertexArrayRange according to supported extenstion.
	_SlowUnlockVBHard = false;

	_AGPVertexArrayRange = new CVertexArrayRange(this);
	_VRAMVertexArrayRange = new CVertexArrayRange(this);

	// Reset VertexArrayRange.
	_CurrentVertexArrayRange = NULL;
	_CurrentVertexBufferHard = NULL;
	_NVCurrentVARPtr = NULL;
	_NVCurrentVARSize = 0;

	initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

	// Init embm if present
	//===========================================================
	initEMBM();

	// Activate the default texture environnments for all stages.
	//===========================================================
	for (uint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage] = NULL;
		_CurrentTextureInfoGL[stage] = NULL;
		// texture are disabled in DriverGLStates.forceDefaults().

		// init default env.
		CMaterial::CTexEnv env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);

		// Not special TexEnv.
		_CurrentTexEnvSpecial[stage] = TexEnvSpecialDisabled;

		// set All TexGen by default to identity matrix (prefer use the textureMatrix scheme)
		setTexGenModeVP(stage, TexGenDisabled); // FIXME GL3 TEXGEN
	}

	if (!initProgramPipeline())
		nlerror("Failed to create Pipeline Object");

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
bool CDriverGL3::stretchRect(ITexture * /* srcText */, NLMISC::CRect &/* srcRect */, ITexture * /* destText */, NLMISC::CRect &/* destRect */)
{
	H_AUTO_OGL(CDriverGL3_stretchRect)

	return false;
}

// ***************************************************************************
bool CDriverGL3::supportBloomEffect() const
{
	return false; // FIXME GL3 // _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportNonPowerOfTwoTextures() const
{
	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::isTextureRectangle(ITexture * tex) const
{
	return (supportTextureRectangle() && tex->isBloomTexture() && tex->mipMapOff()
			&& (!isPowerOf2(tex->getWidth()) || !isPowerOf2(tex->getHeight())));
}

// ***************************************************************************
bool CDriverGL3::activeFrameBufferObject(ITexture * tex)
{
	return false; // TODO GL3 FBO
#if 0
	if (supportFrameBufferObject()/* && supportPackedDepthStencil()*/)
	{
		if (tex)
		{
			CTextureDrvInfosGL3*	gltext = (CTextureDrvInfosGL3*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
			return gltext->activeFrameBufferObject(tex);
		}
		else
		{
			nglBindFramebuffer(GL_FRAMEBUFFER, 0);
			return true;
		}
	}

	return false;
#endif
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
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearZBuffer(float zval)
{
	H_AUTO_OGL(CDriverGL3_clearZBuffer);

	glClearDepth(zval);

	_DriverGLStates.enableZWrite(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearStencilBuffer(float stencilval)
{
	H_AUTO_OGL(CDriverGL3_clearStencilBuffer)
	glClearStencil((int)stencilval);

	glClear(GL_STENCIL_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
void CDriverGL3::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	H_AUTO_OGL(CDriverGL3_setColorMask)
	glColorMask (bRed, bGreen, bBlue, bAlpha);
}

// --------------------------------------------------
bool CDriverGL3::swapBuffers()
{
	H_AUTO_OGL(CDriverGL3_swapBuffers)

	++ _SwapBufferCounter;

#ifdef NL_OS_WINDOWS
	if (_EventEmitter.getNumEmitters() > 1) // is direct input running ?
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}
#endif

	if (!_WndActive)
	{
		if (_AGPVertexArrayRange) _AGPVertexArrayRange->updateLostBuffers();
		if (_VRAMVertexArrayRange) _VRAMVertexArrayRange->updateLostBuffers();
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

	glXSwapBuffers(_dpy, _win);

#endif // NL_OS_WINDOWS

	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o)).
	for (uint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// texture are disabled in DriverGLStates.forceDefaults().

		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		forceActivateTexEnvMode(stage, env);
		forceActivateTexEnvColor(stage, env);
	}

	// Activate the default material.
	//===========================================================
	// Same reasoning as textures :)
	_DriverGLStates.forceDefaults(inlGetNumTextStages());

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
	// on ati, if the window is inactive, check all vertex buffer to see which one are lost
	if (_AGPVertexArrayRange) _AGPVertexArrayRange->updateLostBuffers();
	if (_VRAMVertexArrayRange) _VRAMVertexArrayRange->updateLostBuffers();
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
	for (std::set<CVPBuiltin>::iterator it(m_VPBuiltinCache.begin()), end(m_VPBuiltinCache.end()); it != end; ++it)
		delete it->VertexProgram;
	m_VPBuiltinCache.clear();
	for (std::set<CPPBuiltin>::iterator it(m_PPBuiltinCache.begin()), end(m_PPBuiltinCache.end()); it != end; ++it)
		delete it->PixelProgram;
	m_PPBuiltinCache.clear();

	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	_SwapBufferCounter = 0;

	// delete querries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	// release caustic cube map
//	_CauticCubeMap = NULL;

	// Reset VertexArrayRange.
	resetVertexArrayRange();

	// delete containers
	delete _AGPVertexArrayRange;
	delete _VRAMVertexArrayRange;
	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;

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
	getWindowSize(clientWidth, clientHeight);

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
	glViewport (ix, iy, iwidth, iheight);
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
	getWindowSize(clientWidth, clientHeight);

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
		glDisable(GL_SCISSOR_TEST);
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

		glScissor (ix0, iy0, iwidth, iheight);
		glEnable(GL_SCISSOR_TEST);
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

		glPixelTransferf(GL_DEPTH_SCALE, 1.0f) ;
		glPixelTransferf(GL_DEPTH_BIAS, 0.f) ;
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
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	if (rect.Width!=bitmap.getWidth() || rect.Height!=bitmap.getHeight() || bitmap.getPixelFormat()!=CBitmap::RGBA)
		return false;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glDrawPixels (rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap.getPixels()[0]));

	return true;
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
		glBindTexture(GL_TEXTURE_CUBE_MAP, gltext->ID);
		glCopyTexSubImage2D(NLCubeFaceToGLCubeFace[cubeFace], level, offsetx, offsety, x, y, width, height);
	}
	else
	{
		glBindTexture(gltext->TextureMode, gltext->ID);
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
void CDriverGL3::setPolygonMode (TPolygonMode mode)
{
	H_AUTO_OGL(CDriverGL3_setPolygonMode)
	IDriver::setPolygonMode (mode);

	// Set the polygon mode
	switch (_PolygonMode)
	{
	case Filled:
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		break;
	case Line:
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		break;
	case Point:
		glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
		break;
	}
}

// ***************************************************************************
bool CDriverGL3::fogEnabled()
{
	H_AUTO_OGL(CDriverGL3_fogEnabled)
	return _FogEnabled;
}

// ***************************************************************************
void CDriverGL3::enableFog(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableFog)
	_FogEnabled = enable;
	enableFogVP(enable);
}

// ***************************************************************************
void CDriverGL3::setupFog(float start, float end, CRGBA color)
{
	H_AUTO_OGL(CDriverGL3_setupFog)

	_CurrentFogColor[0]= color.R/255.0f;
	_CurrentFogColor[1]= color.G/255.0f;
	_CurrentFogColor[2]= color.B/255.0f;
	_CurrentFogColor[3]= color.A/255.0f;

	_FogStart = start;
	_FogEnd = end;
}

// ***************************************************************************
float CDriverGL3::getFogStart() const
{
	H_AUTO_OGL(CDriverGL3_getFogStart)
	return _FogStart;
}

// ***************************************************************************
float CDriverGL3::getFogEnd() const
{
	H_AUTO_OGL(CDriverGL3_getFogEnd)
	return _FogEnd;
}

// ***************************************************************************
CRGBA CDriverGL3::getFogColor() const
{
	H_AUTO_OGL(CDriverGL3_getFogColor)
	CRGBA	ret;
	ret.R= (uint8)(_CurrentFogColor[0]*255);
	ret.G= (uint8)(_CurrentFogColor[1]*255);
	ret.B= (uint8)(_CurrentFogColor[2]*255);
	ret.A= (uint8)(_CurrentFogColor[3]*255);
	return ret;
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
		nlinfo ("3D: PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverGL");
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
	nlassert(stage < inlGetNumTextStages());
	_DriverGLStates.activeTexture(stage);

	//glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat);
}

// ***************************************************************************
bool CDriverGL3::supportPerPixelLighting(bool specular) const
{
	H_AUTO_OGL(CDriverGL3_supportPerPixelLighting)

	return false; // FIXME GL3 // _Extensions.GLCore;
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
bool CDriverGL3::supportBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_supportBlendConstantColor)
	return _Extensions.GLCore;
}

// ***************************************************************************
void CDriverGL3::setBlendConstantColor(NLMISC::CRGBA col)
{
	H_AUTO_OGL(CDriverGL3_setBlendConstantColor)

	// bkup
	_CurrentBlendConstantColor = col;

	static const float OO255 = 1.0f / 255.0f;
	nglBlendColor(col.R * OO255, col.G * OO255, col.B * OO255, col.A * OO255);
}

// ***************************************************************************
NLMISC::CRGBA CDriverGL3::getBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	return	_CurrentBlendConstantColor;
}

// ***************************************************************************
uint			CDriverGL3::getNbTextureStages() const
{
	H_AUTO_OGL(CDriverGL3_getNbTextureStages)
	return inlGetNumTextStages();
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
}

// ***************************************************************************
void CDriverGL3::initEMBM()
{
	H_AUTO_OGL(CDriverGL3_initEMBM);

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
		res = nwglSwapIntervalEXT(_Interval) == TRUE;
	}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
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
#elif defined(NL_OS_UNIX)
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
void	CDriverGL3::enablePolygonSmoothing(bool smooth)
{
	H_AUTO_OGL(CDriverGL3_enablePolygonSmoothing);

	if (smooth)
		glEnable(GL_POLYGON_SMOOTH);
	else
		glDisable(GL_POLYGON_SMOOTH);

	_PolygonSmooth= smooth;
}

// ***************************************************************************
bool	CDriverGL3::isPolygonSmoothingEnabled() const
{
	H_AUTO_OGL(CDriverGL3_isPolygonSmoothingEnabled)

	return _PolygonSmooth;
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
	contReset(_VBHardProfiles);
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
	result.push_back(toString("Num VBHard: %d", _VertexBufferHardSet.Set.size()));

	uint	totalMemUsed= 0;
	set<IVertexBufferHardGL*>::iterator	it;
	for (it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
		if (vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			totalMemUsed+= vSize*numVerts;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000));

	for (it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
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
bool CDriverGL3::supportCloudRenderSinglePass() const
{
	H_AUTO_OGL(CDriverGL3_supportCloudRenderSinglePass)

	return _Extensions.GLCore;
}

// ***************************************************************************
bool CDriverGL3::supportMADOperator() const
{
	H_AUTO_OGL(CDriverGL3_supportMADOperator)

	return _Extensions.GLCore;
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
	_AGPVertexArrayRange->dumpMappedBuffers();
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

	return false; // TODO GL _Extensions.GLCore;
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
	nglBeginQuery(GL_SAMPLES_PASSED, ID); // FIXME or GL_ANY_SAMPLES_PASSED
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
		nglGetQueryObjectuiv(ID, GL_QUERY_RESULT, &result);
		OcclusionType = result != 0 ? NotOccluded : Occluded;
		VisibleCount = (uint) result;
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
void CDriverGL3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGL3_setDepthRange)
	_DriverGLStates.setDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL3::getDepthRange(float &znear, float &zfar) const
{
	H_AUTO_OGL(CDriverGL3_getDepthRange)
	_DriverGLStates.getDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGL3_setCullMode)
	_DriverGLStates.setCullMode((CDriverGLStates3::TCullMode) cullMode);
}

// ***************************************************************************
CDriverGL3::TCullMode CDriverGL3::getCullMode() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return (CDriverGL3::TCullMode) _DriverGLStates.getCullMode();
}

// ***************************************************************************
void CDriverGL3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	_DriverGLStates.enableStencilTest(enable);
}

// ***************************************************************************
bool CDriverGL3::isStencilTestEnabled() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return _DriverGLStates.isStencilTestEnabled();
}

// ***************************************************************************
void CDriverGL3::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glstencilFunc = 0;

	switch(stencilFunc)
	{
		case IDriver::never:		glstencilFunc=GL_NEVER; break;
		case IDriver::less:			glstencilFunc=GL_LESS; break;
		case IDriver::lessequal:	glstencilFunc=GL_LEQUAL; break;
		case IDriver::equal:		glstencilFunc=GL_EQUAL; break;
		case IDriver::notequal:		glstencilFunc=GL_NOTEQUAL; break;
		case IDriver::greaterequal:	glstencilFunc=GL_GEQUAL; break;
		case IDriver::greater:		glstencilFunc=GL_GREATER; break;
		case IDriver::always:		glstencilFunc=GL_ALWAYS; break;
		default: nlstop;
	}

	_DriverGLStates.stencilFunc(glstencilFunc, (GLint)ref, (GLuint)mask);
}

// ***************************************************************************
void CDriverGL3::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glFail = 0, glZFail = 0, glZPass = 0;

	switch(fail)
	{
		case IDriver::keep:		glFail=GL_KEEP; break;
		case IDriver::zero:		glFail=GL_ZERO; break;
		case IDriver::replace:	glFail=GL_REPLACE; break;
		case IDriver::incr:		glFail=GL_INCR; break;
		case IDriver::decr:		glFail=GL_DECR; break;
		case IDriver::invert:	glFail=GL_INVERT; break;
		default: nlstop;
	}

	switch(zfail)
	{
		case IDriver::keep:		glZFail=GL_KEEP; break;
		case IDriver::zero:		glZFail=GL_ZERO; break;
		case IDriver::replace:	glZFail=GL_REPLACE; break;
		case IDriver::incr:		glZFail=GL_INCR; break;
		case IDriver::decr:		glZFail=GL_DECR; break;
		case IDriver::invert:	glZFail=GL_INVERT; break;
		default: nlstop;
	}

	switch(zpass)
	{
		case IDriver::keep:		glZPass=GL_KEEP; break;
		case IDriver::zero:		glZPass=GL_ZERO; break;
		case IDriver::replace:	glZPass=GL_REPLACE; break;
		case IDriver::incr:		glZPass=GL_INCR; break;
		case IDriver::decr:		glZPass=GL_DECR; break;
		case IDriver::invert:	glZPass=GL_INVERT; break;
		default: nlstop;
	}

	_DriverGLStates.stencilOp(glFail, glZFail, glZPass);
}

// ***************************************************************************
void CDriverGL3::stencilMask(uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	_DriverGLStates.stencilMask((GLuint)mask);
}

// ***************************************************************************
void CDriverGL3::getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const
{
	lightedMaterial = inlGetNumTextStages();
	unlightedMaterial = inlGetNumTextStages();
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
void CDriverGL3::reloadUserShaders()
{
	usrShaderManager->clear();
	NL3D::CUsrShaderLoader loader;
	loader.setManager(usrShaderManager);
	loader.loadShaders("./shaders");
}

CProgramDrvInfosGL3::CProgramDrvInfosGL3(CDriverGL3 *drv, ItGPUPrgDrvInfoPtrList it) :
IProgramDrvInfos(drv, it)
{
	programId = 0;
}

CProgramDrvInfosGL3::~CProgramDrvInfosGL3()
{
	programId = 0;
}

uint CProgramDrvInfosGL3::getUniformIndex(const char *name) const
{
	int idx = nglGetUniformLocation(programId, name);
	if (idx == -1)
		return ~0;
	else
		return idx;
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

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D
