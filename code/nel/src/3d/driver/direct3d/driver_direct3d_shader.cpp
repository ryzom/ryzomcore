// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "driver_direct3d.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************

CD3DShaderFX::~CD3DShaderFX()
{
	// Must kill the drv mirror of this shader.
	_DrvInfo.kill();
}

// ***************************************************************************

CD3DShaderFX::CD3DShaderFX()
{
	_ShaderChanged = true;
}

// ***************************************************************************

void CD3DShaderFX::setText (const char *text)
{
	_Text = text;
	_ShaderChanged = true;
}

// ***************************************************************************

void CD3DShaderFX::setName (const char *name)
{
	_Name = name;
	_ShaderChanged = true;
}

// ***************************************************************************

bool CD3DShaderFX::loadShaderFile (const char *filename)
{
	_Text.clear();
	// Lookup
	string _filename = NLMISC::CPath::lookup(filename, false, true, true);
	if (!_filename.empty())
	{
		// File length
		uint size = NLMISC::CFile::getFileSize (_filename);
		_Text.reserve (size+1);

		try
		{
			NLMISC::CIFile file;
			if (file.open (_filename))
			{
				// Read it
				while (!file.eof ())
				{
					char line[512];
					file.getline (line, 512);
					_Text += line;
				}

				// Set the shader name
				_Name = NLMISC::CFile::getFilename (filename);
				return true;
			}
			else
			{
				nlwarning ("Can't open the file %s for reading", _filename.c_str());
			}
		}
		catch (const Exception &e)
		{
			nlwarning ("Error while reading %s : %s", _filename.c_str(), e.what());
		}
	}
	return false;
}

// ***************************************************************************

IShaderDrvInfos::~IShaderDrvInfos()
{
	_Driver->removeShaderDrvInfoPtr(_DriverIterator);
}

void CDriverD3D::removeShaderDrvInfoPtr(ItShaderDrvInfoPtrList shaderIt)
{
	_ShaderDrvInfos.erase(shaderIt);
}

// mem allocator for state records
std::allocator<uint8> CStateRecord::Allocator;


// ***************************************************************************
// The state manager with cache
// ***************************************************************************

HRESULT CDriverD3D::QueryInterface(REFIID /* riid */, LPVOID * /* ppvObj */)
{
	H_AUTO_D3D(CDriverD3D_QueryInterface)
	return D3D_OK;
}

// ***************************************************************************

ULONG CDriverD3D::AddRef(VOID)
{
	H_AUTO_D3D(CDriverD3D_AddRef)
	return 0;
}

// ***************************************************************************

ULONG CDriverD3D::Release(VOID)
{
	H_AUTO_D3D(CDriverD3D_Release)
	return 0;
}

// ***************************************************************************

HRESULT CDriverD3D::LightEnable(DWORD Index, BOOL Enable)
{
	H_AUTO_D3D(CDriverD3D_LightEnable)
	enableLight ((uint8)Index, Enable!=FALSE);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetFVF(DWORD /* FVF */)
{
	H_AUTO_D3D(CDriverD3D_SetFVF)
	// Not implemented
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	H_AUTO_D3D(CDriverD3D_SetLight)
	_LightCache[Index].Light = *pLight;
	touchRenderVariable (&_LightCache[Index]);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	H_AUTO_D3D(CDriverD3D_SetMaterial)
	setMaterialState( *pMaterial );
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetNPatchMode(FLOAT /* nSegments */)
{
	H_AUTO_D3D(CDriverD3D_SetNPatchMode)
	// Not implemented
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShader)
	setPixelShader (pShader);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantB)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, (int*)(pConstantData+i*4));
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantF)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetPixelShaderConstantI)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setPixelShaderConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetRenderState)
	setRenderState (State, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetSamplerState)
	setSamplerState (Sampler, Type, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTexture (DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	H_AUTO_D3D(CDriverD3D_SetTexture )
	// Look for the current texture
	uint i;
	const uint count = (uint)_CurrentShaderTextures.size();
	for (i=0; i<count; i++)
	{
		const CTextureRef &ref = _CurrentShaderTextures[i];
		if (ref.D3DTexture == pTexture)
		{
			// Set the additionnal stage set by NeL texture (D3DSAMP_ADDRESSU, D3DSAMP_ADDRESSV, D3DSAMP_MAGFILTER, D3DSAMP_MINFILTER and D3DSAMP_MIPFILTER)
			setTexture (Stage, ref.NeLTexture);
			break;
		}
	}
	if (i == count)
		setTexture (Stage, pTexture);

	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	H_AUTO_D3D(CDriverD3D_SetTextureStageState)
	if (Type == D3DTSS_TEXCOORDINDEX)
		setTextureIndexUV (Stage, Value);
	else
		setTextureState (Stage, Type, Value);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	H_AUTO_D3D(CDriverD3D_SetTransform)
	setMatrix (State, *pMatrix);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShader)
	setVertexProgram (pShader, NULL);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantB)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, (int*)(pConstantData+i*4));
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantF)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

HRESULT CDriverD3D::SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	H_AUTO_D3D(CDriverD3D_SetVertexShaderConstantI)
	uint i;
	for (i=0; i<RegisterCount; i++)
		setVertexProgramConstant (i+StartRegister, pConstantData+i*4);
	return D3D_OK;
}

// ***************************************************************************

CShaderDrvInfosD3D::CShaderDrvInfosD3D(CDriverD3D *drv, ItShaderDrvInfoPtrList it) : IShaderDrvInfos(drv, it)
{
	H_AUTO_D3D(CShaderDrvInfosD3D_CShaderDrvInfosD3D)
	Validated = false;
}

// ***************************************************************************

CShaderDrvInfosD3D::~CShaderDrvInfosD3D()
{
	H_AUTO_D3D(CShaderDrvInfosD3D_CShaderDrvInfosD3DDtor)
	Effect->Release();
}

// ***************************************************************************

bool CDriverD3D::validateShader(CD3DShaderFX *shader)
{
	H_AUTO_D3D(CDriverD3D_validateShader)
	CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)shader->_DrvInfo);

	if (!shaderInfo->Validated)
	{
		// Choose the good method
		D3DXHANDLE hTechnique = NULL;
		D3DXHANDLE hCurrentTechnique = NULL;
		while (hTechnique == NULL)
		{
			if (shaderInfo->Effect->FindNextValidTechnique(hCurrentTechnique, &hCurrentTechnique) != D3D_OK)
				return false;

			// Info

#ifdef NL_FORCE_TEXTURE_STAGE_COUNT
			D3DXTECHNIQUE_DESC desc;
			nlverify (shaderInfo->Effect->GetTechniqueDesc(hCurrentTechnique, &desc) == D3D_OK);

			// Check this is compatible
			const uint len = strlen(desc.Name);
			if (len)
			{
				char shaderStageCount = desc.Name[len-1];
				if ((shaderStageCount>='0') && (shaderStageCount<='9'))
				{
					uint stageCount = NL_FORCE_TEXTURE_STAGE_COUNT;

					if ((uint)(shaderStageCount-'0')<=stageCount)
						// The good technique
						hTechnique = hCurrentTechnique;
				}
			}
#else // NL_FORCE_TEXTURE_STAGE_COUNT
			hTechnique = hCurrentTechnique;
#endif // NL_FORCE_TEXTURE_STAGE_COUNT

#ifdef NL_DEBUG_D3D
			{
				D3DXTECHNIQUE_DESC desc;
				nlverify (shaderInfo->Effect->GetTechniqueDesc(hCurrentTechnique, &desc) == D3D_OK);
				if (hTechnique)
					nlinfo ("Shader \"%s\" : use technique \"%s\" with %d passes.", shader->getName(), desc.Name, desc.Passes);
			}
#endif // NL_DEBUG_D3D
		}

		// Set the technique
		shaderInfo->Effect->SetTechnique(hTechnique);

		// Set the state manager
		shaderInfo->Effect->SetStateManager (this);

		shaderInfo->Validated = true;
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::activeShader(CD3DShaderFX *shd)
{
	H_AUTO_D3D(CDriverD3D_activeShader)
	if (_DisableHardwarePixelShader)
		return false;

	// Clear current textures
	_CurrentShaderTextures.clear();

	// Shader has been changed ?
	if (shd && shd->_ShaderChanged)
	{
		// Remove old shader
		shd->_DrvInfo.kill();

		// Already setuped ?
		CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)shd->_DrvInfo);
		if ( !shd->_DrvInfo )
		{
			// insert into driver list. (so it is deleted when driver is deleted).
			ItShaderDrvInfoPtrList	it= _ShaderDrvInfos.insert(_ShaderDrvInfos.end(), (NL3D::IShaderDrvInfos*)NULL);
			// create and set iterator, for future deletion.
			shaderInfo = new CShaderDrvInfosD3D(this, it);
			*it= shd->_DrvInfo = shaderInfo;
		}

		// Assemble the shader
		LPD3DXBUFFER pErrorMsgs;
		HRESULT hr = D3DXCreateEffect(_DeviceInterface, shd->getText(), (UINT)strlen(shd->getText())+1, NULL, NULL, 0, NULL, &(shaderInfo->Effect), &pErrorMsgs);
		if (hr == D3D_OK)
		{
			// Get the texture handle
			uint i;
			for (i=0; i<CShaderDrvInfosD3D::MaxShaderTexture; i++)
			{
				string name = "texture" + toString (i);
				shaderInfo->TextureHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "color" + toString (i);
				shaderInfo->ColorHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "factor" + toString (i);
				shaderInfo->FactorHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
				name = "scalarFloat" + toString (i);
				shaderInfo->ScalarFloatHandle[i] = shaderInfo->Effect->GetParameterByName(NULL, name.c_str());
			}
		}
		else
		{
			nlwarning ("Can't create shader '%s' (0x%x):", shd->getName(), hr);
			if (pErrorMsgs)
				nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
			shd->_ShaderChanged = false;
			_CurrentShader = NULL;
			return false;
		}

		// Done
		shd->_ShaderChanged = false;
	}

	// Set the shader
	_CurrentShader = shd;

	return true;
}


static void setFX(CD3DShaderFX &s, const char *name, const char *prog, CDriverD3D *drv)
{
	H_AUTO_D3D(setFX)

	s.setName(name);
	s.setText(prog);
	nlverify (drv->activeShader (&s));
}

#define setFx(a,b,c) { setFX(a, b, c, this); }

static const char *CloudFx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
float4 factor0;														\n\
																	\n\
pixelshader two_stages_ps = asm										\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	lrp r0.w, v0, t0, t1;											\n\
	mov r0.xyz, c0;													\n\
	+mul r0.w, c0, r0;												\n\
};																	\n\
																	\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShader = (two_stages_ps);								\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap0Fx =  
"																	\n\
texture texture0;													\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique one_stage_1												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// do a standard lighting with the first light				\n\
		Lighting = true;											\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TEXTURE;										\n\
		ColorArg2[0] = DIFFUSE;										\n\
		AlphaOp[0] = SELECTARG1; // for alpha test					\n\
		AlphaArg1[0] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap0blendFx =  
"																	\n\
texture texture0;													\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique one_stage_1												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// do a standard lighting with the first light				\n\
		Lighting = true;											\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = SRCALPHA;										\n\
		DestBlend = INVSRCALPHA;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TEXTURE;										\n\
		ColorArg2[0] = DIFFUSE;										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap0blend_x2Fx =  
"																	\n\
texture texture0;													\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique one_stage_1												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// do a standard lighting with the first light				\n\
		Lighting = true;											\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = SRCALPHA;										\n\
		DestBlend = INVSRCALPHA;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TEXTURE;										\n\
		ColorArg2[0] = DIFFUSE;										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap0_x2Fx =  
"																	\n\
texture texture0;													\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique one_stage_1												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// do a standard lighting with the first light				\n\
		Lighting = true;											\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TEXTURE;										\n\
		ColorArg2[0] = DIFFUSE;										\n\
		AlphaOp[0] = SELECTARG1; // for alpha test					\n\
		AlphaArg1[0] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap1Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		AlphaOp[0] =   SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] =   SELECTARG1; // for alpha test				\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap1blendFx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = SRCALPHA;										\n\
		DestBlend = INVSRCALPHA;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap1blend_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = SRCALPHA;										\n\
		DestBlend = INVSRCALPHA;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap1_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		AlphaOp[0] =   SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] =   SELECTARG1; // for alpha test				\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Lightmap2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_ps);							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1; // for alpha test					\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
}																	\n\
																	\n\
";

static const char *Lightmap2blendFx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_ps);							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
";

static const char *Lightmap2blend_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_ps);							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
";

static const char *Lightmap2_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_ps);							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1; // for alpha test if enabled		\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
}																	\n\
																	\n\
";

static const char *Lightmap3Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Lighting = false;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture3>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color3>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE; // for case when there's alpha test	\n\
		AlphaOp[2] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1; // alpha in case were alpha test is used\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
																	\n\
";

static const char *Lightmap3blendFx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		DestBlend = one;											\n\
		Lighting = false;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture3>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color3>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
}																	\n\
																	\n\
";

static const char *Lightmap3blend_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		DestBlend = one;											\n\
		Lighting = false;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture3>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color3>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
}																	\n\
																	\n\
";

static const char *Lightmap3_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Lighting = false;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture3>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color3>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE; // for case when there's alpha test	\n\
		AlphaOp[2] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1; // alpha in case were alpha test is used\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
																	\n\
";

static const char *Lightmap4Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
texture texture4;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
dword color4;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
float4 factor4;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 5 stages technique											\n\
pixelshader five_stages_ps = asm									\n\
{																	\n\
	ps_1_4;															\n\
	texld r0, t0;													\n\
	texld r1, t1;													\n\
	texld r2, t2;													\n\
	texld r3, t3;													\n\
	texld r4, t4;													\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r1.xyz, c1, r1, v0;											\n\
	mad r1.xyz, c2, r2, r1;											\n\
	mad r1.xyz, c3, r3, r1;											\n\
	mad r1.xyz, c4, r4, r1;											\n\
	mul r0.xyz, r1, r0;												\n\
};																	\n\
																	\n\
technique five_stages_5												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
		TexCoordIndex[4] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		Texture[4] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShaderConstant[4] = <factor4>;							\n\
		PixelShader = (five_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture4>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color4>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		ColorOp[3] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		AlphaOp[3] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	// 2 pass with the same pixel shader							\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		// second pass: shut down all lighting (lmc ambient term and dynamic lighting already added in first pass)\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialDiffuse= <g_black>;									\n\
																	\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[1] = <texture3>;									\n\
		Texture[2] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor3>;							\n\
		PixelShaderConstant[2] = <factor4>;							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
	pass p3															\n\
	{																\n\
		Texture[0] = <texture4>;									\n\
		TextureFactor = <color4>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
";

static const char *Lightmap4blendFx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
texture texture4;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
dword color4;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
float4 factor4;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
float4 g_dyn_factor = { 1.0f, 1.0f, 1.0f, 1.0f };					\n\
																	\n\
																	\n\
// **** 5 stages technique											\n\
pixelshader five_stages_ps = asm									\n\
{																	\n\
	ps_1_4;															\n\
	texld r0, t0;													\n\
	texld r1, t1;													\n\
	texld r2, t2;													\n\
	texld r3, t3;													\n\
	texld r4, t4;													\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r1.xyz, c1, r1, v0;											\n\
	mad r1.xyz, c2, r2, r1;											\n\
	mad r1.xyz, c3, r3, r1;											\n\
	mad r1.xyz, c4, r4, r1;											\n\
	mul r0.xyz, r1, r0;												\n\
	mov r0.w, r0;													\n\
};																	\n\
																	\n\
technique five_stages_5												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
		TexCoordIndex[4] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		Texture[4] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShaderConstant[4] = <factor4>;							\n\
		PixelShader = (five_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture4>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color4>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		ColorOp[3] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		AlphaOp[3] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul r0.xyz, r0, t0;												\n\
	mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	// 2 pass with the same pixel shader							\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		// second pass: shut down all lighting (lmc ambient term and dynamic lighting already added in first pass)\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialDiffuse= <g_black>;									\n\
																	\n\
		DestBlend = one;											\n\
		Texture[1] = <texture3>;									\n\
		Texture[2] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor3>;							\n\
		PixelShaderConstant[2] = <factor4>;							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE;										\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
	pass p3															\n\
	{																\n\
		Texture[0] = <texture4>;									\n\
		TextureFactor = <color4>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
																	\n\
";

static const char *Lightmap4blend_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
texture texture4;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
dword color4;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
float4 factor4;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 5 stages technique											\n\
pixelshader five_stages_ps = asm									\n\
{																	\n\
	ps_1_4;															\n\
	texld r0, t0;													\n\
	texld r1, t1;													\n\
	texld r2, t2;													\n\
	texld r3, t3;													\n\
	texld r4, t4;													\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r1.xyz, c1, r1, v0;											\n\
	mad r1.xyz, c2, r2, r1;											\n\
	mad r1.xyz, c3, r3, r1;											\n\
	mad r1.xyz, c4, r4, r1;											\n\
	mul_x2 r0.xyz, r1, r0;											\n\
};																	\n\
																	\n\
technique five_stages_5												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
		TexCoordIndex[4] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		Texture[4] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShaderConstant[4] = <factor4>;							\n\
		PixelShader = (five_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture4>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color4>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		ColorOp[3] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		AlphaOp[3] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	// 2 pass with the same pixel shader							\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		// second pass: shut down all lighting (lmc ambient term and dynamic lighting already added in first pass)\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialDiffuse= <g_black>;									\n\
																	\n\
		DestBlend = one;											\n\
		Texture[1] = <texture3>;									\n\
		Texture[2] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor3>;							\n\
		PixelShaderConstant[2] = <factor4>;							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = srcalpha;										\n\
		DestBlend = invsrcalpha;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
	pass p3															\n\
	{																\n\
		Texture[0] = <texture4>;									\n\
		TextureFactor = <color4>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
																	\n\
";

static const char *Lightmap4_x2Fx =  
"																	\n\
texture texture0;													\n\
texture texture1;													\n\
texture texture2;													\n\
texture texture3;													\n\
texture texture4;													\n\
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)\n\
// Other colors are the lightmap Factors for each lightmap			\n\
dword color0;														\n\
dword color1;														\n\
dword color2;														\n\
dword color3;														\n\
dword color4;														\n\
float4 factor0;														\n\
float4 factor1;														\n\
float4 factor2;														\n\
float4 factor3;														\n\
float4 factor4;														\n\
																	\n\
float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };						\n\
// modulate the dyn light by 0.5, because of MODULATE2X				\n\
float4 g_dyn_factor = { 0.5f, 0.5f, 0.5f, 1.0f };					\n\
																	\n\
																	\n\
// **** 5 stages technique											\n\
pixelshader five_stages_ps = asm									\n\
{																	\n\
	ps_1_4;															\n\
	texld r0, t0;													\n\
	texld r1, t1;													\n\
	texld r2, t2;													\n\
	texld r3, t3;													\n\
	texld r4, t4;													\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r1.xyz, c1, r1, v0;											\n\
	mad r1.xyz, c2, r2, r1;											\n\
	mad r1.xyz, c3, r3, r1;											\n\
	mad r1.xyz, c4, r4, r1;											\n\
	mul_x2 r0.xyz, r1, r0;											\n\
};																	\n\
																	\n\
technique five_stages_5												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
		TexCoordIndex[4] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		Texture[4] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShaderConstant[4] = <factor4>;							\n\
		PixelShader = (five_stages_ps);								\n\
	}																\n\
}																	\n\
																	\n\
// **** 4 stages technique											\n\
pixelshader four_stages_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	tex t3;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mad r0.xyz, c3, t3, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique four_stages_4												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
		TexCoordIndex[3] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShaderConstant[3] = <factor3>;							\n\
		PixelShader = (four_stages_ps);								\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture4>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color4>;									\n\
		ColorOp[0] = MODULATE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		ColorOp[2] = DISABLE;										\n\
		ColorOp[3] = DISABLE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
		AlphaOp[2] = DISABLE;										\n\
		AlphaOp[3] = DISABLE;										\n\
		PixelShader = NULL;											\n\
	}																\n\
}																	\n\
																	\n\
// **** 3 stages technique											\n\
pixelshader three_stages_0_ps = asm									\n\
{																	\n\
	ps_1_1;															\n\
	tex t0;															\n\
	tex t1;															\n\
	tex t2;															\n\
	// multiply lightmap with factor, and add with LMCAmbient+DynamicLight term\n\
	mad r0.xyz, c1, t1, v0;											\n\
	mad r0.xyz, c2, t2, r0;											\n\
	mul_x2 r0.xyz, r0, t0;											\n\
	+mov r0.w, t0;													\n\
};																	\n\
																	\n\
technique three_stages_3											\n\
{																	\n\
	// 2 pass with the same pixel shader							\n\
	pass p0															\n\
	{																\n\
		TexCoordIndex[2] = 1;										\n\
																	\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShaderConstant[2] = <factor2>;							\n\
		PixelShader = (three_stages_0_ps);							\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		// second pass: shut down all lighting (lmc ambient term and dynamic lighting already added in first pass)\n\
		MaterialEmissive= <g_black>;								\n\
		MaterialDiffuse= <g_black>;									\n\
																	\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[1] = <texture3>;									\n\
		Texture[2] = <texture4>;									\n\
		PixelShaderConstant[1] = <factor3>;							\n\
		PixelShaderConstant[2] = <factor4>;							\n\
	}																\n\
}																	\n\
																	\n\
// **** 2 stages, no pixel shader technique							\n\
technique two_stages_2												\n\
{																	\n\
	pass p0															\n\
	{																\n\
		// Use Emissive For LMCAmbient, and diffuse for per vertex dynamic lighting\n\
		Lighting = true;											\n\
		MaterialEmissive= <factor0>;								\n\
		MaterialAmbient= <g_black>;									\n\
		MaterialDiffuse= <g_dyn_factor>;							\n\
		MaterialSpecular= <g_black>;								\n\
		AlphaBlendEnable = false;									\n\
																	\n\
		// the DiffuseTexture texture 0 is in last stage			\n\
		TexCoordIndex[0] = 1;										\n\
		TexCoordIndex[1] = 0;										\n\
		Texture[0] = <texture1>;									\n\
		Texture[1] = <texture0>;									\n\
		TextureFactor = <color1>;									\n\
		ColorOp[0] = MULTIPLYADD;									\n\
		ColorArg0[0] = DIFFUSE;										\n\
		ColorArg1[0] = TFACTOR;										\n\
		ColorArg2[0] = TEXTURE;										\n\
		ColorOp[1] = MODULATE2X;									\n\
		ColorArg1[1] = CURRENT;										\n\
		ColorArg2[1] = TEXTURE;										\n\
		// Alpha stage 0 unused										\n\
		AlphaOp[0] = SELECTARG1;									\n\
		AlphaArg1[0] = TFACTOR;										\n\
		AlphaOp[1] = SELECTARG1;									\n\
		AlphaArg1[1] = TEXTURE;										\n\
	}																\n\
	pass p1															\n\
	{																\n\
		FogColor = 0x00000000; // don't accumulate fog several times\n\
		Lighting = false;											\n\
		AlphaBlendEnable = true;									\n\
		SrcBlend = one;												\n\
		DestBlend = one;											\n\
		Texture[0] = <texture2>;									\n\
		TextureFactor = <color2>;									\n\
		ColorOp[0] = MODULATE;										\n\
	}																\n\
	pass p2															\n\
	{																\n\
		Texture[0] = <texture3>;									\n\
		TextureFactor = <color3>;									\n\
	}																\n\
	pass p3															\n\
	{																\n\
		Texture[0] = <texture4>;									\n\
		TextureFactor = <color4>;									\n\
	}																\n\
}																	\n\
																	\n\
																	\n\
";

static const char *Water_diffuseFx =  
"																	\n\
texture texture0; // bumpmap0										\n\
texture texture1; // bumpmap1										\n\
texture texture2; // envmap											\n\
texture texture3; // diffuse										\n\
																	\n\
float4 factor0; // bumpmap0 scale									\n\
float4 factor1; // bumpmap1 scale									\n\
float  scalarFloat0; // bump scale for 1_1 version					\n\
																	\n\
pixelshader water_diffuse_2_0 = asm									\n\
{																	\n\
	ps_2_0;															\n\
	dcl t0.xy;														\n\
	dcl t1.xy;														\n\
	dcl t2.xy;														\n\
	dcl t3.xy;														\n\
	dcl_2d s0;														\n\
	dcl_2d s1;														\n\
	dcl_2d s2;														\n\
	dcl_2d s3;														\n\
	//read bump map 0												\n\
	texld   r0, t0, s0;												\n\
	//bias result (include scaling)									\n\
	mad    r0.xy, r0, c0.z, c0;										\n\
	add    r0.xy, r0, t1;											\n\
	//read bump map 1												\n\
	texld  r0, r0, s1;												\n\
	//bias result (include scaling)									\n\
	mad    r0.xy, r0, c1.z, c1;										\n\
	//add envmap coord												\n\
	add	   r0.xy, r0, t2;											\n\
	// read envmap													\n\
	texld  r0, r0, s2;												\n\
	// read diffuse													\n\
	texld  r1, t3, s3;												\n\
	mul r0, r0, r1;													\n\
	mov oC0, r0														\n\
};																	\n\
																	\n\
technique technique_water_diffuse_2_0								\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_diffuse_2_0);							\n\
	}																\n\
};																	\n\
																	\n\
pixelshader water_diffuse_1_4 = asm									\n\
{																	\n\
	ps_1_4;															\n\
	texld   r0, t0;													\n\
	texld   r1, t1;													\n\
	texcrd  r2.xyz, t2;												\n\
	mad r2.xy, r0_bx2, c0, r2;										\n\
	mad r2.xy, r1_bx2, c1, r2;										\n\
	phase															\n\
	texld r2, r2;													\n\
	texld r3, t3;													\n\
	mul r0, r2, r3;													\n\
};																	\n\
																	\n\
technique technique_water_diffuse_1_4								\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_diffuse_1_4);							\n\
	}																\n\
};																	\n\
																	\n\
pixelshader water_diffuse_1_1 = asm									\n\
{																	\n\
	// note in OpenGL on nVidia cards, it is permitted to chain 2 texbem so the effect is less nice there (no bumpmap animation)\n\
	ps_1_1;															\n\
	tex t1;															\n\
	texbem t2, t1;													\n\
	tex t3;															\n\
	mul r0, t3, t2;													\n\
};																	\n\
																	\n\
technique technique_water_diffuse_1_1								\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		Texture[3] = <texture3>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_diffuse_1_1);							\n\
		BumpEnvMat00[2] = <scalarFloat0>;							\n\
		BumpEnvMat01[0] = 0;										\n\
		BumpEnvMat10[0] = 0;										\n\
		BumpEnvMat11[2] = <scalarFloat0>;							\n\
	}																\n\
};																	\n\
																	\n\
";

static const char *Water_no_diffuseFx =  
"																	\n\
texture texture0; // bumpmap0										\n\
texture texture1; // bumpmap1										\n\
texture texture2; // envmap											\n\
																	\n\
float4 factor0; // bumpmap0 scale									\n\
float4 factor1; // bumpmap1 scale									\n\
float  scalarFloat0; // bump scale for 1_1 version					\n\
																	\n\
pixelshader water_no_diffuse_2_0 = asm								\n\
{																	\n\
	ps_2_0;															\n\
	dcl t0.xy;														\n\
	dcl t1.xy;														\n\
	dcl t2.xy;														\n\
	dcl_2d s0;														\n\
	dcl_2d s1;														\n\
	dcl_2d s2;														\n\
	//read bump map 0												\n\
	texld   r0, t0, s0;												\n\
	//bias result (include scaling)									\n\
	mad    r0.xy, r0, c0.z, c0;										\n\
	add    r0.xy, r0, t1;											\n\
	//read bump map 1												\n\
	texld  r0, r0, s1;												\n\
	//bias result (include scaling)									\n\
	mad    r0.xy, r0, c1.z, c1;										\n\
	//add envmap coord												\n\
	add	   r0.xy, r0, t2;											\n\
	//read envmap													\n\
	texld  r0, r0, s2;												\n\
	mov oC0, r0;													\n\
};																	\n\
																	\n\
technique technique_water_no_diffuse_2_0							\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_no_diffuse_2_0);						\n\
	}																\n\
};																	\n\
																	\n\
pixelshader water_no_diffuse_1_4 = asm								\n\
{																	\n\
	ps_1_4;															\n\
	texld   r0, t0;													\n\
	texld   r1, t1;													\n\
	texcrd  r2.xyz, t2;												\n\
	mad r2.xy, r0_bx2, c0, r2;										\n\
	mad r2.xy, r1_bx2, c1, r2;										\n\
	phase															\n\
	texld r2, r2;													\n\
	mov r0, r2;														\n\
};																	\n\
																	\n\
technique technique_water_no_diffuse_1_4							\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[0] = <texture0>;									\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_no_diffuse_1_4);						\n\
	}																\n\
};																	\n\
																	\n\
pixelshader water_no_diffuse_1_1 = asm								\n\
{																	\n\
	// note in OpenGL on nVidia cards, it is permitted to chain 2 texbem so the effect is less nice there (no bumpmap animation)\n\
	ps_1_1;															\n\
	tex t1;															\n\
	texbem t2, t1;													\n\
	mov r0, t2;														\n\
};																	\n\
																	\n\
technique technique_water_no_diffuse_1_1							\n\
{																	\n\
	pass p0															\n\
	{																\n\
		Texture[1] = <texture1>;									\n\
		Texture[2] = <texture2>;									\n\
		PixelShaderConstant[0] = <factor0>;							\n\
		PixelShaderConstant[1] = <factor1>;							\n\
		PixelShader = (water_no_diffuse_1_1);						\n\
		BumpEnvMat00[2] = <scalarFloat0>;							\n\
		BumpEnvMat01[0] = 0;										\n\
		BumpEnvMat10[0] = 0;										\n\
		BumpEnvMat11[2] = <scalarFloat0>;							\n\
	}																\n\
};																	\n\
																	\n\
";



void CDriverD3D::initInternalShaders()
{
	H_AUTO_D3D(CDriverD3D_initInternalShaders)
	setFx(_ShaderLightmap0,"lightmap0Fx", Lightmap0Fx);
	setFx(_ShaderLightmap1,"lightmap1Fx", Lightmap1Fx);
	setFx(_ShaderLightmap2,"lightmap2Fx", Lightmap2Fx);
	setFx(_ShaderLightmap3,"lightmap3Fx", Lightmap3Fx);
	setFx(_ShaderLightmap4,"lightmap4Fx", Lightmap4Fx);
	setFx(_ShaderLightmap0Blend,"lightmap0blendFx", Lightmap0blendFx);
	setFx(_ShaderLightmap1Blend,"lightmap1blendFx", Lightmap1blendFx);
	setFx(_ShaderLightmap2Blend,"lightmap2blendFx", Lightmap2blendFx);
	setFx(_ShaderLightmap3Blend,"lightmap3blendFx", Lightmap3blendFx);
	setFx(_ShaderLightmap4Blend,"lightmap4blendFx", Lightmap4blendFx);
	setFx(_ShaderLightmap0X2,"lightmap0_x2Fx", Lightmap0_x2Fx);
	setFx(_ShaderLightmap1X2,"lightmap1_x2Fx", Lightmap1_x2Fx);
	setFx(_ShaderLightmap2X2,"lightmap2_x2Fx", Lightmap2_x2Fx);
	setFx(_ShaderLightmap3X2,"lightmap3_x2Fx", Lightmap3_x2Fx);
	setFx(_ShaderLightmap4X2,"lightmap4_x2Fx", Lightmap4_x2Fx);
	setFx(_ShaderLightmap0BlendX2,"lightmap0blend_x2Fx", Lightmap0blend_x2Fx);
	setFx(_ShaderLightmap1BlendX2,"lightmap1blend_x2Fx", Lightmap1blend_x2Fx);
	setFx(_ShaderLightmap2BlendX2,"lightmap2blend_x2Fx", Lightmap2blend_x2Fx);
	setFx(_ShaderLightmap3BlendX2,"lightmap3blend_x2Fx", Lightmap3blend_x2Fx);
	setFx(_ShaderLightmap4BlendX2,"lightmap4blend_x2Fx", Lightmap4blend_x2Fx);
	setFx(_ShaderCloud, "cloudFx", CloudFx);
	setFx(_ShaderWaterNoDiffuse,"water_no_diffuseFx", Water_no_diffuseFx);
	setFx(_ShaderWaterDiffuse, "water_diffuseFx", Water_diffuseFx);
}


// ***************************************************************************

void CDriverD3D::releaseInternalShaders()
{
	H_AUTO_D3D(CDriverD3D_releaseInternalShaders)
	_ShaderLightmap0._DrvInfo.kill();
	_ShaderLightmap1._DrvInfo.kill();
	_ShaderLightmap2._DrvInfo.kill();
	_ShaderLightmap3._DrvInfo.kill();
	_ShaderLightmap4._DrvInfo.kill();
	_ShaderLightmap0Blend._DrvInfo.kill();
	_ShaderLightmap1Blend._DrvInfo.kill();
	_ShaderLightmap2Blend._DrvInfo.kill();
	_ShaderLightmap3Blend._DrvInfo.kill();
	_ShaderLightmap4Blend._DrvInfo.kill();
	_ShaderLightmap0X2._DrvInfo.kill();
	_ShaderLightmap1X2._DrvInfo.kill();
	_ShaderLightmap2X2._DrvInfo.kill();
	_ShaderLightmap3X2._DrvInfo.kill();
	_ShaderLightmap4X2._DrvInfo.kill();
	_ShaderLightmap0BlendX2._DrvInfo.kill();
	_ShaderLightmap1BlendX2._DrvInfo.kill();
	_ShaderLightmap2BlendX2._DrvInfo.kill();
	_ShaderLightmap3BlendX2._DrvInfo.kill();
	_ShaderLightmap4BlendX2._DrvInfo.kill();
	_ShaderCloud._DrvInfo.kill();
	_ShaderWaterNoDiffuse._DrvInfo.kill();
	_ShaderWaterDiffuse._DrvInfo.kill();
}

// ***************************************************************************
bool CDriverD3D::setShaderTexture (uint textureHandle, ITexture *texture, CFXCache *cache)
{
	H_AUTO_D3D(CDriverD3D_setShaderTexture )
	// Setup the texture
	if (!setupTexture(*texture))
		return false;

	// Set the main texture
	nlassert (_CurrentShader);
	CShaderDrvInfosD3D *shaderInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
	nlassert (shaderInfo);
	ID3DXEffect	*effect = shaderInfo->Effect;
	CTextureDrvInfosD3D *d3dtext = getTextureD3D (*texture);
	if (texture)
	{
		if (cache)
		{
			cache->Params.setTexture(textureHandle, d3dtext->Texture);
		}
		else
		{
			effect->SetTexture (shaderInfo->TextureHandle[textureHandle], d3dtext->Texture);
		}
	}


	// Add a ref on this texture
	_CurrentShaderTextures.push_back (CTextureRef());
	CTextureRef &ref = _CurrentShaderTextures.back();
	ref.NeLTexture = texture;
	ref.D3DTexture = d3dtext->Texture;
	return true;
}

// ***************************************************************************

void CDriverD3D::disableHardwareTextureShader()
{
	// cannot disable pixel shader under DX, because it crashes with lightmap
	// => no-op
	/*
	_DisableHardwarePixelShader = true;
	_PixelShader = false;
	*/
}



// ***************************************************************************
void CDriverD3D::notifyAllShaderDrvOfLostDevice()
{
	for(TShaderDrvInfoPtrList::iterator it = _ShaderDrvInfos.begin(); it != _ShaderDrvInfos.end(); ++it)
	{
		nlassert(*it);
		CShaderDrvInfosD3D *drvInfo = NLMISC::safe_cast<CShaderDrvInfosD3D *>(*it);
		if (drvInfo->Effect)
		{
			nlverify(drvInfo->Effect->OnLostDevice() == D3D_OK);
		}
	}
}

// ***************************************************************************
void CDriverD3D::notifyAllShaderDrvOfResetDevice()
{
	for(TShaderDrvInfoPtrList::iterator it = _ShaderDrvInfos.begin(); it != _ShaderDrvInfos.end(); ++it)
	{
		nlassert(*it);
		CShaderDrvInfosD3D *drvInfo = NLMISC::safe_cast<CShaderDrvInfosD3D *>(*it);
		if (drvInfo->Effect)
		{
			//nlverify(
			drvInfo->Effect->OnResetDevice();
			//== D3D_OK);
		}
	}
}


// ***************************************************************************
// state records (for .fx caching)
// ***************************************************************************

class CStateRecordLightEnable : public CStateRecord
{
public:
	DWORD Index;
	BOOL  Enable;
public:
	CStateRecordLightEnable(DWORD index, BOOL enable) : Index(index), Enable(enable) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.enableLight ((uint8)Index, Enable!=FALSE);
	}
};
//
class CStateRecordLight : public CStateRecord
{
public:
	DWORD	  Index;
	D3DLIGHT9 Light;
public:
	CStateRecordLight(DWORD index, const D3DLIGHT9 *pLight) : Index(index), Light(*pLight) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv._LightCache[Index].Light = Light;
		drv.touchRenderVariable (&drv._LightCache[Index]);
	}
};
//
class CStateRecordMaterial : public CStateRecord
{
public:
	D3DMATERIAL9 Material;
public:
	CStateRecordMaterial(const D3DMATERIAL9 *pMaterial) : Material(*pMaterial) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setMaterialState(Material);
	}
};
//
class CStateRecordPixelShader : public CStateRecord
{
public:
	LPDIRECT3DPIXELSHADER9 PixelShader;
public:
	CStateRecordPixelShader(LPDIRECT3DPIXELSHADER9 pixelShader) : PixelShader(pixelShader) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setPixelShader(PixelShader);
	}
};
//
class CStateRecordPixelShaderConstantB : public CStateRecord
{
public:
	std::vector<BOOL> Values;
	uint			  StartRegister;
public:
	CStateRecordPixelShaderConstantB(DWORD startRegister, const BOOL *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const BOOL *curr = &Values[0];
		const BOOL *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
};
//
class CStateRecordPixelShaderConstantF : public CStateRecord
{
public:
	std::vector<FLOAT> Values;
	uint			  StartRegister;
public:
	CStateRecordPixelShaderConstantF(DWORD startRegister, const FLOAT *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const FLOAT *curr = &Values[0];
		const FLOAT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
};
//
class CStateRecordPixelShaderConstantI : public CStateRecord
{
public:
	std::vector<INT>  Values;
	uint			  StartRegister;
public:
	CStateRecordPixelShaderConstantI(DWORD startRegister, const INT *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const INT *curr = &Values[0];
		const INT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setPixelShaderConstant (i, curr);
			curr += 4;
			++ i;
		}
	}
};
//
class CStateRecordRenderState : public CStateRecord
{
public:
	D3DRENDERSTATETYPE State;
	DWORD Value;
public:
	CStateRecordRenderState(D3DRENDERSTATETYPE state, DWORD value) : State(state), Value(value) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setRenderState(State, Value);
	}
};
//
class CStateRecordSamplerState : public CStateRecord
{
public:
	DWORD Sampler;
	D3DSAMPLERSTATETYPE Type;
	DWORD Value;
public:
	CStateRecordSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value) : Sampler(sampler), Type(type), Value(value) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setSamplerState(Sampler, Type, Value);
	}
};
//
class CStateRecordTexture : public CStateRecord
{
public:
	DWORD Stage;
	CRefPtr<ITexture>      TexRef;
	LPDIRECT3DBASETEXTURE9 Texture;
	BOOL IsNelTexture;
public:
	CStateRecordTexture(DWORD stage, LPDIRECT3DBASETEXTURE9 texture, ITexture *nelTexture) : Stage(stage), Texture(texture)
	{
		nlassert(Texture);
		H_AUTO_D3D(CDriverD3D_SetTexture )
		// if not a NeL texture, should add a reference on texture, else use refptr instead
		IsNelTexture = nelTexture != NULL;
		TexRef = nelTexture;
		if (!IsNelTexture)
		{
			HRESULT r = Texture->AddRef();
			nlassert(r == D3D_OK);
		}
	}
	~CStateRecordTexture()
	{
		nlassert(Texture); // no default ctor, so texture should have been set
		if (!IsNelTexture)
		{
			nlassert(TexRef == NULL);
			HRESULT r = Texture->Release();
			nlassert(r == D3D_OK);
		}
	}
	virtual void apply(class CDriverD3D &drv)
	{
		nlassert(Texture);
		if (TexRef)
		{
			drv.setTexture(Stage, TexRef);
		}
		else
		{
			drv.setTexture(Stage, Texture);
		}
	}
};
//
class CStateRecordTextureStageState : public CStateRecord
{
public:
	DWORD Stage;
	D3DTEXTURESTAGESTATETYPE Type;
	DWORD Value;
public:
	CStateRecordTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) : Stage(stage), Type(type), Value(value) {}
	virtual void apply(class CDriverD3D &drv)
	{
		if (Type == D3DTSS_TEXCOORDINDEX)
			drv.setTextureIndexUV (Stage, Value);
		else
			drv.setTextureState (Stage, Type, Value);
	}
};
//
class CStateRecordTransform : public CStateRecord
{
public:
	D3DTRANSFORMSTATETYPE State;
	D3DMATRIX Matrix;
public:
	CStateRecordTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *pMatrix) : State(state), Matrix(*pMatrix) {}
	virtual void apply(class CDriverD3D &drv)
	{
		drv.setMatrix(State, Matrix);
	}
};
//
class CStateRecordVertexShader : public CStateRecord
{
public:
	LPDIRECT3DVERTEXSHADER9 Shader;
public:
	CStateRecordVertexShader(LPDIRECT3DVERTEXSHADER9 shader) : Shader(shader) {}
	virtual void apply(class CDriverD3D &/* drv */)
	{
		nlassert(0); // not supported
		//drv.setVertexProgram(Shader);
	}
};
//
class CStateRecordVertexShaderConstantB : public CStateRecord
{
public:
	std::vector<BOOL> Values;
	uint			  StartRegister;
public:
	CStateRecordVertexShaderConstantB(DWORD startRegister, const BOOL *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const BOOL *curr = &Values[0];
		const BOOL *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setVertexProgramConstant(i, curr);
			curr += 4;
			++ i;
		}
	}
};
class CStateRecordVertexShaderConstantF : public CStateRecord
{
public:
	std::vector<FLOAT> Values;
	uint			   StartRegister;
public:
	CStateRecordVertexShaderConstantF(DWORD startRegister, const FLOAT *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const FLOAT *curr = &Values[0];
		const FLOAT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setVertexProgramConstant(i, curr);
			curr += 4;
			++ i;
		}
	}
};
class CStateRecordVertexShaderConstantI : public CStateRecord
{
public:
	std::vector<INT> Values;
	uint			   StartRegister;
public:
	CStateRecordVertexShaderConstantI(DWORD startRegister, const INT *values, DWORD countVec4) : StartRegister(startRegister)
	{
		Values.resize(countVec4 * 4);
		std::copy(values, values + countVec4 * 4, Values.begin());
	}
	virtual void apply(class CDriverD3D &drv)
	{
		const INT *curr = &Values[0];
		const INT *last = &Values[0] + Values.size();
		uint i = StartRegister;
		while (curr != last)
		{
			drv.setVertexProgramConstant(i, curr);
			curr += 4;
			++ i;
		}
	}
};

// state recorder
HRESULT STDMETHODCALLTYPE CFXPassRecorder::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	nlassert(Driver);
	return Driver->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CFXPassRecorder::AddRef(VOID)
{
	nlassert(Driver);
	return Driver->AddRef();
}

ULONG STDMETHODCALLTYPE CFXPassRecorder::Release(VOID)
{
	nlassert(Driver);
	return Driver->Release();
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::LightEnable(DWORD Index, BOOL Enable)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordLightEnable(Index, Enable));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetFVF(DWORD /* FVF */)
{
	nlassert(0); // not managed
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordLight(Index, pLight));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordMaterial(pMaterial));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetNPatchMode(FLOAT /* nSegments */)
{
	nlassert(0); // not managed
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordPixelShader(pShader));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordPixelShaderConstantB(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordPixelShaderConstantF(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordPixelShaderConstantI(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordRenderState(State, Value));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordSamplerState(Sampler, Type, Value));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	nlassert(Driver);
	nlassert(Target);
	// Look for the current texture
	uint i;
	const uint count = (uint)Driver->getCurrentShaderTextures().size();
	for (i=0; i<count; i++)
	{
		const CDriverD3D::CTextureRef &ref = Driver->getCurrentShaderTextures()[i];
		if (ref.D3DTexture == pTexture)
		{
			// Set the additionnal stage set by NeL texture (D3DSAMP_ADDRESSU, D3DSAMP_ADDRESSV, D3DSAMP_MAGFILTER, D3DSAMP_MINFILTER and D3DSAMP_MIPFILTER)
			Target->States.push_back(new CStateRecordTexture(Stage, pTexture, ref.NeLTexture));
			break;
		}
	}
	if (i == count)
	{
		// The texture isn't a NeL texture (was created inside driver)
		Target->States.push_back(new CStateRecordTexture(Stage, pTexture, NULL));
	}
	return D3D_OK;
}


HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordTextureStageState(Stage, Type, Value));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordTransform(State, pMatrix));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordVertexShader(pShader));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordVertexShaderConstantB(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordVertexShaderConstantF(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE CFXPassRecorder::SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount)
{
	nlassert(Driver);
	nlassert(Target);
	Target->States.push_back(new CStateRecordVertexShaderConstantI(StartRegister, pConstantData, RegisterCount));
	return D3D_OK;
}

//===================================================================================
CFXPassRecord::~CFXPassRecord()
{
	for(std::vector<CStateRecord *>::iterator it = States.begin(); it != States.end(); ++it)
	{
		delete *it;
	}
}

void CFXPassRecord::apply(CDriverD3D &drv)
{
	for(std::vector<CStateRecord *>::iterator it = States.begin(); it != States.end(); ++it)
	{
		(*it)->apply(drv);
	}
}

const uint NL_D3D_NUM_RENDER_FOR_FX_CACHING = 4;

//===================================================================================
void CFXCache::setConstants(CShaderDrvInfosD3D *si)
{
	nlassert(si);
	nlassert(si->Effect);
	for(uint k = 0; k < CFXInputParams::MaxNumParams; ++k)
	{
		if (Params.Textures[k].Set) si->Effect->SetTexture(si->TextureHandle[k], Params.Textures[k].Value);
		if (Params.Vectors[k].Set) si->Effect->SetFloatArray(si->FactorHandle[k], (CONST FLOAT *) &(Params.Vectors[k].Value), 4);
		if (Params.Colors[k].Set) si->Effect->SetInt(si->ColorHandle[k], Params.Colors[k].Value);
		if (Params.Floats[k].Set) si->Effect->SetFloat(si->ScalarFloatHandle[k], Params.Floats[k].Value);
	}
}

void CFXCache::begin(CShaderDrvInfosD3D *si, CDriverD3D *driver)
{
	nlassert(driver);
	// amortize cost of caching for animated material -> ensure that the parameters don't change for
	// a few number of display before doing caching
	if (Params.Touched)
	{
		Steadyness = 0;
		Passes.clear();
		NumPasses = 0;
		Params.Touched = false;
		return;
	}
	if (!Params.Touched)
	{
		++ Steadyness;
	}
	if (Steadyness < NL_D3D_NUM_RENDER_FOR_FX_CACHING)
	{
		NumPasses = 0;
		Passes.clear();
		return;
	}
	if (Passes.empty()) // must record shader ?
	{
		Passes.clear();
		UINT numPasses;
		CFXPassRecorder pr;
		pr.Driver = driver;
		si->Effect->SetStateManager(&pr);
		// Set constants
		setConstants(si);
		//
		HRESULT r = si->Effect->Begin(&numPasses, D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESHADERSTATE);
		nlassert(r == D3D_OK);
		Passes.resize(numPasses);
		for(uint k = 0; k < numPasses; ++k)
		{
			pr.Target = &Passes[k];
#if (DIRECT3D_VERSION >= 0x0900) && (D3D_SDK_VERSION >= 32)
			si->Effect->BeginPass(k);
			si->Effect->EndPass();
#else
			si->Effect->Pass(k);
#endif
		}
		r = si->Effect->End();
		nlassert(r == D3D_OK);
		NumPasses = numPasses;
		si->Effect->SetStateManager(driver);
	}
}

void CFXCache::applyPass(class CDriverD3D &drv, CShaderDrvInfosD3D *si, uint passIndex)
{
	if (Passes.empty() && NumPasses == 0)
	{
		// the shader has not been cached yet (maybe animated)
		// so uses the standard path
		UINT numPasses;
		setConstants(si);
		HRESULT r = si->Effect->Begin(&numPasses, D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESHADERSTATE);
		nlassert(r == D3D_OK);
		NumPasses = numPasses;
	}
	nlassert(passIndex < NumPasses);
	if (Passes.empty())
	{
#if (DIRECT3D_VERSION >= 0x0900) && (D3D_SDK_VERSION >= 32)
		HRESULT r = si->Effect->BeginPass(passIndex);
		si->Effect->EndPass ();
#else
		HRESULT r = si->Effect->Pass(passIndex);
#endif
		nlassert(r == D3D_OK);
	}
	else
	{
		Passes[passIndex].apply(drv);
	}
}

void CFXCache::end(CShaderDrvInfosD3D *si)
{
	if (Passes.empty())
	{
		HRESULT r = si->Effect->End();
		nlassert(r == D3D_OK);
	}
}


void CFXCache::reset()
{
	Params.reset();
	Passes.clear();
	Steadyness = 0;
}



} // NL3D
