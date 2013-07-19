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

#include "std_sound_plugin.h"
#include "PageComplex.h"
#include "PageComtext.h"
#include "PageBgFlags.h"
#include "PageBgFades.h"
#include "PageSimple.h"
#include "PagePosition.h"

#include "sound_dialog.h"

#include "listener_view.h"
#include "sound_document_plugin.h"
#include "resource.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/georges/u_form_elm.h"
#include <mmsystem.h>
#include <math.h>


using namespace std;
using namespace NLMISC;

namespace NLGEORGES
{

CBitmap CSoundDialog::_StopBitmap;
CBitmap CSoundDialog::_StartBitmap;
CBitmap CSoundDialog::_DesactivatedBitmap;
CBitmap CSoundDialog::_NewBitmap;
CBitmap CSoundDialog::_ContextBitmap;
CPen CSoundDialog::_Red;


BEGIN_MESSAGE_MAP(CSoundDialog, CDialog)
	//{{AFX_MSG_MAP(CSoundDialog)
	ON_WM_TIMER()
	ON_WM_KEYUP()
	ON_BN_CLICKED(IDC_CONTROL, OnControlPlayback)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ZOOM, OnZoom)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD_SAMPLES, OnReloadSamples)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CSoundDialog::CSoundDialog() 
: CDialog(), 
	_Plugin(0), 
	_Playing(false),
	_Timer(0), 
	_PagePosition(0),
	_PageSimple(0),
	_PageComplex(0),
	_PageComtext(0),
	_PageBgFlags(0)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CSoundDialog)
	//}}AFX_DATA_INIT

}

// ***************************************************************************

CSoundDialog::~CSoundDialog()     
{ 
}


// ***************************************************************************

void CSoundDialog::init(CSoundPlugin* plugin, HWND documentView)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_Plugin = plugin;

	if (HBITMAP(_StopBitmap) == 0)
	{
		_StopBitmap.LoadBitmap(IDB_STOP);
		_StartBitmap.LoadBitmap(IDB_START);
		_DesactivatedBitmap.LoadBitmap(IDB_DESACTIVATED);
		_NewBitmap.LoadBitmap(IDB_NEW);
		_ContextBitmap.LoadBitmap(IDB_CONTEXT);
		_Red.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	}

	_OuterAngle = 360;
	_InnerAngle = 360;

	Create(IDD_TEST_LOCAL, CWnd::FromHandle(documentView));

	NLSOUND::UAudioMixer *mixer = _Plugin->getMixer();

	// set the name an fade
	FilterFades = mixer->getBackgroundFilterFades();
	_BackgroundFlags = mixer->getBackgroundFlags();
	for (uint i=0; i<NLSOUND::UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		TEnvName envName;

		envName.Name = mixer->getBackgroundFlagName(i);
		envName.ShortName = mixer->getBackgroundFlagShortName(i);

		EnvNames.push_back(envName);
	}

//	CConfigFile &cf = _Plugin->getGlobalInterface()->getConfigFile();

/*	try
	{
		CConfigFile::CVar *penvNames = cf.getVarPtr("background_sound_environment");

		EnvNames.clear();
		sint i;
		for (i=0; i< min(penvNames->size() / 4, 32); ++i)
		{
			TEnvName envName;

			envName.Name = penvNames->asString(i*4);
			envName.ShortName = penvNames->asString(i*4+1);
			FilterFades.FadeIns[i] = penvNames->asInt(i*4+2);
			FilterFades.FadeOuts[i] = penvNames->asInt(i*4+3);

			EnvNames.push_back(envName);
		}
		for (; i < 32; ++i)
		{
			TEnvName envName;
			char tmp[50];
			sprintf(tmp, "Filter %02u", i);
			envName.Name = tmp;
			sprintf(tmp, "%02u", i);
			envName.ShortName = tmp;
			EnvNames.push_back(envName);
		}
	}
	catch(...)
	{
	}

*/	return;

	CRect rect(0,0,200,200);
#undef new
	CWnd *wnd = new CWnd();
#define new NL_NEW

}


void CSoundDialog::DoDataExchange(CDataExchange *pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_MAP(CSoundDialog)
	DDX_Control(pDX, IDC_SHEET_POS, _SheetPos);
	//}}AFX_DATA_MAP
}

// ***************************************************************************

void CSoundDialog::setPlaying(bool play)
{
	_Playing = play;

	updateButton();
}

// ***************************************************************************

// ***************************************************************************

void CSoundDialog::OnTimer(UINT_PTR id)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	updateTime();
}

// ***************************************************************************

void CSoundDialog::OnZoom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

// ***************************************************************************

void CSoundDialog::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_SPACE) && _Plugin->isSoundValid())
	{
//		_Plugin->play(_Filename);
		OnControlPlayback();
	}
}

// ***************************************************************************

void CSoundDialog::updateTime()
{
	_Plugin->update();
	uint32 msec = _Plugin->getTime();

//	if (msec == 0)
	if (!_Plugin->isPlaying())
	{
		setPlaying(false);
		return;
	}

	displayTime(msec);
}

// ***************************************************************************

void CSoundDialog::displayTime(uint32 msec)
{
	char s[256];
	uint sec = msec / 1000;
	uint min = sec / 60;
	sec -= min * 60;
	_snprintf(s, 256, "%02d:%02d", min, sec);
	GetDlgItem(IDC_TIME)->SetWindowText(s);
}

// ***************************************************************************

void CSoundDialog::updateButton()
{
	CButton* control = (CButton*) GetDlgItem(IDC_CONTROL);

//	if (_Filename.empty())
	if (!_Plugin->isSoundValid())
	{
		control->SetBitmap(_DesactivatedBitmap);
		control->SetCheck(0);
	}
	else
	{
		if (_Playing)
		{
			control->SetBitmap(_StopBitmap);
			control->SetCheck(1);
		}
		else
		{
			control->SetBitmap(_StartBitmap);
			control->SetCheck(0);
		}
	}
}

void CSoundDialog::setDuration(uint32 msec)
{
	_Duration = msec;
	updateInfo();
}



// ***************************************************************************

void CSoundDialog::updateInfo()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CPageBase::docChanged();

	SetWindowText(_Name.c_str());

	char s[256];
/*	if (_Filename.empty())
	{
		SetWindowText((_Name.empty()) ? "Sound plugin" : _Name.c_str());
		SetWindowText("[no file selected]");
		GetDlgItem(IDC_NAME)->SetWindowText(_Name.c_str());
		GetDlgItem(IDC_FILENAME)->SetWindowText("[no file selected]");
		GetDlgItem(IDC_AUDIOFORMAT)->SetWindowText("");
		GetDlgItem(IDC_FILESIZE)->SetWindowText("");				
//		GetDlgItem(IDC_DURATION)->SetWindowText("00:00.000");
	}
	else
	{	
		uint sampleRate, sampleSize, channels, size;

		if (getFileInfo(_Filename, sampleRate, sampleSize, channels, size))
		{

			if (channels > 1)
			{
				::MessageBox(NULL, "3D sounds accept only mono files", _Name.c_str(), MB_OK);
			}

			GetDlgItem(IDC_NAME)->SetWindowText(_Name.c_str());

			SetWindowText((_Name.empty()) ? "Sound plugin" : _Name.c_str());
			GetDlgItem(IDC_FILENAME)->SetWindowText(_Filename.c_str());

			_snprintf(s, 256, "%s / %d bits / %d Hz", (channels == 1)? "mono" : "stereo", sampleSize, sampleRate);
			GetDlgItem(IDC_AUDIOFORMAT)->SetWindowText(s);

			_snprintf(s, 256, "%d kb", size / 1024);
			GetDlgItem(IDC_FILESIZE)->SetWindowText(s);

//			uint msec = 1000 * size / sampleRate;
		}
		else
		{
			_Filename.erase();
			SetWindowText((_Name.empty()) ? "Sound plugin" : _Name.c_str());
			GetDlgItem(IDC_NAME)->SetWindowText(_Name.c_str());
			GetDlgItem(IDC_FILENAME)->SetWindowText("[invalid file]");
			GetDlgItem(IDC_AUDIOFORMAT)->SetWindowText("");
			GetDlgItem(IDC_FILESIZE)->SetWindowText("");
//			GetDlgItem(IDC_DURATION)->SetWindowText("00:00.000");
		}
	}
*/
	// the duration is always valid.
	uint msec = _Duration;
	uint sec = msec / 1000;
	uint min = sec / 60;
	msec -= sec * 1000;
	sec -= min * 60;
	_snprintf(s, 256, "%02d:%02d.%03d", min, sec, msec);
	GetDlgItem(IDC_DURATION)->SetWindowText(s);

	updateButton();
}

// ***************************************************************************

bool CSoundDialog::getFileInfo(string& filename, uint& sampleRate, uint& sampleSize, uint& channels, uint& size)
{
	// Try to find the absolute path of the file
	string path = CPath::lookup(filename, false, false, true);

	if (path.empty())
	{
		// If we failed try to open the file as a local file anyway
		path = filename;
	}

	// Open the file 
	HMMIO hmmio = mmioOpen((char*) path.c_str(), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (hmmio == NULL) 
	{
		return false;
	}


	// Check it's a WAVE file 
	MMCKINFO riff_chunk;
	sint error = (sint) mmioDescend(hmmio, &riff_chunk, NULL, 0);
	if ((error != 0) || (riff_chunk.ckid != FOURCC_RIFF) || (riff_chunk.fccType != mmioFOURCC('W', 'A', 'V', 'E'))) 
	{
		mmioClose(hmmio, 0);
		return false;
	}


	// Search the format chunk 
	MMCKINFO chunk;
	chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	error = (sint) mmioDescend(hmmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
	if (error != 0) 
	{
		mmioClose(hmmio, 0);
		return false;
	}

	if (chunk.cksize < (long) sizeof(PCMWAVEFORMAT)) 
	{
		mmioClose(hmmio, 0);
		return false;
	}


	// read in the format data
	WAVEFORMATEX format;
	sint num = mmioRead(hmmio, (HPSTR) &format, (long) sizeof(format));
	if (num != (long) sizeof(format)) 
	{
		mmioClose(hmmio, 0);
		return false;
	}
	format.cbSize = 0;

	// Get out of the format chunk
	if (mmioAscend(hmmio, &chunk, 0) != 0) 
	{
		mmioClose(hmmio, 0);
		return false;
	}


	// Copy the format data 
	if (format.wFormatTag != WAVE_FORMAT_PCM) 
	{
		mmioClose(hmmio, 0);
		return false;
	}

	sampleRate = format.nSamplesPerSec;
	channels = format.nChannels;
	sampleSize = format.wBitsPerSample;

	
	// Set the file position to the beginning of the data chunk 
    sint32 pos = mmioSeek(hmmio, riff_chunk.dwDataOffset + sizeof(FOURCC), SEEK_SET);
    if (pos < 0) 
    {
        mmioClose(hmmio, 0);
		return false;
    }

	// Read the data chunk
	MMCKINFO data_chunk;
    data_chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &data_chunk, &riff_chunk, MMIO_FINDCHUNK) != 0) 
    {
        mmioClose(hmmio, 0);
		return false;
    }


    // Copy the size
    size = data_chunk.cksize;

	// We're done
	mmioClose(hmmio, 0);

	return true;
}

// ***************************************************************************

void CSoundDialog::updateAngles() 
{
//	_ListenerView->setActive(!_Filename.empty());
/*	_ListenerView->setActive(true);
	_ListenerView->setShowAlpha(_Plugin->hasAlpha());
	_ListenerView->setAngles(_InnerAngle, _OuterAngle);
*/
}

void CSoundDialog::fillContextArgs(NLSOUND::CSoundContext *context)
{
	if (_PageComtext != 0)
	{
		for (uint i =0; i<NLSOUND::SoundContextNbArgs; ++i)
			context->Args[i] = _PageComtext->SoundContext.Args[i];
	}
}


// ***************************************************************************

void CSoundDialog::OnControlPlayback()
{
	if (!_Playing)
	{
		// check filename
		string str = NLMISC::CFile::getFilename(_Filename);

		if (str == "*.sound")
		{
			MessageBox("You must save the file before playing it !", "Warning");
			return;
		}
	}
	setPlaying(!_Playing);

	if (_Playing)
	{
		_Plugin->play(_Filename);
	}
	else
	{
		_Plugin->stop();
	}
}


void CSoundDialog::OnReloadSamples() 
{
	_Plugin->reloadSamples();
}

BOOL CSoundDialog::OnInitDialog() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDialog::OnInitDialog();
	
	WINDOWPLACEMENT place;
	_SheetPos.GetWindowPlacement(&place);

	// create the property sheet
#undef new
	CPropertySheet *sheet = new CPropertySheet();

	// create the property page...
	_PagePosition = new CPagePosition(this); 
	_PageSimple = new CPageSimple(this); 
	_PageComplex = new CPageComplex(this);		
	_PageComtext = new CPageComtext(this); 
	_PageBgFlags = new CPageBgFlags(this); 
	_PageBgFades = new CPageBgFades(this); 
#define new NL_NEW

	sheet->AddPage(_PagePosition);
	sheet->AddPage(_PageSimple);
	sheet->AddPage(_PageComplex);
	sheet->AddPage(_PageComtext);
	sheet->AddPage(_PageBgFlags);
	sheet->AddPage(_PageBgFades);

	sheet->Create(this, WS_CHILD | WS_VISIBLE);
	// very important : the folowing two style ensure not entering into an infinite loop !
	sheet->ModifyStyleEx (0, WS_EX_CONTROLPARENT);
//	sheet->ModifyStyle( 0, WS_TABSTOP );
	// place the property sheet onver the picture
	sheet->SetWindowPos(NULL, place.rcNormalPosition.left, place.rcNormalPosition.top, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);
	sheet->ShowWindow(TRUE);

	// start the timer for mixer update
	_Timer = SetTimer(1, 3, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoundDialog::OnDestroy() 
{
	CDialog::OnDestroy();
	KillTimer(_Timer);

	delete _PagePosition;
	delete _PageSimple;
	delete _PageComplex;
	delete _PageComtext;
	delete _PageBgFlags;
	delete _PageBgFades;

	_PagePosition = NULL;
	_PageSimple = NULL;
	_PageComplex = NULL;
	_PageComtext = NULL;
	_PageBgFlags = NULL;
	_PageBgFades = NULL;
}

} // namespace NLGEORGES


