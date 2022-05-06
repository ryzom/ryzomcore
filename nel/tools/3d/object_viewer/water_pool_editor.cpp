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

// water_pool_editor.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "water_pool_editor.h"
#include "editable_range.h"
#include "nel/3d/water_pool_manager.h"
#include "choose_pool_id.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"





/////////////////////////////////////////////////////////////////////////////
// CWaterPoolEditor dialog


CWaterPoolEditor::CWaterPoolEditor(NL3D::CWaterPoolManager *wpm, CWnd* pParent /*= NULL*/)
	: CDialog(CWaterPoolEditor::IDD, pParent), _DampingDlg(NULL),
	  _FilterWeightDlg(NULL), _WaterUnitSizeDlg(NULL), _Wpm(wpm)
{
	//{{AFX_DATA_INIT(CWaterPoolEditor)
	m_AutomaticWavesGeneration = FALSE;
	m_BordersOnly = FALSE;
	m_MapSize = -1;
	//}}AFX_DATA_INIT
}


CWaterPoolEditor::~CWaterPoolEditor()
{
#define REMOVE_WINDOW(wnd) if (wnd) { (wnd)->DestroyWindow(); delete (wnd); }
REMOVE_WINDOW(_DampingDlg);
REMOVE_WINDOW(_FilterWeightDlg);
REMOVE_WINDOW(_WaterUnitSizeDlg);
REMOVE_WINDOW(_ImpulsionStrenghtDlg);
REMOVE_WINDOW(_WavePeriodDlg);
REMOVE_WINDOW(_WaveImpulsionRadiusDlg);
REMOVE_WINDOW(_PropagationTimeDlg);
}


void CWaterPoolEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaterPoolEditor)
	DDX_Control(pDX, IDC_POOL_LIST, m_PoolList);
	DDX_Check(pDX, IDC_AUTOMATIC_WAVES_GENERATION, m_AutomaticWavesGeneration);
	DDX_Check(pDX, IDC_BORDERS_ONLY, m_BordersOnly);
	DDX_CBIndex(pDX, IDC_MAP_SIZE, m_MapSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaterPoolEditor, CDialog)
	//{{AFX_MSG_MAP(CWaterPoolEditor)
	ON_LBN_SELCHANGE(IDC_POOL_LIST, OnSelchangePoolList)
	ON_BN_CLICKED(IDC_AUTOMATIC_WAVES_GENERATION, OnAutomaticWavesGeneration)
	ON_BN_CLICKED(IDC_BORDERS_ONLY, OnBordersOnly)
	ON_BN_CLICKED(IDC_ADD_POOL, OnAddPool)
	ON_BN_CLICKED(IDC_DELETE_POOL, OnDeletePool)
	ON_CBN_SELCHANGE(IDC_MAP_SIZE, OnSelchangeMapSize)
	ON_BN_CLICKED(IDC_LOAD_POOL, OnLoadPool)
	ON_BN_CLICKED(IDC_SAVE_POOL, OnSavePool)
	ON_BN_CLICKED(IDC_RENAME_POOL, OnRenamePool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaterPoolEditor message handlers

BOOL CWaterPoolEditor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT r;
	

	fillPoolList();

	// damping
	GetDlgItem(IDC_DAMPING)->GetWindowRect(&r);
	ScreenToClient(&r);
	_DampingDlg = new CEditableRangeFloat("WATER_DAMPING", NULL,0.9f ,0.999f);
	_DampingDlg->setWrapper(&_DampingWrapper);
	_DampingDlg->init(r.left, r.top, this);
	_DampingDlg->enableLowerBound(0, false);
	_DampingDlg->enableUpperBound(1, true);

	

	// filter weight
	GetDlgItem(IDC_FILTER_WEIGHT)->GetWindowRect(&r);
	ScreenToClient(&r);
	_FilterWeightDlg = new CEditableRangeFloat("FILTER_WEIGHT", NULL, 4, 10);
	_FilterWeightDlg->setWrapper(&_FilterWeightWrapper);
	_FilterWeightDlg->init(r.left, r.top, this);
	_FilterWeightDlg->enableLowerBound(0, false);

	// water unit size
	GetDlgItem(IDC_WATER_UNIT_SIZE)->GetWindowRect(&r);
	ScreenToClient(&r);
	_WaterUnitSizeDlg = new CEditableRangeFloat("WATER_UNIT_SIZE", NULL, 0.1f, 1);
	_WaterUnitSizeDlg->setWrapper(&_WaterUnitSizeWrapper);
	_WaterUnitSizeDlg->init(r.left, r.top, this);	
	_WaterUnitSizeDlg->enableLowerBound(0, true);

	// wave impulsion strenght
	GetDlgItem(IDC_IMPULSION_STRENGHT)->GetWindowRect(&r);
	ScreenToClient(&r);
	_ImpulsionStrenghtDlg = new CEditableRangeFloat("WAVE_IMPULSION_STRENGHT", NULL, 0.2f, 3.0f);
	_ImpulsionStrenghtDlg->setWrapper(&_ImpulsionStrenghtWrapper);
	_ImpulsionStrenghtDlg->init(r.left, r.top, this);	

	// wave period
	GetDlgItem(IDC_PERIOD)->GetWindowRect(&r);
	ScreenToClient(&r);
	_WavePeriodDlg = new CEditableRangeFloat("WAVE_PERIOD", NULL, 0, 5);
	_WavePeriodDlg->setWrapper(&_WavePeriodWrapper);
	_WavePeriodDlg->init(r.left, r.top, this);
	_WavePeriodDlg->enableLowerBound(0, false);

	// propagation time
	GetDlgItem(IDC_PROPAGATION_TIME)->GetWindowRect(&r);
	ScreenToClient(&r);
	_PropagationTimeDlg = new CEditableRangeFloat("PROPAGATION_TIME", NULL, 0, 2);
	_PropagationTimeDlg->setWrapper(&_PropagationTimeWrapper);
	_PropagationTimeDlg->init(r.left, r.top, this);
	_PropagationTimeDlg->enableLowerBound(0, false);

	// wave impulsion radius
	GetDlgItem(IDC_IMPULSION_RADIUS)->GetWindowRect(&r);
	ScreenToClient(&r);
	_WaveImpulsionRadiusDlg = new CEditableRangeUInt("WAVE_IMPULSION_RADIUS", NULL, 2, 5);
	_WaveImpulsionRadiusDlg->setWrapper(&_WaveImpulsionRadiusWrapper);
	_WaveImpulsionRadiusDlg->init(r.left, r.top, this);
	_WaveImpulsionRadiusDlg->enableLowerBound(0, true);
	
	updateWaveControls();
	updateWaveParams();
	updateMapSizeCtrl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CWaterPoolEditor::addPool(uint32 ID)
{
	std::string poolId = NLMISC::toString("%d (%s)", ID, _Wpm->getPoolByID(ID).getName().c_str());
	int index = m_PoolList.AddString(nlUtf8ToTStr(poolId));
	nlassert(index != LB_ERR);
	m_PoolList.SetItemData(index, ID);
	return index;

}

//=====================================================================
void CWaterPoolEditor::fillPoolList()
{
	UpdateData();
	m_PoolList.ResetContent();
	uint numPools = _Wpm->getNumPools();
	if (numPools == 0) // we must have least have one pool
	{
		_Wpm->createWaterPool();
		numPools = 1;
	}
	for (uint k = 0; k < numPools; ++k)
	{
		addPool(_Wpm->getPoolID(k));		
	}
	m_PoolList.SetCurSel(0);
	updateWrappers();
	UpdateData(FALSE);
}


//===================================================================
uint32 CWaterPoolEditor::getCurrentPoolID()
{
	UpdateData();
	nlassert(m_PoolList.GetCurSel() != LB_ERR); // must always have something selected
	return (uint32)m_PoolList.GetItemData(m_PoolList.GetCurSel());
}

//===================================================================
NL3D::CWaterHeightMap &CWaterPoolEditor::getCurrentPool()
{	
	return _Wpm->getPoolByID(getCurrentPoolID());
}

//===================================================================
void CWaterPoolEditor::updateWrappers()
{		
	NL3D::CWaterHeightMap *whm  = &getCurrentPool();
	_DampingWrapper.Whm			= whm;
	_FilterWeightWrapper.Whm    = whm;
	_WaterUnitSizeWrapper.Whm   = whm;
	_ImpulsionStrenghtWrapper.Whm   = whm;
	_WavePeriodWrapper.Whm = whm;
	_WaveImpulsionRadiusWrapper.Whm = whm;	
	_PropagationTimeWrapper.Whm = whm;	

}

//===================================================================
void CWaterPoolEditor::OnSelchangePoolList() 
{
	updateWrappers();
	_DampingDlg->update();
	_FilterWeightDlg->update();
	_WaterUnitSizeDlg->update();
	_ImpulsionStrenghtDlg->update();
	_WavePeriodDlg->update();
	_WaveImpulsionRadiusDlg->update();
	updateWaveControls();
	updateWaveParams();
	updateMapSizeCtrl();
}

//===================================================================

void CWaterPoolEditor::OnAutomaticWavesGeneration() 
{
	UpdateData();
	getCurrentPool().enableWaves(m_AutomaticWavesGeneration ? true : false /* VC++ warning */); 
	updateWaveParams();	
}

//===================================================================

void CWaterPoolEditor::updateWaveParams()
{
	bool enabled = getCurrentPool().areWavesEnabled();	
	_ImpulsionStrenghtDlg->EnableWindow(enabled);
	_WavePeriodDlg->EnableWindow(enabled);
	_WaveImpulsionRadiusDlg->EnableWindow(enabled);
	GetDlgItem(IDC_BORDERS_ONLY)->EnableWindow(enabled);
}

//===================================================================

void CWaterPoolEditor::OnBordersOnly() 
{
	UpdateData();;
	NL3D::CWaterHeightMap &whm = getCurrentPool();
	whm.setWaves(whm.getWaveIntensity(), whm.getWavePeriod(), whm.getWaveImpulsionRadius(), m_BordersOnly ? true : false /* VC++ Warning */); 
}

//===================================================================

void CWaterPoolEditor::updateWaveControls()
{
	const NL3D::CWaterHeightMap &whm  = getCurrentPool();
	m_AutomaticWavesGeneration = whm.areWavesEnabled();
	m_BordersOnly  = whm.getBorderWaves();
	UpdateData(FALSE);
}

//===================================================================
void CWaterPoolEditor::updateMapSizeCtrl()
{	
	switch(getCurrentPool().getSize())
	{
		case 16: m_MapSize = 0; break;
		case 32: m_MapSize = 1; break;
		case 64: m_MapSize = 2; break;
		case 128: m_MapSize = 3; break;
		case 256: m_MapSize = 4; break;
		case 512: m_MapSize = 5; break;
		default:
			nlassert(0);
		break;
	}
	UpdateData(FALSE);
}

//===================================================================
void CWaterPoolEditor::OnAddPool() 
{
	CChoosePoolID cpi(false);
	cpi.PoolID = getCurrentPoolID();
	if (cpi.DoModal() == IDOK)
	{
		if (_Wpm->hasPool(cpi.PoolID) )
		{
			MessageBox(_T("Pool already exists"), _T("error"));
		}
		else
		{
			NL3D::CWaterPoolManager::CWaterHeightMapBuild whmb;
			whmb.ID   = cpi.PoolID;
			whmb.Name = cpi.Name;
			_Wpm->createWaterPool(whmb);			
			m_PoolList.SetCurSel(addPool(cpi.PoolID));
			OnSelchangePoolList();
		}
	}
}

//===================================================================
void CWaterPoolEditor::OnDeletePool() 
{
	UpdateData();
	if (m_PoolList.GetCount() == 1)
	{
		MessageBox(_T("Must have at least one water pool"), _T("error"));
	}
	else
	{
		_Wpm->removePool(getCurrentPoolID());
		m_PoolList.DeleteString(m_PoolList.GetCurSel());
		m_PoolList.SetCurSel(0);
		OnSelchangePoolList();
	}
	UpdateData(FALSE);
}

//===================================================================
void CWaterPoolEditor::OnSelchangeMapSize() 
{
	UpdateData();
	static const uint size[] = { 16, 32, 64, 128, 256, 512 };
	const uint tabSize = sizeof(size) / sizeof(uint);
	nlassert(m_MapSize < tabSize);
	getCurrentPool().setSize(size[m_MapSize]);		
}



void CWaterPoolEditor::OnLoadPool() 
{	
	static TCHAR BASED_CODE szFilter[] = _T("NeL Water Pool Files (*.wpf)|*.wpf||");
	CFileDialog fileDlg( TRUE, _T(".wpf"), _T("*.wpf"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{				
		try
		{
			NLMISC::CIXml iXml;
			NLMISC::CIFile iF;
			if (iF.open(NLMISC::tStrToUtf8(fileDlg.GetPathName())))
			{
				if (iXml.init (iF))
				{
					_Wpm->serial(iXml);
					iXml.release();
					iF.close();
					fillPoolList();
				}
				else
				{
					iF.close();
					MessageBox(nlUtf8ToTStr(NLMISC::toString("Unable to init xml stream from file: %s", NLMISC::tStrToUtf8(fileDlg.GetPathName()).c_str())), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
				}
			}
			else
			{
				MessageBox(nlUtf8ToTStr(NLMISC::toString("Unable to open file: %s", NLMISC::tStrToUtf8(fileDlg.GetPathName()).c_str())), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
		catch (const NLMISC::Exception& e)
		{
			MessageBox(nlUtf8ToTStr(e.what()), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
		}
	}	
}
	

void CWaterPoolEditor::OnSavePool() 
{
	static TCHAR BASED_CODE szFilter[] = _T("NeL Water Pool Files (*.wpf)|*.wpf||");
	CFileDialog fileDlg( TRUE, _T(".wpf"), _T("*.wpf"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{				
		try
		{
			NLMISC::COXml oXml;
			NLMISC::COFile oF;
			if (oF.open(NLMISC::tStrToUtf8(fileDlg.GetPathName())))
			{
				if (oXml.init (&oF))
				{
					_Wpm->serial(oXml);
					oXml.flush();
					oF.close();
				}
				else
				{
					oF.close();
					MessageBox(nlUtf8ToTStr(NLMISC::toString("Unable to init xml stream from file: %s", NLMISC::tStrToUtf8(fileDlg.GetPathName()).c_str())), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
				}
			}
			else
			{
				MessageBox(nlUtf8ToTStr(NLMISC::toString("Unable to open file: %s", NLMISC::tStrToUtf8(fileDlg.GetPathName()).c_str())), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
		catch (const NLMISC::Exception& e)
		{
			MessageBox(nlUtf8ToTStr(e.what()), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
		}
	}
	
}

void CWaterPoolEditor::OnRenamePool() 
{
	CChoosePoolID cpi(true);
	cpi.PoolID = getCurrentPoolID();
	cpi.Name   = getCurrentPool().getName();
	if (cpi.DoModal() == IDOK)
	{
		getCurrentPool().setName(cpi.Name);
		fillPoolList();		
	}	
}
