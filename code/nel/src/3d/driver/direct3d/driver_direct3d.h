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

#ifndef NL_DRIVER_DIRECT3D_H
#define NL_DRIVER_DIRECT3D_H


#include "nel/misc/types_nl.h"

// NeL includes
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/heap_memory.h"
#include "nel/misc/event_emitter_multi.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/win_event_emitter.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/ptr_set.h"
#include "nel/3d/texture_cube.h"
#include "nel/3d/occlusion_query.h"
#include "nel/3d/vertex_program_parse.h"
#include "nel/3d/light.h"
//
#include <algorithm>

typedef HCURSOR nlCursor;
#define EmptyCursor NULL


// *** DEBUG MACRO

// Define this to activate the debug mode (default is undefined)
//#define NL_DEBUG_D3D

// Define this to enable the render state caching (default is defined)
#define NL_D3D_USE_RENDER_STATE_CACHE

// allows to enable / disable cache test at runtime (for debug)
//#define NL_D3D_RUNTIME_DEBUG_CACHE_TEST
#ifdef NL_D3D_RUNTIME_DEBUG_CACHE_TEST
	#define NL_D3D_CACHE_TEST(label, cond) if (!_CacheTest[label] || (cond))
#else
	#define NL_D3D_CACHE_TEST(label, cond) if (cond)
#endif


// Define this to disable hardware vertex program (default is undefined)
//#define NL_DISABLE_HARDWARE_VERTEX_PROGAM

// Define this to disable hardware pixel shaders program (default is undefined)
//#define NL_DISABLE_HARDWARE_PIXEL_SHADER

// Define this to disable hardware vertex array AGP (default is undefined)
//#define NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP

// Define this to force the texture stage count (default is undefined)
//#define NL_FORCE_TEXTURE_STAGE_COUNT 2

// Define this to force the use of pixel shader in the normal shaders (default is undefined)
//#define NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS

// Define this to enable profiling by the NV Perf HUD tool (default is undefined)
//#define NL_D3D_USE_NV_PERF_HUD

// Define this to enable profiling of driver functions (default is undefined).
//#define NL_PROFILE_DRIVER_D3D



#ifdef NL_PROFILE_DRIVER_D3D
	#define H_AUTO_D3D(label) H_AUTO(label)
#else
	#define H_AUTO_D3D(label)
#endif

class CFpuRestorer
{
public:
	CFpuRestorer() { _FPControlWord = _controlfp(0, 0); }
	~CFpuRestorer()
	{
		_controlfp(_FPControlWord, ~0);
	}
private:
	unsigned int _FPControlWord;
};

class CFpuChecker
{
public:
	CFpuChecker(const char *label) { _FPControlWord = _controlfp(0, 0); _Label = label; }
	~CFpuChecker()
	{
		unsigned int newFP = _controlfp(0, 0);
		if ((newFP & (_MCW_DN | _MCW_IC | _MCW_RC | _MCW_PC)) != (_FPControlWord &  (_MCW_DN | _MCW_IC | _MCW_RC | _MCW_PC)))
		{
			nlwarning(_Label);
			nlassert(0);
		}
	}
private:
	const char   *_Label;
	unsigned int _FPControlWord;
};




inline bool operator==(const D3DCOLORVALUE &lhs, const D3DCOLORVALUE &rhs)
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
inline bool operator!=(const D3DCOLORVALUE &lhs, const D3DCOLORVALUE &rhs)
{
	return !(lhs == rhs);
}





// ***************************************************************************

namespace NL3D
{



const uint MAX_NUM_QUADS = 32767; // max number of quads in a single draw call

using NLMISC::CMatrix;
using NLMISC::CVector;

class	CDriverD3D;
class	CTextureDrvInfosD3D;
class   COcclusionQueryD3D;
class   CVolatileVertexBuffer;
class   CVolatileIndexBuffer;

typedef std::list<COcclusionQueryD3D *> TOcclusionQueryList;

// ***************************************************************************
class COcclusionQueryD3D : public IOcclusionQuery
{
public:
	IDirect3DQuery9					*Query;
	NLMISC::CRefPtr<CDriverD3D>		Driver;			// owner driver
	TOcclusionQueryList::iterator   Iterator;		// iterator in owner driver list of queries
	TOcclusionType					OcclusionType;  // current type of occlusion
	uint							VisibleCount;	// number of samples that passed the test
	bool							QueryIssued;
	bool							WasLost; // tells that query was lost, so calls to end() will have not effects (there's no matching begin)
	// From IOcclusionQuery
	virtual void begin();
	virtual void end();
	virtual TOcclusionType getOcclusionType();
	virtual uint getVisibleCount();
};


using NLMISC::CRefCount;


class	IDriver;
class CDriverD3D;

// List typedef.
class	IShaderDrvInfos;
typedef	std::list<IShaderDrvInfos*>		TShaderDrvInfoPtrList;
typedef	TShaderDrvInfoPtrList::iterator	ItShaderDrvInfoPtrList;

/**
  * Interface for shader driver infos.
  */
class IShaderDrvInfos : public CRefCount
{
private:
	CDriverD3D				*_Driver;
	ItShaderDrvInfoPtrList		_DriverIterator;

public:
	IShaderDrvInfos(CDriverD3D	*drv, ItShaderDrvInfoPtrList it) {_Driver= drv; _DriverIterator= it;}
	// The virtual dtor is important.
	virtual ~IShaderDrvInfos();
};


/**
 * Shader resource for the driver. It is just a container for a ".fx" text file.
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// --------------------------------------------------
class CD3DShaderFX
{
public:
	CD3DShaderFX();
	~CD3DShaderFX();

	// Load a shader file
	bool loadShaderFile (const char *filename);

	// Set the shader text
	void setText (const char *text);

	// Get the shader text
	const char *getText () const { return _Text.c_str(); }

	// Set the shader name
	void setName (const char *name);

	// Get the shader name
	const char *getName () const { return _Name.c_str(); }

public:
	// Private. For Driver only.
	bool								_ShaderChanged;
	NLMISC::CRefPtr<IShaderDrvInfos>	_DrvInfo;
private:
	// The shader
	std::string					_Text;
	// The shader name
	std::string					_Name;
};



// ***************************************************************************
class CTextureDrvInfosD3D : public ITextureDrvInfos
{
public:
	/*
		ANY DATA ADDED HERE MUST BE SWAPPED IN swapTextureHandle() !!
	*/

	// The texture
	LPDIRECT3DBASETEXTURE9	Texture;
	IDirect3DTexture9		*Texture2d;
	IDirect3DCubeTexture9	*TextureCube;

	// The texture format and size
	D3DFORMAT				DestFormat;
	uint					Width;
	uint					Height;

	// This is the owner driver.
	CDriverD3D				*_Driver;

	// Is the internal format of the texture is a compressed one?
	bool					SrcCompressed;
	bool					IsCube;

	// Is a render target ?
	bool					RenderTarget;

	// Mipmap levels
	uint8					Levels;
	uint8					FirstMipMap;

	// This is the computed size of what memory this texture take.
	uint32					TextureMemory;

	// The current wrap modes assigned to the texture.
	D3DTEXTUREADDRESS		WrapS;
	D3DTEXTUREADDRESS		WrapT;
	D3DTEXTUREFILTERTYPE	MagFilter;
	D3DTEXTUREFILTERTYPE	MinFilter;
	D3DTEXTUREFILTERTYPE	MipFilter;

	CTextureDrvInfosD3D(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverD3D *drvGl, bool renderTarget);
	~CTextureDrvInfosD3D();
	virtual uint	getTextureMemoryUsed() const {return TextureMemory;}
};



// ***************************************************************************
class CVertexProgamDrvInfosD3D : public IProgramDrvInfos
{
public:

	// The shader
	IDirect3DVertexShader9	*Shader;

	CVertexProgamDrvInfosD3D(IDriver *drv, ItGPUPrgDrvInfoPtrList it);
	~CVertexProgamDrvInfosD3D();

	virtual uint getUniformIndex(const char *name) const
	{ 
		std::map<std::string, uint>::const_iterator it = ParamIndices.find(name);
		if (it != ParamIndices.end()) return it->second; 
		return std::numeric_limits<uint>::max();
	};

	std::map<std::string, uint> ParamIndices;
};
 

// ***************************************************************************
class CPixelProgramDrvInfosD3D : public IProgramDrvInfos
{
public:
 
	// The shader
	IDirect3DPixelShader9	*Shader;

	CPixelProgramDrvInfosD3D(IDriver *drv, ItGPUPrgDrvInfoPtrList it);
	~CPixelProgramDrvInfosD3D();

	virtual uint getUniformIndex(const char *name) const
	{ 
		std::map<std::string, uint>::const_iterator it = ParamIndices.find(name);
		if (it != ParamIndices.end()) return it->second; 
		return std::numeric_limits<uint>::max();
	};

	std::map<std::string, uint> ParamIndices;
};


// ***************************************************************************

class CVertexDeclaration
{
public:
	// The human readable values
	D3DVERTEXELEMENT9				VertexElements[CVertexBuffer::NumValue+1];

	// The driver pointer
	IDirect3DVertexDeclaration9		*VertexDecl;
};


// ***************************************************************************
class CVBDrvInfosD3D : public IVBDrvInfos
{
public:
	IDirect3DVertexDeclaration9		*VertexDecl;
	IDirect3DVertexDeclaration9		*VertexDeclAliasDiffuseToSpecular;
	IDirect3DVertexDeclaration9		*VertexDeclNoDiffuse;
	uint							ColorOffset;        // Fix for Radeon 7xxx series -> see remarks in CDriverD3D::createVertexDeclaration


	IDirect3DVertexBuffer9			*VertexBuffer;
	uint							Offset;				// Vertex buffer offset
	bool							UseVertexColor:1;
	bool							Hardware:1;
	bool							Volatile:1; 		// Volatile vertex buffer
	bool							VolatileRAM:1;
	uint8							Stride:8;
	uint							VolatileLockTime;	// Volatile vertex buffer
	DWORD							Usage;
	CVolatileVertexBuffer			*VolatileVertexBuffer;
	CDriverD3D						*Driver;
	#ifdef NL_DEBUG
	bool Locked;
	#endif

	CVBDrvInfosD3D(CDriverD3D *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb);
	virtual ~CVBDrvInfosD3D();
	virtual uint8	*lock (uint first, uint last, bool readOnly);
	virtual void	unlock (uint first, uint last);
};



// ***************************************************************************

class CIBDrvInfosD3D : public IIBDrvInfos
{
public:
	IDirect3DIndexBuffer9			*IndexBuffer;
	uint							Offset;				// Index buffer offset
	bool							Volatile:1; 		// Volatile index buffer
	bool							VolatileRAM:1;
	uint							VolatileLockTime;	// Volatile index buffer
	CVolatileIndexBuffer			*VolatileIndexBuffer;
	CDriverD3D						*Driver;
	std::vector<uint32>				RamVersion; // If device doesn't support 32 bit indexes, works in ram (unless it a 16 bit index buffer)
	CIBDrvInfosD3D(CDriverD3D *drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib);
	virtual ~CIBDrvInfosD3D();
	virtual void	*lock (uint first, uint last, bool readOnly);
	virtual void	unlock (uint first, uint last);
};

// ***************************************************************************

class CShaderDrvInfosD3D : public IShaderDrvInfos
{
public:
	enum
	{
		MaxShaderTexture=8,
	};


	ID3DXEffect				*Effect;
	bool					Validated;

	// Texture handles
	D3DXHANDLE				TextureHandle[MaxShaderTexture];

	// Color handles
	D3DXHANDLE				ColorHandle[MaxShaderTexture];

	// Factor handles
	D3DXHANDLE				FactorHandle[MaxShaderTexture];

	// Scalar handles
	D3DXHANDLE				ScalarFloatHandle[MaxShaderTexture];

	CShaderDrvInfosD3D(CDriverD3D *drv, ItShaderDrvInfoPtrList it);
	virtual ~CShaderDrvInfosD3D();
};


// ***************************************************************************

// Normal shader description
class CNormalShaderDesc
{
public:
	CNormalShaderDesc ()
	{
		H_AUTO_D3D(CNormalShaderDesc_CNormalShaderDesc);
		memset (this, 0, sizeof(CNormalShaderDesc));
	}
	~CNormalShaderDesc ()
	{
		if (PixelShader)
			PixelShader->Release();
	}
	bool					StageUsed[IDRV_MAT_MAXTEXTURES];
	uint32					TexEnvMode[IDRV_MAT_MAXTEXTURES];

	bool operator==(const CNormalShaderDesc &other) const
	{
		uint i;
		for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
			if ((StageUsed[i] != other.StageUsed[i]) || (StageUsed[i] && (TexEnvMode[i] != other.TexEnvMode[i])))
				return false;
		return true;
	}

	IDirect3DPixelShader9	*PixelShader;
};



// base class for recorded state in an effect
class CStateRecord
{
public:
	// apply record in a driver
	virtual void apply(class CDriverD3D &drv) = 0;
	virtual ~CStateRecord() {}
	// use STL allocator for fast alloc. this works because objects are small ( < 128 bytes)
	void *operator new(size_t size) { return CStateRecord::Allocator.allocate(size); }
	void *operator new(size_t size, int /* blockUse */, char const * /* fileName */, int /* lineNumber */)
	{
		// TODO: add memory leaks detector
		return CStateRecord::Allocator.allocate(size);
	}
	void operator delete(void *block) { CStateRecord::Allocator.deallocate((uint8 *) block, 1); }
	void operator delete(void *block, int /* blockUse */, char const* /* fileName */, int /* lineNumber */)
	{
		// TODO: add memory leaks detector
		CStateRecord::Allocator.deallocate((uint8 *)block, 1);
	}

	static std::allocator<uint8> Allocator;
};

// record of a single .fx pass
class CFXPassRecord
{
public:
	void apply(class CDriverD3D &drv);
	~CFXPassRecord();
	std::vector<CStateRecord *> States;
};


template <class T>
class CFXInputValue
{
public:
	T	 Value;
	bool Set;
	CFXInputValue() : Set(false) {}
	void reset();
	bool operator==(const CFXInputValue &other)
	{
		if (!Set) return !(other.Set);
		return (Value == other.Value) != 0;
	}
};

class CFXInputParams
{
public:
	enum { MaxNumParams = CShaderDrvInfosD3D::MaxShaderTexture };
	CFXInputValue<LPDIRECT3DBASETEXTURE9> Textures[MaxNumParams];
	CFXInputValue<DWORD>				  Colors[MaxNumParams];
	CFXInputValue<D3DXVECTOR4>			  Vectors[MaxNumParams];
	CFXInputValue<FLOAT>			      Floats[MaxNumParams];
	bool Touched;
public:
	CFXInputParams() { Touched = true; }
	void setTexture(uint index, LPDIRECT3DBASETEXTURE9 value)
	{
		nlassert(index < MaxNumParams);
		if (!Textures[index].Set || value != Textures[index].Value)
		{
			Textures[index].Value = value;
			Textures[index].Set = true;
			Touched = true;
		}
	}
	void setColor(uint index, DWORD value)
	{
		nlassert(index < MaxNumParams);
		if (!Colors[index].Set || value != Colors[index].Value)
		{
			Colors[index].Value = value;
			Colors[index].Set = true;
			Touched = true;
		}
	}
	void setVector(uint index, const D3DXVECTOR4 &value)
	{
		nlassert(index < MaxNumParams);
		if (!Vectors[index].Set || value != Vectors[index].Value)
		{
			Vectors[index].Value = value;
			Vectors[index].Set = true;
			Touched = true;
		}
	}
	void setFloat(uint index, FLOAT value)
	{
		nlassert(index < MaxNumParams);
		if (!Floats[index].Set || value != Floats[index].Value)
		{
			Floats[index].Value = value;
			Floats[index].Set = true;
			Touched = true;
		}
	}
	//
	/*
	bool operator==(const CFXInputParams &other)
	{
		return std::equal(Textures, Textures + CShaderDrvInfosD3D::MaxShaderTexture, other.Textures) &&
			   std::equal(Vectors, Vectors + CShaderDrvInfosD3D::MaxShaderTexture, other.Vectors) &&
			   std::equal(Colors, Colors + CShaderDrvInfosD3D::MaxShaderTexture, other.Colors) &&
			   std::equal(Floats, Floats + CShaderDrvInfosD3D::MaxShaderTexture, other.Floats);
	}
	*/
	void reset()
	{
		for(uint k = 0; k < MaxNumParams; ++k)
		{
			Textures[k].Set = false;
			Colors[k].Set = false;
			Vectors[k].Set = false;
			Floats[k].Set = false;
		}
		Touched = true;
	}
};

// .fx cache based on input parameters
class CFXCache
{
public:
	// Input parameters
	CFXInputParams  Params;
	// cache for .fx states
	std::vector<CFXPassRecord> Passes;
	uint					   Steadyness;
	uint					   NumPasses;
public:
	CFXCache() : Steadyness(0) {}
	void begin(CShaderDrvInfosD3D *si, class CDriverD3D *driver);
	void applyPass(class CDriverD3D &drv, CShaderDrvInfosD3D *si, uint passIndex);
	void end(CShaderDrvInfosD3D *si);
	void reset();
	void setConstants(CShaderDrvInfosD3D *si);
};



// optimisation of an effect 2
class CFXPassRecorder : public ID3DXEffectStateManager
{
public:
	CFXPassRecord *Target;
	class CDriverD3D *Driver;
public:
	CFXPassRecorder() : Target(NULL), Driver(NULL) {}
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
	ULONG STDMETHODCALLTYPE AddRef(VOID);
	ULONG STDMETHODCALLTYPE Release(VOID);
	HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable);
	HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF);
	HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT9* pLight);
	HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial);
	HRESULT STDMETHODCALLTYPE SetNPatchMode(FLOAT nSegments);
	HRESULT STDMETHODCALLTYPE SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTexture (DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
	HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	HRESULT STDMETHODCALLTYPE SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);
};


// ***************************************************************************
struct CMaterialDrvInfosD3D : public IMaterialDrvInfos
{
public:
	D3DMATERIAL9	Material;
	D3DCOLOR		UnlightedColor;
	BOOL			SpecularEnabled;
	D3DBLEND		SrcBlend;
	D3DBLEND		DstBlend;
	D3DCMPFUNC		ZComp;
	DWORD			AlphaRef;
	D3DTEXTUREOP	ColorOp[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg0[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg1[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg2[IDRV_MAT_MAXTEXTURES];
	uint			NumColorArg[IDRV_MAT_MAXTEXTURES];
	uint			NumAlphaArg[IDRV_MAT_MAXTEXTURES];
	D3DTEXTUREOP	AlphaOp[IDRV_MAT_MAXTEXTURES];
	DWORD 			AlphaArg0[IDRV_MAT_MAXTEXTURES];
	DWORD 			AlphaArg1[IDRV_MAT_MAXTEXTURES];
	DWORD			AlphaArg2[IDRV_MAT_MAXTEXTURES];
	D3DCOLOR		ConstantColor[IDRV_MAT_MAXTEXTURES];
	DWORD			TexGen[IDRV_MAT_MAXTEXTURES];
	IDirect3DPixelShader9	*PixelShader;
	IDirect3DPixelShader9	*PixelShaderUnlightedNoVertexColor;
	bool			ActivateSpecularWorldTexMT[IDRV_MAT_MAXTEXTURES];
	bool			ActivateInvViewModelTexMT[IDRV_MAT_MAXTEXTURES];
	bool			VertexColorLighted;
	bool			NeedsConstantForDiffuse;	    // Must use TFactor if not vertex color in the vertex buffer
	bool			MultipleConstantNoPixelShader;  // Multiple constant are possibly needed to setup the material. This flag is set only if the device has no pixel shaders
	                                                // In this case diffuse color will be emulated by using an unlighted material with ambient
	bool			MultiplePerStageConstant;       // Are there more than one per-stage constant in the material ?
	uint8			ConstantIndex;				    // Index of the constant color to use (when only one constant color is needed and NeedsConstantForDiffuse == false);
	uint8			ConstantIndex2;                 // stage at which the 2nd constant is used (for emulation without pixel shaders)

	CRGBA			Constant2;						// value of the 2nd constant being used (for emulation without pixel shaders)

	uint			NumUsedTexStages;				// Last number of textures that were set in the material
	                                                // Tex Env are only built for stages at which textures are set so if the number of used texture
	                                                // change they must be rebuilt

	// Relevant parts of the pixel pipe for normal shader
	bool			RGBPipe[IDRV_MAT_MAXTEXTURES];
	bool			AlphaPipe[IDRV_MAT_MAXTEXTURES];

	CFXCache		*FXCache;

	CMaterialDrvInfosD3D(IDriver *drv, ItMatDrvInfoPtrList it) : IMaterialDrvInfos(drv, it)
	{
		H_AUTO_D3D(CMaterialDrvInfosD3D_CMaterialDrvInfosD3D);
		PixelShader = NULL;
		PixelShaderUnlightedNoVertexColor = NULL;
		std::fill(RGBPipe, RGBPipe + IDRV_MAT_MAXTEXTURES, true);
		std::fill(AlphaPipe, AlphaPipe + IDRV_MAT_MAXTEXTURES, true);
		FXCache = NULL;
		NumUsedTexStages = 0;
	}
	~CMaterialDrvInfosD3D()
	{
		delete FXCache;
	}
	void buildTexEnv (uint stage, const CMaterial::CTexEnv &env, bool textured);

};


//

// ***************************************************************************

/* Volatile buffers.
 *
 * The volatile buffer system is a double buffer allocated during the render pass by objects that needs
 * a temporary buffer to render vertex or indexes. Each lock allocates a buffer region
 * and locks it with the NOOVERWRITE flag. Locks are not blocked. The buffer is reset at each frame begin.
 * There is a double buffer to take benefit of the "over swapbuffer" parallelisme.
 */

// ***************************************************************************

class CVolatileVertexBuffer
{
public:
	CVolatileVertexBuffer();
	~CVolatileVertexBuffer();

	CDriverD3D					*Driver;
	IDirect3DVertexBuffer9		*VertexBuffer;
	uint						Size;
	CVertexBuffer::TLocation	Location;
	uint						CurrentIndex;
	uint						MaxSize;
	bool						Locked;

	/* size is in bytes */
	void	init (CVertexBuffer::TLocation	location, uint size, uint maxSize, CDriverD3D *driver);
	void	release ();

	// Runtime buffer access, no-blocking lock.
	void	*lock (uint size, uint stride, uint &offset);
	void	unlock ();

	// Runtime reset (called at the beginning of the frame rendering), blocking lock here.
	void	reset ();
};

// ***************************************************************************

class CVolatileIndexBuffer
{
public:
	CVolatileIndexBuffer();
	~CVolatileIndexBuffer();

	CDriverD3D					*Driver;
	IDirect3DIndexBuffer9		*IndexBuffer;
	uint						Size;
	CIndexBuffer::TLocation		Location;
	// current position in bytes!
	uint						CurrentIndex;
	uint						MaxSize;
	bool						Locked;
	CIndexBuffer::TFormat		Format;

	/* size is in bytes */
	void	init (CIndexBuffer::TLocation	location, uint sizeInBytes, uint maxSize, CDriverD3D *driver, CIndexBuffer::TFormat format);
	void	release ();

	// Runtime buffer access, no-blocking lock. Re
	void	*lock (uint size, uint &offset);
	void	unlock ();

	// Runtime reset (called at the beginning of the frame rendering), blocking lock here.
	void	reset ();
};



// ***************************************************************************

class CDriverD3D : public IDriver, public ID3DXEffectStateManager
{
public:

	enum
	{
		CacheTest_CullMode = 0,
		CacheTest_RenderState = 1,
		CacheTest_TextureState = 2,
		CacheTest_TextureIndexMode = 3,
		CacheTest_TextureIndexUV = 4,
		CacheTest_Texture = 5,
		CacheTest_VertexProgram = 6,
		CacheTest_PixelShader = 7,
		CacheTest_VertexProgramConstant = 8,
		CacheTest_PixelShaderConstant = 9,
		CacheTest_SamplerState = 10,
		CacheTest_VertexBuffer = 11,
		CacheTest_IndexBuffer = 12,
		CacheTest_VertexDecl = 13,
		CacheTest_Matrix = 14,
		CacheTest_RenderTarget = 15,
		CacheTest_MaterialState = 16,
		CacheTest_DepthRange = 17,
		CacheTest_Count
	};

	// Some constants
	enum
	{
		MaxLight=8,
		MaxRenderState=256,
		MaxTextureState=36,
		MaxTexture=8,
		MaxSamplerState=16,
		MaxSampler=8,

		MatrixStateRemap = 24,
		MaxMatrixState=32,
		MaxVertexProgramConstantState=96,
		MaxPixelShaderConstantState=96,
	};

	// Prefered pixel formats
	enum
	{
		// Number of pixel format choice for each pixel format
		FinalPixelFormatChoice = 5,
	};

	// Construction / destruction
							CDriverD3D();
	virtual					~CDriverD3D();

	virtual	bool			isLost() const { return _Lost; }
	// ***************************************************************************
	// Implementation
	// see nel\src\3d\driver.h
	// ***************************************************************************

	// Mode initialisation, requests
	virtual bool			init (uintptr_t windowIcon = 0, emptyProc exitFunc = 0);
	virtual bool			setDisplay(nlWindow wnd, const GfxMode& mode, bool show, bool resizeable);
	virtual bool			release();
	virtual bool			setMode(const GfxMode& mode);
	virtual bool			getModes(std::vector<GfxMode> &modes);
	virtual bool			getCurrentScreenMode(GfxMode &mode);
	virtual void			beginDialogMode();
	virtual void			endDialogMode();
	virtual bool			activate();
	virtual bool			isActive ();
	virtual	bool			initVertexBufferHard(uint agpMem, uint vramMem);

	// Windows interface
	virtual nlWindow		getDisplay();
	virtual emptyProc		getWindowProc();
	virtual NLMISC::IEventEmitter	*getEventEmitter();
	virtual void			getWindowSize (uint32 &width, uint32 &height);
	virtual void			getWindowPos (sint32 &x, sint32 &y);
	virtual uint8			getBitPerPixel ();

	/// Set the title of the NeL window
	virtual void			setWindowTitle(const ucstring &title);

	/// Set icon(s) of the NeL window
	virtual void			setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps);

	/// Set the position of the NeL window
	virtual void			setWindowPos(sint32 x, sint32 y);

	/// Show or hide the NeL window
	virtual void			showWindow(bool show);

	// Driver parameters
	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwarePixelProgram();
	virtual void			disableHardwareIndexArrayAGP();
	virtual void			disableHardwareVertexArrayAGP();
	virtual void			disableHardwareTextureShader();
	virtual void			forceDXTCCompression(bool dxtcComp);
	virtual void			setAnisotropicFilter(sint filter);
	virtual uint			getAnisotropicFilter() const;
	virtual uint			getAnisotropicFilterMaximum() const;
	virtual void			forceTextureResize(uint divisor);

	// Driver information
	virtual uint			getNumAdapter() const;
	virtual bool			getAdapter(uint adapter, CAdapter &desc) const;
	virtual bool			setAdapter(uint adapter);
	virtual uint32			getAvailableVertexAGPMemory ();
	virtual uint32			getAvailableVertexVRAMMemory ();
	virtual	uint			getNbTextureStages() const;
	virtual void			getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const;
	virtual	bool			supportVertexBufferHard() const;
	virtual bool			supportVolatileVertexBuffer() const;
	virtual	bool			supportIndexBufferHard() const;
	// todo hulud d3d vertex buffer hard
	virtual	bool			slowUnlockVertexBufferHard() const {return false;};
	virtual	uint			getMaxVerticesByVertexBufferHard() const;
	virtual uint32			getImplementationVersion () const;
	virtual const char*		getDriverInformation ();
	virtual const char*		getVideocardInformation ();
	virtual sint			getTotalVideoMemory () const;
	virtual CVertexBuffer::TVertexColorType getVertexColorFormat() const;

	// Textures
	virtual bool			isTextureExist(const ITexture&tex);
	virtual bool			setupTexture (ITexture& tex);
	virtual bool			setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture= false);
	virtual bool			uploadTexture (ITexture& tex, NLMISC::CRect& rect, uint8 nNumMipMap);
	// todo hulud d3d texture
	virtual bool			uploadTextureCube (ITexture& /* tex */, NLMISC::CRect& /* rect */, uint8 /* nNumMipMap */, uint8 /* nNumFace */) {return false;};

	// Material
	virtual bool			setupMaterial(CMaterial& mat);

	virtual bool			supportCloudRenderSinglePass () const;

	// Buffer
	virtual bool			clear2D(CRGBA rgba);
	virtual bool			clearZBuffer(float zval=1);
	virtual bool			clearStencilBuffer(float stencilval=0);
	virtual void			setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha);
	virtual bool			swapBuffers();
	virtual void			getBuffer (CBitmap &bitmap);	// Only 32 bits back buffer supported
	virtual void			setDepthRange(float znear, float zfar);
	virtual	void			getDepthRange(float &znear, float &zfar) const;

	virtual void			getZBuffer (std::vector<float> &zbuffer);
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect);

	// return true if driver support Bloom effect.
	virtual	bool			supportBloomEffect() const;

	// return true if driver support non-power of two textures
	virtual bool			supportNonPowerOfTwoTextures() const;

	// copy the first texture in a second one of different dimensions
	virtual bool			stretchRect (ITexture * srcText, NLMISC::CRect &srcRect, ITexture * destText, NLMISC::CRect &destRect);	// Only 32 bits back buffer supported
	virtual bool			isTextureRectangle(ITexture * /* tex */) const {return false;}
	IDirect3DSurface9*		getSurfaceTexture(ITexture * text);
	void					getDirect3DRect(NLMISC::CRect &rect, RECT & d3dRect);

	// todo hulud d3d buffers
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect);
	virtual bool			setRenderTarget (ITexture *tex, uint32 x, uint32 y, uint32 width, uint32 height, uint32 mipmapLevel, uint32 cubeFace);
	virtual ITexture		*getRenderTarget() const;
	virtual bool			copyTargetToTexture (ITexture *tex, uint32 offsetx, uint32 offsety, uint32 x, uint32 y, uint32 width,
													uint32 height, uint32 mipmapLevel);
	virtual bool			textureCoordinateAlternativeMode() const { return true; };
	virtual bool			getRenderTargetSize (uint32 &width, uint32 &height);
	virtual bool			fillBuffer (CBitmap &bitmap);

	// Performances
	virtual void			startSpecularBatch();
	virtual void			endSpecularBatch();
	virtual void			setSwapVBLInterval(uint interval);
	virtual uint			getSwapVBLInterval();
	virtual void			swapTextureHandle(ITexture &tex0, ITexture &tex1);
	virtual	uintptr_t		getTextureHandle(const ITexture&tex);

	// Matrix, viewport and frustum
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);
	virtual	void			setFrustumMatrix(CMatrix &frust);
	virtual	CMatrix			getFrustumMatrix();
	virtual float			getClipSpaceZMin() const { return 0.f; }
	virtual void			setupViewMatrix(const CMatrix& mtx);
	virtual void			setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos);
	virtual void			setupModelMatrix(const CMatrix& mtx);
	virtual CMatrix			getViewMatrix() const;
	virtual	void			forceNormalize(bool normalize);
	virtual	bool			isForceNormalize() const;
	virtual void			setupScissor (const class CScissor& scissor);
	virtual void			setupViewport (const class CViewport& viewport);
	virtual	void			getViewport(CViewport &viewport);

	// Vertex buffers
	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	// Index buffers
	virtual bool			activeIndexBuffer(CIndexBuffer& IB);

	// UV
	virtual	void			mapTextureStageToUV(uint stage, uint uv);

	// Indexed primitives
	virtual bool			renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines);
	virtual bool			renderTriangles(CMaterial& Mat, uint32 firstIndex, uint32 ntris);
	virtual bool			renderSimpleTriangles(uint32 firstTri, uint32 ntris);
	// Indexed primitives with index offset
	virtual bool			renderLinesWithIndexOffset(CMaterial& mat, uint32 firstIndex, uint32 nlines, uint indexOffset);
	virtual bool			renderTrianglesWithIndexOffset(CMaterial& mat, uint32 firstIndex, uint32 ntris, uint indexOffset);
	virtual bool			renderSimpleTrianglesWithIndexOffset(uint32 firstIndex, uint32 ntris, uint indexOffset);
	// Unindexed primitives with index offset
	virtual bool			renderRawPoints(CMaterial& Mat, uint32 startIndex, uint32 numPoints);
	virtual bool			renderRawLines(CMaterial& Mat, uint32 startIndex, uint32 numLines);
	virtual bool			renderRawTriangles(CMaterial& Mat, uint32 startIndex, uint32 numTris);
	virtual bool			renderRawQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads);
	//
	virtual void			setPolygonMode (TPolygonMode mode);
	virtual	void			finish();
	virtual	void			flush();

	// Profile
	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);
	virtual	uint32			profileAllocatedTextureMemory();
	virtual	uint32			profileSetupedMaterials() const;
	virtual	uint32			profileSetupedModelMatrix() const;
	virtual void			enableUsedTextureMemorySum (bool enable);
	virtual uint32			getUsedTextureMemory() const;
	virtual	void			startProfileVBHardLock();
	virtual	void			endProfileVBHardLock(std::vector<std::string> &result);
	virtual	void			profileVBHardAllocation(std::vector<std::string> &result);
	virtual	void			startProfileIBLock();
	virtual	void			endProfileIBLock(std::vector<std::string> &result);
	virtual	void			profileIBAllocation(std::vector<std::string> &result);
	//virtual	void			profileIBAllocation(std::vector<std::string> &result);

	// Misc
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);
	virtual uint64			getSwapBufferCounter() const { return _SwapBufferCounter; }

	// Inputs
	virtual void			showCursor (bool b);
	virtual void			setMousePos(float x, float y);
	virtual void			setCapture (bool b);

	// see if system cursor is currently captured
	virtual bool			isSystemCursorCaptured();

	// Add a new cursor (name is case unsensitive)
	virtual void			addCursor(const std::string &name, const NLMISC::CBitmap &bitmap);

	// Display a cursor from its name (case unsensitive)
	virtual void			setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false);

	// Change default scale for all cursors
	virtual void			setCursorScale(float scale);

	// Lights
	virtual uint			getMaxLight () const;
	virtual void			setLight (uint8 num, const CLight& light);
	virtual void			enableLight (uint8 num, bool enable=true);
	virtual void			setLightMapDynamicLight (bool enable, const CLight& light);
	// todo hulud d3d light
	virtual void			setPerPixelLightingLight(CRGBA /* diffuse */, CRGBA /* specular */, float /* shininess */) {}
	virtual void			setAmbientColor (CRGBA color);

	// Fog
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	virtual	void			setupFog(float start, float end, CRGBA color);
	virtual	float			getFogStart() const;
	virtual	float			getFogEnd() const;
	virtual	CRGBA			getFogColor() const;

	// Texture addressing modes
	// todo hulud d3d adressing mode
	virtual bool			supportTextureShaders() const {return false;};
	virtual	bool			supportMADOperator() const;
	// todo hulud d3d adressing mode
	virtual bool			supportWaterShader() const;
	// todo hulud d3d adressing mode
	virtual bool			supportTextureAddrMode(CMaterial::TTexAddressingMode /* mode */) const {return false;};
	// todo hulud d3d adressing mode
	virtual void			setMatrix2DForTextureOffsetAddrMode(const uint /* stage */, const float /* mat */[4]) {}

	// EMBM support
	virtual bool			supportEMBM() const;
	virtual bool			isEMBMSupportedAtStage(uint stage) const;
	virtual void			setEMBMMatrix(const uint stage, const float mat[4]);
	virtual bool			supportPerPixelLighting(bool /* specular */) const {return false;};

	// index offset support
	virtual bool			supportIndexOffset() const { return true; /* always supported with D3D driver */ }

	// Blend
	virtual	bool			supportBlendConstantColor() const;
	virtual	void			setBlendConstantColor(NLMISC::CRGBA col);
	virtual	NLMISC::CRGBA	getBlendConstantColor() const;

	// Monitor properties
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties);

	// Polygon smoothing
	virtual	void			enablePolygonSmoothing(bool smooth);
	virtual	bool			isPolygonSmoothingEnabled() const;

	// Material multipass
	virtual sint			beginMaterialMultiPass();
	virtual void			setupMaterialPass(uint pass);
	virtual void			endMaterialMultiPass();



	


	/// \name Vertex Program
	// @{

	// Order of preference
	// - activeVertexProgram
	// - CMaterial pass[n] VP (uses activeVertexProgram, but does not override if one already set by code)
	// - default generic VP that mimics fixed pipeline / no VP with fixed pipeline

	/**
	  * Does the driver supports vertex program, but emulated by CPU ?
	  */
	virtual bool			isVertexProgramEmulated() const;

	/** Return true if the driver supports the specified vertex program profile.
	  */
	virtual bool			supportVertexProgram(CVertexProgram::TProfile profile) const;

	/** Compile the given vertex program, return if successful.
	  * If a vertex program was set active before compilation, 
	  * the state of the active vertex program is undefined behaviour afterwards.
	  */
	virtual bool			compileVertexProgram(CVertexProgram *program);

	/** Set the active vertex program. This will override vertex programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getVertexProgram returns NULL.
	  * The vertex program is activated immediately.
	  */
	virtual bool			activeVertexProgram(CVertexProgram *program);
	// @}



	/// \name Pixel Program
	// @{

	// Order of preference
	// - activePixelProgram
	// - CMaterial pass[n] PP (uses activePixelProgram, but does not override if one already set by code)
	// - PP generated from CMaterial (uses activePixelProgram, but does not override if one already set by code)

	/** Return true if the driver supports the specified pixel program profile.
	  */
	virtual bool			supportPixelProgram(CPixelProgram::TProfile profile) const;

	/** Compile the given pixel program, return if successful.
	  * If a pixel program was set active before compilation, 
	  * the state of the active pixel program is undefined behaviour afterwards.
	  */
	virtual bool			compilePixelProgram(CPixelProgram *program);

	/** Set the active pixel program. This will override pixel programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getPixelProgram returns NULL.
	  * The pixel program is activated immediately.
	  */
	virtual bool			activePixelProgram(CPixelProgram *program);
	// @}



	/// \name Geometry Program
	// @{

	// Order of preference
	// - activeGeometryProgram
	// - CMaterial pass[n] PP (uses activeGeometryProgram, but does not override if one already set by code)
	// - none

	/** Return true if the driver supports the specified pixel program profile.
	  */
	virtual bool			supportGeometryProgram(CGeometryProgram::TProfile profile) const { return false; }

	/** Compile the given pixel program, return if successful.
	  * If a pixel program was set active before compilation, 
	  * the state of the active pixel program is undefined behaviour afterwards.
	  */
	virtual bool			compileGeometryProgram(CGeometryProgram *program) { return false; }

	/** Set the active pixel program. This will override pixel programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getGeometryProgram returns NULL.
	  * The pixel program is activated immediately.
	  */
	virtual bool			activeGeometryProgram(CGeometryProgram *program) { return false; }
	// @}



	/// \name Program parameters
	// @{
	// Set parameters
	virtual void			setUniform1f(TProgram program, uint index, float f0);
	virtual void			setUniform2f(TProgram program, uint index, float f0, float f1);
	virtual void			setUniform3f(TProgram program, uint index, float f0, float f1, float f2);
	virtual void			setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3);
	virtual void			setUniform1i(TProgram program, uint index, sint32 i0);
	virtual void			setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1);
	virtual void			setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2);
	virtual void			setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3);
	virtual void			setUniform1ui(TProgram program, uint index, uint32 ui0);
	virtual void			setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1);
	virtual void			setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2);
	virtual void			setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3);
	virtual void			setUniform3f(TProgram program, uint index, const NLMISC::CVector& v);
	virtual void			setUniform4f(TProgram program, uint index, const NLMISC::CVector& v, float f3);
	virtual void			setUniform4f(TProgram program, uint index, const NLMISC::CRGBAF& rgba);
	virtual void			setUniform4x4f(TProgram program, uint index, const NLMISC::CMatrix& m);
	virtual void			setUniform4fv(TProgram program, uint index, size_t num, const float *src);
	virtual void			setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src);
	virtual void			setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src);
	// Set builtin parameters
	virtual void			setUniformMatrix(TProgram program, uint index, TMatrix matrix, TTransform transform);
	virtual void			setUniformFog(TProgram program, uint index);
    // Set feature parameters
	virtual bool			setUniformDriver(TProgram program); // set all driver-specific features params (based on program->features->DriverFlags)
	virtual bool			setUniformMaterial(TProgram program, CMaterial &material); // set all material-specific feature params (based on program->features->MaterialFlags)
	virtual void			setUniformParams(TProgram program, CGPUProgramParams &params); // set all user-provided params from the storage
	virtual bool			isUniformProgramState() { return false; }
	// @}





	virtual void			enableVertexProgramDoubleSidedColor(bool doubleSided);
	virtual bool		    supportVertexProgramDoubleSidedColor() const;

	// Occlusion query
	virtual bool			supportOcclusionQuery() const;
	virtual IOcclusionQuery *createOcclusionQuery();
	virtual void			deleteOcclusionQuery(IOcclusionQuery *oq);



	/** Shader implementation
	  *
	  * Shader can assume this states are setuped:
	  * TextureTransformFlags[n] = DISABLE;
	  * TexCoordIndex[n] = n;
	  * ColorOp[n] = DISABLE;
	  * AlphaOp[n] = DISABLE;
    */
	bool			activeShader(CD3DShaderFX *shd);

	// Bench
	virtual void startBench (bool wantStandardDeviation = false, bool quick = false, bool reset = true);
	virtual void endBench ();
	virtual void displayBench (class NLMISC::CLog *log);


	virtual void			setCullMode(TCullMode cullMode);
	virtual	TCullMode       getCullMode() const;

	virtual void			enableStencilTest(bool enable);
	virtual bool			isStencilTestEnabled() const;
	virtual void			stencilFunc(TStencilFunc stencilFunc, int ref, uint mask);
	virtual void			stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass);
	virtual void			stencilMask(uint mask);

	uint32					getMaxVertexIndex() const { return _MaxVertexIndex; }

		// *** Inline info
	uint					inlGetNumTextStages() const { return _NbNeLTextureStages; }

//private:
public:

	// Hardware render variables, like matrices, render states
	struct CRenderVariable
	{
		CRenderVariable()
		{
			NextModified = NULL;
			Modified = false;
		}

		// Type of render state
		enum
		{
			RenderState = 0,
			TextureState,
			TextureIndexState,
			TexturePtrState,
			VertexProgramPtrState,
			PixelShaderPtrState,
			VertexProgramConstantState,
			PixelShaderConstantState,
			SamplerState,
			MatrixState,
			VBState,
			IBState,
			VertexDecl,
			LightState,
			RenderTargetState,
		}				Type;
		CRenderVariable	*NextModified;
		bool			Modified;
		virtual	void apply(CDriverD3D *driver) = 0;
	};

	// Render state
	struct CRenderState : public CRenderVariable
	{
		CRenderState()
		{
			Type = RenderState;
			Value= 0;
			ValueSet = false;
		}
		D3DRENDERSTATETYPE	StateID;
		DWORD				Value;
		bool				ValueSet;
		virtual	void apply(CDriverD3D *driver);
	};

	// Render texture state
	struct CTextureState : public CRenderVariable
	{
		CTextureState()
		{
			Type = TextureState;
			DeviceValue = 0xcccccccc;
		}
		DWORD						StageID;
		D3DTEXTURESTAGESTATETYPE	StateID;
		DWORD						Value;
		DWORD                       DeviceValue;
		virtual	void apply(CDriverD3D *driver);
	};


	// Render texture index state
	struct CTextureIndexState : public CRenderVariable
	{
		CTextureIndexState()
		{
			Type = TextureIndexState;
		}
		DWORD						StageID;
		DWORD						TexGenMode;
		DWORD						UVChannel;
		bool						TexGen;
		virtual void apply(CDriverD3D *driver);
	};

	// Render texture
	struct CTexturePtrState : public CRenderVariable
	{
		CTexturePtrState()
		{
			Type = TexturePtrState;
			Texture = NULL;
		}
		DWORD						StageID;
		LPDIRECT3DBASETEXTURE9		Texture;
		virtual void apply(CDriverD3D *driver);
	};

	// Render texture
	struct CVertexProgramPtrState : public CRenderVariable
	{
		CVertexProgramPtrState()
		{
			Type = VertexProgramPtrState;
			VertexProgram = NULL;
		}
		LPDIRECT3DVERTEXSHADER9		VertexProgram;
		// for debug
		const CVertexProgram        *VP;
		virtual void apply(CDriverD3D *driver);
	};

	// Render texture
	struct CPixelShaderPtrState : public CRenderVariable
	{
		CPixelShaderPtrState()
		{
			Type = PixelShaderPtrState;
			PixelShader = NULL;
		}
		LPDIRECT3DPIXELSHADER9		PixelShader;
		virtual void apply(CDriverD3D *driver);
	};

	// Vertex buffer constants state
	struct CVertexProgramConstantState : public CRenderVariable
	{
		CVertexProgramConstantState()
		{
			Type = VertexProgramConstantState;
		}
		enum
		{
			Float= 0,
			Int,
			Undef,
		}							ValueType;
		uint						StateID;
		DWORD						Values[4];
		virtual void apply(CDriverD3D *driver);
	};

	// Pixel shader constants state
	struct CPixelShaderConstantState : public CRenderVariable
	{
		CPixelShaderConstantState()
		{
			Type = PixelShaderConstantState;
		}
		enum
		{
			Float= 0,
			Int,
			Undef,
		}							ValueType;
		uint						StateID;
		DWORD						Values[4];
		virtual void apply(CDriverD3D *driver);
	};

	// Render sampler state
	struct CSamplerState : public CRenderVariable
	{
		CSamplerState()
		{
			Type = SamplerState;
		}
		DWORD						SamplerID;
		D3DSAMPLERSTATETYPE			StateID;
		DWORD						Value;
		virtual void apply(CDriverD3D *driver);
	};

	// Render matrix
	struct CMatrixState : public CRenderVariable
	{
		CMatrixState()
		{
			Type = MatrixState;
		}
		D3DTRANSFORMSTATETYPE	TransformType;
		D3DXMATRIX				Matrix;
		virtual void apply(CDriverD3D *driver);
	};

	// Render vertex buffer
	struct CVBState : public CRenderVariable
	{
		CVBState()
		{
			Type = VBState;
			VertexBuffer = NULL;
			ColorOffset = 0;
		}
		IDirect3DVertexBuffer9			*VertexBuffer;
		UINT							Offset;
		UINT							Stride;
		CVertexBuffer::TPreferredMemory	PrefferedMemory;
		DWORD							Usage; // d3d vb usage
		uint							ColorOffset; // Fix for Radeon 7xxx series (see remark in CDriverD3D::createVertexDeclaration)
		virtual void apply(CDriverD3D *driver);
	};

	// Render index buffer
	struct CIBState : public CRenderVariable
	{
		CIBState()
		{
			Type = IBState;
			IndexBuffer = NULL;
		}
		IDirect3DIndexBuffer9			*IndexBuffer;
		virtual void apply(CDriverD3D *driver);
	};

	// Render vertex decl
	struct CVertexDeclState : public CRenderVariable
	{
		CVertexDeclState()
		{
			Type = VertexDecl;
			Decl = NULL;
			DeclAliasDiffuseToSpecular = NULL;
			DeclNoDiffuse = NULL;
			Stride= 0;
			AliasDiffuseToSpecular = false;
			EnableVertexColor = false;
		}
		IDirect3DVertexDeclaration9		*Decl;
		IDirect3DVertexDeclaration9		*DeclAliasDiffuseToSpecular;
		IDirect3DVertexDeclaration9		*DeclNoDiffuse;
		uint							Stride;
		bool							AliasDiffuseToSpecular;
		bool							EnableVertexColor;
		virtual void apply(CDriverD3D *driver);
	};

	// Render vertex buffer
	struct CLightState : public CRenderVariable
	{
		CLightState()
		{
			Type = LightState;
			Enabled = false;
			SettingsTouched = false;
			EnabledTouched = true;
			Light.Type = D3DLIGHT_POINT;
			Light.Range = 1;
			Light.Falloff = 1;
			Light.Position.x = 0;
			Light.Position.y = 0;
			Light.Position.z = 0;
			Light.Direction.x = 1;
			Light.Direction.y = 0;
			Light.Direction.z = 0;
			Light.Attenuation0 = 1;
			Light.Attenuation1 = 1;
			Light.Attenuation2 = 1;
			Light.Theta = 0;
			Light.Phi = 0;
		}
		uint8				LightIndex;
		bool				SettingsTouched;
		bool				EnabledTouched;
		bool				Enabled;
		D3DLIGHT9			Light;
		virtual void apply(CDriverD3D *driver);
	};

	// Render target
	struct CRenderTargetState : public CRenderVariable
	{
		CRenderTargetState()
		{
			Type = RenderTargetState;
			Target = NULL;
			Texture = NULL;
			Level = 0;
			CubeFace = 0;
		}
		IDirect3DSurface9	*Target;
		ITexture			*Texture;
		uint8				Level;
		uint8				CubeFace;
		virtual void apply(CDriverD3D *driver);
	};

	// material state
	struct CMaterialState : public CRenderVariable
	{
		CMaterialState()
		{
			//Current.Power = -1.f;
			Current.Diffuse.r = Current.Diffuse.g = Current.Diffuse.b = Current.Diffuse.a = 0.f;
			Current.Ambient.r = Current.Ambient.g = Current.Ambient.b = Current.Ambient.a = 0.f;
			Current.Specular.r = Current.Specular.g = Current.Specular.b = Current.Specular.a = 0.f;
			Current.Emissive.r = Current.Emissive.g = Current.Emissive.b = Current.Emissive.a = 0.f;
			Current.Power = 0.f;
		}
		D3DMATERIAL9 Current;
		virtual void apply(CDriverD3D *driver);
	};


	// Friends
	friend void D3DWndProc(CDriverD3D *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	friend class CTextureDrvInfosD3D;
	friend class CVBDrvInfosD3D;
	friend class CIBDrvInfosD3D;
	friend class CVolatileVertexBuffer;
	friend class CVolatileIndexBuffer;

	// Init render states
	void initRenderVariables();

	// Reset render states
	void resetRenderVariables();

	// Replace all arguments of color / alpha operators in the pixel pipe with the given value
	void replaceAllArgument(DWORD from, DWORD to, DWORD blendOpFrom);

	// Replace all arguments of color / alpha operators in the pixel pipe at the given stage with the given value
	void replaceAllArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom);
	void replaceAllRGBArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom);
	void replaceAllAlphaArgumentAtStage(uint stage, DWORD from, DWORD to, DWORD blendOpFrom);

	// Replace a the given color / alpha op
	void replaceArgumentAtStage(D3DTEXTURESTAGESTATETYPE state, DWORD stage, DWORD from, DWORD to);

	// Setup lighting & material so that it produces the given constant color (useful to simulate a per stage constant using diffuse)
	void setupConstantDiffuseColorFromLightedMaterial(D3DCOLOR color);

	// Update all modified render states, reset current material
	void updateRenderVariables();

	// Update all modified render states, assume current material is still alive
	void updateRenderVariablesInternal();

	// Reset the driver, release the lost resources
	bool reset (const GfxMode& mode);

	// Handle window size change
	bool handlePossibleSizeChange();

public:
	// Touch a render variable
	inline void touchRenderVariable (CRenderVariable *renderVariable)
	{
		// Modified ?
		if (!renderVariable->Modified)
		{
			// Link to modified states
			renderVariable->NextModified = _ModifiedRenderState;
			_ModifiedRenderState = renderVariable;
			renderVariable->Modified = true;
		}
	}



	// Access render states
	inline void setRenderState (D3DRENDERSTATETYPE renderState, DWORD value)
	{
		H_AUTO_D3D(CDriverD3D_setRenderState);
		#ifdef NL_DEBUG
			nlassert (_DeviceInterface);
			nlassert (renderState<MaxRenderState);
		#endif
		// Ref on the state
		CRenderState &_renderState = _RenderStateCache[renderState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_RenderState, _renderState.Value != value || !_renderState.ValueSet)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_renderState.Value = value;
			_renderState.ValueSet = true;
			touchRenderVariable (&_renderState);
		}
	}


public:

	// compute wrap mode flag in D3D format for that texture
	// texture must have been setuped already
	void				setupTextureWrapMode(ITexture& tex);

	// Access texture states
	inline void setTextureState (DWORD stage, D3DTEXTURESTAGESTATETYPE textureState, DWORD value)
	{
		H_AUTO_D3D(CDriverD3D_setTextureState);
		#ifdef NL_DEBUG
			nlassert (_DeviceInterface);
			nlassert (stage<MaxTexture);
			nlassert (textureState<MaxTextureState);
		#endif
		// Ref on the state
		CTextureState &_textureState = _TextureStateCache[stage][textureState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_TextureState, _textureState.Value != value)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_textureState.Value = value;
			touchRenderVariable (&_textureState);
		}
	}



	// Access texture index states
	inline void setTextureIndexMode (DWORD stage, bool texGenEnabled, DWORD value)
	{
		H_AUTO_D3D(CDriverD3D_setTextureIndexMode);
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		CTextureIndexState &_textureState = _TextureIndexStateCache[stage];
		#ifdef NL_D3D_USE_RENDER_STATE_CACHE
			NL_D3D_CACHE_TEST(CacheTest_TextureIndexMode, _textureState.TexGen = (texGenEnabled || _textureState.TexGenMode != value))
		#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			// Ref on the state
			_textureState.TexGen = texGenEnabled;
			_textureState.TexGenMode = value;
			touchRenderVariable (&_textureState);
		}
	}


	// Access texture index states
	inline void setTextureIndexUV (DWORD stage, DWORD value)
	{
		H_AUTO_D3D(CDriverD3D_setTextureIndexUV);
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Ref on the state
		CTextureIndexState &_textureState = _TextureIndexStateCache[stage];
		#ifdef NL_D3D_USE_RENDER_STATE_CACHE
			NL_D3D_CACHE_TEST(CacheTest_TextureIndexUV, _textureState.TexGen || _textureState.UVChannel != value)
		#endif
		{
			_textureState.TexGen = false;
			_textureState.UVChannel = value;
			touchRenderVariable (&_textureState);
		}
	}

	// Access texture states
	inline void setTexture (DWORD stage, LPDIRECT3DBASETEXTURE9 texture)
	{
		H_AUTO_D3D(CDriverD3D_setTexture);
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Ref on the state
		CTexturePtrState &_textureState = _TexturePtrStateCache[stage];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_RenderState, _textureState.Texture != texture)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_textureState.Texture = texture;
			touchRenderVariable (&_textureState);
		}
	}

	// Set a NeL texture states
	inline void setTexture (DWORD stage, ITexture *texture)
	{
		H_AUTO_D3D(CDriverD3D_setTexture2);
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Set the texture parameters
		CTextureDrvInfosD3D *d3dtext = getTextureD3D(*texture);
		setTexture (stage, d3dtext->Texture);
		setSamplerState (stage, D3DSAMP_ADDRESSU, d3dtext->WrapS);
		setSamplerState (stage, D3DSAMP_ADDRESSV, d3dtext->WrapT);
		setSamplerState (stage, D3DSAMP_MAGFILTER, d3dtext->MagFilter);
		setSamplerState (stage, D3DSAMP_MINFILTER, d3dtext->MinFilter);
		setSamplerState (stage, D3DSAMP_MIPFILTER, d3dtext->MipFilter);
		setSamplerState (stage, D3DSAMP_MAXANISOTROPY, _AnisotropicFilter);

		// Profile, log the use of this texture
		if (_SumTextureMemoryUsed)
		{
			// Insert the pointer of this texture
			_TextureUsed.insert (d3dtext);
		}
	}

	// Access vertex program
	inline void setVertexProgram (LPDIRECT3DVERTEXSHADER9 vertexProgram, const CVertexProgram *vp)
	{
		H_AUTO_D3D(CDriverD3D_setVertexProgram);
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_VertexProgram, _VertexProgramCache.VertexProgram != vertexProgram)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexProgramCache.VertexProgram = vertexProgram;
			_VertexProgramCache.VP = vp;
			touchRenderVariable (&_VertexProgramCache);
		}
	}

	// Access pixel shader
	inline void setPixelShader (LPDIRECT3DPIXELSHADER9 pixelShader)
	{
		H_AUTO_D3D(CDriverD3D_setPixelShader);
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_PixelShader, _PixelShaderCache.PixelShader != pixelShader)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_PixelShaderCache.PixelShader = pixelShader;
			touchRenderVariable (&_PixelShaderCache);
		}
	}

	// Access vertex program constant
	inline void setVertexProgramConstant (uint index, const float *values)
	{
		H_AUTO_D3D(CDriverD3D_setVertexProgramConstant);
		nlassert (_DeviceInterface);
		nlassert (index<MaxVertexProgramConstantState);

		// Ref on the state
		CVertexProgramConstantState &state = _VertexProgramConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool doUpdate = ((*(float*)(state.Values+0)) != values[0]) ||
					((*(float*)(state.Values+1)) != values[1]) ||
					((*(float*)(state.Values+2)) != values[2]) ||
					((*(float*)(state.Values+3)) != values[3]) ||
					(state.ValueType != CVertexProgramConstantState::Float);
		NL_D3D_CACHE_TEST(CacheTest_VertexProgramConstant, doUpdate)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(float*)(state.Values+0) = values[0];
			*(float*)(state.Values+1) = values[1];
			*(float*)(state.Values+2) = values[2];
			*(float*)(state.Values+3) = values[3];
			state.ValueType = CVertexProgramConstantState::Float;
			touchRenderVariable (&state);
		}
	}

	// Access vertex program constant
	inline void setVertexProgramConstant (uint index, const int *values)
	{
		H_AUTO_D3D(CDriverD3D_setVertexProgramConstant);
		nlassert (_DeviceInterface);
		nlassert (index<MaxVertexProgramConstantState);

		// Ref on the state
		CVertexProgramConstantState &state = _VertexProgramConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool doUpdate =	((*(int*)(state.Values+0)) != values[0]) ||
					((*(int*)(state.Values+1)) != values[1]) ||
					((*(int*)(state.Values+2)) != values[2]) ||
					((*(int*)(state.Values+3)) != values[3]) ||
					(state.ValueType != CVertexProgramConstantState::Int);
		NL_D3D_CACHE_TEST(CacheTest_VertexProgramConstant, doUpdate)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(int*)(state.Values+0) = values[0];
			*(int*)(state.Values+1) = values[1];
			*(int*)(state.Values+2) = values[2];
			*(int*)(state.Values+3) = values[3];
			state.ValueType = CVertexProgramConstantState::Int;
			touchRenderVariable (&state);
		}
	}

	// Access pixel shader constant
	inline void setPixelShaderConstant (uint index, const float *values)
	{
		H_AUTO_D3D(CDriverD3D_setPixelShaderConstant);
		nlassert (_DeviceInterface);
		nlassert (index<MaxPixelShaderConstantState);

		// Ref on the state
		CPixelShaderConstantState &state = _PixelShaderConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool doUpdate =	((*(float*)(state.Values+0)) != values[0]) ||
					((*(float*)(state.Values+1)) != values[1]) ||
					((*(float*)(state.Values+2)) != values[2]) ||
					((*(float*)(state.Values+3)) != values[3]) ||
					(state.ValueType != CPixelShaderConstantState::Float);
		NL_D3D_CACHE_TEST(CacheTest_PixelShaderConstant, doUpdate)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(float*)(state.Values+0) = values[0];
			*(float*)(state.Values+1) = values[1];
			*(float*)(state.Values+2) = values[2];
			*(float*)(state.Values+3) = values[3];
			state.ValueType = CPixelShaderConstantState::Float;
			touchRenderVariable (&state);
		}
	}

	// Access vertex program constant
	inline void setPixelShaderConstant (uint index, const int *values)
	{
		H_AUTO_D3D(CDriverD3D_setPixelShaderConstant);
		nlassert (_DeviceInterface);
		nlassert (index<MaxPixelShaderConstantState);

		// Ref on the state
		CPixelShaderConstantState &state = _PixelShaderConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool doUpdate =	((*(int*)(state.Values+0)) != values[0]) ||
					((*(int*)(state.Values+1)) != values[1]) ||
					((*(int*)(state.Values+2)) != values[2]) ||
					((*(int*)(state.Values+3)) != values[3]) ||
					(state.ValueType != CPixelShaderConstantState::Int);
		NL_D3D_CACHE_TEST(CacheTest_PixelShaderConstant, doUpdate)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(int*)(state.Values+0) = values[0];
			*(int*)(state.Values+1) = values[1];
			*(int*)(state.Values+2) = values[2];
			*(int*)(state.Values+3) = values[3];
			state.ValueType = CPixelShaderConstantState::Int;
			touchRenderVariable (&state);
		}
	}

	// Access sampler states
	inline void setSamplerState (DWORD sampler, D3DSAMPLERSTATETYPE samplerState, DWORD value)
	{
		H_AUTO_D3D(CDriverD3D_setSamplerState);
		nlassert (_DeviceInterface);
		nlassert (sampler<MaxSampler);
		nlassert ((int)samplerState<(int)MaxSamplerState);

		// Ref on the state
		CSamplerState &_samplerState = _SamplerStateCache[sampler][samplerState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_SamplerState, _samplerState.Value != value)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_samplerState.Value = value;
			touchRenderVariable (&_samplerState);
		}
	}

	// Set the vertex buffer
	inline void setVertexBuffer (IDirect3DVertexBuffer9 *vertexBuffer, UINT offset, UINT stride, bool useVertexColor, uint size, CVertexBuffer::TPreferredMemory pm, DWORD usage, uint colorOffset)
	{
		H_AUTO_D3D(CDriverD3D_setVertexBuffer);
		nlassert (_DeviceInterface);

		// Ref on the state
	#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		//NL_D3D_CACHE_TEST(CacheTest_VertexBuffer, (_VertexBufferCache.VertexBuffer != vertexBuffer) || (_VertexBufferCache.Offset != offset) || (stride != _VertexBufferCache.Stride) || (colorOffset != _VertexBufferCache.ColorOffset))
		NL_D3D_CACHE_TEST(CacheTest_VertexBuffer, (_VertexBufferCache.VertexBuffer != vertexBuffer) || (_VertexBufferCache.Offset != offset) || (stride != _VertexBufferCache.Stride) || (colorOffset != _VertexBufferCache.ColorOffset))

	#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexBufferCache.VertexBuffer = vertexBuffer;
			_VertexBufferCache.Offset = 0;
			_VertexBufferCache.Stride = stride;
			_VertexBufferCache.PrefferedMemory = pm;
			_VertexBufferCache.Usage = usage;
			_VertexBufferCache.ColorOffset = colorOffset;
			touchRenderVariable (&_VertexBufferCache);

			/* Work around for a NVIDIA bug in driver 53.03 - 56.72
			 * Sometime, after a lock D3DLOCK_NOOVERWRITE, the D3DTSS_TEXCOORDINDEX state seams to change.
			 * So, force it at every vertex buffer set.
			**/
			if (_IsGeforce)
			{
				touchRenderVariable (&_TextureStateCache[0][D3DTSS_TEXCOORDINDEX]);
				touchRenderVariable (&_TextureStateCache[1][D3DTSS_TEXCOORDINDEX]);
				touchRenderVariable (&_TextureStateCache[2][D3DTSS_TEXCOORDINDEX]);
				touchRenderVariable (&_TextureStateCache[3][D3DTSS_TEXCOORDINDEX]);
			}
		}
		_UseVertexColor = useVertexColor;
		_VertexBufferSize = size;
		_VertexBufferOffset = offset;
	}

	// Force to alias diffuse color to specular color in the current vertex buffer
	inline void setAliasDiffuseToSpecular(bool enable)
	{
		H_AUTO_D3D(CDriverD3D_setAliasDiffuseToSpecular);
		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_VertexBuffer, enable != _VertexDeclCache.AliasDiffuseToSpecular)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexDeclCache.AliasDiffuseToSpecular = enable;
			touchRenderVariable (&_VertexDeclCache);
		}
	}

	// Enable / disable vertex color from the vertex declaration
	inline void setEnableVertexColor(bool enable)
	{
		H_AUTO_D3D(CDriverD3D_setEnableVertexColor);
		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_VertexBuffer, enable != _VertexDeclCache.EnableVertexColor)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexDeclCache.EnableVertexColor = enable;
			touchRenderVariable (&_VertexDeclCache);
			touchRenderVariable (&_VertexBufferCache);
		}
	}

	// Set the index buffer
	inline void setIndexBuffer (IDirect3DIndexBuffer9 *indexBuffer, uint offset)
	{
		H_AUTO_D3D(CDriverD3D_setIndexBuffer);
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_IndexBuffer, _IndexBufferCache.IndexBuffer != indexBuffer)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_IndexBufferCache.IndexBuffer = indexBuffer;
			touchRenderVariable (&_IndexBufferCache);
		}
		_IndexBufferOffset = offset;
	}

	// Set the vertex declaration
	inline void setVertexDecl (IDirect3DVertexDeclaration9  *vertexDecl, IDirect3DVertexDeclaration9  *vertexDeclAliasDiffuseToSpecular, IDirect3DVertexDeclaration9  *vertexDeclNoDiffuse, uint stride)
	{
		H_AUTO_D3D(CDriverD3D_setVertexDecl);
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_VertexDecl, stride != _VertexDeclCache.Stride || _VertexDeclCache.Decl != vertexDecl || _VertexDeclCache.DeclAliasDiffuseToSpecular != vertexDeclAliasDiffuseToSpecular || vertexDeclNoDiffuse != _VertexDeclCache.DeclNoDiffuse)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexDeclCache.Decl = vertexDecl;
			_VertexDeclCache.DeclAliasDiffuseToSpecular = vertexDeclAliasDiffuseToSpecular;
			_VertexDeclCache.Stride = stride;
			_VertexDeclCache.DeclNoDiffuse = vertexDeclNoDiffuse;
			touchRenderVariable (&_VertexDeclCache);
		}
	}

	// Access matrices
	inline uint remapMatrixIndex (D3DTRANSFORMSTATETYPE type)
	{
		H_AUTO_D3D(CDriverD3D_remapMatrixIndex);
		if (type>=256)
			return (D3DTRANSFORMSTATETYPE)(MatrixStateRemap + type - 256);
		else
			return (uint)type;
	}
	inline void setMatrix (D3DTRANSFORMSTATETYPE type, const D3DXMATRIX &matrix)
	{
		H_AUTO_D3D(CDriverD3D_setMatrix);
		nlassert (_DeviceInterface);

		// Remap high matrices indexes
		type = (D3DTRANSFORMSTATETYPE)remapMatrixIndex (type);
		nlassert ((int)type<(int)MaxMatrixState);

		CMatrixState &theMatrix = _MatrixCache[type];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool doUpdate = (matrix._11 != theMatrix.Matrix._11) ||
			(matrix._12 != theMatrix.Matrix._12) ||
			(matrix._13 != theMatrix.Matrix._13) ||
			(matrix._14 != theMatrix.Matrix._14) ||
			(matrix._21 != theMatrix.Matrix._21) ||
			(matrix._22 != theMatrix.Matrix._22) ||
			(matrix._23 != theMatrix.Matrix._23) ||
			(matrix._24 != theMatrix.Matrix._24) ||
			(matrix._31 != theMatrix.Matrix._31) ||
			(matrix._32 != theMatrix.Matrix._32) ||
			(matrix._33 != theMatrix.Matrix._33) ||
			(matrix._34 != theMatrix.Matrix._34) ||
			(matrix._41 != theMatrix.Matrix._41) ||
			(matrix._42 != theMatrix.Matrix._42) ||
			(matrix._43 != theMatrix.Matrix._43) ||
			(matrix._44 != theMatrix.Matrix._44);
		NL_D3D_CACHE_TEST(CacheTest_Matrix, doUpdate)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			theMatrix.Matrix = matrix;
			touchRenderVariable (&theMatrix);
		}
	}

	// Access texture states
	inline void setRenderTarget (IDirect3DSurface9 *target, ITexture *texture, uint8 level, uint8 cubeFace)
	{
		H_AUTO_D3D(CDriverD3D_setRenderTarget);
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		NL_D3D_CACHE_TEST(CacheTest_RenderTarget, _RenderTarget.Target != target)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_RenderTarget.Target = target;
			_RenderTarget.Texture = texture;
			_RenderTarget.Level = level;
			_RenderTarget.CubeFace = cubeFace;

			touchRenderVariable (&_RenderTarget);

			_ScissorTouched = true;
		}
	}


public:
	void setMaterialState(const D3DMATERIAL9 &material)
	{
		H_AUTO_D3D(setMaterialState);
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		bool update = material.Power != _MaterialState.Current.Power ||
			material.Ambient != _MaterialState.Current.Ambient ||
			material.Emissive != _MaterialState.Current.Emissive ||
			material.Diffuse != _MaterialState.Current.Diffuse ||
			material.Specular != _MaterialState.Current.Specular;
		NL_D3D_CACHE_TEST(CacheTest_MaterialState, update)
#endif
		{
			_MaterialState.Current = material;
			touchRenderVariable(&_MaterialState);
		}
	}




	// Get the d3dtext mirror of an existing setuped texture.
	static	inline CTextureDrvInfosD3D*	getTextureD3D(ITexture& tex)
	{
		H_AUTO_D3D(CDriverD3D_getTextureD3D);
		CTextureDrvInfosD3D*	d3dtex;
		d3dtex= (CTextureDrvInfosD3D*)(ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
		return d3dtex;
	}

	// Get the d3dtext mirror of an existing setuped pixel program.
	static	inline CPixelProgramDrvInfosD3D*	getPixelProgramD3D(CPixelProgram& pixelProgram)
	{
		H_AUTO_D3D(CDriverD3D_getPixelProgramD3D);
		CPixelProgramDrvInfosD3D*	d3dPixelProgram;
		d3dPixelProgram = (CPixelProgramDrvInfosD3D*)(IProgramDrvInfos*)(pixelProgram.m_DrvInfo);
		return d3dPixelProgram;
	}

	// Get the d3dtext mirror of an existing setuped vertex program.
	static	inline CVertexProgamDrvInfosD3D*	getVertexProgramD3D(CVertexProgram& vertexProgram)
	{
		H_AUTO_D3D(CDriverD3D_getVertexProgramD3D);
		CVertexProgamDrvInfosD3D*	d3dVertexProgram;
		d3dVertexProgram = (CVertexProgamDrvInfosD3D*)(IProgramDrvInfos*)(vertexProgram.m_DrvInfo);
		return d3dVertexProgram;
	}

	// *** Init helper

	bool isDepthFormatOk(UINT adapter, D3DDEVTYPE rasterizer, D3DFORMAT DepthFormat, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat);
	bool isTextureFormatOk(UINT adapter, D3DDEVTYPE rasterizer, D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat);
	bool fillPresentParameter (D3DPRESENT_PARAMETERS &parameters, D3DFORMAT &adapterFormat, const GfxMode& mode, const D3DDISPLAYMODE &adapterMode);

	// *** Texture helper

	D3DFORMAT getD3DDestTextureFormat (ITexture& tex);
	// Generates the texture, degades it, creates / resizes the d3d surface. Don't fill the surface.
	bool generateD3DTexture (ITexture& tex, bool textureDegradation, D3DFORMAT &destFormat, D3DFORMAT &srcFormat, bool &cube);
	// Return the memory used by the surface described iwith the parameters
	uint32 computeTextureMemoryUsage (uint width, uint height, uint levels, D3DFORMAT destFormat, bool cube);
	// Upload a texture part
	bool uploadTextureInternal (ITexture& tex, NLMISC::CRect& rect, uint8 destMipmap, uint8 srcMipmap, D3DFORMAT destFormat, D3DFORMAT srcFormat);

	// *** Matrix helper

	// Update _D3DModelView and _D3DModelViewProjection matrices. Call this if the model, the projection or the view matrix are modified
	void updateMatrices ();
	// Update the projection matrix. Call this if the driver resolution, the viewport or the frustum changes.
	void updateProjectionMatrix ();

	// *** Vertex buffer helper

	// Create a vertex declaration
	bool createVertexDeclaration (uint16 vertexFormat, const uint8 *typeArray,
								  IDirect3DVertexDeclaration9 **vertexDecl,
								  uint &colorOffset,
								  bool aliasDiffuseToSpecular,
								  bool bypassDiffuse,
								  uint *stride = NULL
								 );

	// *** Index buffer helper

	// *** Multipass helpers

	void initInternalShaders();
	void releaseInternalShaders();
	bool setShaderTexture (uint textureHandle, ITexture *texture, CFXCache *cache);

	bool validateShader(CD3DShaderFX *shader);

	void activePass (uint pass)
	{
		H_AUTO_D3D(CDriverD3D_activePass);
		if (_CurrentShader)
		{
			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			if (_CurrentMaterialInfo->FXCache)
			{
				nlassert(_CurrentMaterialInfo);
				_CurrentMaterialInfo->FXCache->applyPass(*this, drvInfo, pass);
			}
			else
			{
#if (DIRECT3D_VERSION >= 0x0900) && (D3D_SDK_VERSION >= 32)
				drvInfo->Effect->BeginPass (pass);
				drvInfo->Effect->EndPass ();
#else
				drvInfo->Effect->Pass (pass);
#endif
			}
		}

		// Update render states
		updateRenderVariablesInternal();
	}

	void beginMultiPass ()
	{
		H_AUTO_D3D(CDriverD3D_beginMultiPass);
		if (_CurrentShader)
		{
			// Does the shader validated ?
			validateShader(_CurrentShader);

			// Init default state value
			uint i;
			for (i=0; i<MaxTexture; i++)
			{
				setTextureState (i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
				setTextureIndexMode (i, false, D3DTSS_TCI_PASSTHRU);
				setTextureIndexUV (i, i);
				setTextureState (i, D3DTSS_COLOROP, D3DTOP_DISABLE);
				//setTextureState (i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}

			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			if (_CurrentMaterialInfo->FXCache)
			{
				nlassert(_CurrentMaterialInfo);
				_CurrentMaterialInfo->FXCache->begin(drvInfo, this);
			}
			else
			{
				drvInfo->Effect->Begin (&_CurrentShaderPassCount, D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESHADERSTATE);
			}
		}
		else
			// No shader setuped
			_CurrentShaderPassCount = 1;
	}

	void endMultiPass ()
	{
		H_AUTO_D3D(CDriverD3D_endMultiPass);
		if (_CurrentShader)
		{
			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			if (_CurrentMaterialInfo->FXCache)
			{
				_CurrentMaterialInfo->FXCache->end(drvInfo);
			}
			else
			{
				drvInfo->Effect->End ();
			}
		}
	}

	// render helpers
	bool renderPrimitives(D3DPRIMITIVETYPE primitiveType, uint numVertexPerPrim, CMaterial& mat, uint firstVertex, uint32 nPrims);
	bool renderIndexedPrimitives(D3DPRIMITIVETYPE primitiveType, uint numVertexPerPrim, CMaterial& mat, uint32 firstIndex, uint32 nPrims, uint indexOffset = 0);
	bool renderSimpleIndexedPrimitives(D3DPRIMITIVETYPE primitiveType, uint numVertexPerPrim, uint32 firstIndex, uint32 nPrims, uint indexOffset = 0);
	void convertToIndices16(uint firstIndex, uint numIndices);

	// Returns true if this material needs a constant color for the diffuse component
	static bool needsConstantForDiffuse (CMaterial &mat);

	/* Returns true if this normal needs constant. If true, numConstant is the number of needed constants, firstConstant is the first
	constants needed. */
	static bool needsConstants (uint &numConstant, uint &firstConstant, uint &secondConstant, CMaterial &mat);

	// For normal shader, compute part of the pipeline that are worth setting
	// For example : if alpha output if not used (no alpha test or blend), then the alpha part won't be set at all
	static void computeRelevantTexEnv(CMaterial &mat, bool rgbPipe[IDRV_MAT_MAXTEXTURES], bool alphaPipe[IDRV_MAT_MAXTEXTURES]);

	// Build a pixel shader for normal shader
	IDirect3DPixelShader9	*buildPixelShader (const CNormalShaderDesc &normalShaderDesc, bool unlightedNoVertexColor);


	// Notify all the shaders drivers info that a device has been lost
	void notifyAllShaderDrvOfLostDevice();
	// Notify all the shaders drivers info that a device has been reset
	void notifyAllShaderDrvOfResetDevice();

	// *** Debug helpers

	void setDebugMaterial();

	// ** From ID3DXEffectStateManager
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
	ULONG STDMETHODCALLTYPE AddRef(VOID);
	ULONG STDMETHODCALLTYPE Release(VOID);
	HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable);
	HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF);
	HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT9* pLight);
	HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial);
	HRESULT STDMETHODCALLTYPE SetNPatchMode(FLOAT nSegments);
	HRESULT STDMETHODCALLTYPE SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTexture (DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
	HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	HRESULT STDMETHODCALLTYPE SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);
private:

	void findNearestFullscreenVideoMode();

	TShaderDrvInfoPtrList	_ShaderDrvInfos;

	// Windows
	std::wstring			_WindowClass;
	HWND					_HWnd;
	sint32					_WindowX;
	sint32					_WindowY;
	bool					_DestroyWindow;
	bool					_Maximized;
	bool					_HandlePossibleSizeChangeNextSize;
	GfxMode					_CurrentMode;
	uint					_Interval;
	bool					_FullScreen;

	// cursors
	enum TColorDepth { ColorDepth16 = 0, ColorDepth32, ColorDepthCount };

	TColorDepth				_ColorDepth;
	std::string				_CurrName;
	NLMISC::CRGBA			_CurrCol;
	uint8					_CurrRot;
	uint					_CurrHotSpotX;
	uint					_CurrHotSpotY;
	float					_CursorScale;

	nlCursor				_DefaultCursor;

	bool					_AlphaBlendedCursorSupported;
	bool					_AlphaBlendedCursorSupportRetrieved;

	class CCursor
	{
	public:
		NLMISC::CBitmap Src;
		TColorDepth		ColorDepth;
		uint			OrigHeight;
		float			HotspotScale;
		uint			HotspotOffsetX;
		uint			HotspotOffsetY;
		sint			HotSpotX;
		sint			HotSpotY;
		nlCursor		Cursor;
		NLMISC::CRGBA	Col;
		uint8			Rot;
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
		Display			*Dpy;
#endif
	public:
		CCursor();
		~CCursor();
		CCursor& operator= (const CCursor& from);

		void reset();
	};

	struct CStrCaseUnsensitiveCmp
	{
		bool operator()(const std::string &lhs, const std::string &rhs) const
		{
			return NLMISC::nlstricmp(lhs, rhs) < 0;
		}
	};

	typedef std::map<std::string, CCursor, CStrCaseUnsensitiveCmp> TCursorMap;

	TCursorMap					_Cursors;

	// Directx
	uint32					_Adapter;
	D3DDEVTYPE				_Rasterizer;
	LPDIRECT3D9				_D3D;
public:
	IDirect3DDevice9		*_DeviceInterface;
private:

	// Events
	NLMISC::CEventEmitterMulti	_EventEmitter; // this can contains a win emitter and eventually a direct input emitter

	// Some matrices (Local -> Model -> World -> Screen)
public:
	CVector					_PZBCameraPos;
private:
	CMatrix					_UserModelMtx;
	CMatrix					_UserViewMtx;
public:
	CViewport				_Viewport;
private:
	D3DVIEWPORT9			_D3DViewport;
public:
	CScissor				_Scissor;
private:
	float					_OODeltaZ;	// Backup znear and zfar
	D3DXMATRIX				_D3DSpecularWorldTex;		// World (as in NeL) to model matrix.
	D3DXMATRIX				_D3DModelView;				// Local to world (as in D3D) matrix.
	D3DXMATRIX				_D3DModelViewProjection;	// Local to screen (as in D3D) matrix.
	D3DXMATRIX				_D3DInvModelView;			// World (as in DX) to local matrix.
	bool					_ForceNormalize;
	bool					_FrustumPerspective;
	bool					_InvertCullMode;
	bool					_DoubleSided;
	float					_FrustumLeft;
	float					_FrustumRight;
	float					_FrustumTop;
	float					_FrustumBottom;
	float					_FrustumZNear;
	float					_FrustumZFar;
	float					_FogStart;
	float					_FogEnd;

	// Vertex memory available
	uint32					_AGPMemoryAllocated;
	uint32					_VRAMMemoryAllocated;

	// Textures caps
	D3DFORMAT				_PreferedTextureFormat[ITexture::UploadFormatCount];
	uint					_ForceTextureResizePower;
	bool					_ForceDXTCCompression:1;
	bool					_TextureCubeSupported;
	bool					_VertexProgram;
	bool					_PixelProgram;
	uint16					_PixelProgramVersion;
	bool					_DisableHardwareVertexProgram;
	bool					_DisableHardwarePixelProgram;
	bool					_DisableHardwareVertexArrayAGP;
	bool					_DisableHardwareIndexArrayAGP;
	bool					_DisableHardwarePixelShader;
	bool					_MADOperatorSupported;
	bool					_EMBMSupported;
	bool					_CubbedMipMapSupported;
	bool					_IsGeforce;
	bool					_NonPowerOfTwoTexturesSupported;
	uint					_MaxAnisotropy;
	bool					_AnisotropicMinSupported;
	bool					_AnisotropicMagSupported;
	bool					_AnisotropicMinCubeSupported;
	bool					_AnisotropicMagCubeSupported;
	uint					_NbNeLTextureStages;			// Number of texture stage for NeL (max IDRV_MAT_MAXTEXTURES)
	uint					_MaxVerticesByVertexBufferHard;
	uint					_MaxLight;
	uint32					_PixelShaderVersion;
	uint32					_MaxPrimitiveCount;
	uint32					_MaxVertexIndex;
	uint					_MaxNumPerStageConstantLighted;
	uint					_MaxNumPerStageConstantUnlighted;

	// Profiling
	CPrimitiveProfile									_PrimitiveProfileIn;
	CPrimitiveProfile									_PrimitiveProfileOut;
	uint32												_AllocatedTextureMemory;
	uint32												_NbSetupMaterialCall;
	uint32												_NbSetupModelMatrixCall;
	bool												_SumTextureMemoryUsed;
	std::set<CTextureDrvInfosD3D*>						_TextureUsed;

	// VBHard Lock Profiling
	struct	CVBHardProfile
	{
		NLMISC::CRefPtr<CVertexBuffer>			VBHard;
		NLMISC::TTicks							AccumTime;
		// true if the VBHard was not always the same for the same chronogical place.
		bool									Change;
		CVBHardProfile()
		{
			AccumTime= 0;
			Change= false;
		}
	};

	// Index buffer lock profiling
	struct	CIBProfile
	{
		NLMISC::CRefPtr<CIndexBuffer>			IB;
		NLMISC::TTicks							AccumTime;
		// true if the VBHard was not always the same for the same chronogical place.
		bool									Change;
		CIBProfile()
		{
			AccumTime= 0;
			Change= false;
		}
	};

	// The vb hard Profiles in chronogical order.
	bool												_VBHardProfiling;
	std::vector<CVBHardProfile>							_VBHardProfiles;
	uint												_CurVBHardLockCount;
	uint												_NumVBHardProfileFrame;
	void												appendVBHardLockProfile(NLMISC::TTicks time, CVertexBuffer *vb);

	// The index buffer profile in chronogical order.
	bool												_IBProfiling;
	std::vector<CIBProfile>								_IBProfiles;
	uint												_CurIBLockCount;
	uint												_NumIBProfileFrame;
public:
	NLMISC::TTicks										_VolatileIBLockTime;
	NLMISC::TTicks										_VolatileVBLockTime;
private:
	void												appendIBLockProfile(NLMISC::TTicks time, CIndexBuffer *ib);

	// VBHard profile
	std::set<CVBDrvInfosD3D*>							_VertexBufferHardSet;

	// The render variables
public:
	CRenderState			_RenderStateCache[MaxRenderState];
	CTextureState			_TextureStateCache[MaxTexture][MaxTextureState];
	CTextureIndexState		_TextureIndexStateCache[MaxTexture];
	CTexturePtrState		_TexturePtrStateCache[MaxTexture];
	CSamplerState			_SamplerStateCache[MaxSampler][MaxSamplerState];
	CMatrixState			_MatrixCache[MaxMatrixState];
	CVertexProgramPtrState	_VertexProgramCache;
	CPixelShaderPtrState	_PixelShaderCache;
	CVertexProgramConstantState	_VertexProgramConstantCache[MaxVertexProgramConstantState];
	CPixelShaderConstantState	_PixelShaderConstantCache[MaxPixelShaderConstantState];
	CVertexDeclState		_VertexDeclCache;
	CLightState				_LightCache[MaxLight];
	CRenderTargetState		_RenderTarget;
	CMaterialState			_MaterialState;
private:

	// last activation of vertex buffer / index buffer
	NLMISC::CRefPtr<CIBDrvInfosD3D>  _LastIndexBufferInfo;

	// Vertex buffer cache
	CVBState				_VertexBufferCache;
	uint					_VertexBufferSize;
	uint					_VertexBufferOffset;
public:
	bool					_UseVertexColor;
private:

	// Index buffer cache
	CIBState				_IndexBufferCache;
	uint					_IndexBufferOffset;		 // Current index buffer offset
	CIndexBuffer::TFormat	_CurrIndexBufferFormat;  // updated at call to activeIndexBuffer

	// The last vertex buffer needs vertex color
	bool					_FogEnabled;

	NLMISC::CRefPtr<CVertexProgram> _VertexProgramUser;
	NLMISC::CRefPtr<CPixelProgram> _PixelProgramUser;

	// *** Internal resources

	// Current render pass
	uint					_CurrentRenderPass;


	// Volatile double buffers
	CVolatileVertexBuffer	*_VolatileVertexBufferRAM[2];
	CVolatileVertexBuffer	*_VolatileVertexBufferAGP[2];
	CVolatileIndexBuffer	*_VolatileIndexBuffer16RAM[2];
	CVolatileIndexBuffer	*_VolatileIndexBuffer16AGP[2];
	CVolatileIndexBuffer	*_VolatileIndexBuffer32RAM[2];
	CVolatileIndexBuffer	*_VolatileIndexBuffer32AGP[2];

	// Special 16 bit index buffer for quads
	IDirect3DIndexBuffer9	*_QuadIB;

	// Vertex declaration list
	std::list<CVertexDeclaration>	_VertexDeclarationList;

	// Pixel shader list
	std::list<CNormalShaderDesc>	_NormalPixelShaders[2];

	// Quad indexes
	CIndexBuffer			_QuadIndexes;
	CIndexBuffer			_QuadIndexesAGP;

	// The last setuped shader
	CD3DShaderFX					*_CurrentShader;
	UINT					_CurrentShaderPassCount;
public:
	struct CTextureRef
	{
		CRefPtr<ITexture>		NeLTexture;
		LPDIRECT3DBASETEXTURE9	D3DTexture;
	};
	const std::vector<CTextureRef> &getCurrentShaderTextures() const { return _CurrentShaderTextures; }
private:
	std::vector<CTextureRef>	_CurrentShaderTextures;

	// The last material setuped
	CMaterial				*_CurrentMaterial;
public:
	CMaterialDrvInfosD3D	*_CurrentMaterialInfo;
private:

	// Optim: To not test change in Materials states if just texture has changed. Very useful for landscape.
	uint32					_MaterialAllTextureTouchedFlag;

	// The modified render variables list
	CRenderVariable			*_ModifiedRenderState;

	// Internal shaders
	CD3DShaderFX					_ShaderLightmap0;
	CD3DShaderFX					_ShaderLightmap1;
	CD3DShaderFX					_ShaderLightmap2;
	CD3DShaderFX					_ShaderLightmap3;
	CD3DShaderFX					_ShaderLightmap4;
	CD3DShaderFX					_ShaderLightmap0Blend;
	CD3DShaderFX					_ShaderLightmap1Blend;
	CD3DShaderFX					_ShaderLightmap2Blend;
	CD3DShaderFX					_ShaderLightmap3Blend;
	CD3DShaderFX					_ShaderLightmap4Blend;
	CD3DShaderFX					_ShaderLightmap0X2;
	CD3DShaderFX					_ShaderLightmap1X2;
	CD3DShaderFX					_ShaderLightmap2X2;
	CD3DShaderFX					_ShaderLightmap3X2;
	CD3DShaderFX					_ShaderLightmap4X2;
	CD3DShaderFX					_ShaderLightmap0BlendX2;
	CD3DShaderFX					_ShaderLightmap1BlendX2;
	CD3DShaderFX					_ShaderLightmap2BlendX2;
	CD3DShaderFX					_ShaderLightmap3BlendX2;
	CD3DShaderFX					_ShaderLightmap4BlendX2;
	CD3DShaderFX					_ShaderCloud;
	CD3DShaderFX					_ShaderWaterNoDiffuse;
	CD3DShaderFX					_ShaderWaterDiffuse;


	// Backup frame buffer
	IDirect3DSurface9		*_BackBuffer;

	// Static bitmap buffer to convert rgba to bgra
	static std::vector<uint8>	_TempBuffer;

	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

	uint64 _SwapBufferCounter;

	// occlusion query
	bool						_OcclusionQuerySupported;
	TOcclusionQueryList			_OcclusionQueryList;

	// depth range
	float						_DepthRangeNear;
	float						_DepthRangeFar;
	//
	bool						_ScissorTouched;
	uint8						_CurrentUVRouting[MaxTexture];
	bool						_MustRestoreLight;
	D3DXMATRIX					_D3DMatrixIdentity;
	DWORD						_FogColor;
	uint						_AnisotropicFilter;

	// stencil buffer
	bool			_CurStencilTest;
	DWORD			_CurStencilFunc;
	DWORD			_CurStencilRef;
	DWORD			_CurStencilMask;
	DWORD			_CurStencilOpFail;
	DWORD			_CurStencilOpZFail;
	DWORD			_CurStencilOpZPass;
	DWORD			_CurStencilWriteMask;

public:

	// private, for access by COcclusionQueryD3D
	COcclusionQueryD3D			*_CurrentOcclusionQuery;

	// *** Lightmap Dynamic Light
	// For Lightmap Dynamic Lighting
	CLight						_LightMapDynamicLight;
	bool						_LightMapDynamicLightEnabled;
	bool						_LightMapDynamicLightDirty;
	CMaterial::TShader			_CurrentMaterialSupportedShader;
	// this is the backup of standard lighting (cause GL states may be modified by Lightmap Dynamic Lighting)
	CLight						_UserLight0;
	bool						_UserLightEnable[MaxLight];
	// methods to enable / disable DX light, without affecting _LightMapDynamicLight*, or _UserLight0*
	void			setLightInternal(uint8 num, const CLight& light);
	void			enableLightInternal(uint8 num, bool enable);
	// on/off Lights for LightMap mode: only the first light is enabled in lightmap mode
	void			setupLightMapDynamicLighting(bool enable);

	TCullMode		_CullMode;

	bool			_Lost;
	bool			_SceneBegun;

	WORD            _DesktopGammaRamp[256 * 3];
	bool			_DesktopGammaRampValid;

	// for debug only
	static bool		_CacheTest[CacheTest_Count];

	static std::vector<uint16>  _QuadIndices; // tmp : quads indices -> to allow support of quads on devices that don't have 32 bit indices

	// reset an index buffer and force it to be reallocated
	void deleteIndexBuffer(CIBDrvInfosD3D *ib);
	// Build 16 bit index buffer for quad
	bool buildQuadIndexBuffer();

	// Test if cursor is in the client area. always true when software cursor is used and window visible
	// (displayed in software when DirectInput is used)
	bool isSystemCursorInClientArea();

	// Check if RGBA cursors are supported
	bool isAlphaBlendedCursorSupported();

	// Update cursor appearance
	void updateCursor(bool forceRebuild = false);

	// Create default cursors
	void createCursors();

	// Release all cursors
	void releaseCursors();

	// Convert a NLMISC::CBitmap to nlCursor
	bool convertBitmapToCursor(const NLMISC::CBitmap &bitmap, nlCursor &cursor, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY);

	// build a cursor from src, src should have the same size that the hardware cursor
	// or a assertion is thrown
	nlCursor buildCursor(const NLMISC::CBitmap &src, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY);

	// reset the cursor shape to the system arrow
	void setSystemArrow();

	bool convertBitmapToIcon(const NLMISC::CBitmap &bitmap, HICON &icon, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col = NLMISC::CRGBA::White, sint hotSpotX = 0, sint hotSpotY = 0, bool cursor = false);

	virtual bool copyTextToClipboard(const ucstring &text);
	virtual bool pasteTextFromClipboard(ucstring &text);

public:
	#ifdef 	NL_DEBUG
		std::set<CVBDrvInfosD3D *> _LockedBuffers;
	#endif

	emptyProc ExitFunc;

	bool beginScene()
	{
		nlassert(!_SceneBegun);
		if (_DeviceInterface->BeginScene() != D3D_OK) return false;
		_SceneBegun = true;
		return true;
	}

	bool endScene()
	{
		nlassert(_SceneBegun);
		if (_DeviceInterface->EndScene() != D3D_OK) return false;
		_SceneBegun = false;
		return true;
	}

	bool hasSceneBegun() const { return _SceneBegun; }

	// Clip the wanted rectangle with window. return true if rect is not NULL.
	bool					clipRect(NLMISC::CRect &rect);

	friend	class	IShaderDrvInfos;

	void			removeShaderDrvInfoPtr(ItShaderDrvInfoPtrList shaderIt);

};

#define NL_D3DCOLOR_RGBA(rgba) (D3DCOLOR_ARGB(rgba.A,rgba.R,rgba.G,rgba.B))
#define D3DCOLOR_NL_RGBA(rgba) (NLMISC::CRGBA((uint8)((rgba>>16)&0xff), (uint8)((rgba>>8)&0xff), (uint8)(rgba&0xff), (uint8)((rgba>>24)&0xff)))
#define NL_D3DCOLORVALUE_RGBA(dest,rgba) \
	dest.a=(float)rgba.A*(1.f/255.f);dest.r=(float)rgba.R*(1.f/255.f);dest.g=(float)rgba.G*(1.f/255.f);dest.b=(float)rgba.B*(1.f/255.f);
#define NL_D3D_MATRIX(d3d_mt,nl_mt) \
	{	const float *nl_mt_ptr = nl_mt.get(); \
	d3d_mt._11 = nl_mt_ptr[0]; \
	d3d_mt._21 = nl_mt_ptr[4]; \
	d3d_mt._31 = nl_mt_ptr[8]; \
	d3d_mt._41 = nl_mt_ptr[12]; \
	d3d_mt._12 = nl_mt_ptr[1]; \
	d3d_mt._22 = nl_mt_ptr[5]; \
	d3d_mt._32 = nl_mt_ptr[9]; \
	d3d_mt._42 = nl_mt_ptr[13]; \
	d3d_mt._13 = nl_mt_ptr[2]; \
	d3d_mt._23 = nl_mt_ptr[6]; \
	d3d_mt._33 = nl_mt_ptr[10]; \
	d3d_mt._43 = nl_mt_ptr[14]; \
	d3d_mt._14 = nl_mt_ptr[3]; \
	d3d_mt._24 = nl_mt_ptr[7]; \
	d3d_mt._34 = nl_mt_ptr[11]; \
	d3d_mt._44 = nl_mt_ptr[15]; }
// build matrix for texture 2D transform (special case in D3D)
#define NL_D3D_TEX2D_MATRIX(d3d_mt,nl_mt) \
	{	const float *nl_mt_ptr = nl_mt.get(); \
		d3d_mt._11 = nl_mt_ptr[0]; \
		d3d_mt._12 = nl_mt_ptr[1]; \
		d3d_mt._13 = nl_mt_ptr[3]; \
		d3d_mt._14 = 0.f; \
		d3d_mt._21 = nl_mt_ptr[4]; \
		d3d_mt._22 = nl_mt_ptr[5]; \
		d3d_mt._23 = nl_mt_ptr[7]; \
		d3d_mt._24 = 0.f; \
		d3d_mt._31 = nl_mt_ptr[12]; \
		d3d_mt._32 = nl_mt_ptr[13]; \
		d3d_mt._33 = nl_mt_ptr[15]; \
		d3d_mt._34 = 0.f; \
		d3d_mt._41 = 0.f; \
		d3d_mt._42 = 0.f; \
		d3d_mt._43 = 0.f; \
		d3d_mt._44 = 1.f; }






#define NL_D3DVECTOR_VECTOR(dest,vect) dest.x=(vect).x;dest.y=(vect).y;dest.z=(vect).z;
#define FTODW(f) (*((DWORD*)&(f)))
#define D3DCOLOR_FLOATS(floats,rgba) {floats[0]=(float)((rgba>>16)&0xff) * (1.f/255.f);floats[1]=(float)((rgba>>8)&0xff) * (1.f/255.f);\
	floats[2]=(float)(rgba&0xff) * (1.f/255.f);floats[3]=(float)((rgba>>24)&0xff) * (1.f/255.f);}
#define NL_FLOATS(floats,rgba) {floats[0] = (float)rgba.R * (1.f/255.f);floats[1] = (float)rgba.G * (1.f/255.f);\
	floats[2] = (float)rgba.B * (1.f/255.f);floats[3] = (float)rgba.A * (1.f/255.f);}

// ***************************************************************************
// nbPixels is pixel count, not a byte count !
inline void copyRGBA2BGRA (uint32 *dest, const uint32 *src, uint nbPixels)
{
	H_AUTO_D3D(CDriverD3D_copyRGBA2BGRA);
	while (nbPixels != 0)
	{
		register uint32 color = *src;
		*dest = (color & 0xff00ff00) | ((color&0xff)<<16) | ((color&0xff0000)>>16);
		dest++;
		src++;
		nbPixels--;
	}
}


// ***************************************************************************
// set a D3DCOLORVALUE
inline void setColor(D3DCOLORVALUE &dest, float r, float g, float b, float a)
{
	dest.r = r;
	dest.g = g;
	dest.b = b;
	dest.a = a;
}

// ***************************************************************************
// set a D3DCOLORVALUE from & D3DCOLOR
inline void setColor(D3DCOLORVALUE &dest, const D3DCOLOR &src)
{
	dest.r = (1.f / 255.f) * ((src >> 16) & 0xff);
	dest.g = (1.f / 255.f) * ((src >> 8) & 0xff);
	dest.b = (1.f / 255.f) * (src & 0xff);
	dest.a = (1.f / 255.f) * (src >> 24);
}

// ***************************************************************************
// set a D3DCOLORVALUE from a CRGBA
inline void setColor(D3DCOLORVALUE &dest, const NLMISC::CRGBA &src)
{
	dest.r = (1.f / 255.f) * src.R;
	dest.g = (1.f / 255.f) * src.G;
	dest.b = (1.f / 255.f) * src.B;
	dest.a = (1.f / 255.f) * src.A;
}

// ***************************************************************************
void fillQuadIndexes (uint16 *indexes, uint first, uint last);

} // NL3D

#ifndef NL_STATIC
extern HINSTANCE HInstDLL;
#endif

#endif // NL_DRIVER_DIRECT3D_H
