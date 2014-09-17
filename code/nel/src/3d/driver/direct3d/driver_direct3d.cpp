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

#include "stddirect3d.h"

// by default, we disable the windows menu keys (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU

#include <tchar.h>

#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/dynloadlib.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

#define RASTERIZER D3DDEVTYPE_HAL
//#define RASTERIZER D3DDEVTYPE_REF

#define D3D_WINDOWED_STYLE (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX)
#define D3D_FULLSCREEN_STYLE (WS_POPUP)

// ***************************************************************************

// Try to allocate 16Mo by default of AGP Ram.
#define	NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE		(16384*1024)

// Initial volatile vertex buffer size
#define NL_VOLATILE_RAM_VB_SIZE	512*1024
#define NL_VOLATILE_AGP_VB_SIZE	128*1024
#define NL_VOLATILE_RAM_IB_SIZE	64*1024
#define NL_VOLATILE_AGP_IB_SIZE	1024
//
#define NL_VOLATILE_RAM_VB_MAXSIZE	512*1024
#define NL_VOLATILE_AGP_VB_MAXSIZE	2500*1024
#define NL_VOLATILE_RAM_IB_MAXSIZE	1024*1024
#define NL_VOLATILE_AGP_IB_MAXSIZE	256*1024



// ***************************************************************************

#ifndef NL_STATIC

HINSTANCE HInstDLL = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	HInstDLL = hinstDLL;

	return true;
}

class CDriverD3DNelLibrary : public INelLibrary {
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CDriverD3DNelLibrary)

#endif /* #ifndef NL_STATIC */

// ***************************************************************************

namespace NL3D
{

// ***************************************************************************

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32		CDriverD3D::ReleaseVersion = 0xe;



// ***************************************************************************

#ifdef NL_STATIC

#	pragma comment(lib, "d3dx9")
#	pragma comment(lib, "d3d9")
#	pragma comment(lib, "dinput8")
#	pragma comment(lib, "dxguid")

IDriver* createD3DDriverInstance ()
{
	return new CDriverD3D;
}

#else

#ifdef NL_COMP_MINGW
extern "C"
{
#endif
__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverD3D;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}
#ifdef NL_COMP_MINGW
}
#endif
#endif

/*static*/ bool CDriverD3D::_CacheTest[CacheTest_Count] =
{
	false, // CacheTest_CullMode = 0;
	false, // CacheTest_RenderState = 1,
	false, // CacheTest_TextureState = 2,
	false, // CacheTest_TextureIndexMode = 3,
	false, // CacheTest_TextureIndexUV = 4,
	false, // CacheTest_Texture = 5,
	false, // CacheTest_VertexProgram = 6,
	false, // CacheTest_PixelShader = 7,
	false, // CacheTest_VertexProgramConstant = 8,
	false, // CacheTest_PixelShaderConstant = 9,
	false, // CacheTest_SamplerState = 10,
	false, // CacheTest_VertexBuffer = 11,
	false, // CacheTest_IndexBuffer = 12,
	false, // CacheTest_VertexDecl = 13,
	false, // CacheTest_Matrix = 14,
	false, // CacheTest_RenderTarget = 15,
	false, // CacheTest_MaterialState = 16,
	false  // CacheTest_DepthRange = 17,
};




// ***************************************************************************

CDriverD3D::CDriverD3D()
{
	_SwapBufferCounter = 0;
	_CurrentOcclusionQuery = NULL;
	_D3D = NULL;
	_HWnd = NULL;
	_DeviceInterface = NULL;
	_DestroyWindow = false;
	_CurrentMode.Width = 640;
	_CurrentMode.Height = 480;
	_WindowX = 0;
	_WindowY = 0;
	_FullScreen = false;

	_ColorDepth = ColorDepth32;

	_DefaultCursor = EmptyCursor;

	_AlphaBlendedCursorSupported = false;
	_AlphaBlendedCursorSupportRetrieved = false;
	_CurrCol = CRGBA::White;
	_CurrRot = 0;
	_CurrHotSpotX = 0;
	_CurrHotSpotY = 0;
	_CursorScale = 1.f;

	_UserViewMtx.identity();
	_UserModelMtx.identity();
	_PZBCameraPos = CVector::Null;
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;
	_CurrentShader = NULL;
	_BackBuffer = NULL;
	_Maximized = false;
	_HandlePossibleSizeChangeNextSize = false;
	_Interval = 1;
	_AGPMemoryAllocated = 0;
	_VRAMMemoryAllocated = 0;
	_Rasterizer = RASTERIZER;
#ifdef NL_DISABLE_HARDWARE_VERTEX_PROGAM
	_DisableHardwareVertexProgram = true;
#else // NL_DISABLE_HARDWARE_VERTEX_PROGAM
	_DisableHardwareVertexProgram = false;
#endif // NL_DISABLE_HARDWARE_VERTEX_PROGAM
#ifdef NL_DISABLE_HARDWARE_PIXEL_PROGAM
	_DisableHardwarePixelProgram = true;
#else // NL_DISABLE_HARDWARE_PIXEL_PROGAM
	_DisableHardwarePixelProgram = false;
#endif // NL_DISABLE_HARDWARE_PIXEL_PROGAM
#ifdef NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
	_DisableHardwareVertexArrayAGP = true;
#else // NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
	_DisableHardwareVertexArrayAGP = false;
#endif // NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP
#ifdef NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
	_DisableHardwareIndexArrayAGP = true;
#else // NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
	_DisableHardwareIndexArrayAGP = false;
#endif // NL_DISABLE_HARDWARE_INDEX_ARRAY_AGP
#ifdef NL_DISABLE_HARDWARE_PIXEL_SHADER
	_DisableHardwarePixelShader = true;
#else // NL_DISABLE_HARDWARE_PIXEL_SHADER
	_DisableHardwarePixelShader = false;
#endif // NL_DISABLE_HARDWARE_PIXEL_SHADER

	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	uint i;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
	}

	// Default adapter
	_Adapter = 0xffffffff;
	_ModifiedRenderState = NULL;

	// Create a Direct 3d object
    if ( NULL == (_D3D = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		nlwarning ("Can't create the direct 3d 9 object.");

#ifdef NL_OS_WINDOWS
		sint val = MessageBoxA(NULL, "Your DirectX version is too old. You need to install the latest one.\r\n\r\nPressing OK will quit the game and automatically open your browser to download the latest version of DirectX.\r\nPress Cancel will just quit the game.\r\n", "Mtp Target Error", MB_OKCANCEL);
		if(val == IDOK)
		{
			openURL("http://www.microsoft.com/downloads/details.aspx?FamilyID=4b1f5d0c-5e44-4864-93cd-464ef59da050");
		}
#endif
		exit(EXIT_FAILURE);

	}
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	// default for lightmap
	_LightMapDynamicLightDirty= false;
	_LightMapDynamicLightEnabled= false;
	_CurrentMaterialSupportedShader= CMaterial::Normal;
	// to avoid any problem if light0 never setuped, and ligthmap rendered
	_UserLight0.setupDirectional(CRGBA::Black, CRGBA::White, CRGBA::White, CVector::K);
	// All User Light are disabled by Default
	for(i=0;i<MaxLight;i++)
		_UserLightEnable[i]= false;
	_CullMode = CCW;
	_ScissorTouched = true;
	_Scissor.X = -1;
	_Scissor.Y = -1;
	_Scissor.Width = -1;
	_Scissor.Height = -1;

	_CurStencilTest = false;
	_CurStencilFunc = D3DCMP_ALWAYS;
	_CurStencilRef = 0;
	_CurStencilMask = std::numeric_limits<DWORD>::max();
	_CurStencilOpFail = D3DSTENCILOP_KEEP;
	_CurStencilOpZFail = D3DSTENCILOP_KEEP;
	_CurStencilOpZPass = D3DSTENCILOP_KEEP;
	_CurStencilWriteMask = std::numeric_limits<DWORD>::max();


	for(uint k = 0; k < MaxTexture; ++k)
	{
		_CurrentUVRouting[k] = (uint8) k;
	}
	_VolatileVertexBufferRAM[0]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferRAM[1]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferAGP[0]	= new CVolatileVertexBuffer;
	_VolatileVertexBufferAGP[1]	= new CVolatileVertexBuffer;
	_VolatileIndexBuffer16RAM[0]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer16RAM[1]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer16AGP[0]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer16AGP[1]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer32RAM[0]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer32RAM[1]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer32AGP[0]= new CVolatileIndexBuffer;
	_VolatileIndexBuffer32AGP[1]= new CVolatileIndexBuffer;
	_MustRestoreLight = false;
	_Lost = false;
	_SceneBegun = false;
	_MaxVertexIndex = 0;
	_QuadIB = NULL;
	_MaxNumPerStageConstantLighted = 0;
	_MaxNumPerStageConstantUnlighted = 0;
	D3DXMatrixIdentity(&_D3DMatrixIdentity);
	_FogColor = 0xffffffff;
	_CurrIndexBufferFormat = CIndexBuffer::IndicesUnknownFormat;
	_IsGeforce = false;
	_NonPowerOfTwoTexturesSupported = false;
	_MaxAnisotropy = 0;
	_AnisotropicMinSupported = false;
	_AnisotropicMagSupported = false;
	_AnisotropicMinCubeSupported = false;
	_AnisotropicMagCubeSupported = false;

	_FrustumLeft= -1.f;
	_FrustumRight= 1.f;
	_FrustumTop= 1.f;
	_FrustumBottom= -1.f;
	_FrustumZNear= -1.f;
	_FrustumZFar= 1.f;
	_FrustumPerspective= false;
	_FogStart = 0;
	_FogEnd = 1;

	_SumTextureMemoryUsed = false;


	_DesktopGammaRampValid = false;
}

// ***************************************************************************

CDriverD3D::~CDriverD3D()
{
	release();

    if(_D3D != NULL)
	{
        _D3D->Release();
		_D3D = NULL;
	}
	delete _VolatileVertexBufferRAM[0];
	delete _VolatileVertexBufferRAM[1];
	delete _VolatileVertexBufferAGP[0];
	delete _VolatileVertexBufferAGP[1];
	delete _VolatileIndexBuffer16RAM[0];
	delete _VolatileIndexBuffer16RAM[1];
	delete _VolatileIndexBuffer16AGP[0];
	delete _VolatileIndexBuffer16AGP[1];
	delete _VolatileIndexBuffer32RAM[0];
	delete _VolatileIndexBuffer32RAM[1];
	delete _VolatileIndexBuffer32AGP[0];
	delete _VolatileIndexBuffer32AGP[1];
}

// ***************************************************************************

void CDriverD3D::resetRenderVariables()
{
	H_AUTO_D3D(CDriver3D_resetRenderVariables);

	uint i;
	for (i=0; i<MaxRenderState; i++)
	{
		if (_RenderStateCache[i].ValueSet)
		{
			// here, don't use 0xcccccccc, because it is a valid value for D3DRS_TFACTOR
			touchRenderVariable (&(_RenderStateCache[i]));
		}
	}

	for (i=0; i<MaxTexture; i++)
	{
		uint j;
		for (j=0; j<MaxTextureState; j++)
		{
			if (_TextureStateCache[i][j].Value != 0xcccccccc)
			{
				touchRenderVariable (&(_TextureStateCache[i][j]));
				_TextureStateCache[i][j].DeviceValue = 0xcccccccc;
			}
		}
	}
	for (i=0; i<MaxTexture; i++)
	{
		if (_TextureIndexStateCache[i].TexGenMode != 0xcccccccc)
			touchRenderVariable (&(_TextureIndexStateCache[i]));
	}
	for (i=0; i<MaxTexture; i++)
	{
		if ((uintptr_t)(_TexturePtrStateCache[i].Texture) != 0xcccccccc)
		{
			touchRenderVariable (&(_TexturePtrStateCache[i]));
			// reset texture because it may reference an old render target
			CTexturePtrState &textureState = (CTexturePtrState &)(_TexturePtrStateCache[i]);
			textureState.Texture = NULL;
		}
	}
	for (i=0; i<MaxSampler; i++)
	{
		uint j;
		for (j=0; j<MaxSamplerState; j++)
		{
			if (_SamplerStateCache[i][j].Value != 0xcccccccc)
				touchRenderVariable (&(_SamplerStateCache[i][j]));
		}
	}
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_VIEW)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE0)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE1)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE2)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE3)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE4)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE5)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE6)]));
	touchRenderVariable (&(_MatrixCache[remapMatrixIndex (D3DTS_TEXTURE7)]));


	// Vertices and indexes are not valid anymore
	_VertexBufferCache.VertexBuffer = NULL;
	_IndexBufferCache.IndexBuffer = NULL;
	_VertexDeclCache.Decl = NULL;

	touchRenderVariable (&(_VertexBufferCache));
	touchRenderVariable (&(_IndexBufferCache));
	touchRenderVariable (&(_VertexDeclCache));

	for (i=0; i<MaxLight; i++)
	{
		if (*(uintptr_t*)(&(_LightCache[i].Light)) != 0xcccccccc)
		{
			_LightCache[i].EnabledTouched = true;
			touchRenderVariable (&(_LightCache[i]));
		}
	}

	// Vertices and indexes are not valid anymore
	_VertexProgramCache.VertexProgram = NULL;
	_VertexProgramCache.VP = NULL;
	touchRenderVariable (&(_VertexProgramCache));
	_PixelShaderCache.PixelShader = NULL;
	touchRenderVariable (&(_PixelShaderCache));
	touchRenderVariable(&_MaterialState);

	for (i=0; i<MaxVertexProgramConstantState; i++)
	{
		touchRenderVariable (&(_VertexProgramConstantCache[i]));
	}
	for (i=0; i<MaxPixelShaderConstantState; i++)
	{
		touchRenderVariable (&(_PixelShaderConstantCache[i]));
	}
	setRenderTarget (NULL, 0, 0, 0, 0, 0, 0);

	CVertexBuffer::TLocation vertexAgpLocation = _DisableHardwareVertexArrayAGP ? CVertexBuffer::RAMResident : CVertexBuffer::AGPResident;
	CIndexBuffer::TLocation indexAgpLocation = _DisableHardwareIndexArrayAGP ? CIndexBuffer::RAMResident : CIndexBuffer::AGPResident;

	// Init volatile vertex buffers
	_VolatileVertexBufferRAM[0]->init (CVertexBuffer::RAMResident, _VolatileVertexBufferRAM[0]->Size, _VolatileVertexBufferRAM[0]->MaxSize, this);
	_VolatileVertexBufferRAM[0]->reset ();
	_VolatileVertexBufferRAM[1]->init (CVertexBuffer::RAMResident, _VolatileVertexBufferRAM[1]->Size, _VolatileVertexBufferRAM[1]->MaxSize, this);
	_VolatileVertexBufferRAM[1]->reset ();
	_VolatileVertexBufferAGP[0]->init (vertexAgpLocation, _VolatileVertexBufferAGP[0]->Size, _VolatileVertexBufferAGP[0]->MaxSize, this);
	_VolatileVertexBufferAGP[0]->reset ();
	_VolatileVertexBufferAGP[1]->init (vertexAgpLocation, _VolatileVertexBufferAGP[1]->Size, _VolatileVertexBufferAGP[1]->MaxSize, this);
	_VolatileVertexBufferAGP[1]->reset ();
	//
	_VolatileIndexBuffer16RAM[0]->init (CIndexBuffer::RAMResident, _VolatileIndexBuffer16RAM[0]->Size, _VolatileIndexBuffer16RAM[0]->MaxSize, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16RAM[0]->reset ();
	_VolatileIndexBuffer16RAM[1]->init (CIndexBuffer::RAMResident, _VolatileIndexBuffer16RAM[1]->Size, _VolatileIndexBuffer16RAM[1]->MaxSize, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16RAM[1]->reset ();
	_VolatileIndexBuffer16AGP[0]->init (indexAgpLocation, _VolatileIndexBuffer16AGP[0]->Size, _VolatileIndexBuffer16AGP[0]->MaxSize, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16AGP[0]->reset ();
	_VolatileIndexBuffer16AGP[1]->init (indexAgpLocation, _VolatileIndexBuffer16AGP[1]->Size, _VolatileIndexBuffer16AGP[1]->MaxSize, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16AGP[1]->reset ();
	//
	if (_MaxVertexIndex > 0xffff) // supports 32 bits ?
	{
		_VolatileIndexBuffer32RAM[0]->init (CIndexBuffer::RAMResident, _VolatileIndexBuffer32RAM[0]->Size, _VolatileIndexBuffer32RAM[0]->MaxSize, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32RAM[0]->reset ();
		_VolatileIndexBuffer32RAM[1]->init (CIndexBuffer::RAMResident, _VolatileIndexBuffer32RAM[1]->Size, _VolatileIndexBuffer32RAM[1]->MaxSize, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32RAM[1]->reset ();
		_VolatileIndexBuffer32AGP[0]->init (indexAgpLocation, _VolatileIndexBuffer32AGP[0]->Size, _VolatileIndexBuffer32AGP[0]->MaxSize, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32AGP[0]->reset ();
		_VolatileIndexBuffer32AGP[1]->init (indexAgpLocation, _VolatileIndexBuffer32AGP[1]->Size, _VolatileIndexBuffer32AGP[1]->MaxSize, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32AGP[1]->reset ();
	}
	_ScissorTouched = true;
}

// ***************************************************************************

void CDriverD3D::initRenderVariables()
{
	H_AUTO_D3D(CDriver3D_initRenderVariables);
	uint i;
	for (i=0; i<MaxRenderState; i++)
	{
		_RenderStateCache[i].StateID = (D3DRENDERSTATETYPE)i;
		_RenderStateCache[i].ValueSet = false;
		_RenderStateCache[i].Modified = false;
	}
	for (i=0; i<MaxTexture; i++)
	{
		uint j;
		for (j=0; j<MaxTextureState; j++)
		{
			_TextureStateCache[i][j].StageID = i;
			_TextureStateCache[i][j].StateID = (D3DTEXTURESTAGESTATETYPE)j;
			_TextureStateCache[i][j].Value = 0xcccccccc;
			_TextureStateCache[i][j].Modified = false;
		}
	}
	for (i=0; i<MaxTexture; i++)
	{
		_TextureIndexStateCache[i].StageID = i;
		_TextureIndexStateCache[i].TexGen = false;
		_TextureIndexStateCache[i].TexGenMode = 0xcccccccc;
		_TextureIndexStateCache[i].UVChannel = 0xcccccccc;
		_TextureIndexStateCache[i].Modified = false;
	}
	for (i=0; i<MaxTexture; i++)
	{
		_TexturePtrStateCache[i].StageID = i;
		*(uintptr_t*)&(_TexturePtrStateCache[i].Texture) = 0xcccccccc;
		_TexturePtrStateCache[i].Modified = false;
	}
	for (i=0; i<MaxSampler; i++)
	{
		uint j;
		for (j=0; j<MaxSamplerState; j++)
		{
			_SamplerStateCache[i][j].SamplerID = i;
			_SamplerStateCache[i][j].StateID = (D3DSAMPLERSTATETYPE)j;
			_SamplerStateCache[i][j].Value = 0xcccccccc;
			_SamplerStateCache[i][j].Modified = false;
		}
	}
	for (i=0; i<MaxMatrixState; i++)
	{
		_MatrixCache[i].TransformType = (D3DTRANSFORMSTATETYPE)((i>=MatrixStateRemap)?i-MatrixStateRemap+256:i);
		memset (&(_MatrixCache[i].Matrix), 0xcc, sizeof(D3DXMATRIX));
		_MatrixCache[i].Modified = false;
	}
	_VertexBufferCache.Modified = false;
	_VertexBufferCache.VertexBuffer = NULL;
	_IndexBufferCache.Modified = false;
	_IndexBufferCache.IndexBuffer = NULL;
	_VertexDeclCache.Modified = false;
	_VertexDeclCache.Decl = NULL;
	for (i=0; i<MaxLight; ++i)
	{
		_LightCache[i].LightIndex = uint8(i);
		*(uintptr_t*)&(_LightCache[i].Light) = 0xcccccccc;
		_LightCache[i].Modified = false;
	}
	_VertexProgramCache.Modified = false;
	_VertexProgramCache.VertexProgram = NULL;
	_PixelShaderCache.Modified = false;
	_PixelShaderCache.PixelShader = NULL;
	for (i=0; i<MaxVertexProgramConstantState; i++)
	{
		_VertexProgramConstantCache[i].StateID = i;
		_VertexProgramConstantCache[i].Modified = false;
		_VertexProgramConstantCache[i].ValueType = CVertexProgramConstantState::Undef;
	}
	for (i=0; i<MaxPixelShaderConstantState; i++)
	{
		_PixelShaderConstantCache[i].StateID = i;
		_PixelShaderConstantCache[i].Modified = false;
		_PixelShaderConstantCache[i].ValueType = CPixelShaderConstantState::Undef;
	}
	_RenderTarget.Modified = false;


	// Set the render states cache to its default values
	setRenderState (D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);
	setRenderState (D3DRS_CULLMODE, D3DCULL_CW);
	setRenderState (D3DRS_FOGENABLE, FALSE);

	// Stencil Buffer
	//affreux
	setRenderState (D3DRS_STENCILENABLE, _CurStencilTest);
	setRenderState (D3DRS_STENCILFUNC, _CurStencilFunc);
	setRenderState (D3DRS_STENCILREF, _CurStencilRef);
	setRenderState (D3DRS_STENCILMASK, _CurStencilMask);
	setRenderState (D3DRS_STENCILFAIL, _CurStencilOpFail);
	setRenderState (D3DRS_STENCILZFAIL, _CurStencilOpZFail);
	setRenderState (D3DRS_STENCILPASS, _CurStencilOpZPass);
	setRenderState (D3DRS_STENCILWRITEMASK, _CurStencilWriteMask);

	// Force normalize
	_ForceNormalize = false;
	_UseVertexColor = false;

	// Material
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;
	_CurrentShader = NULL;

	// Shaders
	initInternalShaders();

	// Fog default values
	_FogStart = 0;
	_FogEnd = 1;
	setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
	setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
	setRenderState (D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

	// Alpha render states
	setRenderState (D3DRS_ALPHATESTENABLE, FALSE);
	setRenderState (D3DRS_ALPHAREF, 128);
	setRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	// Cull mode
	_InvertCullMode = false;
	_DoubleSided = false;

	// Depth range
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;

	// Flush caches
	updateRenderVariablesInternal();
}

// ***************************************************************************

#define NL_SRC_OPERATORS_COUNT 6

// ***************************************************************************

static const D3DTEXTURESTAGESTATETYPE SrcOperators[NL_SRC_OPERATORS_COUNT]=
{
	D3DTSS_COLORARG0,
	D3DTSS_COLORARG1,
	D3DTSS_COLORARG2,
	D3DTSS_ALPHAARG0,
	D3DTSS_ALPHAARG1,
	D3DTSS_ALPHAARG2,
};

// ***************************************************************************

void CDriverD3D::updateRenderVariables()
{
	H_AUTO_D3D(CDriver3D_updateRenderVariables);
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;

	updateRenderVariablesInternal();
}

// ***************************************************************************
/*
inline void CDriverD3D::applyRenderVariable(CRenderVariable *currentRenderState)
{
	switch (currentRenderState->Type)
	{
	case CRenderVariable::RenderState:
		{
			CRenderState *renderState = static_cast<CRenderState*>(currentRenderState);
			_DeviceInterface->SetRenderState (renderState->StateID, renderState->Value);
		}
		break;
	case CRenderVariable::TextureState:
		{
			CTextureState *textureState = static_cast<CTextureState*>(currentRenderState);
			_DeviceInterface->SetTextureStageState (textureState->StageID, textureState->StateID, textureState->Value);
		}
		break;
	case CRenderVariable::TextureIndexState:
		{
			CTextureIndexState *textureState = static_cast<CTextureIndexState*>(currentRenderState);
			if (textureState->TexGen)
				setTextureState (textureState->StageID, D3DTSS_TEXCOORDINDEX, textureState->TexGenMode);
			else
				setTextureState (textureState->StageID, D3DTSS_TEXCOORDINDEX, textureState->UVChannel);
		}
		break;
	case CRenderVariable::TexturePtrState:
		{
			CTexturePtrState *textureState = static_cast<CTexturePtrState*>(currentRenderState);
			_DeviceInterface->SetTexture (textureState->StageID, textureState->Texture);
		}
		break;
	case CRenderVariable::VertexProgramPtrState:
		{
			CVertexProgramPtrState *vertexProgram = static_cast<CVertexProgramPtrState*>(currentRenderState);
			_DeviceInterface->SetVertexShader(vertexProgram->VertexProgram);
		}
		break;
	case CRenderVariable::PixelShaderPtrState:
		{
			CPixelShaderPtrState *pixelShader = static_cast<CPixelShaderPtrState*>(currentRenderState);
			_DeviceInterface->SetPixelShader(pixelShader->PixelShader);
		}
		break;
	case CRenderVariable::VertexProgramConstantState:
		{
			CVertexProgramConstantState *vertexProgramConstant = static_cast<CVertexProgramConstantState*>(currentRenderState);
			switch (vertexProgramConstant->ValueType)
			{
			case CVertexProgramConstantState::Float:
				_DeviceInterface->SetVertexShaderConstantF (vertexProgramConstant->StateID, (float*)vertexProgramConstant->Values, 1);
				break;
			case CVertexProgramConstantState::Int:
				_DeviceInterface->SetVertexShaderConstantI (vertexProgramConstant->StateID, (int*)vertexProgramConstant->Values, 1);
				break;
			}
		}
		break;
	case CRenderVariable::PixelShaderConstantState:
		{
			CPixelShaderConstantState *pixelShaderConstant = static_cast<CPixelShaderConstantState*>(currentRenderState);
			switch (pixelShaderConstant->ValueType)
			{
			case CPixelShaderConstantState::Float:
				_DeviceInterface->SetPixelShaderConstantF (pixelShaderConstant->StateID, (float*)pixelShaderConstant->Values, 1);
				break;
			case CPixelShaderConstantState::Int:
				_DeviceInterface->SetPixelShaderConstantI (pixelShaderConstant->StateID, (int*)pixelShaderConstant->Values, 1);
				break;
			}
		}
		break;
	case CRenderVariable::SamplerState:
		{
			CSamplerState *samplerState = static_cast<CSamplerState*>(currentRenderState);
			_DeviceInterface->SetSamplerState (samplerState->SamplerID, samplerState->StateID, samplerState->Value);
		}
		break;
	case CRenderVariable::MatrixState:
		{
			CMatrixState *renderMatrix = static_cast<CMatrixState*>(currentRenderState);
			_DeviceInterface->SetTransform (renderMatrix->TransformType, &(renderMatrix->Matrix));
		}
		break;
	case CRenderVariable::VBState:
		{
			CVBState *renderVB = static_cast<CVBState*>(currentRenderState);
			if (renderVB->VertexBuffer)
			{
				_DeviceInterface->SetStreamSource (0, renderVB->VertexBuffer, renderVB->Offset, renderVB->Stride);
			}
		}
		break;
	case CRenderVariable::IBState:
		{
			CIBState *renderIB = static_cast<CIBState*>(currentRenderState);
			if (renderIB->IndexBuffer)
			{
				_DeviceInterface->SetIndices (renderIB->IndexBuffer);
			}
		}
		break;
	case CRenderVariable::VertexDecl:
		{
			CVertexDeclState *renderVB = static_cast<CVertexDeclState*>(currentRenderState);
			if (renderVB->Decl)
			{
				_DeviceInterface->SetVertexDeclaration (renderVB->Decl);
			}
		}
		break;
	case CRenderVariable::LightState:
		{
			CLightState *renderLight = static_cast<CLightState*>(currentRenderState);

			// Enabel state modified ?
			if (renderLight->EnabledTouched)
				_DeviceInterface->LightEnable (renderLight->LightIndex, renderLight->Enabled);

			// Light enabled ?
			if (renderLight->Enabled)
			{
				if (renderLight->SettingsTouched)
				{
					// New position
					renderLight->Light.Position.x -= _PZBCameraPos.x;
					renderLight->Light.Position.y -= _PZBCameraPos.y;
					renderLight->Light.Position.z -= _PZBCameraPos.z;
					_DeviceInterface->SetLight (renderLight->LightIndex, &(renderLight->Light));
					renderLight->SettingsTouched = false;
				}
			}

			// Clean
			renderLight->EnabledTouched = false;
		}
		break;
	case CRenderVariable::RenderTargetState:
		{
			CRenderTargetState *renderTarget = static_cast<CRenderTargetState*>(currentRenderState);
			_DeviceInterface->SetRenderTarget (0, renderTarget->Target);
			setupViewport (_Viewport);
			setupScissor (_Scissor);
		}
		break;
	}
}*/




// ***************************************************************************
#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::replaceArgumentAtStage(D3DTEXTURESTAGESTATETYPE state, DWORD stage, DWORD from, DWORD to)
{
	if ((_TextureStateCache[stage][state].Value & D3DTA_SELECTMASK) == from)
	{
		setTextureState (stage, state, (_TextureStateCache[stage][state].Value&~D3DTA_SELECTMASK)|to);
	}
}

// ***************************************************************************
// Replace a constant with diffuse color at the given stage
#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::replaceAllRGBArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom)
{
	replaceArgumentAtStage(D3DTSS_COLORARG1, stage, from, to);
	if (_CurrentMaterialInfo->NumColorArg[stage] > 1)
	{
		replaceArgumentAtStage(D3DTSS_COLORARG2, stage, from, to);
		if (_CurrentMaterialInfo->NumColorArg[stage] > 2)
		{
			replaceArgumentAtStage(D3DTSS_COLORARG0, stage, from, to);
		}
	}
	//
	// Operator is D3DTOP_BLENDDIFFUSEALPHA ?
	if (_TextureStateCache[stage][D3DTSS_COLOROP].Value == blendOpFrom)
	{
		setTextureState (stage, D3DTSS_COLOROP, D3DTOP_LERP);
		setTextureState (stage, D3DTSS_COLORARG0, to|D3DTA_ALPHAREPLICATE);
	}

}

#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::replaceAllAlphaArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom)
{
	replaceArgumentAtStage(D3DTSS_ALPHAARG1, stage, from, to);
	if (_CurrentMaterialInfo->NumAlphaArg[stage] > 1)
	{
		replaceArgumentAtStage(D3DTSS_ALPHAARG2, stage, from, to);
		if (_CurrentMaterialInfo->NumAlphaArg[stage] > 2)
		{
			replaceArgumentAtStage(D3DTSS_ALPHAARG0, stage, from, to);
		}
	}
	if (_TextureStateCache[stage][D3DTSS_ALPHAOP].Value == blendOpFrom)
	{
		setTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_LERP);
		setTextureState (stage, D3DTSS_ALPHAARG0, to);
	}
}

#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::replaceAllArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom)
{
	if (_CurrentMaterialInfo->RGBPipe[stage])
	{
		replaceAllRGBArgumentAtStage(stage, from, to, blendOpFrom);
	}
	if (_CurrentMaterialInfo->AlphaPipe[stage])
	{
		replaceAllAlphaArgumentAtStage(stage, from, to, blendOpFrom);
	}
}

// ***************************************************************************
// Replace all argument at relevant stages with the given value
#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::replaceAllArgument(DWORD from, DWORD to, DWORD blendOpFrom)
{
	const uint maxTexture = inlGetNumTextStages();
	// Look for texture state
	for (uint i=0; i<maxTexture; i++)
	{
		if (_CurrentMaterialInfo->ColorOp[i] == D3DTOP_DISABLE) break;
		replaceAllArgumentAtStage(i, from, to, blendOpFrom);
	}
}

// ***************************************************************************
#ifdef NL_DEBUG
	inline
#endif
void CDriverD3D::setupConstantDiffuseColorFromLightedMaterial(D3DCOLOR color)
{
	for(uint i=1;i<_MaxLight;++i)
		enableLightInternal(uint8(i), false);
	_LightMapDynamicLightDirty= true;
	D3DMATERIAL9 d3dMat;
	setColor(d3dMat.Diffuse, 0.f, 0.f, 0.f, (1.f / 255.f) * (color >> 24));
	setColor(d3dMat.Ambient, 0.f, 0.f, 0.f, 0.f);
	setColor(d3dMat.Specular, 0.f, 0.f, 0.f, 0.f);
	setColor(d3dMat.Emissive, color);
	setMaterialState(d3dMat);
	setRenderState(D3DRS_LIGHTING, TRUE);
}


// ***************************************************************************
void CDriverD3D::updateRenderVariablesInternal()
{
	H_AUTO_D3D(CDriver3D_updateRenderVariablesInternal);
	nlassert (_DeviceInterface);
	bool aliasDiffuseToSpecular =  false;
	bool enableVertexColorFlag = true;
	if (_CurrentMaterialInfo && (_CurrentMaterialInfo->NeedsConstantForDiffuse || _CurrentMaterialInfo->MultipleConstantNoPixelShader)) /* The "unlighted without vertex color" trick */
	{
		// The material IS unlighted
		// No pixel shader ?
		if (_CurrentMaterialInfo->PixelShader)
		{
			/*
			 * We have to set the pixel shader now, because we have to choose between normal pixel shader and pixel shader without vertex color */
			// Must have two pixel shader
			nlassert (_CurrentMaterialInfo->PixelShaderUnlightedNoVertexColor);
			if (!_UseVertexColor && (_VertexProgramCache.VertexProgram == NULL))
			{
				setPixelShader (_CurrentMaterialInfo->PixelShaderUnlightedNoVertexColor);
			}
			else
			{
				setPixelShader (_CurrentMaterialInfo->PixelShader);
			}
		}
		else
		{
			setPixelShader (NULL);
			if (_CurrentMaterialInfo->NeedsConstantForDiffuse)
			{
				/*
				 * We have to change all texture state setuped to D3DTA_DIFFUSE into D3DTA_TFACTOR
				 * if we use a vertex buffer with diffuse color vertex with an unlighted material and no vertex program */
				if (_VertexProgramCache.VertexProgram)
				{
					// Diffuse should be output from vertex program
					// So we can only emulate 1 per stage constant (it has already been setup in CDriverD3D::setupMaterial)
					#ifdef NL_DEBUG
						nlassert(!_CurrentMaterialInfo->MultipleConstantNoPixelShader);
					#endif
				}
				else
				{
					if (!_UseVertexColor)
					{
						if (!_CurrentMaterialInfo->MultipleConstantNoPixelShader)
						{
							// Diffuse is used, but no other constant is used in the shader
							// Max texture
							replaceAllArgument(D3DTA_DIFFUSE, D3DTA_TFACTOR, D3DTOP_BLENDDIFFUSEALPHA);
							setRenderState (D3DRS_TEXTUREFACTOR, _CurrentMaterialInfo->UnlightedColor);
						}
						else
						{
							#ifdef NL_DEBUG
								nlassert(!_CurrentMaterialInfo->MultiplePerStageConstant); // Can't render this material on current hardware
							#endif
							//replaceAllArgumentAtStage(_CurrentMaterialInfo->ConstantIndex, D3DTA_TFACTOR, D3DTA_DIFFUSE, D3DTOP_BLENDFACTORALPHA);
							setupConstantDiffuseColorFromLightedMaterial(_CurrentMaterialInfo->UnlightedColor);
						}
					}
					else
					{
						if (_CurrentMaterialInfo->MultiplePerStageConstant)
						{
							// vertex color, 1st constant from CMaterial::getColor (already setuped in CDriverD3D::setupMaterial)
							//               2nd constant from a stage constant
							// Vertex color is aliased to the specular stream -> all references to D3DTA_DIFFUSE must be replaced with references to D3DTA_SPECULAR
							replaceAllArgument(D3DTA_DIFFUSE, D3DTA_SPECULAR, 0xffffffff);
							aliasDiffuseToSpecular = true;
							replaceAllArgumentAtStage(_CurrentMaterialInfo->ConstantIndex2, D3DTA_TFACTOR, D3DTA_DIFFUSE, D3DTOP_BLENDFACTORALPHA);
							setupConstantDiffuseColorFromLightedMaterial(NL_D3DCOLOR_RGBA(_CurrentMaterialInfo->Constant2)); // set 2nd per stage constant
							#ifdef NL_DEBUG
								nlassert(_VertexDeclCache.DeclAliasDiffuseToSpecular); // VB must not have specular used ... else this material can't render
							#endif
							// NB : currently this don't work with the GeForce2 (seems to be a driver bug).  Fortunately, this case isn't encountered with Ryzom materials :)
							// So it's provided for convenience.
						}
					}
				}
			}
			else
			{
				nlassert(_CurrentMaterialInfo->MultipleConstantNoPixelShader);
				// If vertex color is used, alias it to specular
				/*
				if (_UseVertexColor)
				{
					replaceAllArgument(D3DTA_DIFFUSE, D3DTA_SPECULAR, 0xffffffff);
					aliasDiffuseToSpecular = true; // VB must not have specular used ... else this material can't render
				}
				*/
				// up to 2 constants with no pixel shaders
				// look for constant at other stages and replaces then with diffuse color
				// first constant color has already been set yet (in CD3DDriver::setupMaterial)
				replaceAllArgumentAtStage(_CurrentMaterialInfo->ConstantIndex2, D3DTA_TFACTOR, D3DTA_DIFFUSE, D3DTOP_BLENDFACTORALPHA);
				setupConstantDiffuseColorFromLightedMaterial(NL_D3DCOLOR_RGBA(_CurrentMaterialInfo->Constant2)); // set 2nd per stage constant
			}
		}
	}
	else
	{
		if (_CurrentMaterialInfo) enableVertexColorFlag = _CurrentMaterialInfo->VertexColorLighted;
	}


	if (_NbNeLTextureStages == 3)
	{
		// Fix (one more...) for Radeon 7xxx
	// Don't know why, but the lighting is broken when MULTIPLYADD is used as in the lightmap shader..
	// Correct behaviour with GeForce & Ref. rasterizer...
	// The fix is to disable the light contribution from dynamic lights
		if (_TextureStateCache[0][D3DTSS_COLOROP].Value == D3DTOP_MULTIPLYADD &&
			_TextureStateCache[0][D3DTSS_COLORARG0].Value == D3DTA_DIFFUSE
		)
		{
			_TextureStateCache[0][D3DTSS_COLOROP].Value = D3DTOP_MODULATE;
			touchRenderVariable(&_TextureStateCache[0][D3DTSS_COLOROP]);
		}
		// fix for radeon 7xxx -> should enable vertex color only if really used in pixel pipe
		setEnableVertexColor(enableVertexColorFlag);
	}
	setAliasDiffuseToSpecular(aliasDiffuseToSpecular);


	// Flush all the modified render states
	while (_ModifiedRenderState)
	{
		// Current variable
		CRenderVariable	*currentRenderState = _ModifiedRenderState;

		// It is clean now
		currentRenderState->Modified = false;

		// Unlinked
		_ModifiedRenderState = currentRenderState->NextModified;
		currentRenderState->apply(this);
	}


	// Maybe it is a driver bug : in some situation with GeForce, I got vertex color set to (0, 0, 0, 0) with unlighted vertices and vertex color.
	// Forcing to resetup material solves the prb.. (though D3DRS_LIGHTING is set to FALSE ...)
	// I only have the prb with GeForce. The behaviour doesn't exhibit when using the D3D debug dll.
	if (_IsGeforce)
	{
		if (_RenderStateCache[D3DRS_LIGHTING].Value == FALSE && _VertexProgramCache.VertexProgram == NULL)
		{
			_MaterialState.apply(this);
		}
	}

}



// ***************************************************************************

void D3DWndProc(CDriverD3D *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_D3D(D3DWndProc);

	// Check this message in parents

	if ((message == WM_SIZE) || (message == WM_EXITSIZEMOVE) || (message == WM_MOVE))
	{
		if (driver != NULL)
		{
			// *** Check the hwnd is the root parent of driver->_HWnd

			// hwnd must be a _HWnd parent
			bool sameWindow = hWnd == driver->_HWnd;
			bool rootWindow = false;
			HWND tmp = driver->_HWnd;
			while (tmp)
			{
				if (tmp == hWnd)
				{
					rootWindow = true;
					break;
				}
				tmp = GetParent (tmp);
			}

			// hwnd must be a top window
			rootWindow &= (GetParent (hWnd) == NULL);

			// This is a root parent, not _HWnd
			if (rootWindow)
			{
				if (message == WM_SIZE)
				{
					if( SIZE_MAXIMIZED == wParam)
					{
						driver->_Maximized = true;
						if (sameWindow)
							driver->handlePossibleSizeChange();
						else
							driver->_HandlePossibleSizeChangeNextSize = true;
					}
					else if( SIZE_RESTORED == wParam)
					{
						if (driver->_Maximized)
						{
							driver->_Maximized = false;
							if (sameWindow)
								driver->handlePossibleSizeChange();
							else
								driver->_HandlePossibleSizeChangeNextSize = true;
						}
					}
				}
				else if(message == WM_EXITSIZEMOVE)
				{
					if (driver != NULL)
					{
						driver->handlePossibleSizeChange();
					}
				}
				else if(message == WM_MOVE)
				{
					if (driver != NULL)
					{
						RECT rect;
						GetWindowRect (hWnd, &rect);
						driver->_WindowX = rect.left;
						driver->_WindowY = rect.top;
					}
				}
			}
			// This is the window itself
			else if (sameWindow)
			{
				if (message == WM_SIZE)
				{
					if (driver->_HandlePossibleSizeChangeNextSize)
					{
						driver->handlePossibleSizeChange();
						driver->_HandlePossibleSizeChangeNextSize = false;
					}
				}
			}

		}
	}

	if (driver->_EventEmitter.getNumEmitters() > 0)
	{
		CWinEventEmitter *we = NLMISC::safe_cast<CWinEventEmitter *>(driver->_EventEmitter.getEmitter(0));
		// Process the message by the emitter
		we->setHWnd(hWnd);
		we->processMessage (hWnd, message, wParam, lParam);
	}
}

// ***************************************************************************

bool CDriverD3D::handlePossibleSizeChange()
{
	//DUMP_AUTO(handlePossibleSizeChange);
	H_AUTO_D3D(CDriver3D_handlePossibleSizeChange);
	// If windowed, check if the size as changed
	if (_CurrentMode.Windowed)
	{
		RECT rect;
		GetClientRect (_HWnd, &rect);

		// Setup d3d resolution
		uint16 newWidth = uint16(rect.right-rect.left);
		uint16 newHeight = uint16(rect.bottom-rect.top);

		// Set the new mode. Only change the size, keep the last setDisplay/setMode settings
		GfxMode mode = _CurrentMode;
		mode.Width = newWidth;
		mode.Height = newHeight;

		if ( ( (mode.Width != _CurrentMode.Width) || (mode.Height != _CurrentMode.Height) ) &&
			( mode.Width != 0 ) &&
			( mode.Height != 0 ) )
		{
			return reset (mode);
		}
	}
	return false;
}

// ***************************************************************************

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	H_AUTO_D3D(WndProc);
	// Get the driver pointer..
	CDriverD3D *pDriver=(CDriverD3D*)GetWindowLongPtrW (hWnd, GWLP_USERDATA);
	if (pDriver != NULL)
	{
		D3DWndProc (pDriver, hWnd, message, wParam, lParam);
	}

#ifdef NL_DISABLE_MENU
	// disable menu (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
	if(message == WM_SYSCOMMAND && wParam == SC_KEYMENU)
		return 0;
#endif // NL_DISABLE_MENU

	// ace: if we receive close, exit now or it'll assert after
	if(message == WM_CLOSE)
	{
		if(pDriver && pDriver->ExitFunc)
		{
			pDriver->ExitFunc();
		}
		else
		{
#ifndef NL_DISABLE_MENU
			// if we don't disable menu, alt F4 make a direct exit else we discard the message
			exit(0);
#endif // NL_DISABLE_MENU
		}
		return 0;
	}

	return DefWindowProcW(hWnd, message, wParam, lParam);
}

// ***************************************************************************

bool CDriverD3D::init (uintptr_t windowIcon, emptyProc exitFunc)
{
	H_AUTO_D3D(CDriver3D_init );

	ExitFunc = exitFunc;

	createCursors();

	// Register a window class
	WNDCLASSW		wc;

	memset(&wc,0,sizeof(wc));
	wc.style			= 0; // CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandleW(NULL);
	wc.hIcon			= (HICON)windowIcon;
	wc.hCursor			= _DefaultCursor;
	wc.hbrBackground	= WHITE_BRUSH;
	_WindowClass = "NLD3D" + toString(windowIcon);
	ucstring us = _WindowClass;
	wc.lpszClassName	= (LPCWSTR)us.c_str();
	wc.lpszMenuName		= NULL;
	if (!RegisterClassW(&wc))
	{
		DWORD error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
		{
			nlwarning("CDriverD3D::init: Can't register window class %s (error code %i)", _WindowClass.c_str(), (sint)error);
			_WindowClass = "";
			return false;
		}
	}

	return true;
}

// ***************************************************************************

// From the SDK
bool CDriverD3D::isDepthFormatOk(UINT adapter, D3DDEVTYPE rasterizer, D3DFORMAT DepthFormat,
                      D3DFORMAT AdapterFormat,
                      D3DFORMAT BackBufferFormat)
{
	H_AUTO_D3D(CDriverD3D_isDepthFormatOk);
    // Verify that the depth format exists
    HRESULT hr = _D3D->CheckDeviceFormat(adapter,
                                         rasterizer,
                                         AdapterFormat,
                                         D3DUSAGE_DEPTHSTENCIL,
                                         D3DRTYPE_SURFACE,
                                         DepthFormat);

    if(FAILED(hr)) return FALSE;

    // Verify that the depth format is compatible
    hr = _D3D->CheckDepthStencilMatch(adapter,
                                      (D3DDEVTYPE) rasterizer,
                                      AdapterFormat,
                                      BackBufferFormat,
                                      DepthFormat);

    return SUCCEEDED(hr);

}

// ***************************************************************************

const D3DFORMAT FinalPixelFormat[ITexture::UploadFormatCount][CDriverD3D::FinalPixelFormatChoice]=
{
	{ D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// Auto
	{ D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA8888
	{ D3DFMT_A4R4G4B4,	D3DFMT_A1R5G5B5,	D3DFMT_A8R3G3B2,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA4444
	{ D3DFMT_A1R5G5B5,	D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGBA5551
	{ D3DFMT_R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGB888
	{ D3DFMT_R5G6B5,	D3DFMT_A1R5G5B5,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// RGB565
	{ D3DFMT_DXT1,		D3DFMT_R3G3B2,		D3DFMT_A1R5G5B5,	D3DFMT_R5G6B5,		D3DFMT_A8R8G8B8 },	// DXTC1
	{ D3DFMT_DXT1,		D3DFMT_R3G3B2,		D3DFMT_A1R5G5B5,	D3DFMT_R5G6B5,		D3DFMT_A8R8G8B8 },	// DXTC1Alpha
	{ D3DFMT_DXT3,		D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DXTC3
	{ D3DFMT_DXT5,		D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DXTC5
	{ D3DFMT_L8,		D3DFMT_A8L8,		D3DFMT_R5G6B5,		D3DFMT_A1R5G5B5,	D3DFMT_A8R8G8B8 },	// Luminance
	{ D3DFMT_A8,		D3DFMT_A8L8,		D3DFMT_A8R3G3B2,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// Alpha
	{ D3DFMT_A8L8,		D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// AlphaLuminance
	{ D3DFMT_V8U8,		D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8,	D3DFMT_A8R8G8B8 },	// DsDt
};

// ***************************************************************************

bool CDriverD3D::setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable) throw(EBadDisplay)
{
	H_AUTO_D3D(CDriver3D_setDisplay);
	if (!_D3D)
		return false;
#ifndef NL_NO_ASM
	CFpuRestorer fpuRestorer;
#endif
	// Release the driver if already setuped
	release ();

	// Should be initialized
	nlassert (!_WindowClass.empty());

	// Should be released
	nlassert (_DeviceInterface == NULL);
	nlassert (_HWnd == NULL);

	// memorize desktop gamma ramp
	HDC dc = CreateDCW (L"DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		_DesktopGammaRampValid = GetDeviceGammaRamp (dc, _DesktopGammaRamp) != FALSE;
		// Release the DC
		ReleaseDC (NULL, dc);
	}

	// Create a window
	_HWnd = wnd;

	// Reset window state
	_Maximized = false;
	_HandlePossibleSizeChangeNextSize = false;

	if (_HWnd)
	{
		// We don't have to destroy this window
		_DestroyWindow = false;

		// Init Window Width and Height
		RECT clientRect;
		GetClientRect (_HWnd, &clientRect);
		_CurrentMode.OffScreen = false;
		_CurrentMode.Width = (uint16)(clientRect.right-clientRect.left);
		_CurrentMode.Height = (uint16)(clientRect.bottom-clientRect.top);
		_CurrentMode.Frequency = 0;
		_CurrentMode.Windowed = true;
		_CurrentMode.Depth = 32;
	}
	else
	{
		_CurrentMode = mode;

		// We have to destroy this window
		_DestroyWindow = true;

		// Window flags
		//ULONG WndFlags=(mode.Windowed?D3D_WINDOWED_STYLE:D3D_FULLSCREEN_STYLE)&~WS_VISIBLE;
		ULONG WndFlags;
		if(mode.Windowed)
		{
			WndFlags = D3D_WINDOWED_STYLE;
			if(!resizeable)
			{
				WndFlags &= ~(WS_THICKFRAME|WS_MAXIMIZEBOX);
			}
		}
		else
		{
			WndFlags = D3D_FULLSCREEN_STYLE;
			findNearestFullscreenVideoMode();
		}

		WndFlags &= ~WS_VISIBLE;

		// Window rect
		RECT	WndRect;
		WndRect.left=0;
		WndRect.top=0;
		WndRect.right=_CurrentMode.Width;
		WndRect.bottom=_CurrentMode.Height;
		AdjustWindowRect(&WndRect,WndFlags,FALSE);

		// Create
		ucstring ustr(_WindowClass);
		_HWnd = CreateWindowW((LPCWSTR)ustr.c_str(), L"", WndFlags, CW_USEDEFAULT,CW_USEDEFAULT, WndRect.right-WndRect.left,WndRect.bottom-WndRect.top, NULL, NULL,
			GetModuleHandleW(NULL), NULL);
		if (!_HWnd)
		{
			nlwarning ("CreateWindowW failed");
			release();
			return false;
		}

		// Set the window long integer
		SetWindowLongPtrW (_HWnd, GWLP_USERDATA, (LONG_PTR)this);

		// Show the window
		if (show || !_CurrentMode.Windowed)
			showWindow(true);
	}

	// Choose an adapter
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;

	// Get adapter format
    D3DDISPLAYMODE adapterMode;
	if (_D3D->GetAdapterDisplayMode (adapter, &adapterMode) != D3D_OK)
	{
		nlwarning ("GetAdapterDisplayMode failed");
		release();
		return false;
	}

	// The following code is taken from the NVPerfHud user guide
	// Set default settings
	_Rasterizer = RASTERIZER;
#ifdef NL_D3D_USE_NV_PERF_HUD
		// Look for 'NVIDIA NVPerfHUD' adapter
		// If it is present, override default settings
		for (UINT adapterIndex = 0; adapterIndex < _D3D->GetAdapterCount(); adapterIndex++)
		{
			D3DADAPTER_IDENTIFIER9 identifier;
			HRESULT res = _D3D->GetAdapterIdentifier(adapterIndex, 0, &identifier);
			if (strstr(identifier.Description, "PerfHUD") != 0)
			{
				adapter = adapterIndex;
				_Adapter = adapter;
				_Rasterizer = D3DDEVTYPE_REF;
				nlinfo("Using NVIDIA NVPerfHUD adapter");
				break;
			}
		}
#endif

	// Create device options
	D3DPRESENT_PARAMETERS parameters;
	D3DFORMAT adapterFormat;
	if (!fillPresentParameter (parameters, adapterFormat, _CurrentMode, adapterMode))
	{
		release();
		return false;
	}

	#if WITH_PERFHUD
		// Look for 'NVIDIA PerfHUD' adapter
		// If it is present, override default settings
		for (UINT gAdapter=0;gAdapter<_D3D->GetAdapterCount();gAdapter++)
		{
			D3DADAPTER_IDENTIFIER9 Identifier;
			HRESULT Res;
			Res = _D3D->GetAdapterIdentifier(gAdapter,0,&Identifier);

			if (strstr(Identifier.Description,"PerfHUD") != 0)
			{
				nlinfo ("Setting up with PerfHUD");
				adapter=gAdapter;
				_Rasterizer=D3DDEVTYPE_REF;
				break;
			}
		}
	#endif /* WITH_PERFHUD */
	// Create the D3D device
	HRESULT result = _D3D->CreateDevice (adapter, _Rasterizer, _HWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE, &parameters, &_DeviceInterface);
	if (result != D3D_OK)
	{
		nlwarning ("Can't create device hr:0x%x adap:0x%x rast:0x%x", result, adapter, _Rasterizer);

		// Create the D3D device without puredevice
		HRESULT result = _D3D->CreateDevice (adapter, _Rasterizer, _HWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &parameters, &_DeviceInterface);
		if (result != D3D_OK)
		{
			nlwarning ("Can't create device without puredevice hr:0x%x adap:0x%x rast:0x%x", result, adapter, _Rasterizer);

			// Create the D3D device without puredevice and hardware
			HRESULT result = _D3D->CreateDevice (adapter, _Rasterizer, _HWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &_DeviceInterface);
			if (result != D3D_OK)
			{
				nlwarning ("Can't create device without puredevice and hardware hr:0x%x adap:0x%x rast:0x%x", result, adapter, _Rasterizer);
				release();
				return false;
			}
		}
	}

	
//	_D3D->CreateDevice (adapter, _Rasterizer, _HWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &_DeviceInterface);

	// Check some caps
	D3DCAPS9 caps;
	if (_DeviceInterface->GetDeviceCaps(&caps) == D3D_OK)
	{
		_TextureCubeSupported = (caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) != 0;
		_NbNeLTextureStages = (caps.MaxSimultaneousTextures<IDRV_MAT_MAXTEXTURES)?caps.MaxSimultaneousTextures:IDRV_MAT_MAXTEXTURES;
		_MADOperatorSupported = (caps.TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD) != 0;
		_EMBMSupported = (caps.TextureOpCaps &  D3DTOP_BUMPENVMAP) != 0;
		_PixelShaderVersion = caps.PixelShaderVersion;
		_CubbedMipMapSupported = (caps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP) != 0;
		_MaxPrimitiveCount = caps.MaxPrimitiveCount;
		_MaxVertexIndex = caps.MaxVertexIndex;
		_IsGeforce = !(caps.DevCaps & D3DDEVCAPS_NPATCHES) && (caps.PixelShaderVersion >= D3DPS_VERSION(2, 0) || caps.PixelShaderVersion < D3DPS_VERSION(1, 4));
		_NonPowerOfTwoTexturesSupported = !(caps.TextureCaps & D3DPTEXTURECAPS_POW2) || (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL);
		_MaxAnisotropy = caps.MaxAnisotropy;
		_AnisotropicMinSupported = (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0;
		_AnisotropicMagSupported = (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0;
		_AnisotropicMinCubeSupported = (caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0;
		_AnisotropicMagCubeSupported = (caps.CubeTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0;
	}
	else
	{
		_TextureCubeSupported = false;
		_NbNeLTextureStages = 1;
		_MADOperatorSupported = false;
		_EMBMSupported = false;
		_CubbedMipMapSupported = false;
		_PixelShaderVersion = 0;
		_MaxPrimitiveCount = 0xffff;
		_MaxVertexIndex = 0xffff;
		_IsGeforce = false;
		_NonPowerOfTwoTexturesSupported = false;
		_MaxAnisotropy = 0;
		_AnisotropicMinSupported = false;
		_AnisotropicMagSupported = false;
		_AnisotropicMinCubeSupported = false;
		_AnisotropicMagCubeSupported = false;
	}
	// If 16 bits vertices only, build a vb for quads rendering
	if (_MaxVertexIndex <= 0xffff)
	{
		if (!buildQuadIndexBuffer()) return false;
	}

	// test for occlusion query support
	IDirect3DQuery9 *dummyQuery = NULL;
	if (_DeviceInterface->CreateQuery(D3DQUERYTYPE_OCCLUSION, &dummyQuery) == D3DERR_NOTAVAILABLE)
	{
		_OcclusionQuerySupported = false;
	}
	else
	{
		_OcclusionQuerySupported = true;
		if (dummyQuery) dummyQuery->Release();
	}


#ifdef NL_FORCE_TEXTURE_STAGE_COUNT
	_NbNeLTextureStages = min ((uint)NL_FORCE_TEXTURE_STAGE_COUNT, (uint)IDRV_MAT_MAXTEXTURES);
#endif // NL_FORCE_TEXTURE_STAGE_COUNT

	_VertexProgram = !_DisableHardwareVertexProgram && ((caps.VertexShaderVersion&0xffff) >= 0x0100);
	_PixelProgramVersion = _DisableHardwareVertexProgram ? 0x0000 : caps.PixelShaderVersion & 0xffff;
	nldebug("Pixel Program Version: %i.%i", (uint32)((_PixelProgramVersion & 0xFF00) >> 8), (uint32)(_PixelProgramVersion & 0xFF));
	_PixelProgram = _PixelProgramVersion >= 0x0101;
	_MaxVerticesByVertexBufferHard = caps.MaxVertexIndex;
	_MaxLight = caps.MaxActiveLights;

	if(_MaxLight > 0xFF) _MaxLight = 3;

	if (_PixelProgram)
	{
		_MaxNumPerStageConstantLighted = _NbNeLTextureStages;
		_MaxNumPerStageConstantUnlighted = _NbNeLTextureStages;
	}
	else
	{
		// emulation of per stage constant through diffuse
		_MaxNumPerStageConstantLighted = 1;
		_MaxNumPerStageConstantUnlighted = 2;
	}

	if (_DisableHardwarePixelShader && _NbNeLTextureStages > 3) // yes, 3 is not a bug
	{
		// If pixel shader are disabled, then can't emulate the texEnvColor feature with more than 2 stages. (only 2 constant available by using material emissive in addition to the texture factor)
		// Radeon with 3 stages cases : let the third stage to ensure availability of the EMBM feature
		// There is a special fix in CMaterial::isSupportedByDriver to force the number of stages to 2 for the radeons
		_NbNeLTextureStages = 2;
	}

	// *** Check textures caps
	uint j;
	uint i;
	for (i=0; i<ITexture::UploadFormatCount; i++)
	{
		// Default format : D3DFMT_A8R8G8B8
		_PreferedTextureFormat[i] = D3DFMT_A8R8G8B8;

		for (j=0; j<CDriverD3D::FinalPixelFormatChoice; j++)
		{
			if (isTextureFormatOk(adapter, _Rasterizer, FinalPixelFormat[i][j], adapterFormat))
				break;
		}

		// Set the prefered pixel format
		if (j<CDriverD3D::FinalPixelFormatChoice)
			_PreferedTextureFormat[i] = FinalPixelFormat[i][j];
	}

	// Reset render state cache
	initRenderVariables();

	// *** Event init

	// Release old emitter
	while (_EventEmitter.getNumEmitters() != 0)
	{
		_EventEmitter.removeEmitter(_EventEmitter.getEmitter(_EventEmitter.getNumEmitters() - 1));
	}
	NLMISC::CWinEventEmitter *we = new NLMISC::CWinEventEmitter;

	// Setup the event emitter, and try to retrieve a direct input interface
	_EventEmitter.addEmitter(we, true /*must delete*/); // the main emitter

	// Init some variables
	_ForceDXTCCompression = false;
	_AnisotropicFilter = 0;
	_ForceTextureResizePower = 0;
	_FogEnabled = false;

	// No back buffer backuped for the moment
	_BackBuffer = NULL;

	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;
	_VBHardProfiling= false;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
	_IBProfiling= false;
	_CurIBLockCount= 0;
	_NumIBProfileFrame= 0;

	// try to allocate 16Mo by default of AGP Ram.
	initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

	// If AGP is less than 16 mo, try to keep the max in proportion
	float maxAGPbufferSizeRatio = (float) _AGPMemoryAllocated / (float) NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE;

	// Init volatile vertex buffers
	_CurrentRenderPass = 0;
	_VolatileVertexBufferRAM[0]->init (CVertexBuffer::RAMResident, NL_VOLATILE_RAM_VB_SIZE, NL_VOLATILE_RAM_VB_MAXSIZE, this);
	_VolatileVertexBufferRAM[0]->reset ();
	_VolatileVertexBufferRAM[1]->init (CVertexBuffer::RAMResident, NL_VOLATILE_RAM_VB_SIZE, NL_VOLATILE_RAM_VB_MAXSIZE, this);
	_VolatileVertexBufferRAM[1]->reset ();
	_VolatileVertexBufferAGP[0]->init (CVertexBuffer::AGPResident, NL_VOLATILE_AGP_VB_SIZE, (uint) (NL_VOLATILE_AGP_VB_MAXSIZE * maxAGPbufferSizeRatio), this);
	_VolatileVertexBufferAGP[0]->reset ();
	_VolatileVertexBufferAGP[1]->init (CVertexBuffer::AGPResident, NL_VOLATILE_AGP_VB_SIZE, (uint) (NL_VOLATILE_AGP_VB_MAXSIZE * maxAGPbufferSizeRatio), this);
	_VolatileVertexBufferAGP[1]->reset ();
	//
	_VolatileIndexBuffer16RAM[0]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16RAM[0]->reset ();
	_VolatileIndexBuffer16RAM[1]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16RAM[1]->reset ();
	_VolatileIndexBuffer16AGP[0]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, (uint) (NL_VOLATILE_AGP_IB_MAXSIZE * maxAGPbufferSizeRatio), this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16AGP[0]->reset ();
	_VolatileIndexBuffer16AGP[1]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, (uint) (NL_VOLATILE_AGP_IB_MAXSIZE * maxAGPbufferSizeRatio), this, CIndexBuffer::Indices16);
	_VolatileIndexBuffer16AGP[1]->reset ();
	// 32 bits indices supported
	if (_MaxVertexIndex > 0xffff)
	{
		_VolatileIndexBuffer32RAM[0]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32RAM[0]->reset ();
		_VolatileIndexBuffer32RAM[1]->init (CIndexBuffer::RAMResident, NL_VOLATILE_RAM_IB_SIZE, NL_VOLATILE_RAM_IB_MAXSIZE, this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32RAM[1]->reset ();
		_VolatileIndexBuffer32AGP[0]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, (uint) (NL_VOLATILE_AGP_IB_MAXSIZE * maxAGPbufferSizeRatio), this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32AGP[0]->reset ();
		_VolatileIndexBuffer32AGP[1]->init (CIndexBuffer::AGPResident, NL_VOLATILE_AGP_IB_SIZE, (uint) (NL_VOLATILE_AGP_IB_MAXSIZE * maxAGPbufferSizeRatio), this, CIndexBuffer::Indices32);
		_VolatileIndexBuffer32AGP[1]->reset ();
	}
	//
	setupViewport (CViewport());



	// Begin now
	//nldebug("BeginScene");
	if (!beginScene()) return false;

	// Done
	return true;
}

// ***************************************************************************
extern uint indexCount;
extern uint vertexCount;

bool CDriverD3D::release()
{
	H_AUTO_D3D(CDriver3D_release);
	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	ItShaderDrvInfoPtrList		itshd;
	while( (itshd = _ShaderDrvInfos.begin()) != _ShaderDrvInfos.end() )
	{
		// NB: at IShader deletion, this->_MatDrvInfos is updated (entry deleted);
		delete *itshd;
	}

	_SwapBufferCounter = 0;

	if (_QuadIB)
	{
		_QuadIB->Release();
		_QuadIB = NULL;
	}

	// delete queries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	// Back buffer ref
	if (_BackBuffer)
		_BackBuffer->Release();
	_BackBuffer = NULL;

	// Release all the vertex declaration
	std::list<CVertexDeclaration>::iterator ite = _VertexDeclarationList.begin();
	while (ite != _VertexDeclarationList.end())
	{
		ite->VertexDecl->Release();
		ite++;
	}
	_VertexDeclarationList.clear ();

	// Release pixel shaders
	_NormalPixelShaders[0].clear ();
	_NormalPixelShaders[1].clear ();

    if( _DeviceInterface != NULL)
	{
        _DeviceInterface->Release();
		_DeviceInterface = NULL;
	}

	if (_HWnd)
	{
		releaseCursors();

		// make sure window icons are deleted
		std::vector<NLMISC::CBitmap> bitmaps;
		setWindowIcon(bitmaps);

		if (_DestroyWindow)
			DestroyWindow (_HWnd);
		_HWnd = NULL;
	}


	nlassert (indexCount == 0);
	nlassert (vertexCount == 0);

	// restore desktop gamma ramp
	if (_DesktopGammaRampValid)
	{
		HDC dc = CreateDCW (L"DISPLAY", NULL, NULL, NULL);
		if (dc)
		{
			SetDeviceGammaRamp (dc, _DesktopGammaRamp);
		}
		_DesktopGammaRampValid = false;
	}
	return true;
};

// ***************************************************************************

emptyProc CDriverD3D::getWindowProc()
{
	return (emptyProc)D3DWndProc;
};

// ***************************************************************************

IDriver::TMessageBoxId CDriverD3D::systemMessageBox (const char* message, const char* title, TMessageBoxType type, TMessageBoxIcon icon)
{
	switch (::MessageBox (_HWnd, message, title, ((type==retryCancelType)?MB_RETRYCANCEL:
		(type==yesNoCancelType)?MB_YESNOCANCEL:
		(type==okCancelType)?MB_OKCANCEL:
		(type==abortRetryIgnoreType)?MB_ABORTRETRYIGNORE:
		(type==yesNoType)?MB_YESNO|MB_ICONQUESTION:MB_OK)|

		((icon==handIcon)?MB_ICONHAND:
		(icon==questionIcon)?MB_ICONQUESTION:
		(icon==exclamationIcon)?MB_ICONEXCLAMATION:
		(icon==asteriskIcon)?MB_ICONASTERISK:
		(icon==warningIcon)?MB_ICONWARNING:
		(icon==errorIcon)?MB_ICONERROR:
		(icon==informationIcon)?MB_ICONINFORMATION:
		(icon==stopIcon)?MB_ICONSTOP:0)))
	{
		case IDOK:
			return okId;
		case IDCANCEL:
			return cancelId;
		case IDABORT:
			return abortId;
		case IDRETRY:
			return retryId;
		case IDIGNORE:
			return ignoreId;
		case IDYES:
			return yesId;
		case IDNO:
			return noId;
	}
	return okId;
}

// ***************************************************************************
bool CDriverD3D::activate()
{
	return true;
}

// ***************************************************************************
bool CDriverD3D::isActive ()
{
	return (IsWindow(_HWnd) != 0);
}

// ***************************************************************************
nlWindow CDriverD3D::getDisplay()
{
	return _HWnd;
}

// ***************************************************************************
NLMISC::IEventEmitter	*CDriverD3D::getEventEmitter()
{
	return &_EventEmitter;
}

// ***************************************************************************
void CDriverD3D::getWindowSize (uint32 &width, uint32 &height)
{
	H_AUTO_D3D(CDriverD3D_getWindowSize);
	width = _CurrentMode.Width;
	height = _CurrentMode.Height;
}

// ***************************************************************************

void CDriverD3D::getWindowPos (sint32 &x, sint32 &y)
{
	H_AUTO_D3D(CDriverD3D_getWindowPos);
	x = _WindowX;
	y = _WindowY;
}

// ***************************************************************************
uint32 CDriverD3D::getImplementationVersion () const
{
	H_AUTO_D3D(CDriverD3D_getImplementationVersion);
	return ReleaseVersion;
}

// ***************************************************************************
const char *CDriverD3D::getDriverInformation ()
{
	return "Directx 9 NeL Driver";
}

// ***************************************************************************
uint8 CDriverD3D::getBitPerPixel ()
{
	return _CurrentMode.Depth;
}

// ***************************************************************************
bool CDriverD3D::clear2D(CRGBA rgba)
{
	H_AUTO_D3D(CDriverD3D_clear2D);
	nlassert (_DeviceInterface);

	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_TARGET, NL_D3DCOLOR_RGBA(rgba), 1.0f, 0 ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);
	return result;
}

// ***************************************************************************

bool CDriverD3D::clearZBuffer(float zval)
{
	H_AUTO_D3D(CDriverD3D_clearZBuffer);
	nlassert (_DeviceInterface);

	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,0,0,0), zval, 0 ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);

	// NVidia driver 56.72 needs to reset the vertex buffer after a clear Z
	touchRenderVariable (&_VertexBufferCache);

	return result;
}

// ***************************************************************************

bool CDriverD3D::clearStencilBuffer(float stencilval)
{
	H_AUTO_D3D(CDriverD3D_clearStencilBuffer);
	nlassert (_DeviceInterface);

	// Backup viewport
	CViewport oldViewport = _Viewport;
	setupViewport (CViewport());
	updateRenderVariables ();

	bool result = _DeviceInterface->Clear( 0, NULL, D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0,0,0,0), 1.0f, (unsigned long)stencilval ) == D3D_OK;

	// Restaure the old viewport
	setupViewport (oldViewport);

	return result;
}

// ***************************************************************************

void CDriverD3D::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	H_AUTO_D3D(CDriverD3D_setColorMask);
	setRenderState (D3DRS_COLORWRITEENABLE,
		(bAlpha?D3DCOLORWRITEENABLE_ALPHA:0)|
		(bRed?D3DCOLORWRITEENABLE_RED:0)|
		(bGreen?D3DCOLORWRITEENABLE_GREEN:0)|
		(bBlue?D3DCOLORWRITEENABLE_BLUE:0)
		);
}




// ***************************************************************************
bool CDriverD3D::swapBuffers()
{
	//DUMP_AUTO(swapBuffers);
	H_AUTO_D3D(CDriverD3D_swapBuffers);
	nlassert (_DeviceInterface);

	++ _SwapBufferCounter;
	// Swap & reset volatile buffers
	_CurrentRenderPass++;
	_VolatileVertexBufferRAM[_CurrentRenderPass&1]->reset ();
	_VolatileVertexBufferAGP[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBuffer16RAM[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBuffer16AGP[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBuffer32RAM[_CurrentRenderPass&1]->reset ();
	_VolatileIndexBuffer32AGP[_CurrentRenderPass&1]->reset ();

	// todo hulud volatile
	//_DeviceInterface->SetStreamSource(0, _VolatileVertexBufferRAM[1]->VertexBuffer, 0, 12);

	// End now
	if (!endScene())
	{
		nlstop;
		return false;
	}

	HRESULT result = _DeviceInterface->Present( NULL, NULL, NULL, NULL);
	if (result != D3D_OK)
	{
		// Device lost ?
		if (result == D3DERR_DEVICELOST)
		{
			_Lost = true;
			// check if we can exit lost state
			if (_DeviceInterface->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			{
				// Reset the driver
				reset (_CurrentMode);
			}
		}
	}

	// Check window size
	handlePossibleSizeChange ();

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset vertex program
	setVertexProgram (NULL, NULL);
	// Reset pixel program
	setPixelShader (NULL);

	if (_VBHardProfiling)
	{
		++_NumVBHardProfileFrame;
	}
	if (_IBProfiling)
	{
		++ _NumIBProfileFrame;
	}

	// Begin now
	return beginScene();
};

// ***************************************************************************

void CDriverD3D::setPolygonMode (TPolygonMode mode)
{
	H_AUTO_D3D(CDriverD3D_setPolygonMode);
	IDriver::setPolygonMode (mode);

	setRenderState (D3DRS_FILLMODE, mode==Point?D3DFILL_POINT:mode==Line?D3DFILL_WIREFRAME:D3DFILL_SOLID);
}

// ***************************************************************************

bool CDriverD3D::isTextureFormatOk(UINT adapter, D3DDEVTYPE rasterizer, D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat)
{
	H_AUTO_D3D(CDriverD3D_isTextureFormatOk);
    HRESULT hr = _D3D->CheckDeviceFormat( adapter,
                                          rasterizer,
                                          AdapterFormat,
                                          0,
                                          D3DRTYPE_TEXTURE,
                                          TextureFormat);

    return SUCCEEDED( hr ) != FALSE;
}

// ***************************************************************************

void CDriverD3D::forceDXTCCompression(bool dxtcComp)
{
	H_AUTO_D3D(CDriverD3D_forceDXTCCompression);
	_ForceDXTCCompression = dxtcComp;
}

// ***************************************************************************

void CDriverD3D::setAnisotropicFilter(sint filter)
{
	H_AUTO_D3D(CDriverD3D_setAnisotropicFilter);

	// anisotropic filter not supported
	if (_MaxAnisotropy < 2) return;

	if (filter < 0 || filter > _MaxAnisotropy)
	{
		_AnisotropicFilter = _MaxAnisotropy;
	}
	else
	{
		_AnisotropicFilter = filter;
	}
}

// ***************************************************************************

void CDriverD3D::forceTextureResize(uint divisor)
{
	H_AUTO_D3D(CDriverD3D_forceTextureResize);
	clamp(divisor, 1U, 256U);

	// 16 -> 4.
	_ForceTextureResizePower= getPowerOf2(divisor);
}

// ***************************************************************************

bool CDriverD3D::fogEnabled()
{
	H_AUTO_D3D(CDriverD3D_fogEnabled);
	// Return _RenderStateCache[D3DRS_FOGENABLE].Value == TRUE;
	// Nico Patch : must return the _FogEnabled value here, because it MAY be
	// different of the one found in _RenderStateCache[D3DRS_FOGENABLE]
	// this happens for example when dest blend is one : the actual content of _RenderStateCache[D3DRS_FOGENABLE]
	// is then restored when current material alpha blending settings are modified, or when a new material is set.
	return _FogEnabled;
}

// ***************************************************************************

void CDriverD3D::enableFog(bool enable)
{
	H_AUTO_D3D(CDriverD3D_enableFog);
	_FogEnabled = enable;
	setRenderState (D3DRS_FOGENABLE, enable?TRUE:FALSE);
}

// ***************************************************************************

void CDriverD3D::setupFog(float start, float end, CRGBA color)
{
	H_AUTO_D3D(CDriverD3D_setupFog);
	// Remember fog start and end
	_FogStart = start;
	_FogEnd = end;
	_FogColor = NL_D3DCOLOR_RGBA(color);

	// Set the fog
	setRenderState (D3DRS_FOGCOLOR, _FogColor);
	setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
	setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
}

// ***************************************************************************

float CDriverD3D::getFogStart() const
{
	return _FogStart;
}

// ***************************************************************************

float CDriverD3D::getFogEnd() const
{
	return _FogEnd;
}

// ***************************************************************************

CRGBA CDriverD3D::getFogColor() const
{
	return D3DCOLOR_NL_RGBA(_RenderStateCache[D3DRS_FOGCOLOR].Value);
}

// ***************************************************************************

CVertexBuffer::TVertexColorType CDriverD3D::getVertexColorFormat() const
{
	return CVertexBuffer::TBGRA;
}

// ***************************************************************************

uint CDriverD3D::getNbTextureStages() const
{
	return _NbNeLTextureStages;
}

// ***************************************************************************

bool CDriverD3D::getModes(std::vector<GfxMode> &modes)
{
	H_AUTO_D3D(CDriverD3D_getModes);
	static const D3DFORMAT format[]=
	{
		D3DFMT_A8R8G8B8,
		D3DFMT_X8R8G8B8,
		D3DFMT_R8G8B8,
		D3DFMT_A1R5G5B5,
		D3DFMT_R5G6B5,
	};
	static const uint8 depth[]=
	{
		32,
		32,
		24,
		16,
		16,
	};
	uint f;
	for (f=0; f<sizeof(format)/sizeof(uint); f++)
	{
		UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
		const uint count = _D3D->GetAdapterModeCount(adapter, format[f]);
		uint i;
		for (i=0; i<count; i++)
		{
			D3DDISPLAYMODE mode;
			if (_D3D->EnumAdapterModes( adapter, format[f], i, &mode) == D3D_OK)
			{
				GfxMode gfxMode;
				gfxMode.Windowed=false;
				gfxMode.Width=(uint16)mode.Width;
				gfxMode.Height=(uint16)mode.Height;
				gfxMode.Depth=depth[f];
				gfxMode.Frequency=(uint8)mode.RefreshRate;
				modes.push_back (gfxMode);
			}
		}
	}
	return true;
}

// ***************************************************************************
bool CDriverD3D::getCurrentScreenMode(GfxMode &gfxMode)
{
	H_AUTO_D3D(CDriverD3D_getCurrentScreenMode);
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	D3DDISPLAYMODE mode;
	_D3D->GetAdapterDisplayMode(adapter, &mode);
	gfxMode.Windowed = !_FullScreen;
	gfxMode.Width = (uint16)mode.Width;
	gfxMode.Height = (uint16)mode.Height;
	gfxMode.Depth = ((mode.Format==D3DFMT_A8R8G8B8)||(mode.Format==D3DFMT_X8R8G8B8))?32:16;
	gfxMode.Frequency = (uint8)mode.RefreshRate;

	return true;
}

// ***************************************************************************
void CDriverD3D::setWindowTitle(const ucstring &title)
{
	SetWindowTextW(_HWnd,(WCHAR*)title.c_str());
}

// ***************************************************************************
void CDriverD3D::setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps)
{
	if (!_HWnd)
		return;

	static HICON winIconBig = NULL;
	static HICON winIconSmall = NULL;

	if (winIconBig)
	{
		DestroyIcon(winIconBig);
		winIconBig = NULL;
	}

	if (winIconSmall)
	{
		DestroyIcon(winIconSmall);
		winIconSmall = NULL;
	}

	sint smallIndex = -1;
	uint smallWidth = GetSystemMetrics(SM_CXSMICON);
	uint smallHeight = GetSystemMetrics(SM_CYSMICON);

	sint bigIndex = -1;
	uint bigWidth = GetSystemMetrics(SM_CXICON);
	uint bigHeight = GetSystemMetrics(SM_CYICON);

	// find icons with the exact size
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 &&	bitmaps[i].getWidth() == smallWidth &&	bitmaps[i].getHeight() == smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() == bigWidth && bitmaps[i].getHeight() == bigHeight)
			bigIndex = i;
	}

	// find icons with taller size (we will resize them)
	for(uint i = 0; i < bitmaps.size(); ++i)
	{
		if (smallIndex == -1 && bitmaps[i].getWidth() >= smallWidth && bitmaps[i].getHeight() >= smallHeight)
			smallIndex = i;

		if (bigIndex == -1 && bitmaps[i].getWidth() >= bigWidth && bitmaps[i].getHeight() >= bigHeight)
			bigIndex = i;
	}

	if (smallIndex > -1)
		convertBitmapToIcon(bitmaps[smallIndex], winIconSmall, smallWidth, smallHeight, 32);

	if (bigIndex > -1)
		convertBitmapToIcon(bitmaps[bigIndex], winIconBig, bigWidth, bigHeight, 32);

	if (winIconBig)
	{
		SendMessage(_HWnd, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_HWnd, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconBig);
	}
	else
	{
		SendMessage(_HWnd, WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)winIconSmall);
		SendMessage(_HWnd, WM_SETICON, 1 /* ICON_BIG */, (LPARAM)winIconSmall);
	}
}

// ***************************************************************************
void CDriverD3D::setWindowPos(sint32 x, sint32 y)
{
	_WindowX = x;
	_WindowY = y;
	SetWindowPos(_HWnd, NULL, _WindowX, _WindowY, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

// ***************************************************************************
void CDriverD3D::showWindow(bool show)
{
	ShowWindow (_HWnd, show ? SW_SHOW:SW_HIDE);
}

// ***************************************************************************

uint CDriverD3D::getNumAdapter() const
{
H_AUTO_D3D(CDriverD3D_getNumAdapter);
	if (_D3D)
		return _D3D->GetAdapterCount();
	else
		return 0;
}

// ***************************************************************************

bool CDriverD3D::getAdapter(uint adapter, IDriver::CAdapter &desc) const
{
	H_AUTO_D3D(CDriverD3D_getAdapter);
	if (_D3D)
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		UINT _adapter = (adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)adapter;
		if (_D3D->GetAdapterIdentifier(_adapter, 0, &identifier) == D3D_OK)
		{
			desc.Description = identifier.Description;
			desc.DeviceId = identifier.DeviceId;
			desc.DeviceName = identifier.DeviceName;
			desc.Driver = identifier.Driver;
			desc.DriverVersion = (((sint64)identifier.DriverVersion.HighPart<<32))|(sint64)identifier.DriverVersion.LowPart;
			desc.Revision = identifier.Revision;
			desc.SubSysId = identifier.SubSysId;
			desc.VendorId = identifier.VendorId;
			return true;
		}
	}
	return false;
}

// ***************************************************************************

bool CDriverD3D::setAdapter(uint adapter)
{
	H_AUTO_D3D(CDriverD3D_setAdapter);
	if (_D3D)
	{
		D3DDISPLAYMODE adapterMode;
		UINT _adapter = (adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)adapter;
		if (_D3D->GetAdapterDisplayMode (_adapter, &adapterMode) != D3D_OK)
		{
			nlwarning ("CDriverD3D::setDisplay: GetAdapterDisplayMode failed");
			release();
			return false;
		}
		_Adapter = adapter;
		return true;
	}
	return false;
}

// ***************************************************************************

bool CDriverD3D::supportMADOperator() const
{
	H_AUTO_D3D(CDriverD3D_supportMADOperator);
	if (_DeviceInterface)
	{
		return _MADOperatorSupported;
	}
	return false; // don't know..
}

// ***************************************************************************

bool CDriverD3D::setMode (const GfxMode& mode)
{
	H_AUTO_D3D(CDriverD3D_setMode);

	// if fullscreen
	if(!mode.Windowed)
	{
		// Must check if mode exist, else crash at reset() time
		std::vector<GfxMode>	modes;
		if(getModes(modes))
		{
			bool	found= false;
			for(uint i=0;i<modes.size();i++)
			{
				if( modes[i].Windowed==mode.Windowed &&
					modes[i].Width==mode.Width &&
					modes[i].Height==mode.Height &&
					modes[i].Depth==mode.Depth &&
					modes[i].Frequency==mode.Frequency )
				{
					found= true;
					break;
				}
			}

			// found?
			if(!found)
				return false;
		}
		else
			return false;
	}

	// set the mode
	if( mode.Windowed )
	{
		// Set windowed-mode style
		SetWindowLongW( _HWnd, GWL_STYLE, D3D_WINDOWED_STYLE|WS_VISIBLE);
		_FullScreen = false;
	}
	else
	{
		// Set fullscreen-mode style
		SetWindowLongW( _HWnd, GWL_STYLE, D3D_FULLSCREEN_STYLE|WS_VISIBLE);
		_FullScreen = true;
	}

	SetWindowLongPtrW(_HWnd, GWLP_WNDPROC, GetWindowLongPtr(_HWnd, GWLP_WNDPROC));

	// Reset the driver
	if (reset (mode))
	{
		// Reajust window
		if( _CurrentMode.Windowed )
		{
			// Window rect
			RECT	WndRect;
			WndRect.left=_WindowX;
			WndRect.top=_WindowY;
			WndRect.right=_WindowX+_CurrentMode.Width;
			WndRect.bottom=_WindowY+_CurrentMode.Height;
			AdjustWindowRect(&WndRect, GetWindowLongW (_HWnd, GWL_STYLE), FALSE);

			SetWindowPos( _HWnd, HWND_NOTOPMOST,
				std::max ((int)WndRect.left, 0), std::max ((int)WndRect.top, 0),
						  ( WndRect.right - WndRect.left ),
						  ( WndRect.bottom - WndRect.top ),
						  SWP_NOMOVE | SWP_SHOWWINDOW );
		}
		return true;
	}
	return false;
}


// ***************************************************************************
void CDriverD3D::deleteIndexBuffer(CIBDrvInfosD3D *indexBuffer)
{
	if (!indexBuffer) return;
	CIndexBuffer *ib = indexBuffer->IndexBufferPtr;
	// If resident in RAM, content has not been lost
	if (ib->getLocation () != CIndexBuffer::RAMResident)
	{
		// Realloc local memory
		ib->setLocation (CIndexBuffer::NotResident);
		delete indexBuffer;
	}
}


// ***************************************************************************
bool CDriverD3D::reset (const GfxMode& mode)
{
	//DUMP_AUTO(reset);
	H_AUTO_D3D(CDriverD3D_reset);
	if (_DeviceInterface->TestCooperativeLevel() == D3DERR_DEVICELOST) return false;

	// Choose an adapter
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;

	// Get adapter format
	D3DDISPLAYMODE adapterMode;
	if (_D3D->GetAdapterDisplayMode (adapter, &adapterMode) != D3D_OK)
	{
		nlwarning ("CDriverD3D::reset: GetAdapterDisplayMode failed");
		release();
		return false;
	}

	// Do the reset
	// Increment the IDriver reset counter

	D3DPRESENT_PARAMETERS parameters;
	D3DFORMAT adapterFormat;
	if (!fillPresentParameter (parameters, adapterFormat, mode, adapterMode))
		return false;

	// Current mode
	_CurrentMode = mode;
	_CurrentMaterial = NULL;
	_CurrentMaterialInfo = NULL;

	// Restaure non managed vertex buffer in system memory
	ItVBDrvInfoPtrList iteVb = _VBDrvInfos.begin();
	while (iteVb != _VBDrvInfos.end())
	{
		ItVBDrvInfoPtrList iteVbNext = iteVb;
		iteVbNext++;

		CVBDrvInfosD3D *vertexBuffer = static_cast<CVBDrvInfosD3D*>(*iteVb);
		CVertexBuffer *vb = vertexBuffer->VertexBufferPtr;

		// If resident in RAM, content has not been lost
		if (vb->getLocation () != CVertexBuffer::RAMResident)
		{
			// Realloc local memory
			vb->setLocation (CVertexBuffer::NotResident);	// If resident in RAM, content backuped by setLocation
			delete vertexBuffer;
		}

		iteVb = iteVbNext;
	}

	// Restore non managed index buffer in system memory
	ItIBDrvInfoPtrList iteIb = _IBDrvInfos.begin();
	while (iteIb != _IBDrvInfos.end())
	{
		ItIBDrvInfoPtrList iteIbNext = iteIb;
		iteIbNext++;
		deleteIndexBuffer(static_cast<CIBDrvInfosD3D*>(*iteIb));
		iteIb = iteIbNext;
	}
	deleteIndexBuffer(static_cast<CIBDrvInfosD3D*>((IIBDrvInfos *) _QuadIndexesAGP.DrvInfos));

	// Remove render targets
	ItTexDrvSharePtrList ite = _TexDrvShares.begin();
	while (ite != _TexDrvShares.end())
	{
		ItTexDrvSharePtrList iteNext = ite;
		iteNext++;

		// Render target ?
		nlassert ((*ite)->DrvTexture != NULL);
		CTextureDrvInfosD3D *info = static_cast<CTextureDrvInfosD3D*>(static_cast<ITextureDrvInfos*>((*ite)->DrvTexture));
		if (info->RenderTarget)
		{
			// Remove it
			delete *ite;
		}
		ite = iteNext;
	}

	// Free volatile buffers
	_VolatileVertexBufferRAM[0]->release ();
	_VolatileVertexBufferRAM[1]->release ();
	_VolatileVertexBufferAGP[0]->release ();
	_VolatileVertexBufferAGP[1]->release ();
	_VolatileIndexBuffer16RAM[0]->release ();
	_VolatileIndexBuffer16RAM[1]->release ();
	_VolatileIndexBuffer16AGP[0]->release ();
	_VolatileIndexBuffer16AGP[1]->release ();
	_VolatileIndexBuffer32RAM[0]->release ();
	_VolatileIndexBuffer32RAM[1]->release ();
	_VolatileIndexBuffer32AGP[0]->release ();
	_VolatileIndexBuffer32AGP[1]->release ();

	// Back buffer ref
	if (_BackBuffer)
		_BackBuffer->Release();
	_BackBuffer = NULL;

	// Invalidate all occlusion queries.
	// TODO nico : for now, even if a result has been retrieved successfully by the query, the query is put in the lost state. See if its worth improving...
	for(TOcclusionQueryList::iterator it = _OcclusionQueryList.begin(); it != _OcclusionQueryList.end(); ++it)
	{
		if ((*it)->Query)
		{
			(*it)->WasLost = true;
			(*it)->Query->Release();
			(*it)->Query = NULL;
		}
	}

	// Release internal shaders
	//releaseInternalShaders();


	notifyAllShaderDrvOfLostDevice();



	/* Do not reset if reset will fail */
	_ResetCounter++;
	bool sceneBegun = hasSceneBegun();
	if (sceneBegun)
	{
		//nldebug("EndScene");
		endScene();
	}

	// delete all .fx caches
	for(TMatDrvInfoPtrList::iterator it = _MatDrvInfos.begin(); it != _MatDrvInfos.end(); ++it)
	{
		CMaterialDrvInfosD3D *mi = NLMISC::safe_cast<CMaterialDrvInfosD3D *>(*it);
		if (mi->FXCache)
		{
			mi->FXCache->reset();
		}
	}

	{
#ifndef NL_NO_ASM
		CFpuRestorer fpuRestorer; // fpu control word is changed by "Reset"
#endif
		if (_Rasterizer!=D3DDEVTYPE_REF)
		{
			HRESULT hr = _DeviceInterface->Reset (&parameters);
			if (hr != D3D_OK)
			{
				nlwarning("CDriverD3D::reset: Reset on _DeviceInterface error 0x%x", hr);
				// tmp
				nlstopex(("CDriverD3D::reset: Reset on _DeviceInterface"));
				return false;
			}
		}
	}

	_Lost = false;
	// BeginScene now
	if (sceneBegun)
	{
		//nldebug("BeginScene");
		beginScene();
	}

	notifyAllShaderDrvOfResetDevice();

	// Reset internal caches
	resetRenderVariables();

	// Init shaders
	//initInternalShaders();

	// reallocate occlusion queries
	for(TOcclusionQueryList::iterator it = _OcclusionQueryList.begin(); it != _OcclusionQueryList.end(); ++it)
	{
		if (!(*it)->Query)
		{
			_DeviceInterface->CreateQuery(D3DQUERYTYPE_OCCLUSION, &(*it)->Query);
		}
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::fillPresentParameter (D3DPRESENT_PARAMETERS &parameters, D3DFORMAT &adapterFormat, const GfxMode& mode, const D3DDISPLAYMODE &adapterMode)
{
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	H_AUTO_D3D(CDriverD3D_fillPresentParameter);
	memset (&parameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	parameters.BackBufferWidth = mode.Width;
	parameters.BackBufferHeight = mode.Height;
	parameters.BackBufferCount = 1;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.MultiSampleQuality = 0;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = NULL;
	parameters.Windowed = mode.Windowed;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.FullScreen_RefreshRateInHz = mode.Windowed?0:mode.Frequency;
	switch (_Interval)
	{
	case 0:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		break;
	case 1:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		break;
	case 2:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_TWO;
		break;
	case 3:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_THREE;
		break;
	default:
		parameters.PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
		break;
	}

	// Build a depth index
	const uint depthIndex = (mode.Depth==16)?0:(mode.Depth==24)?1:2;

	// Choose a back buffer format
	const uint numFormat = 4;
	D3DFORMAT backBufferFormats[3][numFormat]=
	{
		{D3DFMT_R5G6B5,		D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5, D3DFMT_A4R4G4B4},
		{D3DFMT_R8G8B8,		D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8},
		{D3DFMT_A8R8G8B8,	D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8},
	};
	bool found = false;
	if (mode.Windowed)
	{
		uint backBuffer;
		for (backBuffer=0; backBuffer<numFormat; backBuffer++)
		{
			if (_D3D->CheckDeviceType(adapter, _Rasterizer, adapterMode.Format, backBufferFormats[depthIndex][backBuffer], TRUE) == D3D_OK)
			{
				parameters.BackBufferFormat = backBufferFormats[depthIndex][backBuffer];
				adapterFormat = adapterMode.Format;
				found = true;
				break;
			}
		}
	}
	else
	{
		// Found a pair display and back buffer format
		uint backBuffer;
		for (backBuffer=0; backBuffer<numFormat; backBuffer++)
		{
			uint display;
			for (display=0; display<numFormat; display++)
			{
				if (_D3D->CheckDeviceType(adapter, _Rasterizer, backBufferFormats[depthIndex][display], backBufferFormats[depthIndex][display], FALSE) == D3D_OK)
				{
					parameters.BackBufferFormat = backBufferFormats[depthIndex][backBuffer];
					adapterFormat = backBufferFormats[depthIndex][display];
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
	}

	// Not found ?
	if (!found)
	{
		nlwarning ("Can't create backbuffer");
		return false;
	}

	// Choose a zbuffer format
	D3DFORMAT zbufferFormats[]=
	{
		//uncomment to save zbuffer D3DFMT_D32F_LOCKABLE,
		//uncomment to save zbuffer D3DFMT_D16_LOCKABLE,
		/*D3DFMT_D32,
		D3DFMT_D24X8,*/
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D24FS8,
		//D3DFMT_D16,
	};

	const uint zbufferFormatCount = sizeof(zbufferFormats)/sizeof(D3DFORMAT);
	uint i;
	for (i=0; i<zbufferFormatCount; i++)
	{
		if (isDepthFormatOk (adapter, _Rasterizer, zbufferFormats[i], adapterFormat, parameters.BackBufferFormat))
			break;
	}

	if (i>=zbufferFormatCount)
	{
		nlwarning ("Can't create zbuffer");
		return false;
	}

	// Set the zbuffer format
	parameters.AutoDepthStencilFormat = zbufferFormats[i];

	if(mode.AntiAlias == -1)
	{
		parameters.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}
	else
	{
		DWORD aa = 0;
		if(mode.AntiAlias == 0)
		{
			if(SUCCEEDED(_D3D->CheckDeviceMultiSampleType(adapter, _Rasterizer, adapterFormat, mode.Windowed, D3DMULTISAMPLE_NONMASKABLE, &aa)))
			{
				parameters.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
				parameters.MultiSampleQuality = aa - 1;
				nlinfo("Use AntiAlias with %d sample", aa);
			}
			else
			{
				nlwarning("No AntiAlias support found");
			}
		}
		else
		{
			parameters.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
			parameters.MultiSampleQuality = mode.AntiAlias - 1;
			nlinfo("Use AntiAlias with %d sample", mode.AntiAlias);
		}
	}

	return true;
}

// ***************************************************************************

const char *CDriverD3D::getVideocardInformation ()
{
	H_AUTO_D3D(CDriverD3D_getVideocardInformation);
	static char name[1024];

//	if (!_DeviceInterface)
//		return "Direct3d isn't initialized";

	D3DADAPTER_IDENTIFIER9 identifier;
	UINT adapter = (_Adapter==0xffffffff)?D3DADAPTER_DEFAULT:(UINT)_Adapter;
	if (_D3D->GetAdapterIdentifier(adapter, 0, &identifier) == D3D_OK)
	{
		uint64 version = ((uint64)identifier.DriverVersion.HighPart) << 32;
		version |= identifier.DriverVersion.LowPart;
		smprintf (name, 1024, "Direct3d / %s / %s / %s / driver version : %d.%d.%d.%d", identifier.Driver, identifier.Description, identifier.DeviceName,
			(uint16)((version>>48)&0xFFFF),(uint16)((version>>32)&0xFFFF),(uint16)((version>>16)&0xFFFF),(uint16)(version&0xffff));
		return name;
	}
	else
		return "Can't get video card information";
}

// ***************************************************************************

void CDriverD3D::getBuffer (CBitmap &bitmap)
{
	H_AUTO_D3D(CDriverD3D_getBuffer);
	NLMISC::CRect rect;
	rect.setWH (0, 0, _CurrentMode.Width, _CurrentMode.Height);
	getBufferPart (bitmap, rect);
}

// ***************************************************************************

void CDriverD3D::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	H_AUTO_D3D(CDriverD3D_getBufferPart);
	if (_DeviceInterface)
	{
		// Resize the bitmap
		const uint lineCount = rect.Height;
		const uint width = rect.Width;
		bitmap.resize (width, lineCount, CBitmap::RGBA);

		// Lock the back buffer
		IDirect3DSurface9 *surface;
		if (_DeviceInterface->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
		{
			// Surface desc
			D3DSURFACE_DESC desc;
			if (surface->GetDesc(&desc) == D3D_OK)
			{
				// 32 bits format supported
				if ((desc.Format == D3DFMT_R8G8B8) ||
					(desc.Format == D3DFMT_A8R8G8B8) ||
					(desc.Format == D3DFMT_X8R8G8B8) ||
					(desc.Format == D3DFMT_A8B8G8R8) ||
					(desc.Format == D3DFMT_X8B8G8R8))
				{
					// Invert RGBA ?
					bool invertRGBA = (desc.Format == D3DFMT_R8G8B8) || (desc.Format == D3DFMT_A8R8G8B8) || (desc.Format == D3DFMT_X8R8G8B8);

					// Lock the surface
					D3DLOCKED_RECT lock;
					::RECT winRect;
					winRect.left = rect.left();
					winRect.right = rect.right();
					winRect.top = rect.top();
					winRect.bottom = rect.bottom();
					if (surface->LockRect (&lock, &winRect, D3DLOCK_READONLY) == D3D_OK)
					{
						// Line count
						uint8 *dest = &(bitmap.getPixels ()[0]);
						uint i;
						for (i=0; i<lineCount; i++)
						{
							if (invertRGBA)
								copyRGBA2BGRA ((uint32*)dest+(i*width), (uint32*)((uint8*)lock.pBits+(i*lock.Pitch)), width);
							else
								memcpy (dest+(4*i*width), ((uint8*)lock.pBits)+(i*lock.Pitch), width*4);
						}

						surface->UnlockRect ();
					}
				}
			}

			surface->Release();
		}
	}
}

// ***************************************************************************

IDirect3DSurface9 * CDriverD3D::getSurfaceTexture(ITexture * text)
{
	if(!text)
	{
		if(!_BackBuffer)
		{
			updateRenderVariables ();
			_DeviceInterface->GetRenderTarget (0, &_BackBuffer);
		}
		nlassert(_BackBuffer);
		return _BackBuffer;
	}

	if(text->TextureDrvShare==NULL || text->TextureDrvShare->DrvTexture.getPtr()==NULL)
	{
		text->setRenderTarget(true);
		setupTexture(*text);
	}
	CTextureDrvInfosD3D* rdvInfosD3D = (NLMISC::safe_cast<CTextureDrvInfosD3D*>(text->TextureDrvShare->DrvTexture.getPtr()));

	IDirect3DTexture9 * texture = rdvInfosD3D->Texture2d;
	nlassert(texture);

	IDirect3DSurface9 * surface;
	HRESULT hr = texture->GetSurfaceLevel(0, &surface);
	nlassert(hr==D3D_OK);

	return surface;
}

void CDriverD3D::getDirect3DRect(NLMISC::CRect &rect, RECT & d3dRect)
{
	d3dRect.left = rect.left();
	d3dRect.top = rect.top();
	d3dRect.right = rect.right();
	d3dRect.bottom = rect.bottom();

	uint32 w, h;
	getWindowSize(w, h);

	if(d3dRect.top>(sint32)h)
		d3dRect.top = h;

	if(d3dRect.right>(sint32)w)
		d3dRect.right = w;

	if(d3dRect.bottom<0)
		d3dRect.bottom = 0;

	if(d3dRect.left<0)
		d3dRect.left = 0;
}

bool CDriverD3D::stretchRect(ITexture * srcText, NLMISC::CRect &srcRect, ITexture * destText, NLMISC::CRect &destRect)
{
	H_AUTO_D3D(CDriverD3D_stretchRect);
	if (_DeviceInterface)
	{
		IDirect3DSurface9 * srcSurface = getSurfaceTexture(srcText);
		IDirect3DSurface9 * destSurface = getSurfaceTexture(destText);

		D3DTEXTUREFILTERTYPE filterType = D3DTEXF_LINEAR;

		RECT srcD3DRect;
		RECT destD3DRect;
		getDirect3DRect(srcRect, srcD3DRect);
		getDirect3DRect(destRect, destD3DRect);

		HRESULT hr = _DeviceInterface->StretchRect(srcSurface, &srcD3DRect, destSurface, &destD3DRect, filterType);

		srcSurface->Release();
		destSurface->Release();

		return (hr==D3D_OK);
	}

	return false;
}

// ***************************************************************************

bool CDriverD3D::supportBloomEffect() const
{
	return supportVertexProgram(CVertexProgram::nelvp);
}

// ***************************************************************************

bool CDriverD3D::supportNonPowerOfTwoTextures() const
{
	return _NonPowerOfTwoTexturesSupported;
}

// ***************************************************************************

bool CDriverD3D::fillBuffer (NLMISC::CBitmap &bitmap)
{
	H_AUTO_D3D(CDriverD3D_fillBuffer);
	bool result = false;
	if (_DeviceInterface)
	{
		// Resize the bitmap
		const uint lineCount = _CurrentMode.Height;
		const uint width = _CurrentMode.Width;
		if ((bitmap.getWidth() == width) && (bitmap.getHeight() == lineCount))
		{
			// Lock the back buffer
			IDirect3DSurface9 *surface;
			if (_DeviceInterface->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
			{
				// Surface desc
				D3DSURFACE_DESC desc;
				if (surface->GetDesc(&desc) == D3D_OK)
				{
					// 32 bits format supported
					if ((desc.Format == D3DFMT_R8G8B8) ||
						(desc.Format == D3DFMT_A8R8G8B8) ||
						(desc.Format == D3DFMT_X8R8G8B8) ||
						(desc.Format == D3DFMT_A8B8G8R8) ||
						(desc.Format == D3DFMT_X8B8G8R8))
					{
						// Invert RGBA ?
						bool invertRGBA = (desc.Format == D3DFMT_R8G8B8) || (desc.Format == D3DFMT_A8R8G8B8) || (desc.Format == D3DFMT_X8R8G8B8);

						// Lock the surface
						D3DLOCKED_RECT lock;
						::RECT winRect;
						winRect.left = 0;
						winRect.right = width;
						winRect.top = 0;
						winRect.bottom = lineCount;
						if (surface->LockRect (&lock, &winRect, 0) == D3D_OK)
						{
							// Line count
							uint8 *dest = &(bitmap.getPixels ()[0]);
							uint i;
							for (i=0; i<lineCount; i++)
							{
								if (invertRGBA)
									copyRGBA2BGRA ((uint32*)((uint8*)lock.pBits+(i*lock.Pitch)), (uint32*)dest+(i*width), width);
								else
									memcpy (((uint8*)lock.pBits)+(i*lock.Pitch), dest+(4*i*width), width*4);
							}

							surface->UnlockRect ();
							result = true;
						}
					}
				}

				surface->Release();
			}
		}
	}
	return result;
}

// ***************************************************************************

void CDriverD3D::setSwapVBLInterval(uint interval)
{
	H_AUTO_D3D(CDriverD3D_setSwapVBLInterval);
	if (_Interval != interval)
	{
		_Interval = interval;

		// Must do a reset because the vsync parameter is choosed in the D3DPRESENT structure
		if (_DeviceInterface)
			reset (_CurrentMode);
	}
}

// ***************************************************************************

uint CDriverD3D::getSwapVBLInterval()
{
	H_AUTO_D3D(CDriverD3D_getSwapVBLInterval);
	return _Interval;
}

// ***************************************************************************
void CDriverD3D::finish()
{
	// TODO : actually do not wait until everythin is rendered
	H_AUTO_D3D(CDriverD3D_finish);
	// Flush now
	//nldebug("EndScene");
	endScene();
	//nldebug("BeginScene");
	beginScene();
}

// ***************************************************************************
void CDriverD3D::flush()
{
	H_AUTO_D3D(CDriverD3D_finish);
	// Flush now
	//nldebug("EndScene");
	endScene();
	//nldebug("BeginScene");
	beginScene();
}

// ***************************************************************************

bool CDriverD3D::setMonitorColorProperties (const CMonitorColorProperties &properties)
{
	/*
	H_AUTO_D3D(CDriverD3D_setMonitorColorProperties);
	// The ramp
	D3DGAMMARAMP ramp;

	// For each composant
	uint c;
	for( c=0; c<3; c++ )
	{
		WORD *table = (c==0)?ramp.red:(c==1)?ramp.green:ramp.blue;
		uint i;
		for( i=0; i<256; i++ )
		{
			// Floating value
			float value = (float)i / 256;

			// Contrast
			value = (float) max (0.0f, (value-0.5f) * (float) pow (3.f, properties.Contrast[c]) + 0.5f );

			// Gamma
			value = (float) pow (value, (properties.Gamma[c]>0) ? 1 - 3 * properties.Gamma[c] / 4 : 1 - properties.Gamma[c] );

			// Luminosity
			value = value + properties.Luminosity[c] / 2.f;
			table[i] = min (65535, max (0, (int)(value * 65535)));
		}
	}

	// Set the ramp
	_DeviceInterface->SetGammaRamp (0, D3DSGR_NO_CALIBRATION, &ramp);
	return true;
	*/

	// TODO nico
	// It would be better to apply the gamma ramp only to the window and not to the whole desktop when playing in windowed mode.
	// This require to switch to D3D 9.0c which has a flag for that purpose in the 'Present' function.
	// Currently the SetGammaRamp only works in fullscreen mode, so we rely to the classic 'SetDeviceGammaRamp' instead.
	HDC dc = CreateDCW (L"DISPLAY", NULL, NULL, NULL);
	if (dc)
	{
		// The ramp
		WORD ramp[256*3];

		// For each composant
		uint c;
		for( c=0; c<3; c++ )
		{
			uint i;
			for( i=0; i<256; i++ )
			{
				// Floating value
				float value = (float)i / 256;

				// Contrast
				value = (float) max (0.0f, (value-0.5f) * (float) pow (3.f, properties.Contrast[c]) + 0.5f );

				// Gamma
				value = (float) pow (value, (properties.Gamma[c]>0) ? 1 - 3 * properties.Gamma[c] / 4 : 1 - properties.Gamma[c] );

				// Luminosity
				value = value + properties.Luminosity[c] / 2.f;
				ramp[i+(c<<8)] = (WORD)min (65535, max (0, (int)(value * 65535)));
			}
		}

		// Set the ramp
		bool result = SetDeviceGammaRamp (dc, ramp) != FALSE;

		// Release the DC
		ReleaseDC (NULL, dc);

		// Returns result
		return result;
	}
	else
	{
		nlwarning ("(CDriverD3D::setMonitorColorProperties): can't create DC");
		return false;
	}

}
// ***************************************************************************

// ****************************************************************************
bool CDriverD3D::supportEMBM() const
{
	H_AUTO_D3D(CDriverD3D_supportEMBM);
	return _EMBMSupported;
}

// ****************************************************************************
bool CDriverD3D::isEMBMSupportedAtStage(uint stage) const
{
	H_AUTO_D3D(CDriverD3D_isEMBMSupportedAtStage);
	// we assume EMBM is supported at all stages except the last one
	return stage < _NbNeLTextureStages - 1;
}

// ****************************************************************************
void CDriverD3D::setEMBMMatrix(const uint stage, const float mat[4])
{
	H_AUTO_D3D(CDriverD3D_setEMBMMatrix);
	nlassert(stage < _NbNeLTextureStages - 1);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT00, (DWORD &) mat[0]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT01, (DWORD &) mat[1]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT10, (DWORD &) mat[2]);
	SetTextureStageState(stage, D3DTSS_BUMPENVMAT11, (DWORD &) mat[3]);
}

// ***************************************************************************
bool CDriverD3D::supportOcclusionQuery() const
{
	H_AUTO_D3D(CDriverD3D_supportOcclusionQuery);
	return _OcclusionQuerySupported;
}

// ***************************************************************************
IOcclusionQuery *CDriverD3D::createOcclusionQuery()
{
	H_AUTO_D3D(CDriverD3D_createOcclusionQuery);
	nlassert(_OcclusionQuerySupported);
	nlassert(_DeviceInterface);
	IDirect3DQuery9 *query;
	if (_DeviceInterface->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query) != D3D_OK) return NULL;
	COcclusionQueryD3D *oqd3d = new COcclusionQueryD3D;
	oqd3d->Driver = this;
	oqd3d->Query = query;
	oqd3d->VisibleCount = 0;
	oqd3d->OcclusionType = IOcclusionQuery::NotAvailable;
	oqd3d->QueryIssued = false;
	oqd3d->WasLost = false;
	_OcclusionQueryList.push_front(oqd3d);
	oqd3d->Iterator = _OcclusionQueryList.begin();
	return oqd3d;
}

// ***************************************************************************
void CDriverD3D::deleteOcclusionQuery(IOcclusionQuery *oq)
{
	H_AUTO_D3D(CDriverD3D_deleteOcclusionQuery);
	if (!oq) return;
	COcclusionQueryD3D *oqd3d = NLMISC::safe_cast<COcclusionQueryD3D *>(oq);
	nlassert((CDriverD3D *) oqd3d->Driver == this); // should come from the same driver
	oqd3d->Driver = NULL;
	if (oqd3d->Query)
	{
		oqd3d->Query->Release();
		oqd3d->Query = NULL;
	}
	_OcclusionQueryList.erase(oqd3d->Iterator);
	if (oqd3d == _CurrentOcclusionQuery)
	{
		_CurrentOcclusionQuery = NULL;
	}
	delete oqd3d;
}

// ***************************************************************************
void COcclusionQueryD3D::begin()
{
	H_AUTO_D3D(COcclusionQueryD3D_begin);
	if (!Query) return; // Lost device
	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == NULL); // only one query at a time
	Query->Issue(D3DISSUE_BEGIN);
	Driver->_CurrentOcclusionQuery = this;
	OcclusionType = NotAvailable;
	QueryIssued = false;
	WasLost = false;
}

// ***************************************************************************
void COcclusionQueryD3D::end()
{
	H_AUTO_D3D(COcclusionQueryD3D_end);
	if (!Query) return; // Lost device
	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == this); // only one query at a time
	if (!WasLost)
	{
		Query->Issue(D3DISSUE_END);
	}
	Driver->_CurrentOcclusionQuery = NULL;
	QueryIssued = true;
}

// ***************************************************************************
IOcclusionQuery::TOcclusionType COcclusionQueryD3D::getOcclusionType()
{
	if (!Query || WasLost) return QueryIssued ? Occluded : NotAvailable;
	H_AUTO_D3D(COcclusionQueryD3D_getOcclusionType);
	nlassert(Driver);
	nlassert(Query);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (OcclusionType == NotAvailable)
	{
		DWORD numPix;
		if (Query->GetData(&numPix, sizeof(DWORD), 0) == S_OK)
		{
			OcclusionType = numPix != 0 ? NotOccluded : Occluded;
			VisibleCount = (uint) numPix;
		}
	}
	return OcclusionType;
}

// ***************************************************************************
uint COcclusionQueryD3D::getVisibleCount()
{
	if (!Query || WasLost) return 0;
	H_AUTO_D3D(COcclusionQueryD3D_getVisibleCount);
	nlassert(Driver);
	nlassert(Query);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (getOcclusionType() == NotAvailable) return 0;
	return VisibleCount;
}

// ***************************************************************************
bool CDriverD3D::supportWaterShader() const
{
	H_AUTO_D3D(CDriverD3D_supportWaterShader);
	return _PixelShaderVersion >= D3DPS_VERSION(1, 1);
}

// ***************************************************************************
void CDriverD3D::setCullMode(TCullMode cullMode)
{
	H_AUTO_D3D(CDriver3D_cullMode);
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
	NL_D3D_CACHE_TEST(CacheTest_CullMode, cullMode != _CullMode)
#endif
	{
		if (_InvertCullMode)
		{
			setRenderState(D3DRS_CULLMODE, _CullMode == CCW ? D3DCULL_CW : D3DCULL_CCW);
		}
		else
		{
			setRenderState(D3DRS_CULLMODE, _CullMode == CCW ? D3DCULL_CCW : D3DCULL_CW);
		}
		_CullMode = cullMode;
	}
}

// ***************************************************************************
IDriver::TCullMode CDriverD3D::getCullMode() const
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);
	return _CullMode;
}

// ***************************************************************************
void CDriverD3D::enableStencilTest(bool enable)
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);

	_CurStencilTest = enable;
	setRenderState(D3DRS_STENCILENABLE, enable?TRUE:FALSE);
}

// ***************************************************************************
bool CDriverD3D::isStencilTestEnabled() const
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);
	return _CurStencilTest?TRUE:FALSE;
}

// ***************************************************************************
void CDriverD3D::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);

	switch(stencilFunc)
	{
		case IDriver::never:		_CurStencilFunc=D3DCMP_NEVER; break;
		case IDriver::less:			_CurStencilFunc=D3DCMP_LESS; break;
		case IDriver::lessequal:	_CurStencilFunc=D3DCMP_LESSEQUAL; break;
		case IDriver::equal:		_CurStencilFunc=D3DCMP_EQUAL; break;
		case IDriver::notequal:		_CurStencilFunc=D3DCMP_NOTEQUAL; break;
		case IDriver::greaterequal:	_CurStencilFunc=D3DCMP_GREATEREQUAL; break;
		case IDriver::greater:		_CurStencilFunc=D3DCMP_GREATER; break;
		case IDriver::always:		_CurStencilFunc=D3DCMP_ALWAYS; break;
		default: nlstop;
	}

	_CurStencilRef = (DWORD)ref;
	_CurStencilMask = (DWORD)mask;
	setRenderState(D3DRS_STENCILFUNC, _CurStencilFunc);
	setRenderState(D3DRS_STENCILREF,  _CurStencilRef);
	setRenderState(D3DRS_STENCILMASK, _CurStencilMask);
}

// ***************************************************************************
void CDriverD3D::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);

	switch(fail)
	{
		case IDriver::keep:		_CurStencilOpFail=D3DSTENCILOP_KEEP; break;
		case IDriver::zero:		_CurStencilOpFail=D3DSTENCILOP_ZERO; break;
		case IDriver::replace:	_CurStencilOpFail=D3DSTENCILOP_REPLACE; break;
		case IDriver::incr:		_CurStencilOpFail=D3DSTENCILOP_INCR; break;
		case IDriver::decr:		_CurStencilOpFail=D3DSTENCILOP_DECR; break;
		case IDriver::invert:	_CurStencilOpFail=D3DSTENCILOP_INVERT; break;
		default: nlstop;
	}

	switch(zfail)
	{
		case IDriver::keep:		_CurStencilOpZFail=D3DSTENCILOP_KEEP; break;
		case IDriver::zero:		_CurStencilOpZFail=D3DSTENCILOP_ZERO; break;
		case IDriver::replace:	_CurStencilOpZFail=D3DSTENCILOP_REPLACE; break;
		case IDriver::incr:		_CurStencilOpZFail=D3DSTENCILOP_INCR; break;
		case IDriver::decr:		_CurStencilOpZFail=D3DSTENCILOP_DECR; break;
		case IDriver::invert:	_CurStencilOpZFail=D3DSTENCILOP_INVERT; break;
		default: nlstop;
	}

	switch(zpass)
	{
		case IDriver::keep:		_CurStencilOpZPass=D3DSTENCILOP_KEEP; break;
		case IDriver::zero:		_CurStencilOpZPass=D3DSTENCILOP_ZERO; break;
		case IDriver::replace:	_CurStencilOpZPass=D3DSTENCILOP_REPLACE; break;
		case IDriver::incr:		_CurStencilOpZPass=D3DSTENCILOP_INCR; break;
		case IDriver::decr:		_CurStencilOpZPass=D3DSTENCILOP_DECR; break;
		case IDriver::invert:	_CurStencilOpZPass=D3DSTENCILOP_INVERT; break;
		default: nlstop;
	}

	setRenderState(D3DRS_STENCILFAIL,  _CurStencilOpFail);
	setRenderState(D3DRS_STENCILZFAIL, _CurStencilOpZFail);
	setRenderState(D3DRS_STENCILPASS,  _CurStencilOpZPass);
}

// ***************************************************************************
void CDriverD3D::stencilMask(uint mask)
{
	H_AUTO_D3D(CDriver3D_CDriverD3D);

	_CurStencilWriteMask = (DWORD)mask;
	setRenderState (D3DRS_STENCILWRITEMASK, _CurStencilWriteMask);
}

// volatile bool preciseStateProfile = false;
// ***************************************************************************
void CDriverD3D::CRenderState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CRenderState);
	/*if (!preciseStateProfile)
	{*/
		driver->_DeviceInterface->SetRenderState (StateID, Value);
	/*
	}
	else
	{
		switch(StateID)
		{
			case D3DRS_ZENABLE: { H_AUTO_D3D(D3DRS_ZENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FILLMODE: { H_AUTO_D3D(D3DRS_FILLMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SHADEMODE: { H_AUTO_D3D(D3DRS_SHADEMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ZWRITEENABLE: { H_AUTO_D3D(D3DRS_ZWRITEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ALPHATESTENABLE: { H_AUTO_D3D(D3DRS_ALPHATESTENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_LASTPIXEL: { H_AUTO_D3D(D3DRS_LASTPIXEL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SRCBLEND: { H_AUTO_D3D(D3DRS_SRCBLEND); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DESTBLEND: { H_AUTO_D3D(D3DRS_DESTBLEND); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CULLMODE: { H_AUTO_D3D(D3DRS_CULLMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ZFUNC: { H_AUTO_D3D(D3DRS_ZFUNC); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ALPHAREF: { H_AUTO_D3D(D3DRS_ALPHAREF); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ALPHAFUNC: { H_AUTO_D3D(D3DRS_ALPHAFUNC); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DITHERENABLE: { H_AUTO_D3D(D3DRS_DITHERENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ALPHABLENDENABLE: { H_AUTO_D3D(D3DRS_ALPHABLENDENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGENABLE: { H_AUTO_D3D(D3DRS_FOGENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SPECULARENABLE: { H_AUTO_D3D(D3DRS_SPECULARENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGCOLOR: { H_AUTO_D3D(D3DRS_FOGCOLOR); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGTABLEMODE: { H_AUTO_D3D(D3DRS_FOGTABLEMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGSTART: { H_AUTO_D3D(D3DRS_FOGSTART); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGEND: { H_AUTO_D3D(D3DRS_FOGEND); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGDENSITY: { H_AUTO_D3D(D3DRS_FOGDENSITY); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_RANGEFOGENABLE: { H_AUTO_D3D(D3DRS_RANGEFOGENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILENABLE: { H_AUTO_D3D(D3DRS_STENCILENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILFAIL: { H_AUTO_D3D(D3DRS_STENCILFAIL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILZFAIL: { H_AUTO_D3D(D3DRS_STENCILZFAIL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILPASS: { H_AUTO_D3D(D3DRS_STENCILPASS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILFUNC: { H_AUTO_D3D(D3DRS_STENCILFUNC); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILREF: { H_AUTO_D3D(D3DRS_STENCILREF); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILMASK: { H_AUTO_D3D(D3DRS_STENCILMASK); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_STENCILWRITEMASK: { H_AUTO_D3D(D3DRS_STENCILWRITEMASK); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_TEXTUREFACTOR: { H_AUTO_D3D(D3DRS_TEXTUREFACTOR); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP0: { H_AUTO_D3D(D3DRS_WRAP0); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP1: { H_AUTO_D3D(D3DRS_WRAP1); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP2: { H_AUTO_D3D(D3DRS_WRAP2); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP3: { H_AUTO_D3D(D3DRS_WRAP3); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP4: { H_AUTO_D3D(D3DRS_WRAP4); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP5: { H_AUTO_D3D(D3DRS_WRAP5); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP6: { H_AUTO_D3D(D3DRS_WRAP6); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP7: { H_AUTO_D3D(D3DRS_WRAP7); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CLIPPING: { H_AUTO_D3D(D3DRS_CLIPPING); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_LIGHTING: { H_AUTO_D3D(D3DRS_LIGHTING); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_AMBIENT: { H_AUTO_D3D(D3DRS_AMBIENT); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_FOGVERTEXMODE: { H_AUTO_D3D(D3DRS_FOGVERTEXMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_COLORVERTEX: { H_AUTO_D3D(D3DRS_COLORVERTEX); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_LOCALVIEWER: { H_AUTO_D3D(D3DRS_LOCALVIEWER); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_NORMALIZENORMALS: { H_AUTO_D3D(D3DRS_NORMALIZENORMALS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DIFFUSEMATERIALSOURCE: { H_AUTO_D3D(D3DRS_DIFFUSEMATERIALSOURCE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SPECULARMATERIALSOURCE: { H_AUTO_D3D(D3DRS_SPECULARMATERIALSOURCE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_AMBIENTMATERIALSOURCE: { H_AUTO_D3D(D3DRS_AMBIENTMATERIALSOURCE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_EMISSIVEMATERIALSOURCE: { H_AUTO_D3D(D3DRS_EMISSIVEMATERIALSOURCE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_VERTEXBLEND: { H_AUTO_D3D(D3DRS_VERTEXBLEND); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CLIPPLANEENABLE: { H_AUTO_D3D(D3DRS_CLIPPLANEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSIZE: { H_AUTO_D3D(D3DRS_POINTSIZE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSIZE_MIN: { H_AUTO_D3D(D3DRS_POINTSIZE_MIN); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSPRITEENABLE: { H_AUTO_D3D(D3DRS_POINTSPRITEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSCALEENABLE: { H_AUTO_D3D(D3DRS_POINTSCALEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSCALE_A: { H_AUTO_D3D(D3DRS_POINTSCALE_A); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSCALE_B: { H_AUTO_D3D(D3DRS_POINTSCALE_B); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSCALE_C: { H_AUTO_D3D(D3DRS_POINTSCALE_C); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_MULTISAMPLEANTIALIAS: { H_AUTO_D3D(D3DRS_MULTISAMPLEANTIALIAS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_MULTISAMPLEMASK: { H_AUTO_D3D(D3DRS_MULTISAMPLEMASK); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_PATCHEDGESTYLE: { H_AUTO_D3D(D3DRS_PATCHEDGESTYLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DEBUGMONITORTOKEN: { H_AUTO_D3D(D3DRS_DEBUGMONITORTOKEN); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POINTSIZE_MAX: { H_AUTO_D3D(D3DRS_POINTSIZE_MAX); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_INDEXEDVERTEXBLENDENABLE: { H_AUTO_D3D(D3DRS_INDEXEDVERTEXBLENDENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_COLORWRITEENABLE: { H_AUTO_D3D(D3DRS_COLORWRITEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_TWEENFACTOR: { H_AUTO_D3D(D3DRS_TWEENFACTOR); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_BLENDOP: { H_AUTO_D3D(D3DRS_BLENDOP); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_POSITIONDEGREE: { H_AUTO_D3D(D3DRS_POSITIONDEGREE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_NORMALDEGREE: { H_AUTO_D3D(D3DRS_NORMALDEGREE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SCISSORTESTENABLE: { H_AUTO_D3D(D3DRS_SCISSORTESTENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SLOPESCALEDEPTHBIAS: { H_AUTO_D3D(D3DRS_SLOPESCALEDEPTHBIAS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ANTIALIASEDLINEENABLE: { H_AUTO_D3D(D3DRS_ANTIALIASEDLINEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_MINTESSELLATIONLEVEL: { H_AUTO_D3D(D3DRS_MINTESSELLATIONLEVEL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_MAXTESSELLATIONLEVEL: { H_AUTO_D3D(D3DRS_MAXTESSELLATIONLEVEL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ADAPTIVETESS_X: { H_AUTO_D3D(D3DRS_ADAPTIVETESS_X); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ADAPTIVETESS_Y: { H_AUTO_D3D(D3DRS_ADAPTIVETESS_Y); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ADAPTIVETESS_Z: { H_AUTO_D3D(D3DRS_ADAPTIVETESS_Z); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ADAPTIVETESS_W: { H_AUTO_D3D(D3DRS_ADAPTIVETESS_W); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_ENABLEADAPTIVETESSELLATION: { H_AUTO_D3D(D3DRS_ENABLEADAPTIVETESSELLATION); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_TWOSIDEDSTENCILMODE: { H_AUTO_D3D(D3DRS_TWOSIDEDSTENCILMODE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CCW_STENCILFAIL: { H_AUTO_D3D(D3DRS_CCW_STENCILFAIL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CCW_STENCILZFAIL: { H_AUTO_D3D(D3DRS_CCW_STENCILZFAIL); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CCW_STENCILPASS: { H_AUTO_D3D(D3DRS_CCW_STENCILPASS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_CCW_STENCILFUNC: { H_AUTO_D3D(D3DRS_CCW_STENCILFUNC); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_COLORWRITEENABLE1: { H_AUTO_D3D(D3DRS_COLORWRITEENABLE1); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_COLORWRITEENABLE2: { H_AUTO_D3D(D3DRS_COLORWRITEENABLE2); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_COLORWRITEENABLE3: { H_AUTO_D3D(D3DRS_COLORWRITEENABLE3); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_BLENDFACTOR: { H_AUTO_D3D(D3DRS_BLENDFACTOR); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SRGBWRITEENABLE: { H_AUTO_D3D(D3DRS_SRGBWRITEENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DEPTHBIAS: { H_AUTO_D3D(D3DRS_DEPTHBIAS); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP8: { H_AUTO_D3D(D3DRS_WRAP8); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP9: { H_AUTO_D3D(D3DRS_WRAP9); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP10: { H_AUTO_D3D(D3DRS_WRAP10); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP11: { H_AUTO_D3D(D3DRS_WRAP11); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP12: { H_AUTO_D3D(D3DRS_WRAP12); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP13: { H_AUTO_D3D(D3DRS_WRAP13); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP14: { H_AUTO_D3D(D3DRS_WRAP14); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_WRAP15: { H_AUTO_D3D(D3DRS_WRAP15); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SEPARATEALPHABLENDENABLE: { H_AUTO_D3D(D3DRS_SEPARATEALPHABLENDENABLE); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_SRCBLENDALPHA: { H_AUTO_D3D(D3DRS_SRCBLENDALPHA); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_DESTBLENDALPHA: { H_AUTO_D3D(D3DRS_DESTBLENDALPHA); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
			case D3DRS_BLENDOPALPHA: { H_AUTO_D3D(D3DRS_BLENDOPALPHA); driver->_DeviceInterface->SetRenderState(StateID, Value); } break;
		}
	}
	*/
}


// ***************************************************************************
void CDriverD3D::CTextureState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CTextureState);
	if (Value != DeviceValue)
	{
		driver->_DeviceInterface->SetTextureStageState (StageID, StateID, Value);
		DeviceValue = Value;
	}
}

// ***************************************************************************
void CDriverD3D::CTextureIndexState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CTextureIndexState);
	if (TexGen)
		driver->setTextureState (StageID, D3DTSS_TEXCOORDINDEX, TexGenMode);
	else
		driver->setTextureState (StageID, D3DTSS_TEXCOORDINDEX, UVChannel);
}

// ***************************************************************************
void CDriverD3D::CTexturePtrState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CTexturePtrState);
	driver->_DeviceInterface->SetTexture (StageID, Texture);
}

// ***************************************************************************
void CDriverD3D::CVertexProgramPtrState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CVertexProgramPtrState);
	driver->_DeviceInterface->SetVertexShader(VertexProgram);
}

// ***************************************************************************
void CDriverD3D::CPixelShaderPtrState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CPixelShaderPtrState);
	if (!driver->_PixelProgram) return;
	driver->_DeviceInterface->SetPixelShader(PixelShader);
}

// ***************************************************************************
void CDriverD3D::CVertexProgramConstantState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CVertexProgramConstantState);
	switch (ValueType)
	{
	case CVertexProgramConstantState::Float:
		driver->_DeviceInterface->SetVertexShaderConstantF (StateID, (float*)Values, 1);
		break;
	case CVertexProgramConstantState::Int:
		driver->_DeviceInterface->SetVertexShaderConstantI (StateID, (int*)Values, 1);
		break;
	}
}

// ***************************************************************************
void CDriverD3D::CPixelShaderConstantState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CPixelShaderConstantState);
	switch (ValueType)
	{
	case CPixelShaderConstantState::Float:
		driver->_DeviceInterface->SetPixelShaderConstantF (StateID, (float*)Values, 1);
		break;
	case CPixelShaderConstantState::Int:
		driver->_DeviceInterface->SetPixelShaderConstantI (StateID, (int*)Values, 1);
		break;
	}
}

// ***************************************************************************
void CDriverD3D::CSamplerState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CSamplerState);
	driver->_DeviceInterface->SetSamplerState (SamplerID, StateID, Value);
}

// ***************************************************************************
void CDriverD3D::CMatrixState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CMatrixState);
	driver->_DeviceInterface->SetTransform (TransformType, &Matrix);
}

// ***************************************************************************
void CDriverD3D::CVBState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CVBState);
	if (VertexBuffer)
	{
		driver->_DeviceInterface->SetStreamSource (0, VertexBuffer, Offset, Stride);
		// Fix for radeon 7xxx & bad vertex layout
		if (driver->inlGetNumTextStages() == 3) // If there are 3 stages this is a Radeon 7xxx
		{
			if (ColorOffset != 0 && driver->_VertexDeclCache.EnableVertexColor)
			{
				driver->_DeviceInterface->SetStreamSource (1, VertexBuffer, Offset + ColorOffset, Stride);
			}
			else
			{
				driver->_DeviceInterface->SetStreamSource (1, NULL, 0, 0);
			}
		}
	}
}
// ***************************************************************************
void CDriverD3D::CIBState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CIBState);
	if (IndexBuffer)
	{
		driver->_DeviceInterface->SetIndices (IndexBuffer);
	}
}

// ***************************************************************************
void CDriverD3D::CVertexDeclState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CVertexDeclState);
	if (Decl)
	{
		if (!EnableVertexColor && DeclAliasDiffuseToSpecular && driver->inlGetNumTextStages() == 3)
		{
			// Fix for radeon 7xxx -> if vertex color is not used it should not be present in the vertex declaration (example : lighted material + vertex color but, no vertexColorLighted)
			nlassert(DeclNoDiffuse);
			driver->_DeviceInterface->SetVertexDeclaration (DeclNoDiffuse);
		}
		else
		if (AliasDiffuseToSpecular)
		{
			nlassert(DeclAliasDiffuseToSpecular);
			driver->_DeviceInterface->SetVertexDeclaration (DeclAliasDiffuseToSpecular);
		}
		else
		{
			nlassert(Decl);
			driver->_DeviceInterface->SetVertexDeclaration (Decl);
		}
	}
}

// ***************************************************************************
void CDriverD3D::CLightState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CLightState);
	// Enable state modified ?
	{
		H_AUTO_D3D(CDriverD3D_CLightStateEnabled);
		if (EnabledTouched)
			driver->_DeviceInterface->LightEnable (LightIndex, Enabled);
	}
	{
		H_AUTO_D3D(CDriverD3D_CLightStateSetup);
		// Light enabled ?
		if (Enabled)
		{
			if (SettingsTouched)
			{
				// New position
				Light.Position.x -= driver->_PZBCameraPos.x;
				Light.Position.y -= driver->_PZBCameraPos.y;
				Light.Position.z -= driver->_PZBCameraPos.z;
				driver->_DeviceInterface->SetLight (LightIndex, &Light);
				SettingsTouched = false;
			}
		}
		// Clean
		EnabledTouched = false;
	}
}

// ***************************************************************************
void CDriverD3D::CRenderTargetState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CRenderTargetState);
	driver->_DeviceInterface->SetRenderTarget (0, Target);
	driver->setupViewport(driver->_Viewport);
	driver->setupScissor(driver->_Scissor);
}

// ***************************************************************************
void CDriverD3D::CMaterialState::apply(CDriverD3D *driver)
{
	H_AUTO_D3D(CDriverD3D_CMaterialState);
	driver->_DeviceInterface->SetMaterial(&Current);
}

// ***************************************************************************
void CDriverD3D::beginDialogMode()
{
	if (_FullScreen && _HWnd)
		ShowWindow(_HWnd, SW_MINIMIZE);
}

// ***************************************************************************
void CDriverD3D::endDialogMode()
{
	if (_FullScreen && _HWnd)
		ShowWindow(_HWnd, SW_MAXIMIZE);
}

bool CDriverD3D::clipRect(NLMISC::CRect &rect)
{
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

void CDriverD3D::getZBuffer(std::vector<float>  &zbuffer)
{
	H_AUTO_D3D(CDriverD3D_getZBuffer);

	CRect rect(0, 0);
	getWindowSize(rect.Width, rect.Height);
	getZBufferPart(zbuffer, rect);
}

void CDriverD3D::getZBufferPart (std::vector<float> &zbuffer, NLMISC::CRect &rect)
{
	zbuffer.clear();

	IDirect3DSurface9 *surface;
	if (SUCCEEDED(_DeviceInterface->GetDepthStencilSurface(&surface)))
	{
		if (clipRect(rect))
		{
			RECT winRect;
			winRect.left = rect.left();
			winRect.right = rect.right();
			winRect.top = rect.top();
			winRect.bottom = rect.bottom();

			// Lock the surface
			D3DLOCKED_RECT lock;
			if (SUCCEEDED(surface->LockRect (&lock, &winRect, D3DLOCK_READONLY)))
			{
				zbuffer.resize(rect.Width*rect.Height);

				// Surface desc
				D3DSURFACE_DESC desc;
				if (SUCCEEDED(surface->GetDesc(&desc)))
				{
					const uint8* pBits = (uint8*)lock.pBits;

					for(uint y=0; y<rect.Height; ++y)
					{
						uint offset = y*rect.Width;
						uint end = offset + rect.Width;

						// 32 bits format supported
						if (desc.Format == D3DFMT_D32F_LOCKABLE)
						{
							const float *src = (float*)(pBits + lock.Pitch * y);
							float *dst = &zbuffer[offset];
							memcpy(dst, src, rect.Width * sizeof(float));
						}
						else if (desc.Format == D3DFMT_D24S8)
						{
							uint32* pRow = (uint32*)(pBits + lock.Pitch * y);
							while(offset != end)
							{
								uint32 value = *pRow++;
								zbuffer[offset++] = (float)value / (float)std::numeric_limits<uint32>::max();
							}
						}
						else if (desc.Format == D3DFMT_D16_LOCKABLE)
						{
							uint16* pRow = (uint16*)(pBits + lock.Pitch * y);
							while(offset != end)
							{
								uint16 value = *pRow++;
								zbuffer[offset++] = (float)value / (float)std::numeric_limits<uint16>::max();
							}
						}
					}
				}

				surface->UnlockRect ();
			}
		}

		surface->Release();
	}
}


void CDriverD3D::findNearestFullscreenVideoMode()
{
	if(_CurrentMode.Windowed)
		return;

	std::vector<GfxMode> modes;
	if(getModes(modes))
	{
		sint32 nbPixels = _CurrentMode.Width * _CurrentMode.Height;
		sint32 minError = nbPixels;
		uint bestMode = (uint)modes.size();
		for(uint i=0; i < modes.size(); i++)
		{
			if(!modes[i].Windowed)
			{
				if(modes[i].Width==_CurrentMode.Width && modes[i].Height==_CurrentMode.Height)
				{
					// ok we found the perfect mode
					return;
				}
				sint32 currentPixels = modes[i].Width * modes[i].Height;
				sint32 currentError = abs(nbPixels - currentPixels);
				if(currentError < minError)
				{
					minError = currentError;
					bestMode = i;
				}
			}
		}
		if(bestMode != modes.size())
		{
			nlwarning("The video mode %dx%d doesn't exist, use the nearest mode %dx%d", _CurrentMode.Width, _CurrentMode.Height, modes[bestMode].Width, modes[bestMode].Height);
			_CurrentMode.Width = modes[bestMode].Width;
			_CurrentMode.Height = modes[bestMode].Height;
		}
	}
}
bool CDriverD3D::copyTextToClipboard(const ucstring &text)
{
	return _EventEmitter.copyTextToClipboard(text);
}

bool CDriverD3D::pasteTextFromClipboard(ucstring &text)
{
	return _EventEmitter.pasteTextFromClipboard(text);
}

bool CDriverD3D::convertBitmapToIcon(const NLMISC::CBitmap &bitmap, HICON &icon, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY, bool cursor)
{
	CBitmap src = bitmap;
	// resample bitmap if necessary
	if (src.getWidth() != iconWidth || src.getHeight() != iconHeight)
	{
		src.resample(iconWidth, iconHeight);
	}
	CBitmap colorBm;
	colorBm.resize(iconWidth, iconHeight, CBitmap::RGBA);
	const CRGBA *srcColorPtr = (CRGBA *) &(src.getPixels()[0]);
	const CRGBA *srcColorPtrLast = srcColorPtr + (iconWidth * iconHeight);
	CRGBA *destColorPtr = (CRGBA *) &(colorBm.getPixels()[0]);
	static volatile uint8 alphaThreshold = 127;
	do
	{
		destColorPtr->modulateFromColor(*srcColorPtr, col);
		std::swap(destColorPtr->R, destColorPtr->B);
		++ srcColorPtr;
		++ destColorPtr;
	}
	while (srcColorPtr != srcColorPtrLast);
	//
	HBITMAP colorHbm = NULL;
	HBITMAP maskHbm = NULL;
	//
	if (iconDepth == 16)
	{
		std::vector<uint16> colorBm16(iconWidth * iconHeight);
		const CRGBA *src32 = (const CRGBA *) &colorBm.getPixels(0)[0];

		for (uint k = 0; k < colorBm16.size(); ++k)
		{
			colorBm16[k] = ((uint16)(src32[k].R&0xf8)>>3) | ((uint16)(src32[k].G&0xfc)<<3) | ((uint16)(src32[k].B & 0xf8)<<8);
		}

		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 16, &colorBm16[0]);
		std::vector<uint8> bitMask((iconWidth * iconHeight + 7) / 8, 0);

		for (uint k = 0;k < colorBm16.size(); ++k)
		{
			if (src32[k].A <= 120)
			{
				bitMask[k / 8] |= (0x80 >> (k & 7));
			}
		}

		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 1, &bitMask[0]);
	}
	else
	{
		colorHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
		maskHbm = CreateBitmap(iconWidth, iconHeight, 1, 32, &colorBm.getPixels(0)[0]);
	}

	ICONINFO iconInfo;
	iconInfo.fIcon = cursor ? FALSE:TRUE;
	iconInfo.xHotspot = (DWORD) hotSpotX;
	iconInfo.yHotspot = (DWORD) hotSpotY;
	iconInfo.hbmMask = maskHbm;
	iconInfo.hbmColor = colorHbm;

	if (colorHbm && maskHbm)
	{
		icon = CreateIconIndirect(&iconInfo);
	}

	//
	if (colorHbm) DeleteObject(colorHbm);
	if (maskHbm) DeleteObject(maskHbm);

	return true;
}

} // NL3D
