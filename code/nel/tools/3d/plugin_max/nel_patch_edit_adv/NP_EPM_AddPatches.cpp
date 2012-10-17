#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern int		attachReorient;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

static void AddPatches(int type, PatchMesh *patch, RPatchMesh *rpatch, BOOL postWeld) 
{
	if (!patch->edgeSel.NumberSet())
		return;		// Nothing to do!

	int lastVert = patch->getNumVerts();
	int edges = patch->getNumEdges();

	if (type==PATCH_TRI)
		MessageBox (NULL, "Rykol tools", "C'est pas cool les tripatches...", MB_OK|MB_ICONEXCLAMATION);

	// Add a patch of the desired type to each selected edge that doesn't have two patches atatched!
	for (int i = 0; i < edges; ++i)
	{
		if (patch->edgeSel[i])
		{
			PatchEdge &edge = patch->edges[i];
#if (MAX_RELEASE < 4000)
			int nFirstPatch=edge.patch1;
			if (edge.patch2 < 0)
#else // (MAX_RELEASE < 4000)
			int nFirstPatch=edge.patches[0];
			if (edge.patches[1] < 0)
#endif // (MAX_RELEASE < 4000)
			{
				int verts = patch->getNumVerts();
				int vecs = patch->getNumVecs();
				int patches = patch->getNumPatches();
				patch->setNumPatches(patches + 1, TRUE);			// Add a patch
				patch->patches[patches].SetType(type);			// Make it the type we want
				patch->setNumVerts(verts + type - 2, TRUE);		// Add the appropriate number of verts
				rpatch->SetNumVerts(verts + type - 2);	// And the appropriate vector count
				patch->setNumVecs(vecs +(type - 1) * 2 + type, TRUE);	// And the appropriate vector count
				Point3 p1 = patch->verts[edge.v1].p;
				Point3 p2 = patch->verts[edge.v2].p;
				Point3 v12 = patch->vecs[edge.vec12].p;
				Point3 v21 = patch->vecs[edge.vec21].p;
				Point3 edgeCenter =(p1 + p2) / 2.0f;
				// Load up the patch with the correct vert/vector indices
#if (MAX_RELEASE < 4000)
				Patch &spatch = patch->patches[edge.patch1];
#else // (MAX_RELEASE < 4000)
				Patch &spatch = patch->patches[edge.patches[0]];
#endif // (MAX_RELEASE < 4000)
				Patch &dpatch = patch->patches[patches];
				switch (type)
				{
				case PATCH_TRI:
					dpatch.setVerts(edge.v2, edge.v1, verts);
					dpatch.setVecs(edge.vec21, edge.vec12, vecs, vecs + 1, vecs + 2, vecs + 3);
					dpatch.setInteriors(vecs + 4, vecs + 5, vecs + 6);
					switch (spatch.type)
					{
					case PATCH_TRI: 
						{		// Tri from Tri
							// Find the opposite vertex in the source triangle
							int opposite, o2a, o1a;
							if (spatch.edge[0] == i)
							{
								opposite = 2;
								o1a = 5;
								o2a = 2;
							}
							else
								if (spatch.edge[1] == i)
								{
									opposite = 0;
									o1a = 1;
									o2a = 4;
								}
								else 
								{
									opposite = 1;
									o1a = 3;
									o2a = 0;
								}
							// Compute the new vert position
							Point3 oppVec = edgeCenter - patch->verts[spatch.v[opposite]].p;
							float oppLen = Length(oppVec);
							if (oppLen == 0.0f)
							{
								oppVec = Point3(0, 0, 1);
								oppLen = 1.0f;
							}
							Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
							Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
							Point3 n1a, n2a;
							if (Length(v1a) > 0.0f)
								n1a = Normalize(v1a);
							else
								n1a = Normalize(oppVec);
							if (Length(v2a) > 0.0f)
								n2a = Normalize(v2a);
							else
								n2a = Normalize(oppVec);
							
							// Build a composite vector based on the two edge vectors
							Point3 compVec = Normalize((n1a + n2a) / 2.0f);
							
							// Create the new vertex
							Point3 newPos = edgeCenter - compVec * oppLen;
							patch->verts[verts].p = newPos;
							
							// Compute the vectors
							patch->vecs[vecs].p = p1 - v1a;
							patch->vecs[vecs + 1].p = newPos -(newPos - p1) / 3.0f;
							patch->vecs[vecs + 2].p = newPos -(newPos - p2) / 3.0f;
							patch->vecs[vecs + 3].p = p2 - v2a;
						}
						break;
					case PATCH_QUAD: 
						{	// Tri from Quad
							// Find the opposite edge verts in the source quad
							int opposite1, opposite2, o1a, o2a;
							if (spatch.edge[0] == i)
							{
								opposite1 = 2;
								opposite2 = 3;
								o1a = 7;
								o2a = 2;
							}
							else
								if (spatch.edge[1] == i)
								{
									opposite1 = 3;
									opposite2 = 0;
									o1a = 1;
									o2a = 4;
								}
								else
									if (spatch.edge[2] == i)
									{
										opposite1 = 0;
										opposite2 = 1;
										o1a = 3;
										o2a = 6;
									}
									else 
									{
										opposite1 = 1;
										opposite2 = 2;
										o1a = 5;
										o2a = 0;
									}
							// Compute the new vert position
							Point3 otherCenter =(patch->verts[spatch.v[opposite1]].p + patch->verts[spatch.v[opposite2]].p) / 2.0f;
							Point3 oppVec = edgeCenter - otherCenter;
							float oppLen = Length(oppVec);
							if (oppLen == 0.0f)
							{
								oppVec = Point3(0, 0, 1);
								oppLen = 1.0f;
							}
							Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
							Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
							Point3 n1a, n2a;
							if (Length(v1a) > 0.0f)
								n1a = Normalize(v1a);
							else
								n1a = Normalize(oppVec);
							if (Length(v2a) > 0.0f)
								n2a = Normalize(v2a);
							else
								n2a = Normalize(oppVec);
							
							// Build a composite vector based on the two edge vectors
							Point3 compVec = Normalize((n1a + n2a) / 2.0f);
							
							// Create the new vertex
							Point3 newPos = edgeCenter - compVec * oppLen;
							patch->verts[verts].p = newPos;
							
							// Compute the vectors
							patch->vecs[vecs].p = p1 - v1a;
							patch->vecs[vecs + 1].p = newPos -(newPos - p1) / 3.0f;
							patch->vecs[vecs + 2].p = newPos -(newPos - p2) / 3.0f;
							patch->vecs[vecs + 3].p = p2 - v2a;
						}
						break;
					}
					break;
				case PATCH_QUAD:
					dpatch.setVerts(edge.v2, edge.v1, verts, verts + 1);
					dpatch.setVecs(edge.vec21, edge.vec12, vecs, vecs + 1, vecs + 2, vecs + 3, vecs + 4, vecs + 5);
					dpatch.setInteriors(vecs + 6, vecs + 7, vecs + 8, vecs + 9);
					switch (spatch.type)
					{
					case PATCH_TRI: 
						{		// Quad from Tri
							// Find the opposite vertex in the source triangle
							int opposite, o2a, o1a;
							if (spatch.edge[0] == i)
							{
								opposite = 2;
								o1a = 5;
								o2a = 2;
							}
							else
								if (spatch.edge[1] == i)
								{
									opposite = 0;
									o1a = 1;
									o2a = 4;
								}
								else 
								{
									opposite = 1;
									o1a = 3;
									o2a = 0;
								}
								
							Point3 oppVec = edgeCenter - patch->verts[spatch.v[opposite]].p;
							float oppLen = Length(oppVec);
							if (oppLen == 0.0f)
							{
								oppVec = Point3(0, 0, 1);
								oppLen = 1.0f;
							}
							Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
							Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
							Point3 n1a, n2a;
							if (Length(v1a) > 0.0f)
								n1a = Normalize(v1a);
							else
								n1a = Normalize(oppVec);
							if (Length(v2a) > 0.0f)
								n2a = Normalize(v2a);
							else
								n2a = Normalize(oppVec);
							
							// Compute the new vert positions
							Point3 newPos1 = p1 - n1a * oppLen;
							Point3 newPos2 = p2 - n2a * oppLen;
							patch->verts[verts].p = newPos1;
							patch->verts[verts + 1].p = newPos2;
							// Compute the vectors
							patch->vecs[vecs].p = p1 - v1a;
							patch->vecs[vecs + 1].p = newPos1 -(newPos1 - p1) / 3.0f;
							patch->vecs[vecs + 2].p = newPos1 +(v12 - p1);
							patch->vecs[vecs + 3].p = newPos2 +(v21 - p2);
							patch->vecs[vecs + 4].p = newPos2 +(p2 - newPos2) / 3.0f;
							patch->vecs[vecs + 5].p = p2 - v2a;
						}
						break;
					case PATCH_QUAD: 
						{	// Quad from Quad
							// Find the opposite edge verts in the source quad
							int opposite1, opposite2, o1a, o2a;
							if (spatch.edge[0] == i)
							{
								opposite1 = 2;
								opposite2 = 3;
								o1a = 7;
								o2a = 2;
							}
							else
								if (spatch.edge[1] == i)
								{
									opposite1 = 3;
									opposite2 = 0;
									o1a = 1;
									o2a = 4;
								}
								else
									if (spatch.edge[2] == i)
									{
										opposite1 = 0;
										opposite2 = 1;
										o1a = 3;
										o2a = 6;
									}
									else 
									{
										opposite1 = 1;
										opposite2 = 2;
										o1a = 5;
										o2a = 0;
									}
									
							Point3 otherCenter =(patch->verts[spatch.v[opposite1]].p + patch->verts[spatch.v[opposite2]].p) / 2.0f;
							Point3 oppVec = edgeCenter - otherCenter;
							float oppLen = Length(oppVec);
							if (oppLen == 0.0f)
							{
								oppVec = Point3(0, 0, 1);
								oppLen = 1.0f;
							}
							Point3 v1a = patch->vecs[spatch.vec[o1a]].p - p1;
							Point3 v2a = patch->vecs[spatch.vec[o2a]].p - p2;
							Point3 n1a, n2a;
							if (Length(v1a) > 0.0f)
								n1a = Normalize(v1a);
							else
								n1a = Normalize(oppVec);
							if (Length(v2a) > 0.0f)
								n2a = Normalize(v2a);
							else
								n2a = Normalize(oppVec);
							
							// Compute the new vert position
							Point3 newPos1 = p1 - n1a * oppLen;
							Point3 newPos2 = p2 - n2a * oppLen;
							patch->verts[verts].p = newPos1;
							patch->verts[verts + 1].p = newPos2;
							
							// Compute the vectors
							patch->vecs[vecs].p = p1 - v1a;
							patch->vecs[vecs + 1].p = newPos1 -(newPos1 - p1) / 3.0f;
							patch->vecs[vecs + 2].p = newPos1 +(v12 - p1);
							patch->vecs[vecs + 3].p = newPos2 +(v21 - p2);
							patch->vecs[vecs + 4].p = newPos2 +(p2 - newPos2) / 3.0f;
							patch->vecs[vecs + 5].p = p2 - v2a;
						}
						break;
					}
					break;
				}
				rpatch->AddPatch (i, nFirstPatch, patch);
			}
		}
	}
	patch->computeInteriors();
	patch->buildLinkages();
	// This step welds all new identical verts
	if (postWeld &&(patch->getNumVerts() != lastVert))
		patch->Weld(0.0f, TRUE, lastVert);
}

// ---------------------------------------------------------------------------

BOOL PatchAddRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	AddPatches(type, patch, rpatch, postWeld);
	return TRUE;
}

// ---------------------------------------------------------------------------

void EditPatchMod::DoPatchAdd(int type) 
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
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoPatchAdd"));
			// Call the patch add function
			AddPatches(type, patch, rpatch, TRUE);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_PATCHADD));
	}
	else 
	{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVALIDEDGESSEL), PROMPT_TIME);
		theHold.End();
	}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

