// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "decal.h"
//
#include "nel/3d/shadow_map.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/landscape.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/landscape_user.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/texture_user.h"
//
// TMP TMP
#include "nel/3d/texture_mem.h"
//
#include "nel/misc/aabbox.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/plane.h"
//
#include "global.h"
//
#include "interface_v3/interface_manager.h"

using namespace NL3D;
using namespace NLMISC;

CDecalRenderList DecalRenderList;

extern uint	SkipFrame;

NL3D::CVertexBuffer CDecal::_VB;
bool CDecal::_VBInitialized = false;



static const char *DecalAttenuationVertexProgramCode =
"!!VP1.0																			\n\
	DP4 o[HPOS].x, c[0], v[0];	          #transform vertex in view space	        \n\
	DP4 o[HPOS].y, c[1], v[0];												        \n\
	DP4 o[HPOS].z, c[2], v[0];												        \n\
	DP4 o[HPOS].w, c[3], v[0];												        \n\
	# transform texcoord 0															\n\
	DP4 o[TEX0].x, c[4], v[0];														\n\
	DP4 o[TEX0].y, c[5], v[0];														\n\
	#compute distance from camera													\n\
	ADD R0, v[0], -c[6];															\n\
	DP3 R0.x, R0, R0;																\n\
	RSQ R0.x, R0.x;																	\n\
	RCP R0.x, R0.x;																	\n\
	MUL o[COL0].xyz, c[8], v[3];													\n\
	#compute attenuation with distance												\n\
	MAD R0.w, R0.x, c[7].x, c[7].y;													\n\
	# clamp	in [0, 1]																\n\
	MIN R0.w, R0.w, c[7].w;															\n\
	MAX R0.w, R0.w, c[7].z;															\n\
	#compute bottom blend															\n\
	MAD R1.x, v[0].z, c[11].x, c[11].y;												\n\
	MIN R1.x, R1.x, c[7].w;															\n\
	MAX R1.x, R1.x, c[7].z;															\n\
	MUL R0.w, R1.x, R0.w;															\n\
	#compute top blend																\n\
	MAD R1.x, v[0].z, c[11].z, c[11].w;												\n\
	MIN R1.x, R1.x, c[7].w;															\n\
	MAX R1.x, R1.x, c[7].z;															\n\
	MUL R0.w, R1.x, R0.w;															\n\
	#apply vertex alpha																\n\
	MUL o[COL0].w, v[3], R0.w;														\n\
	END \n";

class CVertexProgramDecalAttenuation : public CVertexProgram
{
public:
	struct CIdx
	{
		// 0-3 mvp
		uint WorldToUV0; // 4
		uint WorldToUV1; // 5
		uint RefCamDist; // 6
		uint DistScaleBias; // 7
		uint Diffuse; // 8
		// 9
		// 10
		uint BlendScale; // 11
	};
	CVertexProgramDecalAttenuation()
	{
		// nelvp
		{
			CSource *source = new CSource();
			source->Profile = nelvp;
			source->DisplayName = "nelvp/DecalAttenuation";
			source->setSourcePtr(DecalAttenuationVertexProgramCode);
			source->ParamIndices["modelViewProjection"] = 0;
			source->ParamIndices["worldToUV0"] = 4;
			source->ParamIndices["worldToUV1"] = 5;
			source->ParamIndices["refCamDist"] = 6;
			source->ParamIndices["distScaleBias"] = 7;
			source->ParamIndices["diffuse"] = 8;
			source->ParamIndices["blendScale"] = 11;
			addSource(source);
		}
		// TODO_VP_GLSL
	}
	~CVertexProgramDecalAttenuation()
	{
		
	}
	virtual void buildInfo()
	{
		m_Idx.WorldToUV0 = getUniformIndex("worldToUV0");
		m_Idx.WorldToUV1 = getUniformIndex("worldToUV1");
		m_Idx.RefCamDist = getUniformIndex("refCamDist");
		m_Idx.DistScaleBias = getUniformIndex("distScaleBias");
		m_Idx.Diffuse = getUniformIndex("diffuse");
		m_Idx.BlendScale = getUniformIndex("blendScale");
	}
	inline const CIdx &idx() const { return m_Idx; }
private:
	CIdx m_Idx;
};

static CVertexProgramDecalAttenuation DecalAttenuationVertexProgram;


typedef CShadowPolyReceiver::CRGBAVertex CRGBAVertex;

// ****************************************************************************
CDecal::CDecal()
{
	_ShadowMap = new CShadowMap(&(((CSceneUser *) Scene)->getScene().getRenderTrav().getShadowMapManager()));
	_Material.initUnlit();
	_Diffuse = CRGBA::White;
	_Emissive = CRGBA::Black;
	//
	_Material.setBlend(true);
	_Material.setSrcBlend(CMaterial::srcalpha);
	_Material.setDstBlend(CMaterial::invsrcalpha);
	_Material.setZWrite(false);
	_Material.setDoubleSided(true);
	// diffuse color applied at first stage
	_Material.texEnvOpRGB(0,   CMaterial::Modulate);
	_Material.texEnvArg0RGB(0,   CMaterial::Texture,   CMaterial::SrcColor);
	_Material.texEnvArg1RGB(0,   CMaterial::Diffuse,   CMaterial::SrcColor);
	_Material.texEnvOpAlpha(0,   CMaterial::Modulate);
	_Material.texEnvArg0Alpha(0,   CMaterial::Diffuse,   CMaterial::SrcAlpha);
	_Material.texEnvArg1Alpha(0,   CMaterial::Texture,   CMaterial::SrcAlpha);
	//
	_Material.texEnvOpRGB(1,   CMaterial::Add);
	_Material.texEnvArg0RGB(1,   CMaterial::Previous,   CMaterial::SrcColor);
	_Material.texEnvArg1RGB(1,   CMaterial::Constant,   CMaterial::SrcColor);
	_Material.texEnvOpAlpha(1,   CMaterial::Modulate);
	_Material.texEnvArg0Alpha(1,   CMaterial::Previous,   CMaterial::SrcAlpha);
	_Material.texEnvArg1Alpha(1,   CMaterial::Constant,   CMaterial::SrcAlpha);
	//
	setEmissive(CRGBA::Black);
	setDiffuse(CRGBA::White);
	//
	_Material.setAlphaTest(true);
	_Material.setAlphaTestThreshold(1.f / 255.f);
	//
	_Touched = true;
	_ClipDownFacing = false;
	_WorldMatrix.get(_WorldMatrixFlat);
	setWorldMatrix(_WorldMatrix);
	//
	_BottomBlendZMin = -10100.f;
	_BottomBlendZMax = -10000.f;
	_TopBlendZMin = 10000.f;
	_TopBlendZMax = 10100.f;
}

// ****************************************************************************
void CDecal::setCustomUVMatrix(bool on, const NLMISC::CMatrix &matrix)
{
	if (_CustomUVMatrix.set(on, matrix))
	{
		_Touched = true;
	}
}

// ****************************************************************************
const std::string &CDecal::getTextureFileName() const
{
	CTextureFile *tf = dynamic_cast<CTextureFile *>(_Material.getTexture(0));
	if (tf) return tf->getFileName();
	static std::string emptyString;
	return emptyString;
}

// ****************************************************************************
void CDecal::setupMaterialColor()
{
	_Material.texConstantColor(1,  NLMISC::CRGBA(_Emissive.R, _Emissive.G, _Emissive.B, _Diffuse.A));
}

// ****************************************************************************
void CDecal::setEmissive(NLMISC::CRGBA emissive)
{
	_Emissive = emissive;
	setupMaterialColor();
}

// ****************************************************************************
void CDecal::setDiffuse(NLMISC::CRGBA diffuse)
{
	_Diffuse = diffuse;
	setupMaterialColor();
}

// ****************************************************************************
CRGBA CDecal::getDiffuse() const
{
	return _Diffuse;
}

// ****************************************************************************
CDecal::~CDecal()
{
	delete _ShadowMap;
}

// ****************************************************************************
void CDecal::setTexture(const std::string &fileName,   bool clampU,   bool clampV, bool filtered)
{
	if (getTextureFileName() != fileName)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CViewRenderer &vr = *CViewRenderer::getInstance();
		UTexture *tex = vr.getGlobalTexture(fileName);
		if (tex != NULL)
		{
			_Material.setTexture(0,  (dynamic_cast<NL3D::CTextureUser *>(tex))->getITexture());
		}
		else
		{
			_Material.setTexture(0,  fileName.empty() ? NULL : new CTextureFile(fileName));
		}
		if (_Material.getTexture(0))
		{
			_Material.getTexture(0)->setUploadFormat(ITexture::RGBA8888); // don't want ugly dxtc mipmaps for most decals
		}
	}
	if (_Material.getTexture(0))
	{
		_Material.getTexture(0)->setWrapS(clampU ? ITexture::Clamp : ITexture::Repeat);
		_Material.getTexture(0)->setWrapT(clampV ? ITexture::Clamp : ITexture::Repeat);
		if (filtered)
		{
			_Material.getTexture(0)->setFilterMode(ITexture::Linear ,  ITexture::LinearMipMapLinear);
		}
		else
		{
			_Material.getTexture(0)->setFilterMode(ITexture::Nearest,  ITexture::NearestMipMapOff);
		}
		_Material.setTexture(1, _Material.getTexture(0));
	}
	else
	{
		_Material.setTexture(1, NULL);
	}
}

// ****************************************************************************
void CDecal::setWorldMatrix(const NLMISC::CMatrix &matrix)
{
	float newMat[16];
	matrix.get(newMat);
	if (std::equal(newMat, newMat + 16, _WorldMatrixFlat)) return;
	_WorldMatrix = matrix;
	_WorldMatrix.get(_WorldMatrixFlat);
	_InvertedWorldMatrix = matrix.inverted();
	_Touched = true;
	const float bboxHeight = 10000.f;
	static const NLMISC::CVector corners[8] =
	{
		CVector(0.f, 0.f, - bboxHeight),
		CVector(1.f, 0.f, - bboxHeight),
		CVector(0.f, 1.f, - bboxHeight),
		CVector(1.f, 1.f, - bboxHeight),
		CVector(0.f, 0.f, bboxHeight),
		CVector(1.f, 0.f, bboxHeight),
		CVector(0.f, 1.f, bboxHeight),
		CVector(1.f, 1.f, bboxHeight),
	};
	for(uint k = 0; k < 8; ++k)
	{
		_ClipCorners[k] = _WorldMatrix * corners[k];
	}
}

// ****************************************************************************
bool CDecal::clipFront(const NLMISC::CPlane &p) const
{
	for(uint k = 0; k < 8; ++k)
	{
		if (p * _ClipCorners[k] <= 0.f) return false;
	}
	return true;
}

// ****************************************************************************
void CDecal::setWorldMatrixForArrow(const NLMISC::CVector2f &start,    const NLMISC::CVector2f &end,    float halfWidth)
{
	CMatrix matrix;
	CVector I = CVector(end.x,    end.y,    0.f) - CVector(start.x,    start.y,    0.f);
	CVector J = 2.f * halfWidth * CVector::K ^ I.normed();
	matrix.setRot(I,    J,    CVector::K);
	matrix.setPos(start - 0.5f * J);
	setWorldMatrix(matrix);
}

// ****************************************************************************
void CDecal::setWorldMatrixForSpot(const NLMISC::CVector2f &pos,  float radius, float angleInRadians)
{
	CMatrix matrix;
	matrix.rotateZ(angleInRadians);
	matrix.setScale(2.f * radius);
	matrix.setPos(pos - radius * CVector2f(1.f,  1.f));
	setWorldMatrix(matrix);
}


NLMISC::CVector r2MaskOffset(1.f / 4.f,  1.f / 4.f,  0.f);


// ****************************************************************************
void CDecal::renderTriCache(NL3D::IDriver &drv,   NL3D::CShadowPolyReceiver &/* receiver */, bool useVertexProgram)
{
	if (_TriCache.empty()) return;
	if (!_VBInitialized)
	{
		_VB.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
		_VB.setVertexFormat(CVertexBuffer::PositionFlag|CVertexBuffer::PrimaryColorFlag);
		_VBInitialized = true;
	}
	CMatrix modelMat;
	modelMat.setPos(_RefPosition);
	drv.setupModelMatrix(modelMat);
	if (useVertexProgram)
	{
		{
			CVertexBufferReadWrite vba;
			_VB.setNumVertices((uint32)_TriCache.size());
			_VB.lock(vba);
			memcpy(vba.getVertexCoordPointer(), &_TriCache[0], sizeof(CRGBAVertex) * _TriCache.size());
		}
		drv.activeVertexBuffer(_VB);
		drv.setUniformMatrix(IDriver::VertexProgram, DecalAttenuationVertexProgram.getUniformIndex(CGPUProgramIndex::ModelViewProjection), NL3D::IDriver::ModelViewProjection, NL3D::IDriver::Identity);
		drv.setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().WorldToUV0, _WorldToUVMatrix[0][0], _WorldToUVMatrix[1][0], _WorldToUVMatrix[2][0], _WorldToUVMatrix[3][0]);
		drv.setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().WorldToUV1, _WorldToUVMatrix[0][1], _WorldToUVMatrix[1][1], _WorldToUVMatrix[2][1], _WorldToUVMatrix[3][1]);
		drv.setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().Diffuse, _Diffuse.R * (1.f / 255.f), _Diffuse.G * (1.f / 255.f), _Diffuse.B * (1.f / 255.f), 1.f);
		const NLMISC::CVector &camPos = MainCam.getMatrix().getPos();
		drv.setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().RefCamDist, camPos.x - _RefPosition.x, camPos.y - _RefPosition.y, camPos.z - _RefPosition.z, 1.f);
		// bottom & top blend
		float bottomBlendScale = 1.f / favoid0(_BottomBlendZMax - _BottomBlendZMin);
		float topBlendScale = 1.f / favoid0(_TopBlendZMin - _TopBlendZMax);
		drv.setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().BlendScale, bottomBlendScale, bottomBlendScale * (_RefPosition.z - _BottomBlendZMin),
							topBlendScale, topBlendScale * (_RefPosition.z - _TopBlendZMax));
		//
		static volatile bool wantSimpleMat = false;
		if (wantSimpleMat)
		{
			static CMaterial simpleMat;
			static volatile bool disableStencil = false;
			if (disableStencil)
			{
				drv.enableStencilTest(false);
			}
			simpleMat.initUnlit();
			simpleMat.setTexture(0, _Material.getTexture(0));
			simpleMat.texEnvOpRGB(0, CMaterial::Replace);
			simpleMat.texEnvArg0RGB(0, CMaterial::Constant, CMaterial::SrcColor);
			simpleMat.setDoubleSided(true);
			simpleMat.texConstantColor(0, CRGBA::White);
			drv.renderRawTriangles(simpleMat, 0, (uint32)_TriCache.size() / 3);
			IDriver::TPolygonMode pm = drv.getPolygonMode();
			drv.setPolygonMode(IDriver::Line);
			simpleMat.texConstantColor(0, CRGBA::Red);
			drv.renderRawTriangles(simpleMat, 0, (uint32)_TriCache.size() / 3);
			drv.setPolygonMode(pm);
		}
		else
		{
			drv.renderRawTriangles(_Material, 0, (uint32)_TriCache.size() / 3);
		}
	}
	else
	{
		{
			CVertexBufferReadWrite vba;
			_VB.setNumVertices((uint32)_TriCache.size());
			_VB.lock(vba);
			NLMISC::CRGBA col = _Diffuse;
			if (drv.getVertexColorFormat()==CVertexBuffer::TBGRA)
			{
				std::swap(col.R, col.B);
			}
			CRGBAVertex *dest = (CRGBAVertex *) vba.getVertexCoordPointer();
			const CRGBAVertex *destEnd = dest + _TriCache.size();
			const CRGBAVertex *srcVert = &_TriCache[0];
			const NLMISC::CVector camPos = MainCam.getMatrix().getPos() - _RefPosition;
			float scale = 255.f * CDecalRenderList::getInstance()._DistScale;
			float bias = 255.f * CDecalRenderList::getInstance()._DistBias;
			float bottomBlendScale = 1.f / favoid0(_BottomBlendZMax - _BottomBlendZMin);
			float bottomBlendBias = bottomBlendScale * (_RefPosition.z - _BottomBlendZMin);
			do
			{
				dest->V = srcVert->V;
				float dist = (camPos - srcVert->V).norm();
				float intensity = scale * dist + bias;
				float bottomBlend = srcVert->V.z * bottomBlendScale + bottomBlendBias;
				clamp(bottomBlend, 0.f, 1.f);
				clamp(intensity, 0.f, 255.f);
				intensity *= bottomBlend;
				col.A = (uint8) (((uint16) intensity * (uint16) srcVert->Color.A) >> 8);
				dest->Color = col;
				++dest;
				++srcVert;
			}
			while (dest != destEnd);
		}
		drv.activeVertexBuffer(_VB);
		static volatile bool wantSimpleMat2 = false;
		if (wantSimpleMat2)
		{
			static CMaterial simpleMat2;
			simpleMat2.initUnlit();
			simpleMat2.setDoubleSided(true);
			drv.renderRawTriangles(simpleMat2, 0, (uint32)_TriCache.size() / 3);
		}
		else
		{
			drv.renderRawTriangles(_Material, 0, (uint32)_TriCache.size() / 3);
		}
	}
}

// ****************************************************************************
void CDecal::render(NL3D::UDriver &/* drv */,
					NL3D::CShadowPolyReceiver &receiver,
					const std::vector<CPlane> &worldPyramid,
					const std::vector<NLMISC::CVector> &pyramidCorners,
					bool useVertexProgram
				   )
{
	const NLMISC::CVector &camPos = MainCam.getMatrix().getPos();
	if ((camPos - _LastCamPos).norm() >= 4.f)
	{
		_Touched = true;
	}
	if (_Touched)
	{
		_LastCamPos = camPos;
	}
	// if out of only 1 plane, entirely out.
	for(sint i=0;i<(sint)worldPyramid.size();i++)
	{
		if (clipFront(worldPyramid[i])) return;
	}
	// do finer clip
	uint inside = 0;
	for(uint l = 0; l < pyramidCorners.size(); ++l)
	{
		NLMISC::CVector localCorner = _InvertedWorldMatrix * pyramidCorners[l];
		if (localCorner.x >= 0.f) inside |= 1;
		if (localCorner.x <= 1.f) inside |= 2;
		if (localCorner.y >= 0.f) inside |= 4;
		if (localCorner.y <= 1.f) inside |= 8;
	}
	if(inside != 0xf) return;

	// must setup attenuation texture each frame
	/*
	_Material.enableUserTexMat(1,    true);
	_Material.setTexCoordGen(1,    true);
	_Material.setTexCoordGenMode(1,    CMaterial::TexCoordGenObjectSpace);
	// object is in world space
	float scale = 1.f / tileNear;
	CMatrix attenMat;
	attenMat.setScale(CVector(0.5f * scale, tileNear, 0.f));
	attenMat.setPos(CVector(-0.5f * (1.f - camPos.x * scale), -0.5f * (1.f - camPos.y * scale), 0.f));
	_Material.setUserTexMat(1,    attenMat);
	*/
	//
	if (!_Touched)
	{
		NL3D::IDriver *drvInternal = ((CDriverUser *) Driver)->getDriver();
		renderTriCache(*drvInternal, receiver, useVertexProgram);
		return;
	}
	//
	float tileNear = Landscape->getTileNear();
	//
	nlassert(_ShadowMap);
	_ShadowMap->LocalClipPlanes.resize(4);
	CVector corners[4] =
	{
		_WorldMatrix * CVector(0.f,    1.f,    0.f),
		_WorldMatrix * CVector(1.f,    1.f,    0.f),
		_WorldMatrix * CVector(1.f,    0.f,    0.f),
		_WorldMatrix * CVector(0.f,    0.f,    0.f)
	};
	CAABBox bbox;
	_ShadowMap->LocalBoundingBox.setMinMax(corners[0],    corners[1]);
	_ShadowMap->LocalBoundingBox.extend(corners[2]);
	_ShadowMap->LocalBoundingBox.extend(corners[3]);

	for(uint k = 0; k < 4; ++k)
	{
		_ShadowMap->LocalClipPlanes[k].make(corners[k],    corners[(k + 1) & 3],    corners[k] + (corners[(k + 1) & 3] - corners[k]).norm() * CVector::K);
		_ShadowMap->LocalClipPlanes[k].invert();
	}

	_RefPosition = MainCam.getMatrix().getPos();


	// set uv matrix to match the world matrix
	// matrix to map (x,   y ) = (0,    0) to (u,   v) = (0,    1) & (x,   y ) = (0,    1) to (u,   v) = (0,    0) in local decal space
	CMatrix reverseUVMatrix;
	reverseUVMatrix.setRot(CVector::I,    -CVector::J,    CVector::K);
	reverseUVMatrix.setPos(CVector::J);
	CMatrix worldToUVMatrix = _CustomUVMatrix.On ? _CustomUVMatrix.Matrix :
													   (_TextureMatrix * reverseUVMatrix * _InvertedWorldMatrix);
	CMatrix refPosMatrix;
	refPosMatrix.setPos(_RefPosition);
	worldToUVMatrix = worldToUVMatrix * refPosMatrix;

	worldToUVMatrix.get((float *) _WorldToUVMatrix);
	// stage 0
	if (useVertexProgram)
	{
		_Material.enableUserTexMat(0, false);
	}
	else
	{
		_Material.enableUserTexMat(0,    true);
		_Material.setTexCoordGen(0,    true);
		_Material.setTexCoordGenMode(0,    CMaterial::TexCoordGenObjectSpace);
		_Material.setUserTexMat(0, worldToUVMatrix);
	}
	//
	//
	static NLMISC::CPolygon clipPoly;
	static NLMISC::CPolygon2D clipPoly2D;
	clipPoly.Vertices.resize(4);
	std::copy(corners, corners + 4, clipPoly.Vertices.begin());
	// clip with by "near tiles" for better selection (avoid unwanted wrapping during triangle selection ...)
	CPlane planes[4];
	planes[0].make(CVector::J, camPos + tileNear * CVector::J),
	planes[1].make(-CVector::J, camPos - tileNear * CVector::J),
	planes[2].make(CVector::I, camPos + tileNear * CVector::I),
	planes[3].make(-CVector::I, camPos - tileNear * CVector::I);
	uint numVerts = (uint)clipPoly.Vertices.size();
	clipPoly2D.Vertices.resize(numVerts);
	for (uint k = 0; k < numVerts; ++k)
	{
		clipPoly2D.Vertices[k].set(clipPoly.Vertices[k].x, clipPoly.Vertices[k].y);
	}
	NL3D::IDriver *drvInternal = ((CDriverUser *) Driver)->getDriver();

	// rebuild the triangle cache
	if (SkipFrame == 0) // don't update just after a tp because landscape hasn't been updated yet ...
	{
		_Touched = false;
	}
	// compute tris near the camera to avoid precision z-preision problems due to huge translation in the world matrix)
	receiver.computeClippedTrisWithPolyClip(_ShadowMap, CVector::Null, - _RefPosition, clipPoly2D, _TriCache, _ClipDownFacing);
	_Material.setZBias(-0.06f);
	renderTriCache(*drvInternal, receiver, useVertexProgram);
}

// ****************************************************************************
void CDecalRenderList::renderAllDecals()
{
	if (_Empty) return;

	if( !Landscape)
	{
		return;
	}
	Driver->enableFog(false);
	MainCam.buildCameraPyramid(_WorldCamPyramid, false);
	MainCam.buildCameraPyramidCorners(_WorldCamPyramidCorners, false);
	Driver->setModelMatrix(CMatrix::Identity);
	CLandscapeModel *landscapeModel = ((CLandscapeUser *) Landscape)->getLandscape();
	CShadowPolyReceiver &shadowPolyReceiver = landscapeModel->Landscape.getShadowPolyReceiver();
	//
	float maxDist = Landscape->getTileNear() * 0.9f;
	const float threshold = 0.8f; // ratio over the whole dist at which the fade out begins
	float factor = 1.f / (1.f - threshold);
	_DistScale = - factor / maxDist;
	_DistBias = factor;
	//
	bool useVertexProgram = false;
	NL3D::IDriver *drvInternal = ((CDriverUser *) Driver)->getDriver();
	//
	static volatile bool forceNoVertexProgram = false;
	if (!forceNoVertexProgram && drvInternal->compileVertexProgram(&DecalAttenuationVertexProgram))
	{
		drvInternal->activeVertexProgram(&DecalAttenuationVertexProgram);
		//drvInternal->setConstantMatrix(0, NL3D::IDriver::ModelViewProjection, NL3D::IDriver::Identity);
		drvInternal->setUniform4f(IDriver::VertexProgram, DecalAttenuationVertexProgram.idx().DistScaleBias, _DistScale, _DistBias, 0.f, 1.f);
		useVertexProgram = true;
	}
	else
	{
		drvInternal->activeVertexProgram(NULL);
	}
	for(uint k = 0; k < DECAL_NUM_PRIORITIES; ++k)
	{
		std::vector<CDecal::TRefPtr> &renderList = _RenderList[k];
		for(uint l = 0; l < renderList.size(); ++l)
		{
			if (renderList[l])
			{
				renderList[l]->render(*Driver, shadowPolyReceiver, _WorldCamPyramid, _WorldCamPyramidCorners, useVertexProgram);
			}
		}
	}
	if (useVertexProgram)
	{
		drvInternal->activeVertexProgram(NULL);
	}
}

// ****************************************************************************
void CDecalRenderList::clearRenderList()
{
	for(uint k = 0; k < DECAL_NUM_PRIORITIES; ++k)
	{
		_RenderList[k].clear();
	}
	_Empty = true;
}

// ****************************************************************************
void CDecal::addToRenderList(uint priority /*=0*/)
{
	if( !Landscape)
	{
		return;
	}
	nlassert(priority < DECAL_NUM_PRIORITIES);
	CDecalRenderList &drl = CDecalRenderList::getInstance();
	drl._RenderList[priority].push_back(this);
	drl._Empty = false;
}

// ****************************************************************************
bool CDecal::contains(const NLMISC::CVector2f &pos) const
{
	CVector posIn = _InvertedWorldMatrix * CVector(pos.x, pos.y, 0.f);
	return posIn.x >= 0.f && posIn.x <= 1.f && posIn.y >= 0.f && posIn.y <= 1.f;
}

// ****************************************************************************
void CDecal::setClipDownFacing(bool clipDownFacing)
{
	if (clipDownFacing != _ClipDownFacing)
	{
		_Touched = true;
		_ClipDownFacing = clipDownFacing;
	}
}

// ****************************************************************************
void CDecal::setBottomBlend(float zMin, float zMax)
{
	if (zMin > zMax) std::swap(zMin, zMax);
	_BottomBlendZMin = zMin;
	_BottomBlendZMax = zMax;
}

// ****************************************************************************
void CDecal::setTopBlend(float zMin, float zMax)
{
	if (zMin > zMax) std::swap(zMin, zMax);
	_TopBlendZMin = zMin;
	_TopBlendZMax = zMax;
}




