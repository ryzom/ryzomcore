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

// PageBgFlags.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "nel/misc/types_nl.h"
#include "georges_plugin_sound.h"
#include "nel/georges/u_form_elm.h"
#include "sound_document_plugin.h"
#include "nel/sound/u_audio_mixer.h"

#include "PageBgFlags.h"
#include "resource.h"

using namespace std;

// controls id arrays
uint	FILTER_EDIT[] =
{
	IDC_FILTER_00,
	IDC_FILTER_01,
	IDC_FILTER_02,
	IDC_FILTER_03,
	IDC_FILTER_04,
	IDC_FILTER_05,
	IDC_FILTER_06,
	IDC_FILTER_07,
	IDC_FILTER_08,
	IDC_FILTER_09,
	IDC_FILTER_10,
	IDC_FILTER_11,
	IDC_FILTER_12,
	IDC_FILTER_13,
	IDC_FILTER_14,
	IDC_FILTER_15,
	IDC_FILTER_16,
	IDC_FILTER_17,
	IDC_FILTER_18,
	IDC_FILTER_19,
	IDC_FILTER_20,
	IDC_FILTER_21,
	IDC_FILTER_22,
	IDC_FILTER_23,
	IDC_FILTER_24,
	IDC_FILTER_25,
	IDC_FILTER_26,
	IDC_FILTER_27,
	IDC_FILTER_28,
	IDC_FILTER_29,
	IDC_FILTER_30,
	IDC_FILTER_31
};

uint	FILTER_EDIT_NAME[] =
{
	IDC_FILTER_NAME_00,
	IDC_FILTER_NAME_01,
	IDC_FILTER_NAME_02,
	IDC_FILTER_NAME_03,
	IDC_FILTER_NAME_04,
	IDC_FILTER_NAME_05,
	IDC_FILTER_NAME_06,
	IDC_FILTER_NAME_07,
	IDC_FILTER_NAME_08,
	IDC_FILTER_NAME_09,
	IDC_FILTER_NAME_10,
	IDC_FILTER_NAME_11,
	IDC_FILTER_NAME_12,
	IDC_FILTER_NAME_13,
	IDC_FILTER_NAME_14,
	IDC_FILTER_NAME_15,
	IDC_FILTER_NAME_16,
	IDC_FILTER_NAME_17,
	IDC_FILTER_NAME_18,
	IDC_FILTER_NAME_19,
	IDC_FILTER_NAME_20,
	IDC_FILTER_NAME_21,
	IDC_FILTER_NAME_22,
	IDC_FILTER_NAME_23,
	IDC_FILTER_NAME_24,
	IDC_FILTER_NAME_25,
	IDC_FILTER_NAME_26,
	IDC_FILTER_NAME_27,
	IDC_FILTER_NAME_28,
	IDC_FILTER_NAME_29,
	IDC_FILTER_NAME_30,
	IDC_FILTER_NAME_31
};

uint	FILTER_SIM[] =
{
	IDC_ENV_FLAG_00,
	IDC_ENV_FLAG_01,
	IDC_ENV_FLAG_02,
	IDC_ENV_FLAG_03,
	IDC_ENV_FLAG_04,
	IDC_ENV_FLAG_05,
	IDC_ENV_FLAG_06,
	IDC_ENV_FLAG_07,
	IDC_ENV_FLAG_08,
	IDC_ENV_FLAG_09,
	IDC_ENV_FLAG_10,
	IDC_ENV_FLAG_11,
	IDC_ENV_FLAG_12,
	IDC_ENV_FLAG_13,
	IDC_ENV_FLAG_14,
	IDC_ENV_FLAG_15,
	IDC_ENV_FLAG_16,
	IDC_ENV_FLAG_17,
	IDC_ENV_FLAG_18,
	IDC_ENV_FLAG_19,
	IDC_ENV_FLAG_20,
	IDC_ENV_FLAG_21,
	IDC_ENV_FLAG_22,
	IDC_ENV_FLAG_23,
	IDC_ENV_FLAG_24,
	IDC_ENV_FLAG_25,
	IDC_ENV_FLAG_26,
	IDC_ENV_FLAG_27,
	IDC_ENV_FLAG_28,
	IDC_ENV_FLAG_29,
	IDC_ENV_FLAG_30,
	IDC_ENV_FLAG_31,
};

uint	FILTER_SIM_NAME[] =
{
	IDC_ENV_NAME_00,
	IDC_ENV_NAME_01,
	IDC_ENV_NAME_02,
	IDC_ENV_NAME_03,
	IDC_ENV_NAME_04,
	IDC_ENV_NAME_05,
	IDC_ENV_NAME_06,
	IDC_ENV_NAME_07,
	IDC_ENV_NAME_08,
	IDC_ENV_NAME_09,
	IDC_ENV_NAME_10,
	IDC_ENV_NAME_11,
	IDC_ENV_NAME_12,
	IDC_ENV_NAME_13,
	IDC_ENV_NAME_14,
	IDC_ENV_NAME_15,
	IDC_ENV_NAME_16,
	IDC_ENV_NAME_17,
	IDC_ENV_NAME_18,
	IDC_ENV_NAME_19,
	IDC_ENV_NAME_20,
	IDC_ENV_NAME_21,
	IDC_ENV_NAME_22,
	IDC_ENV_NAME_23,
	IDC_ENV_NAME_24,
	IDC_ENV_NAME_25,
	IDC_ENV_NAME_26,
	IDC_ENV_NAME_27,
	IDC_ENV_NAME_28,
	IDC_ENV_NAME_29,
	IDC_ENV_NAME_30,
	IDC_ENV_NAME_31,
};

std::map<uint, uint>	FILTER_SIM_IDX;
std::map<uint, uint>	FILTER_EDIT_IDX;
/////////////////////////////////////////////////////////////////////////////
// CPageBgFlags property page

#undef new
IMPLEMENT_DYNCREATE(CPageBgFlags, CPageBase)
#define new NL_NEW

CPageBgFlags::CPageBgFlags(NLGEORGES::CSoundDialog *soundDialog) 
: CPageBase(soundDialog, CPageBgFlags::IDD),
  _recurse(false)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPageBgFlags)
	_SubSoundName = _T("");
	//}}AFX_DATA_INIT

	for (uint i=0; i<32; ++i)
	{
		FILTER_EDIT_IDX.insert(make_pair(FILTER_EDIT[i], i));
		FILTER_SIM_IDX.insert(make_pair(FILTER_SIM[i], i));
	}
}

CPageBgFlags::~CPageBgFlags()
{
}

void CPageBgFlags::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageBgFlags)
	DDX_Text(pDX, IDC_SUB_SOUND_NAME, _SubSoundName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageBgFlags, CPropertyPage)
	//{{AFX_MSG_MAP(CPageBgFlags)
	ON_BN_CLICKED(IDC_BTN_EDIT_ALL_ON, OnBtnEditAllOn)
	ON_BN_CLICKED(IDC_BTN_EDIT_ALL_OFF, OnBtnEditAllOff)
	ON_BN_CLICKED(IDC_BTN_ENV_ALL_OFF, OnBtnEnvAllOff)
	ON_BN_CLICKED(IDC_BTN_ENV_ALL_ON, OnBtnEnvAllOn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageBgFlags message handlers


BOOL CPageBgFlags::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	_recurse = true;
	if (lParam != 0)
	{
		int id = ::GetDlgCtrlID(HWND(lParam));
		// command come from a control

		if (FILTER_EDIT_IDX.find(id) != FILTER_EDIT_IDX.end())
		{
			bool state = static_cast<CButton*>(GetDlgItem(id))->GetCheck() == 1;
			nldebug("Setting filter %u of sound %u to %s", FILTER_EDIT_IDX[id], _Index, state ? "true" : "false");
			// here, we don't use the updateData method coz it's too slow for only one value
			char path[1024];
			sprintf(path, ".SoundType.Sounds[%u].Filter%02u", _Index, FILTER_EDIT_IDX[id]);
			SoundDialog->getSoundPlugin()->getActiveDocument()->setValue(state ? "true" : "false", path);
			SoundDialog->getSoundPlugin()->getActiveDocument()->refreshView();
		}
		else if (FILTER_SIM_IDX.find(id) != FILTER_SIM_IDX.end())
		{
//			bool state = (static_cast<CButton*>(GetDlgItem(id))->GetState() & 0x0003) != 0;
//			nldebug("Setting simul filter %u to %s", FILTER_SIM_IDX[id] , state ? "true" : "false");
			updateData(false);
		}
	}

	_recurse = false;
	
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPageBgFlags::OnBtnEditAllOn() 
{
	for (uint i=0; i<32; ++i)
	{
		static_cast<CButton*>(GetDlgItem(FILTER_EDIT[i]))->SetCheck(TRUE);
	}
	updateData(true);
}

void CPageBgFlags::OnBtnEditAllOff() 
{
	for (uint i=0; i<32; ++i)
	{
		static_cast<CButton*>(GetDlgItem(FILTER_EDIT[i]))->SetCheck(FALSE);
	}
	updateData(true);
}

void CPageBgFlags::OnBtnEnvAllOn() 
{
	for (uint i=0; i<32; ++i)
	{
		static_cast<CButton*>(GetDlgItem(FILTER_SIM[i]))->SetCheck(TRUE);
	}
	updateData(false);
}

void CPageBgFlags::OnBtnEnvAllOff() 
{
	for (uint i=0; i<32; ++i)
	{
		static_cast<CButton*>(GetDlgItem(FILTER_SIM[i]))->SetCheck(FALSE);
	}
	updateData(false);
}

void CPageBgFlags::onDocChanged()
{
	// the document have been modified, update the dialog
	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();

	bool valid = false;

	if (_recurse)
		return;

	if (pdoc != NULL)
	{
		string type, dfnName;
		NLGEORGES::UFormElm *psoundType;

		pdoc->getForm()->getRootNode().getNodeByName(&psoundType, ".SoundType");

		if (psoundType != NULL)
		{
			psoundType->getDfnName(dfnName);
			if (dfnName == "background_sound.dfn")
			{
				// set the filter flags according to active node
				bool val;
				string	activeNode;
				pdoc->getActiveNode(activeNode);
				if (activeNode.find(".SoundType.Sounds") != string::npos)
				{
					// extract the root path on the sheet.
					string::size_type p = activeNode.find(".SoundType.Sounds");
					while (p < activeNode.size() && activeNode[p] != '[')
						p++;
					if (p < activeNode.size())
					{
						// skip the open [
						p++;
						string index;

						while (isdigit(activeNode[p]))
							index += char(activeNode[p++]);

						// extract the index value
						_Index = atoi(index.c_str());

						// skip the closing ]
						p++;

						string sheetRoot = activeNode.substr(0, p);
						string soundName;

//						pdoc->getForm()->getRootNode().getValueByName(soundName, (sheetRoot+".Sound").c_str());
//						_SoundFilterPane.SetWindowText((string("Edit Sound Filters ")+soundName).c_str());

						for (uint i=0; i<32; ++i)
						{
							char tmp[128];
							sprintf(tmp, "%02u", i);

							pdoc->getForm()->getRootNode().getValueByName(val, (sheetRoot+".Filter"+tmp).c_str());
							GetDlgItem(FILTER_EDIT[i])->EnableWindow(TRUE);
							static_cast<CButton*>(GetDlgItem(FILTER_EDIT[i]))->SetCheck(val ? 1 : 0);
						}

						// set the name of the sub sound
						string s;
						pdoc->getForm()->getRootNode().getValueByName(s, (sheetRoot+".Sound").c_str());

						_SubSoundName = s.c_str();
						UpdateData(FALSE);
						GetDlgItem(IDC_SUB_SOUND_NAME)->EnableWindow(TRUE);

						valid = true;
					}
				}
			}
		}
	}

	if (!valid)
	{
		for (uint i=0; i<32; ++i)
		{
			GetDlgItem(FILTER_EDIT[i])->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_SUB_SOUND_NAME)->EnableWindow(FALSE);

		_Index = -1;
	}
}

void CPageBgFlags::updateData(bool updateEditFilter)
{
	_recurse = true;
	// update the filter flags and simulation status
	if (_Index != -1 && updateEditFilter)
	{
		// filter edition is valid, update them
		for (uint i=0; i<32; ++i)
		{
			bool state = static_cast<CButton*>(GetDlgItem(FILTER_EDIT[i]))->GetCheck() == 1;

			char path[1024];
			sprintf(path, ".SoundType.Sounds[%u].Filter%02u", _Index, i);
			SoundDialog->getSoundPlugin()->getActiveDocument()->setValue(state ? "true" : "false", path);
		}
		SoundDialog->getSoundPlugin()->getActiveDocument()->refreshView();
	}

	// simulation filters
	NLSOUND::UAudioMixer::TBackgroundFlags flags;
	for (uint i=0; i<32; ++i)
	{
		flags.Flags[i] = static_cast<CButton*>(GetDlgItem(FILTER_SIM[i]))->GetCheck() == 1;
	}
	SoundDialog->getSoundPlugin()->getMixer()->setBackgroundFlags(flags);

	_recurse = false;
}



BOOL CPageBgFlags::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// update filter labels and state
	const NLSOUND::UAudioMixer::TBackgroundFlags &flag = SoundDialog->getSoundPlugin()->getMixer()->getBackgroundFlags();
	for (uint i=0; i<32; ++i)
	{
		GetDlgItem(FILTER_EDIT_NAME[i])->SetWindowText(SoundDialog->EnvNames[i].Name.c_str());
		GetDlgItem(FILTER_SIM_NAME[i])->SetWindowText(SoundDialog->EnvNames[i].ShortName.c_str());

		static_cast<CButton*>(GetDlgItem(FILTER_SIM[i]))->SetCheck(flag.Flags[i] ? 1 : 0);
	}

	// force an update of the dialog state
	onDocChanged();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
