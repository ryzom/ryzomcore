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

#include "std3d.h"
#include "nel/3d/water_env_map.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/driver.h"
#include "nel/3d/u_water_env_map.h"
#include "nel/misc/common.h"
#include "nel/3d/viewport.h"

namespace NL3D

{


CVertexBuffer CWaterEnvMap::_FlattenVB; // vb to map cube map top hemisphere to a 2D map
CIndexBuffer CWaterEnvMap::_FlattenIB("CWaterEnvMap::_FlattenIB");
bool CWaterEnvMap::_FlattenVBInitialized;
CMaterial CWaterEnvMap::_MaterialPassThru;
CMaterial CWaterEnvMap::_MaterialPassThruZTest;
CVertexBuffer CWaterEnvMap::_TestVB;
CIndexBuffer CWaterEnvMap::_TestIB("CWaterEnvMap::_TestIB");

// flatten vb params
static const uint FVB_NUM_SIDES = 32;
static const uint FVB_NUM_SECTIONS = 10;
static const uint FVB_NUM_VERTS = FVB_NUM_SIDES * (FVB_NUM_SECTIONS + 1);
static const uint FVB_NUM_TRIS = 2 * FVB_NUM_SIDES * FVB_NUM_SECTIONS;

// tmp : test vertex buffer
static const uint TEST_VB_NUM_SEGMENT = 16;
static const uint TEST_VB_NUM_SLICE = 16;
static const uint TEST_VB_NUM_TRIS = 2 * TEST_VB_NUM_SEGMENT * TEST_VB_NUM_SLICE;

// Get index of a vertex in the flatten vb from its side an section
static uint32 inline getFVBVertex(uint section, uint side)
{
	nlassert(section <= FVB_NUM_SECTIONS);
	return (uint32) (section + (side % FVB_NUM_SIDES) * (FVB_NUM_SECTIONS + 1));
}


const uint NUM_FACES_TO_RENDER = 5;


// *******************************************************************************
CWaterEnvMap::CWaterEnvMap()
{
	_UpdateTime = 0;
	_LastRenderTick = 0;
	invalidate();
	_NumRenderedFaces = 0;
	_EnvCubicSize = 0;
	_Env2DSize = 0;
	_LastRenderTime = -1;
	_StartRenderTime = -1;
	_Alpha = 255;
}

// *******************************************************************************
void CWaterEnvMap::init(uint cubeMapSize, uint projection2DSize, TGlobalAnimationTime updateTime, IDriver &driver)
{
	// Allocate cube map
	// a cubic texture with no sharing allowed
	class CTextureCubeUnshared : public CTextureCube
	{
	public:
		virtual bool supportSharing() const {return false;}
		virtual uint32 getWidth(uint32 numMipMap = 0) const
		{
			nlassert(numMipMap == 0);
			return Size;
		}
		virtual uint32 getHeight(uint32 numMipMap = 0) const
		{
			nlassert(numMipMap == 0);
			return Size;
		}
		uint32 Size;
	};
	// a 2D testure
	class CTexture2DUnshared : public CTextureBlank
	{
	public:
		virtual bool supportSharing() const {return false;}
		virtual uint32 getWidth(uint32 numMipMap = 0) const
		{
			nlassert(numMipMap == 0);
			return Size;
		}
		virtual uint32 getHeight(uint32 numMipMap = 0) const
		{
			nlassert(numMipMap == 0);
			return Size;
		}
		uint32 Size;
	};
	nlassert(cubeMapSize > 0);
	nlassert(NLMISC::isPowerOf2(cubeMapSize));
	nlassert(projection2DSize > 0);
	nlassert(NLMISC::isPowerOf2(projection2DSize));
	CTextureCubeUnshared *envCubic = new CTextureCubeUnshared;
	_EnvCubic = envCubic;
	_EnvCubic->setRenderTarget(true); // we will render to the texture
	_EnvCubic->setWrapS(ITexture::Clamp);
	_EnvCubic->setWrapT(ITexture::Clamp);
	_EnvCubic->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	CTexture2DUnshared *tb = new CTexture2DUnshared;
	tb->resize(cubeMapSize, cubeMapSize); // Unfortunately,  must allocate memory in order for the driver to figure out the size
	                                      // that it needs to allocate for the texture, though its datas are never used (it is a render target)
	tb->Size = cubeMapSize;
	tb->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	for(uint k = 0; k < 6; ++k)
	{
		_EnvCubic->setTexture((CTextureCube::TFace) k, tb);
		_EnvCubic->getTexture((CTextureCube::TFace) k)->setRenderTarget(true);
	}
	envCubic->Size = cubeMapSize;
	// setup the texture to force the driver to allocate vram for it
	driver.setupTexture(*_EnvCubic);
	tb->reset();
	// Allocate projection 2D map
	CTexture2DUnshared *env2D = new CTexture2DUnshared;
	_Env2D = env2D;
	_Env2D->resize(projection2DSize, projection2DSize);
	env2D->Size = projection2DSize;
	_Env2D->setWrapS(ITexture::Clamp);
	_Env2D->setWrapT(ITexture::Clamp);
	_Env2D->setRenderTarget(true); // we will render to the texture
	_Env2D->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	driver.setupTexture(*_Env2D); // allocate vram
	_Env2D->reset();
	_UpdateTime = updateTime;
	_LastRenderTime = -1;
	invalidate();
	_NumRenderedFaces = 0;
	_EnvCubicSize = cubeMapSize;
	_Env2DSize = projection2DSize;
}

// *******************************************************************************
void CWaterEnvMap::update(TGlobalAnimationTime time, IDriver &driver)
{
	if (_LastRenderTime == time) return;
	_LastRenderTime = time;
	// First five updates are used to render the cubemap faces (bottom face is not rendered)
	// Sixth update project the cubemap into a 2D texture
	uint numTexToRender;
	if (_UpdateTime > 0)
	{
		uint64 currRenderTick = (uint64) (time / (_UpdateTime / (NUM_FACES_TO_RENDER + 1)));
		numTexToRender = (uint) (currRenderTick - _LastRenderTick);
		_LastRenderTick = currRenderTick;
	}
	else
	{
		numTexToRender = NUM_FACES_TO_RENDER + 1;
	}
	if (!numTexToRender) return;
	if (_NumRenderedFaces == 0)
	{
		_StartRenderTime = time;
	}
	uint lastCubeFacesToRender = std::min((uint) NUM_FACES_TO_RENDER, _NumRenderedFaces + numTexToRender); // we don't render negative Z (only top hemisphere is used)
	for(uint k = _NumRenderedFaces; k < lastCubeFacesToRender; ++k)
	{
		driver.setRenderTarget(_EnvCubic, 0, 0, _EnvCubicSize, _EnvCubicSize, 0, (uint32) k);
		render((CTextureCube::TFace) k, _StartRenderTime);
	}
	_NumRenderedFaces = lastCubeFacesToRender;
	if (_NumRenderedFaces == NUM_FACES_TO_RENDER && (_NumRenderedFaces + numTexToRender) > NUM_FACES_TO_RENDER)
	{
		// render to 2D map
		driver.setRenderTarget(_Env2D, 0, 0, _Env2DSize, _Env2DSize);
		doInit();
		//
		driver.activeVertexProgram(NULL);
		driver.activeVertexBuffer(_FlattenVB);
		driver.activeIndexBuffer(_FlattenIB);
		driver.setFrustum(-1.f, 1.f, -1.f, 1.f, 0.f, 1.f, false);
		driver.setupViewMatrix(CMatrix::Identity);
		CMatrix mat;
		//mat.scale(0.8f);
		driver.setupModelMatrix(mat);
		_MaterialPassThru.setTexture(0, _EnvCubic);
		_MaterialPassThru.texConstantColor(0, CRGBA(255, 255, 255, _Alpha));
		driver.renderTriangles(_MaterialPassThru, 0, FVB_NUM_TRIS);
		_NumRenderedFaces = 0; // start to render again
	}
	driver.setRenderTarget(NULL);
}

// *******************************************************************************
void CWaterEnvMap::doInit()
{
	if (!_FlattenVBInitialized)
	{
		initFlattenVB();
		initTestVB();
		_FlattenVBInitialized = true;
		_MaterialPassThru.setLighting(false);
		_MaterialPassThru.texEnvOpRGB(0, CMaterial::Replace);
		_MaterialPassThru.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_MaterialPassThru.texEnvOpAlpha(0, CMaterial::Replace);
		_MaterialPassThru.texEnvArg0Alpha(0, CMaterial::Constant, CMaterial::SrcAlpha);
		_MaterialPassThru.texConstantColor(0, CRGBA(255, 255, 255, 255));
		_MaterialPassThru.setDoubleSided(true);
		_MaterialPassThruZTest = _MaterialPassThru;
		_MaterialPassThru.setZWrite(false);
		_MaterialPassThru.setZFunc(CMaterial::always);
	}
}

static const char *testMeshVPstr =
"!!VP1.0\n\
 DP4 o[HPOS].x, c[0], v[0]; \n\
 DP4 o[HPOS].y, c[1], v[0]; \n\
 DP4 o[HPOS].z, c[2], v[0]; \n\
 DP4 o[HPOS].w, c[3], v[0]; \n\
 MAD o[COL0], v[8], c[4].xxxx, c[4].yyyy;   \n\
 MOV o[TEX0], v[8];         \n\
 END";


class CVertexProgramTestMeshVP : public CVertexProgram
{
public:
	struct CIdx
	{
		uint ProgramConstant0;
	};
	CVertexProgramTestMeshVP()
	{
		// nelvp
		{
			CSource *source = new CSource();
			source->Profile = nelvp;
			source->DisplayName = "testMeshVP/nelvp";
			source->setSourcePtr(testMeshVPstr);
			source->ParamIndices["modelViewProjection"] = 0;
			source->ParamIndices["programConstant0"] = 4;
			addSource(source);
		}
		// TODO_VP_GLSL
	}
	virtual ~CVertexProgramTestMeshVP()
	{
		
	}
	virtual void buildInfo()
	{
		m_Idx.ProgramConstant0 = getUniformIndex("programConstant0");
		nlassert(m_Idx.ProgramConstant0 != ~0);
	}
	inline const CIdx &idx() { return m_Idx; }
private:
	CIdx m_Idx;
};


static NLMISC::CSmartPtr<CVertexProgramTestMeshVP> testMeshVP;



// *******************************************************************************
void CWaterEnvMap::renderTestMesh(IDriver &driver)
{
	if (!testMeshVP)
	{
		testMeshVP = new CVertexProgramTestMeshVP();
	}

	doInit();
	CMaterial testMat;
	testMat.setLighting(false);
	testMat.texEnvOpRGB(0, CMaterial::Modulate);
	testMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
	testMat.texEnvArg0RGB(1, CMaterial::Diffuse, CMaterial::SrcColor);
	testMat.texEnvOpAlpha(0, CMaterial::Replace);
	testMat.texEnvArg0Alpha(0, CMaterial::Constant, CMaterial::SrcAlpha);
	testMat.texConstantColor(0, CRGBA(255, 255, 255, 255));
	testMat.setDoubleSided(true);
	testMat.setZWrite(false);
	testMat.setZFunc(CMaterial::always);
	// tmp : test cubemap
	driver.activeVertexProgram(testMeshVP);
	driver.activeVertexBuffer(_TestVB);
	driver.activeIndexBuffer(_TestIB);
	_MaterialPassThruZTest.setTexture(0, _EnvCubic);
	driver.setUniformMatrix(IDriver::VertexProgram, testMeshVP->getUniformIndex(CProgramIndex::ModelViewProjection), IDriver::ModelViewProjection, IDriver::Identity);
	driver.setUniform2f(IDriver::VertexProgram, testMeshVP->idx().ProgramConstant0, 2.f, 1.f);
	//driver.renderTriangles(testMat, 0, TEST_VB_NUM_TRIS);
	driver.renderTriangles(_MaterialPassThruZTest, 0, TEST_VB_NUM_TRIS);
	driver.activeVertexProgram(NULL);
}

// *******************************************************************************
void CWaterEnvMap::initFlattenVB()
{
	_FlattenVB.setPreferredMemory(CVertexBuffer::AGPPreferred, true);
	_FlattenVB.setName("Flatten VB");
	_FlattenVB.clearValueEx();
	_FlattenVB.addValueEx (CVertexBuffer::Position, CVertexBuffer::Float3);
	_FlattenVB.addValueEx (CVertexBuffer::TexCoord0, CVertexBuffer::Float3);
	_FlattenVB.initEx();
	nlctassert(FVB_NUM_SIDES % 4 == 0); // number of sides must be a multiple of 4 so that sections sides will align with corners
	_FlattenVB.setNumVertices(FVB_NUM_VERTS);
	_FlattenIB.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	_FlattenIB.setNumIndexes(3 * FVB_NUM_TRIS);
	{
		CVertexBufferReadWrite vbrw;
		CIndexBufferReadWrite ibrw;
		_FlattenVB.lock(vbrw);
		_FlattenIB.lock(ibrw);
		for(uint l = 0; l < FVB_NUM_SIDES; ++l)
		{
			double angle = NLMISC::Pi * 0.25 + 2 * NLMISC::Pi * (double) l / (double) FVB_NUM_SIDES;
			for(uint k = 0; k < FVB_NUM_SECTIONS + 1; ++k)
			{
				double radius = (double) k / (double) (FVB_NUM_SECTIONS - 1);
				float x = (float) (radius * cos(angle));
				float y = (float) (radius * sin(angle));
				if (k < FVB_NUM_SECTIONS)
				{
					ibrw.setTri(3 * 2 * (k + (l * FVB_NUM_SECTIONS)), getFVBVertex(k, l), getFVBVertex(k + 1, l + 1), getFVBVertex(k + 1, l));
					ibrw.setTri(3 * (2 * (k + (l * FVB_NUM_SECTIONS)) + 1), getFVBVertex(k, l), getFVBVertex(k, l + 1), getFVBVertex(k + 1, l + 1));
				}
				else
				{
					uint side = l / (FVB_NUM_SIDES / 4);
					switch(side)
					{
						case 0: // top
							x /= y;
							y = 1.f;
						break;
						case 1: // left
							y /= -x;
							x = -1.f;
						break;
						case 2: // bottom
							x /= -y;
							y = -1.f;
						break;
						case 3: // right
							y /= x;
							x = 1.f;
						break;
						default:
							nlassert(0);
						break;
					}
				}
				CVector dir;
				//dir.sphericToCartesian(1.f, (float) angle, (float) (NLMISC::Pi * 0.5 * acos(std::max(0.f, (1.f - (float) k / (FVB_NUM_SECTIONS - 1))))));
				dir.sphericToCartesian(1.f, (float) angle, (float) acos(std::min(1.f, (float) k / (FVB_NUM_SECTIONS - 1))));
				vbrw.setValueFloat3Ex(CVertexBuffer::Position, getFVBVertex(k, l), x, 0.5f, y);
				vbrw.setValueFloat3Ex(CVertexBuffer::TexCoord0, getFVBVertex(k, l), -dir.x, dir.z, -dir.y);
			}
		}
	}
}

// *******************************************************************************
void CWaterEnvMap::invalidate()
{
	_LastRenderTime = -1;
	_StartRenderTime = -1;
	if (_UpdateTime == 0)
	{
		_LastRenderTick = std::numeric_limits<uint64>::max();
	}
	else
	{
		_LastRenderTick -= (NUM_FACES_TO_RENDER + 1);
	}
	_NumRenderedFaces = 0;
}

// *******************************************************************************
void CWaterEnvMap::initTestVB()
{
	_TestVB.setPreferredMemory(CVertexBuffer::AGPPreferred, true);
	_TestVB.setName("TestVB");
	_TestVB.clearValueEx();
	_TestVB.addValueEx (CVertexBuffer::Position, CVertexBuffer::Float3);
	_TestVB.addValueEx (CVertexBuffer::TexCoord0, CVertexBuffer::Float3);
	_TestVB.initEx();
	_TestVB.setNumVertices(TEST_VB_NUM_SEGMENT * 2 * (TEST_VB_NUM_SLICE + 1));
	_TestIB.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	_TestIB.setNumIndexes(3 * TEST_VB_NUM_TRIS);
	{
		CVertexBufferReadWrite vbrw;
		CIndexBufferReadWrite ibrw;
		_TestVB.lock(vbrw);
		_TestIB.lock(ibrw);
		uint triIndex = 0;
		for(uint k = 0; k < TEST_VB_NUM_SEGMENT; ++k)
		{
			float theta = 2 * (float) (NLMISC::Pi * (double) k / (double) TEST_VB_NUM_SEGMENT);
			for(uint l = 0; l <= TEST_VB_NUM_SLICE; ++l)
			{
				float phi = (float) (NLMISC::Pi / 2 * (1 - 2 * (double) l / (double) TEST_VB_NUM_SLICE));
				CVector pos;
				pos.sphericToCartesian(1.f, theta, phi);
				#define VERT_INDEX(k, l) ((l) + ((k) %  TEST_VB_NUM_SEGMENT) * (TEST_VB_NUM_SLICE + 1))
				vbrw.setVertexCoord(VERT_INDEX(k, l), pos);
				vbrw.setValueFloat3Ex(CVertexBuffer::TexCoord0, VERT_INDEX(k, l), pos);
				if (l != TEST_VB_NUM_SLICE)
				{
					ibrw.setTri(3 * triIndex++, VERT_INDEX(k, l), VERT_INDEX(k + 1, l), VERT_INDEX(k + 1, l + 1));
					ibrw.setTri(3 * triIndex++, VERT_INDEX(k, l), VERT_INDEX(k + 1, l + 1), VERT_INDEX(k, l + 1));
				}
			}
		}
		nlassert(triIndex == TEST_VB_NUM_TRIS);
	}
}


} // NL3D
