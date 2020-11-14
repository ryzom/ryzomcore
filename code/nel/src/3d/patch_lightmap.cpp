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


#include "nel/3d/patch.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/light_influence_interpolator.h"
#include "nel/3d/patchdlm_context.h"

using	namespace	std;
using	namespace	NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// Precalc table used to decompress shadow map
// NB: if you want to change thoses values, see unpackLumelBlock, cause hardcoded.
static const uint NL3DDecompressLumelFactor0Case0[8]=
{
	7, 0, 6, 5, 4, 3, 2, 1
};
static const uint NL3DDecompressLumelFactor1Case0[8]=
{
	0, 7, 1, 2, 3, 4, 5, 6
};
static const uint NL3DDecompressLumelFactor0Case1[6]=
{
	5, 0, 4, 3, 2, 1,
};
static const uint NL3DDecompressLumelFactor1Case1[6]=
{
	0, 5, 1, 2, 3, 4,
};
// ***************************************************************************
void		CPatch::unpackLumelBlock (uint8 *dest, const uint8 *src)
{
	// Take the two alpha values
	uint alpha0=src[0];
	uint alpha1=src[1];

	// precompute the 8 possible values, for each possible code.
	// ------------------
	uint8	values[8];
	// Case 0
	if (alpha0>alpha1)
	{
		// unrolled, and hardcoded for faster compute
		values[0]= alpha0;
		values[1]= alpha1;
		values[2]= (uint8) ( (alpha0*219 + alpha1*37 ) >>8 ) ;	// 6*256/7
		values[3]= (uint8) ( (alpha0*183 + alpha1*73 ) >>8 ) ;	// 5*256/7
		values[4]= (uint8) ( (alpha0*146 + alpha1*110) >>8 ) ;	// 4*256/7
		values[5]= (uint8) ( (alpha0*110 + alpha1*146) >>8 ) ;	// 3*256/7
		values[6]= (uint8) ( (alpha0*73  + alpha1*183) >>8 ) ;	// 2*256/7
		values[7]= (uint8) ( (alpha0*37  + alpha1*219) >>8 ) ;	// 1*256/7
	}
	// Case 1
	else
	{
		// unrolled, and hardcoded for faster compute
		values[0]= alpha0;
		values[1]= alpha1;
		values[2]= (uint8) ( (alpha0*205 + alpha1*51 ) >>8 ) ;	// 4*256/5
		values[3]= (uint8) ( (alpha0*154 + alpha1*102) >>8 ) ;	// 3*256/5
		values[4]= (uint8) ( (alpha0*102 + alpha1*154) >>8 ) ;	// 2*256/5
		values[5]= (uint8) ( (alpha0*51  + alpha1*205) >>8 ) ;	// 1*256/5
		values[6]= 0;
		values[7]= 255;
	}


	// For each pixel, set the value according to the code
	// ------------------
	uint	block8Pixs[2];
	// Split the 48 bits data in 2 24 bits pass.
	block8Pixs[0]= ((uint)src[2]<<16) + ((uint)src[3]<<8) + ((uint)src[4]) ;
	block8Pixs[1]= ((uint)src[5]<<16) + ((uint)src[6]<<8) + ((uint)src[7]) ;

	// write all lumels
	for(uint i=0; i<2; i++)
	{
		uint	blockPix= block8Pixs[i];
		// parse the 8 pixs, and write seq to dest
		for(uint n=8; n>0; n--, dest++)
		{
			uint code= (blockPix>>21)&0x7;

			// read LUT, and store
			*dest= values[code];

			// shift the block
			blockPix<<= 3;
		}
	}

}

// ***************************************************************************
inline uint8 getUnpackLumelBlock (const uint8 *src, uint pixel)
{
	// Offset of the bit
	pixel*=3;
	uint offset=(pixel>>3)+2;
	uint bits=pixel&7;

	// Uncompress 16 codes
	uint code;

	// Get the code
	if (bits<=5)
		code=(src[offset]>>(5-bits))&0x7;
	else
		code= ( (src[offset]<<(bits-5)) | (src[offset+1]>>(13-bits)) )&0x7;

	// Case 0
	if (src[0]>src[1])
	{
		// Decompress the data
		return (uint8)((NL3DDecompressLumelFactor0Case0[code]*src[0]+NL3DDecompressLumelFactor1Case0[code]*src[1])/7);
	}
	// Case 1
	else
	{
		// Decompress the data
		if (code<6)
			return (uint8)((NL3DDecompressLumelFactor0Case1[code]*src[0]+NL3DDecompressLumelFactor1Case1[code]*src[1])/5);
		else if (code==6)
			return 0;
		else
			return 255;
	}
}

// ***************************************************************************
void		CPatch::unpackShadowMap (uint8 *pLumelDest)
{
	// Input of compresed data
	uint8 *compressedData=&CompressedLumels[0];

	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Destination lumel block size
	uint lumelDestBlockSize=4;

	// Destination lumel line block size
	uint lumelDestLineBlockSize=lumelCount*lumelDestBlockSize;

	// Each line block
	for (uint lineBlock=0; lineBlock<numLineBlock; lineBlock++)
	{
		uint countVx4=16;

		// Block pointer
		uint8 *blockLine=pLumelDest;

		// Each lumel block
		for (uint lumelBlock=0; lumelBlock<numLumelBlock; lumelBlock++)
		{
			// *** Unpack the block
			uint countU=4;

			// Destination lumel
			uint8 *blockDest=blockLine;

			// Temp block
			uint8 block[4*4];

			// Block unpacking...
			unpackLumelBlock (block, compressedData);

			// Copy the lumels
			for (uint v=0; v<countVx4; v+=4)
			{
				for (uint u=0; u<countU; u++)
				{
					// Copy the lumel
					blockDest[u]=block[v+u];
				}

				// Next line
				blockDest+=lumelCount;
			}

			// Next source block
			compressedData+=NL_BLOCK_LUMEL_COMPRESSED_SIZE;

			// Next block on the line
			blockLine+=lumelDestBlockSize;
		}

		// Next line of block
		pLumelDest+=lumelDestLineBlockSize;
	}
}

// ***************************************************************************
uint		CPatch::evalLumelBlock (const uint8 *original, const uint8 *unCompressed, uint width, uint height)
{
	// Sum
	uint sum=0;

	// Eval error for each..
	for (uint v=0; v<height; v++)
	for (uint u=0; u<width; u++)
	{
		sum += abs((sint)original[v*4+u]-(sint)unCompressed[v*4+u]);
	}

	// return the sum
	return sum;
}

// ***************************************************************************
void		CPatch::packLumelBlock (uint8 *dest, const uint8 *source, uint8 alpha0, uint8 alpha1)
{
	// Precalc the height values..
	uint8 value[8];

	// For each value
	uint i;
	for (i=0; i<8; i++)
	{
		// Case 0 or 1 ?
		if (alpha0>alpha1)
			// Case 0
			value[i]=(NL3DDecompressLumelFactor0Case0[i]*alpha0+NL3DDecompressLumelFactor1Case0[i]*alpha1)/7;
		else
		{
			if (i<6)
				value[i]=(NL3DDecompressLumelFactor0Case1[i]*alpha0+NL3DDecompressLumelFactor1Case1[i]*alpha1)/5;
			else if (i==6)
				value[i]=0;
			else
				value[i]=255;
		}
	}

	// Store alpha value
	dest[0]=alpha0;
	dest[1]=alpha1;

	// Clear dest codes
	for (i=0; i<6; i++)
	{
		// Clear the code
		dest[2+i]=0;
	}

	// For each original select the best
	uint codeOffset=2;
	sint codeShift=5;
	for (i=0; i<16; i++)
	{
		// Best dist and code
		uint bestDist=10000;
		uint8 bestCode=0;

		// Calc distance
		for (uint code=0; code<8; code++)
		{
			// Distance from original value
			uint dist=abs ((sint)(source[i])-(sint)(value[code]));

			// The best ?
			if (dist<bestDist)
			{
				// New best
				bestDist=dist;
				bestCode=code;
			}

			// Perfect, stop searching
			if (dist==0)
				break;
		}

		// Store the best
		if (codeShift>=0)
			dest[codeOffset]|=bestCode<<codeShift;
		else
		{
			dest[codeOffset]|=bestCode>>(-codeShift);
			dest[codeOffset+1]|=bestCode<<(8+codeShift);
		}


		// Next shift
		codeShift-=3;
		if (codeShift<=-3)
		{
			codeOffset++;
			codeShift+=8;
		}
	}
}

// ***************************************************************************
void		CPatch::getTileTileColors(uint ts, uint tt, CRGBA corners[4])
{
	for(sint i=0;i<4;i++)
	{
		CTileColor	&tcol= TileColors[ (tt+(i>>1))*(OrderS+1) + (ts+(i&1)) ];
		CRGBA		&col= corners[i];
		col.set565 (tcol.Color565);
	}
}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearColor(CRGBA	corners[4], uint x, uint y, uint &R, uint &G, uint &B)
{
	// Fast bilinear and modulate.
	// hardcoded for 4 pixels.
	nlassert(NL_LUMEL_BY_TILE==4);

	// expand to be on center of pixel=> 1,3,5 or 7.
	x= (x<<1)+1;
	y= (y<<1)+1;
	uint	x1= 8-x;
	uint	y1= 8-y;

	// compute weight factors.
	uint	xy=		x*y;
	uint	x1y=	x1*y;
	uint	xy1=	x*y1;
	uint	x1y1=	x1*y1;

	// bilinear
	// pix left top.
	R = corners[0].R * x1y1;
	G = corners[0].G * x1y1;
	B = corners[0].B * x1y1;
	// pix right top.
	R+= corners[1].R * xy1;
	G+= corners[1].G * xy1;
	B+= corners[1].B * xy1;
	// pix left bottom.
	R+= corners[2].R * x1y;
	G+= corners[2].G * x1y;
	B+= corners[2].B * x1y;
	// pix right bottom.
	R+= corners[3].R * xy;
	G+= corners[3].G * xy;
	B+= corners[3].B * xy;

}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearColorAndModulate(CRGBA	corners[4], uint x, uint y, CRGBA &res)
{
	uint R, G, B;
	bilinearColor(corners, x, y, R, G, B);

	// modulate with input.
	R *= res.R;
	G *= res.G;
	B *= res.B;

	// R,G,B are on 14 bits
	res.R = R >> 14;
	res.G = G >> 14;
	res.B = B >> 14;
}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearColorAndModulatex2(CRGBA	corners[4], uint x, uint y, CRGBA &res)
{
	uint R, G, B;
	bilinearColor(corners, x, y, R, G, B);

	// modulate with input.
	R *= res.R;
	G *= res.G;
	B *= res.B;

	// result not >> 14 but >> 13 and clamp from 0 to 255
	R = R >> 13;
	G = G >> 13;
	B = B >> 13;

	res.R = min (R, 255U);
	res.G = min (G, 255U);
	res.B = min (B, 255U);
}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearColorDiv2AndAdd(CRGBA	corners[4], uint x, uint y, CRGBA &res)
{
	uint	R,G,B;
	bilinearColor(corners, x, y, R, G, B);

	// add with input. Resulting TLI must be on 7 bits.
	R= (R>>7) + res.R;
	G= (G>>7) + res.G;
	B= (B>>7) + res.B;

	R= min(R, 255U);
	G= min(G, 255U);
	B= min(B, 255U);

	// result.
	res.R= R;
	res.G= G;
	res.B= B;
}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearColorAndAdd(CRGBA	corners[4], uint x, uint y, CRGBA &res)
{
	uint	R,G,B;
	bilinearColor(corners, x, y, R, G, B);

	// add with input. Resulting TLI must be on 7 bits.
	R= (R>>6) + res.R;
	G= (G>>6) + res.G;
	B= (B>>6) + res.B;

	R= min(R, 255U);
	G= min(G, 255U);
	B= min(B, 255U);

	// result.
	res.R= R;
	res.G= G;
	res.B= B;
}



// ***************************************************************************
void		CPatch::modulateTileLightmapWithTileColors(uint ts, uint tt, CRGBA *dest, uint stride)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// For all lumel, bilinear.
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// compute this pixel, and modulate
			bilinearColorAndModulatex2(corners, x, y, dest[y*stride + x]);
		}
	}
}


// ***************************************************************************
void		CPatch::modulateTileLightmapEdgeWithTileColors(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// get coordinate according to edge.
	uint	x=0,y=0;
	switch(edge)
	{
	case 0: x= 0; break;
	case 1: y= NL_LUMEL_BY_TILE-1; break;
	case 2: x= NL_LUMEL_BY_TILE-1; break;
	case 3: y= 0; break;
	};

	// For all lumel of the edge, bilinear.
	uint	i;
	for(i=0; i<NL_LUMEL_BY_TILE; i++)
	{
		// if vertical edge
		if( (edge&1)==0 )	y= i;
		// else horizontal edge
		else				x= i;

		// manage inverse.
		uint	where;
		if(inverse)		where= (NL_LUMEL_BY_TILE-1)-i;
		else			where= i;
		// compute this pixel, and modulate
		bilinearColorAndModulatex2(corners, x, y, dest[where*stride]);
	}
}


// ***************************************************************************
void		CPatch::modulateTileLightmapPixelWithTileColors(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// compute this pixel, and modulate
	bilinearColorAndModulatex2(corners, s, t, *dest);
}



// ***************************************************************************
void		CPatch::computeTileLightmapAutomatic(uint ts, uint tt, CRGBA *dest, uint stride)
{
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// compute this pixel.
			computeTileLightmapPixelAutomatic(ts, tt, x, y, dest+ y*stride + x);
		}
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapEdgeAutomatic(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// get coordinate according to edge.
	uint	x=0,y=0;
	switch(edge)
	{
	case 0: x= 0; break;
	case 1: y= NL_LUMEL_BY_TILE-1; break;
	case 2: x= NL_LUMEL_BY_TILE-1; break;
	case 3: y= 0; break;
	};

	uint	i;
	for(i=0; i<NL_LUMEL_BY_TILE; i++)
	{
		// if vertical edge
		if( (edge&1)==0 )	y= i;
		// else horizontal edge
		else				x= i;

		// manage inverse.
		uint	where;
		if(inverse)		where= (NL_LUMEL_BY_TILE-1)-i;
		else			where= i;
		// compute this pixel.
		computeTileLightmapPixelAutomatic(ts, tt, x, y, dest+ where*stride);
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapPixelAutomatic(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	float		u,v;
	static const float	lumelSize= 1.f/NL_LUMEL_BY_TILE;

	// use 3 computeVertex to compute a normal. This is slow....
	CVector		p0, p1 ,p2;
	// 1st vert. Top-left of the lumel.
	u= (ts + s*lumelSize )/OrderS;
	v= (tt + t*lumelSize )/OrderT;
	p0= computeVertex(u, v);
	// 2nd vert. Bottom-left of the lumel.
	u= (ts + s*lumelSize )/OrderS;
	v= (tt + (t+1)*lumelSize )/OrderT;
	p1= computeVertex(u, v);
	// 3rd vert. Center-Right of the lumel.
	u= (ts + (s+1)*lumelSize )/OrderS;
	v= (tt + (t+0.5f)*lumelSize )/OrderT;
	p2= computeVertex(u, v);

	// the normal.
	CVector		normal;
	normal= (p1-p0)^(p2-p0);
	normal.normalize();

	// lighting.
	float	c= -normal*getLandscape()->getAutomaticLightDir();
	c= max(c, 0.f);
	sint	ic;

#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM) && defined(NL_USE_FASTFLOOR)
	// FastFloor using fistp. Don't care convention.
	float	fc= c*256;
	_asm
	{
		fld fc
		fistp ic
	}
#else
	ic= (sint)floor(c*256);
#endif
	clamp(ic, 0, 255);

	// ambiant/diffuse lighting.
	*dest= getLandscape()->getStaticLight()[ic];
}


// ***************************************************************************
void		CPatch::getTileLumelmapPrecomputed(uint ts, uint tt, uint8 *dest, uint stride)
{
	// Unpack the lumels
	uint8 buffer[NL_LUMEL_BY_TILE*NL_LUMEL_BY_TILE];
	unpackLumelBlock (buffer, &(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]));

	// Retrun it
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// lumel
			dest[y*stride + x]= buffer[x+(y<<NL_LUMEL_BY_TILE_SHIFT)];
		}
	}
}


// ***************************************************************************
void		CPatch::getTileLumelmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, uint8 &dest) const
{
	// Return the lumel
	dest= getUnpackLumelBlock (&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]), s+(t<<2));
}


// ***************************************************************************
void		CPatch::computeTileLightmapPrecomputed(uint ts, uint tt, CRGBA *dest, uint stride)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();
	// Unpack the lumels
	uint8 buffer[NL_LUMEL_BY_TILE*NL_LUMEL_BY_TILE];
	unpackLumelBlock (buffer, &(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]));

	// Retrun it
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// lumel
			dest[y*stride + x]=colorTable[buffer[x+(y<<NL_LUMEL_BY_TILE_SHIFT)]];
		}
	}
}

// ***************************************************************************

static uint NL3DPixelStartLumel[4]={0, 4*3, 3, 0};
static uint NL3DDeltaLumel[4]={4, 1, 4, 1};

// ***************************************************************************
void		CPatch::computeTileLightmapEdgePrecomputed(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();

	// Witch corner to start ?
	uint pixel=NL3DPixelStartLumel[edge];
	uint delta=NL3DDeltaLumel[edge];

	// For each lumels
	const uint8 *src=&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]);
	uint x;
	if (inverse)
	{
		uint inverseStride=stride*(4-1);
		for(x=0; x<4; x++)
		{
			// lumel
			dest[inverseStride-x*stride]=colorTable[getUnpackLumelBlock (src, pixel)];
			pixel+=delta;
		}
	}
	else
	{
		for(x=0; x<4; x++)
		{
			// lumel
			dest[x*stride]=colorTable[getUnpackLumelBlock (src, pixel)];
			pixel+=delta;
		}
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();

	// Return the lumel
	*dest=colorTable[getUnpackLumelBlock (&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]), s+(t<<2))];
}



// ***************************************************************************
void		CPatch::computeTileLightmap(uint ts, uint tt, CRGBA *dest, uint stride)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapAutomatic(ts, tt, dest, stride);
	else
	{
		computeTileLightmapPrecomputed(ts, tt, dest, stride);
		// Add the inlufence of TLI.
		addTileLightmapWithTLI(ts, tt, dest, stride);
	}

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapWithTileColors(ts, tt, dest, stride);
}
// ***************************************************************************
void		CPatch::computeTileLightmapEdge(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapEdgeAutomatic(ts, tt, edge, dest, stride, inverse);
	else
	{
		computeTileLightmapEdgePrecomputed(ts, tt, edge, dest, stride, inverse);
		// Add the inlufence of TLI.
		addTileLightmapEdgeWithTLI(ts, tt, edge, dest, stride, inverse);
	}

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapEdgeWithTileColors(ts, tt, edge, dest, stride, inverse);
}


// ***************************************************************************
void		CPatch::computeTileLightmapPixel(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapPixelAutomatic(ts, tt, s, t, dest);
	else
	{
		computeTileLightmapPixelPrecomputed(ts, tt, s, t, dest);
		// Add the inlufence of TLI.
		addTileLightmapPixelWithTLI(ts, tt, s, t, dest);
	}

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapPixelWithTileColors(ts, tt, s, t, dest);
}


// ***************************************************************************
void		CPatch::computeTileLightmapPixelAroundCorner(const CVector2f &stIn, CRGBA *dest, bool lookAround)
{
	bool	mustLookOnNeighbor= false;

	// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis).
	sint	u, v;
	u= (sint)floor(stIn.x*NL_LUMEL_BY_TILE);
	v= (sint)floor(stIn.y*NL_LUMEL_BY_TILE);

	// if allowed, try to go on neighbor patch.
	if(lookAround)
	{
		// try to know if we must go on a neighbor patch (maybe false with bind X/1).
		if( u<0 || u>=OrderS*NL_LUMEL_BY_TILE || v<0 || v>=OrderT*NL_LUMEL_BY_TILE)
			mustLookOnNeighbor= true;
	}


	// If we must get (if possible) the pixel in the current patch, do it.
	if(!mustLookOnNeighbor)
	{
		// if out this patch, abort.
		if( u<0 || u>=OrderS*NL_LUMEL_BY_TILE || v<0 || v>=OrderT*NL_LUMEL_BY_TILE)
			return;
		else
		{
			// get this pixel.
			computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
		}
	}
	// else get from the best neighbor patch.
	else
	{
		// choose against which edge we must find the pixel.
		uint	edge=0;
		if(u<0)					edge=0;
		else if(v>=OrderT*NL_LUMEL_BY_TILE)	edge=1;
		else if(u>=OrderS*NL_LUMEL_BY_TILE)	edge=2;
		else if(v<0)			edge=3;

		// retrieve info on neighbor.
		CBindInfo			bindInfo;
		getBindNeighbor(edge, bindInfo);

		// if neighbor present.
		if(bindInfo.Zone)
		{
			CVector2f	stOut;
			CPatch		*patchOut;
			uint		patchId;

			// Ok, search uv on this patch.
			CPatchUVLocator		uvLocator;
			uvLocator.build(this, edge, bindInfo);
			patchId= uvLocator.selectPatch(stIn);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);

			// retry only one time, so at next call, must find the data IN htis patch (else abort).
			patchOut->computeTileLightmapPixelAroundCorner(stOut, dest, false);
		}
	}
}


// ***************************************************************************
void		CPatch::computeNearBlockLightmap(uint uts, uint utt, CRGBA	*lightText)
{
	sint	ts= uts;
	sint	tt= utt;

	// hardcoded for 10x10.
	nlassert(NL_TILE_LIGHTMAP_SIZE==10);
	CRGBA	*dest;
	uint	edge;
	uint	corner;

	// Compute center of the TessBlock: the 2x2 tiles.
	//=================
	// compute tile 0,0 of the tessBlock. must decal of 1 pixel.
	dest= lightText+NL_TILE_LIGHTMAP_SIZE+1;
	computeTileLightmap(ts, tt, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 1,0 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts+1, tt, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 0,1 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts, tt+1, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 1,1 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE + NL_LUMEL_BY_TILE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts+1, tt+1, dest, NL_TILE_LIGHTMAP_SIZE);


	// Compute edges of the TessBlock.
	//=================
	bool	edgeBorder[4];
	// where are we on a border of a patch??
	edgeBorder[0]= ( ts==0 );
	edgeBorder[1]= ( tt == OrderT-2 );
	edgeBorder[2]= ( ts == OrderS-2 );
	edgeBorder[3]= ( tt==0 );

	// For all edges.
	for(edge=0; edge<4; edge++)
	{
		// compute dest info.
		//==============
		// Are we on a vertical edge or horizontal edge??
		uint	stride= (edge&1)==0? NL_TILE_LIGHTMAP_SIZE : 1;

		// must compute on which tile we must find info.
		sint	decalS=0;
		sint	decalT=0;
		// and must compute ptr, where we store the result of the edge.
		switch(edge)
		{
		case 0: decalS=-1; dest= lightText + 0 + NL_TILE_LIGHTMAP_SIZE; break;
		case 1: decalT= 2; dest= lightText + 1 + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 2: decalS= 2; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + NL_TILE_LIGHTMAP_SIZE; break;
		case 3: decalT=-1; dest= lightText + 1; break;
		};

		// compute the second tile dest info.
		CRGBA	*dest2;
		sint	decalS2;
		sint	decalT2;
		// if vertical edge.
		if((edge&1)==0)
		{
			// Next Y tile.
			dest2= dest + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE;
			decalS2= decalS;
			decalT2= decalT+1;
		}
		else
		{
			// Next X tile.
			dest2= dest + NL_LUMEL_BY_TILE;
			decalS2= decalS+1;
			decalT2= decalT;
		}


		// If we are not on a border of a patch, just compute on the interior of the patch.
		//==============
		if(!edgeBorder[edge])
		{
			// find the result on the mirrored border of us. First tile.
			computeTileLightmapEdge(ts+decalS, tt+decalT, (edge+2)&3, dest, stride, false);

			// find the result on the mirrored border of us. Second Tile.
			computeTileLightmapEdge(ts+decalS2, tt+decalT2, (edge+2)&3, dest2, stride, false);

		}
		// else, slightly complicated, must find the result on neighbor patch(s).
		//==============
		else
		{
			CPatchUVLocator		uvLocator;
			CBindInfo			bindInfo;
			bindInfo.Zone= NULL;

			// if smoothed edge, search the neighbor.
			if(getSmoothFlag(edge))
			{
				// Build the bindInfo against this edge.
				getBindNeighbor(edge, bindInfo);

				// if ok, build the uv info against this edge.
				if(bindInfo.Zone)
				{
					uvLocator.build(this, edge, bindInfo);
					// if there is not same tile order across the edge, invalidate the smooth.
					// This is rare, so don't bother.
					if(!uvLocator.sameEdgeOrder())
						bindInfo.Zone= NULL;
				}
			}


			// Fast reject: if no neighbor, or if not smoothed, or if edge order pb, just copy from my interior.
			if(!bindInfo.Zone)
			{
				CRGBA	*src=0;
				switch(edge)
				{
				case 0: src= dest + 1; break;
				case 1: src= dest - NL_TILE_LIGHTMAP_SIZE; break;
				case 2: src= dest - 1; break;
				case 3: src= dest + NL_TILE_LIGHTMAP_SIZE; break;
				};

				// fill the NL_LUMEL_BY_TILE*2 (8) pixels.
				for(uint n=NL_LUMEL_BY_TILE*2; n>0; n--, src+=stride, dest+=stride)
					*dest= *src;
			}
			// else, ok, get from neighbor.
			else
			{
				CVector2f	stIn, stOut;
				CPatch		*patchOut;
				uint		patchId;
				uint		edgeOut;
				bool		inverse;

				// First Tile.
				//=========
				// to remove floor pbs, take the center of the wanted tile.
				stIn.set(ts+decalS + 0.5f, tt+decalT + 0.5f);
				patchId= uvLocator.selectPatch(stIn);
				uvLocator.locateUV(stIn, patchId, patchOut, stOut);
				// must find what edge on neighbor to compute, and if we must inverse (swap) result.
				// easy: the edge of the tile is the edge of the patch where we are binded.
				edgeOut= bindInfo.Edge[patchId];
				// edge0 is oriented in T increasing order. edge1 is oriented in S increasing order.
				// edge2 is oriented in T decreasing order. edge3 is oriented in S decreasing order.
				// inverse is true if same sens on both edges (because of mirroring, sens should be different).
				inverse= (edge>>1)==(edgeOut>>1);
				// compute the lightmap on the edge of the neighbor.
				patchOut->computeTileLightmapEdge((sint)floor(stOut.x), (sint)floor(stOut.y), edgeOut, dest, stride, inverse);

				// Second Tile.
				//=========
				// same reasoning.
				stIn.set(ts+decalS2 + 0.5f, tt+decalT2 + 0.5f);
				patchId= uvLocator.selectPatch(stIn);
				uvLocator.locateUV(stIn, patchId, patchOut, stOut);
				edgeOut= bindInfo.Edge[patchId];
				inverse= (edge>>1)==(edgeOut>>1);
				patchOut->computeTileLightmapEdge((sint)floor(stOut.x), (sint)floor(stOut.y), edgeOut, dest2, stride, inverse);
			}

		}
	}


	// Compute corners of the TessBlock.
	//=================
	bool	cornerOnPatchEdge[4];
	bool	cornerOnPatchCorner[4];
	// where are we on a edge border of a patch??
	cornerOnPatchEdge[0]= edgeBorder[3] != edgeBorder[0];
	cornerOnPatchEdge[1]= edgeBorder[0] != edgeBorder[1];
	cornerOnPatchEdge[2]= edgeBorder[1] != edgeBorder[2];
	cornerOnPatchEdge[3]= edgeBorder[2] != edgeBorder[3];
	// where are we on a corner border of a patch??
	cornerOnPatchCorner[0]= edgeBorder[3] && edgeBorder[0];
	cornerOnPatchCorner[1]= edgeBorder[0] && edgeBorder[1];
	cornerOnPatchCorner[2]= edgeBorder[1] && edgeBorder[2];
	cornerOnPatchCorner[3]= edgeBorder[2] && edgeBorder[3];

	// For all corners.
	for(corner=0; corner<4; corner++)
	{
		// compute dest info.
		//==============
		// must compute on which tile we must find info.
		sint	decalS=0;
		sint	decalT=0;
		// and must compute ptr, where we store the result of the corner.
		switch(corner)
		{
		case 0: decalS=-1; decalT=-1; dest= lightText + 0 + 0; break;
		case 1: decalS=-1; decalT= 2; dest= lightText + 0 + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 2: decalS= 2; decalT= 2; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 3: decalS= 2; decalT=-1; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + 0; break;
		};


		// If we are not on a border of a patch, just compute on the interior of the patch.
		//==============
		// if the corner is IN the patch.
		if(!cornerOnPatchCorner[corner] && !cornerOnPatchEdge[corner])
		{
			// what pixel to read.
			uint	subS, subT;
			if(decalS==-1)	subS= NL_LUMEL_BY_TILE-1;
			else			subS= 0;
			if(decalT==-1)	subT= NL_LUMEL_BY_TILE-1;
			else			subT= 0;

			// find the result on the corner of the neighbor tile.
			computeTileLightmapPixel(ts+decalS, tt+decalT, subS, subT, dest);
		}
		else
		{
			// By default, fill the corner with our interior corner. Because other methods may fail.
			CRGBA	*src=0;
			switch(corner)
			{
			case 0: src= dest + 1 + NL_TILE_LIGHTMAP_SIZE; break;
			case 1: src= dest + 1 - NL_TILE_LIGHTMAP_SIZE; break;
			case 2: src= dest - 1 - NL_TILE_LIGHTMAP_SIZE; break;
			case 3: src= dest - 1 + NL_TILE_LIGHTMAP_SIZE; break;
			};

			// fill the pixel.
			*dest= *src;

			// get the coordinate of the corner, in our [0,Order] basis. get it at the center of the pixel.
			CBindInfo			bindInfo;
			CPatchUVLocator		uvLocator;
			CVector2f			stIn, stOut;
			CPatch				*patchOut;
			uint				patchId;
			float				decX, decY;
			static const float	lumelSize= 1.f/NL_LUMEL_BY_TILE;
			static const float	semiLumelSize= 0.5f*lumelSize;

			if(decalS==-1)	decX= -  semiLumelSize;
			else			decX= 2+ semiLumelSize;
			if(decalT==-1)	decY= -  semiLumelSize;
			else			decY= 2+ semiLumelSize;
			stIn.set( ts+decX, tt+decY);


			// if the corner is on One edge only of the patch.
			if(cornerOnPatchEdge[corner])
			{
				// find the edge where to read this corner: hard edge after or before this corner.
				if(edgeBorder[corner])	edge= corner;
				else					edge= (corner+4-1) & 3;

				// if this edge is smoothed, find on neighbor.
				if(getSmoothFlag(edge))
				{
					// retrieve neigbhor info.
					getBindNeighbor(edge, bindInfo);

					// if neighbor present.
					if(bindInfo.Zone)
					{
						// Ok, search uv on this patch.
						uvLocator.build(this, edge, bindInfo);
						patchId= uvLocator.selectPatch(stIn);
						uvLocator.locateUV(stIn, patchId, patchOut, stOut);

						// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis), and get from neighbor patch
						sint	u, v;
						u= (sint)floor(stOut.x*NL_LUMEL_BY_TILE);
						v= (sint)floor(stOut.y*NL_LUMEL_BY_TILE);
						patchOut->computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
					}
				}
				// else we must still smooth with our lumel on this patch, so get it from neighbor on edge.
				else
				{
					// first, clamp to our patch (recenter on the previous pixel)
					if(stIn.x<0)			stIn.x+= lumelSize;
					else if(stIn.x>OrderS)	stIn.x-= lumelSize;
					else if(stIn.y<0)		stIn.y+= lumelSize;
					else if(stIn.y>OrderT)	stIn.y-= lumelSize;

					// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis), and get from this patch
					sint	u, v;
					u= (sint)floor(stIn.x*NL_LUMEL_BY_TILE);
					v= (sint)floor(stIn.y*NL_LUMEL_BY_TILE);
					computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
				}
			}
			// else it is on a corner of the patch.
			else
			{
				// if the corner of the patch (same as tile corner) is smoothed, find on neighbor
				if(getCornerSmoothFlag(corner))
				{
					// retrieve neigbhor info. NB: use edgeId=corner, (corner X is the start of the edge X)it works.
					getBindNeighbor(corner, bindInfo);

					// if neighbor present.
					if(bindInfo.Zone)
					{
						// Ok, search uv on this patch.
						uvLocator.build(this, corner, bindInfo);
						patchId= uvLocator.selectPatch(stIn);
						uvLocator.locateUV(stIn, patchId, patchOut, stOut);

						// same reasoning as in computeDisplaceCornerSmooth(), must find the pixel on the neighbor
						// of our neighbor. But the current corner may be a corner on a bind X/1. All is managed by doing
						// this way.
						patchOut->computeTileLightmapPixelAroundCorner(stOut, dest, true);
					}
				}
			}
		}

	}


}


// ***************************************************************************
void		CPatch::getTileLightMap(uint ts, uint tt, CPatchRdrPass *&rdrpass)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// If the lightmap Id has not been computed, compute it.
	if(tessBlock.LightMapRefCount==0)
	{
		// Compute the lightmap texture, with help of TileColors, with neighboring info etc...
		CRGBA	lightText[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE];
		computeNearBlockLightmap(ts&(~1), tt&(~1), lightText);

		// Create a rdrPass with this texture, donlod to Driver etc...
		tessBlock.LightMapId= Zone->Landscape->getTileLightMap(lightText, rdrpass);

		// store this rdrpass ptr.
		tessBlock.LightMapRdrPass= rdrpass;
	}

	// We are using this 2x2 tiles lightmap.
	tessBlock.LightMapRefCount++;


	// get the rdrpass ptr of the tessBlock lightmap
	rdrpass= tessBlock.LightMapRdrPass;
}


// ***************************************************************************
void		CPatch::getTileLightMapUvInfo(uint ts, uint tt, CVector &uvScaleBias)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// Get the uvScaleBias for the tile 0,0  of the block.
	Zone->Landscape->getTileLightMapUvInfo(tessBlock.LightMapId, uvScaleBias);

	// Must increment the bias, for the good tile in the 2x2 block Lightmap.
	uint	tsDec= ts & 1;
	uint	ttDec= tt & 1;
	uvScaleBias.x+= tsDec * uvScaleBias.z;
	uvScaleBias.y+= ttDec * uvScaleBias.z;
}


// ***************************************************************************
void		CPatch::releaseTileLightMap(uint ts, uint tt)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// If no more tileMaterial use this lightmap, release it.
	nlassert(tessBlock.LightMapRefCount>0);
	tessBlock.LightMapRefCount--;
	if(tessBlock.LightMapRefCount==0)
	{
		Zone->Landscape->releaseTileLightMap(tessBlock.LightMapId);
	}
}

// ***************************************************************************
void		CPatch::packShadowMap (const uint8 *pLumelSrc)
{
	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Resize the compressed buffer
	CompressedLumels.resize (numLumelBlock*numLineBlock*NL_BLOCK_LUMEL_COMPRESSED_SIZE);

	// Input of compresed data
	uint8 *compressedData=&CompressedLumels[0];

	// Each line block
	for (uint lineBlock=0; lineBlock<numLineBlock; lineBlock++)
	{
		// Block pointer
		const uint8 *blockLine=pLumelSrc;

		// Each lumel block
		for (uint lumelBlock=0; lumelBlock<numLumelBlock; lumelBlock++)
		{
			// *** Unpack the block
			uint countU;

			// Last block ?
			if (lumelBlock==numLumelBlock-1)
				countU=lumelCount&3;
			else
				countU=4;

			// Destination lumel
			const uint8 *blockSrc=blockLine;

			// Temp block
			uint8 originalBlock[4*4];

			// Copy the lumels in the bloc
			for (uint v=0; v<NL_LUMEL_BY_TILE; v++)
			{
				for (uint u=0; u<NL_LUMEL_BY_TILE; u++)
				{
					// Copy the lumel
					originalBlock[(v<<2)+u]=blockSrc[u];
				}

				// Next line
				blockSrc+=lumelCount;
			}

			// Get min and max alpha
			uint8 alphaMin=255;
			uint8 alphaMax=0;

			// Scan
			for (uint i=0; i<16; i++)
			{
				// Min ?
				if (originalBlock[i]<alphaMin)
					alphaMin=originalBlock[i];
				if (originalBlock[i]>alphaMax)
					alphaMax=originalBlock[i];
			}

			// *** Try to compress by 2 methods

			// Blcok uncompressed
			uint8 uncompressedBlock[4*4];

			// Pack the block
			packLumelBlock (compressedData, originalBlock, alphaMin, alphaMax);

			// Unpack the block
			unpackLumelBlock (uncompressedBlock, compressedData);

			// Eval error
			uint firstMethod=evalLumelBlock (originalBlock, uncompressedBlock, NL_LUMEL_BY_TILE, NL_LUMEL_BY_TILE);

			// second compression
			uint8 secondCompressedBlock[NL_BLOCK_LUMEL_COMPRESSED_SIZE];
			packLumelBlock (secondCompressedBlock, originalBlock, alphaMax, alphaMin);

			// Unpack the block
			unpackLumelBlock (uncompressedBlock, secondCompressedBlock);

			// Eval error
			uint secondMethod=evalLumelBlock (originalBlock, uncompressedBlock, NL_LUMEL_BY_TILE, NL_LUMEL_BY_TILE);

			// Second best ?
			if (secondMethod<firstMethod)
			{
				// Copy compressed data
				memcpy (compressedData, secondCompressedBlock, NL_BLOCK_LUMEL_COMPRESSED_SIZE);
			}

			// Next source block
			compressedData+=NL_BLOCK_LUMEL_COMPRESSED_SIZE;

			// Next block on the line
			blockLine+=4;
		}

		// Next line of block
		pLumelSrc+=lumelCount*4;
	}
}

// ***************************************************************************
void		CPatch::resetCompressedLumels ()
{
	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Size of the lumel array
	uint size=numLineBlock*numLumelBlock*8;

	// 4 bits per lumel
	CompressedLumels.resize (size);

	// No line have shadows.
	memset (&CompressedLumels[0], 0, size);
}


// ***************************************************************************
// ***************************************************************************
// Functions (C/ASM).
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#define		a00	tex[0]
#define		a10	tex[1]
#define		a20	tex[2]
#define		a30	tex[3]
#define		a40	tex[4]

#define		a01	tex[5]
#define		a11	tex[6]
#define		a21	tex[7]
#define		a31	tex[8]
#define		a41	tex[9]

#define		a02	tex[10]
#define		a12	tex[11]
#define		a22	tex[12]
#define		a32	tex[13]
#define		a42	tex[14]

#define		a03	tex[15]
#define		a13	tex[16]
#define		a23	tex[17]
#define		a33	tex[18]
#define		a43	tex[19]

#define		a04	tex[20]
#define		a14	tex[21]
#define		a24	tex[22]
#define		a34	tex[23]
#define		a44	tex[24]

void	NL3D_bilinearTileLightMap(CRGBA *tex)
{
	// Fast bilinear of a 5x5 tile.
	// Corners must be set.
	// Later: pass it to ASM.

	// Fill first column 0 and column 4.
	a02.avg2(a00, a04);
	a01.avg2(a00, a02);
	a03.avg2(a02, a04);
	a42.avg2(a40, a44);
	a41.avg2(a40, a42);
	a43.avg2(a42, a44);

	// Fill Line 0.
	a20.avg2(a00, a40);
	a10.avg2(a00, a20);
	a30.avg2(a20, a40);

	// Fill Line 1.
	a21.avg2(a01, a41);
	a11.avg2(a01, a21);
	a31.avg2(a21, a41);

	// Fill Line 2.
	a22.avg2(a02, a42);
	a12.avg2(a02, a22);
	a32.avg2(a22, a42);

	// Fill Line 3.
	a23.avg2(a03, a43);
	a13.avg2(a03, a23);
	a33.avg2(a23, a43);

	// Fill Line 4.
	a24.avg2(a04, a44);
	a14.avg2(a04, a24);
	a34.avg2(a24, a44);

}


// ***************************************************************************
// ***************************************************************************
// Lightmap get interface.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
uint8		CPatch::getLumel(const CUV &uv) const
{
	// compute tile coord and lumel coord.
	sint	ts, tt;
	// get in lumel coord.
	sint	w= (OrderS<<NL_LUMEL_BY_TILE_SHIFT);
	sint	h= (OrderT<<NL_LUMEL_BY_TILE_SHIFT);
	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	// add 128, to round and get cneter of lumel.
	ts= NLMISC::OptFastFloor(uv.U* (w<<8) + 128);	ts>>=8;
	tt= NLMISC::OptFastFloor(uv.V* (h<<8) + 128);	tt>>=8;
	clamp(ts, 0, w-1);
	clamp(tt, 0, h-1);
	// get the lumel
	uint8	ret;
	getTileLumelmapPixelPrecomputed(ts>>NL_LUMEL_BY_TILE_SHIFT, tt>>NL_LUMEL_BY_TILE_SHIFT,
		ts&(NL_LUMEL_BY_TILE-1), tt&(NL_LUMEL_BY_TILE-1), ret);

	return ret;
}


// ***************************************************************************
// ***************************************************************************
// TileLightInfluences
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::resetTileLightInfluences()
{
	// Fill default.
	TileLightInfluences.resize((OrderS/2 +1) * (OrderT/2 +1));
	// Disable All light influence on all points
	for(uint i=0;i <TileLightInfluences.size(); i++)
	{
		// Disable all light influence on this point.
		TileLightInfluences[i].Light[0]= 0xFF;
		TileLightInfluences[i].Light[1]= 0xFF;
	}
}


// ***************************************************************************
void		CPatch::appendTileLightInfluences(const CUV &uv,
	std::vector<CPointLightInfluence> &pointLightList) const
{
	/*
		WARNING !! only CPointLightNamed must be added here (used for convenience by CPatch::generateTileVegetable() )
	*/

	// Compute TLI coord for BiLinear.
	sint	x,y;
	// There is (OrderS/2+1) * (OrderT/2+1) tileLightInfluences (TLI).
	sint	w= (OrderS>>1);
	sint	h= (OrderT>>1);
	sint	wTLI= w+1;
	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	x= NLMISC::OptFastFloor(uv.U * (w<<8));
	y= NLMISC::OptFastFloor(uv.V * (h<<8));
	clamp(x, 0, w<<8);
	clamp(y, 0, h<<8);
	// compute the TLI coord, and the subCoord for bilinear.
	sint	xTLI,yTLI, xSub, ySub;
	xTLI= x>>8; clamp(xTLI, 0, w-1);
	yTLI= y>>8; clamp(yTLI, 0, h-1);
	// Hence, xSub and ySub range is [0, 256].
	xSub= x - (xTLI<<8);
	ySub= y - (yTLI<<8);


	// Use a CLightInfluenceInterpolator to biLinear light influence
	CLightInfluenceInterpolator		interp;
	// Must support only 2 light per TLI.
	nlassert(CTileLightInfluence::NumLightPerCorner==2);
	nlassert(CLightInfluenceInterpolator::NumLightPerCorner==2);
	// Get ref on array of PointLightNamed.
	CPointLightNamed	*zonePointLights= NULL;
	if( getZone()->_PointLightArray.getPointLights().size() >0 )
	{
		// const_cast, because will only change _IdInfluence, and
		// also because CLightingManager will call appendLightedModel()
		zonePointLights= const_cast<CPointLightNamed*>(&(getZone()->_PointLightArray.getPointLights()[0]));
	}
	// For 4 corners.
	for(y=0;y<2;y++)
	{
		for(x=0;x<2;x++)
		{
			// get ref on TLI, and on corner.
			const CTileLightInfluence				&tli= TileLightInfluences[ (yTLI+y)*wTLI + xTLI+x ];
			CLightInfluenceInterpolator::CCorner	&corner= interp.Corners[y*2 + x];
			// For all lights
			uint lid;
			for(lid= 0; lid<CTileLightInfluence::NumLightPerCorner; lid++)
			{
				// get the id of the light in the zone
				uint	tliLightId= tli.Light[lid];
				// If empty id, stop
				if(tliLightId==0xFF)
					break;
				else
				{
					// Set pointer of the light in the corner
					corner.Lights[lid]= zonePointLights + tliLightId;
				}
			}
			// Reset Empty slots.
			for(; lid<CTileLightInfluence::NumLightPerCorner; lid++)
			{
				// set to NULL
				corner.Lights[lid]= NULL;
			}
		}
	}
	// interpolate.
	interp.interpolate(pointLightList, xSub/256.f, ySub/256.f);
}


// ***************************************************************************
CRGBA		CPatch::getCurrentTLIColor(uint x, uint y) const
{
	CRGBA	ret;
	ret= CRGBA::Black;

	// if at least the zone has pointLights, add them.
	if( getZone()->_PointLightArray.getPointLights().size() >0 )
	{
		const CPointLightNamed	*zonePointLights;
		zonePointLights= (&(getZone()->_PointLightArray.getPointLights()[0]));

		uint	wTLI= (OrderS>>1)+1;

		const CTileLightInfluence	&tli= TileLightInfluences[ y*wTLI + x];
		for(uint lid=0;lid<CTileLightInfluence::NumLightPerCorner;lid++)
		{
			// Not influenced by a pointLight?, stop
			if(tli.Light[lid]==0xFF)
				break;
			// Append the influence of this pointLight. NB: use unanimated version.
			CRGBA	lightCol= zonePointLights[tli.Light[lid]].getUnAnimatedDiffuse();
			// modulate with landscape Material.
			lightCol.modulateFromColorRGBOnly(lightCol, getLandscape()->getPointLightDiffuseMaterial() );
			// modulate with precomputed diffuse factor
			lightCol.modulateFromuiRGBOnly(lightCol, tli.getDiffuseLightFactor(lid) );
			// add to the corner
			ret.addRGBOnly(ret, lightCol);
		}
	}

	return ret;
}


// ***************************************************************************
void		CPatch::getCurrentTileTLIColors(uint ts, uint tt, NLMISC::CRGBA corners[4])
{
	// Get ref on array of PointLightNamed.
	if( getZone()->_PointLightArray.getPointLights().size() >0 )
	{
		// get coord of the tessBlock
		uint	tbs= ts>>1;
		uint	tbt= tt>>1;
		// get tile id local to tessBlock.
		uint	tls= ts-(tbs<<1);
		uint	tlt= tt-(tbt<<1);

		// For each corner of the tessBlock, compute lighting with pointLights.
		CRGBA	tbCorners[4];
		for(uint y=0;y<2;y++)
		{
			for(uint x=0;x<2;x++)
			{
				CRGBA	&cornerCol= tbCorners[y*2+x];
				cornerCol= getCurrentTLIColor(tbs+x, tbt+y);
			}
		}

		// Then biLinear to tile Level (tessBlock==2x2 tiles).
		CRGBA	tbEdges[4];
		CRGBA	tbMiddle;
		// left.
		tbEdges[0].avg2RGBOnly(tbCorners[0], tbCorners[2]);
		tbEdges[0].A = 255;
		// bottom
		tbEdges[1].avg2RGBOnly(tbCorners[2], tbCorners[3]);
		tbEdges[1].A = 255;
		// right
		tbEdges[2].avg2RGBOnly(tbCorners[1], tbCorners[3]);
		tbEdges[2].A = 255;
		// up
		tbEdges[3].avg2RGBOnly(tbCorners[0], tbCorners[1]);
		tbEdges[3].A = 255;
		// middle.
		tbMiddle.avg2RGBOnly(tbEdges[0], tbEdges[2]);
		tbMiddle.A = 255;

		// just copy result according to tile pos in tessBlock.
		if(tlt==0)
		{
			if(tls==0)
			{
				corners[0]= tbCorners[0];
				corners[1]= tbEdges[3];
				corners[2]= tbEdges[0];
				corners[3]= tbMiddle;
			}
			else
			{
				corners[0]= tbEdges[3];
				corners[1]= tbCorners[1];
				corners[2]= tbMiddle;
				corners[3]= tbEdges[2];
			}
		}
		else
		{
			if(tls==0)
			{
				corners[0]= tbEdges[0];
				corners[1]= tbMiddle;
				corners[2]= tbCorners[2];
				corners[3]= tbEdges[1];
			}
			else
			{
				corners[0]= tbMiddle;
				corners[1]= tbEdges[2];
				corners[2]= tbEdges[1];
				corners[3]= tbCorners[3];
			}
		}
	}
	else
	{
		// Just fill with 0s.
		corners[0]= CRGBA::Black;
		corners[1]= CRGBA::Black;
		corners[2]= CRGBA::Black;
		corners[3]= CRGBA::Black;
	}
}

// ***************************************************************************
void		CPatch::addTileLightmapWithTLI(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride)
{
	// compute colors ar corners of the tile.
	CRGBA	corners[4];
	getCurrentTileTLIColors(ts, tt, corners);

	// Bilinear accross the tile, and add to dest.
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// compute this pixel, and add
			bilinearColorDiv2AndAdd(corners, x, y, dest[y*stride + x]);
		}
	}
}

// ***************************************************************************
void		CPatch::addTileLightmapEdgeWithTLI(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse)
{
	// compute colors ar corners of the tile.
	CRGBA	corners[4];
	getCurrentTileTLIColors(ts, tt, corners);

	// get coordinate according to edge.
	uint	x=0,y=0;
	switch(edge)
	{
	case 0: x= 0; break;
	case 1: y= NL_LUMEL_BY_TILE-1; break;
	case 2: x= NL_LUMEL_BY_TILE-1; break;
	case 3: y= 0; break;
	};

	// For all lumel of the edge, bilinear.
	uint	i;
	for(i=0; i<NL_LUMEL_BY_TILE; i++)
	{
		// if vertical edge
		if( (edge&1)==0 )	y= i;
		// else horizontal edge
		else				x= i;

		// manage inverse.
		uint	where;
		if(inverse)		where= (NL_LUMEL_BY_TILE-1)-i;
		else			where= i;
		// compute this pixel, and modulate
		bilinearColorDiv2AndAdd(corners, x, y, dest[where*stride]);
	}
}

// ***************************************************************************
void		CPatch::addTileLightmapPixelWithTLI(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest)
{
	// compute colors ar corners of the tile.
	CRGBA	corners[4];
	getCurrentTileTLIColors(ts, tt, corners);

	// compute this pixel, and modulate
	bilinearColorDiv2AndAdd(corners, s, t, *dest);
}


// ***************************************************************************
void		CPatch::computeCurrentTLILightmapDiv2(NLMISC::CRGBA *array) const
{
	// Size of TileLightInfluences
	uint	wTLI= (OrderS>>1)+1;
	uint	hTLI= (OrderT>>1)+1;
	// colros at corners of tiles size.
	uint	wTC= OrderS+1;
	uint	wTCx2= wTC*2;
	uint	hTC= OrderT+1;
	uint	x, y;

	// Compute TLI colors at each corner of each TessBlocks.
	//=============
	for(y=0;y<hTLI;y++)
	{
		// store every 2 tiles corners.
		CRGBA	*dst= array + y*2*wTC;
		for(x=0;x<wTLI;x++)
		{
			*dst= getCurrentTLIColor(x, y);
			dst->R >>= 1;
			dst->G >>= 1;
			dst->B >>= 1;
			// skip 2 tiles corners.
			dst++;
			dst++;
		}
	}

	// Compute TLI colors at each corner of each Tiles.
	//=============

	// Compute corner at middle of vertical TessBlock edges.
	for(y=0;y<hTC-1;y+=2)
	{
		CRGBA	*dst= array + y*wTC;
		for(x=0;x<wTC;x+=2)
		{
			// Average midlle with cur and next.
			(dst+wTC)->avg2RGBOnly(*dst, *(dst + wTCx2) );

			// skip 2 tiles corners.
			dst++;
			dst++;
		}
	}

	// Compute corner at middle of horizontal TessBlock edges, and at middle of TessBlock.
	for(y=0;y<hTC;y++)
	{
		CRGBA	*dst= array + y*wTC;
		for(x=0;x<wTC-1;x+=2)
		{
			// Average midlle with cur and next.
			(dst+1)->avg2RGBOnly(*dst, *(dst+2));

			// skip 2 tiles corners.
			dst++;
			dst++;
		}
	}


}



// ***************************************************************************
// ***************************************************************************
// UpdateLighting.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CPatch::linkBeforeNearUL(CPatch *patchNext)
{
	nlassert(patchNext);

	// first, unlink others from me. NB: works even if _ULNearPrec==_ULNearNext==this.
	_ULNearNext->_ULNearPrec= _ULNearPrec;
	_ULNearPrec->_ULNearNext= _ULNearNext;
	// link to igNext.
	_ULNearNext= patchNext;
	_ULNearPrec= patchNext->_ULNearPrec;
	// link others to me.
	_ULNearNext->_ULNearPrec= this;
	_ULNearPrec->_ULNearNext= this;
}


// ***************************************************************************
void CPatch::unlinkNearUL()
{
	// first, unlink others from me. NB: works even if _ULNearPrec==_ULNearNext==this.
	_ULNearNext->_ULNearPrec= _ULNearPrec;
	_ULNearPrec->_ULNearNext= _ULNearNext;
	// reset
	_ULNearPrec= this;
	_ULNearNext= this;
}


// ***************************************************************************
uint CPatch::updateTessBlockLighting(uint numTb)
{
	// TessBlocks must have been allocated and tessBlockId must be ok.
	nlassert(numTb<TessBlocks.size());

	// compute tessBlock coordinate
	uint tbWidth= OrderS>>1;
	uint ts= numTb&(tbWidth-1);
	uint tt= numTb/tbWidth;
	// expand to tile coordinate.
	ts*= 2;
	tt*= 2;

	// get what tessBlock to use.
	CTessBlock	&tessBlock= TessBlocks[numTb];

	// If the lightmap Id has not been computed, quit
	if(tessBlock.LightMapRefCount==0)
		return 0;
	else
	{
		// Recompute the lightmap texture, with help of TileColors, with neighboring info etc...
		CRGBA	lightText[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE];
		computeNearBlockLightmap(ts&(~1), tt&(~1), lightText);

		// donlod this texture to Driver etc...
		Zone->Landscape->refillTileLightMap(tessBlock.LightMapId, lightText);

		// return number of pixels computed.
		return	NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE;
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void				CPatch::addRefDLMContext()
{
	// the patch must be compiled.
	nlassert(Zone);

	// if 0, create the context.
	if(_DLMContextRefCount==0)
	{
		nlassert(_DLMContext==NULL);
		_DLMContext= new CPatchDLMContext;
		// init now the context.
		_DLMContext->generate(this, getLandscape()->getTextureDLM(), getLandscape()->getPatchDLMContextList());

		// If the patch is visible, it may have Far Vertices created,
		// hence, we must refill them with good DLM Uvs.
		if(!isRenderClipped())
		{
			// setup DLM Uv with new _DLMContext
			fillVBFarsDLMUvOnly();
		}
	}

	// incRef.
	_DLMContextRefCount++;
}

// ***************************************************************************
void				CPatch::decRefDLMContext(uint count)
{
	// the patch must be compiled.
	nlassert(Zone);
	nlassert(_DLMContextRefCount>0);

	// dec Ref.
	_DLMContextRefCount-= count;
	nlassert(_DLMContextRefCount>=0);

	// If 0, delete the context.
	if(_DLMContextRefCount==0)
	{
		delete _DLMContext;
		_DLMContext= NULL;

		// If the patch is visible, it may have Far Vertices created,
		// hence, we must reset their DLM Uvs (to point to black pixel)
		if(!isRenderClipped())
		{
			// setup DLM Uv with new _DLMContext
			fillVBFarsDLMUvOnly();
		}
	}
}


// ***************************************************************************
void		CPatch::beginDLMLighting()
{
	nlassert(_DLMContext);

	// Must bkup prec pointLightCount in OldPointLightCount, and reset CurPointLightCount
	_DLMContext->OldPointLightCount= _DLMContext->CurPointLightCount;
	_DLMContext->CurPointLightCount= 0;

	// clear lighting, only if patch is visible
	if(!isRenderClipped())
		// NB: no-op if src is already full black.
		_DLMContext->clearLighting();

}

// ***************************************************************************
void		CPatch::processDLMLight(CPatchDLMPointLight &pl)
{
	// add reference to currentLight, creating DLMContext if needed
	addRefDLMContext();

	// add curLight counter
	_DLMContext->CurPointLightCount++;

	// compute lighting, only if patch is visible
	if(!isRenderClipped())
		_DLMContext->addPointLightInfluence(pl);
}

// ***************************************************************************
void		CPatch::endDLMLighting()
{
	nlassert(_DLMContext);

	// delete reference from old pointLight influences, at prec render() pass. _DLMContext may be deleted here,
	// if no more lights use it, and if the patch is not in Near.
	decRefDLMContext(_DLMContext->OldPointLightCount);
}


} // NL3D

