#include "stdafx.h"
#include "nel_patch_paint.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

extern int attachReorient;
extern float weldThreshold;
static int patchDetachCopy = 0;
static int patchDetachReorient = 0;
;
int lockedHandles = 0;
HIMAGELIST hFaceImages = NULL;
BOOL filterVerts = TRUE;
static BOOL filterVecs = TRUE;


extern void CancelEditPatchModes(IObjParam *ip);

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern void LoadImages();

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void PaintPatchMod::SetOpsDlgEnables() 
{
	if (!hOpsPanel)
		return;
	
	nlassert(ip);
	
	ICustButton *but;

	but = GetICustButton(GetDlgItem(hOpsPanel, IDC_PAINT));
	but->Enable(TRUE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

INT_PTR CALLBACK PatchOpsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PaintPatchMod *ep =(PaintPatchMod *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!ep && message != WM_INITDIALOG)
		return FALSE;

	switch (message)
	{
		case WM_INITDIALOG: 
		{
		 	ep =(PaintPatchMod *)lParam;
		 	ep->hOpsPanel = hDlg;

			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ep);		 	

			CheckDlgButton(hDlg, IDC_INCLUDE_MESHES, ep->includeMeshes);
			CheckDlgButton(hDlg, IDC_PRELOAD_TILES, ep->preloadTiles);

			return TRUE;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			ep->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;		
		
		case WM_COMMAND:			
			switch (LOWORD(wParam))
			{				
				case IDC_INCLUDE_MESHES:
					{
						BOOL bCheck=(IsDlgButtonChecked(hDlg, IDC_INCLUDE_MESHES)==BST_CHECKED);
						ep->includeMeshes=(bCheck!=0);
						break;
					}
				case IDC_PRELOAD_TILES:
					{
						BOOL bCheck=(IsDlgButtonChecked(hDlg, IDC_PRELOAD_TILES)==BST_CHECKED);
						ep->preloadTiles=(bCheck!=0);
						break;
					}
				case IDC_PAINT:
					PaintPatchMod::paintMode->DoPaint ();
					break;
			}
			break;
		}
	
	return FALSE;
}

