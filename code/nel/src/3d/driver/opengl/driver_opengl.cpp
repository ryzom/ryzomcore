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
#include "nel/misc/rect.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/dynloadlib.h"
#include "driver_opengl_vertex_buffer_hard.h"


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

#ifdef USE_OPENGLES

class CDriverGLEsNelLibrary : public INelLibrary {
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CDriverGLEsNelLibrary)

#else

class CDriverGLNelLibrary : public INelLibrary {
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CDriverGLNelLibrary)

#endif

#endif /* #ifndef NL_STATIC */

namespace NL3D {

#ifdef NL_STATIC

#ifdef USE_OPENGLES

IDriver* createGlEsDriverInstance ()
{
	return new NLDRIVERGLES::CDriverGL;
}

#else

IDriver* createGlDriverInstance ()
{
	return new NLDRIVERGL::CDriverGL;
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
	return new CDriverGL;
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
		return new CDriverGL;
	}

	uint32 NL3D_interfaceVersion ()
	{
		return IDriver::InterfaceVersion;
	}
}

#endif // NL_OS_WINDOWS

#endif // NL_STATIC

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

CMaterial::CTexEnv CDriverGL::_TexEnvReplace;


#ifdef NL_OS_WINDOWS
uint CDriverGL::_Registered=0;
#endif // NL_OS_WINDOWS

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32 CDriverGL::ReleaseVersion = 0x11;

// Number of register to allocate for the EXTVertexShader extension
const uint CDriverGL::_EVSNumConstant = 97;

GLenum CDriverGL::NLCubeFaceToGLCubeFace[6] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
};

// ***************************************************************************
CDriverGL::CDriverGL()
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

#ifdef USE_OPENGLES

	_EglDisplay = 0;
	_EglContext = 0;
	_EglSurface = 0;

#elif defined(NL_OS_WINDOWS)

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

	_LightSetupDirty= false;
	_ModelViewMatrixDirty= false;
	_RenderSetupDirty= false;
	// All lights default pos.
	uint i;
	for(i=0;i<MaxLight;i++)
		_LightDirty[i]= false;

	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;
	_SupportVBHard= false;
	_SlowUnlockVBHard= false;
	_MaxVerticesByVBHard= 0;

	_AllocatedTextureMemory= 0;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;
	_ForceNativeFragmentPrograms = true;

	_SumTextureMemoryUsed = false;

	_NVTextureShaderEnabled = false;

	_AnisotropicFilter = 0.f;

	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
#ifdef GL_NONE
		_CurrentTexAddrMode[i] = GL_NONE;
#else
		_CurrentTexAddrMode[i] = 0;
#endif
	}

	_UserTexMatEnabled = 0;

	// Ligtmap preca.
	_LastVertexSetupIsLightMap= false;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
		_LightMapUVMap[i]= -1;
	// reserve enough space to never reallocate, nor test for reallocation.
	_LightMapLUT.resize(NL3D_DRV_MAX_LIGHTMAP);
	// must set replace for alpha part.
	_LightMapLastStageEnv.Env.OpAlpha= CMaterial::Replace;
	_LightMapLastStageEnv.Env.SrcArg0Alpha= CMaterial::Texture;
	_LightMapLastStageEnv.Env.OpArg0Alpha= CMaterial::SrcAlpha;

	_ProjMatDirty = true;

	std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);

	ATIWaterShaderHandleNoDiffuseMap = 0;
	ATIWaterShaderHandle = 0;
	ATICloudShaderHandle = 0;

	_ATIDriverVersion = 0;
	_ATIFogRangeFixed = true;

	std::fill(ARBWaterShader, ARBWaterShader + 4, 0);

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
}

// ***************************************************************************
CDriverGL::~CDriverGL()
{
	H_AUTO_OGL(CDriverGL_CDriverGLDtor)
	release();

#if defined(NL_OS_MAC)
	[_autoreleasePool release];
#endif
}

// --------------------------------------------------
bool CDriverGL::setupDisplay()
{
	H_AUTO_OGL(CDriverGL_setupDisplay)

	// Driver caps.
	//=============
	// Retrieve the extensions for the current context.
	registerGlExtensions (_Extensions);
	vector<string> lines;
	explode(_Extensions.toString(), string("\n"), lines);
	for(uint i = 0; i < lines.size(); i++)
		nlinfo("3D: %s", lines[i].c_str());

#ifdef USE_OPENGLES
	registerEGlExtensions(_Extensions, _EglDisplay);
#elif defined(NL_OS_WINDOWS)
	registerWGlExtensions(_Extensions, _hDC);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	registerGlXExtensions(_Extensions, _dpy, DefaultScreen(_dpy));
#endif // NL_OS_WINDOWS

	// Check required extensions!!
	// ARBMultiTexture is a OpenGL 1.2 required extension.
	if(!_Extensions.ARBMultiTexture)
	{
		nlwarning("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
		throw EBadDisplay("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
	}

	if(!_Extensions.EXTTextureEnvCombine)
	{
		nlwarning("Missing Important GL extension: GL_EXT_texture_env_combine => All envcombine are setup to GL_MODULATE!!!");
	}

	// Get num of light for this driver
	int numLight;
	glGetIntegerv (GL_MAX_LIGHTS, &numLight);
	_MaxDriverLight=(uint)numLight;
	if (_MaxDriverLight>MaxLight)
		_MaxDriverLight=MaxLight;

	// All User Light are disabled by Default
	uint i;
	for(i=0;i<MaxLight;i++)
		_UserLightEnable[i]= false;

	// init _DriverGLStates
	_DriverGLStates.init(_Extensions.ARBTextureCubeMap, (_Extensions.NVTextureRectangle || _Extensions.EXTTextureRectangle || _Extensions.ARBTextureRectangle), _MaxDriverLight);

	// Init OpenGL/Driver defaults.
	//=============================
	glViewport(0,0,_CurrentMode.Width,_CurrentMode.Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,_CurrentMode.Width,_CurrentMode.Height,0,-1.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#ifndef USE_OPENGLES
	glDisable(GL_AUTO_NORMAL);
#endif
	glDisable(GL_COLOR_MATERIAL);
#ifndef USE_OPENGLES
	glEnable(GL_DITHER);
#endif
	glDisable(GL_FOG);
	glDisable(GL_LINE_SMOOTH);
#ifndef USE_OPENGLES
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
#endif
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
#ifndef USE_OPENGLES
	glDisable(GL_COLOR_SUM_EXT);
#endif

	_CurrViewport.init(0.f, 0.f, 1.f, 1.f);
	_CurrScissor.initFullScreen();
	_CurrentGlNormalize= false;
	_ForceNormalize= false;
	// Setup defaults for blend, lighting ...
	_DriverGLStates.forceDefaults(inlGetNumTextStages());
	// Default delta camera pos.
	_PZBCameraPos= CVector::Null;

	if (_NVTextureShaderEnabled)
	{
		enableNVTextureShader(false);
	}

	// Be always in EXTSeparateSpecularColor.
	if(_Extensions.EXTSeparateSpecularColor)
	{
#ifndef USE_OPENGLES
		glLightModeli((GLenum)GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
#endif
	}

	if (_Extensions.ARBFragmentShader)
	{
		_ForceNativeFragmentPrograms = false;
	}

	_VertexProgramEnabled= false;
	_PixelProgramEnabled= false;
	_LastSetupGLArrayVertexProgram= false;

	// Init VertexArrayRange according to supported extenstion.
	_SupportVBHard= false;
	_SlowUnlockVBHard= false;
	_MaxVerticesByVBHard= 0;

	// Try with ARB ext first.
	if (_Extensions.ARBVertexBufferObject)
	{
		_AGPVertexArrayRange= new CVertexArrayRangeARB(this);
		_VRAMVertexArrayRange= new CVertexArrayRangeARB(this);
		_SupportVBHard= true;
		_MaxVerticesByVBHard = std::numeric_limits<uint32>::max(); // cant' know the value..
	}
#ifndef USE_OPENGLES
	// Next with NVidia ext
	else if(_Extensions.NVVertexArrayRange)
	{
		_AGPVertexArrayRange= new CVertexArrayRangeNVidia(this);
		_VRAMVertexArrayRange= new CVertexArrayRangeNVidia(this);
		_SupportVBHard= true;
		_MaxVerticesByVBHard= _Extensions.NVVertexArrayRangeMaxVertex;
	}
	else if(_Extensions.ATITextureEnvCombine3 && _Extensions.ATIVertexArrayObject)
	{
		// NB
		// on Radeon 9200 and below : ATI_vertex_array_object is better (no direct access to AGP with ARB_vertex_buffer_object -> slow unlock)
		// on Radeon 9500 and above : ARB_vertex_buffer_object is better
		if (!_Extensions.ATIMapObjectBuffer)
		{
			_AGPVertexArrayRange= new CVertexArrayRangeATI(this);
			_VRAMVertexArrayRange= new CVertexArrayRangeATI(this);
			// BAD ATI extension scheme.
			_SlowUnlockVBHard= true;
		}
		else
		{
			_AGPVertexArrayRange= new CVertexArrayRangeMapObjectATI(this);
			_VRAMVertexArrayRange= new CVertexArrayRangeMapObjectATI(this);
		}
		_SupportVBHard= true;
		// _MaxVerticesByVBHard= 65535; // should always work with recent drivers.
		// tmp fix for ati
		_MaxVerticesByVBHard= 16777216;
	}
#endif

	// Reset VertexArrayRange.
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;

	if (_SupportVBHard)
	{
		// try to allocate 16Mo by default of AGP Ram.
		initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

		// If not success to allocate at least a minimum space in AGP, then disable completely VBHard feature
		if( _AGPVertexArrayRange->sizeAllocated()==0 )
		{
			// reset any allocated VRAM space.
			resetVertexArrayRange();

			// delete containers
			delete _AGPVertexArrayRange;
			delete _VRAMVertexArrayRange;
			_AGPVertexArrayRange= NULL;
			_VRAMVertexArrayRange= NULL;

			// disable.
			_SupportVBHard= false;
			_SlowUnlockVBHard= false;
			_MaxVerticesByVBHard= 0;
		}
	}

	// Init embm if present
	//===========================================================
	initEMBM();

	// Init fragment shaders if present
	//===========================================================
	initFragmentShaders();

	// Activate the default texture environnments for all stages.
	//===========================================================
	for(uint stage=0;stage<inlGetNumTextStages(); stage++)
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

		// Not special TexEnv.
		_CurrentTexEnvSpecial[stage]= TexEnvSpecialDisabled;

		// set All TexGen by default to identity matrix (prefer use the textureMatrix scheme)
		_DriverGLStates.activeTextureARB(stage);
#ifndef USE_OPENGLES
		GLfloat		params[4];
		params[0]=1; params[1]=0; params[2]=0; params[3]=0;
		glTexGenfv(GL_S, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_S, GL_EYE_PLANE, params);
		params[0]=0; params[1]=1; params[2]=0; params[3]=0;
		glTexGenfv(GL_T, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_T, GL_EYE_PLANE, params);
		params[0]=0; params[1]=0; params[2]=1; params[3]=0;
		glTexGenfv(GL_R, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_R, GL_EYE_PLANE, params);
		params[0]=0; params[1]=0; params[2]=0; params[3]=1;
		glTexGenfv(GL_Q, GL_OBJECT_PLANE, params);
		glTexGenfv(GL_Q, GL_EYE_PLANE, params);
#endif
	}

	resetTextureShaders();

	_PPLExponent = 1.f;
	_PPLightDiffuseColor = NLMISC::CRGBA::White;
	_PPLightSpecularColor = NLMISC::CRGBA::White;

	// Backward compatibility: default lighting is Light0 default openGL
	// meaning that light direction is always (0,1,0) in eye-space
	// use enableLighting(0....), to get normal behaviour
	_DriverGLStates.enableLight(0, true);
	_LightMode[0] = CLight::DirectionalLight;
	_WorldLightDirection[0] = CVector::Null;

	_Initialized = true;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;

	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// check whether per pixel lighting shader is supported
	checkForPerPixelLightingSupport();

#ifndef USE_OPENGLES
	// if EXTVertexShader is used, bind  the standard GL arrays, and allocate constant
	if (!_Extensions.NVVertexProgram && !_Extensions.ARBVertexProgram && _Extensions.EXTVertexShader)
	{
		_EVSPositionHandle	= nglBindParameterEXT(GL_CURRENT_VERTEX_EXT);
		_EVSNormalHandle	= nglBindParameterEXT(GL_CURRENT_NORMAL);
		_EVSColorHandle		= nglBindParameterEXT(GL_CURRENT_COLOR);

		if (!_EVSPositionHandle || !_EVSNormalHandle || !_EVSColorHandle)
		{
			nlwarning("Unable to bind input parameters for use with EXT_vertex_shader, vertex program support is disabled");
			_Extensions.EXTVertexShader = false;
		}
		else
		{
			// bind texture units
			for(uint k = 0; k < 8; ++k)
			{
				_EVSTexHandle[k] = nglBindTextureUnitParameterEXT(GL_TEXTURE0_ARB + k, GL_CURRENT_TEXTURE_COORDS);
			}
			// Other attributes are managed using variant pointers :
			// Secondary color
			// Fog Coords
			// Skin Weight
			// Skin palette
			// This mean that they must have 4 components

			// Allocate invariants. One assitionnal variant is needed for fog coordinate if fog bug is not fixed in driver version
			_EVSConstantHandle = nglGenSymbolsEXT(GL_VECTOR_EXT, GL_INVARIANT_EXT, GL_FULL_RANGE_EXT, _EVSNumConstant + (_ATIFogRangeFixed ? 0 : 1));

			if (_EVSConstantHandle == 0)
			{
				nlwarning("Unable to allocate constants for EXT_vertex_shader, vertex program support is disabled");
				_Extensions.EXTVertexShader = false;
			}
		}
	}
#endif

	// Reset the vbl interval
	setSwapVBLInterval(_Interval);

	return true;
}

// ***************************************************************************
bool CDriverGL::stretchRect(ITexture * /* srcText */, NLMISC::CRect &/* srcRect */, ITexture * /* destText */, NLMISC::CRect &/* destRect */)
{
	H_AUTO_OGL(CDriverGL_stretchRect)

	return false;
}

// ***************************************************************************
bool CDriverGL::supportBloomEffect() const
{
	return (supportVertexProgram(CVertexProgram::nelvp) && supportFrameBufferObject() && supportPackedDepthStencil() && supportTextureRectangle());
}

// ***************************************************************************
bool CDriverGL::supportNonPowerOfTwoTextures() const
{
	return _Extensions.ARBTextureNonPowerOfTwo;
}

// ***************************************************************************
bool CDriverGL::isTextureRectangle(ITexture * tex) const
{
	return (!supportNonPowerOfTwoTextures() && supportTextureRectangle() && tex->isBloomTexture() && tex->mipMapOff()
			&& (!isPowerOf2(tex->getWidth()) || !isPowerOf2(tex->getHeight())));
}

// ***************************************************************************
bool CDriverGL::activeFrameBufferObject(ITexture * tex)
{
	if(supportFrameBufferObject()/* && supportPackedDepthStencil()*/)
	{
		if(tex)
		{
			CTextureDrvInfosGL*	gltext = (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
			return gltext->activeFrameBufferObject(tex);
		}
		else
		{
			nglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			return true;
		}
	}

	return false;
}

// --------------------------------------------------
void CDriverGL::disableHardwareVertexProgram()
{
	H_AUTO_OGL(CDriverGL_disableHardwareVertexProgram)
	_Extensions.DisableHardwareVertexProgram= true;
}

void CDriverGL::disableHardwarePixelProgram()
{
	H_AUTO_OGL(CDriverGL_disableHardwarePixelProgram)
	_Extensions.DisableHardwarePixelProgram= true;
}

// ***************************************************************************
void CDriverGL::disableHardwareVertexArrayAGP()
{
	H_AUTO_OGL(CDriverGL_disableHardwareVertexArrayAGP)
	_Extensions.DisableHardwareVertexArrayAGP= true;
}

// ***************************************************************************
void CDriverGL::disableHardwareTextureShader()
{
	H_AUTO_OGL(CDriverGL_disableHardwareTextureShader)
	_Extensions.DisableHardwareTextureShader= true;
}

// --------------------------------------------------
void CDriverGL::resetTextureShaders()
{
	H_AUTO_OGL(CDriverGL_resetTextureShaders);

#ifndef USE_OPENGLES
	if (_Extensions.NVTextureShader)
	{
		glEnable(GL_TEXTURE_SHADER_NV);

		for (uint stage = 0; stage < inlGetNumTextStages(); ++stage)
		{
			_DriverGLStates.activeTextureARB(stage);
			if (stage != 0)
			{
				glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB + stage - 1);
			}

			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);

			_CurrentTexAddrMode[stage] = GL_NONE;
		}

		glDisable(GL_TEXTURE_SHADER_NV);

		_NVTextureShaderEnabled = false;
	}
#endif
}

// --------------------------------------------------
bool CDriverGL::isTextureExist(const ITexture&tex)
{
	H_AUTO_OGL(CDriverGL_isTextureExist)
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
bool CDriverGL::clear2D(CRGBA rgba)
{
	H_AUTO_OGL(CDriverGL_clear2D)
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL::clearZBuffer(float zval)
{
	H_AUTO_OGL(CDriverGL_clearZBuffer);

#ifdef USE_OPENGLES
	glClearDepthf(zval);
#else
	glClearDepth(zval);
#endif

	_DriverGLStates.enableZWrite(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL::clearStencilBuffer(float stencilval)
{
	H_AUTO_OGL(CDriverGL_clearStencilBuffer)
	glClearStencil((int)stencilval);

	glClear(GL_STENCIL_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
void CDriverGL::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	H_AUTO_OGL(CDriverGL_setColorMask )
	glColorMask (bRed, bGreen, bBlue, bAlpha);
}

// --------------------------------------------------
bool CDriverGL::swapBuffers()
{
	H_AUTO_OGL(CDriverGL_swapBuffers)

	++ _SwapBufferCounter;
	// Reset texture shaders
	//resetTextureShaders();
	activeVertexProgram(NULL);
	activePixelProgram(NULL);

#ifndef USE_OPENGLES
	/* Yoyo: must do this (GeForce bug ??) else weird results if end render with a VBHard.
		Setup a std vertex buffer to ensure NVidia synchronisation.
	*/
	if (!_Extensions.ARBVertexBufferObject && _Extensions.NVVertexArrayRange)
	{
		static	CVertexBuffer	dummyVB;
		static	bool			dummyVBinit= false;
		if(!dummyVBinit)
		{
			dummyVBinit= true;
			// setup a full feature VB (maybe not useful ... :( ).
			dummyVB.setVertexFormat(CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag|
				CVertexBuffer::PrimaryColorFlag|CVertexBuffer::SecondaryColorFlag|
				CVertexBuffer::TexCoord0Flag|CVertexBuffer::TexCoord1Flag|
				CVertexBuffer::TexCoord2Flag|CVertexBuffer::TexCoord3Flag
				);
			// some vertices.
			dummyVB.setNumVertices(10);
		}
		// activate each frame to close VBHard rendering.
		//	NVidia: This also force a SetFence on if last VB was a VBHard, "closing" it before swap.
		//
		activeVertexBuffer(dummyVB);
		nlassert(_CurrentVertexBufferHard==NULL);
	}

	/* PATCH For Possible NVidia Synchronisation.
	/*/
	// Because of Bug with GeForce, must finishFence() for all VBHard.
	/*set<IVertexBufferHardGL*>::iterator		itVBHard= _VertexBufferHardSet.Set.begin();
	while(itVBHard != _VertexBufferHardSet.Set.end() )
	{
		// Need only to do it for NVidia VB ones.
		if((*itVBHard)->NVidiaVertexBufferHard)
		{
			CVertexBufferHardGLNVidia	*vbHardNV= static_cast<CVertexBufferHardGLNVidia*>(*itVBHard);
			// If needed, "flush" these VB.
			vbHardNV->finishFence();
		}
		itVBHard++;
	}*/
	/* Need to Do this code only if Synchronisation PATCH before not done!
		AS NV_Fence GeForce Implementation says. Test each frame the NVFence, until completion.
		NB: finish is not required here. Just test. This is like a "non block synchronisation"
	 */
	if (!_Extensions.ARBVertexBufferObject && _Extensions.NVVertexArrayRange)
	{
		set<IVertexBufferHardGL*>::iterator		itVBHard= _VertexBufferHardSet.Set.begin();
		while(itVBHard != _VertexBufferHardSet.Set.end() )
		{
			if((*itVBHard)->VBType == IVertexBufferHardGL::NVidiaVB)
			{
				CVertexBufferHardGLNVidia	*vbHardNV= static_cast<CVertexBufferHardGLNVidia*>(*itVBHard);
				if(vbHardNV->isFenceSet())
				{
					// update Fence Cache.
					vbHardNV->testFence();
				}
			}
			itVBHard++;
		}
	}
#endif

	if (!_WndActive)
	{
		if (_AGPVertexArrayRange) _AGPVertexArrayRange->updateLostBuffers();
		if (_VRAMVertexArrayRange) _VRAMVertexArrayRange->updateLostBuffers();
	}

#ifdef USE_OPENGLES

	eglSwapBuffers (_EglDisplay, _EglSurface);

#elif defined(NL_OS_WINDOWS)

	SwapBuffers(_hDC);

#elif defined(NL_OS_MAC)

	// TODO: maybe do this somewhere else?
	if(_DestroyWindow)
	{
		[_autoreleasePool release];
		_autoreleasePool = [[NSAutoreleasePool alloc] init];
	}

	[_ctx flushBuffer];

#elif defined (NL_OS_UNIX)

	glXSwapBuffers(_dpy, _win);

#endif // NL_OS_WINDOWS

#ifndef USE_OPENGLES
	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o) ).
	for(uint stage=0;stage<inlGetNumTextStages(); stage++)
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
	if (_NVTextureShaderEnabled)
	{
		glDisable(GL_TEXTURE_SHADER_NV);
		_NVTextureShaderEnabled = false;
	}
#endif

	_CurrentMaterial= NULL;

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset Profile VBHardLock
	if(_VBHardProfiling)
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
bool CDriverGL::release()
{
	H_AUTO_OGL(CDriverGL_release)

	// release only if the driver was initialized
	if (!_Initialized) return true;

	// hide window
	showWindow(false);

	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	nlassert(_DepthStencilFBOs.empty());

	_SwapBufferCounter = 0;

	// delete querries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	deleteFragmentShaders();

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
void CDriverGL::setupViewport (const class CViewport& viewport)
{
	H_AUTO_OGL(CDriverGL_setupViewport )

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
		if(clientWidth)
			factorX = (float)_TextureTarget->getWidth() / (float)clientWidth;
		if(clientHeight)
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
void CDriverGL::getViewport(CViewport &viewport)
{
	H_AUTO_OGL(CDriverGL_getViewport)
	viewport = _CurrViewport;
}

// --------------------------------------------------
void CDriverGL::setupScissor (const class CScissor& scissor)
{
	H_AUTO_OGL(CDriverGL_setupScissor )

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
		if(clientWidth)
			factorX = (float) _TextureTarget->getWidth() / (float)clientWidth;
		if(clientHeight)
			factorY = (float) _TextureTarget->getHeight() / (float)clientHeight;
		x *= factorX;
		y *= factorY;
		width *= factorX;
		height *= factorY;
	}

	// enable or disable Scissor, but AFTER textureTarget adjust
	if(x==0.f && y==0.f && width>=1.f && height>=1.f)
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

		sint ix1=(sint)floor((float)clientWidth * (x+width) + 0.5f );
		clamp (ix1, 0, (sint)clientWidth);
		sint iy1=(sint)floor((float)clientHeight* (y+height) + 0.5f );
		clamp (iy1, 0, (sint)clientHeight);

		sint iwidth= ix1 - ix0;
		clamp (iwidth, 0, (sint)clientWidth);
		sint iheight= iy1 - iy0;
		clamp (iheight, 0, (sint)clientHeight);

		glScissor (ix0, iy0, iwidth, iheight);
		glEnable(GL_SCISSOR_TEST);
	}
}

uint8 CDriverGL::getBitPerPixel ()
{
	H_AUTO_OGL(CDriverGL_getBitPerPixel )
	return _CurrentMode.Depth;
}

const char *CDriverGL::getVideocardInformation ()
{
	H_AUTO_OGL(CDriverGL_getVideocardInformation)
	static char name[1024];

	if (!_Initialized) return "OpenGL isn't initialized";

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);
	const char *version = (const char *) glGetString (GL_VERSION);

	smprintf(name, 1024, "OpenGL / %s / %s / %s", vendor, renderer, version);
	return name;
}

bool CDriverGL::clipRect(NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL_clipRect)
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

void CDriverGL::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL_getBufferPart )
	bitmap.reset();

	if(clipRect(rect))
	{
		bitmap.resize(rect.Width, rect.Height, CBitmap::RGBA);
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels ().getPtr());
	}
}

void CDriverGL::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL_getZBufferPart )
	zbuffer.clear();

	if(clipRect(rect))
	{
		zbuffer.resize(rect.Width*rect.Height);

#ifdef USE_OPENGLES
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_DEPTH_COMPONENT16_OES, GL_FLOAT, &(zbuffer[0]));
#else
		glPixelTransferf(GL_DEPTH_SCALE, 1.0f) ;
		glPixelTransferf(GL_DEPTH_BIAS, 0.f) ;
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_DEPTH_COMPONENT , GL_FLOAT, &(zbuffer[0]));
#endif
	}
}

void CDriverGL::getZBuffer (std::vector<float>  &zbuffer)
{
	H_AUTO_OGL(CDriverGL_getZBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getZBufferPart(zbuffer, rect);
}

void CDriverGL::getBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL_getBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getBufferPart(bitmap, rect);
	bitmap.flipV();
}

bool CDriverGL::fillBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL_fillBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	if( rect.Width!=bitmap.getWidth() || rect.Height!=bitmap.getHeight() || bitmap.getPixelFormat()!=CBitmap::RGBA )
		return false;

#ifdef USE_OPENGLES
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rect.Width, rect.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap.getPixels()[0]));
//	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, smBackgroundCrop,0);
	nglDrawTexfOES(0.f, 0.f, 0.f, 1.f, 1.f);
#else
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glDrawPixels (rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap.getPixels()[0]) );
#endif

	return true;
}

// ***************************************************************************
void CDriverGL::copyFrameBufferToTexture(ITexture *tex,
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
	H_AUTO_OGL(CDriverGL_copyFrameBufferToTexture)
	bool compressed = false;
	getGlTextureFormat(*tex, compressed);
	nlassert(!compressed);
	// first, mark the texture as valid, and make sure there is a corresponding texture in the device memory
	setupTexture(*tex);
	CTextureDrvInfosGL*	gltext = (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(NULL);
	_DriverGLStates.activeTextureARB(0);
	// setup texture mode, after activeTextureARB()
	CDriverGLStates::TTextureMode textureMode= CDriverGLStates::Texture2D;

#ifndef USE_OPENGLES
	if(gltext->TextureMode == GL_TEXTURE_RECTANGLE_NV)
		textureMode = CDriverGLStates::TextureRect;
#endif

	_DriverGLStates.setTextureMode(textureMode);
	if (tex->isTextureCube())
	{
		if(_Extensions.ARBTextureCubeMap)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);
			glCopyTexSubImage2D(NLCubeFaceToGLCubeFace[cubeFace], level, offsetx, offsety, x, y, width, height);
		}
	}
	else
	{
		glBindTexture(gltext->TextureMode, gltext->ID);
		glCopyTexSubImage2D(gltext->TextureMode, level, offsetx, offsety, x, y, width, height);
	}
	// disable texturing.
	_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
	_CurrentTexture[0] = NULL;
	_CurrentTextureInfoGL[0] = NULL;
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(tex);
}

// ***************************************************************************
void CDriverGL::setPolygonMode (TPolygonMode mode)
{
	H_AUTO_OGL(CDriverGL_setPolygonMode )
	IDriver::setPolygonMode (mode);

#ifndef USE_OPENGLES
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
#endif
}

// ***************************************************************************
bool CDriverGL::fogEnabled()
{
	H_AUTO_OGL(CDriverGL_fogEnabled)
	return _FogEnabled;
}

// ***************************************************************************
void CDriverGL::enableFog(bool enable)
{
	H_AUTO_OGL(CDriverGL_enableFog)
	_DriverGLStates.enableFog(enable);
	_FogEnabled= enable;
}

// ***************************************************************************
void CDriverGL::setupFog(float start, float end, CRGBA color)
{
	H_AUTO_OGL(CDriverGL_setupFog)
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);

	_CurrentFogColor[0]= color.R/255.0f;
	_CurrentFogColor[1]= color.G/255.0f;
	_CurrentFogColor[2]= color.B/255.0f;
	_CurrentFogColor[3]= color.A/255.0f;

	glFogfv(GL_FOG_COLOR, _CurrentFogColor);

#ifndef USE_OPENGLES
	/** Special : with vertex program, using the extension EXT_vertex_shader, fog is emulated using 1 more constant to scale result to [0, 1]
	  */
	if (_Extensions.EXTVertexShader && !_Extensions.NVVertexProgram && !_Extensions.ARBVertexProgram)
	{
		if (!_ATIFogRangeFixed)
		{
			// last constant is used to store fog information (fog must be rescaled to [0, 1], because of a driver bug)
			if (start != end)
			{
				float datas[] = { 1.f / (start - end), - end / (start - end), 0, 0 };
				nglSetInvariantEXT(_EVSConstantHandle + _EVSNumConstant, GL_FLOAT, datas);
			}
			else
			{
				float datas[] = { 0.f, 0, 0, 0 };
				nglSetInvariantEXT(_EVSConstantHandle + _EVSNumConstant, GL_FLOAT, datas);
			}
		}
	}
#endif

	_FogStart = start;
	_FogEnd = end;
}

// ***************************************************************************
float CDriverGL::getFogStart() const
{
	H_AUTO_OGL(CDriverGL_getFogStart)
	return _FogStart;
}

// ***************************************************************************
float CDriverGL::getFogEnd() const
{
	H_AUTO_OGL(CDriverGL_getFogEnd)
	return _FogEnd;
}

// ***************************************************************************
CRGBA CDriverGL::getFogColor() const
{
	H_AUTO_OGL(CDriverGL_getFogColor)
	CRGBA	ret;
	ret.R= (uint8)(_CurrentFogColor[0]*255);
	ret.G= (uint8)(_CurrentFogColor[1]*255);
	ret.B= (uint8)(_CurrentFogColor[2]*255);
	ret.A= (uint8)(_CurrentFogColor[3]*255);
	return ret;
}


// ***************************************************************************
void			CDriverGL::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{
	H_AUTO_OGL(CDriverGL_profileRenderedPrimitives)
	pIn= _PrimitiveProfileIn;
	pOut= _PrimitiveProfileOut;
}


// ***************************************************************************
uint32			CDriverGL::profileAllocatedTextureMemory()
{
	H_AUTO_OGL(CDriverGL_profileAllocatedTextureMemory)
	return _AllocatedTextureMemory;
}


// ***************************************************************************
uint32			CDriverGL::profileSetupedMaterials() const
{
	H_AUTO_OGL(CDriverGL_profileSetupedMaterials)
	return _NbSetupMaterialCall;
}


// ***************************************************************************
uint32			CDriverGL::profileSetupedModelMatrix() const
{
	H_AUTO_OGL(CDriverGL_profileSetupedModelMatrix)

	return _NbSetupModelMatrixCall;
}


// ***************************************************************************
void			CDriverGL::enableUsedTextureMemorySum (bool enable)
{
	H_AUTO_OGL(CDriverGL_enableUsedTextureMemorySum )

	if (enable)
	{
		nlinfo ("3D: PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverGL");
		_TextureUsed.reserve(512);
	}
	_SumTextureMemoryUsed=enable;
}


// ***************************************************************************
uint32			CDriverGL::getUsedTextureMemory() const
{
	H_AUTO_OGL(CDriverGL_getUsedTextureMemory)

	// Sum memory used
	uint32 memory=0;

	// For each texture used
	std::vector<CTextureDrvInfosGL *>::const_iterator ite = _TextureUsed.begin();
	while (ite!=_TextureUsed.end())
	{
		// Get the gl texture
		CTextureDrvInfosGL*	gltext;
		gltext= (*ite);

		// Sum the memory used by this texture
		if (gltext)
			memory+=gltext->TextureMemory;

		// Next texture
		ite++;
	}

	// Return the count
	return memory;
}


// ***************************************************************************
bool CDriverGL::supportTextureShaders() const
{
	H_AUTO_OGL(CDriverGL_supportTextureShaders)

	// fully supported by NV_TEXTURE_SHADER
	return _Extensions.NVTextureShader;
}

// ***************************************************************************
bool CDriverGL::supportWaterShader() const
{
	H_AUTO_OGL(CDriverGL_supportWaterShader);

	if(_Extensions.ARBFragmentProgram && ARBWaterShader[0] != 0) return true;

	if (!_Extensions.EXTVertexShader && !_Extensions.NVVertexProgram && !_Extensions.ARBVertexProgram) return false; // should support vertex programs
	if (!_Extensions.NVTextureShader && !_Extensions.ATIFragmentShader && !_Extensions.ARBFragmentProgram) return false;
	return true;
}

// ***************************************************************************
bool CDriverGL::supportTextureAddrMode(CMaterial::TTexAddressingMode /* mode */) const
{
	H_AUTO_OGL(CDriverGL_supportTextureAddrMode)

	if (_Extensions.NVTextureShader)
	{
		// all the given addessing mode are supported with this extension
		return true;
	}
	else
	{
		return false;
	}
}

// ***************************************************************************
void CDriverGL::setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4])
{
	H_AUTO_OGL(CDriverGL_setMatrix2DForTextureOffsetAddrMode)

	if (!supportTextureShaders()) return;
	//nlassert(supportTextureShaders());
	nlassert(stage < inlGetNumTextStages() );
	_DriverGLStates.activeTextureARB(stage);

#ifndef USE_OPENGLES
	glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat);
#endif
}


// ***************************************************************************
void CDriverGL::enableNVTextureShader(bool enabled)
{
	H_AUTO_OGL(CDriverGL_enableNVTextureShader)

	if (enabled != _NVTextureShaderEnabled)
	{
#ifndef USE_OPENGLES
		if (enabled)
		{
			glEnable(GL_TEXTURE_SHADER_NV);
		}
		else
		{
			glDisable(GL_TEXTURE_SHADER_NV);
		}
#endif
		_NVTextureShaderEnabled = enabled;
	}
}

// ***************************************************************************
void CDriverGL::checkForPerPixelLightingSupport()
{
	H_AUTO_OGL(CDriverGL_checkForPerPixelLightingSupport)

	// we need at least 3 texture stages and cube map support + EnvCombine4 or 3 support
	// TODO : support for EnvCombine3
	// TODO : support for less than 3 stages

	_SupportPerPixelShaderNoSpec = (_Extensions.NVTextureEnvCombine4 || _Extensions.ATITextureEnvCombine3)
								   && _Extensions.ARBTextureCubeMap
								   && _Extensions.NbTextureStages >= 3
								   && (_Extensions.NVVertexProgram || _Extensions.ARBVertexProgram || _Extensions.EXTVertexShader);

	_SupportPerPixelShader = (_Extensions.NVTextureEnvCombine4 || _Extensions.ATITextureEnvCombine3)
							 && _Extensions.ARBTextureCubeMap
							 && _Extensions.NbTextureStages >= 2
							 && (_Extensions.NVVertexProgram || _Extensions.ARBVertexProgram || _Extensions.EXTVertexShader);
}

// ***************************************************************************
bool CDriverGL::supportPerPixelLighting(bool specular) const
{
	H_AUTO_OGL(CDriverGL_supportPerPixelLighting)

	return specular ? _SupportPerPixelShader : _SupportPerPixelShaderNoSpec;
}

// ***************************************************************************
void CDriverGL::setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess)
{
	H_AUTO_OGL(CDriverGL_setPerPixelLightingLight)

	_PPLExponent = shininess;
	_PPLightDiffuseColor = diffuse;
	_PPLightSpecularColor = specular;
}

// ***************************************************************************
bool CDriverGL::supportBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL_supportBlendConstantColor)
	return _Extensions.EXTBlendColor;
}

// ***************************************************************************
void CDriverGL::setBlendConstantColor(NLMISC::CRGBA col)
{
	H_AUTO_OGL(CDriverGL_setBlendConstantColor)

	// bkup
	_CurrentBlendConstantColor= col;

	// update GL
	if(!_Extensions.EXTBlendColor)
		return;

#ifndef USE_OPENGLES
	static const	float	OO255= 1.0f/255;
	nglBlendColorEXT(col.R*OO255, col.G*OO255, col.B*OO255, col.A*OO255);
#endif
}

// ***************************************************************************
NLMISC::CRGBA CDriverGL::getBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

	return	_CurrentBlendConstantColor;
}

// ***************************************************************************
uint			CDriverGL::getNbTextureStages() const
{
	H_AUTO_OGL(CDriverGL_getNbTextureStages)
	return inlGetNumTextStages();
}

// ***************************************************************************
void CDriverGL::refreshProjMatrixFromGL()
{
	H_AUTO_OGL(CDriverGL_refreshProjMatrixFromGL)

	if (!_ProjMatDirty) return;
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	_GLProjMat.set(mat);
	_ProjMatDirty = false;
}

// ***************************************************************************
bool CDriverGL::supportEMBM() const
{
	H_AUTO_OGL(CDriverGL_supportEMBM);

	// For now, supported via ATI extension
	return _Extensions.ATIEnvMapBumpMap;
}

// ***************************************************************************
bool CDriverGL::isEMBMSupportedAtStage(uint stage) const
{
	H_AUTO_OGL(CDriverGL_isEMBMSupportedAtStage)

	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return _StageSupportEMBM[stage];
}

// ***************************************************************************
void CDriverGL::setEMBMMatrix(const uint stage,const float mat[4])
{
	H_AUTO_OGL(CDriverGL_setEMBMMatrix)

#ifndef USE_OPENGLES
	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	//
	if (_Extensions.ATIEnvMapBumpMap)
	{
		_DriverGLStates.activeTextureARB(stage);
		nglTexBumpParameterfvATI(GL_BUMP_ROT_MATRIX_ATI, const_cast<float *>(mat));
	}
#endif
}

// ***************************************************************************
void CDriverGL::initEMBM()
{
	H_AUTO_OGL(CDriverGL_initEMBM);

#ifndef USE_OPENGLES
	if (supportEMBM())
	{
		std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);
		if (_Extensions.ATIEnvMapBumpMap)
		{
			// Test which stage support EMBM
			GLint numEMBMUnits;

			nglGetTexBumpParameterivATI(GL_BUMP_NUM_TEX_UNITS_ATI, &numEMBMUnits);

			std::vector<GLint> EMBMUnits(numEMBMUnits);

			// get array of units that supports EMBM
			nglGetTexBumpParameterivATI(GL_BUMP_TEX_UNITS_ATI, &EMBMUnits[0]);

			numEMBMUnits = std::min(numEMBMUnits, (GLint) _Extensions.NbTextureStages);

			EMBMUnits.resize(numEMBMUnits);

			uint k;
			for(k = 0; k < EMBMUnits.size(); ++k)
			{
				uint stage = EMBMUnits[k] - GL_TEXTURE0_ARB;
				if (stage < (IDRV_MAT_MAXTEXTURES - 1))
				{
					_StageSupportEMBM[stage] = true;
				}
			}
			// setup each stage to apply the bump map to the next stage (or previous if there's an unit at the last stage)
			for(k = 0; k < (uint) _Extensions.NbTextureStages; ++k)
			{
				if (_StageSupportEMBM[k])
				{
					// setup each stage so that it apply EMBM on the next stage
					_DriverGLStates.activeTextureARB(k);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
					if (k != (uint) (_Extensions.NbTextureStages - 1))
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_BUMP_TARGET_ATI, GL_TEXTURE0_ARB + k + 1);
					}
					else
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_BUMP_TARGET_ATI, GL_TEXTURE0_ARB);
					}
				}
			}
			_DriverGLStates.activeTextureARB(0);
		}
	}
#endif
}

// ***************************************************************************
/** Water fragment program with extension ARB_fragment_program
  */
static const char *WaterCodeNoDiffuseForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    oCol, bmValue, texture[2], 2D;												\n\
END ";

static const char *WaterCodeNoDiffuseWithFogForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB fogValue		  = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#compute fog																		\n\
MAD_SAT tmpFog, fogValue.x, fogFactor.x, fogFactor.y;								\n\
LRP    oCol, tmpFog.x, envMap, fogColor;											\n\
END ";

// **************************************************************************************
/** Water fragment program with extension ARB_fragment_program and a diffuse map applied
  */
static const char *WaterCodeForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#read diffuse																		\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
#modulate diffuse and envmap to get result											\n\
MUL    oCol, diffuse, envMap;														\n\
END ";

static const char *WaterCodeWithFogForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
ATTRIB fogValue		   = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
MAD_SAT tmpFog, fogValue.x, fogFactor.x, fogFactor.y;								\n\
#modulate diffuse and envmap to get result											\n\
MUL    diffuse, diffuse, envMap;													\n\
LRP    oCol, tmpFog.x, diffuse, fogColor;											\n\
END ";

// ***************************************************************************
/** Load a ARB_fragment_program_code, and ensure it is loaded natively
  */
uint loadARBFragmentProgramStringNative(const char *prog, bool forceNativePrograms)
{
	H_AUTO_OGL(loadARBFragmentProgramStringNative);
	if (!prog)
	{
		nlwarning("The param 'prog' is null, cannot load");
		return 0;
	}

#ifndef USE_OPENGLES
	GLuint progID;
	nglGenProgramsARB(1, &progID);
	if (!progID)
	{
		nlwarning("glGenProgramsARB returns a progID NULL");
		return 0;
	}
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progID);
	GLint errorPos, isNative;
	nglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(prog), prog);
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	nglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
	if (errorPos == -1)
	{
		if (!isNative && forceNativePrograms)
		{
			nlwarning("Fragment program isn't supported natively; purging program");
			nglDeleteProgramsARB(1, &progID);
			return 0;
		}
		return progID;
	}
	else
	{
		nlwarning("init fragment program failed: errorPos: %d isNative: %d: %s", errorPos, isNative, (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
	}
#endif

	return 0;
}

// ***************************************************************************
/** R200 Fragment Shader :
  * Send fragment shader to fetch a perturbed envmap from the addition of 2 bumpmap
  * The result is in R2 after the 2nd pass
  */
static void fetchPerturbedEnvMapR200()
{
	H_AUTO_OGL(CDriverGL_fetchPerturbedEnvMapR200);

#ifndef USE_OPENGLES
	////////////
	// PASS 1 //
	////////////
	nglSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 0
	nglSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 1
	nglPassTexCoordATI(GL_REG_2_ATI, GL_TEXTURE2_ARB, GL_SWIZZLE_STR_ATI);	// get texcoord for envmap

	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords
	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_1_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 2 & add to bump map 1

	////////////
	// PASS 2 //
	////////////
	nglSampleMapATI(GL_REG_2_ATI, GL_REG_2_ATI, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords
#endif
}

// ***************************************************************************
void CDriverGL::initFragmentShaders()
{
	H_AUTO_OGL(CDriverGL_initFragmentShaders);

#ifndef USE_OPENGLES
	///////////////////
	// WATER SHADERS //
	///////////////////

	// the ARB_fragment_program is prioritary over other extensions when present
	if (_Extensions.ARBFragmentProgram)
	{
		nlinfo("WATER: Try ARB_fragment_program");
		ARBWaterShader[0] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[1] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseWithFogForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[2] = loadARBFragmentProgramStringNative(WaterCodeForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[3] = loadARBFragmentProgramStringNative(WaterCodeWithFogForARBFragmentProgram, _ForceNativeFragmentPrograms);
		bool ok = true;
		for(uint k = 0; k < 4; ++k)
		{
			if (!ARBWaterShader[k])
			{
				ok = false;
				deleteARBFragmentPrograms();
				nlwarning("WATER: fragment %d is not loaded, not using ARB_fragment_program at all", k);
				break;
			}
		}
		if (ok)
		{
			nlinfo("WATER: ARB_fragment_program OK, Use it");
			return;
		}
	}

	if (_Extensions.ATIFragmentShader)
	{
		nlinfo("WATER: Try ATI_fragment_program");
		///////////
		// WATER //
		///////////
		ATIWaterShaderHandleNoDiffuseMap = nglGenFragmentShadersATI(1);

		ATIWaterShaderHandle = nglGenFragmentShadersATI(1);

		if (!ATIWaterShaderHandle || !ATIWaterShaderHandleNoDiffuseMap)
		{
			ATIWaterShaderHandleNoDiffuseMap = ATIWaterShaderHandle = 0;
			nlwarning("Couldn't generate water shader using ATI_fragment_shader !");
		}
		else
		{
			glGetError();
			// Water shader for R200 : we just add the 2 bump map contributions (du, dv). We then use this contribution to perturbate the envmap
			nglBindFragmentShaderATI(ATIWaterShaderHandleNoDiffuseMap);
			nglBeginFragmentShaderATI();
			//
			fetchPerturbedEnvMapR200();
			nglColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			nglAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			//
			nglEndFragmentShaderATI();
			GLenum error = glGetError();
			nlassert(error == GL_NONE);

			// The same but with a diffuse map added
			nglBindFragmentShaderATI(ATIWaterShaderHandle);
			nglBeginFragmentShaderATI();
			//
			fetchPerturbedEnvMapR200();

			nglSampleMapATI(GL_REG_3_ATI, GL_TEXTURE3_ARB, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords
			nglColorFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords
			nglAlphaFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);

			nglEndFragmentShaderATI();
			error = glGetError();
			nlassert(error == GL_NONE);
			nglBindFragmentShaderATI(0);
		}

		////////////
		// CLOUDS //
		////////////
		ATICloudShaderHandle = nglGenFragmentShadersATI(1);

		if (!ATICloudShaderHandle)
		{
			nlwarning("Couldn't generate cloud shader using ATI_fragment_shader !");
		}
		else
		{
			glGetError();
			nglBindFragmentShaderATI(ATICloudShaderHandle);
			nglBeginFragmentShaderATI();
			//
			nglSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI); // sample texture 0
			nglSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI); // sample texture 1
			// lerp between tex 0 & tex 1 using diffuse alpha
			nglAlphaFragmentOp3ATI(GL_LERP_ATI, GL_REG_0_ATI, GL_NONE, GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_NONE);
			//nglAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE);
			// output 0 as RGB
			//nglColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_ZERO, GL_NONE, GL_NONE);
			// output alpha multiplied by constant 0
			nglAlphaFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_CON_0_ATI, GL_NONE, GL_NONE);
			nglEndFragmentShaderATI();
			GLenum error = glGetError();
			nlassert(error == GL_NONE);
			nglBindFragmentShaderATI(0);
		}
	}

	// if none of the previous programs worked, fallback on NV_texture_shader, or (todo) simpler shader
#endif
}

// ***************************************************************************
void CDriverGL::deleteARBFragmentPrograms()
{
	H_AUTO_OGL(CDriverGL_deleteARBFragmentPrograms);

#ifndef USE_OPENGLES
	for(uint k = 0; k < 4; ++k)
	{
		if (ARBWaterShader[k])
		{
			GLuint progId = (GLuint) ARBWaterShader[k];
			nglDeleteProgramsARB(1, &progId);
			ARBWaterShader[k] = 0;
		}
	}
#endif
}

// ***************************************************************************
void CDriverGL::deleteFragmentShaders()
{
	H_AUTO_OGL(CDriverGL_deleteFragmentShaders)

#ifndef USE_OPENGLES
	deleteARBFragmentPrograms();

	if (ATIWaterShaderHandleNoDiffuseMap)
	{
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandleNoDiffuseMap);
		ATIWaterShaderHandleNoDiffuseMap = 0;
	}
	if (ATIWaterShaderHandle)
	{
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandle);
		ATIWaterShaderHandle = 0;
	}
	if (ATICloudShaderHandle)
	{
		nglDeleteFragmentShaderATI((GLuint) ATICloudShaderHandle);
		ATICloudShaderHandle = 0;
	}
#endif
}

// ***************************************************************************
void CDriverGL::finish()
{
	H_AUTO_OGL(CDriverGL_finish)
	glFinish();
}

// ***************************************************************************
void CDriverGL::flush()
{
	H_AUTO_OGL(CDriverGL_flush)
	glFlush();
}

// ***************************************************************************
void	CDriverGL::setSwapVBLInterval(uint interval)
{
	H_AUTO_OGL(CDriverGL_setSwapVBLInterval);

	if (!_Initialized)
		return;

	bool res = true;

#ifdef USE_OPENGLES
	res = eglSwapInterval(_EglDisplay, _Interval) == EGL_TRUE;
#elif defined(NL_OS_WINDOWS)
	if(_Extensions.WGLEXTSwapControl)
	{
		res = nwglSwapIntervalEXT(_Interval) == TRUE;
	}
#elif defined(NL_OS_MAC)
	[_ctx setValues:(GLint*)&interval forParameter:NSOpenGLCPSwapInterval];
#elif defined(NL_OS_UNIX)
	if (_win && _Extensions.GLXEXTSwapControl)
	{
		nglXSwapIntervalEXT(_dpy, _win, interval);
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
uint	CDriverGL::getSwapVBLInterval()
{
	H_AUTO_OGL(CDriverGL_getSwapVBLInterval)

#ifdef USE_OPENGLES
#elif defined(NL_OS_WINDOWS)
	if(_Extensions.WGLEXTSwapControl)
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
void	CDriverGL::enablePolygonSmoothing(bool smooth)
{
	H_AUTO_OGL(CDriverGL_enablePolygonSmoothing);

	if (_PolygonSmooth == smooth) return;

#ifndef USE_OPENGLES
	if(smooth)
		glEnable(GL_POLYGON_SMOOTH);
	else
		glDisable(GL_POLYGON_SMOOTH);
#endif

	_PolygonSmooth= smooth;
}

// ***************************************************************************
bool	CDriverGL::isPolygonSmoothingEnabled() const
{
	H_AUTO_OGL(CDriverGL_isPolygonSmoothingEnabled)

	return _PolygonSmooth;
}

// ***************************************************************************
void	CDriverGL::startProfileVBHardLock()
{
	if(_VBHardProfiling)
		return;

	// start
	_VBHardProfiles.clear();
	_VBHardProfiles.reserve(50);
	_VBHardProfiling= true;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}

// ***************************************************************************
void	CDriverGL::endProfileVBHardLock(vector<std::string> &result)
{
	if(!_VBHardProfiling)
		return;

	// Fill infos.
	result.clear();
	result.resize(_VBHardProfiles.size() + 1);
	float	total= 0;
	for(uint i=0;i<_VBHardProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CVBHardProfile	&vbProf= _VBHardProfiles[i];
		const char	*vbName;
		if(vbProf.VBHard && !vbProf.VBHard->getName().empty())
		{
			vbName= vbProf.VBHard->getName().c_str();
		}
		else
		{
			vbName= "????";
		}
		// Display in ms.
		float	timeLock= (float)CTime::ticksToSecond(vbProf.AccumTime)*1000 / max(_NumVBHardProfileFrame,1U);
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", vbName, vbProf.Change?'*':' ', timeLock );
		total+= timeLock;

		result[i]= tmp;
	}
	result[_VBHardProfiles.size()]= toString("Total: %2.3f", total);

	// clear.
	_VBHardProfiling= false;
	contReset(_VBHardProfiles);
}

// ***************************************************************************
void	CDriverGL::appendVBHardLockProfile(NLMISC::TTicks time, CVertexBuffer *vb)
{
	// must allocate a new place?
	if(_CurVBHardLockCount>=_VBHardProfiles.size())
	{
		_VBHardProfiles.resize(_VBHardProfiles.size()+1);
		// set the original VBHard
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
	}

	// Accumulate.
	_VBHardProfiles[_CurVBHardLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if(_VBHardProfiles[_CurVBHardLockCount].VBHard != vb)
	{
		// flag, and set new
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
		_VBHardProfiles[_CurVBHardLockCount].Change= true;
	}

	// next!
	_CurVBHardLockCount++;
}

// ***************************************************************************
void CDriverGL::startProfileIBLock()
{
	// not implemented
}

// ***************************************************************************
void CDriverGL::endProfileIBLock(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void CDriverGL::profileIBAllocation(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void	CDriverGL::profileVBHardAllocation(std::vector<std::string> &result)
{
	result.clear();
	result.reserve(1000);
	result.push_back(toString("Memory Allocated: %4d Ko in AGP / %4d Ko in VRAM",
		getAvailableVertexAGPMemory()/1000, getAvailableVertexVRAMMemory()/1000 ));
	result.push_back(toString("Num VBHard: %d", _VertexBufferHardSet.Set.size()));

	uint	totalMemUsed= 0;
	set<IVertexBufferHardGL*>::iterator	it;
	for(it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			totalMemUsed+= vSize*numVerts;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000) );

	for(it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			result.push_back(toString("  %16s: %4d ko (format: %d / numVerts: %d)",
				vbHard->VB->getName().c_str(), vSize*numVerts/1000, vSize, numVerts ));
		}
	}
}

// ***************************************************************************
bool CDriverGL::supportCloudRenderSinglePass() const
{
	H_AUTO_OGL(CDriverGL_supportCloudRenderSinglePass)

	 //return _Extensions.NVTextureEnvCombine4 || (_Extensions.ATIXTextureEnvRoute && _Extensions.EXTTextureEnvCombine);
	// there are slowdown for now with ati fragment shader... don't know why
	return _Extensions.NVTextureEnvCombine4 || _Extensions.ATIFragmentShader;
}

// ***************************************************************************
void CDriverGL::retrieveATIDriverVersion()
{
	H_AUTO_OGL(CDriverGL_retrieveATIDriverVersion)
	_ATIDriverVersion = 0;
	// we may need this driver version to fix flaws of previous ati drivers version (fog issue with V.P)
#ifdef NL_OS_WINDOWS
	// get from the registry
	HKEY parentKey;
	// open key about current video card
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &parentKey);
	if (result == ERROR_SUCCESS)
	{
		// find last config
		DWORD keyIndex = 0;
		uint latestConfigVersion = 0;
		char subKeyName[256];
		char latestSubKeyName[256] = "";
		DWORD nameBufferSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
		FILETIME lastWriteTime;
		bool configFound = false;
		for(;;)
		{
			nameBufferSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
			result = RegEnumKeyEx(parentKey, keyIndex, subKeyName, &nameBufferSize, NULL, NULL, NULL, &lastWriteTime);
			if (result == ERROR_NO_MORE_ITEMS) break;
			if (result == ERROR_SUCCESS)
			{
				// see if the name is numerical.
				bool isNumerical = true;
				for(uint k = 0; k < nameBufferSize; ++k)
				{
					if (!isdigit(subKeyName[k]))
					{
						isNumerical = false;
						break;
					}
				}
				if (isNumerical)
				{
					uint configVersion;
					fromString((const char*)subKeyName, configVersion);
					if (configVersion >= latestConfigVersion)
					{
						configFound = true;
						latestConfigVersion = configVersion;
						strcpy(latestSubKeyName, subKeyName);
					}
				}
				++ keyIndex;
			}
			else
			{
				RegCloseKey(parentKey);
				return;
			}
		}
		if (configFound)
		{
			HKEY subKey;
			result = RegOpenKeyEx(parentKey, latestSubKeyName, 0, KEY_READ, &subKey);
			if (result == ERROR_SUCCESS)
			{
				// see if it is a radeon card
				DWORD valueType;
				char driverDesc[256];
				DWORD driverDescBufSize = sizeof(driverDesc) / sizeof(driverDesc[0]);
				result = RegQueryValueEx(subKey, "DriverDesc", NULL, &valueType, (unsigned char *) driverDesc, &driverDescBufSize);
				if (result == ERROR_SUCCESS && valueType == REG_SZ)
				{
					toLower(driverDesc);
					if (strstr(driverDesc, "radeon")) // is it a radeon card ?
					{
						char driverVersion[256];
						DWORD driverVersionBufSize = sizeof(driverVersion) / sizeof(driverVersion[0]);
						result = RegQueryValueEx(subKey, "DriverVersion", NULL, &valueType, (unsigned char *) driverVersion, &driverVersionBufSize);
						if (result == ERROR_SUCCESS && valueType == REG_SZ)
						{
							int subVersionNumber[4];
							if (sscanf(driverVersion, "%d.%d.%d.%d", &subVersionNumber[0], &subVersionNumber[1], &subVersionNumber[2], &subVersionNumber[3]) == 4)
							{
								_ATIDriverVersion = (uint) subVersionNumber[3];
								/** see if fog range for V.P is bad in that driver version (is so, do a fix during vertex program conversion to EXT_vertex_shader
								  * In earlier versions of the driver, fog coordinates had to be output in the [0, 1] range
								  * From the 6.14.10.6343 driver, fog output must be in world units
								  */
								if (_ATIDriverVersion < 6343)
								{
									_ATIFogRangeFixed = false;
								}
							}
						}
					}
				}
			}
			RegCloseKey(subKey);
		}
		RegCloseKey(parentKey);
	}
#elif defined(NL_OS_MAC)
	// TODO: Missing Mac Implementation for ATI version retrieval
#elif defined (NL_OS_UNIX)
	// TODO for Linux: implement retrieveATIDriverVersion... assuming versions under linux are probably different
#endif
}

// ***************************************************************************
bool CDriverGL::supportMADOperator() const
{
	H_AUTO_OGL(CDriverGL_supportMADOperator)

	return _Extensions.NVTextureEnvCombine4 || _Extensions.ATITextureEnvCombine3;
}

// ***************************************************************************
uint CDriverGL::getNumAdapter() const
{
	H_AUTO_OGL(CDriverGL_getNumAdapter)

	return 1;
}

// ***************************************************************************
bool CDriverGL::getAdapter(uint adapter, CAdapter &desc) const
{
	H_AUTO_OGL(CDriverGL_getAdapter)

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
bool CDriverGL::setAdapter(uint adapter)
{
	H_AUTO_OGL(CDriverGL_setAdapter)

	return adapter == 0;
}

// ***************************************************************************
CVertexBuffer::TVertexColorType CDriverGL::getVertexColorFormat() const
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

	return CVertexBuffer::TRGBA;
}

// ***************************************************************************
void CDriverGL::startBench (bool wantStandardDeviation, bool quick, bool reset)
{
	CHTimer::startBench (wantStandardDeviation, quick, reset);
}

// ***************************************************************************
void CDriverGL::endBench ()
{
	CHTimer::endBench ();
}

// ***************************************************************************
void CDriverGL::displayBench (class NLMISC::CLog *log)
{
	// diplay
	CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	CHTimer::displayHierarchical(log, true, 48, 2);
	CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTimeWithoutSons);
}

#ifdef NL_DEBUG
void CDriverGL::dumpMappedBuffers()
{
	_AGPVertexArrayRange->dumpMappedBuffers();
}
#endif

// ***************************************************************************
void CDriverGL::checkTextureOn() const
{
	H_AUTO_OGL(CDriverGL_checkTextureOn)
	// tmp for debug
	CDriverGLStates &dgs = const_cast<CDriverGLStates &>(_DriverGLStates);
	uint currTexStage = dgs.getActiveTextureARB();
	for(uint k = 0; k < this->getNbTextureStages(); ++k)
	{
		dgs.activeTextureARB(k);
		GLboolean flag2D;
		GLboolean flagCM;
		GLboolean flagTR;
		glGetBooleanv(GL_TEXTURE_2D, &flag2D);
		glGetBooleanv(GL_TEXTURE_CUBE_MAP_ARB, &flagCM);
#ifdef USE_OPENGLES
		flagTR = true; // always true in OpenGL ES
#else
		glGetBooleanv(GL_TEXTURE_RECTANGLE_NV, &flagTR);
#endif
		switch(dgs.getTextureMode())
		{
			case CDriverGLStates::TextureDisabled:
				nlassert(!flag2D);
				nlassert(!flagCM);
			break;
			case CDriverGLStates::Texture2D:
				nlassert(flag2D);
				nlassert(!flagCM);
			break;
			case CDriverGLStates::TextureRect:
				nlassert(flagTR);
				nlassert(!flagCM);
			break;
			case CDriverGLStates::TextureCubeMap:
				nlassert(!flag2D);
				nlassert(flagCM);
			break;
			default:
			break;
		}
	}
	dgs.activeTextureARB(currTexStage);
}

// ***************************************************************************
bool CDriverGL::supportOcclusionQuery() const
{
	H_AUTO_OGL(CDriverGL_supportOcclusionQuery)
	return _Extensions.NVOcclusionQuery || _Extensions.ARBOcclusionQuery;
}

// ***************************************************************************
bool CDriverGL::supportTextureRectangle() const
{
	H_AUTO_OGL(CDriverGL_supportTextureRectangle);

	return (_Extensions.NVTextureRectangle || _Extensions.EXTTextureRectangle || _Extensions.ARBTextureRectangle);
}

// ***************************************************************************
bool CDriverGL::supportPackedDepthStencil() const
{
	H_AUTO_OGL(CDriverGL_supportPackedDepthStencil);

	return _Extensions.PackedDepthStencil;
}

// ***************************************************************************
bool CDriverGL::supportFrameBufferObject() const
{
	H_AUTO_OGL(CDriverGL_supportFrameBufferObject);

	return _Extensions.FrameBufferObject;
}

// ***************************************************************************
IOcclusionQuery *CDriverGL::createOcclusionQuery()
{
	H_AUTO_OGL(CDriverGL_createOcclusionQuery)
	nlassert(_Extensions.NVOcclusionQuery || _Extensions.ARBOcclusionQuery);

#ifndef USE_OPENGLES
	GLuint id;
	if (_Extensions.NVOcclusionQuery)
		nglGenOcclusionQueriesNV(1, &id);
	else
		nglGenQueriesARB(1, &id);
	if (id == 0) return NULL;
	COcclusionQueryGL *oqgl = new COcclusionQueryGL;
	oqgl->Driver = this;
	oqgl->ID = id;
	oqgl->OcclusionType = IOcclusionQuery::NotAvailable;
	_OcclusionQueryList.push_front(oqgl);
	oqgl->Iterator = _OcclusionQueryList.begin();
	oqgl->VisibleCount = 0;
	return oqgl;
#else
	return NULL;
#endif
}

// ***************************************************************************
void CDriverGL::deleteOcclusionQuery(IOcclusionQuery *oq)
{
	H_AUTO_OGL(CDriverGL_deleteOcclusionQuery);

#ifndef USE_OPENGLES
	if (!oq) return;
	COcclusionQueryGL *oqgl = NLMISC::safe_cast<COcclusionQueryGL *>(oq);
	nlassert((CDriverGL *) oqgl->Driver == this); // should come from the same driver
	oqgl->Driver = NULL;
	nlassert(oqgl->ID != 0);
	GLuint id = oqgl->ID;
	if (_Extensions.NVOcclusionQuery)
		nglDeleteOcclusionQueriesNV(1, &id);
	else
		nglDeleteQueriesARB(1, &id);
	_OcclusionQueryList.erase(oqgl->Iterator);
	if (oqgl == _CurrentOcclusionQuery)
	{
		_CurrentOcclusionQuery = NULL;
	}
	delete oqgl;
#endif
}

// ***************************************************************************
void COcclusionQueryGL::begin()
{
	H_AUTO_OGL(COcclusionQueryGL_begin);

#ifndef USE_OPENGLES
	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == NULL); // only one query at a time
	nlassert(ID);
	if (Driver->_Extensions.NVOcclusionQuery)
		nglBeginOcclusionQueryNV(ID);
	else
		nglBeginQueryARB(GL_SAMPLES_PASSED, ID);
	Driver->_CurrentOcclusionQuery = this;
	OcclusionType = NotAvailable;
	VisibleCount = 0;
#endif
}

// ***************************************************************************
void COcclusionQueryGL::end()
{
	H_AUTO_OGL(COcclusionQueryGL_end);

#ifndef USE_OPENGLES
	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == this); // only one query at a time
	nlassert(ID);
	if (Driver->_Extensions.NVOcclusionQuery)
		nglEndOcclusionQueryNV();
	else
		nglEndQueryARB(GL_SAMPLES_PASSED);
	Driver->_CurrentOcclusionQuery = NULL;
#endif
}

// ***************************************************************************
IOcclusionQuery::TOcclusionType COcclusionQueryGL::getOcclusionType()
{
	H_AUTO_OGL(COcclusionQueryGL_getOcclusionType);

#ifndef USE_OPENGLES
	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (OcclusionType == NotAvailable)
	{
		if (Driver->_Extensions.NVOcclusionQuery)
		{
			GLuint result;
			// retrieve result
			nglGetOcclusionQueryuivNV(ID, GL_PIXEL_COUNT_AVAILABLE_NV, &result);
			if (result != GL_FALSE)
			{
				nglGetOcclusionQueryuivNV(ID, GL_PIXEL_COUNT_NV, &result);
				OcclusionType = result != 0 ? NotOccluded : Occluded;
				VisibleCount = (uint) result;
				// Note : we could return the exact number of pixels that passed the z-test, but this value is not supported by all implementation (Direct3D ...)
			}
		}
		else
		{
			GLuint result;
			nglGetQueryObjectuivARB(ID, GL_QUERY_RESULT_AVAILABLE, &result);
			if (result != GL_FALSE)
			{
				nglGetQueryObjectuivARB(ID, GL_QUERY_RESULT, &result);
				OcclusionType = result != 0 ? NotOccluded : Occluded;
				VisibleCount = (uint) result;
			}
		}
	}
#endif
	return OcclusionType;
}

// ***************************************************************************
uint COcclusionQueryGL::getVisibleCount()
{
	H_AUTO_OGL(COcclusionQueryGL_getVisibleCount)
	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (getOcclusionType() == NotAvailable) return 0;
	return VisibleCount;
}

// ***************************************************************************
void CDriverGL::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGL_setDepthRange)
	_DriverGLStates.setDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL::getDepthRange(float &znear, float &zfar) const
{
	H_AUTO_OGL(CDriverGL_getDepthRange)
	_DriverGLStates.getDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGL_setCullMode)
	_DriverGLStates.setCullMode((CDriverGLStates::TCullMode) cullMode);
}

// ***************************************************************************
CDriverGL::TCullMode CDriverGL::getCullMode() const
{
	H_AUTO_OGL(CDriverGL_CDriverGL)
	return (CDriverGL::TCullMode) _DriverGLStates.getCullMode();
}

// ***************************************************************************
void CDriverGL::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGL_CDriverGL)
	_DriverGLStates.enableStencilTest(enable);
}

// ***************************************************************************
bool CDriverGL::isStencilTestEnabled() const
{
	H_AUTO_OGL(CDriverGL_CDriverGL)
	return _DriverGLStates.isStencilTestEnabled();
}

// ***************************************************************************
void CDriverGL::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

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
void CDriverGL::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

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
void CDriverGL::stencilMask(uint mask)
{
	H_AUTO_OGL(CDriverGL_CDriverGL)

	_DriverGLStates.stencilMask((GLuint)mask);
}

// ***************************************************************************
void CDriverGL::getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const
{
	lightedMaterial = inlGetNumTextStages();
	unlightedMaterial = inlGetNumTextStages();
}

// ***************************************************************************
void CDriverGL::beginDialogMode()
{
}

// ***************************************************************************
void CDriverGL::endDialogMode()
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

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
