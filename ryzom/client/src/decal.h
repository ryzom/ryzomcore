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

#ifndef RY_DECAL_H
#define RY_DECAL_H

#include "nel/misc/vector.h"
#include "nel/misc/singleton.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vector_h.h"
//
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/shadow_poly_receiver.h"
//
#include "custom_matrix.h"


namespace NLMISC
{
	class CPlane;
}

namespace NL3D
{
	class CScene;
	class CShadowMap;
	class CShadowPolyReceiver;
	//
	class UScene;
	class UDriver;
}

const uint DECAL_NUM_PRIORITIES = 8;

// Helper class to display a decal on a poly receiver
// Default decal is a unit rectangle (0, 0) - (1, 1)
// TODO nico : put this in NL3D when working ? ...
class CDecal : public NLMISC::CRefCount
{
public:
	typedef NLMISC::CRefPtr<CDecal> TRefPtr;
	typedef NLMISC::CSmartPtr<CDecal> TSmartPtr;
	CDecal();
	~CDecal();
	// Set a texture from its filename. It name match a global texture in the view renderer, it will be used ,first
	void setTexture(const std::string &fileName, bool clampU = true, bool clampV = true, bool filtered = true);
	NL3D::ITexture *getTexture() { return _Material.getTexture(0); }
	const std::string &getTextureFileName() const;
	void setTextureMatrix(const NLMISC::CMatrix &matrix) { _TextureMatrix = matrix; }
	void setDiffuse(NLMISC::CRGBA diffuse);
	NLMISC::CRGBA getDiffuse() const;
	void setEmissive(NLMISC::CRGBA emissive);
	//
	void setWorldMatrix(const NLMISC::CMatrix &matrix);
	void setWorldMatrixForArrow(const NLMISC::CVector2f &start, const NLMISC::CVector2f &end, float halfWidth);
	void setWorldMatrixForSpot(const NLMISC::CVector2f &pos, float radius, float angleInRadians = 0.f);
	// should be called if the decal should be made visible this frame
	void addToRenderList(uint priority = 0);
	// test if a point intersect with this decal
	bool contains(const NLMISC::CVector2f &pos) const;
	// set a custom uv matrix (from world to uvs)
	void setCustomUVMatrix(bool on, const NLMISC::CMatrix &matrix = NLMISC::CMatrix::Identity);
	//
	void setClipDownFacing(bool clipDownFacing);
	bool getClipDownFacing() const { return _ClipDownFacing; }
	//
	void setBottomBlend(float zMin, float zMax);
	void setTopBlend(float zMin, float zMax);
private:
	mutable NL3D::CMaterial				_Material;
	NLMISC::CMatrix						_WorldMatrix;
	float								_WorldMatrixFlat[16];
	NLMISC::CVector						_ClipCorners[8];
	NLMISC::CMatrix						_InvertedWorldMatrix;
	NLMISC::CMatrix						_TextureMatrix;
	CCustomMatrix						_CustomUVMatrix;
	float								_WorldToUVMatrix[4][4];
	NL3D::CShadowMap					*_ShadowMap;
	std::vector<NL3D::CShadowPolyReceiver::CRGBAVertex>		_TriCache;
	NLMISC::CVector						_LastCamPos;
	static NLMISC::CAABBox				_ClipBBox;
	static NL3D::CVertexBuffer			_VB;
	static bool							_VBInitialized;
	NLMISC::CRGBA						_Diffuse;
	NLMISC::CRGBA						_Emissive;
	NLMISC::CVector						_RefPosition; // position for model matrix, computed near camera pos to avoid z-fight
													  // (big translation in the final MVP matrix leads to z precision problems)
	bool								_Touched;
	bool								_ClipDownFacing;
	//
	float								_BottomBlendZMin;
	float								_BottomBlendZMax;
	float								_TopBlendZMin;
	float								_TopBlendZMax;
private:
	friend class CDecalRenderList;
	void render(NL3D::UDriver &drv,
				NL3D::CShadowPolyReceiver &receiver,
				const std::vector<NLMISC::CPlane> &worldPyramid,
				const std::vector<NLMISC::CVector> &pyramidCorners,
				bool useVertexProgram
			   );
	void renderTriCache(NL3D::IDriver &drv,   NL3D::CShadowPolyReceiver &receiver, bool useVertexProgram);
	bool clipFront(const NLMISC::CPlane &p) const;
	void setupMaterialColor();
};

// list of all decals to be rendered after the landscape
class CDecalRenderList : public NLMISC::CSingleton<CDecalRenderList>
{
public:
	CDecalRenderList() : _Empty(true) {}
	void renderAllDecals();
	void clearRenderList();
private:
	friend class CDecal;
	std::vector<CDecal::TRefPtr>	_RenderList[DECAL_NUM_PRIORITIES];
	bool							_Empty;
	std::vector<NLMISC::CPlane>		_WorldCamPyramid;
	std::vector<NLMISC::CVector>	_WorldCamPyramidCorners;
	//	scale / bias for color attenuation
	float							_DistScale;
	float							_DistBias;
};

#endif
