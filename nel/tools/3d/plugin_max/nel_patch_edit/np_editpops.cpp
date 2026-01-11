/**********************************************************************
 *<
	FILE: editsops.cpp

	DESCRIPTION:  Edit Shape OSM operations

	CREATED BY: Tom Hudson & Rolf Berteig

	HISTORY: created 25 April, 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"
#include "editpat.h"

extern CoreExport Class_ID patchClassID; 

// in mods.cpp
extern HINSTANCE hInstance;

/*-------------------------------------------------------------------*/

static void XORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, RGB(255,255,255) ) );
	MoveToEx( hdc, p0.x, p0.y, NULL );
	LineTo( hdc, p1.x, p1.y );		
	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}

/*-------------------------------------------------------------------*/

EPTempData::~EPTempData()
	{
	if (patch) delete patch;
	if (rpatch) delete rpatch;
	}

EPTempData::EPTempData(EditPatchMod *m,EditPatchData *pd)
	{
	patch = NULL;
	rpatch = NULL;
	patchValid.SetEmpty();
	patchData = pd;
	mod = m;
	}

void EPTempData::Invalidate(PartID part,BOOL patchValid)
	{
	if ( !patchValid ) 
	{
		delete patch;
		patch = NULL;
		delete rpatch;
		rpatch = NULL;
	}
	if ( part & PART_TOPO ) 
	{
		if (rpatch)
			rpatch->InvalidateChannels (PART_TOPO|PART_GEOM|PART_SELECT|TEXMAP_CHANNEL);
	}
	if ( part & PART_GEOM ) 
	{
		if (rpatch)
			rpatch->InvalidateChannels (PART_GEOM);
	}
	if ( part & PART_SELECT ) 
	{
		if (rpatch)
			rpatch->InvalidateChannels (PART_SELECT);
	}
}

PatchMesh *EPTempData::GetPatch(TimeValue t, RPatchMesh *&rPatch)
	{
		if ( patchValid.InInterval(t) && patch ) 
		{
			rPatch=rpatch;
			return patch;
		} 
		else 
		{
			patchData->SetFlag(EPD_UPDATING_CACHE,TRUE);
			mod->NotifyDependents(Interval(t,t), 
				PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO,
				REFMSG_MOD_EVAL);
			patchData->SetFlag(EPD_UPDATING_CACHE,FALSE);
			rPatch=rpatch;
			return patch;
		}
	}

BOOL EPTempData::PatchCached(TimeValue t)
{
	return (patchValid.InInterval(t) && patch);
}

void EPTempData::UpdateCache(RPO *patchOb)
{
	if ( patch ) delete patch;
	if ( rpatch ) delete rpatch;
	patch = new PatchMesh(patchOb->patch);
	rpatch = new RPatchMesh(*patchOb->rpatch);

	patchValid = FOREVER;
	
	// These are the channels we care about.
	patchValid &= patchOb->ChannelValidity(0,GEOM_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,TOPO_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,SELECT_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,SUBSEL_TYPE_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,DISP_ATTRIB_CHAN_NUM);	
	patchValid &= patchOb->ChannelValidity(0,TEXMAP_CHAN_NUM);	
}

