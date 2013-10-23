// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// edit_morph_mesh_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "edit_morph_mesh_dlg.h"
#include "nel/3d/ps_mesh.h"
#include "nel/3d/particle_system_model.h"
//
#include "attrib_dlg.h"
#include "particle_dlg.h"
#include "mesh_dlg.h"

using NL3D::CPSConstraintMesh;


/////////////////////////////////////////////////////////////////////////////
// CEditMorphMeshDlg dialog


CEditMorphMeshDlg::CEditMorphMeshDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSConstraintMesh *cm, CWnd* pParent, CParticleDlg  *particleDlg, IPopupNotify *pn /*= NULL*/)
									: _Node(ownerNode),
									  _PN(pn),
									  _CM(cm),
									  CDialog(CEditMorphMeshDlg::IDD, pParent),
									  _ParticleDlg(particleDlg)
{
	nlassert(cm);
	//{{AFX_DATA_INIT(CEditMorphMeshDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditMorphMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMorphMeshDlg)
	DDX_Control(pDX, IDC_MESHS, m_MeshList);
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CEditMorphMeshDlg, CDialog)
	//{{AFX_MSG_MAP(CEditMorphMeshDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//====================================================================
void CEditMorphMeshDlg::init(CWnd *pParent)
{
	Create(IDD_EDIT_MORPH_MESH, pParent);	
	ShowWindow(SW_SHOW);
}


//====================================================================
bool CEditMorphMeshDlg::getShapeNameFromDlg(std::string &name)
{
	CFileDialog fd(TRUE, ".shape", "*.shape", 0, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		/*
		char drive[256];
		char dir[256];
		char path[256];
		char fname[256];
		char ext[256];
		*/


		// Add search path for the texture
		/*
		_splitpath (fd.GetPathName(), drive, dir, fname, ext);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);
		*/

		name = fd.GetPathName();
		
		return true;
	}
	else
	{
		return false;
	}
}

//====================================================================
void CEditMorphMeshDlg::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{	
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}

//====================================================================
void CEditMorphMeshDlg::OnAdd() 
{
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{		
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes() + 1);
		_CM->getShapesNames(&shapeNames[0]);
		uint index = (uint)shapeNames.size() - 1;
		shapeNames[index] = shapeName;
		_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
		std::vector<sint> numVerts;
		_CM->getShapeNumVerts(numVerts);		
		m_MeshList.AddString(getShapeDescStr(index, numVerts[index]).c_str());
		GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
	}
	touchPSState();
	updateValidFlag();
}

//====================================================================
void CEditMorphMeshDlg::OnRemove() 
{
	UpdateData();
	sint selItem = m_MeshList.GetCurSel();
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	shapeNames.erase(shapeNames.begin() + selItem);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
	if (_CM->getNumShapes() == 2)
	{
		GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
	}	
	touchPSState();
	updateMeshList();
	updateValidFlag();
}

//====================================================================
void CEditMorphMeshDlg::OnInsert() 
{
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{	
		sint selItem = m_MeshList.GetCurSel();
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes());
		_CM->getShapesNames(&shapeNames[0]);
		shapeNames.insert(shapeNames.begin() + selItem, shapeName);
		_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());		
		GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
		touchPSState();
		updateMeshList();
		m_MeshList.SetCurSel(selItem);
	}
	updateValidFlag();
}

//====================================================================
void CEditMorphMeshDlg::OnUp() 
{		
	sint selItem = m_MeshList.GetCurSel();
	if (selItem == 0) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[selItem - 1], shapeNames[selItem]);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());		
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);		
	updateMeshList();
	m_MeshList.SetCurSel(selItem - 1);
	updateValidFlag();
}

//====================================================================
void CEditMorphMeshDlg::OnDown() 
{
	sint selItem = m_MeshList.GetCurSel();
	if (selItem == (sint) (_CM->getNumShapes() - 1)) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[selItem + 1], shapeNames[selItem]);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());		
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);		
	updateMeshList();
	m_MeshList.SetCurSel(selItem + 1);	
	updateValidFlag();
}

//====================================================================
void CEditMorphMeshDlg::OnChange() 
{
	UpdateData();
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{			
		sint selItem = m_MeshList.GetCurSel();
		_CM->setShape(selItem, shapeName);	
		updateMeshList();
		touchPSState();
	}
	updateValidFlag();
}

//====================================================================
float CEditMorphMeshDlg::CMorphSchemeWrapper::get(void) const
{
	nlassert(CM);
	return CM->getMorphValue();
}	

//====================================================================	
void CEditMorphMeshDlg::CMorphSchemeWrapper::set(const float &v)
{
	nlassert(CM);
	CM->setMorphValue(v);
}

//====================================================================
CEditMorphMeshDlg::CMorphSchemeWrapper::scheme_type *CEditMorphMeshDlg::CMorphSchemeWrapper::getScheme(void) const
{
	nlassert(CM);
	return CM->getMorphScheme();
}

//====================================================================
void CEditMorphMeshDlg::CMorphSchemeWrapper::setScheme(scheme_type *s)
{
	nlassert(CM);
	CM->setMorphScheme(s);
}

//====================================================================
void CEditMorphMeshDlg::updateMeshList()
{
	nlassert(_CM);
	std::vector<sint> numVerts;
	_CM->getShapeNumVerts(numVerts);
	m_MeshList.ResetContent();
	for (uint k = 0; k < _CM->getNumShapes(); ++k)
	{	
		m_MeshList.AddString(getShapeDescStr(k, numVerts[k]).c_str());		
	}
	m_MeshList.SetCurSel(0);
	updateValidFlag();
	UpdateData(FALSE);
}

//====================================================================
BOOL CEditMorphMeshDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	updateMeshList(); // fill the list box with the neames of the meshs

	/// create the morph scheme edition dialog
	RECT r;
	CAttribDlgFloat *mvd = new CAttribDlgFloat("MORPH_VALUE", _Node);
	_MorphSchemeWrapper.CM = _CM;
	mvd->setWrapper(&_MorphSchemeWrapper);	
	mvd->setSchemeWrapper(&_MorphSchemeWrapper);
	GetDlgItem(IDC_MORPH_SCHEME)->GetWindowRect(&r);
	ScreenToClient(&r);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_MORPH_SCHEME));
	mvd->init(bmh, r.left, r.top, this);	
	pushWnd(mvd);
	if (_CM->getNumShapes() == 2)
	{
		GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
	}	
	updateValidFlag();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//====================================================================
void CEditMorphMeshDlg::OnClose() 
{
	CDialog::OnClose();
	if (_PN) _PN->childPopupClosed(this);	
}

//====================================================================
void CEditMorphMeshDlg::updateValidFlag()
{
	nlassert(_CM);
	int cmdShow = _CM->isValidBuild() ? SW_HIDE : SW_SHOW;
	GetDlgItem(IDC_INVALID_BUILD)->ShowWindow(cmdShow);
}

//====================================================================
std::string CEditMorphMeshDlg::getShapeDescStr(uint shapeIndex, sint numVerts) const
{
	if (numVerts >= 0)
	{	
		CString verts;
		verts.LoadString(IDS_VERTICES);
		std::string msg = _CM->getShape(shapeIndex) + " (" + NLMISC::toString(numVerts) + " " + (LPCTSTR) verts + ")";
		return msg;
	}
	else
	{		
		std::string result =  _CM->getShape(shapeIndex) + " (" + (LPCTSTR) CMeshDlg::getShapeErrorString(numVerts) + ")";
		return result;
	}
}


