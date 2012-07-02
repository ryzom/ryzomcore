#include "stdafx.h"
#include "nel_patch_paint.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchOpsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern void CancelEditPatchModes(IObjParam *ip);

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void CancelEditPatchModes(IObjParam *ip) 
{
	switch (ip->GetCommandMode()->ID())
	{
	case CID_STDPICK:
		ip->SetStdCommandMode(CID_OBJMOVE);
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------
// IPatchSelect and IPatchOps interfaces   (JBW 2/2/99)
void* PaintPatchMod::GetInterface(ULONG id) 
{
	switch (id)
	{
		case I_PATCHSELECT: 
			return (IPatchSelect*)this;
		case I_PATCHSELECTDATA: 
			return (IPatchSelectData*)this;
		case I_PATCHOPS: 
			return (IPatchOps*)this;
		case I_SUBMTLAPI: 
			return (ISubMtlAPI*)this;
	}
	return Modifier::GetInterface(id);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void PaintPatchMod::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	this->ip = ip;
	
	CreatePatchDataTempData();

	hOpsPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_EDPATCH_OPS),
		PatchOpsDlgProc, "Geometry", (LPARAM) this, rsOps ? 0 : APPENDROLL_CLOSED);

	// Create sub object editing modes.
	paintMode		= new EPM_PaintCMode(this, ip);
	
	// Disable show end result.
	ip->EnableShowEndResult(FALSE);
	
	// Setup named selection sets	
	SetupNamedSelDropDown();
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void PaintPatchMod::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	if (hOpsPanel)
	{
		rsOps = IsRollupPanelOpen(hOpsPanel);
		ip->DeleteRollupPage(hOpsPanel);
		hOpsPanel = NULL;
	}
	
	// Enable show end result
	ip->EnableShowEndResult(TRUE);
	
	CancelEditPatchModes(ip);

	if (ip->GetCommandMode()->ID()==CID_EP_PAINT)
		ip->SetStdCommandMode(CID_OBJMOVE);
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t, t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	ClearAFlag(A_MOD_BEING_EDITED);
	
	DeletePatchDataTempData();
	this->ip = NULL;
	
	ip->DeleteMode(paintMode);
	
	if (paintMode)
		delete paintMode;
	paintMode = NULL;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

PaintPatchMod::PaintPatchMod()
{
	channelModified = EDITPAT_CHANNELS;
	// 3-18-99 to suport render steps and removal of the mental tesselator
	
	includeMeshes=false;
	preloadTiles=false;
	//	meshAdaptive = FALSE;	// Future use (Not used now)
}

PaintPatchMod::~PaintPatchMod()
{
}
