#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void ChangePatchType(PatchMesh *patch, int index, int type);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::RememberPatchThere(HWND hWnd, IPoint2 m) 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	
	// Initialize so there isn't any remembered patch
	rememberedPatch = NULL;
	
	if (!ip)
		return 0;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	// See if we're over a patch
#if MAX_VERSION_MAJOR >= 19
	ViewExp *vpt = &ip->GetViewExp(hWnd);
#else
	ViewExp *vpt = ip->GetViewport(hWnd);
#endif
	GraphicsWindow *gw = vpt->getGW();

	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubPatchHitList hitList;
	
	int result = 0;
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		patch->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_PATCH_PATCHES/* | HIT_ABORTONHIT*/, hitList);
		PatchSubHitRec *hit = hitList.First();
		if (hit)
		{
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while (hit) 
			{
				if (patch->patchSel[hit->index])
				{
					if (patch->SelPatchesSameType())
					{
						rememberedPatch = NULL;
						rememberedData = patch->patches[hit->index].flags &(~PATCH_INTERIOR_MASK);
						goto finish;
					}
					// Selected patches not all the same type!
					rememberedPatch = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
				}
				hit = hit->Next();
			}
			if (ip->SelectionFrozen())
				goto finish;
			// Select just this patch
			hit = hitList.First();
			theHold.Begin();
			if (theHold.Holding())
				theHold.Put(new PatchSelRestore(patchData, this, patch));
			patch->patchSel.ClearAll();
			patch->patchSel.Set(hit->index);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			theHold.Accept(GetString(IDS_DS_SELECT));
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
			PatchSelChanged();
			
			rememberedPatch = patch;
			rememberedIndex = hit->index;
			rememberedData = patch->patches[rememberedIndex].flags &(~PATCH_INTERIOR_MASK);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
finish:
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

#if MAX_VERSION_MAJOR < 19
	if (vpt)
		ip->ReleaseViewport(vpt);
#endif

	return result;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::ChangeRememberedPatch(int type) 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		if (patch == rememberedPatch)
		{
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);
			
			theHold.Begin();
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, _T("ChangeRememberedPatch")));
			// Call the patch type change function
			ChangePatchType(patch, rememberedIndex, type);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
			ClearPatchDataFlag(mcList, EPD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::RememberVertThere(HWND hWnd, IPoint2 m) 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	
	// Initialize so there isn't any remembered patch
	rememberedPatch = NULL;
	
	if (!ip)
		return 0;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	// See if we're over a vertex
#if MAX_VERSION_MAJOR >= 19
	ViewExp *vpt = &ip->GetViewExp(hWnd);
#else
	ViewExp *vpt = ip->GetViewport(hWnd);
#endif
	GraphicsWindow *gw = vpt->getGW();

	HitRegion hr;
	MakeHitRegion(hr, HITTYPE_POINT, 1, 4, &m);
	gw->setHitRegion(&hr);
	SubPatchHitList hitList;
	
	int result = 0;
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		INode *inode = nodes[i];
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		patch->SubObjectHitTest(gw, gw->getMaterial(), &hr, SUBHIT_PATCH_VERTS/* | HIT_ABORTONHIT*/, hitList);
		PatchSubHitRec *hit = hitList.First();
		if (hit)
		{
			result = 1;
			// Go thru the list and see if we have one that's selected
			// If more than one selected and they're different types, set unknown type
			hit = hitList.First();
			while (hit) 
			{
				if (patch->vertSel[hit->index])
				{
					if (patch->SelVertsSameType())
					{
						rememberedPatch = NULL;
						rememberedData = patch->verts[hit->index].flags &(~PVERT_TYPE_MASK);
						goto finish;
					}
					// Selected verts not all the same type!
					rememberedPatch = NULL;
					rememberedData = -1;	// Not all the same!
					goto finish;
				}
				hit = hit->Next();
			}
			if (ip->SelectionFrozen())
				goto finish;
			// Select just this vertex
			hit = hitList.First();
			theHold.Begin();
			if (theHold.Holding())
				theHold.Put(new PatchSelRestore(patchData, this, patch));
			patch->vertSel.ClearAll();
			patch->vertSel.Set(hit->index);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			theHold.Accept(GetString(IDS_DS_SELECT));
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
			PatchSelChanged();
			
			rememberedPatch = patch;
			rememberedIndex = hit->index;
			rememberedData = patch->verts[rememberedIndex].flags &(~PVERT_TYPE_MASK);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
finish:
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

#if MAX_VERSION_MAJOR < 19
	if (vpt)
		ip->ReleaseViewport(vpt);
#endif
	return result;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::ChangeRememberedVert(int type) 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		
		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			continue;
		if (patch == rememberedPatch)
		{
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);
			
			theHold.Begin();
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, _T("ChangeRememberedVert")));
			// Call the vertex type change function
			patch->ChangeVertType(rememberedIndex, type);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			theHold.Accept(GetString(IDS_TH_VERTCHANGE));
			ClearPatchDataFlag(mcList, EPD_BEENDONE);
			NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
			nodes.DisposeTemporary();
			return;
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

