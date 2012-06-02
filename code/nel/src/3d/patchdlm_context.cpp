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

#include "nel/3d/patchdlm_context.h"
#include "nel/3d/patch.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/point_light.h"
#include "nel/3d/texture_dlm.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/tile_far_bank.h"
#include "nel/3d/landscape.h"
#include "nel/misc/system_info.h"
#include "nel/misc/fast_mem.h"


using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatchDLMPointLight::compile(const CPointLight &pl, NLMISC::CRGBA landDiffMat, float maxAttEnd)
{
	nlassert(maxAttEnd>0);

	// copy color
	R= (float) (( pl.getDiffuse().R*(landDiffMat.R+1) ) >>8);
	G= (float) (( pl.getDiffuse().G*(landDiffMat.G+1) ) >>8);
	B= (float) (( pl.getDiffuse().B*(landDiffMat.B+1) ) >>8);
	// Copy Spot/Pos/Dir.
	IsSpot= pl.getType() == CPointLight::SpotLight;
	Pos= pl.getPosition();
	Dir= pl.getSpotDirection();

	// compute spot params
	if(IsSpot)
	{
		CosMax= cosf(pl.getSpotAngleBegin());
		CosMin= cosf(pl.getSpotAngleEnd());
	}
	else
	{
		// with tesse Values, we have always (cosSpot-CosMin) * OOCosDelta > 1.0f
		CosMax= -1;
		CosMin= -2;
	}
	OOCosDelta= 1.f / (CosMax-CosMin);

	// compute att params
	AttMax= pl.getAttenuationEnd();
	AttMin= pl.getAttenuationBegin();
	// infinite pointLight?
	if(AttMax==0)
	{
		AttMax= maxAttEnd;
		AttMin= maxAttEnd*0.99f;
	}
	// To big pointLigt?
	else if(AttMax>maxAttEnd)
	{
		AttMax= maxAttEnd;
		AttMin= min(AttMin, maxAttEnd*0.99f);
	}
	// compile distance
	OOAttDelta= 1.f / (AttMin-AttMax);


	// Compute bounding sphere.
	// If not a spot or if angleMin>Pi/2
	if(!IsSpot || CosMin<0)
	{
		// Take sphere of pointlight sphere
		BSphere.Center= Pos;
		BSphere.Radius= AttMax;
		// The bbox englobe the sphere.
		BBox.setCenter(Pos);
		BBox.setHalfSize(CVector(AttMax, AttMax, AttMax));
	}
	else
	{
		// Compute BSphere.
		//==============

		// compute sinus of AngleMin
		float	sinMin= sqrtf(1-sqr(CosMin));

		// Test 2 centers: Center of radius along Dir: Pos+Dir*AttMax/2, and intersection of end cone with line (Pos,Dir)
		// Don't know why but I think they are sufficiently good :)
		// See below for computing of those centers.

		/* compute radius of each sphere by taking max of 3 distances: distance to spotLight center, distance
			to spotLight forward extremity, and distance to spotLight circle interstion Cone/Sphere. (named DCCS)
			NB: Do the compute with radius=1 at first, then multiply later.
		*/
		float	radius1= 0.5f;		// =max(0.5, 0.5); max distance to spot center and extremity center :)
		// for distance DCCS, this is the hypothenuse of (cosMin-0.5) + sinMin.
		float	dccs= sqrtf( sqr(CosMin-0.5f) + sqr(sinMin));
		// take the bigger.
		radius1= max(radius1, dccs );

		// Same reasoning for center2.
		float	radius2= max(CosMin, 1-CosMin);	// max distance to spot center and extremity center :)
		// for distance DCCS, it is simply sinMin!!
		dccs= sinMin;
		// take the bigger.
		radius2= max(radius2, dccs );


		// Then take the center which gives the smaller sphere
		if(radius1<radius2)
		{
			BSphere.Center= Pos + (Dir*0.5f*AttMax);
			// radius1 E [0,1], must take real size.
			BSphere.Radius= radius1 * AttMax;
		}
		else
		{
			BSphere.Center= Pos + (Dir*CosMin*AttMax);
			// radius2 E [0,1], must take real size.
			BSphere.Radius= radius2 * AttMax;
		}


		// Compute BBox.
		//==============

		// just take bbox of the sphere, even if not optimal.
		BBox.setCenter(BSphere.Center);
		float	rad= BSphere.Radius;
		BBox.setHalfSize( CVector(rad, rad, rad) );
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CPatchDLMContext::CPatchDLMContext()
{
	_Patch= NULL;
	_DLMTexture= NULL;
	_DLMContextList= NULL;
	OldPointLightCount= 0;
	CurPointLightCount= 0;
	// By default there is crash in textures
	_IsSrcTextureFullBlack= false;
	_IsDstTextureFullBlack= false;
}


// ***************************************************************************
CPatchDLMContext::~CPatchDLMContext()
{
	// release the lightmap in the texture
	if(_DLMTexture)
	{
		_DLMTexture->releaseLightMap(TextPosX, TextPosY);
	}
	// exit
	_Patch= NULL;
	_DLMTexture= NULL;

	// remove it from list.
	if(_DLMContextList)
		_DLMContextList->remove(this);
}


// ***************************************************************************
#ifdef NL_DLM_TILE_RES
// if tileRes defined, still start to clip at tessBlockLevel.
#define	NL_DLM_CLIP_FACTOR		2
#else
// start to clip at tessBlockLevel (same as dlm map precision)
#define	NL_DLM_CLIP_FACTOR		1
#endif

#define	NL_DLM_CLIP_NUM_LEVEL	3

// ***************************************************************************
bool			CPatchDLMContext::generate(CPatch *patch, CTextureDLM *textureDLM, CPatchDLMContextList *ctxList)
{
	nlassert(patch);
	nlassert(textureDLM);
	nlassert(ctxList);

	// keep info on patch/landscape.
	_Patch= patch;
	_DLMTexture= textureDLM;
	// append to the list.
	_DLMContextList= ctxList;
	_DLMContextList->append(this);

	// Get Texture Size info;
#ifdef NL_DLM_TILE_RES
	// get coord at cornes of tiles
	Width= (_Patch->getOrderS())+1;
	Height= (_Patch->getOrderT())+1;
#else
	// get coord at cornes of tessBlocks
	Width= (_Patch->getOrderS()/2)+1;
	Height= (_Patch->getOrderT()/2)+1;
#endif

	// Allocate space in texture
	if(!_DLMTexture->createLightMap(Width, Height, TextPosX, TextPosY))
	{
		// Mark as not allocated.
		// NB: the context still work with NULL _DLMTexture, but do nothing (excpetionnal case)
		_DLMTexture= NULL;
	}

	// If the lightmap is correclty allocated in the global texture, compute UVBias.
	if(_DLMTexture)
	{
		// Compute patch UV matrix from pixels. Must map to center of pixels.
		DLMUScale= (float)(Width-1) / (float)_DLMTexture->getWidth();
		DLMVScale= (float)(Height-1) / (float)_DLMTexture->getHeight();
		DLMUBias= ((float)TextPosX+0.5f) / (float)_DLMTexture->getWidth();
		DLMVBias= ((float)TextPosY+0.5f) / (float)_DLMTexture->getHeight();
	}
	else
	{
		// Build UVBias such that the UVs point to Black
		// NB: TextureDLM ensure that point (MaxX,MaxY) of texture is black.
		DLMUScale= 0;
		DLMVScale= 0;
		DLMUBias= 1;
		DLMVBias= 1;
	}

	// TestYoyo: to see lightmap usage in the big texture
	/*DLMUScale= _Patch->getOrderS();
	DLMVScale= _Patch->getOrderT();
	DLMUBias= 0;
	DLMVBias= 0;*/


	// Bound 8bits UV for Vegetable. This is to ensure vegetable Dlm UVs won't peek in neighbor lightmaps.
	sint	tmpU, tmpV;
	// Bound U minimum
	tmpU= (sint)ceil ( (DLMUBias) * 255 );
	clamp(tmpU, 0, 255);
	MinU8= tmpU;
	// Bound U maximum
	tmpU= (sint)floor( (DLMUBias+DLMUScale) * 255 );
	clamp(tmpU, (sint)MinU8, 255);
	MaxU8= tmpU;
	// Bound V minimum
	tmpV= (sint)ceil ( (DLMVBias) * 255 );
	clamp(tmpV, 0, 255);
	MinV8= tmpV;
	// Bound V maximum
	tmpV= (sint)floor( (DLMVBias+DLMVScale) * 255 );
	clamp(tmpV, (sint)MinV8, 255);
	MaxV8= tmpV;


	// Allocate RAM Lightmap
	_LightMap.resize(Width*Height);

	// generate Vertices: pos and normals
	_Vertices.resize(Width*Height);
	float	s, t;
	float	ds= 1.0f / (Width-1);
	float	dt= 1.0f / (Height-1);
	// eval all the patch.
	t= 0;
	uint	x,y;
	for(y=0; y<Height; y++, t+=dt)
	{
		s= 0;
		for(x=0; x<Width; x++, s+=ds)
		{
			CVertex	&vert= _Vertices[y*Width+x];
			// NB: use the bezier patch, and don't take Noise into account, for speed reason.
			CBezierPatch	*bpatch= _Patch->unpackIntoCache();
			// Eval pos.
			vert.Pos= bpatch->eval(s, t);
			// Eval Normal.
			vert.Normal= bpatch->evalNormal(s, t);
		}
	}

	// Build bounding Spheres QuadTree
	//============

	// Size of the cluster array (at level 0)
	uint	bsx, bsy;
#ifdef NL_DLM_TILE_RES
	// level 0 is at tile level.
	bsx= max(1, (_Patch->getOrderS())/NL_DLM_CLIP_FACTOR );
	bsy= max(1, (_Patch->getOrderT())/NL_DLM_CLIP_FACTOR );
#else
	// level 0 is at tessBlock level.
	bsx= max(1, (_Patch->getOrderS()/2)/NL_DLM_CLIP_FACTOR );
	bsy= max(1, (_Patch->getOrderT()/2)/NL_DLM_CLIP_FACTOR );
#endif

	// resize bboxes for level 0.
	static	vector<CAABBox>		tmpBBoxes[NL_DLM_CLIP_NUM_LEVEL];
	tmpBBoxes[0].resize(bsx * bsy);

	// Extend all leaves clusters BBoxes with patch coordinates
	for(y=0;y<bsy;y++)
	{
		// For Y, compute how many patch Positions used to extend bbox.
		uint	beginY= y*NL_DLM_CLIP_FACTOR;
		uint	endY= min( (y+1)*NL_DLM_CLIP_FACTOR+1, Height);
		for(x=0;x<bsx;x++)
		{
			// For X, compute how many patch Positions used to extend bbox.
			uint	beginX= x*NL_DLM_CLIP_FACTOR;
			uint	endX= min((x+1)*NL_DLM_CLIP_FACTOR+1, Width);
			// Build a bbox.
			CAABBox		bbox;
			bbox.setCenter(_Vertices[beginY*Width + beginX].Pos);
			for(uint yi= beginY; yi<endY; yi++)
			{
				for(uint xi= beginX; xi<endX; xi++)
				{
					bbox.extend(_Vertices[yi*Width + xi].Pos);
				}
			}
			// Set the BBox info.
			tmpBBoxes[0][y*bsx + x]= bbox;
		}
	}

	// build parent BSpheres for quadTree hierarchy
	uint	curLevel= 0;
	uint	nextLevel= 1;
	uint	nextBsx= max(1U, bsx/2);
	uint	nextBsy= max(1U, bsy/2);
	// the number of cluster Sons, and descendants this cluster level owns.
	uint	tmpClusterNumToSkip[NL_DLM_CLIP_NUM_LEVEL];
	// width for this cluster level.
	uint	tmpClusterWidth[NL_DLM_CLIP_NUM_LEVEL];
	// Number of sons per line/column
	uint	tmpClusterWSon[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterHSon[NL_DLM_CLIP_NUM_LEVEL];
	// Fill level 0 info
	tmpClusterNumToSkip[0]= 0;
	tmpClusterWidth[0]= bsx;
	tmpClusterWSon[0]= 0;
	tmpClusterHSon[0]= 0;
	uint	finalClusterSize= bsx * bsy;

	// If the next level has 1x1 cases, it is not useful (since same sphere as entire Patch)
	while(nextBsx * nextBsy > 1 && nextLevel<NL_DLM_CLIP_NUM_LEVEL )
	{
		finalClusterSize+= nextBsx * nextBsy;

		uint	wSon= (bsx/nextBsx);
		uint	hSon= (bsy/nextBsy);
		// compute cluster level info.
		tmpClusterWidth[nextLevel]= nextBsx;
		tmpClusterWSon[nextLevel]= wSon;
		tmpClusterHSon[nextLevel]= hSon;
		// NB: level 0 has 0 sons to skip, hence level1 must skip (1+0)*4= 4  (wSon==hSon==2)
		// level2 must skip (1+4)*4= 20    (wSon==hSon==2)
		tmpClusterNumToSkip[nextLevel]= (1+tmpClusterNumToSkip[curLevel]) * wSon * hSon;

		// alloc bboxes.
		tmpBBoxes[nextLevel].resize(nextBsx * nextBsy);

		// For all cluster of upper level, build bb, as union of finers clusters
		for(y=0;y<nextBsy;y++)
		{
			for(x=0;x<nextBsx;x++)
			{
				// compute coordinate in curLevel tmpBBoxes to look
				uint	x2= x*wSon;
				uint	y2= y*hSon;
				// Build a bbox for 4 (or 2) children clusters
				if(wSon>1 && hSon>1)
				{
					CAABBox		bbox1;
					CAABBox		bbox2;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][y2*bsx + x2+1]);
					bbox2= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][(y2+1)*bsx + x2], tmpBBoxes[curLevel][(y2+1)*bsx + x2+1]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= CAABBox::computeAABBoxUnion(bbox1, bbox2);
				}
				else if(wSon==1)
				{
					CAABBox		bbox1;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][(y2+1)*bsx + x2]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= bbox1;
				}
				else if(hSon==1)
				{
					CAABBox		bbox1;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][y2*bsx + x2+1]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= bbox1;
				}
				else
					// impossible...
					nlstop;
			}
		}

		// upper level.
		bsx= nextBsx;
		bsy= nextBsy;
		nextBsx= max(1U, nextBsx/2);
		nextBsy= max(1U, nextBsy/2);
		curLevel++;
		nextLevel++;
	}


	// Resize clusters with size according to all levels
	_Clusters.resize(finalClusterSize);
	uint	iDstCluster= 0;

	// Fill cluster hierarchy, in _Clusters.
	uint	numLevels= nextLevel;
	// NB: the principle is recursive, but it is "iterated", with a stack-like: tmpClusterX and tmpClusterY;
	uint	tmpClusterX[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterY[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterXMin[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterYMin[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterXMax[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterYMax[NL_DLM_CLIP_NUM_LEVEL];
	// we start at curLevel (the highest Level), and we must fill all the squares of this level
	tmpClusterX[curLevel]= 0;
	tmpClusterY[curLevel]= 0;
	tmpClusterXMin[curLevel]= 0;
	tmpClusterYMin[curLevel]= 0;
	tmpClusterXMax[curLevel]= bsx;
	tmpClusterYMax[curLevel]= bsy;
	// while the "root" level is not pop
	while(curLevel < numLevels)
	{
		// If we ended with this level (all lines done).
		if(tmpClusterY[curLevel] >= tmpClusterYMax[curLevel])
		{
			// Ok, finished with this level, pop up.
			curLevel++;
			// skip.
			continue;
		}

		nlassert(iDstCluster<_Clusters.size());

		// get the bbox from current position.
		CAABBox	bbox= tmpBBoxes[curLevel][ tmpClusterY[curLevel] * tmpClusterWidth[curLevel] + tmpClusterX[curLevel] ];
		// Fill _Clusters for this square.
		_Clusters[iDstCluster].BSphere.Center= bbox.getCenter();
		_Clusters[iDstCluster].BSphere.Radius= bbox.getRadius();
		// If leaf level, fill special info
		if(curLevel == 0)
		{
			_Clusters[iDstCluster].NSkips= 0;
			_Clusters[iDstCluster].X= tmpClusterX[0];
			_Clusters[iDstCluster].Y= tmpClusterY[0];
		}
		// else, set total number of sons to skips if "invisible"
		else
			_Clusters[iDstCluster].NSkips= tmpClusterNumToSkip[curLevel];

		// next dst cluster
		iDstCluster ++;


		// If not Leaf level, recurs. First pass, use curLevel params (tmpClusterX...)
		if(curLevel > 0)
		{
			// compute info for next level.
			tmpClusterXMin[curLevel-1]= tmpClusterX[curLevel] * tmpClusterWSon[curLevel];
			tmpClusterYMin[curLevel-1]= tmpClusterY[curLevel] * tmpClusterHSon[curLevel];
			tmpClusterXMax[curLevel-1]= (tmpClusterX[curLevel]+1) * tmpClusterWSon[curLevel];
			tmpClusterYMax[curLevel-1]= (tmpClusterY[curLevel]+1) * tmpClusterHSon[curLevel];
			// begin iteration of child level
			tmpClusterX[curLevel-1]= tmpClusterXMin[curLevel-1];
			tmpClusterY[curLevel-1]= tmpClusterYMin[curLevel-1];
		}


		// next square for this level
		tmpClusterX[curLevel]++;
		// if ended for X.
		if(tmpClusterX[curLevel] >= tmpClusterXMax[curLevel])
		{
			// reset X.
			tmpClusterX[curLevel]= tmpClusterXMin[curLevel];
			// next line.
			tmpClusterY[curLevel]++;
		}


		// If not Leaf level, recurs. Second pass, after tmpClusterX and tmpClusterY of curLevel are changed
		if(curLevel > 0)
		{
			// descend in hierarchy. (recurs)
			curLevel--;
		}

	}

	// All dst clusters must have been filled
	nlassert(iDstCluster == _Clusters.size());


	// PreProcess Patch TileColors.
	//============
	// Verify that a CTileColor is nothing more than a 565 color.
	nlassert(sizeof(CTileColor)==sizeof(uint16));
#ifndef NL_DLM_TILE_RES

	// retrieve patch tileColor pointer.
	nlassert(_Patch->TileColors.size()>0);
	CTileColor	*tileColor= &_Patch->TileColors[0];

	// skip 1 tiles colors per column and per row
	uint		wTileColor= _Patch->getOrderS()+1;
	CTileColor	*tcOrigin= tileColor;
	// alloc _LowResTileColors at same resolution than lightmap
	_LowResTileColors.resize(Width*Height);
	uint16	*dstLRtc= &_LowResTileColors[0];

	// For all lines of dst.
	for(y=0;y<Height;y++)
	{
		// tileColor start of line.
		tileColor= tcOrigin + y*2* wTileColor;
		sint	npix= Width;
		// for all pixels at corner of tessBlock.
		for(;npix>0; npix--, tileColor+=2, dstLRtc++)
		{
			*dstLRtc= tileColor->Color565;
		}

	}
#endif


	// compute the TextureFar used for Far dynamic lightmaping.
	//============
	// NB: simpler to compute it at generate() time, even if not necessarly needed for near
	computeTextureFar();


	// fill texture with Black
	//============
	clearLighting();

	return true;
}

// ***************************************************************************
void			CPatchDLMContext::clearLighting()
{
	// If the srcTexture is not already black.
	if(!_IsSrcTextureFullBlack)
	{
		// Reset Lightmap with black.
		uint	count= _LightMap.size();
		if(count>0)
		{
			memset(&_LightMap[0], 0, count * sizeof(CRGBA));
		}

		// Now the src lightmap is fully black
		_IsSrcTextureFullBlack= true;
	}
}


// ***************************************************************************

// TestYoyo: I thought this code was better, but actually, this is not the case
/*
static	float	NL3D_Val1= 1.f;
inline void	__stdcall fastClamp01(float &x)
{
	__asm
	{
		mov esi, x
		mov eax, [esi]

		// clamp to 0.
		cmp eax, 0x80000001		// set carry if sign bit is set.
		sbb ecx, ecx			// if attDist is negative, ecx==0 , else 0xFFFFFFFF.
		and eax, ecx			// if attDist is negative, eax=0, else unchanged

		// clamp eax to 1 (NB: now we are sure eax>=0).
		cmp	eax, NL3D_Val1		// set carry if < Val1.
		sbb	ecx, ecx			// if < Val1, ecx==0xFFFFFFFF, else 0.
		and	eax, ecx			// if < Val1, ecx= eax, else ecx=0
		not	ecx
		and	ecx, NL3D_Val1		// if > Val1, ecx== Val1, else ecx= 0.
		add	eax, ecx			// finally, eax= val clamped to 1.

		// store.
		mov [esi], eax
	}
}*/

// faster to do a simple clamp ???
inline void	fastClamp01(float &x)
{
	clamp(x, 0.f, 1.f);
}


// ***************************************************************************
void			CPatchDLMContext::addPointLightInfluence(const CPatchDLMPointLight &pl)
{

    uint		nverts= _Vertices.size();
	nlassert(nverts==_LightMap.size());

	if(nverts==0)
		return;
	CVertex		*vert= &_Vertices[0];


	// precise clip: parse the quadTree of sphere
	//================
	uint	i, x,y;
	uint	startX, startY, endX, endY;
	startX= 0xFFFFFFFF;
	startY= 0xFFFFFFFF;
	endX= 0;
	endY= 0;
	for(i=0;i<_Clusters.size();)
	{
		// If the sphere intersect pl,
		if(_Clusters[i].BSphere.intersect(pl.BSphere) )
		{
			// if this cluster is a leaf, extend start/end
			if(_Clusters[i].NSkips==0)
			{
				x= _Clusters[i].X;
				y= _Clusters[i].Y;
				startX= min(startX, x);
				startY= min(startY, y);
				endX= max(endX, x+1);
				endY= max(endY, y+1);
			}
			// go to next cluster (a brother, a parent or a son)
			i++;
		}
		else
		{
			// if this cluster is a leaf, just go to next cluster (a parent or a brother)
			if(_Clusters[i].NSkips==0)
				i++;
			// else, go to next brother or parent (NSkips say how to go)
			else
				i+= _Clusters[i].NSkips;
		}
	}
	// if never intersect, just quit.
	if(startX==0xFFFFFFFF)
		return;

	// get vertices in array to process.
	startX*=NL_DLM_CLIP_FACTOR;
	startY*=NL_DLM_CLIP_FACTOR;
	endX= min(endX*NL_DLM_CLIP_FACTOR+1, Width);
	endY= min(endY*NL_DLM_CLIP_FACTOR+1, Height);

	// TestYoyo only.
	//extern uint YOYO_LandDLCount;
	//YOYO_LandDLCount+= (endX - startX) * (endY - startY);

	// process all vertices
	//================
	float	r,g,b;
	CRGBA	*dst= &_LightMap[0];
	CVertex		*originVert= vert;
	CRGBA		*originDst= dst;

	// TestYoyo: finally, precache does not seems to impact final result.
	// precache loading, for better cache use. NB: precache the entire line, ignoring clip result.
	// Precache only if interesting.
	//if( (endX - startX)*4>=Width && (endY-startY)>=2)
	//{
		//vert= originVert + startY*Width;
		//dst= originDst + startY*Width;
		//uint	nPixelLine= (endY-startY)*Width;
		//CFastMem::precacheBest(vert, nPixelLine * sizeof(CVertex));
		//CFastMem::precacheBest(dst, nPixelLine * sizeof(CRGBA));
	//}

	// Start 24 precision, for faster compute.
	OptFastFloorBegin24();

	// If the pointLight is a spot, compute is more complex/slower
	if(pl.IsSpot)
	{
		for(y=startY; y<endY; y++)
		{
			nverts= endX - startX;

			vert= originVert + startX + y*Width;
			dst= originDst + startX + y*Width;
			for(;nverts>0; nverts--, vert++, dst++)
			{
				CVector	dirToP= vert->Pos - pl.Pos;
				float	dist= dirToP.norm();
				dirToP/= dist;

				// compute cos for pl. attenuation
				float	cosSpot= dirToP * pl.Dir;
				float	attSpot= (cosSpot-pl.CosMin) * pl.OOCosDelta;
				fastClamp01(attSpot);

				// distance attenuation
				float	attDist= (dist-pl.AttMax) * pl.OOAttDelta;
				fastClamp01(attDist);

				// compute diffuse lighting
				float	diff= -(vert->Normal * dirToP);
				fastClamp01(diff);

				// compute colors.
				diff*= attSpot * attDist;
				r= pl.R*diff;
				g= pl.G*diff;
				b= pl.B*diff;

				CRGBA	col;
#ifdef NL_OS_MAC
				// OptFastFloor24 should compiles but it generates an internal compiler error
				col.R= (uint8)floor(r);
				col.G= (uint8)floor(g);
				col.B= (uint8)floor(b);
#else
				// we need to do the 0xff mask or run time type check can break here because sometime r g b are > 255
				col.R= uint8(OptFastFloor24(r) & 0xff);
				col.G= uint8(OptFastFloor24(g) & 0xff);
				col.B= uint8(OptFastFloor24(b) & 0xff);
#endif

				// add to map.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
				// Fast AddClamp.
				__asm
				{
					mov	esi, dst

					mov	al, [esi]dst.R
					add	al, col.R
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.R, al

					mov	al, [esi]dst.G
					add	al, col.G
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.G, al

					mov	al, [esi]dst.B
					add	al, col.B
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.B, al
				}
#else
				// add and clamp to map.
				dst->addRGBOnly(*dst, col);
#endif
			}
		}
	}
	// else, pointLight with no Spot cone attenuation
	else
	{
		// TestYoyo
		//extern	void	YOYO_startDLMItCount();
		//YOYO_startDLMItCount();

		// Compute lightmap pixels of interest
		for(y=startY; y<endY; y++)
		{
			nverts= endX - startX;

			vert= originVert + startX + y*Width;
			dst= originDst + startX + y*Width;
			for(;nverts>0; nverts--, vert++, dst++)
			{
				CVector	dirToP= vert->Pos - pl.Pos;
				float	dist= dirToP.norm();
				float	OODist= 1.0f / dist;
				dirToP*= OODist;

				// distance attenuation
				float	attDist= (dist-pl.AttMax) * pl.OOAttDelta;
				fastClamp01(attDist);

				// compute diffuse lighting
				float	diff= -(vert->Normal * dirToP);
				fastClamp01(diff);

				// compute colors.
				diff*= attDist;
				r= pl.R*diff;
				g= pl.G*diff;
				b= pl.B*diff;

				CRGBA	col;
#ifdef NL_OS_MAC
				// OptFastFloor24 should compiles but it generates an internal compiler error
				col.R= (uint8)floor(r);
				col.G= (uint8)floor(g);
				col.B= (uint8)floor(b);
#else
				// we need to do the 0xff mask or run time type check can break here because sometime r g b are > 255
				col.R= uint8(OptFastFloor24(r) & 0xff);
				col.G= uint8(OptFastFloor24(g) & 0xff);
				col.B= uint8(OptFastFloor24(b) & 0xff);
#endif
				// add to map.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
				// Fast AddClamp.
				__asm
				{
					mov	esi, dst

					mov	al, [esi]dst.R
					add	al, col.R
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.R, al

					mov	al, [esi]dst.G
					add	al, col.G
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.G, al

					mov	al, [esi]dst.B
					add	al, col.B
					sbb	cl, cl
					or	al, cl
					mov	[esi]dst.B, al
				}
#else
				// add and clamp to map.
				dst->addRGBOnly(*dst, col);
#endif
			}
		}

		// TestYoyo
		//extern	void	YOYO_endDLMItCount();
		//YOYO_endDLMItCount();
	}

	// Stop 24 bit precision
	OptFastFloorEnd24();

	// Src texture is modified, hence it can't be black.
	//==============
	_IsSrcTextureFullBlack= false;
}


// ***************************************************************************
void			CPatchDLMContext::compileLighting(TCompileType compType, CRGBA modulateCte)
{
	// If srcTexture is full black, and if dst texture is already full black too, don't need to update dst texture
	if(! (_IsSrcTextureFullBlack && _IsDstTextureFullBlack) )
	{
		// if lightMap allocated
		if(_LightMap.size()>0 && _DLMTexture)
		{
			// If the srcTexture is full black (ie no pointLight influence touch it),
			if(_IsSrcTextureFullBlack)
			{
				// reset the texture to full black.
				_DLMTexture->fillRect(TextPosX, TextPosY, Width, Height, 0);
			}
			// else the srcTexture is not full black (ie some pointLight influence touch it),
			else
			{
				// if must modulate with tileColor
				if(compType == ModulateTileColor)
				{
					// a vector can't have negative size
					//nlassert(_Patch->TileColors.size()>=0);
					#ifdef NL_DLM_TILE_RES
					// retrieve userColor pointer.
					uint16	*tileColor= (uint16*)(&_Patch->TileColors[0]);
					#else
					uint16	*tileColor= (uint16*)(&_LowResTileColors[0]);
					#endif

					// modulate and fill dest.
					_DLMTexture->modulateAndfillRect565(TextPosX, TextPosY, Width, Height, &_LightMap[0], tileColor);
				}
				// else if must modulate with textureFar
				else if(compType == ModulateTextureFar)
				{
					// modulate and fill dest.
					_DLMTexture->modulateAndfillRect8888(TextPosX, TextPosY, Width, Height, &_LightMap[0], &_TextureFar[0]);
				}
				// else if must modulate with constante
				else if(compType == ModulateConstant)
				{
					// modulate and fill dest.
					_DLMTexture->modulateConstantAndfillRect(TextPosX, TextPosY, Width, Height, &_LightMap[0], modulateCte);
				}
				// else, no Modulate.
				else
				{
					// just copy lightmap to texture
					_DLMTexture->copyRect(TextPosX, TextPosY, Width, Height, &_LightMap[0]);
				}
			}
		}


		// copy full black state
		_IsDstTextureFullBlack= _IsSrcTextureFullBlack;
	}
}


// ***************************************************************************
uint			CPatchDLMContext::getMemorySize() const
{
	uint	size= sizeof(CPatchDLMContext);
	size+= _Vertices.size() * sizeof(CVertex);
	size+= _LightMap.size() * sizeof(CRGBA);
	size+= _Clusters.size() * sizeof(CCluster);
	size+= _TextureFar.size() * sizeof(CRGBA);
#ifndef NL_DLM_TILE_RES
	size+= _LowResTileColors.size() * sizeof(uint16);
#endif

	return size;
}


// ***************************************************************************
void			CPatchDLMContext::computeTextureFar()
{
	// First compute Far at order1 Level (ie 2x2 pixels per tiles).
	//==================
	static	vector<CRGBA>	tileFars;
	// Get the FarBank from landscape.
	CTileFarBank	&farBank= _Patch->getLandscape()->TileFarBank;
	// size of the texture.
	uint	os= _Patch->getOrderS();
	uint	ot= _Patch->getOrderT();
	// resize tmp texture. keep a border of 1 pixel around this texture (for average with border)
	uint	tfWidth= os*2+2;
	uint	tfHeight= ot*2+2;
	uint	tfSize= tfWidth * tfHeight;
	tileFars.resize(tfSize);
	CRGBA	*dst= &tileFars[0];

	// default: fill dst with black (for possible non-existing tiles).
	memset(dst, 0, tfSize*sizeof(CRGBA));

	// For all tiles.
	uint	x, y;
	for(y=0; y<ot; y++)
	{
		for(x=0;x<os;x++)
		{
			// get the tile from patch.
			CTileElement	&tileElm= _Patch->Tiles[y*os + x];

			// For all layers
			for(uint l=0; l<3;l++)
			{
				uint16		tileId= tileElm.Tile[0];
				if (tileId!=NL_TILE_ELM_LAYER_EMPTY)
				{
					// Get the read only pointer on the far tile
					const CTileFarBank::CTileFar*	pTile= farBank.getTile (tileId);
					// if exist.
					if(pTile && pTile->isFill (CTileFarBank::diffuse))
					{
						// get tile element information.
						sint	nRot= tileElm.getTileOrient(l);
						bool	is256x256;
						uint8	uvOff;
						tileElm.getTile256Info(is256x256, uvOff);

						// compute src pixel
						const CRGBA	*srcPixel= pTile->getPixels(CTileFarBank::diffuse, CTileFarBank::order1);
						// compute src info, for this tile rot and 256x256 context.
						sint srcDeltaX = 0;
						sint srcDeltaY = 0;
						srcPixel= computeTileFarSrcDeltas(nRot, is256x256, uvOff, srcPixel, srcDeltaX, srcDeltaY);

						// compute dst coordinate. start writing at pixel (1,1)
						CRGBA	*dstPixel= dst + (y*2+1)*tfWidth + x*2+1;

						if(l==0)
						{
							// copy the tile content to the texture.
							copyTileToTexture(srcPixel, srcDeltaX, srcDeltaY, dstPixel, tfWidth);
						}
						else
						{
							// blend the tile content to the texture.
							blendTileToTexture(srcPixel, srcDeltaX, srcDeltaY, dstPixel, tfWidth);
						}
					}
					else
						// go to next tile.
						break;
				}
				else
					// go to next tile.
					break;
			}
		}
	}

	/* copy borders pixels from border of current patch
		NB: this is not correct, but visually sufficient.
		To look on neighbor would be more complex.
	*/

	// copy lines up and down.
	y= tfHeight-1;
	for(x=1;x<tfWidth-1;x++)
	{
		// copy line 0 from line 1.
		dst[0*tfWidth + x]= dst[1*tfWidth + x];
		// copy last line from last line-1.
		dst[y*tfWidth + x]= dst[(y-1)*tfWidth + x];
	}

	// copy column left and right
	x= tfWidth-1;
	for(y=1;y<tfHeight-1;y++)
	{
		// copy column 0 from column 1.
		dst[y*tfWidth + 0]= dst[y*tfWidth + 1];
		// copy last column from last column-1.
		dst[y*tfWidth + x]= dst[y*tfWidth + x-1];
	}

	// copy 4 corners
	x= tfWidth-1;
	y= tfHeight-1;
	// top-left corner
	dst[0]= dst[1];
	// top-right corner
	dst[x]= dst[x-1];
	// bottom-left corner
	dst[y*tfWidth + 0]= dst[y*tfWidth + 1];
	// bottom-right corner
	dst[y*tfWidth + x]= dst[y*tfWidth + x-1];


	// Average to DLM resolution (ie OrderS+1, OrderT+1)
	//==================
	// resize _TextureFar.
	_TextureFar.resize(Width*Height);
	CRGBA	*src= &tileFars[0];
	dst= &_TextureFar[0];

	// for all pixels of dst texture.
	for(y=0;y<Height;y++)
	{
		for(x=0;x<Width;x++, dst++)
		{
			// compute coordinate in tileFars.
			uint	x2, y2;
#ifdef	NL_DLM_TILE_RES
			x2= x * 2;
			y2= y * 2;
#else
			// easiest method: sample every 2 tiles.
			x2= x * 4;
			y2= y * 4;
#endif

			// Average the 4 pixels around this tile corner
			dst->avg4RGBOnly(src[y2*tfWidth + x2],
				src[y2*tfWidth + x2+1],
				src[(y2+1)*tfWidth + x2],
				src[(y2+1)*tfWidth + x2+1]);
		}
	}


	// Modulate result with TileColors.
	//==================
	// vector-size is always >= 0
	//nlassert(_Patch->TileColors.size()>=0);
	#ifdef NL_DLM_TILE_RES
	// retrieve userColor pointer.
	uint16	*tileColor= (uint16*)(&_Patch->TileColors[0]);
	#else
	uint16	*tileColor= (uint16*)(&_LowResTileColors[0]);
	#endif

	// For all pixels
	dst= &_TextureFar[0];
	for(sint n= Width*Height; n>0; n--, dst++, tileColor++)
	{
		uint16	tc= *tileColor;
		// modulate R.
		dst->R= ( (tc>>11) * dst->R)>>5;
		// modulate G.
		dst->G= (((tc>>5)&63) * dst->G)>>6;
		// modulate B.
		dst->B= ( (tc&31) * dst->B)>>5;
	}

}



// ***************************************************************************
const CRGBA	*CPatchDLMContext::computeTileFarSrcDeltas(sint nRot, bool is256x256, uint8 uvOff, const CRGBA *srcPixel, sint &srcDeltaX, sint &srcDeltaY)
{
	// NB: code copied from CTextureFar::rebuildRectangle()

	// The tileSize at order1 is 2.
	uint	tileSize= 2;

	// Source size
	sint sourceSize;

	// Source offset (for 256)
	uint sourceOffset=0;

	// 256 ?
	if (is256x256)
	{
		// On the left ?
		if (uvOff&0x02)
			sourceOffset+=tileSize;

		// On the bottom ?
		if ((uvOff==1)||(uvOff==2))
			sourceOffset+=2*tileSize*tileSize;

		// Yes, 256
		sourceSize=tileSize<<1;
	}
	else
	{
		// No, 128
		sourceSize=tileSize;
	}

	// Compute offset and deltas
	switch (nRot)
	{
	case 0:
		// Source pointers
		srcPixel= srcPixel+sourceOffset;

		// Source delta
		srcDeltaX=1;
		srcDeltaY=sourceSize;
		break;
	case 1:
		{
			// Source pointers
			uint newOffset=sourceOffset+(tileSize-1);
			srcPixel=srcPixel+newOffset;

			// Source delta
			srcDeltaX=sourceSize;
			srcDeltaY=-1;
		}
		break;
	case 2:
		{
			// Destination pointer
			uint newOffset=sourceOffset+(tileSize-1)*sourceSize+tileSize-1;
			srcPixel=srcPixel+newOffset;

			// Source delta
			srcDeltaX=-1;
			srcDeltaY=-sourceSize;
		}
		break;
	case 3:
		{
			// Destination pointer
			uint newOffset=sourceOffset+(tileSize-1)*sourceSize;
			srcPixel=srcPixel+newOffset;

			// Source delta
			srcDeltaX=-sourceSize;
			srcDeltaY=1;
		}
		break;
	}

	return srcPixel;
}


// ***************************************************************************
void		CPatchDLMContext::copyTileToTexture(const CRGBA *srcPixel, sint srcDeltaX, sint srcDeltaY, CRGBA *dstPixel, uint dstStride)
{
	// copy the 2x2 tile to the texture.

	// first line.
	dstPixel[0]= srcPixel[0];
	dstPixel[1]= srcPixel[srcDeltaX];
	// second line.
	dstPixel[0+dstStride]= srcPixel[srcDeltaY];
	dstPixel[1+dstStride]= srcPixel[srcDeltaY+srcDeltaX];
}

// ***************************************************************************
void		CPatchDLMContext::blendTileToTexture(const CRGBA *srcPixel, sint srcDeltaX, sint srcDeltaY, CRGBA *dstPixel, uint dstStride)
{
	// blend the 2x2 tile with the texture.
	CRGBA	*dst;
	CRGBA	src;

	// first line.
	dst= &dstPixel[0]; src= srcPixel[0];
	dst->blendFromuiRGBOnly(*dst, src, src.A);

	dst= &dstPixel[1]; src= srcPixel[srcDeltaX];
	dst->blendFromuiRGBOnly(*dst, src, src.A);

	// second line.
	dst= &dstPixel[0+dstStride]; src= srcPixel[srcDeltaY];
	dst->blendFromuiRGBOnly(*dst, src, src.A);

	dst= &dstPixel[1+dstStride]; src= srcPixel[srcDeltaY+srcDeltaX];
	dst->blendFromuiRGBOnly(*dst, src, src.A);
}


} // NL3D
