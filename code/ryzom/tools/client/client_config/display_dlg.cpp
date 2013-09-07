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

// ***************************************************************************
// display_dlg.cpp : implementation file
// ***************************************************************************

#include "stdafx.h"
#include "client_config.h"
#include "display_dlg.h"
#include "cfg_file.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

std::vector<string>		GLExtensions;
std::string				GLRenderer;
std::string				GLVendor;
std::string				GLVersion;
std::string				D3DDescription;
std::string				D3DDeviceName;
std::string				D3DDriver;
std::string				D3DDriverVersion;
std::string				D3DVendor;

uint					VideoMemory;
uint					HardwareSoundBuffer;
uint64					SystemMemory;
uint					CPUFrequency;

bool GetGLInformation ()
{
	// *** INIT VARIABLES

	GLExtensions.clear ();
	GLRenderer = "";
	GLVendor = "";
	GLVersion = "";

	// *** INIT OPENGL

	// Register a window class
	WNDCLASS		wc;
	memset(&wc,0,sizeof(wc));
	wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
	wc.lpfnWndProc		= DefWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.hIcon			= (HICON)NULL;
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= WHITE_BRUSH;
	wc.lpszClassName	= "RyzomGetGlInformation";
	wc.lpszMenuName		= NULL;
	if ( !RegisterClass(&wc) ) 
		return false;

	// Create a window
	ULONG	WndFlags = WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
	RECT	WndRect;
	WndRect.left=0;
	WndRect.top=0;
	WndRect.right=100;
	WndRect.bottom=100;
	HWND hWnd = CreateWindow (	"RyzomGetGlInformation",
		"",
		WndFlags,
		CW_USEDEFAULT,CW_USEDEFAULT,
		WndRect.right,WndRect.bottom,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);
	if (!hWnd) 
		return false;

	HDC hDC = GetDC(hWnd);

	// Remove current gl context
	wglMakeCurrent (hDC, NULL);

	// Select pixel format
	int depth = GetDeviceCaps (hDC, BITSPIXEL);
	PIXELFORMATDESCRIPTOR	pfd;
	memset(&pfd,0,sizeof(pfd));
	pfd.nSize        = sizeof(pfd);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType   = PFD_TYPE_RGBA;
	pfd.cColorBits   = (char)depth;
	if(depth<=16)
		pfd.cDepthBits   = 16;
	else
	{
		pfd.cDepthBits = 24;
		pfd.cAlphaBits	= 8;
	}
	pfd.iLayerType	  = PFD_MAIN_PLANE;
	int pf=ChoosePixelFormat(hDC, &pfd);
	if (!pf)
		return false;
	if ( !SetPixelFormat(hDC, pf, &pfd) ) 
		return false;

	// Create final context
	HGLRC hRC = wglCreateContext (hDC);
	wglMakeCurrent(hDC, hRC);

	// *** GET INFORMATION

	GLVendor = (const char *) glGetString (GL_VENDOR);
	GLRenderer = (const char *) glGetString (GL_RENDERER);
	GLVersion = (const char *) glGetString (GL_VERSION);

	// Get extension string
	string glext = (char*)glGetString(GL_EXTENSIONS);
	const char *token = strtok ( (char*)glext.c_str (), " ");
	while( token != NULL )
	{
		GLExtensions.push_back (token);
		token = strtok( NULL, " ");
	}

	// Get proc adress
	typedef	const char*	(APIENTRY * PFNWGFGETEXTENSIONSSTRINGARB) (HDC);
	PFNWGFGETEXTENSIONSSTRINGARB wglGetExtensionsStringARB;
	if ((wglGetExtensionsStringARB=(PFNWGFGETEXTENSIONSSTRINGARB)wglGetProcAddress("wglGetExtensionsStringARB")))
	{
		// Get extension string
		glext = wglGetExtensionsStringARB (hDC);
		token = strtok ( (char*)glext.c_str (), " ");
		while( token != NULL )
		{
			GLExtensions.push_back (token);
			token = strtok( NULL, " ");
		}
	}

	// *** RELEASE OPENGL

	if (hRC)
		wglDeleteContext(hRC);
	if (hWnd&&hDC)
	{
		ReleaseDC (hWnd,hDC);
		DestroyWindow (hWnd);
	}

	// Done
	return true;
}

// ***************************************************************************

bool GetD3DInformation (NL3D::IDriver *d3dDriver)
{
	IDriver::CAdapter desc;
	if (d3dDriver->getAdapter (0xffffffff, desc))
	{
		D3DDescription = desc.Description;
		D3DDeviceName = desc.DeviceName;
		D3DDriver = desc.Driver;
		D3DDriverVersion = toString ((uint16)(desc.DriverVersion>>48))+"."+
			toString ((uint16)(desc.DriverVersion>>32))+"."+
			toString ((uint16)(desc.DriverVersion>>16))+"."+
			toString ((uint16)(desc.DriverVersion&0xffff));
		D3DVendor = desc.Vendor;		
		return true;
	}
	return false;
}

// ***************************************************************************

bool GetVideoMemory ()
{
	VideoMemory = 0;
	bool ret = false;

	// Initialise Direct Draw
	IDirectDraw *dd;
	HRESULT result = DirectDrawCreate(NULL, &dd, NULL);
	if (result == DD_OK)
	{
		DDCAPS caps;
		memset (&caps, 0, sizeof(DDCAPS));
		caps.dwSize = sizeof(DDCAPS);
		dd->GetCaps (&caps, NULL);
		VideoMemory = caps.dwVidMemTotal;
		ret = true;
		dd->Release ();
	}

	// Can't get it
	return ret;
}

// ***************************************************************************

bool GetHardwareSoundBuffer ()
{
	bool ret = false;
	HardwareSoundBuffer = 0;

	// The DirectSound object
#if (DIRECTSOUND_VERSION >= 0x0800)
    LPDIRECTSOUND8 _DirectSound;
#else
	LPDIRECTSOUND _DirectSound;
#endif

#if (DIRECTSOUND_VERSION >= 0x0800)
	if (DirectSoundCreate8(NULL, &_DirectSound, NULL) == DS_OK)
#else
	if (DirectSoundCreate(NULL, &_DirectSound, NULL) == DS_OK)
#endif
	{
		DSCAPS caps;
		memset (&caps, 0, sizeof (DSCAPS));
		caps.dwSize = sizeof (DSCAPS);

		HRESULT result = _DirectSound->GetCaps (&caps);
		if (result == DS_OK)
		{
			HardwareSoundBuffer = caps.dwFreeHw3DStaticBuffers;
			ret = true;
		}
		_DirectSound->Release ();
	}
	return ret;
}

// ***************************************************************************

bool GetSystemInformation (IDriver *d3dDriver)
{
	bool result = GetGLInformation ();
	result |= GetD3DInformation (d3dDriver);
	result |= GetVideoMemory ();
	result |= GetHardwareSoundBuffer ();
	SystemMemory = CSystemInfo::totalPhysicalMemory ();
	CPUFrequency = (uint)(CSystemInfo::getProcessorFrequency () / (uint64)1000000);
	return result;
}
// ***************************************************************************

std::vector<CVideoMode>		VideoModes[2];

// ***************************************************************************

void RegisterVideoModes (uint driverId, IDriver *driver)
{
	vector<GfxMode> modes;
	driver->getModes (modes);

	uint i;
	VideoModes[driverId].clear();
	for (i=0; i<modes.size(); i++)
	{
		// Keep only 32 bits
		if ((modes[i].Depth == 32) && (modes[i].Width >= 800) && (modes[i].Width >= 600))
		{
			// Add this mode
			CVideoMode mode;
			mode.Width = modes[i].Width;
			mode.Height = modes[i].Height;
			mode.ColorDepth = modes[i].Depth;
			mode.Frequency = modes[i].Frequency;
			VideoModes[driverId].push_back (mode);
		}
	}
}

// ***************************************************************************
// CDisplayDlg dialog
// ***************************************************************************

CDisplayDlg::CDisplayDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDisplayDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayDlg)
	DriverChoiceMode = DrvChooseUnknwown;
	Windowed = -1;
	Width = 0;
	Height = 0;
	Mode = -1;
	PositionX = 0;
	PositionY = 0;
	//}}AFX_DATA_INIT
}

// ***************************************************************************

void CDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayDlg)
	DDX_Control(pDX, IDC_DISPLAY_FS_TEXT0, TextFS0);
	DDX_Control(pDX, IDC_DISPLAY_WND_TEXT0, TextWnd0);
	DDX_Control(pDX, IDC_DISPLAY_WND_TEXT1, TextWnd1);
	DDX_Control(pDX, IDC_DISPLAY_WND_TEXT2, TextWnd2);
	DDX_Control(pDX, IDC_DISPLAY_WND_TEXT3, TextWnd3);
	DDX_Control(pDX, IDC_POSITION_Y, PositionYCtrl);
	DDX_Control(pDX, IDC_POSITION_X, PositionXCtrl);
	DDX_Control(pDX, IDC_HEIGHT, HeightCtrl);
	DDX_Control(pDX, IDC_WIDTH, WidthCtrl);
	DDX_Control(pDX, IDC_MODE, ModeCtrl);
	DDX_Radio(pDX, IDC_DRV3D_AUTO, DriverChoiceMode);
	DDX_Radio(pDX, IDC_FULLSCREEN, Windowed);
	DDX_Text(pDX, IDC_WIDTH, Width);
	DDX_Text(pDX, IDC_HEIGHT, Height);
	DDX_CBIndex(pDX, IDC_MODE, Mode);
	DDX_Text(pDX, IDC_POSITION_X, PositionX);
	DDX_Text(pDX, IDC_POSITION_Y, PositionY);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

BEGIN_MESSAGE_MAP(CDisplayDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayDlg)
	ON_BN_CLICKED(IDC_FULLSCREEN, OnFullscreen)
	ON_BN_CLICKED(IDC_WINDOW, OnWindow)
	ON_BN_CLICKED(IDC_DIRECT3D, OnDirect3d)
	ON_BN_CLICKED(IDC_DRV3D_AUTO, OnDrv3DAuto)
	ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
	ON_BN_CLICKED(IDC_OPENGL, OnOpengl)
	ON_EN_CHANGE(IDC_POSITION_X, OnChangePositionX)
	ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
	ON_EN_CHANGE(IDC_POSITION_Y, OnChangePositionY)
	ON_CBN_SELCHANGE(IDC_MODE, OnSelchangeMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ***************************************************************************
// CDisplayDlg message handlers
// ***************************************************************************

void CDisplayDlg::OnFullscreen() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnWindow() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::updateState ()
{
	ModeCtrl.EnableWindow (Windowed == 0);
	TextFS0.EnableWindow (Windowed == 0);
	WidthCtrl.EnableWindow (Windowed == 1);
	HeightCtrl.EnableWindow (Windowed == 1);
	PositionXCtrl.EnableWindow (Windowed == 1);
	PositionYCtrl.EnableWindow (Windowed == 1);
	TextWnd0.EnableWindow (Windowed == 1);
	TextWnd1.EnableWindow (Windowed == 1);
	TextWnd2.EnableWindow (Windowed == 1);
	TextWnd3.EnableWindow (Windowed == 1);

	// Fill the combobox values
	ModeCtrl.ResetContent ();
	uint i;
	if (DriverChoiceMode != DrvChooseUnknwown)
	{
		TDriver actualDriver = getActualDriver();
		for (i=0; i<VideoModes[actualDriver].size (); i++)
		{
			ucstring videoMode = toString (VideoModes[actualDriver][i].Width) + "x" + toString (VideoModes[actualDriver][i].Height) + 
				" " + toString (VideoModes[actualDriver][i].ColorDepth) + " " + CI18N::get ("uiConfigBits") + " " + 
				toString (VideoModes[actualDriver][i].Frequency) + " " + CI18N::get ("uiConfigHz");

			// hulud : Don't know how to set a wide string in a combo box...
			ModeCtrl.SendMessage (CB_INSERTSTRING, -1, (LPARAM)(videoMode.toString().c_str()));
		}
	}

	// Select the string
	ModeCtrl.SetCurSel (Mode);
}

// ***************************************************************************

BOOL CDisplayDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	updateState ();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CDisplayDlg::OnDirect3d() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************
void CDisplayDlg::OnDrv3DAuto()
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnChangeHeight() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnOpengl() 
{
	UpdateData (TRUE);
	updateState ();
	UpdateData (FALSE);
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnChangePositionX() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnChangeWidth() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnChangePositionY() 
{
	InvalidateConfig ();
}

// ***************************************************************************

void CDisplayDlg::OnSelchangeMode() 
{
	InvalidateConfig ();
}

// ***************************************************************************
CDisplayDlg::TDriver CDisplayDlg::getActualDriver() const
{
	switch(DriverChoiceMode)
	{
		case DrvChooseOpenGL: return OpenGL;
		case DrvChooseDirect3D: return Direct3D;
		default:		
		{				
			// D3D better on ATI Radeon cards
			std::string deviceName;
			uint64 drvVersion;
			CSystemInfo::getVideoInfo(deviceName, drvVersion);
			return strstr(toLower(deviceName).c_str(), "radeon") != NULL ? Direct3D : OpenGL;
		}		
	}
	return OpenGL;
}
