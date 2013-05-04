#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EPM_BevelMouseProc::proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) 
{	
	ViewExp *vpt = ip->GetViewport(hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;
	int ln, ln2;
	IPoint2 m2;
	float amount;

	switch (msg)
	{
	case MOUSE_PROPCLICK:
		ip->SetStdCommandMode(CID_OBJMOVE);
		break;

	case MOUSE_POINT:
		if (point == 0)
		{
			po->BeginExtrude(ip->GetTime());		
			om = m;
			} 
		else if (point == 1)
		{
			po->EndExtrude(ip->GetTime(), TRUE);
			po->BeginBevel(ip->GetTime());		
			om = m;
			} 
		else 
		{
			ip->RedrawViews(ip->GetTime(), REDRAW_END);
			po->EndBevel(ip->GetTime(), TRUE);
		}
		break;

	case MOUSE_MOVE:
		if (point == 1)
			{
			p0 = vpt->MapScreenToView(om, float(-200));
			// sca 1999.02.24: find worldspace point with om's x value and m's y value
			m2.x = om.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView(m2, float(-200));
			amount = Length(p1 - p0);
			ln = IsDlgButtonChecked(po->hOpsPanel, IDC_EM_EXTYPE_B);					
			if (om.y < m.y)
				amount *= -1.0f;
			po->Extrude(ip->GetTime(), amount, ln);

			spin = GetISpinner(GetDlgItem(po->hOpsPanel, IDC_EP_EXTRUDESPINNER));
			if (spin)
			{
				spin->SetValue(amount, FALSE);
				ReleaseISpinner(spin);
				}
			ip->RedrawViews(ip->GetTime(), REDRAW_INTERACTIVE);
			}
		else if (point == 2)
			{
			p0 = vpt->MapScreenToView(om, float(-200));
			// sca 1999.02.24: find worldspace point with om's x value and m's y value
			m2.x = om.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView(m2, float(-200));
			if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH))
				ln = 0;					
			else if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH2))
				ln = 1;					
			else if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH3))
				ln = 2;					

			if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH4))
				ln2 = 0;					
			else if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH5))
				ln2 = 1;					
			else if (IsDlgButtonChecked(po->hOpsPanel, IDC_EP_SM_SMOOTH6))
				ln2 = 2;					

			amount = Length(p1 - p0);
			if (om.y < m.y)
				amount *= -1.0f;
			po->Bevel(ip->GetTime(), amount, ln, ln2);

			spin = GetISpinner(GetDlgItem(po->hOpsPanel, IDC_EP_OUTLINESPINNER));
			if (spin)
			{
				spin->SetValue(amount, FALSE);
				ReleaseISpinner(spin);
				}
			ip->RedrawViews(ip->GetTime(), REDRAW_INTERACTIVE);
			}
		break;

	case MOUSE_ABORT:
		if (point == 1)
			po->EndExtrude(ip->GetTime(), FALSE);			
		else if (point>1)
			po->EndBevel(ip->GetTime(), FALSE);			
			

		ip->RedrawViews(ip->GetTime(), REDRAW_END);
		break;
	}

	if (vpt)
		ip->ReleaseViewport(vpt);
	return TRUE;
}

// --------------------------------------------------------------------------------

HCURSOR EPM_BevelSelectionProcessor::GetTransformCursor() 
{ 
	static HCURSOR hCur = NULL;
	if (!hCur)
		hCur = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_BEVEL));
	return hCur; 
}

// --------------------------------------------------------------------------------

void EPM_BevelCMode::EnterMode() 
{
	if (!po->hOpsPanel)
		return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel, IDC_EP_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

// --------------------------------------------------------------------------------

void EPM_BevelCMode::ExitMode() 
{
	if (!po->hOpsPanel)
		return;
	ICustButton *but = GetICustButton(GetDlgItem(po->hOpsPanel, IDC_EP_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(po->hOpsPanel, IDC_EP_OUTLINESPINNER));
	if (spin)
	{
		spin->SetValue(0.0f, FALSE);
		ReleaseISpinner(spin);
		}

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::DoBevel()
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
			patch->CreateBevel();
//			patch->CreateExtrusion();
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

	patch.CreateBevel();
	
	ResolveTopoChanges();
	theHold.Accept(GetResString(IDS_TH_PATCHADD));

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::BeginBevel(TimeValue t) 
{	
	if (inBevel)
		return;
	inBevel = TRUE;
	theHold.SuperBegin();
	DoBevel();
//	PlugControllersSel(t,sel);
	theHold.Begin();
}

void EditPatchMod::EndBevel(TimeValue t, BOOL accept) 
{		
	if (!ip)
		return;
	if (!inBevel)
		return;
	inBevel = FALSE;
//	TempData()->freeBevelInfo();
	ISpinnerControl *spin;

	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_OUTLINESPINNER));
	if (spin)
	{
		spin->SetValue(0, FALSE);
		ReleaseISpinner(spin);
		}


	theHold.Accept(GetString(IDS_EM_BEVEL));
	if (accept)
		theHold.SuperAccept(GetString(IDS_EM_BEVEL));
	else theHold.SuperCancel();

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::Bevel(TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd) 
{
	if (!inBevel)
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
			patch->Bevel(amount, smoothStart, smoothEnd);
//			patch->MoveNormal(amount,useLocalNorms);
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
	theHold.Restore();
	patch.Bevel(amount, smoothStart, smoothEnd);

	patch.InvalidateGeomCache();
	InvalidateMesh();

	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
*/
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------


