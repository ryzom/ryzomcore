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
#include "nel/3d/tile_noise_map.h"
#include "nel/3d/patchuv_locator.h"
using	namespace	std;
using	namespace	NLMISC;


namespace NL3D
{


// ***************************************************************************


#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM) && defined(NL_USE_FASTFLOOR)

/* This floor works only for floor with noise, because floor/ceil are only made on decimal coordinates:
	sTile =1.25 ....  NB: because of difference of mapping (rare case), we may have sometimes values with
	precision < 1/4 (eg 1.125). Just use f*256 to compute the floor.

  NB: using a fastFloor() (fistp changing the controlfp() is not very a good idea here, because
  computeNoise() are not "packed", so change on controlFp() would bee too frequent...
  And also because we need either floor() or ceil() here.
*/
inline	sint noiseFloor(float f)
{
	// build a fixed 24:8.
	sint	a;
	f*=256;

	// fast ftol. work if no decimal.
	_asm
	{
		fld f
		fistp a
	}

	// floor.
	a>>=8;

	return a;
}


inline	sint noiseCeil(float f)
{
	// build a fixed 24:8.
	sint	a;
	f*=256;

	// fast ftol. work if no decimal.
	_asm
	{
		fld f
		fistp a
	}

	// ceil.
	a+=255;
	a>>=8;

	return a;
}


inline	float noiseFloorF(float f)
{
	return (float)noiseFloor(f);
}
inline	float noiseCeilF(float f)
{
	return (float)noiseCeil(f);
}


#else


inline	float noiseFloorF(float f)
{
	return (float)floor(f);
}
inline	float noiseCeilF(float f)
{
	return (float)ceil(f);
}

inline	sint noiseFloor(float f)
{
	return (sint)floor(f);
}
inline	sint noiseCeil(float f)
{
	return (sint)ceil(f);
}

#endif



// ***************************************************************************
float		CPatch::computeDisplaceRawInteger(sint ts, sint tt, sint ms, sint mt) const
{
	// Choose the noiseMap.
	// ===============================
	clamp(ts, 0, OrderS-1);
	clamp(tt, 0, OrderT-1);

	uint	tileId= tt*OrderS + ts;
	// Get the tile for pass0. This is the principal tile, and this one tells what noise to take.
	sint	tileNumber= Tiles[tileId].Tile[0];
	// Get the subNoise from tileElement.
	uint	tileSubNoise= Tiles[tileId].getTileSubNoise();

	// retrieve the wanted noiseMap.
	CTileNoiseMap	*noiseMap;
	noiseMap = getZone()->getLandscape()->TileBank.getTileNoiseMap (tileNumber, tileSubNoise);

	if (noiseMap == NULL)
		return 0.0f;

	// Sample the noiseMap with (s,t).
	// ===============================

	// sample from map.
	sint8	pix= noiseMap->Pixels[mt*NL3D_TILE_NOISE_MAP_SIZE + ms];

	// normalize.
	return  (float)pix * (NL3D_NOISE_MAX / 127.f);
}


// ***************************************************************************
void		CPatch::computeDisplaceRawCoordinates(float sTile, float tTile, float s, float t,
	sint &ts, sint &tt, sint &ms, sint &mt) const
{
	// Choose the noiseMap.
	// ===============================
	// Compute coordinate in the patch.
	ts= noiseFloor(sTile);
	tt= noiseFloor(tTile);


	// Sample the noiseMap with (s,t).
	// ===============================

	// scale the map.
	float	u= s * NL3D_TILE_NOISE_MAP_TILE_FACTOR;
	float	v= t * NL3D_TILE_NOISE_MAP_TILE_FACTOR;

	// Speed rotation.
	CUV		uv;
	switch(NoiseRotation & 3)
	{
		case 0:
			uv.U= u;
			uv.V= v;
			break;
		case 1:
			uv.U= NL3D_TILE_NOISE_MAP_SIZE-v;
			uv.V= u;
			break;
		case 2:
			uv.U= NL3D_TILE_NOISE_MAP_SIZE-u;
			uv.V= NL3D_TILE_NOISE_MAP_SIZE-v;
			break;
		case 3:
			uv.U= v;
			uv.V= NL3D_TILE_NOISE_MAP_SIZE-u;
			break;
	}

	// direct map (no bilinear, no round, the case where s,t < 1/4 of a tile is very rare).
	ms= noiseFloor(uv.U);
	mt= noiseFloor(uv.V);

	// Manage Tiling (add NL3D_TILE_NOISE_MAP_SIZE*1 should be sufficient, but take margin).
	ms= (ms + (NL3D_TILE_NOISE_MAP_SIZE*256)) & (NL3D_TILE_NOISE_MAP_SIZE-1);
	mt= (mt + (NL3D_TILE_NOISE_MAP_SIZE*256)) & (NL3D_TILE_NOISE_MAP_SIZE-1);
}



// ***************************************************************************
float		CPatch::computeDisplaceRaw(float sTile, float tTile, float s, float t) const
{
	sint	ts,tt,ms,mt;
	computeDisplaceRawCoordinates(sTile, tTile, s, t, ts, tt, ms, mt);
	return computeDisplaceRawInteger(ts, tt, ms, mt);

}

// ***************************************************************************
static inline	void	computeDisplaceBilinear(float sTile, float tTile,
	float &sInc, float &tInc, float &sa, float &ta, float &sa1, float &ta1)
{
	float	sDecimal= sTile-noiseFloor(sTile);
	float	tDecimal= tTile-noiseFloor(tTile);
	float	sDist, tDist;

	// Do a bilinear centered on 0.5, 0.5.

	// Compute increment, according to position against center.
	if(sDecimal>=0.5)
		sInc= 1;
	else
		sInc= -1;
	if(tDecimal>=0.5)
		tInc= 1;
	else
		tInc= -1;

	// Compute weight factor.
	sDist= (float)fabs(0.5 - sDecimal);	// s distance from center.
	tDist= (float)fabs(0.5 - tDecimal);	// t distance from center.
	sa= 1-sDist;
	ta= 1-tDist;
	sa1= 1-sa;
	ta1= 1-ta;
}


// ***************************************************************************
float		CPatch::computeDisplaceInteriorSmooth(float s, float t) const
{
	float sTile= s;
	float tTile= t;
	float	ret;

	// compute bi-linear weight factors.
	float	sInc, tInc, sa, ta, sa1, ta1;
	computeDisplaceBilinear(sTile, tTile, sInc, tInc, sa, ta, sa1, ta1);


	// NB: to have smooth transition, must keep the same (s,t), so we do a transition with the noise tile of
	// our neigbhor, but under us.

	// speed up, using just one computeDisplaceRawCoordinates(), and multiple computeDisplaceRawInteger().
	sint	ts,tt,ms,mt;
	computeDisplaceRawCoordinates(sTile, tTile, s, t, ts, tt, ms, mt);

	sint	sIncInt= (sint) sInc;
	sint	tIncInt= (sint) tInc;
	ret = computeDisplaceRawInteger(ts, tt, ms,mt) * sa * ta;
	ret+= computeDisplaceRawInteger(ts+sIncInt, tt, ms,mt) * sa1 * ta;
	ret+= computeDisplaceRawInteger(ts, tt+tIncInt, ms,mt) * sa * ta1;
	ret+= computeDisplaceRawInteger(ts+sIncInt, tt+tIncInt, ms,mt) * sa1 * ta1;

	return ret;
}


// ***************************************************************************
float		CPatch::computeDisplaceEdgeSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const
{
	float sTile= s;
	float tTile= t;
	CBindInfo	bindInfo;
	uint		edge=0;

	// only one must be not null
	nlassert( (smoothBorderX==0) != (smoothBorderY==0) );


	// Get the edge against we must share displace.
	if(smoothBorderX==-1)	edge=0;
	else if(smoothBorderY==1)	edge=1;
	else if(smoothBorderX==1)	edge=2;
	else if(smoothBorderY==-1)	edge=3;
	else nlstop;

	// Build the bindInfo against this edge.
	getBindNeighbor(edge, bindInfo);

	// Fast reject: if no neighbor, just do a simple computeDisplaceInteriorSmooth.
	if(!bindInfo.Zone)
		return computeDisplaceInteriorSmooth(s, t);
	// else, look for result in neighborhood.
	else
	{
		float	ret;


		// compute bi-linear weight factors.
		float	sInc, tInc, sa, ta, sa1, ta1;
		computeDisplaceBilinear(sTile, tTile, sInc, tInc, sa, ta, sa1, ta1);
		// Manage limit case: if bilinear has not chosen the good direction (because of floor and orientation).
		// eg on Right edge: This case arise if sDecimal==0, so if sa==sa1==0.5f. result is that
		// smoothBorderX is != than sInc on right border. same reasoning with downBorder (smoothBorderY=1).

		// NO NEED TO DO HERE, because sInc or tInc is not used if it is bad (smoothBorder? used instead).
		// and no need to correct sa, sa1, because in this case they are both equal to 0.5.


		// compute Neighboring info.
		CPatchUVLocator		uvLocator;
		uvLocator.build(this, edge, bindInfo);

		/* NB: there is floor problems with neighbors:
			- difference of orientation => uv.v=1. This point to the 1th tile. But this is not the same, if
				v goes to up, or goes to down.
			- if multiple bind, problem at limit (eg a bind 1/2 on edge 0 with OrdertT=8, when uv.v= 4).
				because, selection of the patch is dependent of orientation too.
			To avoid them, just take center of (sTile, tTile) to remove ambiguity.
			This works because computeDisplaceRaw() use sTile, tTile to get the noiseMap, so the decimal part is not
			used.

			Notice that we do this AFTER computeDisplaceBilinear() of course.
		*/
		sTile= noiseFloor(sTile) + 0.5f;
		tTile= noiseFloor(tTile) + 0.5f;
		// If we were exactly on the superior edge, prec compute is false... so correct this here.
		if(sTile>OrderS) sTile--;
		if(tTile>OrderT) tTile--;


		// Bilinear across an edge (enjoy!!).
		CVector2f	stTileIn, stIn;
		CVector2f	stTileOut, stOut;
		CPatch		*patchOut;
		uint		patchId;


		// if vertical edge.
		if(smoothBorderX!=0)
		{
			// compute contribution of our patch.
			ret = computeDisplaceRaw(sTile,tTile, s,t) * sa * ta;
			ret+= computeDisplaceRaw(sTile,tTile+tInc, s,t) * sa * ta1;

			// compute contribution of next(s) patchs.

			// contribution of next at tTile.
			// Keep the same coordinate.
			stIn.set(s, t);
			// But look for the neighbor noise tile.
			stTileIn.set(sTile+smoothBorderX, tTile);
			// change basis: find the s,t on the neighbor patch.
			patchId= uvLocator.selectPatch(stTileIn);
			uvLocator.locateUV(stTileIn, patchId, patchOut, stTileOut);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			// Compute displace, and bi-linear on the neighbor patch.
			ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa1 * ta;

			// contribution of next at tTile+tInc (same reasoning).
			stIn.set(s, t);
			stTileIn.set(sTile+smoothBorderX, tTile+tInc);
			patchId= uvLocator.selectPatch(stTileIn);
			uvLocator.locateUV(stTileIn, patchId, patchOut, stTileOut);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa1 * ta1;

		}
		// else if horizontal edge.
		else
		{
			// same reasoning as above.

			// compute contribution of our patch.
			ret = computeDisplaceRaw(sTile, tTile, s,t) * sa * ta;
			ret+= computeDisplaceRaw(sTile+sInc,tTile, s,t) * sa1 * ta;

			// compute contribution of next(s) patchs.
			// contribution of next at tTile.
			stIn.set(s, t);
			stTileIn.set(sTile, tTile+smoothBorderY);
			patchId= uvLocator.selectPatch(stTileIn);
			uvLocator.locateUV(stTileIn, patchId, patchOut, stTileOut);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa * ta1;

			// contribution of next at tTile+tInc (same reasoning).
			stIn.set(s, t);
			stTileIn.set(sTile+sInc, tTile+smoothBorderY);
			patchId= uvLocator.selectPatch(stTileIn);
			uvLocator.locateUV(stTileIn, patchId, patchOut, stTileOut);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa1 * ta1;
		}

		return ret;
	}

}


// ***************************************************************************
float		CPatch::computeDisplaceRawOnNeighbor(float sTile, float tTile, float s, float t) const
{
	sint	edge= -1;

	// look on what neighbor patch we must find the value (if any).
	if(sTile<0)	edge=0;
	else if(tTile>OrderT)	edge=1;
	else if(sTile>OrderS)	edge=2;
	else if(tTile<0)		edge=3;

	// If the location is In the patch, just return normal value.
	if(edge==-1)
		return computeDisplaceRaw(sTile, tTile, s, t);
	// else must find on neighbor.
	else
	{
		CBindInfo	bindInfo;
		getBindNeighbor(edge, bindInfo);

		// Fast reject: if no neighbor on the edge, just do a simple computeDisplaceRaw()
		if(!bindInfo.Zone)
		{
			return computeDisplaceRaw(sTile, tTile, s, t);
		}
		// else must find on neighbor.
		else
		{
			CPatchUVLocator		uvLocator;
			uvLocator.build(this, edge, bindInfo);

			CVector2f	stTileIn, stIn;
			CVector2f	stTileOut, stOut;
			CPatch		*patchOut;
			uint		patchId;

			// look on neighbor. same reasoning as in computeDisplaceEdgeSmooth();
			stIn.set(s, t);
			stTileIn.set(sTile, tTile);
			patchId= uvLocator.selectPatch(stTileIn);
			uvLocator.locateUV(stTileIn, patchId, patchOut, stTileOut);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			return patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y);
		}

	}
}


// ***************************************************************************
float		CPatch::computeDisplaceCornerSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const
{
	// compute the value across the corner (enjoy!!)
	// NB: Only corners with Edges==4 and corners on a bind are correclty supported.
	// ignore problems with corner which nbEdges!=4, because Blend of normals blend to 0 on corner (see computenoise()).

	float sTile= s;
	float tTile= t;
	CBindInfo	bindInfoX;
	CBindInfo	bindInfoY;
	uint		edgeX=0;
	uint		edgeY=0;

	// both must be not null
	nlassert( (smoothBorderX!=0) && (smoothBorderY!=0) );


	// Get the edge against we must share displace.
	if(smoothBorderX==-1)	edgeX=0;
	else if(smoothBorderX==1)	edgeX=2;
	else nlstop;
	if(smoothBorderY==1)	edgeY=1;
	else if(smoothBorderY==-1)	edgeY=3;
	else nlstop;

	// Build the bindInfo against those 2 edge.
	getBindNeighbor(edgeX, bindInfoX);
	getBindNeighbor(edgeY, bindInfoY);

	// Fast reject: if no neighbor on one of the edge, just do a simple computeDisplaceInteriorSmooth.
	if(!bindInfoX.Zone || !bindInfoY.Zone)
		return computeDisplaceInteriorSmooth(s, t);
	else
	{
		float	ret;


		// compute bi-linear weight factors.
		float	sInc, tInc, sa, ta, sa1, ta1;
		computeDisplaceBilinear(sTile, tTile, sInc, tInc, sa, ta, sa1, ta1);
		// Manage limit case: if bilinear has not chosen the good direction (because of floor and orientation).
		// eg on Right edge: This case arise if sDecimal==0, so if sa==sa1==0.5f. result is that
		// smoothBorderX is != than sInc on right border. same reasoning with downBorder (smoothBorderY=1).

		// NO NEED TO DO HERE, because sInc or tInc are not used at all.



		// compute Neighboring info.
		CPatchUVLocator		uvLocatorX;
		CPatchUVLocator		uvLocatorY;
		uvLocatorX.build(this, edgeX, bindInfoX);
		uvLocatorY.build(this, edgeY, bindInfoY);


		/* NB: see floor problems note in computeDisplaceEdgeSmooth();
		*/
		sTile= noiseFloor(sTile) + 0.5f;
		tTile= noiseFloor(tTile) + 0.5f;
		// If we were exactly on the superior edge, prec compute is false... so correct this here.
		if(sTile>OrderS) sTile--;
		if(tTile>OrderT) tTile--;


		// Bilinear across a corner.
		CVector2f	stTileIn, stIn;
		CVector2f	stTileOut, stOut;
		CPatch		*patchOut;
		uint		patchId;


		// compute contribution of our patch.
		ret = computeDisplaceRaw(sTile,tTile, s,t) * sa * ta;

		// compute contribution of the patch on the left/right side. same reasoning as in computeDisplaceEdgeSmooth();
		stIn.set(s, t);
		stTileIn.set(sTile+smoothBorderX, tTile);
		patchId= uvLocatorX.selectPatch(stTileIn);
		uvLocatorX.locateUV(stTileIn, patchId, patchOut, stTileOut);
		uvLocatorX.locateUV(stIn, patchId, patchOut, stOut);
		ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa1 * ta;


		// compute contribution of the patch on the up/down side. same reasoning as in computeDisplaceEdgeSmooth();
		stIn.set(s, t);
		stTileIn.set(sTile, tTile+smoothBorderY);
		patchId= uvLocatorY.selectPatch(stTileIn);
		uvLocatorY.locateUV(stTileIn, patchId, patchOut, stTileOut);
		uvLocatorY.locateUV(stIn, patchId, patchOut, stOut);
		ret+= patchOut->computeDisplaceRaw(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa * ta1;


		/* compute contribution of the patch adjacent to me.
			There is multiple case to consider here. Take example with corner=0 (ie smBdX=smBdY=-1):
				- if we are a normal corner with 4 edges, take the result from the left patch of our top patch.
				- if the corner is on a bind Edge, the top patch may be the bigger patch, so don't take the result
					from his neighbor (of course).
				- if we are a normal corner with N!=4 edges, just do same thing than if N==4. this is false but don't bother.

			To solve smoothly cases 1 and 2, use computeDisplaceRawOnNeighbor().
			This method, if nessecary, look on his neighbor to compute the value.
		*/
		stIn.set(s, t);
		stTileIn.set(sTile+smoothBorderX, tTile+smoothBorderY);
		// look on our "top" patch (this is arbitrary).
		patchId= uvLocatorY.selectPatch(stTileIn);
		uvLocatorY.locateUV(stTileIn, patchId, patchOut, stTileOut);
		uvLocatorY.locateUV(stIn, patchId, patchOut, stOut);
		ret+= patchOut->computeDisplaceRawOnNeighbor(stTileOut.x, stTileOut.y, stOut.x, stOut.y) * sa1 * ta1;


		return ret;
	}

}

// ***************************************************************************
CVector		CPatch::computeNormalEdgeSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const
{
	CBindInfo	bindInfo;
	uint		edge=0;
	CBezierPatch	*bpatch;
	bpatch= unpackIntoCache();

	// only one must be not null
	nlassert( (smoothBorderX==0) != (smoothBorderY==0) );


	// Get the edge against we must share displace.
	if(smoothBorderX==-1)	edge=0;
	else if(smoothBorderY==1)	edge=1;
	else if(smoothBorderX==1)	edge=2;
	else if(smoothBorderY==-1)	edge=3;
	else nlstop;

	// If the edge is smoothed, blend with neighbor.
	if(getSmoothFlag(edge))
	{
		// Build the bindInfo against this edge.
		getBindNeighbor(edge, bindInfo);

		// Fast reject: if no neighbor, just do a simple computeDisplaceInteriorSmooth.
		if(!bindInfo.Zone)
			return bpatch->evalNormal(s/getOrderS(), t/getOrderT());
		else
		{
			CVector		r0, r1;

			// Compute our contribution.
			r0= bpatch->evalNormal(s/getOrderS(), t/getOrderT());

			// Compute the coordinate on the border of the edge, and the coef of the blend.
			float		se=s;
			float		te=t;
			float		coef=0.0;
			if(smoothBorderX==-1)		se= noiseFloorF(se), coef=s-se;
			else if(smoothBorderX==1)	se= noiseCeilF(se), coef=se-s;
			else if(smoothBorderY==-1)	te= noiseFloorF(te), coef=t-te;
			else if(smoothBorderY==1)	te= noiseCeilF(te), coef=te-t;
			coef= 0.5f + coef*0.5f;

			// Compute contribution of the normal on the neighbor, on the border of the edge.
			CPatchUVLocator		uvLocator;
			CVector2f	stIn, stOut;
			CPatch		*patchOut;
			uint		patchId;

			uvLocator.build(this, edge, bindInfo);
			stIn.set(se, te);
			patchId= uvLocator.selectPatch(stIn);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);

			bpatch= patchOut->unpackIntoCache();
			r1= bpatch->evalNormal(stOut.x/patchOut->getOrderS(), stOut.y/patchOut->getOrderT());

			// NB: don't bother problems with bind 1/X and the choice of the patch, because bind are C1, so normal is C0.

			// Blend 2 result. For speed optim, don't normalize.
			return r0*coef + r1*(1-coef);
		}
	}
	// else blend with vector Null.
	else
	{
		// compute coef.
		float		se=s;
		float		te=t;
		float		coef=0.0;
		if(smoothBorderX==-1)		se= noiseFloorF(se), coef=s-se;
		else if(smoothBorderX==1)	se= noiseCeilF(se), coef=se-s;
		else if(smoothBorderY==-1)	te= noiseFloorF(te), coef=t-te;
		else if(smoothBorderY==1)	te= noiseCeilF(te), coef=te-t;

		// Compute our contribution.
		CVector		r0;
		r0= bpatch->evalNormal(s/getOrderS(), t/getOrderT());

		// Blend with 0.
		return r0*coef;
	}
}


// ***************************************************************************
CVector		CPatch::computeNormalOnNeighbor(float s, float t, uint edgeExclude) const
{
	sint	edge= -1;

	// look on what neighbor patch we must find the value (if any).
	if(s<1 && edgeExclude!=0)				edge=0;
	else if(t>OrderT-1 && edgeExclude!=1)	edge=1;
	else if(s>OrderS-1 && edgeExclude!=2)	edge=2;
	else if(t<1 && edgeExclude!=3)			edge=3;


	// If the location is In the patch, just return normal value. (case of a bind 1/X).
	if(edge==-1)
	{
		CBezierPatch	*bpatch= unpackIntoCache();
		return bpatch->evalNormal(s/getOrderS(), t/getOrderT());
	}
	// else must find on neighbor.
	else
	{
		CBindInfo	bindInfo;
		getBindNeighbor(edge, bindInfo);

		// Fast reject: if no neighbor on the edge, just do a simple computeDisplaceRaw()
		if(!bindInfo.Zone)
		{
			CBezierPatch	*bpatch= unpackIntoCache();
			return bpatch->evalNormal(s/getOrderS(), t/getOrderT());
		}
		// else must find on neighbor.
		else
		{
			CPatchUVLocator		uvLocator;
			uvLocator.build(this, edge, bindInfo);

			CVector2f	stIn;
			CVector2f	stOut;
			CPatch		*patchOut;
			uint		patchId;

			// look on neighbor. same reasoning as in computeDisplaceEdgeSmooth();
			stIn.set(s, t);
			patchId= uvLocator.selectPatch(stIn);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);
			CBezierPatch	*bpatch= patchOut->unpackIntoCache();
			return bpatch->evalNormal(stOut.x/patchOut->getOrderS(), stOut.y/patchOut->getOrderT());
		}

	}
}


// ***************************************************************************
CVector		CPatch::computeNormalCornerSmooth(float	s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const
{
	CBindInfo	bindInfoX;
	CBindInfo	bindInfoY;
	uint		edgeX=0;
	uint		edgeY=0;
	uint		corner;
	CBezierPatch	*bpatch;
	bpatch= unpackIntoCache();

	// both must be not null
	nlassert( (smoothBorderX!=0) && (smoothBorderY!=0) );


	// Get the edge against we must share displace.
	if(smoothBorderX==-1)	edgeX=0;
	else if(smoothBorderX==1)	edgeX=2;
	else nlstop;
	if(smoothBorderY==1)	edgeY=1;
	else if(smoothBorderY==-1)	edgeY=3;
	else nlstop;

	// Get the corner against we must share displace.
	if(smoothBorderX==-1)
	{
		if(smoothBorderY==-1)	corner=0;
		else					corner=1;
	}
	else
	{
		if(smoothBorderY==-1)	corner=3;
		else					corner=2;
	}

	// If this corner is smoothed, blend with 4 neighbors patchs.
	if(getCornerSmoothFlag(corner))
	{
		// Build the bindInfo against the 2 edge.
		getBindNeighbor(edgeX, bindInfoX);
		getBindNeighbor(edgeY, bindInfoY);

		// Fast reject: if no neighbor, just do a simple computeDisplaceInteriorSmooth.
		if(!bindInfoX.Zone || !bindInfoY.Zone)
			return bpatch->evalNormal(s/getOrderS(), t/getOrderT());
		else
		{
			CVector		ret;


			// Compute the coordinate on the border of the edge, and the coef of the blend.
			float		se=s;
			float		te=t;
			float		coefX;
			float		coefY;
			if(smoothBorderX==-1)	se= noiseFloorF(se), coefX=s-se;
			else					se= noiseCeilF(se), coefX=se-s;
			if(smoothBorderY==-1)	te= noiseFloorF(te), coefY=t-te;
			else					te= noiseCeilF(te), coefY=te-t;
			coefX= 0.5f + coefX*0.5f;
			coefY= 0.5f + coefY*0.5f;


			// Compute our contribution.
			ret= bpatch->evalNormal(s/getOrderS(), t/getOrderT()) *coefX*coefY;


			// compute Neighboring info.
			CPatchUVLocator		uvLocatorX;
			CPatchUVLocator		uvLocatorY;
			CVector2f	stIn, stOut;
			CPatch		*patchOut;
			uint		patchId;

			uvLocatorX.build(this, edgeX, bindInfoX);
			uvLocatorY.build(this, edgeY, bindInfoY);

			// Patch on our X side.
			stIn.set(se, te);
			patchId= uvLocatorX.selectPatch(stIn);
			uvLocatorX.locateUV(stIn, patchId, patchOut, stOut);
			bpatch= patchOut->unpackIntoCache();
			ret+= bpatch->evalNormal(stOut.x/patchOut->getOrderS(), stOut.y/patchOut->getOrderT()) *(1-coefX)*coefY;

			// Patch on our Y side.
			stIn.set(se, te);
			patchId= uvLocatorY.selectPatch(stIn);
			uvLocatorY.locateUV(stIn, patchId, patchOut, stOut);
			bpatch= patchOut->unpackIntoCache();
			ret+= bpatch->evalNormal(stOut.x/patchOut->getOrderS(), stOut.y/patchOut->getOrderT()) *coefX*(1-coefY);

			/* compute contribution of the patch adjacent to me.
				Same reasoning as in computeDisplaceCornerSmooth().
			*/
			stIn.set(se, te);
			patchId= uvLocatorY.selectPatch(stIn);
			uvLocatorY.locateUV(stIn, patchId, patchOut, stOut);
			// Because we compute the normal exactly on the edge, we must inform this method not to take us as neighbor.
			// ugly but simpler.
			ret+= patchOut->computeNormalOnNeighbor(stOut.x, stOut.y, bindInfoY.Edge[patchId]) *(1-coefX)*(1-coefY);

			return ret;
		}
	}
	// else must blend with 0.
	else
	{
		// compute coef.
		float		se=s;
		float		te=t;
		float		coefX;
		float		coefY;
		if(smoothBorderX==-1)	se= noiseFloorF(se), coefX=s-se;
		else					se= noiseCeilF(se), coefX=se-s;
		if(smoothBorderY==-1)	te= noiseFloorF(te), coefY=t-te;
		else					te= noiseCeilF(te), coefY=te-t;


		// To have smooth continuities with smooth on edge (if any), we must do this.
		CVector		rx, ry;
		// Compute a smooth with my X neighbor.
		rx= computeNormalEdgeSmooth(s, t, smoothBorderX, 0);
		// Compute a smooth with my Y neighbor.
		ry= computeNormalEdgeSmooth(s, t, 0, smoothBorderY);

		// Blend the 2 result.
		if(coefY + coefX>0)
		{
			// This the weight used to blend to 0.
			float	maxCoef= max(coefY, coefX);
			// This the weight used to blend beetween rx and ry.
			float	ooSum= 1.0f / (coefY + coefX);
			float	blendCoefX= coefX * ooSum;
			float	blendCoefY= coefY * ooSum;

			return maxCoef* (rx*blendCoefY + ry*blendCoefX);
		}
		else
		{
			return CVector::Null;
		}
	}
}



// ***************************************************************************
void		CPatch::computeNoise(float s, float t, CVector &displace) const
{
	float	so= s*OrderS;
	float	to= t*OrderT;


	// Pre-Compute Border Smothing.
	//=========================
	// If we are on a border, flag it.
	sint8	smoothNormalBorderX= 0;
	sint8	smoothNormalBorderY= 0;
	// NB: because OrderS and OrderT >=2, smoothNormalBorderX=-1 and smoothNormalBorderX=1 are exclusive (as smoothNormalBorderY).
	if(so < 1) smoothNormalBorderX= -1;
	else if(so > OrderS-1) smoothNormalBorderX= 1;
	if(to < 1) smoothNormalBorderY= -1;
	else if(to > OrderT-1) smoothNormalBorderY= 1;

	bool	smoothNormalEdge= (smoothNormalBorderX!=0) != (smoothNormalBorderY!=0);
	bool	smoothNormalCorner= (smoothNormalBorderX!=0) && (smoothNormalBorderY!=0);


	// Do same thing, but to know if we must compute a displace on an interior, on an edge or on a corner.
	sint8	smoothDisplaceBorderX= 0;
	sint8	smoothDisplaceBorderY= 0;
	// NB: because OrderS and OrderT >=2, smoothBorderX=-1 and smoothBorderX=1 are exclusive (as smoothBorderY).
	if(so < 0.5) smoothDisplaceBorderX= -1;
	else if(so > OrderS-0.5) smoothDisplaceBorderX= 1;
	if(to < 0.5) smoothDisplaceBorderY= -1;
	else if(to > OrderT-0.5) smoothDisplaceBorderY= 1;

	bool	smoothDisplaceEdge= (smoothDisplaceBorderX!=0) != (smoothDisplaceBorderY!=0);
	bool	smoothDisplaceCorner= (smoothDisplaceBorderX!=0) && (smoothDisplaceBorderY!=0);


	// Compute Displace value.
	//=========================
	float	displaceValue;

	if(smoothDisplaceCorner)
		displaceValue= computeDisplaceCornerSmooth(so, to, smoothDisplaceBorderX, smoothDisplaceBorderY);
	else if(smoothDisplaceEdge)
		displaceValue= computeDisplaceEdgeSmooth(so, to, smoothDisplaceBorderX, smoothDisplaceBorderY);
	else
		displaceValue= computeDisplaceInteriorSmooth(so, to);



	// Compute Displace normal.
	//=========================

	// Evaluate the normal.
	CVector		displaceNormal;


	// smooth on edges and on corners.
	if(smoothNormalCorner)
		displaceNormal= computeNormalCornerSmooth(so, to, smoothNormalBorderX, smoothNormalBorderY);
	else if(smoothNormalEdge)
		displaceNormal= computeNormalEdgeSmooth(so, to, smoothNormalBorderX, smoothNormalBorderY);
	else
	{
		// unpack...
		CBezierPatch	*bpatch= unpackIntoCache();
		// eval.
		displaceNormal= bpatch->evalNormal(s, t);
	}



	// Final result.
	//=========================
	displace= displaceNormal * displaceValue;
}



// ***************************************************************************
void			CPatch::setCornerSmoothFlag(uint corner, bool smooth)
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	if(smooth)
		_CornerSmoothFlag|= mask;
	else
		_CornerSmoothFlag&= ~mask;
}

// ***************************************************************************
bool			CPatch::getCornerSmoothFlag(uint corner) const
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	return	(_CornerSmoothFlag& mask)!=0;
}


} // NL3D
