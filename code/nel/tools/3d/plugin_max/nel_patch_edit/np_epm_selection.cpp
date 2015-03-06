#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

INT_PTR CALLBACK PatchSurfDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchTileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchEdgeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void ChangePatchType(PatchMesh *patch, int index, int type);
extern BOOL filterVerts;
// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::GetSubobjectLevel()
{
	return selLevel;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSubobjectLevel(int level)
{
	selLevel = level;
	if (hSelectPanel)
		RefreshSelType();
	// Setup named selection sets	
	SetupNamedSelDropDown();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

static int butIDs[] = { 0, EP_VERTEX, EP_EDGE, EP_PATCH, EP_TILE };

void EditPatchMod::RefreshSelType() 
{
	if (!hSelectPanel)
		return;
	if (hOpsPanel)
	{
		// Set up or remove the surface properties rollup if needed
		if (hSurfPanel)
		{
			rsSurf = IsRollupPanelOpen(hSurfPanel);
			ip->DeleteRollupPage(hSurfPanel);
			hSurfPanel = NULL;
		}
		if (hTilePanel)
		{
			rsTile = IsRollupPanelOpen(hTilePanel);
			ip->DeleteRollupPage(hTilePanel);
			hTilePanel = NULL;
		}
		if (hEdgePanel)
		{
			rsEdge = IsRollupPanelOpen(hEdgePanel);
			ip->DeleteRollupPage(hEdgePanel);
			hEdgePanel = NULL;
		}
		/* watje 3 - 18 - 99
		if (selLevel == EP_OBJECT)
		{
		hSurfPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF_OBJ),
		PatchObjSurfDlgProc, GetString(IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
		}
		else
		*/
		if (selLevel == EP_PATCH)
		{
			hSurfPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDPATCH_SURF),
				PatchSurfDlgProc, GetString(IDS_TH_SURFACEPROPERTIES), (LPARAM) this, rsSurf ? 0 : APPENDROLL_CLOSED);
		}
		if (selLevel == EP_TILE)
		{
			hTilePanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDPATCH_TILE),
				PatchTileDlgProc, "Tile Properties", (LPARAM) this, rsTile ? 0 : APPENDROLL_CLOSED);
		}
		if (selLevel == EP_EDGE)
		{
			hEdgePanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDPATCH_EDGE),
				PatchEdgeDlgProc, "Edge Properties", (LPARAM) this, rsEdge ? 0 : APPENDROLL_CLOSED);
		}
		SetSurfDlgEnables();
		SetTileDlgEnables();
		SetEdgeDlgEnables();
	}
	
	ICustToolbar *iToolbar = GetICustToolbar(GetDlgItem(hSelectPanel, IDC_SELTYPE));
	ICustButton *but;
	for (int i = 1; i < 5; i++)
	{
		but = iToolbar->GetICustButton(butIDs[i]);
		but->SetCheck(GetSubobjectLevel() == i);
		ReleaseICustButton(but);
	}
	ReleaseICustToolbar(iToolbar);
	SetSelDlgEnables();
	SetOpsDlgEnables();
	UpdateSelectDisplay();
	PatchSelChanged();
}

void EditPatchMod::SelectionChanged() 
{
	if (hSelectPanel)
	{
		UpdateSelectDisplay();
		InvalidateRect(hSelectPanel, NULL, FALSE);
	}
	// Now see if the selection set matches one of the named selections!
	if (ip &&(selLevel != EP_OBJECT)&&(selLevel != EP_TILE))
	{
		ModContextList mcList;		
		INodeTab nodes;
		TimeValue t = ip->GetTime();
		ip->GetModContexts(mcList, nodes);
		int sublevel = selLevel - 1;
		int dataSet;
		for (int set = 0; set < namedSel[sublevel].Count(); ++set)
		{
			ClearPatchDataFlag(mcList, EPD_BEENDONE);
			BOOL gotMatch = FALSE;
			for (int i = 0; i < mcList.Count(); i++)
			{
				EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
				if (!patchData)
					continue;
				if (patchData->GetFlag(EPD_BEENDONE))
					continue;
				RPatchMesh *rpatch;
				PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
				if (!patch)
					continue;
				// See if this patch has the named selection set
				switch (selLevel)
				{
				case EP_VERTEX: 
					for (dataSet = 0; dataSet < patchData->vselSet.Count(); ++dataSet)
					{
						if (*(patchData->vselSet.names[dataSet]) == *namedSel[sublevel][set])
						{
							if (!(*patchData->vselSet.sets[set] == patch->vertSel))
								goto next_set;
							gotMatch = TRUE;
							break;
						}
					}
					break;
				case EP_EDGE:
					for (dataSet = 0; dataSet < patchData->eselSet.Count(); ++dataSet)
					{
						if (*(patchData->eselSet.names[dataSet]) == *namedSel[sublevel][set])
						{
							if (!(*patchData->eselSet.sets[set] == patch->edgeSel))
								goto next_set;
							gotMatch = TRUE;
							break;
						}
					}
					break;
				case EP_PATCH:
					for (dataSet = 0; dataSet < patchData->pselSet.Count(); ++dataSet)
					{
						if (*(patchData->pselSet.names[dataSet]) == *namedSel[sublevel][set])
						{
							if (!(*patchData->pselSet.sets[set] == patch->patchSel))
								goto next_set;
							gotMatch = TRUE;
							break;
						}
					}
					break;
				}
				patchData->SetFlag(EPD_BEENDONE, TRUE);
			}
			// If we reach here, we might have a set that matches
			if (gotMatch)
			{
				ip->SetCurNamedSelSet(*namedSel[sublevel][set]);
				goto namedSelUpdated;
			}
next_set:;
		}
		// No set matches, clear the named selection
		ip->ClearCurNamedSelSet();
		
		
namedSelUpdated:
		nodes.DisposeTemporary();
		ClearPatchDataFlag(mcList, EPD_BEENDONE);
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSelDlgEnables() 
{
	if (!hSelectPanel)
		return;
	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel, IDC_NS_PASTE));
	but->Disable();
	switch (GetSubobjectLevel())
	{
		case PO_VERTEX:
			if (GetPatchNamedSelClip(CLIP_P_VERT))
				but->Enable();
			break;
		case PO_EDGE:
			if (GetPatchNamedSelClip(CLIP_P_EDGE))
				but->Enable();
			break;
		case PO_PATCH:
			if (GetPatchNamedSelClip(CLIP_P_PATCH))
				but->Enable();
			break;
		case PO_TILE:
			break;
		}
	ReleaseICustButton(but);
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

DWORD EditPatchMod::GetSelLevel()
{
	return GetSubobjectLevel();
}

void EditPatchMod::SetSelLevel(DWORD level)
{
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SelectSubPatch(int index)
{
	if (!ip)
		return; 
	TimeValue t = ip->GetTime();

	ip->ClearCurNamedSelSet();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
	
		if (!patchData)
			return;
		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			return;
		
		patchData->BeginEdit(t);
		if (theHold.Holding()) 
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SelectSubComponent"));

		patch->patchSel.Set(index);

		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
		{
			patchData->tempData->Invalidate(PART_SELECT);
		}
	}
	PatchSelChanged();
		
	UpdateSelectDisplay();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// Select a subcomponent within our object(s).  WARNING! Because the HitRecord list can
// indicate any of the objects contained within the group of patches being edited, we need
// to watch for control breaks in the patchData pointer within the HitRecord!

	void EditPatchMod::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
{
	// Don't do anything if at vertex level with verts turned off
	if (selLevel == EP_VERTEX && !filterVerts)
		return;

	if (!ip)
		return; 
	TimeValue t = ip->GetTime();

	ip->ClearCurNamedSelSet();

	// Keep processing hit records as long as we have them!
	while (hitRec) 
	{	
		EditPatchData *patchData =(EditPatchData*)hitRec->modContext->localData;
		
		if (!patchData)
			return;
		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		if (!patch)
			return;
		
		patchData->BeginEdit(t);
		if (theHold.Holding()) 
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SelectSubComponent"));
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				if (all)
				{				
					if (invert)
					{
						while (hitRec) 
						{
							// If the object changes, we're done!
							if (patchData !=(EditPatchData*)hitRec->modContext->localData)
								goto vert_done;
							int index =((PatchHitData *)(hitRec->hitData))->index;
							if (((PatchHitData *)(hitRec->hitData))->type == PATCH_HIT_VERTEX)
							{
								if (patch->vertSel[index])
									patch->vertSel.Clear(index);
								else
									patch->vertSel.Set(index);
							}
							hitRec = hitRec->Next();
						}
					}
					else
						if (selected)
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto vert_done;
								PatchHitData *hit =(PatchHitData *)(hitRec->hitData);
								if (hit->type == PATCH_HIT_VERTEX)
									patch->vertSel.Set(hit->index);
								hitRec = hitRec->Next();
							}
						}
						else 
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto vert_done;
								PatchHitData *hit =(PatchHitData *)(hitRec->hitData);
								if (hit->type == PATCH_HIT_VERTEX)
									patch->vertSel.Clear(hit->index);
								hitRec = hitRec->Next();
							}
						}
				}
				else 
				{
					int index =((PatchHitData *)(hitRec->hitData))->index;
					if (((PatchHitData *)(hitRec->hitData))->type == PATCH_HIT_VERTEX)
					{
						if (invert)
						{
							if (patch->vertSel[index])
								patch->vertSel.Clear(index);
							else
								patch->vertSel.Set(index);
						}
						else
							if (selected)
 								patch->vertSel.Set(index);
							else
								patch->vertSel.Clear(index);
					}
					hitRec = NULL;	// Reset it so we can exit	
				}
vert_done:
				break;
			}
		case EP_EDGE: 
			{
				if (all)
				{				
					if (invert)
					{
						while (hitRec) 
						{
							// If the object changes, we're done!
							if (patchData !=(EditPatchData*)hitRec->modContext->localData)
								goto edge_done;
							int index =((PatchHitData *)(hitRec->hitData))->index;
							if (patch->edgeSel[index])
								patch->edgeSel.Clear(index);
							else
								patch->edgeSel.Set(index);
							hitRec = hitRec->Next();
						}
					}
					else
						if (selected)
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto edge_done;
								patch->edgeSel.Set(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
						else 
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto edge_done;
								patch->edgeSel.Clear(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
				}
				else 
				{
					int index =((PatchHitData *)(hitRec->hitData))->index;
					if (invert)
					{
						if (patch->edgeSel[index])
							patch->edgeSel.Clear(index);
						else
							patch->edgeSel.Set(index);
					}
					else
						if (selected)
						{
							patch->edgeSel.Set(index);
						}
						else 
						{
							patch->edgeSel.Clear(index);
						}
						hitRec = NULL;	// Reset it so we can exit	
				}
edge_done:
				break;
			}
		case EP_PATCH: 
			{
				if (all)
				{				
					if (invert)
					{
						while (hitRec) 
						{
							// If the object changes, we're done!
							if (patchData !=(EditPatchData*)hitRec->modContext->localData)
								goto patch_done;
							int index =((PatchHitData *)(hitRec->hitData))->index;
							if (patch->patchSel[index])
								patch->patchSel.Clear(index);
							else
								patch->patchSel.Set(index);
							hitRec = hitRec->Next();
						}
					}
					else
						if (selected)
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto patch_done;
								patch->patchSel.Set(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
						else 
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto patch_done;
								patch->patchSel.Clear(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
				}
				else 
				{
					int index =((PatchHitData *)(hitRec->hitData))->index;
					if (invert)
					{
						if (patch->patchSel[index])
							patch->patchSel.Clear(index);
						else
							patch->patchSel.Set(index);
					}
					else
						if (selected)
						{
							patch->patchSel.Set(index);
						}
						else 
						{
							patch->patchSel.Clear(index);
						}
						hitRec = NULL;	// Reset it so we can exit	
				}
patch_done:
				break;
			}
		case EP_TILE:
			{
				if (all)
				{
					if (invert)
					{
						while (hitRec)
						{
							// If the object changes, we're done!
							if (patchData !=(EditPatchData*)hitRec->modContext->localData)
								goto tile_done;
							int index =((PatchHitData *)(hitRec->hitData))->index;
							if (rpatch->tileSel[index])
								rpatch->tileSel.Clear(index);
							else
								rpatch->tileSel.Set(index);
							hitRec = hitRec->Next();
						}
					}
					else
						if (selected)
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto tile_done;
								rpatch->tileSel.Set(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
						else 
						{
							while (hitRec) 
							{
								// If the object changes, we're done!
								if (patchData !=(EditPatchData*)hitRec->modContext->localData)
									goto tile_done;
								rpatch->tileSel.Clear(((PatchHitData *)(hitRec->hitData))->index);
								hitRec = hitRec->Next();
							}
						}
				}
				else 
				{
					int index =((PatchHitData *)(hitRec->hitData))->index;
					if (invert)
					{
						if (rpatch->tileSel[index])
							rpatch->tileSel.Clear(index);
						else
							rpatch->tileSel.Set(index);
					}
					else
						if (selected)
						{
							rpatch->tileSel.Set(index);
						}
						else 
						{
							rpatch->tileSel.Clear(index);
						}
						hitRec = NULL;	// Reset it so we can exit	
				}
tile_done:
				break;
			}
		case EP_OBJECT:
		default:
			return;
		}
		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
		{
			patchData->tempData->Invalidate(PART_SELECT);
		}
		PatchSelChanged();
	}
		
	UpdateSelectDisplay();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}



void EditPatchMod::ClearSelection(int selLevel) 
{
	// Don't do anything if at vertex level with verts turned off
	if (selLevel == EP_VERTEX && !filterVerts)
		return;
	if (selLevel == EP_OBJECT)
		return;
	
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	ip->ClearCurNamedSelSet();
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patchData->BeginEdit(ip->GetTime());
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "ClearSelection"));
		}
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				patch->vertSel.ClearAll();
				break;
			}
		case EP_EDGE: 
			{
				patch->edgeSel.ClearAll();
				break;
			}
		case EP_PATCH: 
			{
				patch->patchSel.ClearAll();
				break;
			}
		case EP_TILE: 
			{
				rpatch->tileSel.ClearAll();
				break;
			}
		}
		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		PatchSelChanged();
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void UnselectHiddenPatches(int level, PatchMesh *patch)
{
	switch (level)
	{
	case EP_VERTEX: 
		{
			for (int i = 0; i < patch->numVerts; i++)
			{
				if (patch->getVert(i).IsHidden())
					patch->vertSel.Set(i, FALSE);
			}
			break;
		}
	case EP_EDGE: 
		{
			for (int i = 0; i < patch->numEdges; i++)
			{
				int a, b;
				a = patch->edges[i].v1;
				b = patch->edges[i].v2;
				if (patch->getVert(a).IsHidden() && patch->getVert(b).IsHidden())
					patch->edgeSel.Set(i, FALSE);
			}
			break;
		}
	case EP_PATCH:
		{
			for (int i = 0; i < patch->numPatches; i++)
			{
				if (patch->patches[i].IsHidden())
					patch->patchSel.Set(i, FALSE);
			}
			break;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SelectAll(int selLevel) 
{
	// Don't do anything if at vertex level with verts turned off
	if (selLevel == EP_VERTEX && !filterVerts)
		return;
	if (selLevel == EP_OBJECT)
		return;
	
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	ip->ClearCurNamedSelSet();
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patchData->BeginEdit(ip->GetTime());
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SelectAll"));
		}
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				patch->vertSel.SetAll();
				break;
			}
		case EP_EDGE: 
			{
				patch->edgeSel.SetAll();
				break;
			}
		case EP_PATCH: 
			{
				patch->patchSel.SetAll();
				break;
			}
		case EP_TILE: 
			{
				rpatch->tileSel.SetAll();
				break;
			}
		}
		UnselectHiddenPatches(selLevel, patch);
		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		PatchSelChanged();
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::InvertSelection(int selLevel) 
{
	// Don't do anything if at vertex level with verts turned off
	if (selLevel == EP_VERTEX && !filterVerts)
		return;
	if (selLevel == EP_OBJECT)
		return;
	
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	ip->ClearCurNamedSelSet();
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patchData->BeginEdit(ip->GetTime());
		if (theHold.Holding())
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "InvertSelection"));
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				patch->vertSel = ~patch->vertSel;
				break;
			}
		case EP_EDGE: 
			{
				patch->edgeSel = ~patch->edgeSel;
				break;
			}
		case EP_PATCH: 
			{
				patch->patchSel = ~patch->patchSel;
				break;
			}
		case EP_TILE: 
			{
				rpatch->tileSel = ~rpatch->tileSel;
				break;
			}
		}
		UnselectHiddenPatches(selLevel, patch);
		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		PatchSelChanged();
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::ActivateSubobjSel(int level, XFormModes& modes)
{	
	ModContextList mcList;
	INodeTab nodes;
	int old = selLevel;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);

	selLevel = level;
	// 3-10-99 watje
	if (level != EP_PATCH)
	{
		if (ip->GetCommandMode() == bevelMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (ip->GetCommandMode() == extrudeMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
		if (inBevel)
		{
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_OUTLINESPINNER));
			if (spin)
			{
				HWND hWnd = spin->GetHwnd();
				SendMessage(hWnd, WM_LBUTTONUP, 0, 0);
				ReleaseISpinner(spin);
			}
			
		}
		if (inExtrude)
		{
			ISpinnerControl *spin;
			spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_EXTRUDESPINNER));
			if (spin)
			{
				HWND hWnd = spin->GetHwnd();
				SendMessage(hWnd, WM_LBUTTONUP, 0, 0);
				ReleaseISpinner(spin);
			}
		}
	}	
	if (level != EP_VERTEX)
	{
		if (ip->GetCommandMode() == bindMode)
			ip->SetStdCommandMode(CID_OBJMOVE);
	}


	switch (level)
	{
	case EP_OBJECT:
		// Not imp.
		break;
		
	case EP_PATCH:
		modes = XFormModes(moveMode, rotMode, nuscaleMode, uscaleMode, squashMode, selectMode);
		break;
		
	case EP_EDGE:
		modes = XFormModes(moveMode, rotMode, nuscaleMode, uscaleMode, squashMode, selectMode);
		break;
		
	case EP_VERTEX:
		
		modes = XFormModes(moveMode, rotMode, nuscaleMode, uscaleMode, squashMode, selectMode);
		break;
		
	case EP_TILE:
		
		modes = XFormModes(NULL, NULL, NULL, NULL, NULL, selectMode);
		break;
	}

	if (selLevel != old)
	{
		SetSubobjectLevel(level);
		
		// Modify the caches to reflect the new sel level.
		for (int i = 0; i < mcList.Count(); i++)
		{
			EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
			if (!patchData)
				continue;		
			
			if (patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()))
			{		
				RPatchMesh *rpatch;
				PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
				if (patch)
				{
					if (selLevel == EP_VERTEX)
						patch->dispFlags = DISP_VERTS;
					else
						patch->dispFlags = 0;
					if (displayLattice)
						patch->SetDispFlag(DISP_LATTICE);
					patch->SetDispFlag(patchLevelDispFlags[selLevel]);
					patch->selLevel = patchLevel[selLevel];
					rpatch->SetSelLevel (selLevel);
				}
			}
		}		
		
		NotifyDependents(FOREVER, PART_SUBSEL_TYPE | PART_DISPLAY, REFMSG_CHANGE);
		ip->PipeSelLevelChanged();
		// Update selection UI display, named sel
		SelectionChanged();
	}

	nodes.DisposeTemporary();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::ChangeSelPatches(int type) 
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
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		
		// If any bits are set in the selection set, let's DO IT!!
		if (patch->patchSel.NumberSet())
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "ChangeSelPatches"));
			// Call the vertex type change function
			ChangePatchType(patch, -1, type);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
		theHold.Accept(GetString(IDS_TH_PATCHCHANGE));
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

void EditPatchMod::SetRememberedPatchType(int type) 
{
	if (rememberedPatch)
		ChangeRememberedPatch(type);
	else
		ChangeSelPatches(type);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::ChangeSelVerts(int type) 
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
		
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);
		
		// If any bits are set in the selection set, let's DO IT!!
		if (patch->vertSel.NumberSet())
		{
			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "ChangeSelVerts"));
			// Call the vertex type change function
			patch->ChangeVertType(-1, type);
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
		theHold.Accept(GetString(IDS_TH_VERTCHANGE));
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL), PROMPT_TIME);
		theHold.End();
	}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

static void AssignSetMatchSize(BitArray &dst, BitArray &src)
{
	int size = dst.GetSize();
	dst = src;
	if (dst.GetSize() != size)
	{
		dst.SetSize(size, TRUE);
	}
}

void EditPatchMod::ActivateSubSelSet(TSTR &setName)
{
	MaybeFixupNamedSels();
	ModContextList mcList;
	INodeTab nodes;
	int index = FindSet(setName, selLevel);
	if (index < 0 || !ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		patchData->BeginEdit(ip->GetTime());
		// If that set exists in this context, deal with it
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		BitArray *set = sel.GetSet(setName);
		if (set)
		{
			if (theHold.Holding())
				theHold.Put(new PatchSelRestore(patchData, this, patch));
			BitArray *psel = GetLevelSelectionSet(patch, rpatch);	// Get the appropriate selection set
			AssignSetMatchSize(*psel, *set);				
			PatchSelChanged();
		}
		
		patchData->UpdateChanges(patch, rpatch, FALSE);
		if (patchData->tempData)
			patchData->TempData(this)->Invalidate(PART_SELECT);
	}
	
	theHold.Accept(GetString(IDS_DS_SELECT));
	nodes.DisposeTemporary();	
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::NewSetFromCurSel(TSTR &setName)
{
	MaybeFixupNamedSels();
	
	ModContextList mcList;
	INodeTab nodes;	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		BitArray *exist = sel.GetSet(setName);	
		switch (selLevel)
		{
		case EP_VERTEX:	
			if (exist)
			{
				*exist = patch->vertSel;
			} else 
			{
				patchData->vselSet.AppendSet(patch->vertSel, 0, setName);
			}
			break;
			
		case EP_PATCH:
			if (exist)
			{
				*exist = patch->patchSel;
			} else 
			{
				patchData->pselSet.AppendSet(patch->patchSel, 0, setName);
			}
			break;
			
		case EP_EDGE:
			if (exist)
			{
				*exist = patch->edgeSel;
			} else 
			{
				patchData->eselSet.AppendSet(patch->edgeSel, 0, setName);
			}
			break;
		}
	}	
	
	int index = FindSet(setName, selLevel);
	if (index < 0)
		AddSet(setName, selLevel);		
	nodes.DisposeTemporary();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::RemoveSubSelSet(TSTR &setName)
{
	MaybeFixupNamedSels();
	
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		patchData->BeginEdit(ip->GetTime());
		GenericNamedSelSetList &sel = patchData->GetSelSet(this);
		sel.RemoveSet(setName);
	}
	// Remove the modifier's entry
	RemoveSet(setName, selLevel);
	ip->ClearCurNamedSelSet();
	SetupNamedSelDropDown();
	nodes.DisposeTemporary();
}