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

#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"


using namespace NLMISC;
using namespace std;


// define it only for debug bind.
//#define	NL3D_DEBUG_DONT_BIND_PATCH


namespace NL3D
{



// ***************************************************************************
// ***************************************************************************
// CPatchInfo
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatchInfo::setCornerSmoothFlag(uint corner, bool smooth)
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	if(smooth)
		_CornerSmoothFlag|= mask;
	else
		_CornerSmoothFlag&= ~mask;
}

// ***************************************************************************
bool			CPatchInfo::getCornerSmoothFlag(uint corner) const
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	return	(_CornerSmoothFlag & mask)!=0;
}


// ***************************************************************************
// ***************************************************************************
// CZone
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CZone::CZone()
{
	ZoneId= 0;
	Compiled= false;
	Landscape= NULL;
	ClipResult= ClipOut;
}
// ***************************************************************************
CZone::~CZone()
{
	// release() must have been called.
	nlassert(!Compiled);
}


// ***************************************************************************
void			CZone::computeBBScaleBias(const CAABBox	&bb)
{
	ZoneBB= bb;
	// Take a security for noise. (useful for zone clipping).
	ZoneBB.setHalfSize(ZoneBB.getHalfSize()+CVector(NL3D_NOISE_MAX, NL3D_NOISE_MAX, NL3D_NOISE_MAX));
	CVector	hs= ZoneBB.getHalfSize();
	float	rmax= maxof(hs.x, hs.y, hs.z);
	PatchScale= rmax / 32760;		// Prevent from float imprecision by taking 32760 and not 32767.
	PatchBias= ZoneBB.getCenter();
}


// ***************************************************************************
void			CZone::build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices, uint32 numVertices)
{
	CZoneInfo	zinfo;
	zinfo.ZoneId= zoneId;
	zinfo.Patchs= patchs;
	zinfo.BorderVertices= borderVertices;

	build(zinfo, numVertices);
}
// ***************************************************************************
void			CZone::build(const CZoneInfo &zoneInfo, uint32 numVertices)
{
	sint	i,j;
	nlassert(!Compiled);

	// Ref inupt
	uint16		zoneId= zoneInfo.ZoneId;
	const std::vector<CPatchInfo> &patchs= zoneInfo.Patchs;
	const std::vector<CBorderVertex> &borderVertices= zoneInfo.BorderVertices;


	ZoneId= zoneId;
	BorderVertices= borderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	CAABBox		bb;
	if(!patchs.empty())
		bb.setCenter(patchs[0].Patch.Vertices[0]);
	bb.setHalfSize(CVector::Null);
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CBezierPatch	&p= patchs[j].Patch;
		for(i=0;i<4;i++)
			bb.extend(p.Vertices[i]);
		for(i=0;i<8;i++)
			bb.extend(p.Tangents[i]);
		for(i=0;i<4;i++)
			bb.extend(p.Interiors[i]);
	}
	// Compute BBox, and Patch Scale Bias, according to Noise.
	computeBBScaleBias(bb);


	// Compute/compress Patchs.
	//=========================
	Patchs.resize(patchs.size());
	PatchConnects.resize(patchs.size());
	sint	maxVertex=-1;
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CPatchInfo	&pi= patchs[j];
		const CBezierPatch	&p= pi.Patch;
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		// Smoothing flags
		pa.Flags&=~NL_PATCH_SMOOTH_FLAG_MASK;
		pa.Flags|=NL_PATCH_SMOOTH_FLAG_MASK&(pi.Flags<<NL_PATCH_SMOOTH_FLAG_SHIFT);


		// Noise Data
		// copy noise rotation.
		pa.NoiseRotation= pi.NoiseRotation;
		// copy all noise smoothing info.
		for(i=0;i<4;i++)
		{
			pa.setCornerSmoothFlag(i, pi.getCornerSmoothFlag(i));
		}

		// Copy order of the patch
		pa.OrderS= pi.OrderS;
		pa.OrderT= pi.OrderT;

		// Build the patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].pack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].pack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].pack(p.Interiors[i], PatchBias, PatchScale);
		pa.Tiles= pi.Tiles;
		pa.TileColors= pi.TileColors;
		/* Copy TileLightInfluences. It is possible that pi.TileLightInfluences.size()!= 0
			and pi.TileLightInfluences.size()!= (uint)(pi.OrderS/2+1)*(pi.OrderT/2+1)
			Because of a preceding bug where pa.OrderS and pa.OrderT were not initialized before the
			pa.resetTileLightInfluences();
		*/
		if( pi.TileLightInfluences.size()!= (uint)(pi.OrderS/2+1)*(pi.OrderT/2+1) )
		{
			pa.resetTileLightInfluences();
		}
		else
		{
			pa.TileLightInfluences= pi.TileLightInfluences;
		}

		// Number of lumels in this patch
		uint lumelCount=(pi.OrderS*NL_LUMEL_BY_TILE)*(pi.OrderT*NL_LUMEL_BY_TILE);

		// Lumel empty ?
		if (pi.Lumels.size ()==lumelCount)
		{
			// Pack the lumel map
			pa.packShadowMap (&pi.Lumels[0]);
		}
		else
		{
			// Reset lightmap
			pa.resetCompressedLumels ();
		}

		nlassert(pa.Tiles.size()== (uint)pi.OrderS*pi.OrderT);
		nlassert(pa.TileColors.size()== (uint)(pi.OrderS+1)*(pi.OrderT+1));

		// Build the patchConnect.
		pc.ErrorSize= pi.ErrorSize;
		for(i=0;i<4;i++)
		{
			pc.BaseVertices[i]= pi.BaseVertices[i];
			maxVertex= max((sint)pc.BaseVertices[i], maxVertex);
		}
		for(i=0;i<4;i++)
			pc.BindEdges[i]= pi.BindEdges[i];
	}

	NumVertices= maxVertex+1;
	NumVertices= max((uint32)NumVertices, numVertices);

	// Init the Clip Arrays
	_PatchRenderClipped.resize((uint)Patchs.size());
	_PatchOldRenderClipped.resize((uint)Patchs.size());
	_PatchRenderClipped.setAll();
	_PatchOldRenderClipped.setAll();

	// Copy PointLights.
	//=========================
	// build array, lights are sorted
	std::vector<uint>	plRemap;
	_PointLightArray.build(zoneInfo.PointLights, plRemap);
	// Check TileLightInfluences integrity, and remap PointLight Indices.
	for(j=0;j<(sint)patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		for(uint k= 0; k<pa.TileLightInfluences.size(); k++)
		{
			CTileLightInfluence		&tli= pa.TileLightInfluences[k];
			for(uint l=0; l<CTileLightInfluence::NumLightPerCorner; l++)
			{
				// If NULL light, break and continue to next TileLightInfluence.
				if(tli.Light[l]== 0xFF)
					break;
				else
				{
					// Check good index.
					nlassert(tli.Light[l] < _PointLightArray.getPointLights().size());
					// Remap index, because of light sorting.
					tli.Light[l]= plRemap[tli.Light[l]];
				}

			}
		}
	}
}

// ***************************************************************************
void			CZone::retrieve(std::vector<CPatchInfo> &patchs, std::vector<CBorderVertex> &borderVertices)
{
	CZoneInfo	zinfo;

	retrieve(zinfo);

	patchs= zinfo.Patchs;
	borderVertices= zinfo.BorderVertices;
}

// ***************************************************************************
void			CZone::retrieve(CZoneInfo &zoneInfo)
{
	sint i,j;

	// Ref on input.
	std::vector<CPatchInfo> &patchs= zoneInfo.Patchs;
	std::vector<CBorderVertex> &borderVertices= zoneInfo.BorderVertices;
	// Copy zoneId.
	zoneInfo.ZoneId= getZoneId();


	// uncompress Patchs.
	//=========================
	patchs.resize(Patchs.size());
	for(j=0;j<(sint)patchs.size();j++)
	{
		CPatchInfo			&pi= patchs[j];
		CBezierPatch		&p= pi.Patch;
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];


		// Smoothing flags
		pi.Flags= (pa.Flags&NL_PATCH_SMOOTH_FLAG_MASK)>>NL_PATCH_SMOOTH_FLAG_SHIFT;


		// Noise Data
		// copy noise rotation.
		pi.NoiseRotation= pa.NoiseRotation;
		// copy all noise smoothing info.
		for(i=0;i<4;i++)
		{
			pi.setCornerSmoothFlag(i, pa.getCornerSmoothFlag(i));
		}


		// re-Build the uncompressed bezier patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].unpack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].unpack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].unpack(p.Interiors[i], PatchBias, PatchScale);
		pi.Tiles= pa.Tiles;
		pi.TileColors= pa.TileColors;
		pi.TileLightInfluences= pa.TileLightInfluences;
		pi.Lumels.resize ((pa.OrderS*4)*(pa.OrderT*4));
		pi.Flags=(pa.Flags&NL_PATCH_SMOOTH_FLAG_MASK)>>NL_PATCH_SMOOTH_FLAG_SHIFT;

		// Unpack the lumel map
		pa.unpackShadowMap (&pi.Lumels[0]);

		// from the patchConnect.
		pi.OrderS= pa.OrderS;
		pi.OrderT= pa.OrderT;
		pi.ErrorSize= pc.ErrorSize;
		for(i=0;i<4;i++)
		{
			pi.BaseVertices[i]= pc.BaseVertices[i];
		}
		for(i=0;i<4;i++)
			pi.BindEdges[i]= pc.BindEdges[i];
	}

	// retrieve bordervertices.
	//=========================
	borderVertices= BorderVertices;

	// retrieve PointLights.
	//=========================
	zoneInfo.PointLights= _PointLightArray.getPointLights();

}


// ***************************************************************************
void			CZone::build(const CZone &zone)
{
	nlassert(!Compiled);

	ZoneId= zone.ZoneId;
	BorderVertices= zone.BorderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	ZoneBB= zone.ZoneBB;
	PatchScale= zone.PatchScale;
	PatchBias= zone.PatchBias;


	// Compute/compress Patchs.
	//=========================
	Patchs= zone.Patchs;
	PatchConnects= zone.PatchConnects;

	// Init the Clip Arrays
	_PatchRenderClipped.resize((uint)Patchs.size());
	_PatchOldRenderClipped.resize((uint)Patchs.size());
	_PatchRenderClipped.setAll();
	_PatchOldRenderClipped.setAll();


	// copy pointLights.
	//=========================
	_PointLightArray= zone._PointLightArray;


	NumVertices= zone.NumVertices;
}



// ***************************************************************************
void			CBorderVertex::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	(void)f.serialVersion(0);

	f.xmlSerial (CurrentVertex, "CURRENT_VERTEX");
	f.xmlSerial (NeighborZoneId, "NEIGHTBOR_ZONE_ID");
	f.xmlSerial (NeighborVertex, "NEIGHTBOR_VERTEX");
}
void			CZone::CPatchConnect::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	uint	ver= f.serialVersion(1);

	if (ver<1)
		f.serial(OldOrderS, OldOrderT, ErrorSize);
	else
		f.serial(ErrorSize);
	f.xmlSerial (BaseVertices[0], BaseVertices[1], BaseVertices[2], BaseVertices[3], "BASE_VERTICES");
	f.xmlSerial (BindEdges[0], BindEdges[1], BindEdges[2], BindEdges[3], "BIND_EDGES");
}
void			CPatchInfo::CBindInfo::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	(void)f.serialVersion(0);
	f.xmlSerial(NPatchs, "NPATCH");
	nlassert ( (NPatchs==0) | (NPatchs==1) | (NPatchs==2) | (NPatchs==4) | (NPatchs==5) );
	f.xmlSerial (ZoneId, "ZONE_ID");
	f.xmlSerial (Next[0], Next[1], Next[2], Next[3], "NEXT_PATCH");
	f.xmlSerial (Edge[0], Edge[1], Edge[2], Edge[3], "NEXT_EDGE");
}

// ***************************************************************************
void			CZone::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	/*
	Version 4:
		- PointLights
	Version 3:
		- Lumels compression version 2.
	Version 2:
		- Lumels.
	Version 1:
		- Tile color.
	Version 0:
		- base verison.
	*/
	uint	ver= f.serialVersion(4);

	// No more compatibility before version 3
	if (ver<3)
	{
		throw EOlderStream(f);
	}

	f.serialCheck(NELID("ENOZ"));

	f.xmlSerial (ZoneId, "ZONE_ID");
	f.xmlSerial (ZoneBB, "BB");
	f.xmlSerial (PatchBias, "PATCH_BIAS");
	f.xmlSerial (PatchScale, "PATCH_SCALE");
	f.xmlSerial (NumVertices, "NUM_VERTICES");

	f.xmlPush ("BORDER_VERTICES");
	f.serialCont(BorderVertices);
	f.xmlPop ();

	f.xmlPush ("PATCHES");
	f.serialCont(Patchs);
	f.xmlPop ();

	f.xmlPush ("PATCH_CONNECTS");
	f.serialCont(PatchConnects);
	f.xmlPop ();

	if (ver>=4)
	{
		f.xmlPush ("POINT_LIGHTS");
		f.serial(_PointLightArray);
		f.xmlPop ();
	}

	// If read, must create and init Patch Clipped state to true (clipped even if not compiled)
	if(f.isReading())
	{
		_PatchRenderClipped.resize((uint)Patchs.size());
		_PatchOldRenderClipped.resize((uint)Patchs.size());
		_PatchRenderClipped.setAll();
		_PatchOldRenderClipped.setAll();
	}

	// If read and version 0, must init default TileColors of patchs.
	//===============================================================
	// if(f.isReading() && ver<2) ...
	// Deprecated, because ver<3 not supported
}



// ***************************************************************************
void			CZone::compile(CLandscape *landscape, TZoneMap &loadedZones)
{
	sint	i,j;
	TZoneMap		neighborZones;

	//nlinfo("Compile Zone: %d \n", (sint32)getZoneId());

	// Can't compile if compiled.
	nlassert(!Compiled);
	Landscape= landscape;

	// Attach this to loadedZones.
	//============================
	nlassert(loadedZones.find(ZoneId)==loadedZones.end());
	loadedZones[ZoneId]= this;

	// Create/link the base vertices according to present neigbor zones.
	//============================
	BaseVertices.clear();
	BaseVertices.resize(NumVertices);
	// First try to link vertices to other.
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	vertto= BorderVertices[i].NeighborVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
			// Doesn't matter if BaseVertices is already linked to another zone...
			// This should be the same pointer in this case...
			BaseVertices[cur]=  zone->getBaseVertex(vertto);
		}
	}
	// Else, create unbounded vertices.
	for(i=0;i<(sint)BaseVertices.size();i++)
	{
		if(BaseVertices[i]==NULL)
		{
			BaseVertices[i]=  new CTessBaseVertex;
		}
	}


	// compile() the patchs.
	//======================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];
		CTessVertex			*baseVertices[4];

		baseVertices[0]= &(BaseVertices[pc.BaseVertices[0]]->Vert);
		baseVertices[1]= &(BaseVertices[pc.BaseVertices[1]]->Vert);
		baseVertices[2]= &(BaseVertices[pc.BaseVertices[2]]->Vert);
		baseVertices[3]= &(BaseVertices[pc.BaseVertices[3]]->Vert);
		pa.compile(this, j, pa.OrderS, pa.OrderT, baseVertices, pc.ErrorSize);
	};

	// compile() the Clip information for the patchs.
	//======================
	_PatchBSpheres.resize(Patchs.size());
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];

		// Buil the BSPhere of the patch.
		CAABBox	bb= pa.buildBBox();
		_PatchBSpheres[j].Center= bb.getCenter();
		_PatchBSpheres[j].Radius= bb.getRadius();
	}

	// bind() the patchs. (after all compiled).
	//===================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		// bind the patch. This is the original bind, not a rebind.
		bindPatch(loadedZones, pa, pc, false);
	}


	// rebindBorder() on neighbor zones.
	//==================================
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		(*zoneIt).second->rebindBorder(loadedZones);
	}

	// End!!
	Compiled= true;
}

// ***************************************************************************
void			CZone::release(TZoneMap &loadedZones)
{
	sint	i,j;

	if(!Compiled)
		return;

	// detach this zone to loadedZones.
	//=================================
	nlassert(loadedZones.find(ZoneId)!=loadedZones.end());
	loadedZones.erase(ZoneId);
	// It doesn't server to unbindPatch(), since patch is not binded to neigbors.


	// unbind() the patchs.
	//=====================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		unbindPatch(pa);
	}


	// rebindBorder() on neighbor zones.
	//==================================
	// Build the nieghborood.
	TZoneMap		neighborZones;
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
		}
	}
	// rebind borders.
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		// Since
		(*zoneIt).second->rebindBorder(loadedZones);
	}


	// release() the patchs.
	//======================
	// unbind() need compiled neigbor patchs, so do the release after all unbind (so after rebindBorder() too...).
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		pa.release();
	}


	// destroy/unlink the base vertices (internal..), according to present neigbor zones.
	//=================================
	// Just release the smartptrs (easy!!). Do it after patchs released...
	BaseVertices.clear();


	// End!!
	Compiled= false;
	Landscape= NULL;
	ClipResult= ClipOut;
}


// ***************************************************************************
// ***************************************************************************
// Private part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::rebindBorder(TZoneMap &loadedZones)
{
	sint	j;

	// rebind patchs which are on border.
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		if(patchOnBorder(pc))
		{
			// rebind the patch. This is a rebind.
			bindPatch(loadedZones, pa, pc, true);
		}
	}
}

// ***************************************************************************
CPatch		*CZone::getZonePatch(TZoneMap &loadedZones, sint zoneId, sint patch)
{
#ifdef NL3D_DEBUG_DONT_BIND_PATCH
	return NULL;
#endif
	if(loadedZones.find(zoneId)==loadedZones.end())
		return NULL;
	else
		return (loadedZones[zoneId])->getPatch(patch);
}


// ***************************************************************************
void		CZone::buildBindInfo(uint patchId, uint edge, CZone *neighborZone, CPatch::CBindInfo	&paBind)
{
	nlassert(patchId < Patchs.size());
	nlassert(neighborZone);

	CPatchConnect	&pc= PatchConnects[patchId];


	// Get the bind info of this patch to his neighbor on "edge".
	CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[edge];
	nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);


	// copy zone ptr.
	paBind.Zone= neighborZone;


	// Special case of a small patch connected to a bigger.
	if(pcBind.NPatchs==5)
	{
		paBind.NPatchs= 1;
		paBind.Next[0]= neighborZone->getPatch(pcBind.Next[0]);
		paBind.Edge[0]= pcBind.Edge[0];

		// Get the twin bindInfo of pcBind.
		const CPatchInfo::CBindInfo	&pcBindNeighbor=
			neighborZone->getPatchConnect(pcBind.Next[0])->BindEdges[pcBind.Edge[0]];
		// must have a multiple bind.
		nlassert(pcBindNeighbor.NPatchs == 2 || pcBindNeighbor.NPatchs == 4);

		// number of bind is stored on the twin bindInfo.
		paBind.MultipleBindNum= pcBindNeighbor.NPatchs;

		// Search our patchId on neighbor;
		paBind.MultipleBindId= 255;
		for(sint i=0; i<paBind.MultipleBindNum; i++)
		{
			if(pcBindNeighbor.Next[i]==patchId)
				paBind.MultipleBindId= i;
		}
		nlassert(paBind.MultipleBindId!= 255);
	}
	else
	{
		paBind.MultipleBindNum= 0;
		paBind.NPatchs= pcBind.NPatchs;
		for(sint i=0;i<paBind.NPatchs; i++)
		{
			paBind.Next[i]= neighborZone->getPatch(pcBind.Next[i]);
			paBind.Edge[i]= pcBind.Edge[i];
		}
	}


}


// ***************************************************************************
void		CZone::bindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc, bool rebind)
{
	CPatch::CBindInfo	edges[4];

	// Fill all edges.
	for(sint i=0;i<4;i++)
	{
		CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];
		CPatch::CBindInfo		&paBind= edges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		paBind.NPatchs= pcBind.NPatchs;


		// Find the zone.
		TZoneMap::iterator	itZoneMap;
		// If no neighbor, or if zone neighbor not loaded.
		if( paBind.NPatchs==0 || (itZoneMap=loadedZones.find(pcBind.ZoneId)) == loadedZones.end() )
			paBind.Zone= NULL;
		else
			paBind.Zone= itZoneMap->second;


		// Special case of a small patch connected to a bigger.
		if(paBind.NPatchs==5)
		{
			paBind.Edge[0]= pcBind.Edge[0];
			paBind.Next[0]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[0])
				paBind.NPatchs=0;
			else
			{
				// pa.bind() will do the job.
				// Leave it flagged with NPatchs==5.
				continue;
			}
		}


		// Bind 1/1 and 1/2,1/4
		if(paBind.NPatchs>=1)
		{
			paBind.Edge[0]= pcBind.Edge[0];
			paBind.Next[0]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[0])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=2)
		{
			paBind.Edge[1]= pcBind.Edge[1];
			paBind.Next[1]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[1]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[1])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=4)
		{
			paBind.Edge[2]= pcBind.Edge[2];
			paBind.Edge[3]= pcBind.Edge[3];
			paBind.Next[2]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[2]);
			paBind.Next[3]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[3]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[2] || !paBind.Next[3])
				paBind.NPatchs=0;
		}
	}

	// First, unbind.
	pa.unbind();

	// Then bind.
	pa.bind(edges, rebind);
}


// ***************************************************************************
void		CZone::unbindPatch(CPatch &pa)
{
	/*
		Remind: the old version with CPatch::unbindFrom*() doesn't work because of CZone::release(). This function
		first erase the zone from loadedZones...
		Not matter here. We use CPatch::unbind() which should do all the good job correctly (unbind pa from ohters
		, and unbind others from pa at same time).
	*/

	pa.unbind();
}


// ***************************************************************************
bool			CZone::patchOnBorder(const CPatchConnect &pc) const
{
	// If only one of neighbor patch is not of this zone, we are on a border.

	// Test all edges.
	for(sint i=0;i<4;i++)
	{
		const CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		if(pcBind.NPatchs>=1)
		{
			if(pcBind.ZoneId != ZoneId)
				return true;
		}
	}

	return false;
}


// ***************************************************************************
// ***************************************************************************
// Render part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
const CBSphere	&CZone::getPatchBSphere(uint patch) const
{
	static	CBSphere	dummySphere;
	if(patch<_PatchBSpheres.size())
		return _PatchBSpheres[patch];
	else
		return dummySphere;
}


// ***************************************************************************
void			CZone::clip(const std::vector<CPlane>	&pyramid)
{
	H_AUTO( NLMISC_ClipZone );

	nlassert(Compiled);

	// bkup old ClipResult. NB: by default, it is ClipOut (no VB created).
	sint	oldClipResult= ClipResult;

	// Pyramid with only the planes that clip the zone
	static std::vector<CPlane>		patchPyramid(10);
	static std::vector<uint>		patchPyramidIndex(10);
	patchPyramidIndex.clear();

	// Compute ClipResult.
	//-------------------
	ClipResult= ClipIn;
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// If entirely out.
		if(!ZoneBB.clipBack(pyramid[i]))
		{
			ClipResult= ClipOut;
			// If out of only one plane, out of all.
			break;
		}
		// If partially IN (ie not entirely out, and not entirely IN)
		else if(ZoneBB.clipFront(pyramid[i]))
		{
			// Force ClipResult to be ClipSide, and not ClipIn.
			ClipResult=ClipSide;
			// Append the plane index to list to test
			patchPyramidIndex.push_back(i);
		}
	}


	// Easy Clip  :)
	if(Patchs.empty())
	{
		ClipResult= ClipOut;
		// don't need to go below...
		return;
	}


	// Clip By Patch Pass.
	//--------------------
	if(ClipResult==ClipOut)
	{
		H_AUTO( NLMISC_ClipZone_Out );

		// Set All RenderClip flags to true.
		_PatchRenderClipped.setAll();
	}
	else if(ClipResult==ClipIn)
	{
		H_AUTO( NLMISC_ClipZone_In );

		// Set All RenderClip flags to false.
		_PatchRenderClipped.clearAll();
	}
	else
	{
		H_AUTO( NLMISC_ClipZone_Side );

		// Copy only the pyramid planes of interest
		patchPyramid.resize(patchPyramidIndex.size());
		uint i;
		for(i=0;i<patchPyramidIndex.size();i++)
		{
			patchPyramid[i]= pyramid[patchPyramidIndex[i]];
		}

		// clip all patchs with the simplified pyramid
		clipPatchs(patchPyramid);
	}


	// delete / reallocate / fill VBuffers.
	//-------------------
	// If there is a change in the Clip of the zone, or if patchs may have change (ie ClipSide is undetermined).
	if(oldClipResult!=ClipResult || oldClipResult==ClipSide)
	{
		// get BitSet as Raw Array of uint32
		uint32	*oldRenderClip= const_cast<uint32*>(&_PatchOldRenderClipped.getVector()[0]);
		const	uint32	*newRenderClip= &_PatchRenderClipped.getVector()[0];
		uint	numPatchs= (uint)Patchs.size();
		// Then, we must test by patch.
		for(uint i=0;i<numPatchs;oldRenderClip++, newRenderClip++)
		{
			uint32	oldWord= *oldRenderClip;
			uint32	newWord= *newRenderClip;
			// process at max 32 patch
			uint	maxNumBits= min((numPatchs-i), 32U);
			uint32	mask= 1;
			for(;maxNumBits>0;maxNumBits--, mask<<=1, i++)
			{
				// same as: if(_PatchOldRenderClipped[i] != _PatchRenderClipped[i])
				if( (oldWord^newWord)&mask )
				{
					// set the flag.
					*oldRenderClip&= ~mask;
					*oldRenderClip|= newWord&mask;
					// update clip patch
					Patchs[i].updateClipPatchVB( (newWord&mask)!=0 );
				}
			}
		}

	}

}


// ***************************************************************************
void			CZone::clipPatchs(const std::vector<CPlane>	&pyramid)
{
	// Init all to Not clipped
	_PatchRenderClipped.clearAll();

	for(uint j=0;j<_PatchBSpheres.size();j++)
	{
		CBSphere	&bSphere= _PatchBSpheres[j];
		for(sint i=0;i<(sint)pyramid.size();i++)
		{
			// If entirely out.
			if(!bSphere.clipBack(pyramid[i]))
			{
				_PatchRenderClipped.set(j, true);
				break;
			}
		}
	}
}


// ***************************************************************************
// DebugYoyo.
// Code for Debug test Only.. Do not erase it, may be used later :)
/*
static	void	cleanTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		cleanTess(face->SonLeft);
		cleanTess(face->SonRight);
	}
	// If has father, clean it.
	if(face->Father)
	{
		CTessFace	*face1=face->Father;
		CTessFace	*face2=face->Father->FBase;
		face1->FLeft= face1->SonLeft->FBase;
		face1->FRight= face1->SonRight->FBase;
		if(face2!=NULL)
		{
			face2->FLeft= face2->SonLeft->FBase;
			face2->FRight= face2->SonRight->FBase;
		}
	}
}
static	void	testTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		testTess(face->SonLeft);
		testTess(face->SonRight);
	}
	// Test validity.
	nlassert(!face->FBase || face->FBase->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FLeft || face->FLeft->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FRight || face->FRight->Patch!=(CPatch*)0xdddddddd);
}
static	void	checkTess()
{
	// This test should be inserted at begin of CZone::refine().
	// And it needs hacking public/private.
	CPatch		*pPatch;
	sint		n;
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		cleanTess(pPatch->Son0);
		cleanTess(pPatch->Son1);
	}
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		testTess(pPatch->Son0);
		testTess(pPatch->Son1);
	}
}
*/


// ***************************************************************************
void			CZone::excludePatchFromRefineAll(uint patch, bool exclude)
{
	nlassert(Compiled);
	nlassert(patch<Patchs.size());

	if(patch>=Patchs.size())
		return;

	Patchs[patch].ExcludeFromRefineAll= exclude;
}


// ***************************************************************************
void			CZone::refineAll()
{
	nlassert(Compiled);

	if(Patchs.size()==0)
		return;

	// DO NOT do a forceNoRenderClip(), to avoid big allocation of Near/Far VB vertices in driver.
	// DO NOT modify ClipResult, to avoid big allocation of Near/Far VB vertices in driver.

	// refine ALL patchs (even those which may be invisible).
	CPatch		*pPatch= &(*Patchs.begin());
	sint n;
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		// For Pacs construction: may exclude some patch from refineAll (for speed improvement).
		if(!pPatch->ExcludeFromRefineAll)
			pPatch->refineAll();
	}

}


// ***************************************************************************
void			CZone::averageTesselationVertices()
{
	nlassert(Compiled);

	if(Patchs.size()==0)
		return;

	// averageTesselationVertices of ALL patchs.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->averageTesselationVertices();
	}
}


// ***************************************************************************
void			CZone::preRender()
{
	nlassert(Compiled);

	// Must be 2^X-1.
	static const	uint	updateFarRefineFreq= 15;
	// Take the renderDate here.
	uint		curDateMod= CLandscapeGlobals::CurrentRenderDate & updateFarRefineFreq;

	// If no patchs, do nothing.
	if(Patchs.empty())
		return;

	/* If patchs invisible, must still update their Far Textures,
		else, there may be slowdown when we turn the head.
	*/


	// If all the zone is invisible.
	if(ClipResult==ClipOut)
	{
		// No patchs are visible, but maybe update the far textures.
		if( curDateMod==(ZoneId & updateFarRefineFreq) )
		{
			// updateTextureFarOnly for all patchs.
			for(uint i=0;i<Patchs.size();i++)
			{
				Patchs[i].updateTextureFarOnly(_PatchBSpheres[i]);
			}
		}
	}
	// else If some patchs only are visible.
	else if(ClipResult==ClipSide)
	{
		// PreRender Pass, or updateTextureFarOnly(), according to _PatchRenderClipped state.
		for(uint i=0;i<Patchs.size();i++)
		{
			// If the patch is visible
			if(!_PatchRenderClipped[i])
			{
				// Then preRender it.
				Patchs[i].preRender(_PatchBSpheres[i]);
			}
			else
			{
				// else maybe updateFar it.
				// ZoneId+i for better repartition.
				if( curDateMod==((ZoneId+i) & updateFarRefineFreq) )
					Patchs[i].updateTextureFarOnly(_PatchBSpheres[i]);
			}
		}
	}
	else	// ClipResult==ClipIn
	{
		// PreRender Pass for All
		for(uint i=0;i<Patchs.size();i++)
		{
			Patchs[i].preRender(_PatchBSpheres[i]);
		}
	}

}


// ***************************************************************************
void			CZone::resetRenderFarAndDeleteVBFV()
{
	for(uint i=0;i<Patchs.size();i++)
	{
		// If patch is visible
		if(!_PatchRenderClipped[i])
		{
			// release VertexBuffer, and FaceBuffer
			Patchs[i].deleteVBAndFaceVector();
			// Flag.
			_PatchRenderClipped.set(i, true);
		}

		Patchs[i].resetRenderFar();
	}
}


// ***************************************************************************
void			CZone::forceMergeAtTileLevel()
{
	CPatch		*pPatch=0;
	if(Patchs.size()>0)
		pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->forceMergeAtTileLevel();
	}
}


// ***************************************************************************
// ***************************************************************************
// Misc part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::changePatchTextureAndColor (sint numPatch, const std::vector<CTileElement> *tiles, const std::vector<CTileColor> *colors)
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());


	// Update the patch texture.
	if (tiles)
	{
		nlassert( Patchs[numPatch].Tiles.size() == tiles->size() );
		Patchs[numPatch].Tiles = *tiles;
	}

	// Update the patch colors.
	if (colors)
	{
		nlassert( Patchs[numPatch].TileColors.size() == colors->size() );
		Patchs[numPatch].TileColors = *colors;
	}

	if (Compiled)
	{
		// If the patch is visible, then we must LockBuffers, because new VertexVB may be created.
		if(!_PatchRenderClipped[numPatch])
			Landscape->updateGlobalsAndLockBuffers(CVector::Null);

		// Recompute UVs for new setup of Tiles.
		Patchs[numPatch].deleteTileUvs();
		Patchs[numPatch].recreateTileUvs();

		// unlockBuffers() if necessary.
		if(!_PatchRenderClipped[numPatch])
		{
			Landscape->unlockBuffers();
			// This patch is visible, and TileFaces have been deleted / added.
			// So must update TessBlock.
			Landscape->updateTessBlocksFaceVector();
		}
	}
}


// ***************************************************************************
void			CZone::refreshTesselationGeometry(sint numPatch)
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());
	nlassert(Compiled);

	// At next render, we must re-fill the entire unclipped VB, so change are taken into account.
	Landscape->_RenderMustRefillVB= true;

	Patchs[numPatch].refreshTesselationGeometry();
}


// ***************************************************************************
const std::vector<CTileElement> &CZone::getPatchTexture(sint numPatch) const
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());

	// Update the patch texture.
	return Patchs[numPatch].Tiles;
}


// ***************************************************************************
const std::vector<CTileColor> &CZone::getPatchColor(sint numPatch) const
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());

	// Update the patch texture.
	return Patchs[numPatch].TileColors;
}

// ***************************************************************************
void CZone::setTileColor(bool monochrome, float factor)
{
	nlassert(factor >= 0.0f); // factor must not be negative as its a multiplier

	if (monochrome)
	{
		for (uint32 i = 0; i < Patchs.size(); ++i)
		{
			vector<CTileColor> &rTC = Patchs[i].TileColors;
			for (uint32 j =  0; j < rTC.size(); ++j)
			{
				float fR = (rTC[j].Color565 & 31) / 32.0f;
				float fG = ((rTC[j].Color565 >> 5) & 63) / 64.0f;
				float fB = ((rTC[j].Color565 >> 11) & 31) / 32.0f;

				fR = 0.28f * fR + 0.59f * fG + 0.13f * fB;

				nlassert(fR < 0.99f);

				fR *= factor;
				if (fR > 0.99f) fR = 0.99f; // Avoid reaching 1

				uint16 nR = (uint16)(fR * 32.0f);
				uint16 nG = (uint16)(fR * 64.0f);
				uint16 nB = (uint16)(fR * 32.0f);

				rTC[j].Color565 = nR + (nG << 5) + (nB << 11);
			}
		}
	}
	else
	{
		if (factor != 1.0f)
		{
			for (uint32 i = 0; i < Patchs.size(); ++i)
			{
				vector<CTileColor> &rTC = Patchs[i].TileColors;
				for (uint32 j =  0; j < rTC.size(); ++j)
				{
					float fR = (rTC[j].Color565 & 31) / 32.0f;
					float fG = ((rTC[j].Color565 >> 5) & 63) / 64.0f;
					float fB = ((rTC[j].Color565 >> 11) & 31) / 32.0f;

					fR *= factor;
					fG *= factor;
					fB *= factor;

					if (fR > 0.99f) fR = 0.99f;
					if (fG > 0.99f) fG = 0.99f;
					if (fB > 0.99f) fB = 0.99f;

					uint16 nR = (uint16)(fR * 32.0f);
					uint16 nG = (uint16)(fG * 64.0f);
					uint16 nB = (uint16)(fB * 32.0f);

					rTC[j].Color565 = nR + (nG << 5) + (nB << 11);
				}
			}
		}
	}
}

// ***************************************************************************
void			CZone::debugBinds(FILE *f)
{
	fprintf(f, "*****************************\n");
	fprintf(f, "ZoneId: %d. NPatchs:%u\n", ZoneId, (uint)PatchConnects.size());
	sint i;
	for(i=0;i<(sint)PatchConnects.size();i++)
	{
		CPatchConnect	&pc= PatchConnects[i];
		fprintf(f, "patch%d:\n", i);
		for(sint j=0;j<4;j++)
		{
			CPatchInfo::CBindInfo	&bd= pc.BindEdges[j];
			fprintf(f, "    edge%d: Zone:%u. NPatchs:%u. ", j, (uint)bd.ZoneId, (uint)bd.NPatchs);
			for(sint k=0;k<bd.NPatchs;k++)
			{
				fprintf(f, "p%ue%u - ", (uint)bd.Next[k], (uint)bd.Edge[k]);
			}
			fprintf(f, "\n");
		}
	}

	fprintf(f,"Vertices :\n");
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		fprintf(f,"current : %u -> (zone %u) vertex %u\n", (uint)BorderVertices[i].CurrentVertex,
											(uint)BorderVertices[i].NeighborZoneId,
											(uint)BorderVertices[i].NeighborVertex);
	}
}


// ***************************************************************************
void			CZone::applyHeightField(const CLandscape &landScape)
{
	sint	i,j;
	vector<CBezierPatch>	patchs;

	// no patch, do nothing.
	if(Patchs.size()==0)
		return;

	// 0. Unpack patchs to Bezier Patchs.
	//===================================
	patchs.resize(Patchs.size());
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];
		CPatch				&pa= Patchs[j];

		// re-Build the uncompressed bezier patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].unpack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].unpack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].unpack(p.Interiors[i], PatchBias, PatchScale);
	}

	// 1. apply heightfield on bezier patchs.
	//===================================
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];

		// apply delta.
		for(i=0;i<4;i++)
			p.Vertices[i]+= landScape.getHeightFieldDeltaZ(p.Vertices[i].x, p.Vertices[i].y);
		for(i=0;i<8;i++)
			p.Tangents[i]+= landScape.getHeightFieldDeltaZ(p.Tangents[i].x, p.Tangents[i].y);
		for(i=0;i<4;i++)
			p.Interiors[i]+= landScape.getHeightFieldDeltaZ(p.Interiors[i].x, p.Interiors[i].y);
	}


	// 2. Re-compute Patch Scale/Bias, and Zone BBox.
	//===================================
	CAABBox		bb;
	bb.setCenter(patchs[0].Vertices[0]);
	bb.setHalfSize(CVector::Null);
	for(j=0;j<(sint)patchs.size();j++)
	{
		// extend bbox.
		const CBezierPatch	&p= patchs[j];
		for(i=0;i<4;i++)
			bb.extend(p.Vertices[i]);
		for(i=0;i<8;i++)
			bb.extend(p.Tangents[i]);
		for(i=0;i<4;i++)
			bb.extend(p.Interiors[i]);
	}
	// Compute BBox, and Patch Scale Bias, according to Noise.
	computeBBScaleBias(bb);


	// 3. Re-pack patchs.
	//===================================
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];
		CPatch				&pa= Patchs[j];

		// Build the packed patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].pack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].pack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].pack(p.Interiors[i], PatchBias, PatchScale);
	}
}

// ***************************************************************************
void CZone::setupColorsFromTileFlags(const NLMISC::CRGBA colors[4])
{
	for (uint k = 0; k < Patchs.size(); ++k)
	{
		Patchs[k].setupColorsFromTileFlags(colors);
	}
}


// ***************************************************************************
void CZone::copyTilesFlags(sint destPatchId, const CPatch *srcPatch)
{
	CPatch *destPatch = getPatch(destPatchId);

	destPatch->copyTileFlagsFromPatch(srcPatch);
}


// ***************************************************************************
bool CPatchInfo::getNeighborTile (uint patchId, uint edge, sint position, uint &patchOut, sint &sOut, sint &tOut,
								  const vector<CPatchInfo> &patchInfos) const
{
	nlassert (edge<4);

	// S or T ?
	uint length = (edge&1) ? OrderS : OrderT;
	nlassert ((uint)position<length);

	// What kind of case ?
	switch (BindEdges[edge].NPatchs)
	{
	case 1:
	case 2:
	case 4:
		{
			// Get neighbor index and position in neighbor
			uint neighborLength = (length / BindEdges[edge].NPatchs);
			uint neighbor = position / neighborLength;
			uint neighborPosition = neighborLength - (position % neighborLength) - 1;
			uint neighborEdge = BindEdges[edge].Edge[neighbor];

			// Patch id
			patchOut = BindEdges[edge].Next[neighbor];

			// Check neighbor
			uint neighborRealLength = (neighborEdge&1) ? patchInfos[patchOut].OrderS : patchInfos[patchOut].OrderT;
			if (neighborRealLength == neighborLength)
			{
				// Get final coordinate
				switch (neighborEdge)
				{
				case 0:
					sOut = 0;
					tOut = neighborPosition;
					break;
				case 1:
					sOut = neighborPosition;
					tOut = patchInfos[patchOut].OrderT-1;
					break;
				case 2:
					sOut = patchInfos[patchOut].OrderS-1;
					tOut = patchInfos[patchOut].OrderT-neighborPosition-1;
					break;
				case 3:
					sOut = patchInfos[patchOut].OrderS-neighborPosition-1;
					tOut = 0;
					break;
				}

				// Ok todo remove
				return true;
			}
		}
		break;

	case 5:
		{
			// Find in the neighbor where we are
			patchOut = BindEdges[edge].Next[0];
			uint neighborEdge = BindEdges[edge].Edge[0];
			uint neighborEdgeCount = patchInfos[patchOut].BindEdges[neighborEdge].NPatchs;

			// Check neighbor
			uint neighborRealLength = (neighborEdge&1) ? patchInfos[patchOut].OrderS : patchInfos[patchOut].OrderT;

			// Good length ?
			if ((neighborRealLength / neighborEdgeCount) == length)
			{
				// Find us in the neighbor
				uint neighborPosition;
				for (neighborPosition=0; neighborPosition<neighborEdgeCount; neighborPosition++)
				{
					// Found ?
					if (patchInfos[patchOut].BindEdges[neighborEdge].Next[neighborPosition] == patchId)
						break;
				}

				// Must be found
				nlassert (neighborPosition!=neighborEdgeCount);
				neighborPosition = (neighborPosition + 1) * (neighborRealLength / neighborEdgeCount) - position - 1;

				// Get final coordinate
				switch (neighborEdge)
				{
				case 0:
					sOut = 0;
					tOut = neighborPosition;
					break;
				case 1:
					sOut = neighborPosition;
					tOut = patchInfos[patchOut].OrderT-1;
					break;
				case 2:
					sOut = patchInfos[patchOut].OrderS-1;
					tOut = patchInfos[patchOut].OrderT-neighborPosition-1;
					break;
				case 3:
					sOut = patchInfos[patchOut].OrderS-neighborPosition-1;
					tOut = 0;
					break;
				}

				// Ok
				return true;
			}
		}
		break;
	}

	return false;
}


// ***************************************************************************

bool CPatchInfo::getTileSymmetryRotate (const CTileBank &bank, uint tile, bool &symmetry, uint &rotate)
{
	// Need check the tile ?
	if ( (symmetry || (rotate != 0)) && (tile != 0xffffffff) )
	{
		// Tile exist ?
		if (tile < (uint)bank.getTileCount())
		{
			// Get xref
			int tileSet;
			int number;
			CTileBank::TTileType type;

			// Get tile xref
			bank.getTileXRef ((int)tile, tileSet, number, type);

			if ((tileSet < 0) || (tileSet >= bank.getTileSetCount()))
			{
				nlwarning("tile %d has an unknown tileSet (%d)",tile, tileSet);
				return false;
			}

			// Is it an oriented tile ?
			if (bank.getTileSet (tileSet)->getOriented())
			{
				// New rotation value
				rotate = 0;
			}

			// Ok
			return true;
		}

		return false;
	}
	else
		return true;
}

// ***************************************************************************

bool CPatchInfo::transformTile (const CTileBank &bank, uint &tile, uint &tileRotation, bool symmetry, uint rotate, bool goofy)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		if (tile < (uint)bank.getTileCount())
		{
			// Get xref
			int tileSet;
			int number;
			CTileBank::TTileType type;

			// Get tile xref
			bank.getTileXRef ((int)tile, tileSet, number, type);

			// Transition ?
			if (type == CTileBank::transition)
			{
				// Rotation for transition
				uint transRotate = rotate;

				// Number should be ok
				nlassert (number>=0);
				nlassert (number<CTileSet::count);

				// Tlie set number
				const CTileSet *pTileSet = bank.getTileSet (tileSet);

				// Get border desc
				CTileSet::TFlagBorder oriented[4] =
				{
					pTileSet->getOrientedBorder (CTileSet::left, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::left)),
					pTileSet->getOrientedBorder (CTileSet::bottom, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::bottom)),
					pTileSet->getOrientedBorder (CTileSet::right, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::right)),
					pTileSet->getOrientedBorder (CTileSet::top, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::top))
				};

				// Symmetry ?
				if (symmetry)
				{
					if ( (tileRotation & 1) ^ goofy )
					{
						CTileSet::TFlagBorder tmp = oriented[1];
						oriented[1] = CTileSet::getInvertBorder (oriented[3]);
						oriented[3] = CTileSet::getInvertBorder (tmp);
						oriented[2] = CTileSet::getInvertBorder (oriented[2]);
						oriented[0] = CTileSet::getInvertBorder (oriented[0]);
					}
					else
					{
						CTileSet::TFlagBorder tmp = oriented[0];
						oriented[0] = CTileSet::getInvertBorder (oriented[2]);
						oriented[2] = CTileSet::getInvertBorder (tmp);
						oriented[1] = CTileSet::getInvertBorder (oriented[1]);
						oriented[3] = CTileSet::getInvertBorder (oriented[3]);
					}
				}

				// Rotation
				CTileSet::TFlagBorder edges[4];
				edges[0] = pTileSet->getOrientedBorder (CTileSet::left, oriented[(0 + transRotate )&3]);
				edges[1] = pTileSet->getOrientedBorder (CTileSet::bottom, oriented[(1 + transRotate )&3]);
				edges[2] = pTileSet->getOrientedBorder (CTileSet::right, oriented[(2 + transRotate )&3]);
				edges[3] = pTileSet->getOrientedBorder (CTileSet::top, oriented[(3 + transRotate )&3]);

				// Get the good tile number
				CTileSet::TTransition transition = pTileSet->getTransitionTile (edges[3], edges[1], edges[0], edges[2]);
				nlassert ((CTileSet::TTransition)transition != CTileSet::notfound);
				tile = (uint)(pTileSet->getTransition (transition)->getTile ());
			}

			// Transform rotation: invert rotation
			tileRotation += rotate;

			// If goofy, add +2
			if (goofy && symmetry)
				tileRotation += 2;

			// Mask the rotation
			tileRotation &= 3;
		}
		else
			return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

void CPatchInfo::transform256Case (const CTileBank &bank, uint8 &case256, uint tileRotation, bool symmetry, uint rotate, bool goofy)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		// Symmetry ?
		if (symmetry)
		{
			// Take the symmetry
			uint symArray[4] = {3, 2, 1, 0};
			case256 = symArray[case256];

			if (goofy && ((tileRotation & 1) ==0))
				case256 += 2;
			if ((!goofy) && (tileRotation & 1))
				case256 += 2;
		}

		// Rotation ?
		case256 -= rotate;
		case256 &= 3;
	}
}

// ***************************************************************************

bool CPatchInfo::transform (std::vector<CPatchInfo> &patchInfo, NL3D::CZoneSymmetrisation &zoneSymmetry, const NL3D::CTileBank &bank, bool symmetry, uint rotate, float snapCell, float weldThreshold, const NLMISC::CMatrix &toOriginalSpace)
{
	uint patchCount = (uint)patchInfo.size ();
	uint i;

	// --- Export tile info Symmetry of the bind info.
	// --- Parse each patch and each edge

	// For each patches
	NL3D::CZoneSymmetrisation::CError error;

	// Build the structure
	if (!zoneSymmetry.build (patchInfo, snapCell, weldThreshold, bank, error, toOriginalSpace))
	{
		return false;
	}

	// Symmetry ?
	if (symmetry)
	{
		for(i=0 ; i<patchCount; i++)
		{
			// Ref on the current patch
			CPatchInfo &pi = patchInfo[i];

			// --- Symmetry vertex indexes

			// Vertices
			CVector tmp = pi.Patch.Vertices[0];
			pi.Patch.Vertices[0] = pi.Patch.Vertices[3];
			pi.Patch.Vertices[3] = tmp;
			tmp = pi.Patch.Vertices[1];
			pi.Patch.Vertices[1] = pi.Patch.Vertices[2];
			pi.Patch.Vertices[2] = tmp;

			// Tangents
			tmp = pi.Patch.Tangents[0];
			pi.Patch.Tangents[0] = pi.Patch.Tangents[5];
			pi.Patch.Tangents[5] = tmp;
			tmp = pi.Patch.Tangents[1];
			pi.Patch.Tangents[1] = pi.Patch.Tangents[4];
			pi.Patch.Tangents[4] = tmp;
			tmp = pi.Patch.Tangents[2];
			pi.Patch.Tangents[2] = pi.Patch.Tangents[3];
			pi.Patch.Tangents[3] = tmp;
			tmp = pi.Patch.Tangents[6];
			pi.Patch.Tangents[6] = pi.Patch.Tangents[7];
			pi.Patch.Tangents[7] = tmp;

			// Interior
			tmp = pi.Patch.Interiors[0];
			pi.Patch.Interiors[0] = pi.Patch.Interiors[3];
			pi.Patch.Interiors[3] = tmp;
			tmp = pi.Patch.Interiors[1];
			pi.Patch.Interiors[1] = pi.Patch.Interiors[2];
			pi.Patch.Interiors[2] = tmp;

			// ** Symmetries tile colors

			uint u,v;
			uint countU = pi.OrderS/2+1;
			uint countV = pi.OrderT+1;
			for (v=0; v<countV; v++)
			for (u=0; u<countU; u++)
			{
				// Store it in the tile info
				uint index0 = u+v*(pi.OrderS+1);
				uint index1 = (pi.OrderS-u)+v*(pi.OrderS+1);

				// XChg
				uint16 tmp = pi.TileColors[index0].Color565;
				pi.TileColors[index0].Color565 = pi.TileColors[index1].Color565;
				pi.TileColors[index1].Color565 = tmp;
			}

			// Smooth flags
			uint flags = (uint)(pi.getSmoothFlag (0))<<2;
			flags |= (uint)(pi.getSmoothFlag (2))<<0;
			flags |= (uint)(pi.getSmoothFlag (1))<<1;
			flags |= (uint)(pi.getSmoothFlag (3))<<3;
			pi.Flags &= ~3;
			pi.Flags |= flags;
		}

		// --- Symmetry of the bind info.
		// --- Parse each patch and each edge
		// For each patches
		for (i=0 ; i<patchCount; i++)
		{
			// Ref on the patch info
			CPatchInfo &pi = patchInfo[i];

			// Xchg left and right
			swap (pi.BindEdges[0], pi.BindEdges[2]);
			swap (pi.BaseVertices[0], pi.BaseVertices[3]);
			swap (pi.BaseVertices[1], pi.BaseVertices[2]);

			// Flip edges
			for (uint edge=0; edge<4; edge++)
			{
				// Ref on the patch info
				CPatchInfo::CBindInfo &bindEdge = pi.BindEdges[edge];

				uint next;
				// Look if it is a bind ?
				if ( (bindEdge.NPatchs>1) && (bindEdge.NPatchs!=5) )
				{
					for (next=0; next<(uint)bindEdge.NPatchs/2; next++)
					{
						swap (bindEdge.Next[bindEdge.NPatchs - next - 1], bindEdge.Next[next]);
						swap (bindEdge.Edge[bindEdge.NPatchs - next - 1], bindEdge.Edge[next]);
					}
				}

				// Look if we are binded on a reversed edge
				uint bindCount = (bindEdge.NPatchs==5) ? 1 : bindEdge.NPatchs;
				for (next=0; next<bindCount; next++)
				{
					// Left or right ?
					if ( (bindEdge.Edge[next] & 1) == 0)
					{
						// Invert
						bindEdge.Edge[next] += 2;
						bindEdge.Edge[next] &= 3;
					}
				}
			}
		}
	}

	// For each patches
	for (i=0 ; i<patchCount; i++)
	{
		// Tile infos
		CPatchInfo &pi = patchInfo[i];

		// Backup tiles
		std::vector<CTileElement>	tiles = pi.Tiles;

		int u,v;
		for (v=0; v<pi.OrderT; v++)
		for (u=0; u<pi.OrderS; u++)
		{
			// U tile
			int uSymmetry = symmetry ? (pi.OrderS-u-1) : u;

			// Destination tile
			CTileElement &element = pi.Tiles[u+v*pi.OrderS];

			// Copy the orginal symmetrical element
			element = tiles[uSymmetry+v*pi.OrderS];

			// For each layer
			for (int l=0; l<3; l++)
			{
				// Empty ?
				if (element.Tile[l] != 0xffff)
				{
					// Get the tile index
					uint tile = element.Tile[l];
					uint tileRotation = element.getTileOrient (l);

					// Get rot and symmetry for this tile
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;
					bool goofy = symmetry && (zoneSymmetry.getTileState (i, uSymmetry+v*pi.OrderS, l) == CZoneSymmetrisation::Goofy);

					// Transform the transfo
					if (getTileSymmetryRotate (bank, tile, tileSymmetry, tileRotate))
					{
						// Transform the tile
						if (!transformTile (bank, tile, tileRotation, tileSymmetry, (4-tileRotate)&3, goofy))
						{
							// Info
							nlwarning ("Error getting symmetrical / rotated zone tile.");
							return false;
						}
					}
					else
					{
						// Info
						nlwarning ("Error getting symmetrical / rotated zone tile.");
						return false;
					}

					// Set the tile
					element.Tile[l] = tile;
					element.setTileOrient (l, (uint8)tileRotation);
				}
			}

			// Empty ?
			if (element.Tile[0]!=0xffff)
			{
				// Get 256 info
				bool is256x256;
				uint8 uvOff;
				element.getTile256Info (is256x256, uvOff);

				// 256 ?
				if (is256x256)
				{
					// Get rot and symmetry for this tile
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;
					uint tileRotation = tiles[uSymmetry+v*pi.OrderS].getTileOrient (0);
					bool goofy = symmetry && (zoneSymmetry.getTileState (i, uSymmetry+v*pi.OrderS, 0) == CZoneSymmetrisation::Goofy);

					// Transform the transfo
					getTileSymmetryRotate (bank, element.Tile[0], tileSymmetry, tileRotate);

					// Transform the case
					transform256Case (bank, uvOff, tileRotation, tileSymmetry, (4-tileRotate)&3, goofy);

					element.setTile256Info (true, uvOff);
				}
			}
		}
	}

	// Ok
	return true;
}

// ***************************************************************************

} // NL3D
