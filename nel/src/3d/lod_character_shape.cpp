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

#include "nel/3d/lod_character_shape.h"
#include "nel/misc/vectord.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/lod_character_texture.h"
#include "nel/misc/triangle.h"
#include "nel/misc/polygon.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// CLodCharacterShape
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const CLodCharacterShapeBuild::CPixelInfo		CLodCharacterShapeBuild::CPixelInfo::EmptyPixel(
		CVector::Null, CVector(1000,0,0));


// ***************************************************************************
CLodCharacterShapeBuild::CLodCharacterShapeBuild()
{
	_Width= 0;
	_Height= 0;
}


// ***************************************************************************
void	CLodCharacterShapeBuild::compile(const std::vector<bool> &triangleSelection, uint textureOverSample)
{
	nlassert(UVs.size()==Vertices.size());
	nlassert(Normals.size()==Vertices.size());

	// take the sqrtf.
	textureOverSample= (uint)sqrtf((float)textureOverSample);
	textureOverSample= max(textureOverSample, 1U);

	_Width= NL3D_CLOD_TEXT_WIDTH;
	_Height= NL3D_CLOD_TEXT_HEIGHT;

	uint	wOver= _Width*textureOverSample;
	uint	hOver= _Height*textureOverSample;
	std::vector<CPixelInfo>		overTextureInfo;

	// do some tri selection?
	bool	triSelectOk= triangleSelection.size()==TriangleIndices.size()/3;


	// **** reset the texture and init with "empty" pixel (ie normal.x==1000, which is not possible because normal.norm()==1)
	_TextureInfo.resize(_Width*_Height);
	fill(_TextureInfo.begin(), _TextureInfo.end(), CPixelInfo::EmptyPixel);
	// do it to the oversampled texture
	overTextureInfo.resize(wOver*hOver, CPixelInfo::EmptyPixel);


	// **** For each triangle in the shape, polyfill this triangle from a texture view, in the overSampledTexture
	CPolygon2D	poly;
	poly.Vertices.resize(3);
	for(uint i=0; i<TriangleIndices.size();i+=3)
	{
		// if selection OK, and the tri is not selected, skip
		if(triSelectOk && !triangleSelection[i/3])
			continue;

		// Setup triangle.
		// -----------
		uint	idx[3];
		idx[0]= TriangleIndices[i+0];
		idx[1]= TriangleIndices[i+1];
		idx[2]= TriangleIndices[i+2];
		// compute corners UVs, in texel size
		CTriangle		triangleUV;
		triangleUV.V0= CVector(UVs[idx[0]].U*wOver, UVs[idx[0]].V*hOver, 0);
		triangleUV.V1= CVector(UVs[idx[1]].U*wOver, UVs[idx[1]].V*hOver, 0);
		triangleUV.V2= CVector(UVs[idx[2]].U*wOver, UVs[idx[2]].V*hOver, 0);
		// compute corners pixelInfos
		CPixelInfo		pInf[3];
		for(uint corner=0; corner<3; corner++)
		{
			pInf[corner].Pos= Vertices[idx[corner]];
			pInf[corner].Normal= Normals[idx[corner]];
		}
		// Compute Gradients
		CVector		GradPosX, GradPosY, GradPosZ;
		CVector		GradNormalX, GradNormalY, GradNormalZ;
		triangleUV.computeGradient(pInf[0].Pos.x, pInf[1].Pos.x, pInf[2].Pos.x, GradPosX);
		triangleUV.computeGradient(pInf[0].Pos.y, pInf[1].Pos.y, pInf[2].Pos.y, GradPosY);
		triangleUV.computeGradient(pInf[0].Pos.z, pInf[1].Pos.z, pInf[2].Pos.z, GradPosZ);
		triangleUV.computeGradient(pInf[0].Normal.x, pInf[1].Normal.x, pInf[2].Normal.x, GradNormalX);
		triangleUV.computeGradient(pInf[0].Normal.y, pInf[1].Normal.y, pInf[2].Normal.y, GradNormalY);
		triangleUV.computeGradient(pInf[0].Normal.z, pInf[1].Normal.z, pInf[2].Normal.z, GradNormalZ);
		// Compute Gradients offset
		float		OffPosX, OffPosY, OffPosZ;
		float		OffNormalX, OffNormalY, OffNormalZ;
		OffPosX= pInf[0].Pos.x - GradPosX*triangleUV.V0;
		OffPosY= pInf[0].Pos.y - GradPosY*triangleUV.V0;
		OffPosZ= pInf[0].Pos.z - GradPosZ*triangleUV.V0;
		OffNormalX= pInf[0].Normal.x - GradNormalX*triangleUV.V0;
		OffNormalY= pInf[0].Normal.y - GradNormalY*triangleUV.V0;
		OffNormalZ= pInf[0].Normal.z - GradNormalZ*triangleUV.V0;


		// PolyFiller
		// -----------
		CVector2f	dCenter(0.5f, 0.5f);
		// select texels if their centers is in the triangle
		poly.Vertices[0]= triangleUV.V0-dCenter;
		poly.Vertices[1]= triangleUV.V1-dCenter;
		poly.Vertices[2]= triangleUV.V2-dCenter;
		// polyFiller
		CPolygon2D::TRasterVect	rasters;
		sint	minY;
		poly.computeBorders(rasters, minY);
		for(sint y=0;y<(sint)rasters.size();y++)
		{
			sint x0= rasters[y].first;
			sint x1= rasters[y].second;
			CVector	v;
			// get the center coord of this texel
			v.y= y+minY+0.5f;
			v.z= 0;
			for(sint x=x0;x<=x1;x++)
			{
				// get the center coord of this texel
				v.x= x+0.5f;
				// Compute Pos/Normal on this texel.
				CPixelInfo	texelInf;
				texelInf.Pos.x= OffPosX + GradPosX * v;
				texelInf.Pos.y= OffPosY + GradPosY * v;
				texelInf.Pos.z= OffPosZ + GradPosZ * v;
				texelInf.Normal.x= OffNormalX + GradNormalX * v;
				texelInf.Normal.y= OffNormalY + GradNormalY * v;
				texelInf.Normal.z= OffNormalZ + GradNormalZ * v;
				// normalize.
				texelInf.Normal.normalize();
				// Store (NB: overwrite any pixel)
				sint	texX= ((uint)x)%wOver;
				sint	texY= ((uint)y+minY)%hOver;
				overTextureInfo[texY*wOver+texX]= texelInf;
			}
		}

	}


	// **** Down sample the overSampled texture.
	sint	x,y;
	if(textureOverSample==1)
	{
		_TextureInfo= overTextureInfo;
	}
	else
	{
		// eg: 3 gives 1. delta ranges from -1 to 1.
		float	middle= (textureOverSample-1)/2.f;

		// for all pixels.
		for(y=0;y<(sint)_Height;y++)
		{
			for(x=0;x<(sint)_Width;x++)
			{
				// for all samples, take the best one.
				sint		xo, yo;
				float		bestDist= FLT_MAX;
				CPixelInfo	bestPixel= CPixelInfo::EmptyPixel;
				for(yo=0;yo<(sint)textureOverSample;yo++)
				{
					for(xo=0;xo<(sint)textureOverSample;xo++)
					{
						// compute distance to the pixel center.
						float	dist= sqr(yo-middle)+sqr(xo-middle);
						const CPixelInfo	&overPixel= overTextureInfo[(y*textureOverSample+yo)*wOver+(x*textureOverSample+xo)];
						// if the overPixel is not empty.
						if( !(overPixel==CPixelInfo::EmptyPixel) )
						{
							// take it if the best: nearest to the pixel center.
							if(dist<bestDist)
							{
								bestDist= dist;
								bestPixel= overPixel;
							}
						}
					}
				}

				// fill textureInfo
				_TextureInfo[y*_Width+x]= bestPixel;
			}
		}
	}


	// **** Dilate texture 1 time: each empty pixel info get neighbor full pixel (8box)
	// copy the non dilated texture
	std::vector<CPixelInfo>		tmpTextureInfo= _TextureInfo;
	// process all pixels
	for(y=0;y<(sint)_Height;y++)
	{
		sint	y0=y-1, y1=y+1;
		y0= max(y0, 0);
		y1= min(y1, (sint)_Height-1);
		for(x=0;x<(sint)_Width;x++)
		{
			CPixelInfo	&texelInf= _TextureInfo[y*_Width+x];
			// if an empty pixel.
			if(texelInf==CPixelInfo::EmptyPixel)
			{
				// dilate: look around for non empty pixel.
				sint	x0=x-1, x1=x+1;
				x0= max(x0, 0);
				x1= min(x1, (sint)_Width-1);
				// For the 8 possible pixels (nb: look us too, but doesn't matter since we are an empty pixel)
				for(sint yb= y0; yb<=y1;yb++)
				{
					for(sint xb= x0; xb<=x1;xb++)
					{
						// if the neighbor is not an empty pixel. NB: avoid override problems using not Dilated texture
						CPixelInfo	&nbTexelInf= tmpTextureInfo[yb*_Width+xb];
						if( !(nbTexelInf==CPixelInfo::EmptyPixel) )
						{
							// write it in the center pixel, and skip the search
							texelInf= nbTexelInf;
							yb= y1+1;
							break;
						}
					}
				}
			}
		}
	}

}


// ***************************************************************************
void	CLodCharacterShapeBuild::serial(NLMISC::IStream &f)
{
	// NEL_CLODBULD
	f.serialCheck(NELID("_LEN"));
	f.serialCheck(NELID("DOLC"));
	f.serialCheck(NELID("DLUB"));

	/*
	Version 1:
		- UVs and Normals + texture info
	*/
	sint	ver= f.serialVersion(1);

	f.serialCont(Vertices);
	f.serialCont(SkinWeights);
	f.serialCont(BonesNames);
	#ifdef NL_LOD_CHARACTER_INDEX16
		// must serial 16 bits index as 32 bits
		if (f.isReading())
		{
			std::vector<uint32> readVect;
			f.serialCont(readVect);
			TriangleIndices.resize(readVect.size());
			for(uint k = 0; k < readVect.size(); ++k)
			{
				nlassert(readVect[k] <= 0xffff);
				TriangleIndices[k] = (uint16) readVect[k];
			}
		}
		else
		{
			std::vector<uint32> saveVect(TriangleIndices.size());
			std::copy(TriangleIndices.begin(), TriangleIndices.end(), saveVect.begin()); //  copy will do the job
			f.serialCont(saveVect);
		}
	#else
		f.serialCont(TriangleIndices);
	#endif
	if(ver>=1)
	{
		f.serialCont(UVs);
		f.serialCont(Normals);
		f.serial(_Width, _Height);
		f.serialCont(_TextureInfo);
	}
	else
	{
		// Must init dummy UVs/normals
		UVs.resize(Vertices.size(), CUV(0,0));
		Normals.resize(Vertices.size(), CVector::K);
		// Must init dummy texture
		_Width= NL3D_CLOD_TEXT_WIDTH;
		_Height= NL3D_CLOD_TEXT_HEIGHT;
		_TextureInfo.resize(_Width*_Height, CPixelInfo::EmptyPixel);
	}

}


// ***************************************************************************
const CLodCharacterShapeBuild::CPixelInfo	*CLodCharacterShapeBuild::getTextureInfoPtr()
{
	if(_TextureInfo.empty())
		return NULL;
	else
		return &_TextureInfo[0];
}


// ***************************************************************************
// ***************************************************************************
// CLodCharacterShape
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLodCharacterShape::CLodCharacterShape()
{
	_NumVertices= 0;
	_NumTriangles= 0;
}

// ***************************************************************************
void			CLodCharacterShape::buildMesh(const std::string &name, const CLodCharacterShapeBuild &lodBuild)
{
	uint	numVertices= (uint)lodBuild.Vertices.size();
	const vector<uint32>				&triangleIndices= lodBuild.TriangleIndices;
	const vector<CMesh::CSkinWeight>	&skinWeights= lodBuild.SkinWeights;
	const vector<CUV>					&uvs= lodBuild.UVs;
	const vector<CVector>				&normals= lodBuild.Normals;

	nlassert(numVertices>0);
	nlassert(!triangleIndices.empty());
	nlassert((triangleIndices.size()%3)==0);
	nlassert(skinWeights.size() == numVertices);
	nlassert(uvs.size() == numVertices);
	nlassert(normals.size() == numVertices);

	// reset data
	contReset(_Anims);
	contReset(_AnimMap);
	contReset(_Bones);
	contReset(_BoneMap);
	contReset(_TriangleIndices);
	contReset(_UVs);
	contReset(_Normals);

	// Copy data.
	_Name= name;
	_NumVertices= numVertices;
	_NumTriangles= (uint32)triangleIndices.size()/3;
	#ifdef NL_LOD_CHARACTER_INDEX16
		_TriangleIndices.resize(triangleIndices.size());
		for(uint k = 0; k < triangleIndices.size(); ++k)
		{
			nlassert(triangleIndices[k] <= 0xffff);
			_TriangleIndices[k] = (uint16) triangleIndices[k];
		}
	#else
		_TriangleIndices= triangleIndices;
	#endif
	_UVs= uvs;
	_Normals= normals;

	// check indices.
	uint	i;
	for(i=0;i<triangleIndices.size();i++)
	{
		nlassert(triangleIndices[i]<_NumVertices);
	}

	// Copy bone names, and compute bone Map
	_Bones.resize(lodBuild.BonesNames.size());
	for(i=0; i<_Bones.size(); i++)
	{
		_Bones[i].Name= lodBuild.BonesNames[i];
		_BoneMap.insert( make_pair(_Bones[i].Name, i) );
	}

	// "Normalize" SkinWeights for CLodCharacterShape
	for(i=0;i<skinWeights.size();i++)
	{
		nlassert(skinWeights[i].Weights[0]>0);
		// for all slots not 0
		for(uint j=0;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			// if this this slot is used.
			if(skinWeights[i].Weights[j]>0)
			{
				uint boneId= skinWeights[i].MatrixId[j];
				nlassert(boneId < _Bones.size());
				// init the vInf data
				CVertexInf	vInf;
				vInf.VertexId= i;
				vInf.Influence= skinWeights[i].Weights[j];
				// Insert this vertex influence in the bone.
				_Bones[boneId].InfVertices.push_back(vInf);
			}
			else
				// stop for this vertex.
				break;
		}
	}
}

// ***************************************************************************
bool			CLodCharacterShape::addAnim(const CAnimBuild &animBuild)
{
	// first, verify don't exist.
	if(getAnimIdByName(animBuild.Name)!=-1)
		return false;

	// build basics of the animation
	CAnim	dstAnim;
	dstAnim.Name= animBuild.Name;
	dstAnim.AnimLength= animBuild.AnimLength;
	// Possible to have an Anim with just one key. setup an epsilon for animLength if 0.
	if(dstAnim.AnimLength<=0)
		dstAnim.AnimLength= 0.001f;
	dstAnim.OOAnimLength= 1.0f / animBuild.AnimLength;
	dstAnim.NumKeys= animBuild.NumKeys;
	// verify size of the array
	nlassert(dstAnim.NumKeys>0);
	nlassert(dstAnim.NumKeys * _NumVertices == animBuild.Keys.size());
	// resize dest array
	dstAnim.Keys.resize(animBuild.Keys.size());


	// Pack animation. 1st pass: compute max size over the animation vertices
	uint	i;
	// minimum shape size is , say, 1 cm :)
	CVector		maxSize(0.01f, 0.01f, 0.01f);
	for(i=0;i<animBuild.Keys.size();i++)
	{
		// take the maxSize of the abs values
		maxSize.maxof(maxSize, -animBuild.Keys[i]);
		maxSize.maxof(maxSize, animBuild.Keys[i]);
	}

	// compute the UnPackScaleFactor ie maxSize, to be multiplied by max Abs value of a sint16
	dstAnim.UnPackScaleFactor= maxSize * (1.0f/32767);

	// Pack animation. 2st pass: pack.
	CVectorD		packScaleFactor;
	packScaleFactor.x= 1.0 / dstAnim.UnPackScaleFactor.x;
	packScaleFactor.y= 1.0 / dstAnim.UnPackScaleFactor.y;
	packScaleFactor.z= 1.0 / dstAnim.UnPackScaleFactor.z;
	// For all key vertices
	for(i=0;i<animBuild.Keys.size();i++)
	{
		CVector		v= animBuild.Keys[i];
		CVector3s	&dstV= dstAnim.Keys[i];

		// compress
		v.x= float(v.x*packScaleFactor.x);
		v.y= float(v.y*packScaleFactor.y);
		v.z= float(v.z*packScaleFactor.z);
		// clamp to sint16 limits (for float precision problems).
		clamp(v.x, -32767, 32767);
		clamp(v.y, -32767, 32767);
		clamp(v.z, -32767, 32767);
		// get into the vector3s
		dstV.x= (sint16)floor(v.x);
		dstV.y= (sint16)floor(v.y);
		dstV.z= (sint16)floor(v.z);
	}


	// Add the anim to the array, and add an entry to the map
	_Anims.push_back(dstAnim);
	_AnimMap.insert(make_pair(dstAnim.Name, (uint32)_Anims.size()-1));

	return true;
}

// ***************************************************************************
void			CLodCharacterShape::CAnim::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(Name);
	f.serial(NumKeys);
	f.serial(AnimLength);
	f.serial(OOAnimLength);
	f.serial(UnPackScaleFactor);
	f.serialCont(Keys);
}


// ***************************************************************************
void			CLodCharacterShape::CBoneInfluence::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(Name);
	f.serialCont(InfVertices);
}


// ***************************************************************************
void			CLodCharacterShape::serial(NLMISC::IStream &f)
{
	// NEL_CLODSHAP
	f.serialCheck(NELID("_LEN"));
	f.serialCheck(NELID("DOLC"));
	f.serialCheck(NELID("PAHS"));

	/*
	Version 1:
		- UVs and Normals.
	*/
	sint ver= f.serialVersion(1);

	f.serial(_Name);
	f.serial(_NumVertices);
	f.serial(_NumTriangles);
	f.serialCont(_Bones);
	f.serialCont(_BoneMap);
	// nb : indices are always saved in 32 bits for compatibility
	#ifndef NL_LOD_CHARACTER_INDEX16
		f.serialCont(_TriangleIndices);
	#else
		if (f.isReading())
		{
			std::vector<uint32> savedIndices;
			f.serialCont(savedIndices);
			_TriangleIndices.resize(savedIndices.size());
			for(uint k = 0; k < savedIndices.size(); ++k)
			{
				nlassert(savedIndices[k] <= 0xffff);
				_TriangleIndices[k] = (uint16) savedIndices[k];
			}
		}
		else
		{
			std::vector<uint32> savedIndices;
			savedIndices.resize(_TriangleIndices.size());
			for(uint k = 0; k < savedIndices.size(); ++k)
			{
				savedIndices[k] = _TriangleIndices[k];
			}
			f.serialCont(savedIndices);
		}
	#endif
	f.serialCont(_Anims);
	f.serialCont(_AnimMap);

	if(ver>=1)
	{
		f.serialCont(_UVs);
		f.serialCont(_Normals);
	}
	else
	{
		// Must init dummy UVs/normals
		_UVs.resize(_NumVertices, CUV(0,0));
		_Normals.resize(_NumVertices, CVector::K);
	}
}

// ***************************************************************************
sint			CLodCharacterShape::getAnimIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _AnimMap.find(name);
	if(it == _AnimMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
sint			CLodCharacterShape::getBoneIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _BoneMap.find(name);
	if(it == _BoneMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
const TLodCharacterIndexType	*CLodCharacterShape::getTriangleArray() const
{
	if(_NumTriangles)
		return &_TriangleIndices[0];
	else
		return NULL;
}

// ***************************************************************************
const CLodCharacterShape::CVector3s	*CLodCharacterShape::getAnimKey(uint animId, TGlobalAnimationTime time, bool wrapMode, CVector &unPackScaleFactor) const
{
	H_AUTO( NL3D_LodCharacterShape_getAnimKey )

	float	localTime;

	if(animId>=_Anims.size())
		return NULL;

	// get the anim.
	const CAnim &anim= _Anims[animId];

	// scale info
	unPackScaleFactor= anim.UnPackScaleFactor;

	// Loop mgt.
	if(wrapMode)
		localTime= (float)fmod((float)time, (float)anim.AnimLength);
	else
		localTime= (float)time;

	// Clamp to the range.
	clamp(localTime, 0, anim.AnimLength);

	// get the key.
	sint	keyId= (sint)floor( (localTime*anim.OOAnimLength) * anim.NumKeys );
	clamp(keyId, 0, sint(anim.NumKeys-1));

	// return the key.
	return &anim.Keys[keyId * _NumVertices];
}

// ***************************************************************************
const CUV		*CLodCharacterShape::getUVs() const
{
	if(_NumVertices==0)
		return NULL;

	return &_UVs[0];
}

// ***************************************************************************
const CVector	*CLodCharacterShape::getNormals() const
{
	if(_NumVertices==0)
		return NULL;

	return &_Normals[0];
}



// ***************************************************************************
// ***************************************************************************
// Bone Alpha Testing
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLodCharacterShape::startBoneAlpha(std::vector<uint8>	&tmpAlphas) const
{
	// clear
	tmpAlphas.clear();
	// alocate, and fill
	tmpAlphas.resize(getNumVertices(), 0);
}

// ***************************************************************************
void			CLodCharacterShape::addBoneAlpha(uint boneId, std::vector<uint8> &tmpAlphas) const
{
	// Yoyo: This is an error to not have the same skeleton that the one stored in the lod shape. But must not crash
	if(boneId>=_Bones.size())
		return;
	const CBoneInfluence	&bone= _Bones[boneId];

	// for all vertices influenced by this bone, must set the alpha to full
	for(uint i=0; i<bone.InfVertices.size(); i++)
	{
		const CVertexInf	&vInf= bone.InfVertices[i];
		tmpAlphas[vInf.VertexId]= 255;
	}
}


} // NL3D
