#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EPM_ExtrudeMouseProc::proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) 
{	
#if MAX_VERSION_MAJOR >= 19
	ViewExp *vpt = &ip->GetViewExp(hwnd);
#else
	ViewExp *vpt = ip->GetViewport(hwnd);
#endif
	Point3 p0, p1;
	ISpinnerControl *spin;
	BOOL ln;
	IPoint2 m2;
	float amount;
	switch (msg)
	{
	case MOUSE_PROPCLICK:
		ip->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		if (!point)
		{
			po->BeginExtrude(ip->GetTime());		
			om = m;
		} else 
		{
			ip->RedrawViews(ip->GetTime(), REDRAW_END);
			po->EndExtrude(ip->GetTime(), TRUE);

		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om, -200.f);

		// sca 1999.02.24: Find m's projection in om's vertical axis:
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView(m2, -200.f);

		amount = Length(p1 - p0);
		if (m.y > om.y)
			amount *= -1.0f;

		ln = IsDlgButtonChecked(po->hOpsPanel, IDC_EM_EXTYPE_B);
		po->Extrude(ip->GetTime(), amount, ln);

		spin = GetISpinner(GetDlgItem(po->hOpsPanel, IDC_EP_EXTRUDESPINNER));
		if (spin)
		{
			spin->SetValue(amount, FALSE);	// sca - use signed value here too.
			ReleaseISpinner(spin);
		}
		ip->RedrawViews(ip->GetTime(), REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		po->EndExtrude(ip->GetTime(), FALSE);			
		ip->RedrawViews(ip->GetTime(), REDRAW_END);
		break;
	}

#if MAX_VERSION_MAJOR < 19
	if (vpt)
		ip->ReleaseViewport(vpt);
#endif

	return TRUE;
}

// --------------------------------------------------------------------------------

HCURSOR EPM_ExtrudeSelectionProcessor::GetTransformCursor() 
{ 
	static HCURSOR hCur = NULL;
	if (!hCur)
		hCur = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_EXTRUDECUR));
	return hCur; 
}

// --------------------------------------------------------------------------------

void EPM_ExtrudeCMode::EnterMode() 
{
	if (!po->hOpsPanel)
		return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel, IDC_EP_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

// --------------------------------------------------------------------------------

void EPM_ExtrudeCMode::ExitMode() 
{
	if (!po->hOpsPanel)
		return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel, IDC_EP_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(po->hOpsPanel, IDC_EP_EXTRUDESPINNER));
	if (spin)
	{
		spin->SetValue(0.0f, FALSE);
		ReleaseISpinner(spin);
		}

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoExtrude()
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
		if (patch->patchSel.NumberSet())
		{

			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));
			// Call the vertex type change function
			patch->CreateExtrusion();
			rpatch->CreateExtrusion(patch);
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

/*
	theHold.Begin();
	patch.RecordTopologyTags();
	POPatchGenRecord *rec = new POPatchGenRecord(this);
	if (theHold.Holding())
		theHold.Put(new PatchObjectRestore(this, rec));

	patch.CreateExtrusion();
	
	ResolveTopoChanges();
	theHold.Accept(GetResString(IDS_TH_PATCHADD));

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
}


void EditPatchMod::BeginExtrude(TimeValue t) 
{	
	if (inExtrude)
		return;
	inExtrude = TRUE;
	theHold.SuperBegin();
	DoExtrude();
//	PlugControllersSel(t,sel);
	theHold.Begin();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::EndExtrude(TimeValue t, BOOL accept) 
{		
	if (!ip)
		return;

	if (!inExtrude)
		return;

	ISpinnerControl *spin;
	inExtrude = FALSE;
	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_EXTRUDESPINNER));
	if (spin)
	{
		spin->SetValue(0, FALSE);
		ReleaseISpinner(spin);
		}
//	TempData()->freeBevelInfo();

	theHold.Accept(GetString(IDS_RB_EXTRUDE));
	if (accept)
		theHold.SuperAccept(GetString(IDS_RB_EXTRUDE));
	else theHold.SuperCancel();

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::Extrude(TimeValue t, float amount, BOOL useLocalNorms) 
{
	if (!inExtrude)
		return;


	ModContextList mcList;		
	INodeTab nodes;
	BOOL holdNeeded = FALSE;
	BOOL hadSelected = FALSE;

	if (!ip)
		return;

	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);

//	theHold.Begin();
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
		if (patch->patchSel.NumberSet())
		{

			altered = holdNeeded = TRUE;
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, this, patch, rpatch));

			// Call the vertex type change function
			#if (MAX_RELEASE < 4000)
				patch->MoveNormal(amount, useLocalNorms);
			#else
				patch->MoveNormal(amount, useLocalNorms, PATCH_PATCH);
			#endif
			patch->InvalidateGeomCache();
			//InvalidateMesh();

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

/*	theHold.Restore();
	patch.MoveNormal(amount, useLocalNorms);

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/


}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

