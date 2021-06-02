#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoAddHook(PatchMesh *pMesh, int vert0, int vert1, int vert2, int seg, int config)
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);


	if (mcList.Count() != 1)
		return;

	ClearPatchDataFlag(mcList, EPD_BEENDONE);

	theHold.Begin();
//	RecordTopologyTags();
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
		if ((!patch) ||(patch != pMesh))
			continue;		
//		patchData->RecordTopologyTags(patch);
		// If this is the first edit, then the delta arrays will be allocated
		patchData->BeginEdit(t);

		// If any bits are set in the selection set, let's DO IT!!
//		if(patch->vertSel.NumberSet()) {

		altered = holdNeeded = TRUE;
		if (theHold.Holding())
			theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
		
		// ** Hulud bug hack for hooking my way  \\\\\\\\\///////////

		//patch->AddHook(vert1, seg);

		// Config 0
		switch (config)
		{
		case 0:
			rpatch->AddHook (vert1, seg, *patch);
			break;
		case 1:
			rpatch->AddHook (vert0, vert1, vert2, seg, *patch);
			break;
		default:
			nlassert (0);
		}

		// ** //////////\\\\\\\\\\

//		patch->UpdateHooks();
//			InvalidateMesh();

		patchData->UpdateChanges(patch, rpatch);
		patchData->TempData(this)->Invalidate(PART_TOPO);
//			}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
//		ResolveTopoChanges();
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

/*	// If any bits are set in the selection set, let's DO IT!!
	if (!ip)
	return;
	theHold.Begin();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if (theHold.Holding())
		theHold.Put(new PatchObjectRestore(this, rec));
		// Call the patch type change function

	patch.AddHook();
	patch.InvalidateGeomCache();
	InvalidateMesh();
	theHold.Accept(GetResString(IDS_TH_PATCHCHANGE));

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoRemoveHook() 
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
			
			// Modif by Hulud
			//patch->RemoveHook();
			rpatch->RemoveHook (*patch);

//			patch->InvalidateGeomCache();
//			InvalidateMesh();

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



