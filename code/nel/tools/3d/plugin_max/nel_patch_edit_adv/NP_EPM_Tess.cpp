#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

extern AdvParams sParams;
INT_PTR CALLBACK AdvParametersDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetTessUI(HWND hDlg, TessApprox *tess)
{
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), FALSE);

	CheckDlgButton(hDlg, IDC_TESS_SET, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_REGULAR, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_PARAM, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_SPATIAL, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_CURV, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_LDA, FALSE);

	ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_HIDE);

// watje 12-10-98
	if (tess->showInteriorFaces)
		CheckDlgButton(hDlg, IDC_SHOW_INTERIOR_FACES, TRUE);
	else CheckDlgButton(hDlg, IDC_SHOW_INTERIOR_FACES, FALSE);
	if (tileMode)
	{
		CheckDlgButton(hDlg, IDC_TILE_MODE, TRUE);
	}
	else 
	{
		CheckDlgButton(hDlg, IDC_TILE_MODE, FALSE);
	}
	// Old
	BOOL bCheck=(IsDlgButtonChecked(hDlg, IDC_TILE_MODE)==BST_CHECKED);
	EnableWindow (GetDlgItem (hDlg, IDC_STEPS), !bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_STEPSSPINNER), !bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_STEPS_RENDER), !bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_STEPSRENDERSPINNER), !bCheck);
	
	// New
	EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPS), bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPSSPINNER), bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_TRANSITION), bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_TRANSITIONSPINNER), bCheck);
	EnableWindow (GetDlgItem (hDlg, IDC_KEEP_MAPPING), bCheck);

	if (keepMapping)
		CheckDlgButton(hDlg, IDC_KEEP_MAPPING, TRUE);
	else 
		CheckDlgButton(hDlg, IDC_KEEP_MAPPING, FALSE);

	switch (tess->type)
	{
	case TESS_SET:
		CheckDlgButton(hDlg, IDC_TESS_SET, TRUE);
		mergeSpin->Disable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
		break;

	case TESS_REGULAR:
		CheckDlgButton(hDlg, IDC_TESS_REGULAR, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), TRUE);

		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), SW_HIDE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_PARAM:
		CheckDlgButton(hDlg, IDC_TESS_PARAM, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_U_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_V_SPINNER), TRUE);

		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_SPATIAL:
		CheckDlgButton(hDlg, IDC_TESS_SPATIAL, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_CURVE:
		CheckDlgButton(hDlg, IDC_TESS_CURV, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;

	case TESS_LDA:
		CheckDlgButton(hDlg, IDC_TESS_LDA, TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_EDGE_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_DIST_SPINNER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_TESS_ANG_SPINNER), TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), !settingViewportTess);
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), settingViewportTess?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_PARAMETERS), TRUE);
		mergeSpin->Enable();
		EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), FALSE);
		break;
	}


	if (settingViewportTess)
	{
		ShowWindow(GetDlgItem(hDlg, IDC_TESS_SET), SW_SHOW);

		if (tess->type != TESS_SET)
		{
			ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_SHOW);
			EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), !GetViewTessWeld());
			EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), tess->merge > 0.0f);
		}
	} else 
	{
		if (settingDisp)
		{
			ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_SHOW);
		} else 
		{
			if (tess->type != TESS_SET)
			{
				ShowWindow(GetDlgItem(hDlg, IDC_MESH), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_DISP), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_WELDTESS), SW_SHOW);
				EnableWindow(GetDlgItem(hDlg, IDC_TESS_NORMALS), !GetProdTessWeld());
				EnableWindow(GetDlgItem(hDlg, IDC_WELDTESS), tess->merge > 0.0f);
				CheckDlgButton(hDlg, IDC_MESH, TRUE);
			}
			ShowWindow(GetDlgItem(hDlg, IDC_TESS_SET), SW_SHOW);
		}
	}

	// now set all the settings
	uSpin->SetValue(tess->u, FALSE);
	vSpin->SetValue(tess->v, FALSE);
	edgeSpin->SetValue(tess->edge, FALSE);
	distSpin->SetValue(tess->dist, FALSE);
	angSpin->SetValue(tess->ang, FALSE);
	mergeSpin->SetValue(tess->merge, FALSE);
	CheckDlgButton(hDlg, IDC_TESS_VIEW_DEP, tess->view);
	if (settingViewportTess)
	{
		CheckDlgButton(hDlg, IDC_TESS_VIEW, TRUE);
		CheckDlgButton(hDlg, IDC_TESS_RENDERER, FALSE);
		CheckDlgButton(hDlg, IDC_TESS_NORMALS, GetViewTessNormals());
		CheckDlgButton(hDlg, IDC_WELDTESS, GetViewTessWeld());
	} else 
	{
		CheckDlgButton(hDlg, IDC_TESS_VIEW, FALSE);
		CheckDlgButton(hDlg, IDC_TESS_RENDERER, TRUE);
		CheckDlgButton(hDlg, IDC_TESS_NORMALS, GetProdTessNormals());
		CheckDlgButton(hDlg, IDC_WELDTESS, GetProdTessWeld());
	}
	CheckDlgButton(hDlg, IDC_DISP, settingDisp);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
	
INT_PTR CALLBACK PatchObjSurfDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;
	
	switch (message)
	{
		case WM_INITDIALOG: 
			{

		 	ep =(EditPatchMod *)lParam;
		 	ep->hSurfPanel = hDlg;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);		 	
			if (!ep->settingViewportTess  && ep->settingDisp && ep->GetProdTess().type == TESS_SET)
				ep->settingDisp = FALSE;
			TessApprox t;
			if (ep->settingViewportTess)
			{
				t = ep->GetViewTess();
			} else 
			{
				if (ep->settingDisp)
					t = ep->GetDispTess();
				else
					t = ep->GetProdTess();
			}
			ep->uSpin = SetupIntSpinner(hDlg, IDC_TESS_U_SPINNER, IDC_TESS_U, 1, 100, t.u);
			ep->vSpin = SetupIntSpinner(hDlg, IDC_TESS_V_SPINNER, IDC_TESS_V, 1, 100, t.v);
#define MAX_F 1000.0f
			ep->edgeSpin = SetupFloatSpinner(hDlg, IDC_TESS_EDGE_SPINNER, IDC_TESS_EDGE, 0.0f, MAX_F, t.edge);
			ep->distSpin = SetupFloatSpinner(hDlg, IDC_TESS_DIST_SPINNER, IDC_TESS_DIST, 0.0f, MAX_F, t.dist);
			ep->angSpin =  SetupFloatSpinner(hDlg, IDC_TESS_ANG_SPINNER,  IDC_TESS_ANG, 0.0f, MAX_F, t.ang);
			ep->mergeSpin =  SetupFloatSpinner(hDlg, IDC_MERGE_SPINNER,  IDC_MERGE, 0.000f, MAX_F, t.merge);
			ep->SetTessUI(hDlg, &t);
		 	ep->SetSurfDlgEnables();
			ep->SetTileDlgEnables();
			return TRUE;
			}

		case WM_DESTROY:
			if (ep->uSpin)
			{
				ReleaseISpinner(ep->uSpin);
				ep->uSpin = NULL;
				}
			if (ep->vSpin)
			{
				ReleaseISpinner(ep->vSpin);
				ep->vSpin = NULL;
				}
			if (ep->edgeSpin)
			{
				ReleaseISpinner(ep->edgeSpin);
				ep->edgeSpin = NULL;
				}
			if (ep->distSpin)
			{
				ReleaseISpinner(ep->distSpin);
				ep->distSpin = NULL;
				}
			if (ep->angSpin)
			{
				ReleaseISpinner(ep->angSpin);
				ep->angSpin = NULL;
				}
			if (ep->mergeSpin)
			{
				ReleaseISpinner(ep->mergeSpin);
				ep->mergeSpin = NULL;
				}
			return FALSE;
		
		case CC_SPINNER_BUTTONUP: 
			{
			TessApprox tess;
			if (ep->settingViewportTess)
			{
				tess = ep->GetViewTess();
			} else 
			{
				if (ep->settingDisp)
					tess = ep->GetDispTess();
				else
					tess = ep->GetProdTess();
			}
			ep->SetTessUI(hDlg, &tess);
			}
			break;

		case CC_SPINNER_CHANGE:	
			switch (LOWORD(wParam))
			{
				case IDC_TESS_U_SPINNER:
				case IDC_TESS_V_SPINNER:
				case IDC_TESS_EDGE_SPINNER:
				case IDC_TESS_DIST_SPINNER:
				case IDC_TESS_ANG_SPINNER:
				case IDC_MERGE_SPINNER:
					{
					TessApprox tess;
					if (ep->settingViewportTess)
					{
						tess = ep->GetViewTess();
					} else 
					{
						if (ep->settingDisp)
							tess = ep->GetDispTess();
						else
							tess = ep->GetProdTess();
					}
					switch (LOWORD(wParam))
					{
						case IDC_TESS_U_SPINNER:
							tess.u = ep->uSpin->GetIVal();
							break;
						case IDC_TESS_V_SPINNER:
							tess.v = ep->vSpin->GetIVal();
							break;
						case IDC_TESS_EDGE_SPINNER:
							tess.edge = ep->edgeSpin->GetFVal();
							break;
						case IDC_TESS_DIST_SPINNER:
							tess.dist = ep->distSpin->GetFVal();
							break;
						case IDC_TESS_ANG_SPINNER:
							tess.ang = ep->angSpin->GetFVal();
							break;
						case IDC_MERGE_SPINNER:
							tess.merge = ep->mergeSpin->GetFVal();
							break;
						}
					if (ep->settingViewportTess)
					{
						ep->SetViewTess(tess);
					} else 
					{
						if (ep->settingDisp)
							ep->SetDispTess(tess);
						else
							ep->SetProdTess(tess);
					}
					if (!HIWORD(wParam))
						ep->SetTessUI(hDlg, &tess);
					break;
					}
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch (LOWORD(wParam))
			{				
				// Tessellation
				case IDC_TESS_VIEW:	
					{
					ep->settingViewportTess = TRUE;
					TessApprox t = ep->GetViewTess();
					ep->SetTessUI(hDlg, &t);
					EnableWindow(GetDlgItem(hDlg, IDC_TESS_VIEW_DEP), FALSE); // always off here
					break;
					}
				case IDC_TESS_RENDERER: 
					{
					ep->settingViewportTess = FALSE;
					if (ep->settingDisp)
					{
						TessApprox t = ep->GetDispTess();
						ep->SetTessUI(hDlg, &t);
					} else 
					{
						TessApprox t = ep->GetProdTess();
						ep->SetTessUI(hDlg, &t);
					}
					break;
					}
				case IDC_MESH:
					ep->settingDisp = FALSE;
					ep->SetTessUI(hDlg, &ep->GetProdTess());
					break;
				case IDC_DISP:
					ep->settingDisp = TRUE;
					ep->SetTessUI(hDlg, &ep->GetDispTess());
					break;
				case IDC_TILE_MODE:
					{
						BOOL bCheck=(IsDlgButtonChecked(hDlg, IDC_TILE_MODE)==BST_CHECKED);
						ep->SetTileMode (bCheck!=0);
						
						// Old
						EnableWindow (GetDlgItem (hDlg, IDC_STEPS), !bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_STEPSSPINNER), !bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_STEPS_RENDER), !bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_STEPSRENDERSPINNER), !bCheck);
						
						// New
						EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPS), bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPSSPINNER), bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_TRANSITION), bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_TRANSITIONSPINNER), bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_KEEP_MAPPING), bCheck);

						ep->SetTessUI(hDlg, &ep->GetDispTess());
					}
					break;
				case IDC_KEEP_MAPPING:
					ep->SetKeepMapping (IsDlgButtonChecked(hDlg, IDC_KEEP_MAPPING)!=0);
					ep->SetTessUI(hDlg, &ep->GetDispTess());
					break;
// watje 12-10-98
				case IDC_SHOW_INTERIOR_FACES:
				case IDC_TESS_SET:
				case IDC_TESS_REGULAR:
				case IDC_TESS_PARAM:
				case IDC_TESS_SPATIAL:
				case IDC_TESS_CURV:
				case IDC_TESS_LDA:
					{
					TessApprox tess;
					if (ep->settingViewportTess)
					{
						tess = ep->GetViewTess();
					} else 
					{
						if (ep->settingDisp)
							tess = ep->GetDispTess();
						else
							tess = ep->GetProdTess();
					}
					switch (LOWORD(wParam))
					{
// watje 12-10-98
					case IDC_SHOW_INTERIOR_FACES:
						tess.showInteriorFaces = IsDlgButtonChecked(hDlg, IDC_SHOW_INTERIOR_FACES);
						break;
					case IDC_TESS_SET:
						tess.type = TESS_SET;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					case IDC_TESS_REGULAR:
						tess.type = TESS_REGULAR;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					case IDC_TESS_PARAM:
						tess.type = TESS_PARAM;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					case IDC_TESS_SPATIAL:
						tess.type = TESS_SPATIAL;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					case IDC_TESS_CURV:
						tess.type = TESS_CURVE;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					case IDC_TESS_LDA:
						tess.type = TESS_LDA;
						EnableWindow(GetDlgItem(hDlg, IDC_SHOW_INTERIOR_FACES), TRUE);
						break;
					}
					if (ep->settingViewportTess)
					{
						ep->SetViewTess(tess);
					} else 
					{
						if (ep->settingDisp)
							ep->SetDispTess(tess);
						else
							ep->SetProdTess(tess);
					}
					ep->SetTessUI(hDlg, &tess);
					}
					break;
				case IDC_TESS_VIEW_DEP: 
					{
					TessApprox tess;
					tess = ep->GetProdTess();
					tess.view = IsDlgButtonChecked(hDlg, IDC_TESS_VIEW_DEP);
					if (ep->settingDisp)
						ep->SetDispTess(tess);
					else
						ep->SetProdTess(tess);
					}
					break;
				case IDC_TESS_NORMALS:
					if (ep->settingViewportTess)
					{
						ep->SetViewTessNormals(IsDlgButtonChecked(hDlg, IDC_TESS_NORMALS));
						ep->SetTessUI(hDlg, &ep->GetViewTess());
					} else 
					{
						ep->SetProdTessNormals(IsDlgButtonChecked(hDlg, IDC_TESS_NORMALS));
						if (ep->settingDisp)
							ep->SetTessUI(hDlg, &ep->GetDispTess());
						else
							ep->SetTessUI(hDlg, &ep->GetProdTess());
					}
					break;
				case IDC_WELDTESS:
					if (ep->settingViewportTess)
					{
						ep->SetViewTessWeld(IsDlgButtonChecked(hDlg, IDC_WELDTESS));
						ep->SetTessUI(hDlg, &ep->GetViewTess());
					} else 
					{
						ep->SetProdTessWeld(IsDlgButtonChecked(hDlg, IDC_WELDTESS));
						if (ep->settingDisp)
							ep->SetTessUI(hDlg, &ep->GetDispTess());
						else
							ep->SetTessUI(hDlg, &ep->GetProdTess());
					}
					break;
				case IDC_ADVANCED_PARAMETERS: 
					{
					TessApprox tess;
					if (ep->settingViewportTess)
					{
						tess = ep->GetViewTess();
					} else 
					{
						if (ep->settingDisp)
							tess = ep->GetDispTess();
						else
							tess = ep->GetProdTess();
					}
					sParams.mStyle = tess.subdiv;
					sParams.mMin = tess.minSub;
					sParams.mMax = tess.maxSub;
					sParams.mTris = tess.maxTris;
					int retval = DialogBox(hInstance,
								MAKEINTRESOURCE(IDD_SURF_APPROX_ADV),
								ep->ip->GetMAXHWnd(), AdvParametersDialogProc);
					if (retval == 1)
					{
						BOOL confirm = FALSE;
						if ((sParams.mStyle == SUBDIV_DELAUNAY && sParams.mTris > 200000) ||
(sParams.mStyle != SUBDIV_DELAUNAY && sParams.mMax > 5))
						{
							// warning!
							TSTR title = GetString(IDS_ADV_SURF_APPROX_WARNING_TITLE),
								warning = GetString(IDS_ADV_SURF_APPROX_WARNING);
							if (MessageBox(hDlg, warning, title,
								MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)
								confirm = TRUE;
 
						} else
							confirm = TRUE;
						if (confirm)
						{
							// do it, they've been warned!
							tess.subdiv = sParams.mStyle;
							tess.minSub = sParams.mMin;
							tess.maxSub = sParams.mMax;
							tess.maxTris = sParams.mTris;
							if (ep->settingViewportTess)
							{
								ep->SetViewTess(tess);
							} else 
							{
								if (ep->settingDisp)
									ep->SetDispTess(tess);
								else
									ep->SetProdTess(tess);
							}
						}
					}
					break;
					}
				}
			break;
		}
	
	return FALSE;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

AdvParams sParams;
static ISpinnerControl* psMinSpin = NULL;
static ISpinnerControl* psMaxSpin = NULL;
static ISpinnerControl* psMaxTrisSpin = NULL;
// this max matches the MI max.
#define MAX_SUBDIV 7
static BOOL initing = FALSE; // this is a hack but CenterWindow causes bad commands
INT_PTR CALLBACK
AdvParametersDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
	{
    case WM_INITDIALOG: 
		{
		initing = TRUE;
        CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
		initing = FALSE;
		psMinSpin = SetupIntSpinner(hDlg, IDC_TESS_MIN_REC_SPINNER, IDC_TESS_MIN_REC, 0, sParams.mMax, sParams.mMin);
		psMaxSpin = SetupIntSpinner(hDlg, IDC_TESS_MAX_REC_SPINNER, IDC_TESS_MAX_REC, sParams.mMin, MAX_SUBDIV, sParams.mMax);
		psMaxTrisSpin = SetupIntSpinner(hDlg, IDC_TESS_MAX_TRIS_SPINNER, IDC_TESS_MAX_TRIS, 0, 2000000, sParams.mTris);
		switch (sParams.mStyle)
		{
		case SUBDIV_GRID:
			CheckDlgButton(hDlg, IDC_GRID, TRUE);
			CheckDlgButton(hDlg, IDC_TREE, FALSE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_TREE:
			CheckDlgButton(hDlg, IDC_GRID, FALSE);
			CheckDlgButton(hDlg, IDC_TREE, TRUE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, FALSE);
			break;
		case SUBDIV_DELAUNAY:
			CheckDlgButton(hDlg, IDC_GRID, FALSE);
			CheckDlgButton(hDlg, IDC_TREE, FALSE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, TRUE);
			break;
		}
		break; 
		}

    case WM_COMMAND:
		if (initing)
			return FALSE;
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, 1);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDC_GRID:
			sParams.mStyle = SUBDIV_GRID;
			CheckDlgButton(hDlg, IDC_GRID, TRUE);
			CheckDlgButton(hDlg, IDC_TREE, FALSE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, FALSE);
			break;
		case IDC_TREE:
			sParams.mStyle = SUBDIV_TREE;
			CheckDlgButton(hDlg, IDC_GRID, FALSE);
			CheckDlgButton(hDlg, IDC_TREE, TRUE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, FALSE);
			break;
		case IDC_DELAUNAY:
			sParams.mStyle = SUBDIV_DELAUNAY;
			CheckDlgButton(hDlg, IDC_GRID, FALSE);
			CheckDlgButton(hDlg, IDC_TREE, FALSE);
			CheckDlgButton(hDlg, IDC_DELAUNAY, TRUE);
			break;
		}
		break;

    case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam))
		{
		case IDC_TESS_MIN_REC_SPINNER:
			sParams.mMin = psMinSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_REC_SPINNER:
			sParams.mMax = psMaxSpin->GetIVal();
			psMinSpin->SetLimits(0, sParams.mMax, FALSE);
			psMaxSpin->SetLimits(sParams.mMin, MAX_SUBDIV, FALSE);
			break;
		case IDC_TESS_MAX_TRIS_SPINNER:
			sParams.mTris = psMaxTrisSpin->GetIVal();
			break;
		}
		break;

	case WM_DESTROY:
		if (psMinSpin)
		{
			ReleaseISpinner(psMinSpin);
			psMinSpin = NULL;
		}
		if (psMaxSpin)
		{
			ReleaseISpinner(psMaxSpin);
			psMaxSpin = NULL;
		}
		if (psMaxTrisSpin)
		{
			ReleaseISpinner(psMaxTrisSpin);
			psMaxTrisSpin = NULL;
		}
		break;
	}

	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetViewTess(TessApprox &tess) 
{
	viewTess = tess;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetViewTess(tess);
		patchData->viewTess = tess;
		//rpatch->rTess = rTess;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetProdTess(TessApprox &tess) 
{
	prodTess = tess;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetProdTess(tess);
		patchData->prodTess = tess;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetDispTess(TessApprox &tess) 
{
	dispTess = tess;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetDispTess(tess);
		patchData->dispTess = tess;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetViewTessNormals(BOOL use) 
{
	mViewTessNormals = use;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetViewTessNormals(use);
		patchData->mViewTessNormals = use;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetProdTessNormals(BOOL use) 
{
	mProdTessNormals = use;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetProdTessNormals(use);
		patchData->mProdTessNormals = use;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetViewTessWeld(BOOL weld) 
{
	mViewTessWeld = weld;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetViewTessWeld(weld);
		patchData->mViewTessWeld = weld;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetProdTessWeld(BOOL weld) 
{
	mProdTessWeld = weld;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(),rpatch);
		if (!patch)
			continue;
		
		patch->SetProdTessWeld(weld);
		patchData->mProdTessWeld = weld;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

