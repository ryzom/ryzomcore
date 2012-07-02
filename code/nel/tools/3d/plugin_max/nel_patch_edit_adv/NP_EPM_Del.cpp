#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void DeletePatchParts(PatchMesh *patch, RPatchMesh *rpatch, BitArray &delVerts, BitArray &delPatches);

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoDeleteSelected() 
{
	switch (GetSubobjectLevel())
	{
	case EP_VERTEX:
		DoVertDelete();
		break;
	case EP_EDGE:
		DoEdgeDelete();
		break;
	case EP_PATCH:
		DoPatchDelete();
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

static void DeleteSelPatches(PatchMesh *patch, RPatchMesh *rpatch)
{
	if (!patch->patchSel.NumberSet())
		return;		// Nothing to do!
	
	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();
	
	// Tag the patches that are selected
	BitArray delPatches(patches);
	delPatches = patch->patchSel;
	
	BitArray delVerts(verts);
	delVerts.ClearAll();
	
	DeletePatchParts(patch, rpatch, delVerts, delPatches);
	patch->computeInteriors();
}

// ---------------------------------------------------------------------------

BOOL PatchDeleteRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	DeleteSelPatches(patch, rpatch);
	return TRUE;
}

// ---------------------------------------------------------------------------

void EditPatchMod::DoPatchDelete() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
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
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoPatchDelete"));
			// Call the patch delete function
			DeleteSelPatches(patch, rpatch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHDELETE));
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

void DeleteSelVerts(PatchMesh *patch, RPatchMesh *rpatch) 
{
	if (!patch->vertSel.NumberSet())
		return;		// Nothing to do!
	
	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();
	
	// Tag the patches that use selected vertices
	BitArray delPatches(patches);
	delPatches.ClearAll();
	for (int i = 0; i < patches; ++i)
	{
		Patch& p = patch->patches[i];
		for (int j = 0; j < p.type; ++j)
		{
			if (patch->vertSel[p.v[j]])
			{
				delPatches.Set(i);
				goto next_patch;
			}
		}
next_patch:;
	}
	
	BitArray delVerts(verts);
	delVerts = patch->vertSel;
	DeletePatchParts(patch, rpatch, delVerts, delPatches);
	patch->computeInteriors();
}

// ---------------------------------------------------------------------------

// Vertex Delete modifier method
void EditPatchMod::DoVertDelete() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
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
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoVertDelete"));
			// Call the vertex delete function
			DeleteSelVerts(patch, rpatch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTDELETE));
	}
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

// Edger Delete modifier method
void EditPatchMod::DoEdgeDelete() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();
	RecordTopologyTags();
	for (int i = 0; i < mcList.Count(); i++)
	{
		int altered = 0;
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
			altered = holdNeeded = 1;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoEdgeDelete"));
			int edges = patch->getNumEdges();
			int patches = patch->getNumPatches();
			int verts = patch->getNumVerts();
			
			// Tag the patches that are attached to selected edges
			BitArray delPatches(patches);
			delPatches.ClearAll();
			
			for (int i = 0; i < edges; ++i)
			{
				if (patch->edgeSel[i])
				{
#if (MAX_RELEASE < 4000)
					if (patch->edges[i].patch1 >= 0)
						delPatches.Set(patch->edges[i].patch1);
					if (patch->edges[i].patch2 >= 0)
						delPatches.Set(patch->edges[i].patch2);
#else // (MAX_RELEASE < 4000)
					if (patch->edges[i].patches[0] >= 0)
						delPatches.Set(patch->edges[i].patches[0]);
					if (patch->edges[i].patches[1] >= 0)
						delPatches.Set(patch->edges[i].patches[1]);
#endif // (MAX_RELEASE < 4000)
				}
			}
			
			BitArray delVerts(verts);
			delVerts.ClearAll();
			
			DeletePatchParts(patch, rpatch, delVerts, delPatches);
			patch->computeInteriors();
			
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_EDGEDELETE));
	}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOEDGESSEL), PROMPT_TIME);
		theHold.End();
	}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// Deletes any vertices tagged, also any patches tagged.  Automatically deletes the vectors that
// are deleted as a result of the patch deletion and sweeps any vertices floating in space.

void DeletePatchParts(PatchMesh *patch, RPatchMesh *rpatch, BitArray &delVerts, BitArray &delPatches) 
{
	int patches = patch->getNumPatches();
	int verts = patch->getNumVerts();
	int vecs = patch->getNumVecs();
	int dest;

	// We treat vectors specially in order to clean up after welds.  First, we tag 'em all,
	// then untag only those on unselected patches so that any dangling vectors will be deleted.
	BitArray delVectors(vecs);
	delVectors.SetAll();

	// Untag vectors that are on nondeleted patches
	int i;
	for (i = 0; i < patches; ++i)
	{
		if (!delPatches[i])
		{
			Patch& p = patch->patches[i];
			int j;
			for (j = 0; j <(p.type * 2); ++j)
			{
				delVectors.Clear(p.vec[j]);
			}
			for (j = 0; j < p.type; ++j)
				delVectors.Clear(p.interior[j]);
		}
	}

	// Make a table of vertices that are still in use -- Used to
	// delete those vertices which are floating, unused, in space.
	BitArray usedVerts(verts);
	usedVerts.ClearAll();
	for (i = 0; i < patches; ++i)
	{
		if (!delPatches[i])
		{
			Patch& p = patch->patches[i];
			for (int j = 0; j < p.type; ++j)
			{
				usedVerts.Set(p.v[j]);
			}
		}
	}
	for (i = 0; i < verts; ++i)
	{
		if (!usedVerts[i])
			delVerts.Set(i);
	}

	// If we have texture vertices, handle them, too
	for (int chan = 0; chan < patch->getNumMaps(); ++chan)
	{
		int tverts = patch->numTVerts[chan];
		if (tverts && patch->tvPatches[chan])
		{
			BitArray delTVerts(tverts);
			delTVerts.SetAll();
			for (i = 0; i < patches; ++i)
			{
				if (!delPatches[i])
				{
					Patch& p = patch->patches[i];
					TVPatch& tp = patch->tvPatches[chan][i];
					for (int j = 0; j < p.type; ++j)
						delTVerts.Clear(tp.tv[j]);
				}
			}
			// Got the list of tverts to delete -- now delete 'em
			// Build a table of redirected texture vertex indices
			int newTVerts = tverts - delTVerts.NumberSet();
			IntTab tVertIndex;
			tVertIndex.SetCount(tverts);
			UVVert *newTVertArray = new UVVert[newTVerts];
			dest = 0;
			for (i = 0; i < tverts; ++i)
			{
				if (!delTVerts[i])
				{
					newTVertArray[dest] = patch->tVerts[chan][i];
					tVertIndex[i] = dest++;
				}
			}
			delete[] patch->tVerts[chan];
#if MAX_RELEASE <= 3100
			patch->tVerts[chan] = newTVertArray;
#else
			*(patch->tVerts[chan]) = *newTVertArray;
#endif
			patch->numTVerts[chan] = newTVerts;
			// Now, copy the untagged texture patches to a new array
			// While you're at it, redirect the vertex indices
			int newTVPatches = patches - delPatches.NumberSet();
			TVPatch *newArray = new TVPatch[newTVPatches];
			dest = 0;
			for (i = 0; i < patches; ++i)
			{
				if (!delPatches[i])
				{
					Patch& p = patch->patches[i];
					TVPatch& tp = newArray[dest++];
					tp = patch->tvPatches[chan][i];
					for (int j = 0; j < p.type; ++j)
						tp.tv[j] = tVertIndex[tp.tv[j]];
				}
			}
			delete[] patch->tvPatches[chan];
			patch->tvPatches[chan] = newArray;;
		}
	}

	// Build a table of redirected vector indices
	IntTab vecIndex;
	vecIndex.SetCount(vecs);
	int newVectors = vecs - delVectors.NumberSet();
	PatchVec *newVecArray = new PatchVec[newVectors];
	dest = 0;
	for (i = 0; i < vecs; ++i)
	{
		if (!delVectors[i])
		{
			newVecArray[dest] = patch->vecs[i];
			vecIndex[i] = dest++;
		}
		else
			vecIndex[i] = -1;
	}
	delete[] patch->vecs;
	patch->vecs = newVecArray;
	patch->numVecs = newVectors;

	// Build a table of redirected vertex indices
	int newVerts = verts - delVerts.NumberSet();
	IntTab vertIndex;
	vertIndex.SetCount(verts);
	PatchVert *newVertArray = new PatchVert[newVerts];
	BitArray newVertSel(newVerts);
	newVertSel.ClearAll();
	dest = 0;
	for (i = 0; i < verts; ++i)
	{
		if (!delVerts[i])
		{
			newVertArray[dest] = patch->verts[i];
			newVertSel.Set(dest, patch->vertSel[i]);
			// redirect & adjust attached vector list
			PatchVert& v = newVertArray[dest];
			for (int j = 0; j < v.vectors.Count(); ++j)
			{
				v.vectors[j] = vecIndex[v.vectors[j]];
				if (v.vectors[j] < 0)
				{
					v.vectors.Delete(j, 1);
					j--;	// realign index
				}
			}
			vertIndex[i] = dest++;
		}
	}
	delete[] patch->verts;
	patch->verts = newVertArray;
	patch->numVerts = newVerts;
	patch->vertSel = newVertSel;

	// Now, copy the untagged patches to a new array
	// While you're at it, redirect the vertex and vector indices
	int newPatches = patches - delPatches.NumberSet();
	Patch *newArray = new Patch[newPatches];
	BitArray newPatchSel(newPatches);
	newPatchSel.ClearAll();
	dest = 0;
	for (i = 0; i < patches; ++i)
	{
		if (!delPatches[i])
		{
			newArray[dest] = patch->patches[i];
			Patch& p = newArray[dest];
			int j;
			for (j = 0; j < p.type; ++j)
				p.v[j] = vertIndex[p.v[j]];
			for (j = 0; j <(p.type * 2); ++j)
				p.vec[j] = vecIndex[p.vec[j]];
			for (j = 0; j < p.type; ++j)
				p.interior[j] = vecIndex[p.interior[j]];
			newPatchSel.Set(dest++, patch->patchSel[i]);
		}
	}

	// Rebuild info in rpatch
	rpatch->DeleteAndSweep (delVerts, delPatches, *patch);

	delete[] patch->patches;
	patch->patches = newArray;;
	patch->numPatches = newPatches;
	patch->patchSel.SetSize(newPatches, TRUE);
	patch->patchSel = newPatchSel;
	patch->buildLinkages();
}

