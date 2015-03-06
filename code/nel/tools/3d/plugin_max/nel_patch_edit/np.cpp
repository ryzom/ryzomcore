
/**********************************************************************
 *<
	FILE: editpat.cpp

	DESCRIPTION:  Edit Patch OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 23 June, 1995

	IMPORTANT USAGE NOTE:

		When you do an operation in edit patch which will change the topology, the form
		of the code should look like this code, taken from the vertex deletion:

		-----

			ip->GetModContexts(mcList, nodes);
			ClearPatchDataFlag(mcList, EPD_BEENDONE);

			theHold.Begin();
		-->	RecordTopologyTags();
			for (int i = 0; i < mcList.Count(); i++)
		{
		int altered = 0;
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
		continue;
		if (patchData->GetFlag(EPD_BEENDONE))
		continue;
		
		  // If the mesh isn't yet cache, this will cause it to get cached.
		  PatchMesh *patch = patchData->TempData(this)->GetPatch(t);
		  if (!patch)
		  continue;
		  -->		patchData->RecordTopologyTags(patch);
		  
			// If this is the first edit, then the delta arrays will be allocated
			patchData->BeginEdit(t);
			
			  // If any bits are set in the selection set, let's DO IT!!
			  if (patch->vertSel.NumberSet())
			  {
			  altered = holdNeeded = 1;
			  if (theHold.Holding())
			  theHold.Put(new PatchRestore(patchData, this, patch, "DoVertDelete"));
			  // Call the vertex delete function
			  DeleteSelVerts(patch);
			  -->			patchData->UpdateChanges(patch);
			  patchData->TempData(this)->Invalidate(PART_TOPO);
			  }
			  patchData->SetFlag(EPD_BEENDONE, TRUE);
			  }
			
			if (holdNeeded)
		{
		-->		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTDELETE));
		}
			else 
		{
		ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL), PROMPT_TIME);
		theHold.End();
		}
			
			nodes.DisposeTemporary();
			ClearPatchDataFlag(mcList, EPD_BEENDONE);

		-----

		The key elements in the "changed topology" case are the calls noted by arrows.
		These record special tags inside the object so that after the topology is changed
		by the modifier code, the UpdateChanges code can make a new mapping from the old
		object topology to the new.

		If the operation doesn't change the topology, then the three topology tag calls
		aren't needed and the UpdateChanges call becomes:

			patchData->UpdateChanges(patch, FALSE);

		This tells UpdateChanges not to bother remapping the topology.

 *>	Copyright(c) 1994, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"

#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#	include <maxscript/foundation/3dmath.h>
#	include <maxscript/foundation/numbers.h>
#	include <maxscript/maxwrapper/maxclasses.h>
#	include <maxscript/foundation/streams.h>
#	include <maxscript/foundation/mxstime.h>
#	include <maxscript/maxwrapper/mxsobjects.h>
#	include <maxscript/compiler/parser.h>
#	include <maxscript/macros/define_instantiation_functions.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#	include <MaxScrpt/3dmath.h>
//	Various MAX and MXS includes
#	include <MaxScrpt/Numbers.h>
#	include <MaxScrpt/MAXclses.h>
#	include <MaxScrpt/Streams.h>
#	include <MaxScrpt/MSTime.h>
#	include <MaxScrpt/MAXObj.h>
#	include <MaxScrpt/Parser.h>
//	define the new primitives using macros from SDK
#	include <MaxScrpt/definsfn.h>
#endif

#include "editpat.h"
#include "../nel_patch_lib/vertex_neighborhood.h"

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx

#define DBG_NAMEDSELSx

// Uncomment this for vert mapper debugging
//#define VMAP_DEBUG 1

// Forward references
INT_PTR CALLBACK PatchSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchOpsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchObjSurfDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchSurfDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchTileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PatchEdgeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void ResetVert (PatchMesh *patch);

// A handy zero point
Point3 zeroPoint(0, 0, 0);

// Our temporary prompts last 2 seconds:
#define PROMPT_TIME 2000

// in mods.cpp
extern HINSTANCE hInstance;

// Select by material parameters
int sbmParams[4]     = {1, 1, RPO_DEFAULT_TESSEL, RPO_DEFAULT_TESSEL};

// Select by smooth parameters
DWORD sbsParams[3]   = {1, 1, 0};

float weldThreshold = 0.1f;

// Checkbox items for rollup pages

int attachReorient = 0;
// This is a special override value which allows us to hit-test on
// any sub-part of a patch

extern int patchHitOverride;	// If zero, no override is done

void SetPatchHitOverride(int value) 
{
	patchHitOverride = value;
}

void ClearPatchHitOverride() 
{
	patchHitOverride = 0;
}




PatchDeleteUser pDel;
extern PatchRightMenu pMenu;

/*-------------------------------------------------------------------*/

static EditPatchClassDesc editPatchDesc;
extern ClassDesc* GetEditPatchModDesc() { return &editPatchDesc; }

void EditPatchClassDesc::ResetClassParams(BOOL fileReset)
{
	sbmParams[0]   = 1;
	sbmParams[1]   = 1;
	sbmParams[2]   = RPO_DEFAULT_TESSEL;
	sbmParams[3]   = RPO_DEFAULT_TESSEL;
	EditPatchMod::condenseMat = FALSE;
	EditPatchMod::attachMat = ATTACHMAT_IDTOMAT;
}

/*-------------------------------------------------------------------*/		

int EditPatchMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) 
{	
	return 0;
}

void EditPatchMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) 
{
	box.Init();
}

//---------------------------------------------------------------------
// UI stuff

void EditPatchMod::RecordTopologyTags() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
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
		patch->RecordTopologyTags();
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
}

class ChangeNamedSetRestore : public RestoreObj 
{
public:
	BitArray oldset, newset;
	int index;
	GenericNamedSelSetList *setList;
	
	ChangeNamedSetRestore(GenericNamedSelSetList *sl, int ix, BitArray *o) 
	{
		setList = sl;
		index = ix;
		oldset = *o;
	}   		
	void Restore(int isUndo) 
	{
		newset = *(setList->sets[index]);
		*(setList->sets[index]) = oldset;
	}
	void Redo() 
	{
		*(setList->sets[index]) = newset;
	}
				
	TSTR Description() {return TSTR(_T("Change Named Sel Set"));}
};

// Selection set, misc fixup utility function
// This depends on PatchMesh::RecordTopologyTags being called prior to the topo changes
void EditPatchMod::ResolveTopoChanges() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
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
		// First, the vertex selections
		int set;
		for (set = 0; set < patchData->vselSet.Count(); ++set)
		{
			BitArray *oldVS = &patchData->vselSet[set];
			BitArray newVS;
			newVS.SetSize(patch->numVerts);
			for (int vert = 0; vert < patch->numVerts; ++vert)
			{
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->verts[vert].aux1;
				if (tag >= 0)
					newVS.Set(vert, (*oldVS)[tag]);
				else
					newVS.Clear(vert);
			}
			if (theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->vselSet, set, oldVS));
			patchData->vselSet[set] = newVS;
		}
		// Now the edge selections
		for (set = 0; set < patchData->eselSet.Count(); ++set)
		{
			BitArray *oldES = &patchData->eselSet[set];
			BitArray newES;
			newES.SetSize(patch->numEdges);
			for (int edge = 0; edge < patch->numEdges; ++edge)
			{
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->edges[edge].aux1;
				if (tag >= 0)
					newES.Set(edge, (*oldES)[tag]);
				else
					newES.Clear(edge);
			}
			if (theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->eselSet, set, oldES));
			patchData->eselSet[set] = newES;
		}
		// Now the patch selections
		for (set = 0; set < patchData->pselSet.Count(); ++set)
		{
			BitArray *oldPS = &patchData->pselSet[set];
			BitArray newPS;
			newPS.SetSize(patch->numPatches);
			for (int p = 0; p < patch->numPatches; ++p)
			{
				// Get the knot's previous location, then copy that selection into the new set
				int tag = patch->patches[p].aux1;
				if (tag >= 0)
					newPS.Set(p, (*oldPS)[tag]);
				else
					newPS.Clear(p);
			}
			if (theHold.Holding())
				theHold.Put(new ChangeNamedSetRestore(&patchData->pselSet, set, oldPS));
			patchData->pselSet[set] = newPS;
		}
		
		// watje 4-16-99
		patch->HookFixTopology();
		
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
}

class EPModContextEnumProc : public ModContextEnumProc 
{
	float f;
public:
	EPModContextEnumProc(float f) { this->f = f; }
	BOOL proc(ModContext *mc);  // Return FALSE to stop, TRUE to continue.
};

BOOL EPModContextEnumProc::proc(ModContext *mc) 
{
	EditPatchData *patchData =(EditPatchData*)mc->localData;
	if (patchData)		
		patchData->RescaleWorldUnits(f);
	return TRUE;
}

// World scaling
void EditPatchMod::RescaleWorldUnits(float f) 
{
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	
	// rescale all our references
	for (int i = 0; i < NumRefs(); i++)
	{
		ReferenceMaker *srm = GetReference(i);
		if (srm) 
			srm->RescaleWorldUnits(f);
	}
	
	// Now rescale stuff inside our data structures
	EPModContextEnumProc proc(f);
	EnumModContexts(&proc);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void EditPatchMod::InvalidateSurfaceUI() 
{
	if (hSurfPanel && selLevel == EP_PATCH)
	{
		InvalidateRect(hSurfPanel, NULL, FALSE);
		patchUIValid = FALSE;
	}
}

void EditPatchMod::InvalidateTileUI() 
{
	if (hTilePanel && selLevel == EP_TILE)
	{
		InvalidateRect(hTilePanel, NULL, FALSE);
		tileUIValid = FALSE;
	}
}

void EditPatchMod::InvalidateEdgeUI() 
{
	if (hEdgePanel && selLevel == EP_EDGE)
	{
		InvalidateRect(hEdgePanel, NULL, FALSE);
		edgeUIValid = FALSE;
	}
}

BitArray *EditPatchMod::GetLevelSelectionSet(PatchMesh *patch, RPatchMesh *rpatch)
{
	switch (selLevel)
	{
	case EP_VERTEX:
		return &patch->vertSel;
		
	case EP_PATCH:
		return &patch->patchSel;
		
	case EP_EDGE:
		return &patch->edgeSel;
		
	case EP_TILE:
		return &rpatch->tileSel;
	}
	nlassert(0);
	return NULL;
}

void EditPatchMod::UpdateSelectDisplay() 
{	
	TSTR buf;
	int num, j;

	if (!hSelectPanel)
		return;

	ModContextList mcList;
	INodeTab nodes;
	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);

	switch (GetSubobjectLevel())
	{
	case EP_OBJECT:
		buf.printf(GetString(IDS_TH_OBJECT_SEL));
		break;
		
	case EP_VERTEX: 
		{
			num = 0;
			PatchMesh *thePatch = NULL;
			for (int i = 0; i < mcList.Count(); i++)
			{
				EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
				if (!patchData)
					continue;		
				
				if (patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()))
				{
					RPatchMesh *rpatch;
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
					if (!patch)
						continue;
					int thisNum = patch->vertSel.NumberSet();
					if (thisNum)
					{
						num += thisNum;
						thePatch = patch;
					}
				}
			}
			if (num == 1)
			{
				for (j = 0; j < thePatch->vertSel.GetSize(); j++)
					if (thePatch->vertSel[j])
						break;
					buf.printf(GetString(IDS_TH_NUMVERTSEL), j + 1);
			}
			else
				buf.printf(GetString(IDS_TH_NUMVERTSELP), num);
		}
		break;
		
	case EP_PATCH: 
		{
			num = 0;
			PatchMesh *thePatch = NULL;
			for (int i = 0; i < mcList.Count(); i++)
			{
				EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
				if (!patchData)
					continue;		
				
				if (patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()))
				{
					RPatchMesh *rpatch;
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
					if (!patch)
						continue;
					int thisNum = patch->patchSel.NumberSet();
					if (thisNum)
					{
						num += thisNum;
						thePatch = patch;
					}
				}
			}
			if (num == 1)
			{
				for (j = 0; j < thePatch->patchSel.GetSize(); j++)
					if (thePatch->patchSel[j])
						break;
					buf.printf(GetString(IDS_TH_NUMPATCHSEL), j + 1);
			}
			else
				buf.printf(GetString(IDS_TH_NUMPATCHSELP), num);
		}
		break;
		
	case EP_EDGE: 
		{
			num = 0;
			PatchMesh *thePatch = NULL;
			for (int i = 0; i < mcList.Count(); i++)
			{
				EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
				if (!patchData)
					continue;		
				
				if (patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()))
				{
					RPatchMesh *rpatch;
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
					if (!patch)
						continue;
					int thisNum = patch->edgeSel.NumberSet();
					if (thisNum)
					{
						num += thisNum;
						thePatch = patch;
					}
				}
			}
			if (num == 1)
			{
				for (j = 0; j < thePatch->edgeSel.GetSize(); j++)
					if (thePatch->edgeSel[j])
						break;
					buf.printf(GetString(IDS_TH_NUMEDGESEL), j + 1);
			}
			else
				buf.printf(GetString(IDS_TH_NUMEDGESELP), num);
		}
		break;
		
	case EP_TILE: 
		{
			num = 0;
			RPatchMesh *thePatch = NULL;
			for (int i = 0; i < mcList.Count(); i++)
			{
				EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
				if (!patchData)
					continue;		
				
				if (patchData->tempData && patchData->TempData(this)->PatchCached(ip->GetTime()))
				{
					RPatchMesh *rpatch;
					PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
					if (!patch)
						continue;
					int thisNum = rpatch->tileSel.NumberSet();
					if (thisNum)
					{
						num += thisNum;
						thePatch = rpatch;
					}
				}
			}
			if (num == 1)
			{
				for (j = 0; j < thePatch->tileSel.GetSize(); j++)
					if (thePatch->tileSel[j])
						break;
				buf.printf("Tile %d Selected", j + 1);
			}
			else
				buf.printf("%d Tiles Selected", num);
		}
	break;
	}
	
	nodes.DisposeTemporary();
	SetDlgItemText(hSelectPanel, IDC_NUMSEL_LABEL, buf);
}

void EditPatchMod::DoVertWeld() 
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	BOOL hadSel = FALSE;

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
		if (patch->vertSel.NumberSet() > 1)
		{
			hadSel = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoVertWeld"));
			// Call the patch weld function
			if (patch->Weld(weldThreshold))
			{
				rpatch->Weld (patch);
				altered = holdNeeded = TRUE;
				patchData->UpdateChanges(patch, rpatch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
			}
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTWELD));
	}
	else 
	{
		if (!hadSel)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL), PROMPT_TIME);
		else
			ip->DisplayTempPrompt(GetString(IDS_TH_NOWELDPERFORMED), PROMPT_TIME);
		theHold.End();
	}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::DoVertReset ()
{
	ModContextList mcList;		
	INodeTab nodes;
	TimeValue t = ip->GetTime();
	int holdNeeded = 0;
	BOOL hadSel = FALSE;

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
		if (patch->vertSel.NumberSet() > 0)
		{
			hadSel = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoVertReset"));
			// Call the patch weld function
			ResetVert (patch);
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_GEOM);
			/*if (patch->Weld(weldThreshold))
			{
				rpatch->Weld (patch);
				altered = holdNeeded = TRUE;
				patchData->UpdateChanges(patch, rpatch);
				patchData->TempData(this)->Invalidate(PART_TOPO);
			}*/
		}
		patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	ResolveTopoChanges();
	theHold.Accept("Reset Vertex");
	/*if (holdNeeded)
	{
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_VERTWELD));
	}
	else 
	{
		if (!hadSel)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOVERTSSEL), PROMPT_TIME);
		else
			ip->DisplayTempPrompt(GetString(IDS_TH_NOWELDPERFORMED), PROMPT_TIME);
		theHold.End();
	}*/

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::PatchSelChanged() 
{
	SelectionChanged();
	if (hSurfPanel && selLevel == EP_PATCH)
		InvalidateSurfaceUI();
	if (hTilePanel && selLevel == EP_TILE)
		InvalidateTileUI();
	if (hEdgePanel && selLevel == EP_EDGE)
		InvalidateEdgeUI();
}

/*
class AdvParams 
{
public:
	TessSubdivStyle mStyle;
	int mMin, mMax;
	int mTris;
};
*/





void EditPatchMod::LocalDataChanged()
{
}

int GetPointIndex (int nVertex, int nPatch, PatchMesh* patch)
{
	for (int n=0; n<4; n++)
	{
		if (patch->patches[nPatch].v[n]==nVertex)
			return n;
	}
	nlassert (0);
	return 0;
}

Point3 GetInterior (int nPatch, int nInt, PatchMesh* patch)
{
	return patch->vecs[patch->patches[nPatch].interior[nInt]].p;
}

void ResetVert (PatchMesh *patch)
{
	// Make a edge table
	// Static table to avoid alloc prb
	CVertexNeighborhood& edgeTab=vertexNeighborhoodGlobal;
	edgeTab.build (*patch);

	// For each vertices
	for (int nV=0; nV<patch->numVerts; nV++)
	{
		// Selected ?
		if (patch->vertSel[nV])
		{
			Point3 vert=patch->verts[nV].p;
			Point3 normal (0,0,0);

			// Count of neigbor for vertex n
			uint listSize=edgeTab.getNeighborCount (nV);

			// List of neigbor
			const uint* pList=edgeTab.getNeighborList (nV);

			// For each neigbor
			uint nn;
			for (nn=0; nn<listSize; nn++)
			{
#if (MAX_RELEASE < 4000)
				// Compute average plane
				if (patch->edges[pList[nn]].patch1!=-1)
					normal+=patch->PatchNormal(patch->edges[pList[nn]].patch1);
				if (patch->edges[pList[nn]].patch2!=-1)
					normal+=patch->PatchNormal(patch->edges[pList[nn]].patch2);
#else // (MAX_RELEASE <= 4000)
				// Compute average plane
				if (patch->edges[pList[nn]].patches[0]!=-1)
					normal+=patch->PatchNormal(patch->edges[pList[nn]].patches[0]);
				if (patch->edges[pList[nn]].patches[1]!=-1)
					normal+=patch->PatchNormal(patch->edges[pList[nn]].patches[1]);
#endif // (MAX_RELEASE <= 4000)
			}
			
			// Normalize
			normal=normal.Normalize();
			
			// Plane
			float fD=-DotProd(normal, vert);

			// Reset normales
			float fNorme=0.f;

			// For each neigbor
			for (nn=0; nn<listSize; nn++)
			{
				Point3 vect2=patch->verts[(patch->edges[pList[nn]].v1==nV)?patch->edges[pList[nn]].v2:patch->edges[pList[nn]].v1].p;
				vect2-=vert;
				vect2/=3.f;
				Point3 tmp1=CrossProd (vect2, normal);
				tmp1=CrossProd (normal, tmp1);
				tmp1=Normalize(tmp1);
				int nTang=(patch->edges[pList[nn]].v1==nV)?patch->edges[pList[nn]].vec12:patch->edges[pList[nn]].vec21;
				patch->vecs[nTang].p=vert+tmp1*DotProd (tmp1,vect2);
				tmp1=patch->vecs[nTang].p;
				tmp1-=vert;
				fNorme+=tmp1.Length();
			}

			// Renorme new normal
			/*fNorme/=(float)edgeTab[nV].size();
			ite=edgeTab[nV].begin();
			while (ite!=edgeTab[nV].end())
			{
				int nTang=(patch->edges[pList[nn]].v1==nV)?patch->edges[pList[nn]].vec12:patch->edges[pList[nn]].vec21;
				patch->vecs[nTang].p=fNorme*(Normalize(patch->vecs[nTang].p-vert))+vert;

				ite++;
			}*/
		}
	}
	patch->computeInteriors();
	patch->InvalidateGeomCache ();
}

def_visible_primitive(turn_patch, "RykolTurnPatch");

Value *turn_patch_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(RykolTurnPatch, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "RykolTurnPatch [Node] [Modifier] [Patch]");
    type_check(arg_list[1], MAXModifier, "RykolTurnPatch [Node] [Modifier] [Patch]"); 
	type_check(arg_list[2], Integer, "RykolTurnPatch [Node] [Modifier] [Patch]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os = node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) 
		{
			bRet = true;
			RPO *tri = (RPO *)os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				Modifier *mod = arg_list[1]->to_modifier();
				if (mod)
				{
					EditPatchMod *epmod = (EditPatchMod *)mod;
					epmod->ClearSelection(EP_PATCH);
					epmod->SelectSubPatch(arg_list[2]->to_int() - 1);
					epmod->DoPatchTurn(true);
					epmod->ClearSelection(EP_PATCH);
				}
				else
				{
					bRet = false;
				}
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}