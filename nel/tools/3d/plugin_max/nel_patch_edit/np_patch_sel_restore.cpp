#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void Cancel2StepPatchModes(IObjParam *ip);

// ------------------------------------------------------------------------------------------------------------------------------------------------------

PatchSelRestore::PatchSelRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch)
{
	gotRedo = FALSE;
	epd = pd;
	this->mod = mod;
	oldVSel = patch->vertSel;
	oldESel = patch->edgeSel;
	oldPSel = patch->patchSel;
	t = mod->ip->GetTime();
}

void PatchSelRestore::Restore(int isUndo)
{
	if (epd->tempData && epd->TempData(mod)->PatchCached(t))
	{
		RPatchMesh *rpatch;
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t, rpatch);
		if (patch)
		{
			if (isUndo && !gotRedo)
			{
				newVSel = patch->vertSel;
				newESel = patch->edgeSel;
				newPSel = patch->patchSel;
				gotRedo = TRUE;
			}
		}
		patch->vertSel = oldVSel;
		patch->edgeSel = oldESel;
		patch->patchSel = oldPSel;
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
	}
	else
		if (epd->tempData)
		{
			epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
		if (mod->ip)
			Cancel2StepPatchModes(mod->ip);
		mod->InvalidateSurfaceUI();
		//	mod->PatchSelChanged();
		//	mod->UpdateSelectDisplay();
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
}

void PatchSelRestore::Redo()
{
	if (epd->tempData && epd->TempData(mod)->PatchCached(t))
	{
		RPatchMesh *rpatch;
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t,rpatch);
		if (patch)
		{
			patch->vertSel = newVSel;
			patch->edgeSel = newESel;
			patch->patchSel = newPSel;
		}
		epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT);
	}
	else
		if (epd->tempData)
		{
			epd->TempData(mod)->Invalidate(PART_GEOM | PART_TOPO | PART_SELECT, FALSE);
		}
		if (mod->ip)
			Cancel2StepPatchModes(mod->ip);
		mod->InvalidateSurfaceUI();
		//	mod->PatchSelChanged();
		//	mod->UpdateSelectDisplay();
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
}

