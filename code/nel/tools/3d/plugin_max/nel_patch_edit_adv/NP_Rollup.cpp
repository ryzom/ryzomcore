#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

extern int attachReorient;
extern float weldThreshold;
extern PatchRightMenu pMenu;
extern PatchDeleteUser pDel;
static int patchDetachCopy = 0;
static int patchDetachReorient = 0;
;
int lockedHandles = 0;
HIMAGELIST hFaceImages = NULL;
BOOL filterVerts = TRUE;
static BOOL filterVecs = TRUE;


static void SetVertFilter() 
{
	patchHitLevel[EP_VERTEX] =(filterVerts ? SUBHIT_PATCH_VERTS : 0) |(filterVecs ? SUBHIT_PATCH_VECS : 0);
}


extern void CancelEditPatchModes(IObjParam *ip);
// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void LoadImages();

INT_PTR CALLBACK PatchSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static char string[64];
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	ICustToolbar *iToolbar;
	if (!ep && message != WM_INITDIALOG)
		return FALSE;
	
	switch (message)
	{
		case WM_INITDIALOG: 
			{
			// Get the module path
			HMODULE hModule = GetModuleHandle("neleditpatch.dlm");
			if (hModule)
			{
				// Get module file name
				char moduldeFileName[512];
				if (GetModuleFileName (hModule, moduldeFileName, 512))
				{
					// Get version info size
					DWORD doomy;
					uint versionInfoSize=GetFileVersionInfoSize (moduldeFileName, &doomy);
					if (versionInfoSize)
					{
						// Alloc the buffer
						char *buffer=new char[versionInfoSize];

						// Find the verion resource
						if (GetFileVersionInfo(moduldeFileName, 0, versionInfoSize, buffer))
						{
							uint *versionTab;
							uint versionSize;
							if (VerQueryValue (buffer, "\\", (void**)&versionTab,  &versionSize))
							{
								// Get the pointer on the structure
								VS_FIXEDFILEINFO *info=(VS_FIXEDFILEINFO*)versionTab;
								if (info)
								{
 									// Setup version number
									char version[512];
									sprintf (version, "Version %d.%d.%d.%d", 
										info->dwFileVersionMS>>16, 
										info->dwFileVersionMS&0xffff, 
										info->dwFileVersionLS>>16,  
										info->dwFileVersionLS&0xffff);
									SetWindowText (GetDlgItem (hDlg, IDC_VERSION), version);
								}
								else
									SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "VS_FIXEDFILEINFO * is NULL");
							}
							else
								SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "VerQueryValue failed");
						}
						else
							SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "GetFileVersionInfo failed");

						// Free the buffer
						delete [] buffer;
					}
					else
						SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "GetFileVersionInfoSize failed");
				}
				else
					SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "GetModuleFileName failed");
			}
			else
				SetWindowText (GetDlgItem (hDlg, IDC_VERSION), "GetModuleHandle failed");

		 	ep =(EditPatchMod *)lParam;
		 	ep->hSelectPanel = hDlg;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);
			// Set up the editing level selector
			LoadImages();
			iToolbar = GetICustToolbar(GetDlgItem(hDlg, IDC_SELTYPE));
			iToolbar->SetImage(hFaceImages);
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 0, 4, 0, 4, 24, 23, 24, 23, EP_VERTEX));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 1, 5, 1, 5, 24, 23, 24, 23, EP_EDGE));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 2, 6, 2, 6, 24, 23, 24, 23, EP_PATCH));
			iToolbar->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 3, 7, 3, 7, 24, 23, 24, 23, EP_TILE));
			ReleaseICustToolbar(iToolbar);
			ep->RefreshSelType();
			CheckDlgButton(hDlg, IDC_DISPLATTICE, ep->displayLattice);
//			CheckDlgButton( hDlg, IDC_DISPSURFACE, ep->displaySurface);
			CheckDlgButton(hDlg, IDC_FILTVERTS, filterVerts);
			CheckDlgButton(hDlg, IDC_FILTVECS, filterVecs);
			CheckDlgButton(hDlg, IDC_LOCK_HANDLES, lockedHandles);
			ep->SetSelDlgEnables();

			sprintf(string,"%s - %s",__DATE__,__TIME__);
			SetDlgItemText(hDlg,ID_VERSION,string);
		 	return TRUE;
			}

		case WM_DESTROY:
			// Don't leave in one of our modes!
			ep->ip->ClearPickMode();
			CancelEditPatchModes(ep->ip);
			return FALSE;
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND: 
			{
			BOOL needRedraw = FALSE;
			switch (LOWORD(wParam))
			{				
				case EP_VERTEX:
					if (ep->GetSubobjectLevel() == EP_VERTEX)
						ep->ip->SetSubObjectLevel(PO_OBJECT);
					else ep->ip->SetSubObjectLevel(EP_VERTEX);
					needRedraw = TRUE;
					break;

				case EP_EDGE:
					if (ep->GetSubobjectLevel() == EP_EDGE)
						ep->ip->SetSubObjectLevel(PO_OBJECT);
					else ep->ip->SetSubObjectLevel(EP_EDGE);
					needRedraw = TRUE;
					break;

				case EP_PATCH:
					if (ep->GetSubobjectLevel() == EP_PATCH)
						ep->ip->SetSubObjectLevel(PO_OBJECT);
					else ep->ip->SetSubObjectLevel(EP_PATCH);
					needRedraw = TRUE;
					break;

				case EP_TILE:
					if (ep->GetSubobjectLevel() == EP_TILE)
						ep->ip->SetSubObjectLevel(PO_OBJECT);
					else ep->ip->SetSubObjectLevel(EP_TILE);
					needRedraw = TRUE;
					break;

				case IDC_DISPLATTICE:
					ep->SetDisplayLattice(IsDlgButtonChecked(hDlg, IDC_DISPLATTICE));
					needRedraw = TRUE;
					break;
				case IDC_DISPSURFACE:
					ep->SetDisplaySurface(IsDlgButtonChecked(hDlg, IDC_DISPSURFACE));
					needRedraw = TRUE;
					break;
				case IDC_FILTVERTS:
					filterVerts = IsDlgButtonChecked(hDlg, IDC_FILTVERTS);
					EnableWindow(GetDlgItem(hDlg, IDC_FILTVECS), filterVerts ? TRUE : FALSE);
					SetVertFilter();
					break;
				case IDC_FILTVECS:
					filterVecs = IsDlgButtonChecked(hDlg, IDC_FILTVECS);
					EnableWindow(GetDlgItem(hDlg, IDC_FILTVERTS), filterVecs ? TRUE : FALSE);
					SetVertFilter();
					break;
				case IDC_LOCK_HANDLES:
					lockedHandles = IsDlgButtonChecked(hDlg, IDC_LOCK_HANDLES);
					break;
				case IDC_NS_COPY:
					ep->NSCopy();
					break;
				case IDC_NS_PASTE:
					ep->NSPaste();
					break;
				}
			if (needRedraw)
			{
				ep->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_NORMAL);
				}
			}
			break;
		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->code == TTN_NEEDTEXT)
			{
				LPTOOLTIPTEXT lpttt;
				lpttt =(LPTOOLTIPTEXT)lParam;				
				switch (lpttt->hdr.idFrom)
				{
				case EP_VERTEX:
					lpttt->lpszText = GetString(IDS_TH_VERTEX);
					break;
				case EP_EDGE:
					lpttt->lpszText = GetString(IDS_TH_EDGE);
					break;
				case EP_PATCH:
					lpttt->lpszText = GetString(IDS_TH_PATCH);
					break;
				case EP_TILE:
					lpttt->lpszText = "Tile";
					break;
				}
			}
			break;

		}
	
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void EditPatchMod::SetOpsDlgEnables() 
{
	if (!hOpsPanel)
		return;
	
	nlassert(ip);
	
	// Disconnect right-click and delete mechanisms
	ip->GetRightClickMenuManager()->Unregister(&pMenu);
	ip->UnRegisterDeleteUser(&pDel);

	BOOL oType =(GetSubobjectLevel() == EP_OBJECT) ? TRUE : FALSE;
	BOOL vType =(GetSubobjectLevel() == EP_VERTEX) ? TRUE : FALSE;
	BOOL eType =(GetSubobjectLevel() == EP_EDGE) ? TRUE : FALSE;
	BOOL pType =(GetSubobjectLevel() == EP_PATCH) ? TRUE : FALSE;
	BOOL tType =(GetSubobjectLevel() == EP_TILE) ? TRUE : FALSE;
	BOOL epType =(eType || pType) ? TRUE : FALSE;
	BOOL vepType =(vType || eType || pType) ? TRUE : FALSE;


	ICustButton *but;
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_BIND));
	but->Enable(vType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_UNBIND));
	but->Enable(vType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_SUBDIVIDE));
	but->Enable(epType);
	ReleaseICustButton(but);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_PROPAGATE), epType);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_ADDTRI));
	but->Enable(eType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_ADDQUAD));
	but->Enable(eType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_WELD));
	but->Enable(vType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_RESET));
	but->Enable(vType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_DETACH));
	but->Enable(pType);
	ReleaseICustButton(but);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_DETACHREORIENT), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_DETACHCOPY), pType);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_PATCH_DELETE));
	but->Enable(vepType);
	ReleaseICustButton(but);
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_THRESHSPINNER));
	spin->Enable(vType);
	ReleaseISpinner(spin);

// 3-1-99 watje
// 10-4-00 hulud --- bug! :-)
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_HIDE));
	but->Enable(vepType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_UNHIDE));
	but->Enable(!tType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_EP_EXTRUDE));
	but->Enable(pType);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_EP_BEVEL));
	but->Enable(pType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_TURN));
	but->Enable(pType);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_TURN2));
	but->Enable(pType);
	ReleaseICustButton(but);

	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_EXTRUDESPINNER));
	spin->Enable(pType);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hOpsPanel, IDC_EP_OUTLINESPINNER));
	spin->Enable(pType);
	ReleaseISpinner(spin);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EM_EXTYPE_A), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EM_EXTYPE_B), pType);

	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH4), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH5), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH6), pType);

	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH2), pType);
	EnableWindow(GetDlgItem(hOpsPanel, IDC_EP_SM_SMOOTH3), pType);

	// Enable/disable right-click and delete mechanisms
	if (!oType)
	{			
		pMenu.SetMod(this);
		ip->GetRightClickMenuManager()->Register(&pMenu);
		pDel.SetMod(this);
		ip->RegisterDeleteUser(&pDel);
		}
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchOpsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	EditPatchMod *ep =(EditPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;

	
	ISpinnerControl *spin;
	ICustButton *ebut;

	switch (message)
	{
		case WM_INITDIALOG: 
			{
		 	ep =(EditPatchMod *)lParam;
		 	ep->hOpsPanel = hDlg;
			for (int i = IDC_SMOOTH_GRP1; i < IDC_SMOOTH_GRP1 + 32; i++)
				SendMessage(GetDlgItem(hDlg, i), CC_COMMAND, CC_CMD_SET_TYPE, CBT_CHECK);
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);
			ICustButton *but = GetICustButton(GetDlgItem(hDlg, IDC_ATTACH));
			but->SetHighlightColor(GREEN_WASH);
			but->SetType(CBT_CHECK);
			ReleaseICustButton(but);
			CheckDlgButton(hDlg, IDC_ATTACHREORIENT, attachReorient);
			CheckDlgButton(hDlg, IDC_DETACHCOPY, patchDetachCopy);
			CheckDlgButton(hDlg, IDC_DETACHREORIENT, patchDetachReorient);
			CheckDlgButton(hDlg, IDC_PROPAGATE, ep->GetPropagate());
		 	ep->stepsSpin = GetISpinner(GetDlgItem(hDlg, IDC_STEPSSPINNER));
			ep->stepsSpin->SetLimits(0, 100, FALSE);
			ep->stepsSpin->LinkToEdit(GetDlgItem(hDlg, IDC_STEPS), EDITTYPE_POS_INT);
			ep->stepsSpin->SetValue(ep->GetMeshSteps(), FALSE);

			// Tile Step
		 	ep->tileSpin = GetISpinner(GetDlgItem(hDlg, IDC_TILESTEPSSPINNER));
			ep->tileSpin->SetLimits(-5, 5, FALSE);
			ep->tileSpin->LinkToEdit(GetDlgItem(hDlg, IDC_TILESTEPS), EDITTYPE_INT);
			ep->tileSpin->SetValue(ep->GetTileLevel(), FALSE);

			// Tile Step
		 	ep->transitionSpin = GetISpinner(GetDlgItem(hDlg, IDC_TRANSITIONSPINNER));
			ep->transitionSpin->SetLimits(1, 3, FALSE);
			ep->transitionSpin->LinkToEdit(GetDlgItem(hDlg, IDC_TRANSITION), EDITTYPE_INT);
			ep->transitionSpin->SetValue(ep->GetTransitionLevel(), FALSE);

// 3-18-99 to suport render steps and removal of the mental tesselator
		 	ep->stepsRenderSpin = GetISpinner(GetDlgItem(hDlg, IDC_STEPSRENDERSPINNER));
			ep->stepsRenderSpin->SetLimits(0, 100, FALSE);
			ep->stepsRenderSpin->LinkToEdit(GetDlgItem(hDlg, IDC_STEPS_RENDER), EDITTYPE_POS_INT);
			ep->stepsRenderSpin->SetValue(ep->GetMeshStepsRender(), FALSE);
			CheckDlgButton(hDlg, IDC_TILE_MODE, ep->GetTileMode());
			CheckDlgButton(hDlg, IDC_SHOW_INTERIOR_FACES, ep->GetShowInterior());
			CheckDlgButton(hDlg, IDC_KEEP_MAPPING, ep->GetKeepMapping());

			// Old
			EnableWindow (GetDlgItem (hDlg, IDC_STEPS), !IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_STEPSSPINNER), !IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_STEPS_RENDER), !IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_STEPSRENDERSPINNER), !IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			
			// New
			EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPS), IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_TILESTEPSSPINNER), IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_KEEP_MAPPING), IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			
			// New
			EnableWindow (GetDlgItem (hDlg, IDC_TRANSITION), IsDlgButtonChecked(hDlg, IDC_TILE_MODE));
			EnableWindow (GetDlgItem (hDlg, IDC_TRANSITIONSPINNER), IsDlgButtonChecked(hDlg, IDC_TILE_MODE));

		 	ep->weldSpin = GetISpinner(GetDlgItem(hDlg, IDC_THRESHSPINNER));
			ep->weldSpin->SetLimits(0, 999999, FALSE);
			ep->weldSpin->LinkToEdit(GetDlgItem(hDlg, IDC_WELDTHRESH), EDITTYPE_UNIVERSE);
			ep->weldSpin->SetValue(weldThreshold, FALSE);

			CheckDlgButton(hDlg, IDC_EM_EXTYPE_B, TRUE);
			CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH, TRUE);
			CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH4, TRUE);

			ep->inExtrude  = FALSE;
			ep->inBevel  = FALSE;

		// Set up spinners
			spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_EXTRUDESPINNER));
			spin->SetLimits(-9999999, 9999999, FALSE);
			spin->LinkToEdit(GetDlgItem(hDlg, IDC_EP_EXTRUDEAMOUNT), EDITTYPE_FLOAT);
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_OUTLINESPINNER));
			spin->SetLimits(-9999999, 9999999, FALSE);
			spin->LinkToEdit(GetDlgItem(hDlg, IDC_EP_OUTLINEAMOUNT), EDITTYPE_FLOAT);
			ReleaseISpinner(spin);


			ebut = GetICustButton(GetDlgItem(hDlg, IDC_EP_EXTRUDE));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg, IDC_EP_BEVEL));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ebut = GetICustButton(GetDlgItem(hDlg, IDC_BIND));
			ebut->SetType(CBT_CHECK);
			ebut->SetHighlightColor(GREEN_WASH);
			ReleaseICustButton(ebut);

			ep->matSpin   = SetupIntSpinner(hDlg, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, 0);
			ep->tessUSpin = SetupIntSpinner(hDlg, IDC_TESS_U_SPIN, IDC_TESS_U2, 1, RPO_DEFAULT_TESSEL, 0);
			ep->tessVSpin = SetupIntSpinner(hDlg, IDC_TESS_V_SPIN, IDC_TESS_V2, 1, RPO_DEFAULT_TESSEL, 0);
			ep->tileNum = SetupIntSpinner(hDlg, IDC_TILE_MAT_SPIN, IDC_TILE_MAT, 0, 65535, 0);
			ep->tileRot = SetupIntSpinner(hDlg, IDC_TILE_ROT_SPIN, IDC_TILE_ROT, 0, 3, 0);
		 	ep->SetOpsDlgEnables();

			return TRUE;
			}

		case WM_DESTROY:
			if (ep->weldSpin)
			{
				ReleaseISpinner(ep->weldSpin);
				ep->weldSpin = NULL;
			}
			if (ep->stepsSpin)
			{
				ReleaseISpinner(ep->stepsSpin);
				ep->stepsSpin = NULL;
			}
			if (ep->tileSpin)
			{
				ReleaseISpinner(ep->tileSpin);
				ep->tileSpin = NULL;
			}
			if (ep->transitionSpin)
			{
				ReleaseISpinner(ep->transitionSpin);
				ep->transitionSpin = NULL;
			}
// 3-18-99 to suport render steps and removal of the mental tesselator
			if (ep->stepsRenderSpin)
			{
				ReleaseISpinner(ep->stepsRenderSpin);
				ep->stepsRenderSpin = NULL;
			}

			// Don't leave in one of our modes!
			ep->ip->ClearPickMode();
			CancelEditPatchModes(ep->ip);
			ep->ip->UnRegisterDeleteUser(&pDel);
			ep->ip->GetRightClickMenuManager()->Unregister(&pMenu);
			return FALSE;
		
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam))
			{
				case IDC_STEPSSPINNER:
					ep->SetMeshSteps(ep->stepsSpin->GetIVal());
					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_NORMAL);
					break;
				case IDC_TILESTEPSSPINNER:
					ep->SetTileSteps(ep->tileSpin->GetIVal());
					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_NORMAL);
					break;
				case IDC_TRANSITIONSPINNER:
					ep->SetTransitionLevel(ep->transitionSpin->GetIVal());
					ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_NORMAL);
					break;
				case IDC_STEPSRENDERSPINNER:
					ep->SetMeshStepsRender(ep->stepsRenderSpin->GetIVal());
					break;

				case IDC_THRESHSPINNER:
					weldThreshold = ep->weldSpin->GetFVal();
					break;
				case IDC_EP_EXTRUDESPINNER:
					{
					bool enterKey;
					enterKey = FALSE;
					if (!HIWORD(wParam) && !ep->inExtrude)
					{
						enterKey = TRUE;
						ep->BeginExtrude(ep->ip->GetTime());
						}
					BOOL ln = IsDlgButtonChecked(hDlg, IDC_EM_EXTYPE_B);
					spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_EXTRUDESPINNER));

					ep->Extrude(ep->ip->GetTime(), spin->GetFVal(), ln);
					if (enterKey)
					{
						ep->EndExtrude(ep->ip->GetTime(), TRUE);
						spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_EXTRUDESPINNER));
						if (spin)
						{
							spin->SetValue(0, FALSE);
							ReleaseISpinner(spin);
							}

						ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
						} else 
					{
						ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_INTERACTIVE);
						}
					break;
					}
				case IDC_EP_OUTLINESPINNER:
					{
					bool enterKey;
					enterKey = FALSE;
					if (!HIWORD(wParam) && !ep->inBevel)
					{
						enterKey = TRUE;
						ep->BeginBevel(ep->ip->GetTime());
						}
					int sm =0;
					int sm2 = 0;
					if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH))
						sm = 0;					
					else if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH2))
						sm = 1;					
					else if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH3))
						sm = 2;					

					if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH4))
						sm2 = 0;					
					else if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH5))
						sm2 = 1;					
					else if (IsDlgButtonChecked(hDlg, IDC_EP_SM_SMOOTH6))
						sm2 = 2;					

					spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_OUTLINESPINNER));
					ep->Bevel(ep->ip->GetTime(), spin->GetFVal(), sm, sm2);
					if (enterKey)
					{
						ep->EndBevel(ep->ip->GetTime(), TRUE);
						spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_OUTLINESPINNER));
						if (spin)
						{
							spin->SetValue(0, FALSE);
							ReleaseISpinner(spin);
							}

						ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
						} else 
					{
						ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_INTERACTIVE);
						}
					break;
					}

				}
			break;
		case CC_SPINNER_BUTTONDOWN:
			switch (LOWORD(wParam))
			{
			case IDC_EP_EXTRUDESPINNER:
				ep->BeginExtrude(ep->ip->GetTime());
				break;
			case IDC_EP_OUTLINESPINNER:
				ep->BeginBevel(ep->ip->GetTime());
				break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			switch (LOWORD(wParam))
			{
				case IDC_EP_EXTRUDESPINNER:
					ep->EndExtrude(ep->ip->GetTime(), HIWORD(wParam));
					spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_EXTRUDESPINNER));
					if (spin)
					{
						spin->SetValue(0, FALSE);
						ReleaseISpinner(spin);
						}

					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;
				case IDC_EP_OUTLINESPINNER:
					ep->EndBevel(ep->ip->GetTime(), HIWORD(wParam));
					spin = GetISpinner(GetDlgItem(hDlg, IDC_EP_OUTLINESPINNER));
					if (spin)
					{
						spin->SetValue(0, FALSE);
						ReleaseISpinner(spin);
						}

					ep->ip->RedrawViews(ep->ip->GetTime(), REDRAW_END);
					break;


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
				// Subdivision
// watje 3-18-99
				case IDC_SHOW_INTERIOR_FACES:
						ep->SetShowInterior(IsDlgButtonChecked(hDlg, IDC_SHOW_INTERIOR_FACES));
//						ep->InvalidateMesh();
//						ep->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
//						ep->ip->RedrawViews (ep->ip->GetTime(),REDRAW_END);
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
						EnableWindow (GetDlgItem (hDlg, IDC_KEEP_MAPPING), bCheck);
						
						// New
						EnableWindow (GetDlgItem (hDlg, IDC_TRANSITION), bCheck);
						EnableWindow (GetDlgItem (hDlg, IDC_TRANSITIONSPINNER), bCheck);
						break;
					}
				case IDC_KEEP_MAPPING:
					{
						ep->SetKeepMapping(IsDlgButtonChecked(hDlg, IDC_KEEP_MAPPING)!=0);
						break;
					}
// watje 12-10-98
				case IDC_HIDE:
					ep->DoHide(ep->GetSubobjectLevel());
					break;
				case IDC_UNHIDE:
					ep->DoUnHide();
					break;
				case IDC_BIND:
//			ep->DoAddHook();
					if (ep->ip->GetCommandMode() == ep->bindMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->bindMode);
					break;
				case IDC_UNBIND:
					ep->DoRemoveHook();
					break;
// extrude and bevel stuff
// watje 12-10-98
				case IDC_EP_SM_SMOOTH:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH2, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH3, FALSE);
					break;
				case IDC_EP_SM_SMOOTH2:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH3, FALSE);
					break;
				case IDC_EP_SM_SMOOTH3:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH2, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH, FALSE);
					break;

				case IDC_EP_SM_SMOOTH4:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH5, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH6, FALSE);
					break;
				case IDC_EP_SM_SMOOTH5:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH4, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH6, FALSE);
					break;
				case IDC_EP_SM_SMOOTH6:
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH4, FALSE);
					CheckDlgButton(hDlg, IDC_EP_SM_SMOOTH5, FALSE);
					break;


				case IDC_EP_EXTRUDE:
					if (ep->ip->GetCommandMode() == ep->extrudeMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->extrudeMode);
					break;
				case IDC_EP_BEVEL:
					if (ep->ip->GetCommandMode() == ep->bevelMode)
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
					else ep->ip->SetCommandMode(ep->bevelMode);
					break;
				
				case IDC_TURN:
					nlassert (ep->GetSubobjectLevel()==PO_PATCH);
					ep->DoPatchTurn(true);
					break;

				case IDC_TURN2:
					nlassert (ep->GetSubobjectLevel()==PO_PATCH);
					ep->DoPatchTurn(false);
					break;

				case IDC_SUBDIVIDE:
					ep->DoSubdivide(ep->GetSubobjectLevel());
					break;
				case IDC_PROPAGATE:
					ep->SetPropagate(IsDlgButtonChecked(hDlg, IDC_PROPAGATE));
					break;
				// Topology
				case IDC_ADDTRI:
					if (ep->GetSubobjectLevel() == PO_EDGE)
						ep->DoPatchAdd(PATCH_TRI);
					break;
				case IDC_ADDQUAD:
					if (ep->GetSubobjectLevel() == PO_EDGE)
						ep->DoPatchAdd(PATCH_QUAD);
					break;
				case IDC_WELD:
					ep->DoVertWeld();
					break;
				case IDC_RESET:
					ep->DoVertReset();
					break;
				case IDC_DETACH:
					ep->DoPatchDetach(patchDetachCopy, patchDetachReorient);
					break;
				case IDC_DETACHCOPY:
					patchDetachCopy = IsDlgButtonChecked(hDlg, IDC_DETACHCOPY);
					break;
				case IDC_DETACHREORIENT:
					patchDetachReorient = IsDlgButtonChecked(hDlg, IDC_DETACHREORIENT);
					break;
				case IDC_ATTACH: 
					{
					ModContextList mcList;
					INodeTab nodes;
					// If the mode is on, turn it off and bail
					if (ep->ip->GetCommandMode()->ID() == CID_STDPICK)
					{
						ep->ip->SetStdCommandMode(CID_OBJMOVE);
						return FALSE;
						}
					// Want to turn on the mode.  Make sure we're valid first
					ep->ip->GetModContexts(mcList, nodes);
					ep->pickCB.ep = ep;
					ep->ip->SetPickMode(&ep->pickCB);
					nodes.DisposeTemporary();
					break;
					}
				case IDC_ATTACHREORIENT:
					attachReorient = IsDlgButtonChecked(hDlg, IDC_ATTACHREORIENT);
					break;
				case IDC_PATCH_DELETE:
					ep->DoDeleteSelected();
					break;
				}
			break;
		}
	
	return FALSE;
}

