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

// DailogFlags.cpp : implementation file
//

#include "stdafx.h"
#include "DialogFlags.h"
#include "plugin.h"
#include "list_box_color.h"
#include "entity_display_info.h"
#include "../../../../common/src/game_share/mode_and_behaviour.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;




// registry tag to save info about entity display
const std::string ENTITY_DISPLAY_MODE_REG_ID[EntityDisplayModeCount] = 
{
	"entity_type",
	"entity_alive",
	"entity_hit_points",
	"entity_mode"
};

const bool ENTITY_DISPLAY_VISIBLE_CHECK_BOX_FLAG[EntityDisplayModeCount] = 
{
	true,  // entity type
	false, // alive
	false, // hit points
	true   // mode
};

const bool ENTITY_DISPLAY_ICONS[EntityDisplayModeCount] =
{
	true,  // entity type
	false, // alive
	false, // hit points
	true   // mode
};


static void initDisplayInfo(const CEntityDisplayInfo *data, uint count, TEntityDisplayInfoVect &dest)
{
	dest.resize(count);
	std::copy(data, data + count, dest.begin());
}

/////////////////////////////////////////////////////////////////////////////
// CDialogFlags dialog

CDialogFlags::CDialogFlags(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogFlags::IDD, pParent)
{
	// init default display infos
	{
		CEntityDisplayInfo di[] = 
		{
			CEntityDisplayInfo(RYZOMID::building,	   "Building",	         NLMISC::CRGBA(192, 0, 0)),
			CEntityDisplayInfo(RYZOMID::creature,	   "Creature",           NLMISC::CRGBA(255, 127, 0), 2, 0),
			CEntityDisplayInfo(RYZOMID::deposit,       "Deposit",	         NLMISC::CRGBA(0, 255, 0)),
			CEntityDisplayInfo(RYZOMID::flora,		   "Flora",              NLMISC::CRGBA(255, 127, 0)),
			CEntityDisplayInfo(RYZOMID::forageSource,  "Forage Source",	     NLMISC::CRGBA(0, 255, 0)),		
			CEntityDisplayInfo(RYZOMID::mount,		   "Mount",	             NLMISC::CRGBA(127, 63, 0), 3, 0),
			CEntityDisplayInfo(RYZOMID::npc,		   "NPC", 	             NLMISC::CRGBA(255, 0, 0), 1, 0),	
			CEntityDisplayInfo(RYZOMID::object,		   "Object",	         NLMISC::CRGBA(192, 255, 255)),
			CEntityDisplayInfo(RYZOMID::pack_animal,   "Pack Animal",        NLMISC::CRGBA(127, 63, 0), 3, 0),
			CEntityDisplayInfo(RYZOMID::player,		   "Player",	         NLMISC::CRGBA(127, 127, 255), 0, 0),
			CEntityDisplayInfo(RYZOMID::fx_entity,     "FX Entity",			 NLMISC::CRGBA(0, 255, 0)),
			CEntityDisplayInfo(RYZOMID::unknown,       "Unknown",	         NLMISC::CRGBA(127, 127, 127)),
		};
		initDisplayInfo(di, sizeofarray(di), _EntityDisplayInfo[EntityType]);
	}
	//
	{
		CEntityDisplayInfo di[] = 
		{
			CEntityDisplayInfo(0,	   "Dead",	         NLMISC::CRGBA(255, 0, 0)),
			CEntityDisplayInfo(1,	   "Weak",	         NLMISC::CRGBA(255, 255, 0)),
			CEntityDisplayInfo(2,	   "Full shape",     NLMISC::CRGBA(0, 255, 0)),
			CEntityDisplayInfo(3,	   "No Hit Points",  NLMISC::CRGBA(127, 127, 127)),
		};
		initDisplayInfo(di, sizeofarray(di), _EntityDisplayInfo[EntityHitPoints]);
	}
	//
	{
		CEntityDisplayInfo di[] = 
		{
			CEntityDisplayInfo(0,	   "Dead",	         NLMISC::CRGBA(255, 0, 0)),
			CEntityDisplayInfo(1,	   "Alive",	         NLMISC::CRGBA(0, 0, 255)),		
			CEntityDisplayInfo(2,	   "No Hit Points",  NLMISC::CRGBA(127, 127, 127)),
		};
		initDisplayInfo(di, sizeofarray(di), _EntityDisplayInfo[EntityAlive]);
	}
	//
	{
		CEntityDisplayInfo di[] = 
		{
			CEntityDisplayInfo(MBEHAV::UNKNOWN_MODE,  "Unknown", NLMISC::CRGBA(127, 127, 127)),
			CEntityDisplayInfo(MBEHAV::NORMAL,        "Normal", NLMISC::CRGBA(255, 255, 255)),
			CEntityDisplayInfo(MBEHAV::COMBAT_FLOAT,  "Combat float",  NLMISC::CRGBA(255, 0, 0), 0, 1),
			CEntityDisplayInfo(MBEHAV::COMBAT,        "Combat",  NLMISC::CRGBA(255, 0, 0), 0, 1),
			CEntityDisplayInfo(MBEHAV::SWIM,          "Swim",  NLMISC::CRGBA(0, 0, 255)),
			CEntityDisplayInfo(MBEHAV::SIT,           "Sit",  NLMISC::CRGBA(0, 255, 255)),
			CEntityDisplayInfo(MBEHAV::MOUNT_NORMAL,  "Mount Normal",  NLMISC::CRGBA(192, 128, 0)),
			CEntityDisplayInfo(MBEHAV::MOUNT_SWIM,    "Mount Swim",  NLMISC::CRGBA(0, 0, 255)),
			CEntityDisplayInfo(MBEHAV::EAT,           "Eat",  NLMISC::CRGBA(0, 255, 0)),
			CEntityDisplayInfo(MBEHAV::ALERT,         "Alert",  NLMISC::CRGBA(255, 127, 0)),
			CEntityDisplayInfo(MBEHAV::HUNGRY,        "Hungry",  NLMISC::CRGBA(255, 255, 0)),
			CEntityDisplayInfo(MBEHAV::DEATH,         "Death",  NLMISC::CRGBA(0, 0, 0)),
			CEntityDisplayInfo(MBEHAV::SWIM_DEATH,    "SwimDeath",  NLMISC::CRGBA(0, 0, 0))
		};
		initDisplayInfo(di, sizeofarray(di), _EntityDisplayInfo[EntityMode]);
	}		
}

//******************************************************************************************************
void CDialogFlags::init(CPlugin *plugin)
{
	_Plugin = plugin;
}

//******************************************************************************************************
void CDialogFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogFlags)	
	DDX_Control(pDX, IDC_DETAILS_DISTANCE, DetailsDistanceCtrl);
	DDX_Control(pDX, IDC_COLOR_LIST, m_EntityColorList);
	DDX_Control(pDX, IDC_DOWNLOAD, DownloadCtrl);
	DDX_Control(pDX, IDC_CONNECT, ConnectCtrl);
	DDX_Text(pDX, IDC_ENTITES, Entites);
	DDX_Text(pDX, IDC_RECEIVED, Received);
	DDX_Text(pDX, IDC_SENT, Sent);
	DDX_Text(pDX, IDC_STATE, State);
	DDX_Slider(pDX, IDC_DOWNLOAD, Download);
	DDX_Text(pDX, IDC_DLDVALUE, DownloadValue);
	DDX_Slider(pDX, IDC_DETAILS_DISTANCE, DetailsDistance);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogFlags, CDialog)
	//{{AFX_MSG_MAP(CDialogFlags)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_DISPLAY_MODE_COMBO, OnSelchangeDisplayModeCombo)
	ON_LBN_SELCHANGE(IDC_COLOR_LIST, OnSelChangeColorList)
	ON_BN_CLICKED(IDC_BROWSE_COLOR, OnBrowseColor)
	ON_CLBN_CHKCHANGE(IDC_COLOR_LIST, OnCheckVisible)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_UNSELECT_ALL, OnUnselectAll)
	ON_BN_CLICKED(IDC_HP_DOWN, OnHpDown)
	ON_BN_CLICKED(IDC_CLOSE_UP_SHOW_TYPE, OnCloseUpShowType)
	ON_BN_CLICKED(IDC_CLOSE_UP_SHOW_MODE, OnCloseUpShowMode)
	ON_BN_CLICKED(IDC_CLOSE_UP_SHOW_HP, OnCloseUpShowHp)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_DETAILS_DISTANCE, OnReleasedcaptureDetailsDistance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//******************************************************************************************************
int CDialogFlags::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	return 0;
}

//******************************************************************************************************
void CDialogFlags::OnDestroy() 
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
}

//******************************************************************************************************
BOOL CDialogFlags::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialog::OnCommand(wParam, lParam);
}

//******************************************************************************************************
void CDialogFlags::OnConnect() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_Plugin->connectDisconnect();	
}

//******************************************************************************************************
BOOL CDialogFlags::OnInitDialog() 
{
	CDialog::OnInitDialog();

	HRSRC rsc = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ENTITY_ICONS_TGA), "TGA");
	if (rsc != NULL)
	{
		NLMISC::CBitmap bm;
		if (_Plugin->getPluginAccess()->buildNLBitmapFromTGARsc(rsc, AfxGetInstanceHandle(), bm))
		{
			m_EntityColorList.setIconBitmap(bm, ENTITY_ICON_SIZE);
		}
	}	
	
	
	DownloadCtrl.SetRange(512, 32768);
	DetailsDistanceCtrl.SetRange(0, 100);

	for(uint k = 0; k < EntityDisplayModeCount; ++k)
	{
		loadEntityDisplayInfoToRegistry(_EntityDisplayInfo[k], ENTITY_DISPLAY_MODE_REG_ID[k]);
	}

	setCurrentEntityDisplayMode(EntityType);

	updateCtrlGrayedState();

	((CComboBox *) GetDlgItem(IDC_DISPLAY_MODE_COMBO))->SetCurSel(0);

	((CButton *) GetDlgItem(IDC_HP_DOWN))->SetCheck(1);
	((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_TYPE))->SetCheck(1);
	((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_MODE))->SetCheck(1);
	
	((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_HP))->SetCheck(1);
	
	::CRect shardListRect;
	GetDlgItem(IDC_SHARD_LIST_PLACEMENT)->GetWindowRect(shardListRect);
	ScreenToClient(&shardListRect);
	ShardCtrl.create(WS_CHILD|WS_TABSTOP, shardListRect, this, 0, REGKEY_BASE_PATH "\\shard_list", 10);
	ShardCtrl.ShowWindow (SW_SHOW);	
	CFont* font = GetFont ();
	ShardCtrl.SetFont(font);
	ShardCtrl.refreshStrings();
	//ShardCtrl.setFocus();
	if (ShardCtrl.getCount() != 0)
	{		
		ShardCtrl.setCurSel(0);		
	}	

	CConfigFile ConfigFile;	
	ConfigFile.load ("world_editor_plugin.cfg");
	CConfigFile::CVar *var = ConfigFile.getVarPtr("MOSHost");
	if (var)
	{
		ShardCtrl.pushString(var->asString().c_str());
		ShardCtrl.setCurSel(var->asString());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//******************************************************************************************************
void CDialogFlags::setCurrentEntityDisplayMode(TEntityDisplayMode edm)
{
	nlassert(edm < EntityDisplayModeCount);
	const TEntityDisplayInfoVect &currList = _EntityDisplayInfo[edm];
	const std::string &regId = ENTITY_DISPLAY_MODE_REG_ID[edm];	
	m_EntityColorList.ResetContent();
	m_EntityColorList.enableCheckBoxes(ENTITY_DISPLAY_VISIBLE_CHECK_BOX_FLAG[edm]);
	m_EntityColorList.enableIcons(ENTITY_DISPLAY_VISIBLE_CHECK_BOX_FLAG[edm]);
	for(uint k = 0; k < currList.size(); ++k)
	{
		m_EntityColorList.AddString(currList[k].Name);
		CRGBA col = currList[k].Color;
		m_EntityColorList.setColor(k, RGB(col.R, col.G, col.B));
		m_EntityColorList.setIcon(k, currList[k].Icon.X, currList[k].Icon.Y);
		m_EntityColorList.SetCheck(k, currList[k].Visible);
	}
	_CurrentEntityDisplayMode = edm;	
	updateCtrlGrayedState();
}



#define REGKEY_ENTITY_DISPLAY_INFO REGKEY_BASE_PATH "\\entity_display_info\\"

//******************************************************************************************************
void CDialogFlags::loadEntityDisplayInfoToRegistry(TEntityDisplayInfoVect &infos, const std::string &regId)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, (REGKEY_ENTITY_DISPLAY_INFO + regId).c_str(), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD index  = 0;
		for(;;)
		{
			char valueStr[256] = { 0 };
			BYTE dataStr[256] = { 0 };
			DWORD valueSize = sizeofarray(valueStr);
			DWORD dataSize = sizeofarray(dataStr);
			LONG result = RegEnumValue(hKey, index, valueStr, &valueSize, NULL, NULL, dataStr, &dataSize);
			if (result != ERROR_SUCCESS) break;
			uint value = (uint) atoi(valueStr);
			for(uint k = 0; k < infos.size(); ++k)
			{
				if (infos[k].Value == value)
				{
					int r, g, b, visible;
					if (sscanf((const char *) dataStr, "%d, %d, %d, visible = %d", &r, &g, &b, &visible) == 4)
					{
						infos[k].Color = CRGBA(r, g, b);
						infos[k].Visible = visible != 0;
					}
					else
					{
						nlwarning("Can't retrieve color in registry for type %s", infos[k].Name);
					}
				}
			}
			++ index;
		}
	}
}

//******************************************************************************************************
void CDialogFlags::saveEntityDisplayInfoToRegistry(const TEntityDisplayInfoVect &infos, const std::string &regId)
{
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, (REGKEY_ENTITY_DISPLAY_INFO + regId).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
	{
		for(uint k = 0; k < infos.size(); ++k)
		{
			char colorStr[128];
			CRGBA color = infos[k].Color;
			sprintf(colorStr, "%d, %d, %d, visible = %d", (int) color.R, (int) color.G, (int) color.B, (int) (infos[k].Visible ? 1 : 0));
			LONG result = RegSetValueEx(hKey, toString((int) infos[k].Value).c_str(), 0, REG_SZ, (const BYTE *) colorStr, strlen(colorStr));
			if (result  != ERROR_SUCCESS)
			{
				nlwarning("Couldn't write registry key for entity % color", infos[k].Name);
			}
		}
	}
}

//******************************************************************************************************
void CDialogFlags::OnClose()
{
	_Plugin->closePlugin();
//	CDialog::OnClose();
}

//******************************************************************************************************
void CDialogFlags::OnSelchangeDisplayModeCombo() 
{
	CComboBox *cb = (CComboBox *) GetDlgItem(IDC_DISPLAY_MODE_COMBO);		
	TEntityDisplayMode displayMode = (TEntityDisplayMode) cb->GetCurSel();
	if (displayMode == LB_ERR) return;
	nlassert(displayMode < EntityDisplayModeCount);
	setCurrentEntityDisplayMode(displayMode);	
	updateCtrlGrayedState();
	_Plugin->setEntityDisplayMode((TEntityDisplayMode) displayMode);
}



//******************************************************************************************************
void CDialogFlags::OnSelChangeColorList() 
{
	updateCtrlGrayedState();
}

//******************************************************************************************************
void CDialogFlags::updateCtrlGrayedState()
{	
	GetDlgItem(IDC_BROWSE_COLOR)->EnableWindow(m_EntityColorList.GetCurSel() != LB_ERR ? TRUE : FALSE);	
	BOOL hasCheckBoxes = ENTITY_DISPLAY_VISIBLE_CHECK_BOX_FLAG[_CurrentEntityDisplayMode];
	GetDlgItem(IDC_SELECT_ALL)->EnableWindow(hasCheckBoxes);
	GetDlgItem(IDC_UNSELECT_ALL)->EnableWindow(hasCheckBoxes);
}

//******************************************************************************************************
void CDialogFlags::saveCurrentEntityDisplayInfo()
{
	saveEntityDisplayInfoToRegistry(_EntityDisplayInfo[_CurrentEntityDisplayMode], ENTITY_DISPLAY_MODE_REG_ID[_CurrentEntityDisplayMode]);
}


//******************************************************************************************************
void CDialogFlags::OnBrowseColor() 
{	
	int currSel = m_EntityColorList.GetCurSel();
	if (currSel == LB_ERR) return;
	CColorDialog cd;	
	if (cd.DoModal() == IDOK)
	{
		COLORREF col = cd.GetColor();
		m_EntityColorList.setColor(currSel, col);
		m_EntityColorList.Invalidate();
		nlassert(currSel < (int) _EntityDisplayInfo[_CurrentEntityDisplayMode].size());
		CRGBA nelCol(GetRValue(col), GetGValue(col), GetBValue(col));
		_EntityDisplayInfo[_CurrentEntityDisplayMode][currSel].Color = nelCol;
		saveCurrentEntityDisplayInfo();
		_Plugin->updateDisplay();
	}
}

// ***************************************************************************
const TEntityDisplayInfoVect &CDialogFlags::getEntityDisplayInfos(TEntityDisplayMode mode) const
{
	nlassert(mode < EntityDisplayModeCount);
	return _EntityDisplayInfo[mode];	
}

// ***************************************************************************
void CDialogFlags::OnCheckVisible()
{	
	int currSel = m_EntityColorList.GetCurSel();
	if (currSel == LB_ERR) return;
	nlassert(currSel < (int) _EntityDisplayInfo[_CurrentEntityDisplayMode].size());
	_EntityDisplayInfo[_CurrentEntityDisplayMode][currSel].Visible = m_EntityColorList.GetCheck(currSel) != 0;
	saveCurrentEntityDisplayInfo();
	_Plugin->updateDisplay();
}

// ***************************************************************************
void CDialogFlags::OnSelectAll() 
{
	setAllChecks(true);	
}

// ***************************************************************************
void CDialogFlags::OnUnselectAll() 
{
	setAllChecks(false);	
}

// ***************************************************************************
void CDialogFlags::setAllChecks(bool checked)
{	
	TEntityDisplayInfoVect &di = _EntityDisplayInfo[_CurrentEntityDisplayMode];
	for(uint k = 0; k < di.size(); ++k)
	{
		di[k].Visible = checked;
		m_EntityColorList.SetCheck(k, checked);
	}
	saveCurrentEntityDisplayInfo();
	_Plugin->updateDisplay();
}

// ***************************************************************************
void CDialogFlags::OnHpDown() 
{
	bool on = ((CButton *) GetDlgItem(IDC_HP_DOWN))->GetCheck() != 0;
	_Plugin->setDisplayHitsFlag(on);
}

// ***************************************************************************
void CDialogFlags::OnCloseUpShowType() 
{
	bool on = ((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_TYPE))->GetCheck() != 0;
	_Plugin->setCloseUpFlag(CPlugin::CloseUpEntityType, on);
}

// ***************************************************************************
void CDialogFlags::OnCloseUpShowMode() 
{
	bool on = ((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_MODE))->GetCheck() != 0;
	_Plugin->setCloseUpFlag(CPlugin::CloseUpEntityMode, on);
}

// ***************************************************************************
void CDialogFlags::OnCloseUpShowHp() 
{	
	bool on = ((CButton *) GetDlgItem(IDC_CLOSE_UP_SHOW_HP))->GetCheck() != 0;
	_Plugin->setCloseUpFlag(CPlugin::CloseUpEntityHP, on);
}


// ***************************************************************************
void CDialogFlags::OnReleasedcaptureDetailsDistance(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();
	float lambda = DetailsDistance / 100.f;
	_Plugin->setCloseUpDisplayDistance((1.f - lambda) * 10.f  + lambda * (1.f / 160.f));
	*pResult = 0;
}
