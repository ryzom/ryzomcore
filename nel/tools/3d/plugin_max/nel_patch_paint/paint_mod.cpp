#include "stdafx.h"
#include "nel_patch_paint.h"
#include "nel/3d/zone_symmetrisation.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME 2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IObjParam*          PaintPatchMod::ip              = NULL;
HWND				PaintPatchMod::hOpsPanel       = NULL;
EPM_PaintCMode*		PaintPatchMod::paintMode   = NULL;

int					PaintPatchMod::channelModified = EDITPAT_CHANNELS;

bool				PaintPatchMod::ShowCurrentState	= false;
uint				PaintPatchMod::CurrentState	= (uint)CZoneSymmetrisation::Nothing;
int					PaintPatchMod::CurrentTileSet	= -1;
int					PaintPatchMod::brushSize		= 0;		// Default 1 tile
int					PaintPatchMod::ColorBushSize	= 0;
int					PaintPatchMod::tileSize			= 1;		// Default 256
bool				PaintPatchMod::additiveTile		= false;	// 
int					PaintPatchMod::TileGroup		= 0;		// Default all tiles
int					PaintPatchMod::DisplaceTile		= 0;		// Default displace 0
int					PaintPatchMod::DisplaceTileSet	= -1;		// 
uint				PaintPatchMod::TileFillRotation	= 0;
bool				PaintPatchMod::TileTrick		= false;
bool				PaintPatchMod::automaticLighting= false;
bool				PaintPatchMod::lockBorders		= false;
BOOL				PaintPatchMod::rsOps			= TRUE;

Interval PaintPatchMod::LocalValidity(TimeValue t)
{
	// Force a cache if being edited.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  			   
	return FOREVER;
}

RefTargetHandle PaintPatchMod::Clone(RemapDir& remap) 
{
	PaintPatchMod* newmod = new PaintPatchMod();	
	newmod->includeMeshes = includeMeshes;
	newmod->preloadTiles = preloadTiles;
	return (newmod);
}

void PaintPatchMod::ClearPatchDataFlag(ModContextList& mcList, DWORD f)
{
	for (int i = 0; i < mcList.Count(); i++)
	{
		PaintPatchData *patchData =(PaintPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		patchData->SetFlag(f, FALSE);
	}
}

void PaintPatchMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{		
	// Alert(_T("in ModifyObject"));
	nlassert(os->obj->IsSubClassOf(RYKOLPATCHOBJ_CLASS_ID));
	// Alert(_T("ModifyObject class ID is OK"));
	
	RPO *patchOb =(RPO*)os->obj;
	PaintPatchData *patchData;

	if (!mc.localData)
	{
		mc.localData = new PaintPatchData (this);
		patchData =(PaintPatchData*)mc.localData;
	} else 
	{
		patchData =(PaintPatchData*)mc.localData;
	}
	
	PatchMesh &pmesh = patchOb->patch;
	nlassert(pmesh.numVerts == pmesh.vertSel.GetSize());
	nlassert(pmesh.getNumEdges() == pmesh.edgeSel.GetSize());
	nlassert(pmesh.numPatches == pmesh.patchSel.GetSize());
	
	patchData->Apply(t, patchOb, 0 /* selLevel */);
}

void PaintPatchMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if (mc->localData)
	{
		PaintPatchData *patchData =(PaintPatchData*)mc->localData;
		if (patchData)
		{
			// The FALSE parameter indicates the the mesh cache itself is
			// invalid in addition to any other caches that depend on the
			// mesh cache.
			patchData->Invalidate(partID, FALSE);
		}
	}
}

BOOL PaintPatchMod::DependOnTopology(ModContext &mc)
{
	PaintPatchData *patchData =(PaintPatchData*)mc.localData;
	if (patchData)
	{
		if (patchData->GetFlag(EPD_HASDATA))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void PaintPatchMod::DeletePatchDataTempData()
{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;		
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		PaintPatchData *patchData =(PaintPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;				
		if (patchData->tempData)
		{
			delete patchData->tempData;
		}
		patchData->tempData = NULL;
	}
	nodes.DisposeTemporary();
}


void PaintPatchMod::CreatePatchDataTempData()
{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;		
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		PaintPatchData *patchData =(PaintPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;				
		if (!patchData->tempData)
		{
			patchData->tempData = new EPTempData(this, patchData);
		}		
	}
	nodes.DisposeTemporary();
}
