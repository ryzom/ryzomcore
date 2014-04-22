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

// PageSimple.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include "georges_plugin_sound.h"
#include "nel/georges/u_form_elm.h"
#include "sound_document_plugin.h"
#include <string>
#include "PageSimple.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CPageSimple property page

#undef new
IMPLEMENT_DYNCREATE(CPageSimple, CPageBase)
#define new NL_NEW

CPageSimple::CPageSimple(NLGEORGES::CSoundDialog *soundDialog) : CPageBase(soundDialog, CPageSimple::IDD)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPageSimple)
	_AudioFormat = _T("");
	_Filename = _T("");
	_Filesize = _T("");
	//}}AFX_DATA_INIT
}

CPageSimple::~CPageSimple()
{
}

void CPageSimple::onDocChanged()
{
	// the document have been modified, update the dialog
	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();

	bool valid = false;

	if (pdoc != NULL)
	{
		string type, dfnName;
		NLGEORGES::UFormElm *psoundType;

		pdoc->getForm()->getRootNode().getNodeByName(&psoundType, ".SoundType");

		if (psoundType != NULL)
		{
			psoundType->getDfnName(dfnName);
			if (dfnName == "simple_sound.dfn")
			{
				uint sampleRate, sampleSize, channels, size;
				string filename;
				char s[256];

				psoundType->getValueByName(filename, "Filename");
				

				if (SoundDialog->getFileInfo(filename, sampleRate, sampleSize, channels, size))
				{

/*					if (channels > 1)
					{
						::MessageBox(NULL, "3D sounds accept only mono files", _Name.c_str(), MB_OK);
					}
*/
//					_Name = _Name.c_str());

//					SetWindowText((_Name.empty()) ? "Sound plugin" : _Name.c_str());
					_Filename = filename.c_str();

					_snprintf(s, 256, "%s / %d bits / %d Hz", (channels == 1)? "mono" : "stereo", sampleSize, sampleRate);
					_AudioFormat = s;

					_snprintf(s, 256, "%d kb", size / 1024);
					_Filesize = s;

					UpdateData(FALSE);
					valid = true;
				}
			}
		}
	}

	if (!valid)
	{
		// This is not a valid simple sound...
		_AudioFormat = "";
		_Filename = "";
		_Filesize = "";
		UpdateData(FALSE);
	}
}


void CPageSimple::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageSimple)
	DDX_Control(pDX, IDC_FILESIZE, _FilesizeCtrl);
	DDX_Control(pDX, IDC_FILENAME, _FilenameCtrl);
	DDX_Control(pDX, IDC_AUDIOFORMAT, _AudioFormatCtrl);
	DDX_Text(pDX, IDC_AUDIOFORMAT, _AudioFormat);
	DDX_Text(pDX, IDC_FILENAME, _Filename);
	DDX_Text(pDX, IDC_FILESIZE, _Filesize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageSimple, CPropertyPage)
	//{{AFX_MSG_MAP(CPageSimple)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageSimple message handlers
