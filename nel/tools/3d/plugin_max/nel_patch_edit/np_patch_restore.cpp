#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void Cancel2StepPatchModes(IObjParam *ip);

// ------------------------------------------------------------------------------------------------------------------------------------------------------

PatchRestore::PatchRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch, RPatchMesh *rpatch, TCHAR *id)
{
	gotRedo = FALSE;
	epd = pd;
	this->mod = mod;
	oldPatch = *patch;

	roldPatch=NULL;
	rnewPatch=NULL;

	// rpatch
	if (rpatch)
	{
		roldPatch=new RPatchMesh();
		*roldPatch = *rpatch;
	}

	t = mod->ip->GetTime();
	where = TSTR(id);
}

PatchRestore::~PatchRestore()
{
	if (roldPatch)
		delete roldPatch;
	if (rnewPatch)
		delete rnewPatch;
}

void PatchRestore::Restore(int isUndo)
{
	if (epd->tempData && epd->TempData(mod)->PatchCached(t))
	{
		RPatchMesh *rpatch;
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t,rpatch);
		if (patch)
		{
			if (isUndo && !gotRedo)
			{
				newPatch = *patch;

				// rpatch
				if (!rnewPatch)
					rnewPatch = new RPatchMesh();

				*rnewPatch = *rpatch;

				gotRedo = TRUE;
			}
		}
		DWORD selLevel = patch->selLevel;	// Grab this...
		DWORD dispFlags = patch->dispFlags;	// Grab this...
		*patch = oldPatch;
		
		if (roldPatch)
			*rpatch = *roldPatch;

		patch->selLevel = selLevel;	// ...and put it back in
		patch->dispFlags = dispFlags;	// ...and put it back in
		patch->InvalidateGeomCache();
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
		mod->SelectionChanged();
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
}

void PatchRestore::Redo()
{
	if (epd->tempData && epd->TempData(mod)->PatchCached(t))
	{
		RPatchMesh *rpatch;
		PatchMesh *patch = epd->TempData(mod)->GetPatch(t,rpatch);
		if (patch)
		{
			DWORD selLevel = patch->selLevel;	// Grab this...
			DWORD dispFlags = patch->dispFlags;	// Grab this...
			*patch = newPatch;

			nlassert (rnewPatch);		// should not be NULL
			*rpatch = *rnewPatch;

			patch->selLevel = selLevel;	// ...and put it back in
			patch->dispFlags = dispFlags;	// ...and put it back in
			patch->InvalidateGeomCache();
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
		mod->SelectionChanged();
		mod->NotifyDependents(FOREVER, PART_GEOM | PART_TOPO | PART_SELECT, REFMSG_CHANGE);
}


