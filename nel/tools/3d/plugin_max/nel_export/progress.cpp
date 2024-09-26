// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std_afx.h"
#include "progress.h"
#include "resource.h"
#include "nel_export.h"
#include "nel/misc/time_nl.h"
using namespace std;
using namespace NLMISC;


// -----------------------------------------------------------------------------------------------
// Window dialog callback
// -----------------------------------------------------------------------------------------------
INT_PTR CALLBACK CalculatingDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	double TimeCurrent = CTime::ticksToSecond( CTime::getPerformanceTime() );
	CProgressBar *pClass = (CProgressBar*)GetWindowLongPtr (hwndDlg, GWLP_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			LONG res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			pClass = (CProgressBar*)GetWindowLongPtr (hwndDlg, GWLP_USERDATA);

			CenterWindow( hwndDlg, theCNelExport._Ip->GetMAXHWnd() );
			ShowWindow( hwndDlg, SW_SHOWNORMAL );
			pClass->rRatioCalculated = 0.0;
			pClass->bCancelCalculation = false;
		}
		break;

		case WM_PAINT:
		{
			TCHAR temp[256];
			SendMessage( GetDlgItem( hwndDlg, IDC_PROGRESS1 ), PBM_SETPOS, (int)(pClass->rRatioCalculated*100), 0 );

			if( pClass->rRatioCalculated > 0.0 )
			{
				double TimeLeft = (TimeCurrent - pClass->rTimeBegin) * (1.0-pClass->rRatioCalculated);
				_stprintf( temp, _T("Time remaining : %02d h %02d m %02d s"), ((uint32)TimeLeft)/3600,
																		(((uint32)TimeLeft)/60)%60,
																		(((uint32)TimeLeft))%60 );
				if (pClass->bCancelCalculation)
					_tcscpy_s (temp, 256, _T("INTERRUPTED - Finishing current object..."));

				SendMessage (GetDlgItem (hwndDlg, IDC_STATICTIMELEFT), WM_SETTEXT, 0, (LPARAM)temp);
				SendMessage (GetDlgItem (hwndDlg, IDC_BUTTONCANCEL), WM_PAINT, 0, 0);
			}
			string all;
			for (uint32 i = 0; i < 14; ++i)
				all += pClass->sInfoProgress[i] + "\n";
			SendMessage (GetDlgItem (hwndDlg, IDC_STATICINFO), WM_SETTEXT, 0, (LPARAM)MaxTStrFromUtf8(all).data());
		}
		break;

		case WM_DESTROY:
			pClass->bCancelCalculation = true;
		break;
		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				// ---
				case IDC_BUTTONCANCEL:
					if( HIWORD(wParam) == BN_CLICKED )
						pClass->bCancelCalculation = true;
				break;
				default:
				break;
			}
		}		
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}


// -----------------------------------------------------------------------------------------------
CProgressBar::CProgressBar()
{
	hWndProgress = NULL;
}

// -----------------------------------------------------------------------------------------------
CProgressBar::~CProgressBar()
{
}

// -----------------------------------------------------------------------------------------------
// Initialize the dialog with the total number of meshes to treat
void CProgressBar::initProgressBar( sint32 nNbMesh, Interface &ip)
{
	rTimeBegin = CTime::ticksToSecond( CTime::getPerformanceTime() );
	nNbTotalMeshes = nNbMesh;
	hWndProgress = CreateDialogParam(	CNelExportDesc.HInstance(), 
										MAKEINTRESOURCE(IDD_CALCULATING),
										NULL,//ip.GetMAXHWnd(), 
										CalculatingDialogCallback,
										(LPARAM)this );
}

// -----------------------------------------------------------------------------------------------
void CProgressBar::uninitProgressBar()
{
	DestroyWindow( hWndProgress );
	hWndProgress = NULL;
}

// -----------------------------------------------------------------------------------------------
void CProgressBar::update()
{
	if (hWndProgress == NULL)
		return;

	MSG msg;
	PeekMessage (&msg, (HWND)hWndProgress,0, 0, PM_REMOVE);
	{
		if (IsDialogMessage (hWndProgress ,&msg))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
}

// -----------------------------------------------------------------------------------------------
void CProgressBar::setLine(uint32 nLineNb, string &sText)
{
	sInfoProgress[nLineNb] = sText;
}

// -----------------------------------------------------------------------------------------------
// Update with the current mesh treated
void CProgressBar::updateProgressBar (sint32 NMeshNb)
{
	rRatioCalculated = ((float)NMeshNb) / ((float)nNbTotalMeshes);
	update();
}

// -----------------------------------------------------------------------------------------------
bool CProgressBar::isCanceledProgressBar()
{
	return bCancelCalculation;
}

