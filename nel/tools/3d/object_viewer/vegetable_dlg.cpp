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
#include "vegetable_dlg.h"
#include "vegetable_density_page.h"
#include "vegetable_apperance_page.h"
#include "vegetable_scale_page.h"
#include "vegetable_rotate_page.h"
#include "vegetable_copy_dlg.h"
#include "vegetable_edit_tools.h"
#include "vegetable_wind_dlg.h"
#include "nel/3d/vegetable.h"
#include "nel/3d/tile_vegetable_desc.h"



/////////////////////////////////////////////////////////////////////////////
// CVegetableDlg dialog


CVegetableDlg::CVegetableDlg(CObjectViewer *viewer, CWnd* pParent /*=NULL*/)
	: CDialog(CVegetableDlg::IDD, pParent), _ObjView(viewer),
	_PropertySheet(NULL), _VegetableDensityPage(NULL), _VegetableApperancePage(NULL), 
	_VegetableScalePage(NULL), _VegetableRotatePage(NULL),
	_VegetableWindDlg(NULL)
{
	nlassert(viewer);

	_LastVegetSetName= "*.vegetset";

	// init VegetableList
	VegetableList.VegetableDlg= this;

	//{{AFX_DATA_INIT(CVegetableDlg)
	//}}AFX_DATA_INIT
}


void CVegetableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVegetableDlg)
	DDX_Control(pDX, IDC_LIST_VEGETABLE, VegetableList);
	DDX_Control(pDX, IDC_STATIC_VEGETABLE_PERF, StaticPolyCount);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_SNAPTOGROUND, CheckSnapToGround);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_ENABLE, CheckEnableVegetable);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_AUTOMATIC, CheckAutomaticRefresh);
	DDX_Control(pDX, IDC_BUTTON_VEGETABLE_REFRESH, ButtonRefreshLandscape);
	DDX_Control(pDX, IDC_CHECK_VEGETABLE_SHOW, CheckShowLandscape);
	DDX_Control(pDX, IDC_STATIC_SELECT_VEGETABLE, SelectVegetableStaticText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVegetableDlg, CDialog)
	//{{AFX_MSG_MAP(CVegetableDlg)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST_VEGETABLE, OnSelchangeListVegetable)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_ADD, OnButtonVegetableAdd)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_CLEAR, OnButtonVegetableClear)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_INSERT, OnButtonVegetableInsert)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_LOAD_DESC, OnButtonVegetableLoadDesc)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_LOAD_SET, OnButtonVegetableLoadSet)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_REMOVE, OnButtonVegetableRemove)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_SAVE_DESC, OnButtonVegetableSaveDesc)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_SAVE_SET, OnButtonVegetableSaveSet)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_APPEND_SET, OnButtonVegetableAppendSet)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_COPY, OnButtonVegetableCopy)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_REFRESH, OnButtonVegetableRefresh)
	ON_BN_CLICKED(IDC_CHECK_VEGETABLE_SHOW, OnCheckVegetableShow)
	ON_BN_CLICKED(IDC_BUTTON_VEGETABLE_SETUP_WIND, OnButtonVegetableSetupWind)
	ON_BN_CLICKED(IDC_CHECK_VEGETABLE_AUTOMATIC, OnCheckVegetableAutomatic)
	ON_BN_CLICKED(IDC_CHECK_VEGETABLE_ENABLE, OnCheckVegetableEnable)
	ON_BN_CLICKED(IDC_CHECK_VEGETABLE_SNAPTOGROUND, OnCheckVegetableSnaptoground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVegetableDlg message handlers

void CVegetableDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_VIEW_VEGETABLE_DLG);

	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}


CVegetableDlg::~CVegetableDlg()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_VegetableDensityPage);
	REMOVE_WND(_VegetableApperancePage);	
	REMOVE_WND(_VegetableScalePage);
	REMOVE_WND(_VegetableRotatePage);
	REMOVE_WND(_PropertySheet);	
	// destroy and remove wind dialog
	REMOVE_WND(_VegetableWindDlg);	
}




// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CVegetableDlg::doRefreshVegetableDisplay()
{
	NL3D::CTileVegetableDesc	vegetSet;

	// first build the vegetSet, but don't keep <default> shapeName. and skip Hiden vegetables too
	buildVegetableSet(vegetSet, false, false);

	// then refresh window.
	_ObjView->refreshVegetableLandscape(vegetSet);
}


// ***************************************************************************
void			CVegetableDlg::refreshVegetableDisplay()
{
	// if automatic refresh is checked.
	if(CheckAutomaticRefresh.GetCheck()==1)
		// then do it
		doRefreshVegetableDisplay();
}


// ***************************************************************************
void			CVegetableDlg::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	if(vegetable == NULL)
	{
		// Show the SelectVegetableStaticText
		SelectVegetableStaticText.ShowWindow(true);

		// Hide the property sheet.
		_PropertySheet->ShowWindow(false);
	}
	else
	{
		// Hide the SelectVegetableStaticText
		SelectVegetableStaticText.ShowWindow(false);

		// Setup 3 property pages.
		_VegetableDensityPage->setVegetableToEdit(vegetable);
		_VegetableApperancePage->setVegetableToEdit(vegetable);
		_VegetableScalePage->setVegetableToEdit(vegetable);
		_VegetableRotatePage->setVegetableToEdit(vegetable);

		// Show the property sheet.
		_PropertySheet->ShowWindow(true);
	}
}

// ***************************************************************************
uint				CVegetableDlg::getNumVegetables() const
{
	return (uint)_Vegetables.size();
}
// ***************************************************************************
std::string			CVegetableDlg::getVegetableName(uint id) const
{
	nlassert(id<_Vegetables.size());
	return	_Vegetables[id].VegetableName;
}
// ***************************************************************************
void				CVegetableDlg::updateCurSelVegetableName()
{
	sint	id= VegetableList.GetCurSel();
	if(id!=LB_ERR)
	{
		_Vegetables[id].updateVegetableName();
		// replace name in the listBox: must delete, and re-insert
		VegetableList.DeleteString(id);
		VegetableList.InsertString(id, nlUtf8ToTStr(_Vegetables[id].VegetableName));
		VegetableList.SetCurSel(id);
	}
}

// ***************************************************************************
NL3D::CVegetable	*CVegetableDlg::getVegetable(uint id) const
{
	nlassert(id<_Vegetables.size());
	return _Vegetables[id].Vegetable;
}


static const char	*NL_DefaultVegetName= "<default>";

// ***************************************************************************
CVegetableDlg::CVegetableDesc::CVegetableDesc()
{
	Vegetable= NULL;
	VegetableName= NL_DefaultVegetName;
	Visible= true;
}

// ***************************************************************************
void		CVegetableDlg::CVegetableDesc::initDefaultVegetable()
{
	Vegetable= new NL3D::CVegetable;
	// update vegetableName according to Vegetable
	updateVegetableName();

	// init Vegetable with some good default values.

	// General/Density.
	// Density.
	Vegetable->Density.Abs= NL_VEGETABLE_DENSITY_ABS_DEFAULT;
	Vegetable->Density.Rand= NL_VEGETABLE_DENSITY_RAND_DEFAULT;
	Vegetable->Density.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// disable MaxDensity
	Vegetable->MaxDensity= -1;
	// Leave ShapeName to ""
	// Default DistType is always 0.
	Vegetable->DistType= 0;


	// Apperance
	// BendPhase
	Vegetable->BendPhase.Abs= NL_VEGETABLE_BENDPHASE_ABS_DEFAULT;
	Vegetable->BendPhase.Rand= NL_VEGETABLE_BENDPHASE_RAND_DEFAULT;
	Vegetable->BendPhase.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// BendFactor
	Vegetable->BendFactor.Abs= NL_VEGETABLE_BENDFACTOR_ABS_DEFAULT;
	Vegetable->BendFactor.Rand= NL_VEGETABLE_BENDFACTOR_RAND_DEFAULT;
	Vegetable->BendFactor.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// Color
	Vegetable->Color.NoiseValue.Abs= NL_VEGETABLE_COLOR_ABS_DEFAULT;
	Vegetable->Color.NoiseValue.Rand= NL_VEGETABLE_COLOR_RAND_DEFAULT;
	Vegetable->Color.NoiseValue.Frequency= NL_VEGETABLE_FREQ_DEFAULT;

	// Scale
	// ScaleXY
	Vegetable->Sxy.Abs= NL_VEGETABLE_SCALE_ABS_DEFAULT;
	Vegetable->Sxy.Rand= NL_VEGETABLE_SCALE_RAND_DEFAULT;
	Vegetable->Sxy.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// ScaleZ.
	Vegetable->Sz.Abs= NL_VEGETABLE_SCALE_ABS_DEFAULT;
	Vegetable->Sz.Rand= NL_VEGETABLE_SCALE_RAND_DEFAULT;
	Vegetable->Sz.Frequency= NL_VEGETABLE_FREQ_DEFAULT;

	// Rotate
	// RotateX
	Vegetable->Rx.Abs= NL_VEGETABLE_ROTATEX_ABS_DEFAULT;
	Vegetable->Rx.Rand= NL_VEGETABLE_ROTATEX_RAND_DEFAULT;
	Vegetable->Rx.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// RotateY
	Vegetable->Ry.Abs= NL_VEGETABLE_ROTATEY_ABS_DEFAULT;
	Vegetable->Ry.Rand= NL_VEGETABLE_ROTATEY_RAND_DEFAULT;
	Vegetable->Ry.Frequency= NL_VEGETABLE_FREQ_DEFAULT;
	// RotateZ
	Vegetable->Rz.Abs= NL_VEGETABLE_ROTATEZ_ABS_DEFAULT;
	Vegetable->Rz.Rand= NL_VEGETABLE_ROTATEZ_RAND_DEFAULT;
	Vegetable->Rz.Frequency= NL_VEGETABLE_ROTATEZ_FREQ_DEFAULT;

}

// ***************************************************************************
void		CVegetableDlg::CVegetableDesc::initVegetable(const NL3D::CVegetable &vegetable)
{
	Vegetable= new NL3D::CVegetable(vegetable);
	// update vegetableName according to Vegetable
	updateVegetableName();
}


// ***************************************************************************
void		CVegetableDlg::CVegetableDesc::updateVegetableName()
{
	// Build the vegetable Name according to the ShapeName
	if(Vegetable->ShapeName.empty())
	{
		VegetableName= NL_DefaultVegetName;
	}
	else
	{
		std::string::size_type pos= Vegetable->ShapeName.find(".veget");
		VegetableName= Vegetable->ShapeName.substr(0, pos);
		// And (to be clearer) append distance of creation.
		char	str[256];
		sprintf(str, " - %dm", (Vegetable->DistType+1)*10);
		VegetableName+= str;
		// NB: if you add info with other parameters, you must use updateCurSelVegetableName() if they change
	}
}


// ***************************************************************************
void		CVegetableDlg::CVegetableDesc::deleteVegetable()
{
	delete Vegetable;
	Vegetable= NULL;
	VegetableName= NL_DefaultVegetName;
}


// ***************************************************************************
void		CVegetableDlg::clearVegetables()
{
	// delete all vegetables.
	for(uint i=0; i<_Vegetables.size(); i++)
	{
		_Vegetables[i].deleteVegetable();
	}
	_Vegetables.clear();

	// update view
	VegetableList.ResetContent();
	setVegetableToEdit(NULL);
}


// ***************************************************************************
bool		CVegetableDlg::loadVegetableSet(NL3D::CTileVegetableDesc &vegetSet, const TCHAR *title)
{
	vegetSet.clear();
	bool	ok= false;

	CFileDialog fd(TRUE, _T(".vegetset"), _T("*.vegetset"), 0, NULL, this) ;
	fd.m_ofn.lpstrTitle = title;
	if (fd.DoModal() == IDOK)
	{
		NLMISC::CIFile	f;
		
		ok= true;

		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			try
			{
				// read the vegetable
				f.serial(vegetSet);
				// bkup fileName.
				_LastVegetSetName = NLMISC::tStrToUtf8(fd.GetFileName());
			}
			catch(const NLMISC::EStream &)
			{
				ok= false;
				MessageBox(_T("Failed to load file!"));
			}
		}
		else
		{
			ok= false;
			MessageBox(_T("Failed to open file!"));
		}
	}

	return ok;
}

// ***************************************************************************
void		CVegetableDlg::buildVegetableSet(NL3D::CTileVegetableDesc &vegetSet, bool keepDefaultShapeName, bool keepHiden )
{
	vegetSet.clear();
	float	degToRad= (float)(NLMISC::Pi / 180.f);

	// build the list.
	std::vector<NL3D::CVegetable>	vegetables;
	for(uint i=0;i<_Vegetables.size();i++)
	{
		// if don't want to keep <default> ShapeNames, skip them.
		if(!keepDefaultShapeName && _Vegetables[i].Vegetable->ShapeName.empty())
			continue;
		// if don't want to keep hiden vegetables, skip them.
		if(!keepHiden && !_Vegetables[i].Visible)
			continue;

		vegetables.push_back(*_Vegetables[i].Vegetable);
		// get dst index.
		uint	dstId= (uint)vegetables.size()-1;
		// transform degrees in radians.
		vegetables[dstId].Rx.Abs*= degToRad;
		vegetables[dstId].Rx.Rand*= degToRad;
		vegetables[dstId].Ry.Abs*= degToRad;
		vegetables[dstId].Ry.Rand*= degToRad;
		vegetables[dstId].Rz.Abs*= degToRad;
		vegetables[dstId].Rz.Rand*= degToRad;
	}

	// build the set.
	vegetSet.build(vegetables);
}


// ***************************************************************************
void		CVegetableDlg::appendVegetableSet(NL3D::CTileVegetableDesc &vegetSet)
{
	float	radToDeg= (float)(180.f / NLMISC::Pi);

	// for all distances Types.
	for(uint distType=0; distType<NL3D_VEGETABLE_BLOCK_NUMDIST; distType++)
	{
		// retrieve list of vegetable
		const std::vector<NL3D::CVegetable>		&vegetList= vegetSet.getVegetableList(distType);

		// for all of them
		for(uint i=0;i<vegetList.size();i++)
		{
			// append the vegetable to the list.
			NL3D::CVegetable	veget= vegetList[i];

			// transform radians into degrees.
			veget.Rx.Abs*= radToDeg;
			veget.Rx.Rand*= radToDeg;
			veget.Ry.Abs*= radToDeg;
			veget.Ry.Rand*= radToDeg;
			veget.Rz.Abs*= radToDeg;
			veget.Rz.Rand*= radToDeg;

			// Add a new vegetable to the list.
			_Vegetables.push_back( CVegetableDesc ());
			uint	id= (uint)_Vegetables.size()-1;
			_Vegetables[id].initVegetable(veget);

			// update view
			VegetableList.AddString(nlUtf8ToTStr(_Vegetables[id].VegetableName));
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CVegetableDlg message handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
BOOL CVegetableDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();


	_PropertySheet= new CPropertySheet();

	// Create the 4 pages.
	// density
	_VegetableDensityPage= new CVegetableDensityPage();
	_VegetableDensityPage->initVegetableDlg(this);
	_PropertySheet->AddPage(_VegetableDensityPage);
	// appearance
	_VegetableApperancePage= new CVegetableApperancePage();
	_VegetableApperancePage->initVegetableDlg(this);
	_PropertySheet->AddPage(_VegetableApperancePage);
	// scale
	_VegetableScalePage= new CVegetableScalePage();
	_VegetableScalePage->initVegetableDlg(this);
	_PropertySheet->AddPage(_VegetableScalePage);
	// rot
	_VegetableRotatePage= new CVegetableRotatePage();
	_VegetableRotatePage->initVegetableDlg(this);
	_PropertySheet->AddPage(_VegetableRotatePage);

	// Create the _PropertySheet in the DialogBox.
	_PropertySheet->Create(this, WS_CHILD | DS_CONTROL | WS_VISIBLE, 0);
	// Enlarge at max size of the PropertySheet
	_PropertySheet->MoveWindow(160, 0, 1000, 570);


	// Force creation of the 4 pages, by selecting them.
	_PropertySheet->SetActivePage(0);
	_PropertySheet->SetActivePage(1);
	_PropertySheet->SetActivePage(2);
	_PropertySheet->SetActivePage(3);
	// Start with Density page selected.
	_PropertySheet->SetActivePage(0);


	// Init the Dlg with no vegetable to edit.
	setVegetableToEdit(NULL);

	// Enable the Automatic update by default.
	CheckAutomaticRefresh.SetCheck(1);
	CheckEnableVegetable.SetCheck(1);
	CheckSnapToGround.SetCheck(1);

	// Disable the refresh button, because landscape not displayed by default
	ButtonRefreshLandscape.EnableWindow(false);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// ***************************************************************************
void CVegetableDlg::OnSelchangeListVegetable() 
{
	// set the new vegetable to edit.
	sint	id= VegetableList.GetCurSel();
	if(id==LB_ERR)
		setVegetableToEdit( NULL );
	else
		setVegetableToEdit( _Vegetables[id].Vegetable );
}

void CVegetableDlg::OnButtonVegetableAdd() 
{
	// Add a new vegetable to the list.
	_Vegetables.push_back(CVegetableDesc ());
	uint	id= (uint)_Vegetables.size()-1;
	_Vegetables[id].initDefaultVegetable();

	// update view
	VegetableList.AddString(nlUtf8ToTStr(_Vegetables[id].VegetableName));

	// update 3D view
	refreshVegetableDisplay();
}

void CVegetableDlg::OnButtonVegetableClear() 
{
	if(_Vegetables.size()==0)
		return;

	if( MessageBox(_T("Clear all the list?"), _T("Clear List"), MB_OKCANCEL | MB_ICONWARNING | MB_APPLMODAL)==IDOK )
	{
		clearVegetables();

		// update 3D view
		refreshVegetableDisplay();
	}
}

void CVegetableDlg::OnButtonVegetableInsert() 
{
	sint	id= VegetableList.GetCurSel();
	if(id!=LB_ERR)
	{
		// Add a new vegetable to the list.
		_Vegetables.insert(_Vegetables.begin()+id, CVegetableDesc());
		_Vegetables[id].initDefaultVegetable();

		// update view
		VegetableList.InsertString(id, nlUtf8ToTStr(_Vegetables[id].VegetableName));

		// update 3D view
		refreshVegetableDisplay();
	}
	else
	{
		// perform like an add.
		OnButtonVegetableAdd();
	}
}

void CVegetableDlg::OnButtonVegetableRemove() 
{
	sint	id= VegetableList.GetCurSel();
	if(id!=LB_ERR)
	{
		// UnSelect
		setVegetableToEdit(NULL);

		// erase the vegetable from the list.
		_Vegetables[id].deleteVegetable();
		_Vegetables.erase(_Vegetables.begin()+id);

		// update view
		VegetableList.DeleteString(id);

		// select if posssible at the same id.
		if(id>=(sint)_Vegetables.size())
			id--;
		if(id>=0)
		{
			// set the cur selection
			VegetableList.SetCurSel(id);
			// And set the new to edit
			OnSelchangeListVegetable();
		}

		// update 3D view
		refreshVegetableDisplay();
	}
}

// ***************************************************************************
void CVegetableDlg::OnButtonVegetableLoadDesc() 
{
	CFileDialog fd(TRUE, _T(".vegetdesc"), _T("*.vegetdesc"), 0, NULL, this) ;
	fd.m_ofn.lpstrTitle = _T("Open Vegetable Descriptor");
	if (fd.DoModal() == IDOK)
	{
		NLMISC::CIFile	f;
		
		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			NL3D::CVegetable	veget;
			try
			{
				// read the vegetable
				f.serial(veget);
				// Add a new vegetable to the list.
				_Vegetables.push_back(CVegetableDesc ());
				uint	id= (uint)_Vegetables.size()-1;
				_Vegetables[id].initVegetable(veget);

				// update view
				VegetableList.AddString(nlUtf8ToTStr(_Vegetables[id].VegetableName));

				// update 3D view
				refreshVegetableDisplay();
			}
			catch(const NLMISC::EStream &)
			{
				MessageBox(_T("Failed to load file!"));
			}
		}
		else
		{
			MessageBox(_T("Failed to open file!"));
		}
	}

}

void CVegetableDlg::OnButtonVegetableSaveDesc() 
{
	sint	id= VegetableList.GetCurSel();
	if(id!=LB_ERR)
	{
		NL3D::CVegetable	&veget= *_Vegetables[id].Vegetable;

		std::string		fileName= _Vegetables[id].VegetableName + ".vegetdesc";

		CFileDialog fd(FALSE, _T("vegetdesc"), nlUtf8ToTStr(fileName), OFN_OVERWRITEPROMPT, _T("VegetDescFiles (*.vegetdesc)|*.vegetdesc|All Files (*.*)|*.*||"), this);
		fd.m_ofn.lpstrTitle = _T("Save Vegetable Descriptor");
		if (fd.DoModal() == IDOK)
		{
			NLMISC::COFile	f;
			
			if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
			{
				try
				{
					// save the vegetable
					f.serial(veget);
				}
				catch(const NLMISC::EStream &)
				{
					MessageBox(_T("Failed to save file!"));
				}
			}
			else
			{
				MessageBox(_T("Failed to open file for write!"));
			}
		}
	}

}


// ***************************************************************************
void CVegetableDlg::OnButtonVegetableLoadSet() 
{
	NL3D::CTileVegetableDesc	vegetSet;
	// if succes to load the vegetSet
	if(loadVegetableSet(vegetSet, _T("Load Vegetable Set")))
	{
		// Delete all vegetables.
		clearVegetables();

		// build them from list.
		appendVegetableSet(vegetSet);

		// update 3D view
		refreshVegetableDisplay();
	}
}


void CVegetableDlg::OnButtonVegetableAppendSet() 
{
	NL3D::CTileVegetableDesc	vegetSet;
	// if succes to load the vegetSet
	if(loadVegetableSet(vegetSet, _T("Append Vegetable Set")))
	{
		// Do not Delete any vegetables.
		// build them from list.
		appendVegetableSet(vegetSet);

		// update 3D view
		refreshVegetableDisplay();
	}
}


void CVegetableDlg::OnButtonVegetableSaveSet() 
{
	NL3D::CTileVegetableDesc	vegetSet;

	// first build the vegetSet.
	buildVegetableSet(vegetSet);

	// Then try to save it.
	CFileDialog fd(FALSE, _T("vegetset"), nlUtf8ToTStr(_LastVegetSetName), OFN_OVERWRITEPROMPT, _T("VegetSetFiles (*.vegetset)|*.vegetset|All Files (*.*)|*.*||"), this);
	fd.m_ofn.lpstrTitle = _T("Save Vegetable Set");
	if (fd.DoModal() == IDOK)
	{
		NLMISC::COFile	f;
		
		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			try
			{
				// save the vegetable set
				f.serial(vegetSet);
			}
			catch(const NLMISC::EStream &)
			{
				MessageBox(_T("Failed to save file!"));
			}
		}
		else
		{
			MessageBox(_T("Failed to open file for write!"));
		}
	}
	
}


// ***************************************************************************
void CVegetableDlg::OnButtonVegetableCopy() 
{
	sint	dstid= VegetableList.GetCurSel();
	if(dstid!=LB_ERR)
	{
		CVegetableCopyDlg	dlg(this);

		if(dlg.DoModal()==IDOK)
		{
			sint	srcid= dlg.VegetableSelected;
			if(srcid!=LB_ERR && srcid!=dstid)
			{
				// copy from src to dst
				NL3D::CVegetable	&vegetSrc= *_Vegetables[srcid].Vegetable;
				NL3D::CVegetable	&vegetDst= *_Vegetables[dstid].Vegetable;

				// copy all?
				if(!dlg.SubsetCopy)
				{
					// copy all the vegetable
					vegetDst= vegetSrc;
				}
				else
				{
					// copy mesh part.
					if(dlg.Mesh)
					{
						vegetDst.ShapeName= vegetSrc.ShapeName;
					}
					// Density/dist.
					if(dlg.Distance)
						vegetDst.DistType = vegetSrc.DistType;
					if(dlg.Density)
						vegetDst.Density = vegetSrc.Density;
					if(dlg.MaxDensity)
						vegetDst.MaxDensity = vegetSrc.MaxDensity;
					if(dlg.AngleSetup)
					{
						switch(vegetSrc.getAngleType())
						{
						case NL3D::CVegetable::AngleGround:
							vegetDst.setAngleGround(vegetSrc.getCosAngleMin());
							break;
						case NL3D::CVegetable::AngleCeiling:
							vegetDst.setAngleCeiling(vegetSrc.getCosAngleMax());
							break;
						case NL3D::CVegetable::AngleWall:
							vegetDst.setAngleWall(vegetSrc.getCosAngleMin(), vegetSrc.getCosAngleMax());
							break;
						}
					}
					// Appearance
					if(dlg.BendPhase)
						vegetDst.BendPhase = vegetSrc.BendPhase;
					if(dlg.BendFactor)
						vegetDst.BendFactor = vegetSrc.BendFactor;
					if(dlg.ColorNoise)
						vegetDst.Color.NoiseValue = vegetSrc.Color.NoiseValue;
					if(dlg.ColorSetup)
						vegetDst.Color.Gradients = vegetSrc.Color.Gradients;
					// Scale/Rot
					if(dlg.ScaleXY)
						vegetDst.Sxy = vegetSrc.Sxy;
					if(dlg.ScaleZ)
						vegetDst.Sz = vegetSrc.Sz;
					if(dlg.RotateX)
						vegetDst.Rx = vegetSrc.Rx;
					if(dlg.RotateY)
						vegetDst.Ry = vegetSrc.Ry;
					if(dlg.RotateZ)
						vegetDst.Rz = vegetSrc.Rz;
				}

				// update dst vegetableName according to Vegetable copied
				_Vegetables[dstid].updateVegetableName();


				// deselect, then reselect, to refresh.
				setVegetableToEdit(NULL);
				// set the cur selection
				VegetableList.SetCurSel(dstid);
				// And set the new to edit
				OnSelchangeListVegetable();

				// must also update our name in view
				updateCurSelVegetableName();


				// update 3D view
				refreshVegetableDisplay();
			}
		}
	}
}

void CVegetableDlg::OnButtonVegetableRefresh() 
{
	// refresh view, independently of checkBox
	doRefreshVegetableDisplay();
}

void CVegetableDlg::OnCheckVegetableAutomatic() 
{
	// Enabling the checkBox do a refresh (NB: disabling don't do a refresh).
	refreshVegetableDisplay();
}

void CVegetableDlg::OnCheckVegetableShow() 
{
	if(CheckShowLandscape.GetCheck()==1)
	{
		// Landscape not created ??
		if(!_ObjView->isVegetableLandscapeCreated())
		{
			// if success to create / Load the landscape.
			if(_ObjView->createVegetableLandscape())
			{
				// Enable the refresh button
				ButtonRefreshLandscape.EnableWindow(true);

				// refresh view, independently of checkBox
				doRefreshVegetableDisplay();
			}
			else
			{
				// Failed in load, never retry.
				CheckShowLandscape.SetCheck(0);
				CheckShowLandscape.EnableWindow(false);
			}
		}

		// show the landscape
		_ObjView->showVegetableLandscape();
	}
	else
	{
		_ObjView->hideVegetableLandscape();
	}
}


// ***************************************************************************
void CVegetableDlg::OnCheckVegetableEnable() 
{
	// update view.
	_ObjView->enableLandscapeVegetable(CheckEnableVegetable.GetCheck()==1);
}


// ***************************************************************************
void CVegetableDlg::OnButtonVegetableSetupWind() 
{
	// create the window if necessary
	if(!_VegetableWindDlg)
	{
		_VegetableWindDlg= new CVegetableWindDlg(_ObjView, this);
		_VegetableWindDlg->Create(CVegetableWindDlg::IDD, this);
	}

	// show the window.
	_VegetableWindDlg->ShowWindow(true);
}


// ***************************************************************************
void CVegetableDlg::OnCheckVegetableSnaptoground() 
{
	// update view.
	_ObjView->snapToGroundVegetableLandscape(CheckSnapToGround.GetCheck()==1);
}


// ***************************************************************************
void CVegetableDlg::swapShowHideVegetable (uint id)
{
	if(id>=_Vegetables.size())
		return;

	_Vegetables[id].Visible ^= true;

	// update 3D view
	refreshVegetableDisplay();
}

// ***************************************************************************
void CVegetableDlg::setShowHideVegetable (uint id, bool visible, bool refreshDisplay)
{
	if(id>=_Vegetables.size())
		return;

	_Vegetables[id].Visible= visible;

	// update 3D view
	if(refreshDisplay)
		refreshVegetableDisplay();
}

// ***************************************************************************
bool CVegetableDlg::isVegetableVisible (uint id)
{
	if(id>=_Vegetables.size())
		return true;

	return _Vegetables[id].Visible;
}
