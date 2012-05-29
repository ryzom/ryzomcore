#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// watje unhide all
static void UnHidePatches(PatchMesh *patch) 
{
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	int i;
	for (i = 0; i < patches; i++)
	{
		if (patch->patches[i].IsHidden())
			patch->patches[i].SetHidden(FALSE);
	}
	int verts = patch->numVerts;
	for (i = 0; i < verts; i++)
	{
		if (patch->verts[i].IsHidden())
			patch->verts[i].SetHidden(FALSE);
	}
}

// --------------------------------------------------------------------------------

// watje 12-10-98
void EditPatchMod::DoUnHide() {
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		BOOL altered = FALSE;
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
		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
//		if(patch->patchSel.NumberSet()) {
		if (1)
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
			UnHidePatches(patch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoHide(int type) 
{
	switch (type)
	{
		case EP_VERTEX:
			DoVertHide();
			break;
		case EP_EDGE:
			DoEdgeHide();
			break;
		case EP_PATCH:
			DoPatchHide();
			break;
		}
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// watje
static void FixUpVerts(PatchMesh *patch) 
{
	int patches = patch->numPatches;
	for (int i = 0; i < patches; i++)
	{
		
		if (!(patch->patches[i].IsHidden()))
		{
			int ct = 4;
			if (patch->patches[i].type == PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
			{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(FALSE);
			}
			
		}
	}
	
}

// --------------------------------------------------------------------------------

// watje hide patch
static void HidePatches(PatchMesh *patch) 
{
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	BitArray &psel = patch->patchSel;
	for (int i = 0; i < patches; i++)
	{
		if (psel[i])
		{
			patch->patches[i].SetHidden(TRUE);
			// hide all 
			int ct = 4;
			if (patch->patches[i].type == PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
			{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
			}
		}
	}
	FixUpVerts(patch);
}

// --------------------------------------------------------------------------------

void EditPatchMod::DoPatchHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		BOOL altered = FALSE;
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
		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->patchSel.NumberSet())
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
			HidePatches(patch);
			patch->patchSel.ClearAll();
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);

	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// watje hide patches by verts
static void HideVerts(PatchMesh *patch) 
{
	// If positive vertex number, do it to just one vertex
	int patches = patch->numPatches;
	BitArray &vsel = patch->vertSel;
	int i;
	for (i = 0; i < patches; i++)
	{
		int ct = 4;
		if (patch->patches[i].type == PATCH_TRI)
			ct = 3;
		for (int k = 0; k < ct; k++)
		{
			int a = patch->patches[i].v[k];
			
			if (vsel[a])
			{
				patch->patches[i].SetHidden(TRUE);
			}
		}
	}
	for (i = 0; i < patches; i++)
	{
		if (patch->patches[i].IsHidden())
		{
			// hide all 
			int ct = 4;
			if (patch->patches[i].type == PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
			{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
			}
		}
	}
	
	FixUpVerts(patch);
}

// --------------------------------------------------------------------------------

void EditPatchMod::DoVertHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		BOOL altered = FALSE;
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
		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->vertSel.NumberSet())
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
			HideVerts(patch);
			patch->vertSel.ClearAll();
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);

	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// watje hide patches by verts
static void HideEdges(PatchMesh *patch) 
{
	// If positive vertex number, do it to just one vertex
	int edges = patch->numEdges;
	BitArray &esel = patch->edgeSel;
	int i;
	for (i = 0; i < edges; i++)
	{
		if (esel[i])
		{
#if (MAX_RELEASE < 4000)
			int a = patch->edges[i].patch1;
			int b = patch->edges[i].patch2;
#else // (MAX_RELEASE < 4000)
			int a = patch->edges[i].patches[0];
			int b = patch->edges[i].patches[1];
#endif // (MAX_RELEASE < 4000)
			if (a>0)
				patch->patches[a].SetHidden(TRUE);
			if (b>0)
				patch->patches[b].SetHidden(TRUE);
		}
	}
	int patches = patch->numPatches;
	for (i = 0; i < patches; i++)
	{
		if (patch->patches[i].IsHidden())
		{
			// hide all 
			int ct = 4;
			if (patch->patches[i].type == PATCH_TRI)
				ct = 3;
			for (int k = 0; k < ct; k++)
			{
				int a = patch->patches[i].v[k];
				patch->verts[a].SetHidden(TRUE);
			}
		}
	}
	FixUpVerts(patch);
}

// --------------------------------------------------------------------------------

void EditPatchMod::DoEdgeHide() 
	{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		BOOL altered = FALSE;
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
		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
		if (patch->edgeSel.NumberSet())
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
			HideEdges(patch);
			patch->edgeSel.ClearAll();
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
		}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);

	}
