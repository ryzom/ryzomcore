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

// Precompiled header
#include "stdafx.h"

#include "export_nel.h"
#include <vector>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

#include "calc_lm_plane.h"

// -----------------------------------------------------------------------------------------------
SLMPlane::SLMPlane ()
{ 
	nNbLayerUsed = 1;
	x = y = 0; w = h = 1; msk.resize(1); msk[0] = 0; 
	col.resize(1); 
	col[0].R = col[0].G = col[0].B = col[0].A = 0.0f;
	ray.resize(1);
	ray[0] = 0;
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::newLayer ()
{
	col.resize( w*h*(nNbLayerUsed+1), CRGBAF(0.0f,0.0f,0.0f,0.0f) );
	nNbLayerUsed += 1;
}

// -----------------------------------------------------------------------------------------------
bool SLMPlane::isAllBlack (uint8 nLayerNb)
{
	for( uint32 i = 0; i < w*h; ++i )
		if( (col[i+w*h*nLayerNb].R > 0.06f) || // around 15/255
			(col[i+w*h*nLayerNb].G > 0.06f) ||
			(col[i+w*h*nLayerNb].B > 0.06f) )
			return false; // Not all is black
	return true;
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::copyColToBitmap8x2 (CBitmap* pImage, uint32 nLayerNb, CRGBAF lA, CRGBAF lD)
{
	if( nLayerNb >= nNbLayerUsed )
		return;
	
	if( ( pImage->getWidth() != w ) ||
		( pImage->getHeight() != h ) )
	{
		pImage->resize(w,h);
	}
	
	/* 
		We want to compress the RGBA lightmap L32 into a (lA,lD,L8) lightmap, where lA and lD are constants.

		lA and lD are given, so we have to compute L8.

		In the standard 32 bits Lightmap shader, for a single lightmap, the final formula is:
			O= L32*F*T
			where:
				O:		output
				L32:	the lightmap (actually in 16 bits RGB)
				F:		the lightmap factor (for light animation or activation)
				T:		the diffuse texture

		In the "8bits *2" Lightmap shader, for a single lightmap, the final formula is:
			O= (mA+L8*mD)*F*T*2
			where:
				L8:		the 8 bits lightmap
				mA:		Material Lightmap Ambient term. equal to lA/2, for convenience, and avoid too much "ambient clamping"
				mD:		Material Lightmap Diffuse term. equal to lD.
				*2:		the pixel shader multiplyBy2, to allow some TextureDiffuse burn.

		Hence, we have to solve the simple equation for L8:
			(lA/2+L8*lD)*2 = L32

		which result into:

			L8= ((L32-lA) / lD) * 0.5

		We define Vector0/Vector1 operation as the projection operation:  
			scalar= Vector0*Vector1 / sqrnorm(Vector1).
	*/

	// the dir vector lD
	CVector		vDir(lD.R, lD.G, lD.B);
	float		OOSqrNorm= 0;
	if(!vDir.isNull())
		OOSqrNorm= 1.0f / vDir.sqrnorm();

	// for all pixel of the layer
	CObjectVector<uint8> &vBitmap = pImage->getPixels();
	for( uint32 i = 0; i < w*h; ++i )
	{
		const CRGBAF	&srcCol= col[i+w*h*nLayerNb];

		// find the factor f, such that f= (L32-lA)/lD
		float	f;
		f= vDir * CVector(srcCol.R - lA.R, srcCol.G - lA.G, srcCol.B - lA.B);
		f*= OOSqrNorm;

		// we are in multiply x2 => compress to 8 bits from 0 to 127
		const float fMult = 127.0f;
		
		f*= fMult;
		clamp(f, 0.f, 255.0f);
		uint8	f8= (uint8)f;

		vBitmap[4*i+0] = f8;
		vBitmap[4*i+1] = f8;
		vBitmap[4*i+2] = f8;
		
		if (msk[i] != 0)
			vBitmap[4*i+3] = 255;
		else
			vBitmap[4*i+3] = 0;
	}
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::copyColToBitmap32 (CBitmap* pImage, uint32 nLayerNb)
{
	if( nLayerNb >= nNbLayerUsed )
		return;
	if( ( pImage->getWidth() != w ) ||
		( pImage->getHeight() != h ) )
	{
		pImage->resize(w,h);
	}

	CObjectVector<uint8> &vBitmap = pImage->getPixels();

	for( uint32 i = 0; i < w*h; ++i )
	{
		// if we are in multiply x2 we have to set the following value to 127.0 else set to 255.0
		// By default we are in multiply x2 in Lightmap 8 bits mode but not in 32 bits mode
		const float fMult = 255.0;
		
		if( (fMult*col[i+w*h*nLayerNb].R) > 255.0 )
			vBitmap[4*i+0] = 255;
		else
			vBitmap[4*i+0] = (uint8)(fMult*col[i+w*h*nLayerNb].R);

		if( (fMult*col[i+w*h*nLayerNb].G) > 255.0 )
			vBitmap[4*i+1] = 255;
		else
			vBitmap[4*i+1] = (uint8)(fMult*col[i+w*h*nLayerNb].G);
		if( (fMult*col[i+w*h*nLayerNb].B) > 255.0 )
			vBitmap[4*i+2] = 255;
		else
			vBitmap[4*i+2] = (uint8)(fMult*col[i+w*h*nLayerNb].B);
		// Mask go into alpha channel
		if (msk[i] != 0)
			vBitmap[4*i+3] = 255;
		else
			vBitmap[4*i+3] = 0;
	}
}

// -----------------------------------------------------------------------------------------------
// Put me in the plane Dst (copy the col and mask or mask only)
void SLMPlane::putIn (SLMPlane &Dst, bool bMaskOnly)
{
	uint32 a, b, c;

	while (this->nNbLayerUsed > Dst.nNbLayerUsed)
		Dst.newLayer();

	if( ( (this->w + this->x) > Dst.w ) || ( (this->h + this->y) > Dst.h ) )
	{
		a = 0; b = 0;
	}
	for( b = 0; b < this->h; ++b )
	for( a = 0; a < this->w; ++a )
		if( this->msk[a+b*this->w] != 0 )
		{
			Dst.msk[(this->x+a)+(this->y+b)*Dst.w] = this->msk[a+b*this->w];
			if( bMaskOnly == false )
				for( c = 0; c < this->nNbLayerUsed; ++c )
					Dst.col[(this->x+a)+(this->y+b)*Dst.w+Dst.w*Dst.h*c] = this->col[a+b*this->w+w*h*c];
		}
}

// -----------------------------------------------------------------------------------------------
// Test the mask between me and the plane dst (with my decalage)
bool SLMPlane::testIn (SLMPlane &Dst)
{
	uint32 a, b;
	for( b = 0; b < this->h; ++b )
	for( a = 0; a < this->w; ++a )
		if( this->msk[a+b*this->w] != 0 )
			if( Dst.msk[(this->x+a)+(this->y+b)*Dst.w] != 0 )
				return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
// Try all position to put me in the plane dst
bool SLMPlane::tryAllPosToPutIn (SLMPlane &Dst)
{
	uint32 i, j;

	if( this->w > Dst.w ) return false;
	if( this->h > Dst.h ) return false;

	// For all position test if the Src plane can be put in
	for( j = 0; j < (Dst.h-this->h); ++j )
	for( i = 0; i < (Dst.w-this->w); ++i )
	{
		this->x = i; this->y = j;
		if( testIn( Dst ) )
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
// Do not stretch the image inside the plane just enlarge and fill with black
void SLMPlane::resize (uint32 nNewSizeX, uint32 nNewSizeY)
{
	vector<uint8> vImgTemp;
	vector<CRGBAF> vImgTemp2;
	uint32 i, j, k;

	// Resize the mask
	vImgTemp.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(this->h,nNewSizeY); ++j )
	for( i = 0; i < min(this->w,nNewSizeX); ++i )
		vImgTemp[i+j*nNewSizeX] = this->msk[i+j*this->w];

	this->msk.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		this->msk[i] = vImgTemp[i];

	// Resize the raytrace information
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(this->h,nNewSizeY); ++j )
	for( i = 0; i < min(this->w,nNewSizeX); ++i )
		vImgTemp[i+j*nNewSizeX] = this->ray[i+j*this->w];

	this->ray.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		this->ray[i] = vImgTemp[i];

	// The same as the mask but for the bitmap
	vImgTemp2.resize(nNewSizeX*nNewSizeY*nNbLayerUsed);
	for( i = 0; i < nNewSizeX*nNewSizeY*nNbLayerUsed; ++i )
	{ vImgTemp2[i].R = vImgTemp2[i].G = vImgTemp2[i].B = vImgTemp2[i].A = 0.0f; }

	for( k = 0; k < nNbLayerUsed; ++k )
		for( j = 0; j < min(this->h,nNewSizeY); ++j )
		for( i = 0; i < min(this->w,nNewSizeX); ++i )
			vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k] = this->col[i+j*this->w+w*h*k];

	this->col.resize(nNewSizeX*nNewSizeY*nNbLayerUsed);
	for( i = 0; i < nNewSizeX*nNewSizeY*nNbLayerUsed; ++i )
		this->col[i] = vImgTemp2[i];

	this->w = nNewSizeX;
	this->h = nNewSizeY;
}

// -----------------------------------------------------------------------------------------------
// Stretch a plane by a given factor 4.0 -> multiply its size by 4 and 0.5 -> halves its size
// Take care the decalage is affected by the scaling factor (like homothetie)
void SLMPlane::stretch (double osFactor)
{
	if( osFactor < 0.0 )
		osFactor = 0.0;
	uint32 nNewSizeX = (uint32)(this->w * osFactor);
	uint32 nNewSizeY = (uint32)(this->h * osFactor);
	vector<uint8> vImgTemp;
	vector<uint8> vImgTempRay;
	vector<CRGBAF> vImgTemp2;
	uint32 i, j, k;

	// Reduce the color
	vImgTempRay.resize(nNewSizeX * nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTempRay[i] = 0;	

	vImgTemp2.resize( nNewSizeX * nNewSizeY * nNbLayerUsed );
	for( i = 0; i < nNewSizeX*nNewSizeY*nNbLayerUsed; ++i )
	{ vImgTemp2[i].R = vImgTemp2[i].G = vImgTemp2[i].B = vImgTemp2[i].A = 0.0f; }

	vImgTemp.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	double dx, dy, x, y;
	if( osFactor > 1.0 ) // Enlarge the image
	{
		dx = 1.0/osFactor;
		dy = 1.0/osFactor;
		y = 0.0;
		for( j = 0; j < nNewSizeY; ++j )
		{
			x = 0.0;
			for( i = 0; i < nNewSizeX; ++i )
			{
				if( this->msk[((sint32)x)+((sint32)y)*this->w] != 0 )
				{
					vImgTemp[i+j*nNewSizeX] = 1;
				}
				for( k = 0; k < nNbLayerUsed; ++k )
				{
					vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].R += col[((sint32)x)+((sint32)y)*w+w*h*k].R;
					vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].G += col[((sint32)x)+((sint32)y)*w+w*h*k].G;
					vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].B += col[((sint32)x)+((sint32)y)*w+w*h*k].B;
					vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A += 1.0f;
				}
				vImgTempRay[i+j*nNewSizeX] = ray[((sint32)x)+((sint32)y)*w];
				x += dx;
			}
			y += dy;
		}
	}
	else // Reduce image
	{
		dx = osFactor;
		dy = osFactor;
		y = 0.0;
		for( j = 0; j < this->h; ++j )
		{
			x = 0.0;
			for( i = 0; i < this->w; ++i )
			{
				if( this->msk[i+j*this->w] != 0 )
				{
					vImgTemp[((sint32)x)+((sint32)y)*nNewSizeX] = 1;
				}
				for( k = 0; k < nNbLayerUsed; ++k )
				{
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX+nNewSizeX*nNewSizeY*k].R += col[i+j*w+w*h*k].R;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX+nNewSizeX*nNewSizeY*k].G += col[i+j*w+w*h*k].G;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX+nNewSizeX*nNewSizeY*k].B += col[i+j*w+w*h*k].B;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX+nNewSizeX*nNewSizeY*k].A += 1.0f;
				}
				vImgTempRay[((sint32)x)+((sint32)y)*nNewSizeX] = ray[i+j*w];
				x += dx;
			}
			y += dy;
		}
	}

	for( k = 0; k < nNbLayerUsed; ++k )
	for( j = 0; j < nNewSizeY; ++j )
	for( i = 0; i < nNewSizeX; ++i )
	if( vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A > 1.0f )
	{
		vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].R /= vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A;
		vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].G /= vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A;
		vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].B /= vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A;
		vImgTemp2[i+j*nNewSizeX+nNewSizeX*nNewSizeY*k].A = 1.0f;
	}

	col.resize( nNewSizeX * nNewSizeY * nNbLayerUsed );
	for( i = 0; i < nNewSizeX*nNewSizeY*nNbLayerUsed; ++i )
		col[i] = vImgTemp2[i];

	msk.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		msk[i] = vImgTemp[i];

	ray.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		ray[i] = vImgTempRay[i];

	this->w = nNewSizeX;
	this->h = nNewSizeY;
	this->x = (sint32)(this->x * osFactor);
	this->y = (sint32)(this->y * osFactor);
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::createFromPlane (SLMPlane &Src)
{
	uint i;
	// Resize to the same size
	resize(Src.w,Src.h);
	x = Src.x;
	y = Src.y;
	// Copy the mask
	for( i = 0; i < Src.w*Src.h; ++i )
		msk[i] = Src.msk[i];
	// Copy the faces
	faces.resize(Src.faces.size());
	for( i = 0; i < Src.faces.size(); ++i )
		faces[i] = Src.faces[i];
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::copyFirstLayerTo (SLMPlane &Dst, uint8 nDstLayer)
{
	uint i;

	if( ( Dst.w != w ) || ( Dst.h != h ) )
		Dst.resize( w, h );

	while( nDstLayer >= Dst.nNbLayerUsed )
		Dst.newLayer();

	for( i = 0; i < w*h; ++i )
		Dst.col[i+w*h*nDstLayer] = col[i];
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::contourDetect ()
{
	uint i, j;
	vector<uint8> vImgTemp;
	vImgTemp.resize(w*h);
	for( i = 0; i < w*h; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < h; ++j )
	for( i = 0; i < w; ++i )
	{
		if( i > 0 )
		if( ray[i+j*w] != ray[i-1+j*w] )
			vImgTemp[i+j*w] = 1;
		if( i < (w-1) )
		if( ray[i+j*w] != ray[i+1+j*w] )
			vImgTemp[i+j*w] = 1;
		if( j > 0 )
		if( ray[i+j*w] != ray[i+(j-1)*w] )
			vImgTemp[i+j*w] = 1;
		if( j < (h-1) )
		if( ray[i+j*w] != ray[i+(j+1)*w] )
			vImgTemp[i+j*w] = 1;

		if( ray[i+j*w] == 255 )
			vImgTemp[i+j*w] = 1;
	}

	for( i = 0; i < w*h; ++i )
		ray[i] = vImgTemp[i];


	/*{
		CBitmap b;
		COFile f( "c:\\temp\\gloup.tga" );
		b.resize(w,h);
		CObjectVector<uint8>&bits = b.getPixels();
		for( i = 0; i < w*h; ++i )
		{
			bits[i*4+0] = bits[i*4+1] = bits[i*4+2] = bits[i*4+3] = ray[i]*255;
		}

		b.writeTGA( f, 32 );
	}*/
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::andRayWidthMask ()
{
	for( uint i = 0; i < w*h; ++i )
	if( ray[i] == 0 ) 
		msk[i] = 0;
}

// -----------------------------------------------------------------------------------------------
bool SLMPlane::isInTriangleOrEdge(	double x, double y, 
									double xt1, double yt1, 
									double xt2, double yt2, 
									double xt3, double yt3)
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}

// -----------------------------------------------------------------------------------------------
bool SLMPlane::segmentIntersection(	double x1, double y1,
									double x2, double y2, 
									double x3, double y3, 
									double x4, double y4)
{
	double denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	if( denominator == 0.0 )
		return false; // The segment are colinear
	double k = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	k = ( (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::createFromFace (CMesh::CFace *pF)
{
	double	lumx1 = pF->Corner[0].Uvws[1].U, lumy1 = pF->Corner[0].Uvws[1].V, 
			lumx2 = pF->Corner[1].Uvws[1].U, lumy2 = pF->Corner[1].Uvws[1].V, 
			lumx3 = pF->Corner[2].Uvws[1].U, lumy3 = pF->Corner[2].Uvws[1].V;
	double minx, miny;
	double maxx, maxy;
	sint32 j, k;

	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	sint32 decalX = ((sint32)floor(minx-0.5)) - x;
	sint32 decalY = ((sint32)floor(miny-0.5)) - y;
	sint32 sizeX = 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5));
	sint32 sizeY = 1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5));

	lumx1 -= x; lumy1 -= y;
	lumx2 -= x; lumy2 -= y;
	lumx3 -= x;	lumy3 -= y;

	// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
	// Or the square is in the triangle
	
	for (j = decalY; j < (decalY+sizeY-1); ++j)
	for (k = decalX; k < (decalX+sizeX-1); ++k)
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangleOrEdge(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			msk[k   + j    *w] = 1;
			msk[1+k + j    *w] = 1;
			msk[k   + (1+j)*w] = 1;
			msk[1+k + (1+j)*w] = 1;
		}
		else
		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			msk[k   + j    *w] = 1;
			msk[1+k + j    *w] = 1;
			msk[k   + (1+j)*w] = 1;
			msk[1+k + (1+j)*w] = 1;
		}

	}
	
	// For all the points of the triangle update the square
	msk[((sint32)(lumx1-0.5))   + ((sint32)(lumy1-0.5))    *w] = 1;
	msk[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))    *w] = 1;
	msk[((sint32)(lumx1-0.5))   + (1+((sint32)(lumy1-0.5)))*w] = 1;
	msk[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*w] = 1;

	msk[((sint32)(lumx2-0.5))   + ((sint32)(lumy2-0.5))    *w] = 1;
	msk[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))    *w] = 1;
	msk[((sint32)(lumx2-0.5))   + (1+((sint32)(lumy2-0.5)))*w] = 1;
	msk[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*w] = 1;
	
	msk[((sint32)(lumx3-0.5))   + ((sint32)(lumy3-0.5))    *w] = 1;
	msk[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))    *w] = 1;
	msk[((sint32)(lumx3-0.5))   + (1+((sint32)(lumy3-0.5)))*w] = 1;
	msk[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*w] = 1;
}

// -----------------------------------------------------------------------------------------------
void SLMPlane::createFromFaceGroup (vector<CMesh::CFace*>::iterator ItFace, uint32 nNbFace)
{
	uint32 i, j;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	CMesh::CFace *pF;

	faces.resize( nNbFace );

	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;
		for( j = 0; j < 3; ++j )
		{
			if( rMinU > pF->Corner[j].Uvws[1].U ) rMinU = pF->Corner[j].Uvws[1].U;
			if( rMaxU < pF->Corner[j].Uvws[1].U ) rMaxU = pF->Corner[j].Uvws[1].U;
			if( rMinV > pF->Corner[j].Uvws[1].V ) rMinV = pF->Corner[j].Uvws[1].V;
			if( rMaxV < pF->Corner[j].Uvws[1].V ) rMaxV = pF->Corner[j].Uvws[1].V;
		}
		faces[i] = pF;
		++ItParseI;
	}

	uint32 w = ( 1 + ((sint32)floor( rMaxU + 0.5 )) - ((sint32)floor( rMinU - 0.5 )) );
	uint32 h = ( 1 + ((sint32)floor( rMaxV + 0.5 )) - ((sint32)floor( rMinV - 0.5 )) );
	resize( w, h );
	x = ( ((sint32)floor( rMinU - 0.5 )) );
	y = ( ((sint32)floor( rMinV - 0.5 )) );
	for( j = 0; j < w*h; ++j )
		msk[j] = 0;

	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;

		// Create Mask
		createFromFace (pF);

		++ItParseI;
	}
}

// -----------------------------------------------------------------------------------------------
CRGBAF SLMPlane::getAverageColor (uint8 nLayerNb)
{
	uint32 a, b, c, nNbPixel = 0;
	CRGBAF ret (0.0f, 0.0f, 0.0f, 0.0f);

	if (nLayerNb > nNbLayerUsed)
		return ret;

	c = nLayerNb;
	for( b = 0; b < this->h; ++b )
	for( a = 0; a < this->w; ++a )
		if( this->msk[a+b*this->w] != 0 )
		{
			ret += this->col[a+b*this->w+w*h*c];
			++nNbPixel;
		}
	if (nNbPixel > 0)
	{
		ret.R = ret.R / (float)nNbPixel;
		ret.G = ret.G / (float)nNbPixel;
		ret.B = ret.B / (float)nNbPixel;
		ret.A = ret.A / (float)nNbPixel;
	}
	return ret;
}

// -----------------------------------------------------------------------------------------------
bool SLMPlane::isSameColorAs (uint8 nLayerNb, CRGBAF color, float precision)
{
	uint32 a, b, c;

	if (nLayerNb > nNbLayerUsed)
		return false;

	c = nLayerNb;
	for( b = 0; b < this->h; ++b )
	for( a = 0; a < this->w; ++a )
		if( this->msk[a+b*this->w] != 0 )
		{
			CRGBAF lc = this->col[a+b*this->w+w*h*c];

			if ((fabsf(lc.R-color.R) > precision) ||
				(fabsf(lc.G-color.G) > precision) ||
				(fabsf(lc.B-color.B) > precision))
				return false;
		}
	return true;
}
