// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#if !defined(AFX_DISPLAY_DLG_H__1287701D_07D1_48F0_8F13_3FCE937DA3A2__INCLUDED_)
#define AFX_DISPLAY_DLG_H__1287701D_07D1_48F0_8F13_3FCE937DA3A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// display_dlg.h : header file
//

#include "base_dialog.h"
#include "resource.h"

// ***************************************************************************
// CDisplayDlg dialog
// ***************************************************************************

class CDisplayDlg : public CBaseDialog
{
// Construction
public:
	CDisplayDlg(CWnd* pParent = NULL);   // standard constructor
	enum TDriverChoiceMode { DrvChooseAuto = 0, DrvChooseOpenGL, DrvChooseDirect3D, DrvChooseUnknwown = -1 };
	enum TDriver { OpenGL = 0, Direct3D };
// Dialog Data
	//{{AFX_DATA(CDisplayDlg)
	enum { IDD = IDD_DISPLAY };
	CStatic	TextFS0;
	CStatic	TextWnd0;
	CStatic	TextWnd1;
	CStatic	TextWnd2;
	CStatic	TextWnd3;
	CEdit	PositionYCtrl;
	CEdit	PositionXCtrl;
	CEdit	HeightCtrl;
	CEdit	WidthCtrl;
	CComboBox	ModeCtrl;
	int		DriverChoiceMode; // one of the TDriverChoiceMode values
	int		Windowed;
	UINT	Width;
	UINT	Height;
	int		Mode;
	int		PositionX;
	int		PositionY;
	//}}AFX_DATA

	// Update data
	void updateState ();

	/** Get the selected driver (if Driver is in mode Auto, choose the best driver depending on the hardware)
	  * \return 0 for D3D & 1 for OpenGL
	  */
	TDriver	getActualDriver() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDisplayDlg)
	afx_msg void OnFullscreen();
	afx_msg void OnWindow();
	virtual BOOL OnInitDialog();
	afx_msg void OnDirect3d();
	afx_msg void OnDrv3DAuto();
	afx_msg void OnChangeHeight();
	afx_msg void OnOpengl();
	afx_msg void OnChangePositionX();
	afx_msg void OnChangeWidth();
	afx_msg void OnChangePositionY();
	afx_msg void OnColorDepth32();
	afx_msg void OnSelchangeMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

class CVideoMode
{
public:
	uint	Width;
	uint	Height;
	uint	ColorDepth;	// 32 only
	uint	Frequency;

	// Operator
	bool	operator== (const CVideoMode &other) const
	{
		return (Width == other.Width) && (Height == other.Height) && (ColorDepth == other.ColorDepth) && (Frequency == other.Frequency);
	}
	bool	operator< (const CVideoMode &other) const
	{
		if (Width < other.Width) return true;
		else if (Width > other.Width) return false;
		else if (Height < other.Height) return true;
		else if (Height > other.Height) return false;
		else if (ColorDepth < other.ColorDepth) return true;
		else if (ColorDepth > other.ColorDepth) return false;
		else if (Frequency < other.Frequency) return true;
		else return false;
	}
};

// ***************************************************************************

enum
{
	ModeOpenGL = 0,
	ModeDirectX
};

// ***************************************************************************

extern std::vector<CVideoMode>		VideoModes[2];
extern std::vector<std::string>		GLExtensions;
extern std::string					GLRenderer;
extern std::string					GLVendor;
extern std::string					GLVersion;
extern std::string					D3DDescription;
extern std::string					D3DDeviceName;
extern std::string					D3DDriver;
extern std::string					D3DDriverVersion;
extern std::string					D3DVendor;
extern uint							VideoMemory;
extern uint							HardwareSoundBuffer;
extern uint64						SystemMemory;
extern uint							CPUFrequency;

// ***************************************************************************

// Register video modes
void RegisterVideoModes (uint mode, NL3D::IDriver *driver);

// Get opengl information
bool GetSystemInformation (NL3D::IDriver *d3dDriver);

// ***************************************************************************

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAY_DLG_H__1287701D_07D1_48F0_8F13_3FCE937DA3A2__INCLUDED_)


