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

#include "object_viewer.h"
#include "main_frame.h"
#include "snapshot_tool_dlg.h"
#include "choose_name.h"
#include "object_viewer.h"
//
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include "nel/misc/file.h"
//
#include "nel/3d/u_scene.h"
#include "nel/3d/u_driver.h"
//
#include "nel/3d/driver.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/camera.h"
#include "nel/3d/texture_multi_file.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/particle_system_model.h"




#define NEL_OV_SNAPSHOT_TOOL_REGKEY _T("Software\\Nevrax\\nel\\object_viewer\\snapshot_dlg")


using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CSnapshotToolDlg dialog

//****************************************************************************************
CSnapshotToolDlg::CSnapshotToolDlg(CObjectViewer *ov, CWnd* pParent /*=NULL*/)
	: CDialog(CSnapshotToolDlg::IDD, pParent)
{
	nlassert(ov);
	_ObjectViewer = ov;
	//{{AFX_DATA_INIT(CSnapshotToolDlg)
	m_OutputPath = _T("");
	m_InputPath = _T("");
	m_RecurseSubFolder = FALSE;
	m_OutputHeight = 100;		
	m_OutputWidth = 160;	
	m_Format = -1;
	m_OutputPathOption = -1;
	m_DumpTextureSets = FALSE;
	m_ViewBack = FALSE;
	m_ViewBottom = FALSE;
	m_ViewFront = FALSE;
	m_ViewLeft = FALSE;
	m_ViewRight = FALSE;
	m_ViewTop = FALSE;
	m_PostFixViewName = FALSE;
	//}}AFX_DATA_INIT
}

//****************************************************************************************
CSnapshotToolDlg::~CSnapshotToolDlg()
{	
}

//****************************************************************************************
void CSnapshotToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSnapshotToolDlg)
	DDX_Control(pDX, IDC_LOG, m_Log);
	DDX_Control(pDX, IDC_FILTERS, m_Filters);
	DDX_Text(pDX, IDC_OUTPUT_PATH, m_OutputPath);
	DDX_Text(pDX, IDC_INPUT_PATH, m_InputPath);
	DDX_Check(pDX, IDC_RECURSE_SUBFOLDER, m_RecurseSubFolder);	
	DDX_Text(pDX, IDC_HEIGHT, m_OutputHeight);
	DDV_MinMaxUInt(pDX, m_OutputHeight, 1, 1024);
	DDX_Text(pDX, IDC_WIDTH, m_OutputWidth);
	DDV_MinMaxUInt(pDX, m_OutputWidth, 1, 1024);	
	DDX_CBIndex(pDX, IDC_FORMAT, m_Format);
	DDX_CBIndex(pDX, IDC_OUTPUTPATH_OPTION, m_OutputPathOption);
	DDX_Check(pDX, IDC_DUMP_TEXTURE_SETS, m_DumpTextureSets);
	DDX_Check(pDX, IDC_VIEW_BACK, m_ViewBack);
	DDX_Check(pDX, IDC_VIEW_BOTTOM, m_ViewBottom);
	DDX_Check(pDX, IDC_VIEW_FRONT, m_ViewFront);
	DDX_Check(pDX, IDC_VIEW_LEFT, m_ViewLeft);
	DDX_Check(pDX, IDC_VIEW_RIGHT, m_ViewRight);
	DDX_Check(pDX, IDC_VIEW_TOP, m_ViewTop);
	DDX_Check(pDX, IDC_POSTFIX_VIEWNAME, m_PostFixViewName);
	//}}AFX_DATA_MAP
}

//****************************************************************************************
void CSnapshotToolDlg::stringFromRegistry(HKEY hKey, const TCHAR *name, CString &dest, const CString &defaultStr)
{	
	DWORD type;
	DWORD size;
	LONG result = RegQueryValueEx(hKey, name, NULL, &type, NULL, &size);

	if (type != REG_SZ || result != ERROR_SUCCESS || size == 0)
	{
		dest = defaultStr;
		return;
	}

	std::vector<TCHAR> tmpDest(size);
	result = RegQueryValueEx(hKey, name, NULL, &type, (BYTE*)&tmpDest[0], &size);

	if (result != ERROR_SUCCESS)
	{
		dest = defaultStr;
		return;
	}

	dest = &tmpDest[0];
}


//****************************************************************************************
template <class T, class U> void integralTypeFromRegistry(HKEY hKey, const TCHAR *name, T &dest, const U &defaultValue)
{	
	if (hKey == 0)
	{
		dest = defaultValue;
		return;
	}
	DWORD type;
	DWORD size;
	LONG result = RegQueryValueEx(hKey, name, NULL, &type, NULL, &size);

	if (type != REG_DWORD || result != ERROR_SUCCESS || size == 0)
	{
		dest = (T) defaultValue;
		return;
	}		

	DWORD value;
	result = RegQueryValueEx(hKey, name, NULL, &type, LPBYTE(&value), &size);

	if (result != ERROR_SUCCESS)
	{
		dest = defaultValue;
		return;
	}

	dest = (T) value;
}


//****************************************************************************************
void CSnapshotToolDlg::fromRegistry()
{
	HKEY hKey = 0;
	RegOpenKeyEx(HKEY_CURRENT_USER, NEL_OV_SNAPSHOT_TOOL_REGKEY, 0, KEY_READ, &hKey);					
	stringFromRegistry(hKey, _T("InputPath"), m_InputPath, "");
	stringFromRegistry(hKey, _T("OutputPath"), m_OutputPath, "");

	CString filters;
	stringFromRegistry(hKey, _T("Filters"), filters, "*.shape");

	std::vector<std::string> filterList;
	NLMISC::splitString(tStrToUtf8(filters), ",", filterList);

	m_Filters.ResetContent();

	for (uint k = 0; k < filterList.size(); ++k)
	{
		m_Filters.AddString(nlUtf8ToTStr(filterList[k]));
	}	
	
	integralTypeFromRegistry(hKey, _T("RecurseSubFolder"), (int &) m_RecurseSubFolder, FALSE);
	integralTypeFromRegistry(hKey, _T("DumpTextureSets"), (int &) m_DumpTextureSets, TRUE);
	integralTypeFromRegistry(hKey, _T("PostFixViewName"), (int &) m_PostFixViewName, TRUE);
	integralTypeFromRegistry(hKey, _T("ViewBack"), (int &) m_ViewBack, FALSE);
	integralTypeFromRegistry(hKey, _T("ViewBottom"), (int &) m_ViewBottom, FALSE);
	integralTypeFromRegistry(hKey, _T("ViewFront"), (int &) m_ViewFront, TRUE);
	integralTypeFromRegistry(hKey, _T("ViewLeft"), (int &) m_ViewLeft, FALSE);
	integralTypeFromRegistry(hKey, _T("ViewRight"), (int &) m_ViewRight, FALSE);
	integralTypeFromRegistry(hKey, _T("ViewTop"), (int &) m_ViewTop, FALSE);
	integralTypeFromRegistry(hKey, _T("OutputWidth"), m_OutputWidth, 128);
	integralTypeFromRegistry(hKey, _T("OutputHeight"), m_OutputHeight, 128);
	integralTypeFromRegistry(hKey, _T("Format"), m_Format, 0);
	integralTypeFromRegistry(hKey, _T("OutputPathOption"), m_OutputPathOption, 1);
	UpdateData(FALSE);
	updateUIEnabledState();
}

//****************************************************************************************
void CSnapshotToolDlg::toRegistry()
{
	UpdateData();
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, NEL_OV_SNAPSHOT_TOOL_REGKEY, &hKey)==ERROR_SUCCESS)
	{		
		RegSetValueEx(hKey, _T("InputPath"), 0, REG_SZ, (BYTE*) (LPCTSTR) m_InputPath, (m_InputPath.GetLength() + 1) * sizeof(TCHAR));
		RegSetValueEx(hKey, _T("OutputPath"), 0, REG_SZ, (BYTE*) (LPCTSTR) m_OutputPath, (m_OutputPath.GetLength() + 1) * sizeof(TCHAR));
		CString filters;
		for (uint k = 0; k < (uint) m_Filters.GetCount(); ++k)
		{
			if (k!=0) filters += ",";
			CString filter;
			m_Filters.GetText(k, filter);
			filters += filter;			
		}

		RegSetValueEx(hKey, _T("Filters"), 0, REG_SZ, (BYTE*) (LPCTSTR) filters, (filters.GetLength() + 1) * sizeof(TCHAR));
		DWORD recurseSubFolder = m_RecurseSubFolder;
		DWORD dumpTextureSets = m_DumpTextureSets;
		DWORD width = (DWORD) m_OutputWidth;
		DWORD height = (DWORD) m_OutputHeight;		
		DWORD format = m_Format;
		DWORD outputPathOption = m_OutputPathOption;
		DWORD postFixViewName = m_PostFixViewName;
		integralTypeFromRegistry(hKey, _T("PostFixViewName"), (int &) m_PostFixViewName, TRUE);
		RegSetValueEx(hKey, _T("ViewBack"), 0, REG_DWORD, (const BYTE *) &m_ViewBack, sizeof(DWORD));
		RegSetValueEx(hKey, _T("ViewBottom"), 0, REG_DWORD, (const BYTE *) &m_ViewBottom, sizeof(DWORD));
		RegSetValueEx(hKey, _T("ViewFront"), 0, REG_DWORD, (const BYTE *) &m_ViewFront, sizeof(DWORD));
		RegSetValueEx(hKey, _T("ViewLeft"), 0, REG_DWORD, (const BYTE *) &m_ViewLeft, sizeof(DWORD));
		RegSetValueEx(hKey, _T("ViewRight"), 0, REG_DWORD, (const BYTE *) &m_ViewRight, sizeof(DWORD));
		RegSetValueEx(hKey, _T("ViewTop"), 0, REG_DWORD, (const BYTE *) &m_ViewTop, sizeof(DWORD));
		RegSetValueEx(hKey, _T("RecurseSubFolder"), 0, REG_DWORD, (const BYTE *) &recurseSubFolder, sizeof(DWORD));
		RegSetValueEx(hKey, _T("DumpTextureSets"), 0, REG_DWORD, (const BYTE *) &dumpTextureSets, sizeof(DWORD));
		RegSetValueEx(hKey, _T("OutputWidth"), 0, REG_DWORD, (const BYTE *) &width, sizeof(DWORD));
		RegSetValueEx(hKey, _T("OutputHeight"), 0, REG_DWORD, (const BYTE *) &height, sizeof(DWORD));
		RegSetValueEx(hKey, _T("Format"), 0, REG_DWORD, (const BYTE *) &format, sizeof(DWORD));
		RegSetValueEx(hKey, _T("OutputPathOption"), 0, REG_DWORD, (const BYTE *) &outputPathOption, sizeof(DWORD));
	}
}


BEGIN_MESSAGE_MAP(CSnapshotToolDlg, CDialog)
	//{{AFX_MSG_MAP(CSnapshotToolDlg)
	ON_BN_CLICKED(IDC_BROWSE_INPUT_PATH, OnBrowseInputPath)
	ON_BN_CLICKED(IDC_BROWSE_OUTPUT_PATH, OnBrowseOutputPath)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_BN_CLICKED(IDC_ADD_FILTER, OnAddFilter)
	ON_BN_CLICKED(IDC_REMOVE_FILTER, OnRemoveFilter)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
	ON_EN_KILLFOCUS(IDC_HEIGHT, OnKillfocusHeight)
	ON_EN_KILLFOCUS(IDC_WIDTH, OnKillfocusWidth)
	ON_CBN_SELCHANGE(IDC_OUTPUTPATH_OPTION, OnSelchangeOutputpathOption)
	ON_CBN_SELCHANGE(IDC_FORMAT, OnSelchangeFormat)
	ON_BN_CLICKED(IDC_CLOSE, OnCloseButton)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOP, OnStopSnapshots)
	ON_BN_CLICKED(IDC_VIEW_FRONT, OnViewFront)
	ON_BN_CLICKED(IDC_VIEW_LEFT, OnViewLeft)
	ON_BN_CLICKED(IDC_VIEW_RIGHT, OnViewRight)
	ON_BN_CLICKED(IDC_VIEW_TOP, OnViewTop)
	ON_BN_CLICKED(IDC_VIEW_ALL, OnViewAll)
	ON_BN_CLICKED(IDC_VIEW_NONE, OnViewNone)
	ON_BN_CLICKED(IDC_POSTFIX_VIEWNAME, OnPostFixViewName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSnapshotToolDlg message handlers


//****************************************************************************************
BOOL CSnapshotToolDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	fromRegistry();	
	SetTimer(1, 10, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//****************************************************************************************
void CSnapshotToolDlg::OnClose() 
{
	toRegistry();
	CDialog::OnClose();
}


//****************************************************************************************
void CSnapshotToolDlg::OnBrowseInputPath() 
{
	browseFolder(getStrRsc(IDS_CHOOSE_SNAPSHOT_INPUT_PATH), m_InputPath, this->m_hWnd);
	UpdateData(FALSE);
	toRegistry();
}

//****************************************************************************************
void CSnapshotToolDlg::OnBrowseOutputPath() 
{
	browseFolder(getStrRsc(IDS_CHOOSE_SNAPSHOT_OUTPUT_PATH), m_OutputPath, this->m_hWnd);
	UpdateData(FALSE);
	toRegistry();
}


//****************************************************************************************
void CSnapshotToolDlg::OnAddFilter() 
{
	CChooseName cn("*.shape", this);
	if (cn.DoModal() == IDOK && !cn.m_Name.IsEmpty())
	{
		m_Filters.AddString(cn.m_Name);
	}
	toRegistry();
}

//****************************************************************************************
void CSnapshotToolDlg::OnRemoveFilter() 
{
	if (m_Filters.GetCurSel() != LB_ERR)
	{
		m_Filters.DeleteString(m_Filters.GetCurSel());
	}
}


//****************************************************************************************
void CSnapshotToolDlg::OnEditFilter() 
{
	if (m_Filters.GetCurSel() != LB_ERR)
	{
		CString current;
		m_Filters.GetText(m_Filters.GetCurSel(), current);
		CChooseName cn((LPCTSTR) current, this);
		if (cn.DoModal() == IDOK && !cn.m_Name.IsEmpty())
		{
			int curSel = m_Filters.GetCurSel();
			m_Filters.DeleteString(m_Filters.GetCurSel());
			m_Filters.InsertString(curSel, cn.m_Name);
			m_Filters.SetCurSel(curSel);			
		}
	}
	toRegistry();
}


//****************************************************************************************
uint CSnapshotToolDlg::getSelectedViewCount() 
{
	UpdateData();
	return (m_ViewBack ? 1 : 0) +
			(m_ViewBottom ? 1 : 0) +
			(m_ViewFront ? 1 : 0) + 
			(m_ViewLeft ? 1 : 0) +
			(m_ViewRight ? 1 : 0) +
			(m_ViewTop ? 1 : 0);
}

//****************************************************************************************
void CSnapshotToolDlg::updateUIEnabledState() 
{
	BOOL enabled = _FilteredFiles.empty() ? TRUE : FALSE;
	m_Filters.EnableWindow(enabled);		
    GetDlgItem(IDC_INPUT_PATH)->EnableWindow(enabled);
    GetDlgItem(IDC_BROWSE_INPUT_PATH)->EnableWindow(enabled);
    GetDlgItem(IDC_OUTPUT_PATH)->EnableWindow(enabled  && m_OutputPathOption == OutputPath_Custom);
    GetDlgItem(IDC_BROWSE_OUTPUT_PATH)->EnableWindow(enabled);
    GetDlgItem(IDC_RECURSE_SUBFOLDER)->EnableWindow(enabled);
    GetDlgItem(IDC_OUTPUTPATH_OPTION)->EnableWindow(enabled);
    GetDlgItem(IDC_FILTERS)->EnableWindow(enabled);
    GetDlgItem(IDC_ADD_FILTER)->EnableWindow(enabled);
    GetDlgItem(IDC_REMOVE_FILTER)->EnableWindow(enabled);
    GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(enabled);
    GetDlgItem(IDC_WIDTH)->EnableWindow(enabled);
    GetDlgItem(IDC_HEIGHT)->EnableWindow(enabled);    
    GetDlgItem(IDC_FORMAT)->EnableWindow(enabled);
    GetDlgItem(IDC_GO)->EnableWindow(enabled);
	GetDlgItem(IDC_DUMP_TEXTURE_SETS)->EnableWindow(enabled);
    GetDlgItem(IDC_STOP)->EnableWindow(!enabled);				
	GetDlgItem(IDC_POSTFIX_VIEWNAME)->EnableWindow((enabled && getSelectedViewCount() == 1) ? TRUE : FALSE);
    //GetDlgItem(IDC_CLOSE)->EnaleWindow(enabled);

}

//****************************************************************************************
void CSnapshotToolDlg::OnGo() 
{	
	UpdateData();
	if (getSelectedViewCount() == 0)
	{
		MessageBox(getStrRsc(IDS_SNAPSHOT_NO_VIEW_SELECTED), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
		return;
	}
	if (m_InputPath.IsEmpty())
	{
		MessageBox(getStrRsc(IDS_SNAPSHOT_EMPTY_INPUT_PATH), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
		return;
	}
	if (!NLMISC::CFile::isDirectory(tStrToUtf8(m_InputPath)))
	{
		MessageBox(getStrRsc(IDS_SNAPSHOT_EMPTY_INPUT_PATH_NOT_FOUND), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
		return;
	}
	if (m_OutputPathOption == OutputPath_Custom && m_OutputPath.IsEmpty())
	{
		MessageBox(getStrRsc(IDS_SNAPSHOT_EMPTY_OUTPUT_PATH), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
		return;
	}
	if (m_OutputPathOption == OutputPath_Custom && !NLMISC::CFile::isDirectory(tStrToUtf8(m_OutputPath)))
	{
		if (MessageBox(getStrRsc(IDS_SNAPSHOT_CREATE_OUTPUT_DIRECTORY), getStrRsc(IDS_OBJECT_VIEWER), MB_OKCANCEL) != IDOK)
		{
			return;
		}
		if(!NLMISC::CFile::createDirectoryTree(tStrToUtf8(m_OutputPath)))
		{
			MessageBox(getStrRsc(IDS_SNAPSHOT_OUTPUT_PATH_CREATION_FAILED), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
			return;
		}
	}

	// make sure that the screen can contains the window client area
	RECT desktopSize;
	::GetClientRect(::GetDesktopWindow(), &desktopSize);
	if (desktopSize.right < LONG(m_OutputWidth) ||
		desktopSize.bottom < LONG(m_OutputHeight))
	{
		MessageBox(getStrRsc(IDS_DESKTOP_TOO_SMALL_FOR_SNAPSHOT), getStrRsc(IDS_OBJECT_VIEWER), MB_ICONEXCLAMATION);
		return;
	}
	
	m_Log.ResetContent();
	m_Log.AddString(getStrRsc(IDS_GETTING_PATH_CONTENT));
	std::vector<std::string> files;
	CPath::getPathContent(tStrToUtf8(m_InputPath), m_RecurseSubFolder == TRUE, false, true, files);
	if (files.empty())
	{
		m_Log.AddString(getStrRsc(IDS_SNAPSHOT_NO_FILES_FOUND));
		return;
	}
	_FilteredFiles.clear();
	for (uint k = 0; k < files.size(); ++k)
	{
		for (uint l = 0; l < uint(m_Filters.GetCount()); ++l)
		{
			CString wildCard;
			m_Filters.GetText(l, wildCard);
			wildCard.MakeLower();
			if (testWildCard(toLowerAscii(NLMISC::CFile::getFilename(files[k])).c_str(), tStrToUtf8(wildCard).c_str()))
			{
				_FilteredFiles.push_back(files[k]);
				break;
			}
		}
	}
	if (_FilteredFiles.empty())
	{
		m_Log.AddString(getStrRsc(IDS_SNAPSHOT_NO_FILTER_MATCH_FOUND));
		return;
	}
	m_Log.AddString(toString("%d", (int) _FilteredFiles.size()).c_str() + getStrRsc(IDS_SNAPSHOT_FILES_TO_PROCESS));			
	updateUIEnabledState();
}

//****************************************************************************************
void CSnapshotToolDlg::OnChangeWidth() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

}

//****************************************************************************************
void CSnapshotToolDlg::OnKillfocusHeight() 
{
	toRegistry();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnKillfocusWidth() 
{
	toRegistry();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnSelchangeOutputpathOption() 
{
	toRegistry();
	updateUIEnabledState();
}


//****************************************************************************************
void CSnapshotToolDlg::OnSelchangeFormat() 
{
	toRegistry();	
}


//****************************************************************************************
void CSnapshotToolDlg::OnCloseButton() 
{
	toRegistry();
	SendMessage(WM_CLOSE);
}

//****************************************************************************************
void CSnapshotToolDlg::setCamFromView(uint view, CCamera *cam, const CAABBox &bbox) 
{
	static const CVector camBackDir[] = 
	{
		-CVector::J,
		CVector::I,
		-CVector::I,
		CVector::K,
		-CVector::K,		
		CVector::J
	};
	const float viewDistScale = 3.f;
	CVector halfSize = bbox.getHalfSize();
	if (view == SnapshotAngle_Top) // top
	{
		CMatrix camMat;
		camMat.setRot(CVector::I, -CVector::K, CVector::J);
		camMat.setPos(bbox.getCenter() + viewDistScale * CVector::K);
		cam->setMatrix(camMat);
	}
	else if (view == SnapshotAngle_Bottom)
	{
		CMatrix camMat;
		camMat.setRot(CVector::I, CVector::K, -CVector::J);
		camMat.setPos(bbox.getCenter() - viewDistScale * CVector::K);
		cam->setMatrix(camMat);
	}
	else
	{
		// standard lookat for other directions
		cam->lookAt(bbox.getCenter() + viewDistScale * maxof(halfSize.x, halfSize.y, halfSize.z) * camBackDir[view], bbox.getCenter());
	}
}

//****************************************************************************************
std::string CSnapshotToolDlg::viewToString(uint view)
{
	switch(view)
	{
	case SnapshotAngle_Front: return "front";
	case SnapshotAngle_Right: return "right";
	case SnapshotAngle_Left: return "left";
	case SnapshotAngle_Top: return "top";
	case SnapshotAngle_Bottom: return "bottom";
	case SnapshotAngle_Back: return "back";
	}
	nlassert(0);
	return "";
}

//****************************************************************************************
void CSnapshotToolDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (_FilteredFiles.empty()) return;	
	if (nIDEvent == 1)
	{
		// resize the window if needed
		HWND wnd = (HWND) NL3D::CNELU::Driver->getDisplay();		
		CScene		 scene(true /* small scene */);

		scene.initDefaultRoots();		
		scene.setDriver(CNELU::Driver);		
		scene.initQuadGridClipManager();
		NL3D::CViewport vp;
		vp.init(0.f, 0.f, 1.f, 1.f);
		scene.setViewport(vp);
		scene.enableLightingSystem(true);
		scene.setAmbientGlobal(CRGBA::White);

		CShapeBank   sb;
		scene.setShapeBank(&sb);
		CCamera *cam= safe_cast<CCamera *>(scene.createModel(CameraId));
		scene.setCam(cam);
		cam->setFrustum(0.1f, 0.1f, 0.1f, 5000.f);				
				

		try
		{
			CShapeStream ss;
			m_Log.AddString(nlUtf8ToTStr(_FilteredFiles[0]));
			CIFile stream(_FilteredFiles[0]);
			ss.serial(stream);
			nlassert(ss.getShapePointer());
			CAABBox bbox;
			ss.getShapePointer()->getAABBox(bbox);
			sb.add("shape", ss.getShapePointer());
			CTransformShape	*model = scene.createInstance("shape");						
			CMatrix initTM = model->getMatrix();
			model->setTransformMode(CTransform::DirectMatrix);
			initTM.setPos(CVector::Null);
			model->setMatrix(initTM);
			CVector newBBoxCenter = initTM * bbox.getCenter();
			CVector newBBoxHalfSize = initTM.mulVector(bbox.getHalfSize());
			bbox.setCenter(newBBoxCenter);
			bbox.setHalfSize(newBBoxHalfSize);
			if (model)
			{				
				CMeshBaseInstance *mbi = dynamic_cast<CMeshBaseInstance *>(model);
				uint textureSetCount = 1;
				CMeshBase *mb = dynamic_cast<CMeshBase *>((IShape *) model->Shape);
				CMesh	  *mesh = dynamic_cast<CMesh *>((IShape *) model->Shape);
				if (m_DumpTextureSets && mbi)
				{					
					if (mb)
					{
						const uint numMat = mb->getNbMaterial();	
						// see which material are selectable
						for(uint k = 0; k < numMat; ++k)
						{
							CMaterial &mat = mb->getMaterial(k);
							for(uint l = 0; l < IDRV_MAT_MAXTEXTURES; ++l)
							{
								CTextureMultiFile *tmf = dynamic_cast<CTextureMultiFile *>(mat.getTexture(l));
								if (tmf)
								{
									textureSetCount = std::max(textureSetCount, (uint) tmf->getNumFileName());
								}
							}
						}
					}
				}		
				// for particle systems :  simulate until half duration, then capture bbox
				CParticleSystemModel *psm = dynamic_cast<CParticleSystemModel *>(model);
				if (psm)
				{
					psm->forceInstanciate();
					CParticleSystem *ps = psm->getPS();
					if (ps)
					{
						float duration = ps->evalDuration();
						float deltaT = .5f * ps->getTimeTheshold();
						for (float date = 0.f; date < duration * 0.5f; date += deltaT) 
						{
							ps->setSysMat(&CMatrix::Identity);
							ps->step(CParticleSystem::Anim, deltaT, *safe_cast<CParticleSystemShape *>((IShape *) psm->Shape), *psm);
						}
						ps->computeBBox(bbox);
					}
				}

				cam->setTransformMode(ITransformable::DirectMatrix);				
				BOOL views[] = 
				{ 
					m_ViewFront,
					m_ViewRight,
					m_ViewLeft,
					m_ViewTop,
					m_ViewBottom,
					m_ViewBack
				};
				for (uint viewIndex = 0; viewIndex < 6; ++viewIndex)
				{
					if (!views[viewIndex]) continue;
					setCamFromView(viewIndex, cam, bbox);					
					for (uint textureSet = 0; textureSet < textureSetCount; ++textureSet)
					{
						if (mbi)
						{
							mbi->selectTextureSet(textureSet);
						}

						CNELU::Driver->clear2D(_ObjectViewer->getBackGroundColor());
						CNELU::Driver->clearZBuffer();
						scene.render();
						NLMISC::CBitmap snapshot;
						CNELU::Driver->getBuffer(snapshot);
						if (snapshot.getWidth() == 0 || snapshot.getHeight() == 0)
						{
							m_Log.AddString(getStrRsc(IDS_SNAPSHOT_NO_CONTENT_RETRIEVED));
						}
						else
						{
							snapshot.resample(m_OutputWidth, m_OutputHeight);
							size_t lastPoint = _FilteredFiles[0].find_last_of('.');
							std::string outputFilename = _FilteredFiles[0].substr(0, lastPoint);
							if (textureSetCount > 1)
							{
								outputFilename += toString("_%u", (unsigned int) textureSet);
							}
							if (getSelectedViewCount() != 1 || m_PostFixViewName)
							{
								outputFilename += "_" + viewToString(viewIndex);
							}

							std::string ext;
							switch (m_Format)
							{
							case OutputFormat_Tga:
								ext = "tga";
								break;
							case OutputFormat_Png:
								ext = "png";
								break;
							case OutputFormat_Jpg:
								ext = "jpg";
								break;
							default:
								nlerror("Unsupported format %d", m_Format);
								break;
							}
							outputFilename += "." + ext;

							switch(m_OutputPathOption)
							{
								case OutputPath_Custom: // custom output path
									outputFilename = tStrToUtf8(m_OutputPath) + "\\" + NLMISC::CFile::getFilename(outputFilename);
								break;
								case OutputPath_SameAsInput: // Input path
									outputFilename = tStrToUtf8(m_InputPath) + "\\" + NLMISC::CFile::getFilename(outputFilename);
								break;
								case OutputPath_CurrShapeDirectory: // current path
									// no op
								break;
							}
							COFile output(outputFilename);

							if (m_Format == OutputFormat_Tga)
							{
								snapshot.writeTGA(output);
							}
							else if (m_Format == OutputFormat_Png)
							{
								snapshot.writePNG(output);
							}
							else
							{
								snapshot.writeJPG(output);
							}						
						}
						CNELU::Driver->swapBuffers();
					}
				}
				scene.deleteInstance(model);
			}
			sb.reset();
		}
		catch(const std::exception &e)
		{
			nlwarning(e.what());
			
		}				
		_FilteredFiles.pop_front();
		if (_FilteredFiles.empty())
		{
			m_Log.AddString(getStrRsc(IDS_SNAPSHOT_FINISHED));
			updateUIEnabledState();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

//****************************************************************************************
void CSnapshotToolDlg::OnStopSnapshots() 
{
	_FilteredFiles.clear();
	updateUIEnabledState();
}


//****************************************************************************************
void CSnapshotToolDlg::OnViewFront() 
{
	toRegistry();
	updateUIEnabledState();
}

//****************************************************************************************
void CSnapshotToolDlg::OnViewLeft() 
{
	toRegistry();
	updateUIEnabledState();
}

//****************************************************************************************
void CSnapshotToolDlg::OnViewRight() 
{
	toRegistry();
	updateUIEnabledState();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnViewTop() 
{
	toRegistry();
	updateUIEnabledState();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnViewAll() 
{
	m_ViewBack = TRUE;
	m_ViewBottom = TRUE;
	m_ViewFront = TRUE;
	m_ViewLeft = TRUE;
	m_ViewRight = TRUE;
	m_ViewTop = TRUE;
	UpdateData(FALSE);
	toRegistry();
	updateUIEnabledState();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnViewNone() 
{
	m_ViewBack = FALSE;
	m_ViewBottom = FALSE;
	m_ViewFront = FALSE;
	m_ViewLeft = FALSE;
	m_ViewRight = FALSE;
	m_ViewTop = FALSE;
	UpdateData(FALSE);
	toRegistry();
	updateUIEnabledState();	
}

//****************************************************************************************
void CSnapshotToolDlg::OnPostFixViewName() 
{
	toRegistry();
}
