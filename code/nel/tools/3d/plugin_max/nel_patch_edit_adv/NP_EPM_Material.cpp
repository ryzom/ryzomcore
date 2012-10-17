#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::GetSelMatIndex()
	{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	int mat=-1;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			if (patch->patchSel[j])
			{
				if (first)
				{
					first = FALSE;
					mat   =(int)patch->getPatchMtlIndex(j);					
				} else 
				{
					if ((int)patch->getPatchMtlIndex(j) != mat)
					{
						return -1;
						}
					}
				}
			}
		}
	
	nodes.DisposeTemporary();
	return mat;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSelMatIndex(int index)
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetSelMatIndex"));
			}

		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->patchSel[j])
			{
				altered = holdNeeded = TRUE;
				patch->setPatchMtlIndex(j, (MtlID)index);			
				}
			}
		
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);		
		}	
	
	if (holdNeeded)
		theHold.Accept(GetString(IDS_TH_PATCHMTLCHANGE));
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::GetSelTessU()
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	int mat=-1;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			if (patch->patchSel[j])
			{
				if (first)
				{
					first = FALSE;
					mat = (int)rpatch->getUIPatch (j).NbTilesU;
				} 
				else 
				{
					if ((int)rpatch->getUIPatch (j).NbTilesU != mat)
					{
						return -1;
					}
				}
			}
		}
	}
	
	nodes.DisposeTemporary();
	return mat;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::GetSelTessV()
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	int mat=-1;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			if (patch->patchSel[j])
			{
				if (first)
				{
					first = FALSE;
					mat = (int)rpatch->getUIPatch (j).NbTilesV;
				} 
				else 
				{
					if ((int)rpatch->getUIPatch (j).NbTilesV != mat)
					{
						return -1;
					}
				}
			}
		}
	}
	
	nodes.DisposeTemporary();
	return mat;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSelTess(int nU, int nV)
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetSelTess"));
		}

		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->patchSel[j])
			{
				altered = holdNeeded = TRUE;
				rpatch->getUIPatch (j).Init (nU, nV, true);
			}
		}
		
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}

		patchData->SetFlag(EPD_BEENDONE, TRUE);		
	}	
	
	if (holdNeeded)
		theHold.Accept("Tile count in U and V change");
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::getSmoothFlags ()
{
	ModContextList mcList;	
	INodeTab nodes;
	bool notFlagged=false;
	bool flagged=false;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// For each edges
		for (int j = 0; j < patch->getNumEdges(); j++)
		{
			// Is edge selected
			if (patch->edgeSel[j])
			{
				// Patch num
#if (MAX_RELEASE < 4000)
				int p0=patch->edges[j].patch1;
				int p1=patch->edges[j].patch2;
#else // (MAX_RELEASE < 4000)
				int p0=patch->edges[j].patches[0];
				int p1=patch->edges[j].patches[1];
#endif // (MAX_RELEASE < 4000)
				
				// Is edge flaged ?
				if (
					((p0!=-1)&&(rpatch->getUIPatch (p0).getEdgeFlag (WhereIsTheEdge (p0, j, *patch)))) ||
					((p1!=-1)&&(rpatch->getUIPatch (p1).getEdgeFlag (WhereIsTheEdge (p1, j, *patch))))
				   )
				{
					// Flagged !
					flagged=true;
				}
				else
				{
					// Not flagged !
					notFlagged=true;
				}
			}
		}
	}
	
	nodes.DisposeTemporary();
	return flagged?(notFlagged?2:1):0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::setSmoothFlags (bool smooth)
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetSmoothFlags"));
		}

		// For each edges
		for (int j = 0; j < patch->getNumEdges(); j++)
		{
			// Is edge selected
			if (patch->edgeSel[j])
			{
				// Patch num
#if (MAX_RELEASE < 4000)
				int p0=patch->edges[j].patch1;
				int p1=patch->edges[j].patch2;
#else // (MAX_RELEASE < 4000)
				int p0=patch->edges[j].patches[0];
				int p1=patch->edges[j].patches[1];
#endif // (MAX_RELEASE < 4000)
				
				// Patch exist ?
				if (p0!=-1)
				{
					// Set the flag
					rpatch->getUIPatch (p0).setEdgeFlag (WhereIsTheEdge (p0, j, *patch), smooth);

					// Touched
					altered=TRUE;
					holdNeeded=TRUE;
				}

				// idem patch #2
				if (p1!=-1)
				{
					// Set the flag
					rpatch->getUIPatch (p1).setEdgeFlag (WhereIsTheEdge (p1, j, *patch), smooth);

					// Touched
					altered=TRUE;
					holdNeeded=TRUE;
				}
			}
		}
		
		// If touched
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}

		patchData->SetFlag(EPD_BEENDONE, TRUE);		
	}	
	
	if (holdNeeded)
		theHold.Accept("Tile count in U and V change");
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	InvalidateEdgeUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::BalanceSelPatch (int patch, int size, bool balanceU, std::set<int>& visitedU, std::set<int>& visitedV, RPatchMesh* rpatch, PatchMesh *ppatch)
{
	// already visited ?
	if ((balanceU&&(visitedU.find(patch)!=visitedU.end()))||((!balanceU)&&(visitedV.find(patch)!=visitedV.end())))
		return;

	// insert in the visit set
	if (balanceU)
		visitedU.insert (patch);
	else
		visitedV.insert (patch);

	// Get patch tess level..
	int nU=rpatch->getUIPatch (patch).NbTilesU;
	int nV=rpatch->getUIPatch (patch).NbTilesV;
	if (balanceU)
		nU=size;
	else
		nV=size;

	// Resize the patch
	rpatch->getUIPatch (patch).Init (nU, nV, true);

	// Call neiborhood
	for (int i=0; i<4; i++)
	{
		int nEdge=ppatch->patches[patch].edge[i];
#if (MAX_RELEASE < 4000)
		int nOtherPatch=(ppatch->edges[nEdge].patch1==patch)?ppatch->edges[nEdge].patch2:ppatch->edges[nEdge].patch1;
#else // (MAX_RELEASE < 4000)
		int nOtherPatch=(ppatch->edges[nEdge].patches[0]==patch)?ppatch->edges[nEdge].patches[1]:ppatch->edges[nEdge].patches[0];
#endif // (MAX_RELEASE < 4000)
		if (nOtherPatch>=0)
		{
			int nNeiborEdge=WhereIsTheEdge (nOtherPatch, nEdge, *ppatch);
			BalanceSelPatch (nOtherPatch, (i&1)?nU:nV, (nNeiborEdge&1)!=0, visitedU, visitedV, rpatch, ppatch);
		}
	}
}

void EditPatchMod::BalanceSelPatch ()
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetSelTess"));
		}

		std::set<int> visitedU;
		std::set<int> visitedV;
		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->patchSel[j])
			{
				altered = holdNeeded = TRUE;
				
				// insert it
				visitedU.insert (j);
				visitedV.insert (j);

				// Get patch tess level..
				int nU=rpatch->getUIPatch (j).NbTilesU;
				int nV=rpatch->getUIPatch (j).NbTilesV;
				
				// Call neiborhood
				for (int i=0; i<4; i++)
				{
					int nEdge=patch->patches[j].edge[i];
#if (MAX_RELEASE < 4000)
					int nOtherPatch=(patch->edges[nEdge].patch1==j)?patch->edges[nEdge].patch2:patch->edges[nEdge].patch1;
#else // (MAX_RELEASE < 4000)
					int nOtherPatch=(patch->edges[nEdge].patches[0]==j)?patch->edges[nEdge].patches[1]:patch->edges[nEdge].patches[0];
#endif // (MAX_RELEASE < 4000)
					if (nOtherPatch>=0)
					{
						int nNeiborEdge=WhereIsTheEdge (nOtherPatch, nEdge, *patch);
						BalanceSelPatch (nOtherPatch, (i&1)?nU:nV, (nNeiborEdge&1)!=0, visitedU, visitedV, rpatch, patch);
					}
				}

			}
		}
		
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}

		patchData->SetFlag(EPD_BEENDONE, TRUE);		
	}	
	
	if (holdNeeded)
		theHold.Accept("Balance tile");
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

/*void EditPatchMod::SetTileNum (ULONG nU)
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetTileNum"));
		}

		nlassert (patch->numPatches==(int)rpatch->UIPatch.size());
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			int nTileU=1<<rpatch->UIPatch[j].NbTilesU;
			int nTileV=1<<rpatch->UIPatch[j].NbTilesV;

			for (int v = 0; v < nTileV; v++)
			for (int u= 0; u < nTileU; u++)
			{
				if (rpatch->tileSel[rpatch->GetTileNumber(j, u, v)])
				{
					altered = holdNeeded = TRUE;
					rpatch->UIPatch[j].Tile[u+v*nTileU].MatID = nU;
				}
			}
		}
		
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}

		patchData->SetFlag(EPD_BEENDONE, TRUE);		
	}	
	
	if (holdNeeded)
		theHold.Accept("Tile number change");
	else 
	{
		ip->DisplayTempPrompt("No tile selected", PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------

/*ULONG EditPatchMod::GetTileNum ()
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	ULONG num=0xffffffff;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		nlassert (patch->numPatches==(int)rpatch->UIPatch.size());
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			int nTileU=1<<rpatch->UIPatch[j].NbTilesU;
			int nTileV=1<<rpatch->UIPatch[j].NbTilesV;

			for (int v = 0; v < nTileV; v++)
			for (int u= 0; u < nTileU; u++)
			{
				if (rpatch->tileSel[rpatch->GetTileNumber(j, u, v)])
				{
					if (first)
					{
						first = FALSE;
						num = rpatch->UIPatch[j].Tile[u+v*nTileU].MatID;
					} 
					else 
					{
						if (rpatch->UIPatch[j].Tile[u+v*nTileU].MatID != num)
						{
							return 0xffffffff;
						}
					}
				}
			}
		}
	}
	
	nodes.DisposeTemporary();
	return num;
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------

/*void EditPatchMod::SetTileRot (int nRot)
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL holdNeeded = FALSE;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);	

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		BOOL altered = FALSE;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "SetTileRot"));
		}

		nlassert (patch->numPatches==(int)rpatch->UIPatch.size());
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			int nTileU=1<<rpatch->UIPatch[j].NbTilesU;
			int nTileV=1<<rpatch->UIPatch[j].NbTilesV;

			for (int v = 0; v < nTileV; v++)
			for (int u= 0; u < nTileU; u++)
			{
				if (rpatch->tileSel[rpatch->GetTileNumber(j, u, v)])
				{
					altered = holdNeeded = TRUE;
					rpatch->UIPatch[j].Tile[u+v*nTileU].Rotate = nRot;
				}
			}
		}
		
		if (altered)
		{
			patchData->UpdateChanges(patch, rpatch, FALSE);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}

		patchData->SetFlag(EPD_BEENDONE, TRUE);		
	}	
	
	if (holdNeeded)
		theHold.Accept("Tile rotation change");
	else 
	{
		ip->DisplayTempPrompt("No tile selected", PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	InvalidateSurfaceUI();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------

/*int EditPatchMod::GetTileRot ()
{
	ModContextList mcList;	
	INodeTab nodes;
	BOOL first = 1;
	int rot=-1;

	if (!ip)
		return -1;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		nlassert (patch->numPatches==(int)rpatch->UIPatch.size());
		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			int nTileU=1<<rpatch->UIPatch[j].NbTilesU;
			int nTileV=1<<rpatch->UIPatch[j].NbTilesV;

			for (int v = 0; v < nTileV; v++)
			for (int u= 0; u < nTileU; u++)
			{
				if (rpatch->tileSel[rpatch->GetTileNumber(j, u, v)])
				{
					if (first)
					{
						first = FALSE;
						rot = rpatch->UIPatch[j].Tile[u+v*nTileU].Rotate;
					} 
					else 
					{
						if (rpatch->UIPatch[j].Tile[u+v*nTileU].Rotate != rot)
						{
							return -1;
						}
					}
				}
			}
		}
	}
	
	nodes.DisposeTemporary();
	return rot;
}*/

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SelectByMat(int index, BOOL clear)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchSelRestore(patchData, this, patch));
			}
		
		if (clear)
			patch->patchSel.ClearAll();

		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->getPatchMtlIndex(j) == index)
				patch->patchSel.Set(j);
			}
		
		patchData->UpdateChanges(patch, rpatch, FALSE);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

MtlID EditPatchMod::GetNextAvailMtlID(ModContext* mc) 
{
	if (!mc)
		return 1;
	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return 1;
	
	int mtlID = GetSelFaceUniqueMtlID(mc);

	if (mtlID == -1)
	{
		int i;
 		
		MtlID min, max;
		BOOL first = TRUE;

		int p;
		for (p = 0; p < patch->numPatches; ++p)
		{
			MtlID thisID = patch->getPatchMtlIndex(p);
			if (first)
			{
				min = max = thisID;
				first = FALSE;
				}
			else
			if (thisID < min)
				min = thisID;
			else
			if (thisID > max)
				max = thisID;
			}
		// If room below, return it
		if (min > 0)
			return min - 1;
		// Build a bit array to find any gaps		
		BitArray b;
		int bits = max - min + 1;
		b.SetSize(bits);
		b.ClearAll();
		for (p = 0; p < patch->numPatches; ++p)
			b.Set(patch->getPatchMtlIndex(p) - min);
		for (i = 0; i < bits; ++i)
		{
			if (!b[i])
				return (MtlID)(i + min);
			}
		// No gaps!  If room above, return it
		if (max < 65535)
			return max + 1;
		}
	return (MtlID)mtlID;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL EditPatchMod::HasFaceSelection(ModContext* mc) 
{
	// Are we the edited object?
	if (ip == NULL)
		return FALSE;

	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return FALSE;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return FALSE;

	// Is Patch selection active?
	if (selLevel == EP_PATCH && patch->patchSel.NumberSet())
		return TRUE;
	
	return FALSE;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel) 
{
	int altered = 0;
	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return;
	
	// If this is the first edit, then the delta arrays will be allocated
	patchData->BeginEdit(ip->GetTime());

	if (theHold.Holding())
		theHold.Put(new PatchRestore(patchData, this, patch, rpatch));

	for (int p = 0; p < patch->numPatches; ++p)
	{
		if (patch->patchSel[p])
		{
			altered = TRUE;
			patch->setPatchMtlIndex(p, id);
			}
		}

	if (altered)
	{
		patchData->UpdateChanges(patch, rpatch, FALSE);
		InvalidateSurfaceUI();
		}

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int	EditPatchMod::GetSelFaceUniqueMtlID(ModContext* mc) 
{
	int	mtlID;

	mtlID = GetSelFaceAnyMtlID(mc);
	if (mtlID == -1)
		return mtlID;

	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return 1;

	for (int p = 0; p < patch->numPatches; ++p)
	{
		if (patch->patchSel[p])
			continue;
		if (patch->getPatchMtlIndex(p) != mtlID)
			continue;
		mtlID = -1;
		}
	return mtlID;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int	EditPatchMod::GetSelFaceAnyMtlID(ModContext* mc) 
{
	int				mtlID = -1;
	BOOL			bGotFirst = FALSE;

	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return 1;

	for (int p = 0; p < patch->numPatches; ++p)
	{
		if (!patch->patchSel[p])
			continue;
		if (bGotFirst)
		{
			if (mtlID != patch->getPatchMtlIndex(p))
			{
				mtlID = -1;
				break;
				}
			}
		else 
		{
			mtlID = patch->getPatchMtlIndex(p);
			bGotFirst = TRUE;
			}
		}
	return mtlID;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int	EditPatchMod::GetMaxMtlID(ModContext* mc) 
{
	MtlID mtlID = 0;

	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (!patchData)
		return 1;

	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
		return 1;

	for (int p = 0; p < patch->numPatches; ++p)
		mtlID = std::max(mtlID, patch->getPatchMtlIndex(p));

	return mtlID;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------



