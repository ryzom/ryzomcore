#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern int		sbmParams[4];
extern DWORD	sbsParams[3];
// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK SelectByMatDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int *param;
	switch (msg)
	{
		case WM_INITDIALOG:
			param =(int*)lParam;
			SetupIntSpinner(hWnd, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, param[0]);
			CheckDlgButton(hWnd, IDC_CLEARSELECTION, param[1]);
			/*SetupIntSpinner(hWnd, IDC_TESS_U_SPIN, IDC_TESS_U2, 1, RPO_DEFAULT_TESSEL, param[2]);
			SetupIntSpinner(hWnd, IDC_TESS_V_SPIN, IDC_TESS_V2, 1, RPO_DEFAULT_TESSEL, param[3]);*/
			CenterWindow(hWnd, GetParent(hWnd));
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK: 
					{
					ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd, IDC_MAT_IDSPIN));
					param[0] = spin->GetIVal();
					param[1] = IsDlgButtonChecked(hWnd, IDC_CLEARSELECTION);
					ReleaseISpinner(spin);
					EndDialog(hWnd, 1);					
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void SetSmoothButtonState(HWND hWnd, DWORD bits, DWORD invalid, DWORD unused = 0) 
{
	for (int i = IDC_SMOOTH_GRP1; i < IDC_SMOOTH_GRP1 + 32; i++)
	{
		if ((unused&(1 << (i - IDC_SMOOTH_GRP1))))
		{
			ShowWindow(GetDlgItem(hWnd, i), SW_HIDE);
			continue;
		}

		if ((invalid&(1 << (i - IDC_SMOOTH_GRP1))))
		{
			SetWindowText(GetDlgItem(hWnd, i), NULL);
			SendMessage(GetDlgItem(hWnd, i), CC_COMMAND, CC_CMD_SET_STATE, FALSE);
		} else 
		{
			TSTR buf;
			buf.printf(_T("%d"), i - IDC_SMOOTH_GRP1 + 1);
			SetWindowText(GetDlgItem(hWnd, i), buf);
			SendMessage(GetDlgItem(hWnd, i), CC_COMMAND, CC_CMD_SET_STATE,(bits&(1 << (i - IDC_SMOOTH_GRP1)))?TRUE:FALSE);
		}
		InvalidateRect(GetDlgItem(hWnd, i), NULL, TRUE);
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK SelectBySmoothDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	static DWORD *param;
	switch (msg)
	{
	case WM_INITDIALOG:
		param =(DWORD*)lParam;
		int i;
		for (i = IDC_SMOOTH_GRP1; i < IDC_SMOOTH_GRP1 + 32; i++)
			SendMessage(GetDlgItem(hWnd, i), CC_COMMAND, CC_CMD_SET_TYPE, CBT_CHECK);
		SetSmoothButtonState(hWnd, param[0], 0, param[2]);
		CheckDlgButton(hWnd, IDC_CLEARSELECTION, param[1]);
		CenterWindow(hWnd, GetParent(hWnd));
		break;

	case WM_COMMAND: 
		if (LOWORD(wParam) >= IDC_SMOOTH_GRP1 &&
			LOWORD(wParam) <= IDC_SMOOTH_GRP32)
		{
			ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, LOWORD(wParam)));				
			int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;				
			if (iBut->IsChecked())
			{
				param[0] |= 1 << shift;
			} else 
			{
				param[0] &= ~(1 << shift);
			}				
			ReleaseICustButton(iBut);
			break;
		}

		switch (LOWORD(wParam))
		{
		case IDOK:
			param[1] = IsDlgButtonChecked(hWnd, IDC_CLEARSELECTION);					
			EndDialog(hWnd, 1);					
			break;					

		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;			

	default:
		return FALSE;
	}
	return TRUE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchTileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;
	
	switch (message)
	{
		case WM_INITDIALOG: 
			{

		 	ep =(EditPatchMod *)lParam;
		 	ep->hTilePanel = hDlg;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);
			ep->tileNum = SetupIntSpinner(hDlg, IDC_TILE_MAT_SPIN, IDC_TILE_MAT, 0, 65535, 0);
			ep->tileRot = SetupIntSpinner(hDlg, IDC_TILE_ROT_SPIN, IDC_TILE_ROT, 0, 3, 0);
			ep->SetTileDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			if (ep->tileNum)
			{
				ReleaseISpinner(ep->tileNum);
				ep->tileNum = NULL;
			}
			if (ep->tileRot)
			{
				ReleaseISpinner(ep->tileRot);
				ep->tileRot = NULL;
			}
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			/*switch (LOWORD(wParam))
			{
				case IDC_TILE_MAT_SPIN: 
					if (HIWORD(wParam))
						break;		// No interactive action
					ep->SetTileNum (ep->tileNum->GetIVal());
					break;
				case IDC_TILE_ROT_SPIN: 
					if (HIWORD(wParam))
						break;		// No interactive action
					ep->SetTileRot (ep->tileRot->GetIVal());
					break;
			}*/
			break;

		case CC_SPINNER_BUTTONUP:
			/*switch (LOWORD(wParam))
			{
				case IDC_TILE_MAT_SPIN:
					ep->SetTileNum (ep->tileNum->GetIVal());
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;
				case IDC_TILE_ROT_SPIN:
					ep->SetTileRot (ep->tileRot->GetIVal());
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;
			}*/
			break;

		case WM_PAINT:
			if (!ep->tileUIValid)
			{
				// Tilenum
				/*ULONG u = ep->GetTileNum();
				if (u == 0xffffffff)
				{
					ep->tileNum->SetIndeterminate(TRUE);
				} 
				else 
				{
					ep->tileNum->SetIndeterminate(FALSE);
					ep->tileNum->SetValue((int)u, FALSE);
				}

				// Tilerot
				int v = ep->GetTileRot();
				if (v == -1)
				{
					ep->tileRot->SetIndeterminate(TRUE);
				} 
				else 
				{
					ep->tileRot->SetIndeterminate(FALSE);
					ep->tileRot->SetValue(v, FALSE);
				}*/
			
				ep->patchUIValid = TRUE;
			}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			/*switch (LOWORD(wParam))
			{
			}*/
			break;
		}
	
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchEdgeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;
	
	switch (message)
	{
		case WM_INITDIALOG: 
			{

		 	ep =(EditPatchMod *)lParam;
		 	ep->hEdgePanel = hDlg;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);
			ep->SetEdgeDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			return FALSE;
		
		case WM_PAINT:
			if (!ep->edgeUIValid)
			{
				// No smooth active ?
				HWND hButton=GetDlgItem (hDlg, IDC_NO_SMOOTH);
				nlassert (hButton);
				if (IsWindowEnabled (hButton))
				{
					// Get its value
					switch (ep->getSmoothFlags ())
					{
					case 0:
						CheckDlgButton (hDlg, IDC_NO_SMOOTH, BST_UNCHECKED);
						break;
					case 1:
						CheckDlgButton (hDlg, IDC_NO_SMOOTH, BST_CHECKED);
						break;
					case 2:
						CheckDlgButton (hDlg, IDC_NO_SMOOTH, BST_INDETERMINATE);
						break;
					}
				}

				// Valid now
				ep->edgeUIValid = TRUE;
			}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch (LOWORD(wParam))
			{				
				case IDC_NO_SMOOTH:
					// 3 states management
					if (IsDlgButtonChecked(hDlg, IDC_NO_SMOOTH)==BST_INDETERMINATE)
						CheckDlgButton (hDlg, IDC_NO_SMOOTH, BST_UNCHECKED);

					// Set the smooth flag for selected edges if state is checked or indeterminate
					ep->setSmoothFlags (IsDlgButtonChecked(hDlg, IDC_NO_SMOOTH)==BST_CHECKED);
					break;
			}
			break;
		}
	
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchSurfDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;
	
	switch (message)
	{
		case WM_INITDIALOG: 
			{
			ep = (EditPatchMod *)lParam;
		 	ep->hSurfPanel = hDlg;

			for (int i = IDC_SMOOTH_GRP1; i < IDC_SMOOTH_GRP1 + 32; i++)
 				SendMessage(GetDlgItem(hDlg, i), CC_COMMAND, CC_CMD_SET_TYPE, CBT_CHECK);
 			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);		 	
 			ep->matSpin = SetupIntSpinner(hDlg, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, 0);
 			ep->tessUSpin = SetupIntSpinner(hDlg, IDC_TESS_U_SPIN, IDC_TESS_U2, 1, 4, RPO_DEFAULT_TESSEL);
 			ep->tessVSpin = SetupIntSpinner(hDlg, IDC_TESS_V_SPIN, IDC_TESS_V2, 1, 4, RPO_DEFAULT_TESSEL);
		 	
			ep->SetSurfDlgEnables();

			return TRUE;
			}

		case WM_DESTROY:
			if (ep->matSpin)
			{
				ReleaseISpinner(ep->matSpin);
				ep->matSpin = NULL;
			}
			if (ep->tessUSpin)
			{
				ReleaseISpinner(ep->tessUSpin);
				ep->tessUSpin = NULL;
			}
			if (ep->tessVSpin)
			{
				ReleaseISpinner(ep->tessVSpin);
				ep->tessVSpin = NULL;
			}
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam))
			{
				case IDC_MAT_IDSPIN: 
					if (HIWORD(wParam))
						break;		// No interactive action
					ep->SetSelMatIndex(ep->matSpin->GetIVal() - 1);
					break;
				case IDC_TESS_U_SPIN: 
				case IDC_TESS_V_SPIN: 
					if (HIWORD(wParam))
						break;		// No interactive action
					ep->SetSelTess(ep->tessUSpin->GetIVal(), ep->tessVSpin->GetIVal());
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			switch (LOWORD(wParam))
			{
				case IDC_MAT_IDSPIN:
					ep->SetSelMatIndex(ep->matSpin->GetIVal() - 1);
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;
				case IDC_TESS_U_SPIN: 
				case IDC_TESS_V_SPIN: 
					ep->SetSelTess(ep->tessUSpin->GetIVal(), ep->tessVSpin->GetIVal());
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;
				}
			break;

		case WM_PAINT:
			if (!ep->patchUIValid)
			{
				// Material index
				int mat = ep->GetSelMatIndex();
				if (mat == -1)
				{
					ep->matSpin->SetIndeterminate(TRUE);
				} 
				else 
				{
					ep->matSpin->SetIndeterminate(FALSE);
					ep->matSpin->SetValue(mat + 1, FALSE);
				}
				// Smoothing groups
				DWORD invalid, bits;
				bits = ep->GetSelSmoothBits(invalid);
				SetSmoothButtonState(hDlg, bits, invalid);

				// U tess index
				int u = ep->GetSelTessU();
				if (u == -1)
				{
					ep->tessUSpin->SetIndeterminate(TRUE);
				} 
				else 
				{
					ep->tessUSpin->SetIndeterminate(FALSE);
					ep->tessUSpin->SetValue(u, FALSE);
				}

				// V tess index
				int v = ep->GetSelTessV();
				if (v == -1)
				{
					ep->tessVSpin->SetIndeterminate(TRUE);
				} 
				else 
				{
					ep->tessVSpin->SetIndeterminate(FALSE);
					ep->tessVSpin->SetValue(v, FALSE);
				}

				ep->patchUIValid = TRUE;
			}
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			if (LOWORD(wParam) >= IDC_SMOOTH_GRP1 &&
				LOWORD(wParam) <= IDC_SMOOTH_GRP32)
			{
				ICustButton *iBut = GetICustButton(GetDlgItem(hDlg, LOWORD(wParam)));
				int bit = iBut->IsChecked() ? 1 : 0;
				int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
				ep->SetSelSmoothBits(bit << shift, 1 << shift);
				ReleaseICustButton(iBut);
				break;
			}
			switch (LOWORD(wParam))
			{				
				// Material
				case IDC_SELECT_BYID: 
					{										
					if (DialogBoxParam(
						hInstance, 
						MAKEINTRESOURCE(IDD_SELECTBYMAT),
						ep->ip->GetMAXHWnd(), 
						SelectByMatDlgProc,
						(LPARAM)sbmParams))
					{
					
						ep->SelectByMat(sbmParams[0] - 1/*index*/, sbmParams[1]/*clear*/);
					}
					break;
					}
				// Smoothing groups
				case IDC_SELECTBYSMOOTH: 
					{										
					sbsParams[2] = ~ep->GetUsedSmoothBits();
					if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EM_SELECTBYSMOOTH),
								ep->ip->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)sbsParams))
					{
						ep->SelectBySmoothGroup(sbsParams[0], (BOOL)sbsParams[1]);
					}
					break;
					}
				case IDC_SMOOTH_CLEAR:
					ep->SetSelSmoothBits(0, 0xffffffff);
					break;
				// Balance button
				case IDC_BALANCE_SELECTED:
					{
						ep->BalanceSelPatch ();
						ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					}
					break;
				}
			break;
		}
	
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSurfDlgEnables() 
{
	if (!hSurfPanel)
		return;
	
	nlassert(ip);
	
	BOOL oType =(GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL pType =(GetSubobjectLevel() == EP_PATCH) ? TRUE : FALSE;

	if (oType)
		return;
	if (!pType)
		return;

	ICustButton *but;
	ISpinnerControl *spin;
	but = GetICustButton(GetDlgItem(hSurfPanel, IDC_SELECT_BYID));
	but->Enable(pType);
	ReleaseICustButton(but);
	spin = GetISpinner(GetDlgItem(hSurfPanel, IDC_MAT_IDSPIN));
	spin->Enable(pType);
	ReleaseISpinner(spin);
	for (int i = 0; i < 32; ++i)
	{
		but = GetICustButton(GetDlgItem(hSurfPanel, IDC_SMOOTH_GRP1 + i));
		but->Enable(pType);
		ReleaseICustButton(but);
		}
	but = GetICustButton(GetDlgItem(hSurfPanel, IDC_SELECTBYSMOOTH));
	but->Enable(pType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hSurfPanel, IDC_SMOOTH_CLEAR));
	but->Enable(pType);
	ReleaseICustButton(but);
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetTileDlgEnables() 
{
	if (!hTilePanel)
		return;
	
	nlassert(ip);
	
	BOOL oType =(GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL pType =(GetSubobjectLevel() == EP_TILE) ? TRUE : FALSE;

	if (oType)
		return;
	if (!pType)
		return;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetEdgeDlgEnables() 
{
	if (!hEdgePanel)
		return;
	
	nlassert(ip);
	
	BOOL oType =(GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL pType =(GetSubobjectLevel() == EP_TILE) ? TRUE : FALSE;

	if (oType)
		return;
	if (!pType)
		return;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

DWORD EditPatchMod::GetSelSmoothBits(DWORD &invalid)
	{
	BOOL first = 1;
	DWORD bits = 0;
	invalid = 0;
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip)
		return 0;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;

		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			if (patch->patchSel[j])
			{
				if (first)
				{
					first = FALSE;
					bits  = patch->patches[j].smGroup;					
				} else 
				{
					if (patch->patches[j].smGroup != bits)
					{
						invalid |= patch->patches[j].smGroup^bits;
						}
					}
				}
			}

		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
		
	nodes.DisposeTemporary();
	return bits;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

DWORD EditPatchMod::GetUsedSmoothBits()
	{	
	DWORD bits = 0;
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip)
		return 0;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;

		for (int j = 0; j < patch->getNumPatches(); j++)
		{
			bits |= patch->patches[j].smGroup;
			}		

		patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
		
	nodes.DisposeTemporary();
	return bits;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SelectBySmoothGroup(DWORD bits, BOOL clear)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchSelRestore(patchData, this, patch));
			}
		
		if (clear)
			patch->patchSel.ClearAll();			
		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->patches[j].smGroup & bits)
			{
				patch->patchSel.Set(j);			
				}
			}
		
		patchData->UpdateChanges(patch, rpatch, FALSE);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SELECTBYSMOOTH));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetSelSmoothBits(DWORD bits, DWORD which)
	{
	ModContextList mcList;	
	INodeTab nodes;

	if (!ip)
		return;
	ip->GetModContexts(mcList, nodes);
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;
		patchData->BeginEdit(ip->GetTime());
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;

		// Start a restore object...
		if (theHold.Holding())
		{
			theHold.Put(new PatchSelRestore(patchData, this, patch));
			}
		
		for (int j = 0; j < patch->getNumPatches(); j++)
		{			
			if (patch->patchSel[j])
			{
				patch->patches[j].smGroup &= ~which;
				patch->patches[j].smGroup |= bits&which;			
				}
			}
		
		patchData->UpdateChanges(patch, rpatch, FALSE);
		patchData->SetFlag(EPD_BEENDONE, TRUE);
		patchData->TempData(this)->Invalidate(PART_SELECT);
		}
		
	PatchSelChanged();
	theHold.Accept(GetString(IDS_RB_SETSMOOTHGROUP));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	InvalidateSurfaceUI();
	ip->RedrawViews(ip->GetTime());
	}

