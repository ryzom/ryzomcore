#include "stdafx.h"
#include "nel_patch_paint.h"
#include "../nel_patch_lib/vertex_neighborhood.h"

// in mods.cpp
extern HINSTANCE hInstance;

/*-------------------------------------------------------------------*/

static EditPatchClassDesc editPatchDesc;
extern ClassDesc* GetEditPatchModDesc() { return &editPatchDesc; }

void EditPatchClassDesc::ResetClassParams(BOOL fileReset)
{
}

/*-------------------------------------------------------------------*/		

int PaintPatchMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) 
{	
	return 0;
}

void PaintPatchMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) 
{
	box.Init();
}

//---------------------------------------------------------------------
// UI stuff

class EPModContextEnumProc : public ModContextEnumProc 
{
	float f;
public:
	EPModContextEnumProc(float f) { this->f = f; }
	BOOL proc(ModContext *mc);  // Return FALSE to stop, TRUE to continue.
};

BOOL EPModContextEnumProc::proc(ModContext *mc) 
{
	PaintPatchData *patchData =(PaintPatchData*)mc->localData;
	if (patchData)		
		patchData->RescaleWorldUnits(f);
	return TRUE;
}

// World scaling
void PaintPatchMod::RescaleWorldUnits(float f) 
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

