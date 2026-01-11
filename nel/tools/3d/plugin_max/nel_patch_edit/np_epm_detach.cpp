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

static TSTR detachName;

static BOOL CALLBACK DetachDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	TCHAR tempName[256];
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_DETACH_NAME, detachName);
		SetFocus(GetDlgItem(hDlg, IDC_DETACH_NAME));
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_DETACH_NAME, tempName, 255);
			detachName = TSTR(tempName);
			EndDialog(hDlg, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}

// ---------------------------------------------------------------------------

static int GetDetachOptions(IObjParam *ip, TSTR& newName) 
{
	detachName = newName;
	ip->MakeNameUnique(detachName);	
	if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_DETACH), ip->GetMAXHWnd(), (DLGPROC)DetachDialogProc) == 1)
	{
		newName = detachName;
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------

static void MakeDummyMapPatches(int channel, PatchMesh *patch) 
{
	patch->setNumMapVerts(channel, 1);
	patch->tVerts[channel][0] = UVVert(0, 0, 0);
	patch->setNumMapPatches(channel, patch->numPatches);
	for (int i = 0; i < patch->numPatches; ++i)
	{
		Patch &p = patch->patches[i];
		TVPatch &tp = patch->tvPatches[channel][i];
		tp.Init();	// Sets all indices to zero
		}
	}

// ---------------------------------------------------------------------------

// Detach all selected patches
void EditPatchMod::DoPatchDetach(int copy, int reorient) 
{
	int dialoged = 0;
	TSTR newName(GetString(IDS_TH_PATCH));
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

	// Create a patch mesh object
	RPO *patchOb = new RPO;
	patchOb->rpatch = new RPatchMesh;
	PatchMesh &pmesh = patchOb->patch;
	RPatchMesh &prmesh = *patchOb->rpatch;
	int verts = 0;
	int vecs = 0;
	int patches = 0;

	int multipleObjects =(mcList.Count() > 1) ? 1 : 0;
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

		// If any patches selected, we'll need to process this one
		if (patch->patchSel.NumberSet())
		{
			if (!dialoged)
			{
				dialoged = 1;
				if (!GetDetachOptions(ip, newName))
					goto bail_out;
				}
			// Save the unmodified info.
			if (theHold.Holding())
			{
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch, _T("DoPatchDetach")));
				}
			PatchMesh wpatch = *patch;
			RPatchMesh wrpatch = *rpatch;
			BitArray vdel(wpatch.numVerts);
			vdel.ClearAll();
			BitArray pdel = wpatch.patchSel;	// Get inverse selection set
			// If not copying, delete the patches from this object
			if (!copy)
				DeletePatchParts(patch, rpatch, vdel, pdel);
			pdel = ~wpatch.patchSel;	// Get inverse selection set
			if (pdel.NumberSet())
			{
				vdel.ClearAll();
				DeletePatchParts(&wpatch, &wrpatch, vdel, pdel);
			}
			
			// RPatchMesh validity
			wrpatch.Validity (wpatch, true);
			
			// We've deleted everything that wasn't selected -- Now add this to the patch object accumulator
			int oldVerts = pmesh.numVerts;
			int oldVecs = pmesh.numVecs;
			int oldPatches = pmesh.numPatches;
			int newVerts = oldVerts + wpatch.numVerts;
			int newVecs = oldVecs + wpatch.numVecs;
			int newPatches = oldPatches + wpatch.numPatches;
			pmesh.setNumVerts(newVerts, TRUE);
			pmesh.setNumVecs(newVecs, TRUE);
			pmesh.setNumPatches(newPatches, TRUE);
			prmesh.SetNumVerts (newVerts);
			prmesh.SetNumPatches (newPatches);
			altered = holdNeeded = 1;
			Matrix3 tm(1);
			if (multipleObjects && !reorient)
				tm = nodes[i]->GetObjectTM(t);
			int i, i2;
			for (i = 0, i2 = oldVerts; i < wpatch.numVerts; ++i, ++i2)
			{
				pmesh.verts[i2] = wpatch.verts[i];
				pmesh.verts[i2].p = pmesh.verts[i2].p * tm;

				// Copy rpatch info
				prmesh.getUIVertex (i2)=wrpatch.getUIVertex (i);
			}
			for (i = 0, i2 = oldVecs; i < wpatch.numVecs; ++i, ++i2)
			{
				pmesh.vecs[i2] = wpatch.vecs[i];
				pmesh.vecs[i2].p = pmesh.vecs[i2].p * tm;
			}
			for (i = 0, i2 = oldPatches; i < wpatch.numPatches; ++i, ++i2)
			{
				Patch &p = wpatch.patches[i];
				Patch &p2 = pmesh.patches[i2];
				p2 = p;
				int j;
				for (j = 0; j < p2.type; ++j)
				{
					// Adjust vertices and interior vectors
					p2.v[j] += oldVerts;
					p2.interior[j] += oldVecs;
				}
				for (j = 0; j <(p2.type * 2); ++j)	// Adjust edge vectors
					p2.vec[j] += oldVecs;

				// Copy rpatch info
				prmesh.getUIPatch (i2)=wrpatch.getUIPatch (i);
			}
			// Now copy over mapping information
			int dmaps = pmesh.getNumMaps();
			int smaps = wpatch.getNumMaps();
			int maxMaps = dmaps > smaps ? dmaps : smaps;
			if (maxMaps != dmaps)
				pmesh.setNumMaps(maxMaps, TRUE);
			if (maxMaps != smaps)
				wpatch.setNumMaps(maxMaps, TRUE);
			// Then make sure any active maps are active in both:
			int chan;
			for (chan = 0; chan < maxMaps; ++chan)
			{
				if (pmesh.tvPatches[chan] || wpatch.tvPatches[chan])
				{
					if (!pmesh.tvPatches[chan])
						MakeDummyMapPatches(chan, &pmesh);
					if (!wpatch.tvPatches[chan])
						MakeDummyMapPatches(chan, &wpatch);
					}
				}
			for (chan = 0; chan < pmesh.getNumMaps(); ++chan)
			{
				if (chan < wpatch.getNumMaps())
				{
					int oldTVerts = pmesh.numTVerts[chan];
					int newTVerts = oldTVerts + wpatch.numTVerts[chan];
					pmesh.setNumMapVerts(chan, newTVerts, TRUE);
					for (i = 0, i2 = oldTVerts; i < wpatch.numTVerts[chan]; ++i, ++i2)
						pmesh.tVerts[chan][i2] = wpatch.tVerts[chan][i];
					if (pmesh.tvPatches[chan])
					{
						for (i = 0, i2 = oldPatches; i < wpatch.numPatches; ++i, ++i2)
						{
							Patch &p = wpatch.patches[i];
							TVPatch &tp = wpatch.tvPatches[chan][i];
							TVPatch &tp2 = pmesh.tvPatches[chan][i2];
							tp2 = tp;
							for (int j = 0; j < p.type; ++j)	// Adjust vertices
								tp2.tv[j] += oldTVerts;
							}
						}
					}
				}
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(this)->Invalidate(PART_TOPO);
			}

		bail_out:
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	
	if (holdNeeded)
	{
		pmesh.computeInteriors();
		pmesh.buildLinkages();
		INode *newNode = ip->CreateObjectNode(patchOb);
		newNode->SetMtl(nodes[0]->GetMtl());
		newNode->SetName(newName.data());
		patchOb->patch.InvalidateGeomCache();
		if (!multipleObjects)
		{
			// Single input object?
			if (!reorient)
			{
				Matrix3 tm = nodes[0]->GetObjectTM(t);
				newNode->SetNodeTM(t, tm);	// Use this object's TM.
				}
			}
		else 
		{
			if (!reorient)
			{
				Matrix3 matrix;
				matrix.IdentityMatrix();
				newNode->SetNodeTM(t, matrix);	// Use identity TM
				}
			}
		newNode->FlagForeground(t);		// WORKAROUND!
		ResolveTopoChanges();
		theHold.Accept(GetString(IDS_TH_DETACHPATCH));
		}
	else 
	{
		delete patchOb;	// Didn't need it after all!
		if (!dialoged)
			ip->DisplayTempPrompt(GetString(IDS_TH_NOPATCHESSEL), PROMPT_TIME);
		theHold.End();
		}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(t, REDRAW_NORMAL);
	}
