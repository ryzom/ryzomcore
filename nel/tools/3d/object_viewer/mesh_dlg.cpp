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
#include "mesh_dlg.h"
#include "edit_morph_mesh_dlg.h"
//
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_mesh.h"
#include "nel/3d/particle_system_model.h"
//
#include "nel/misc/path.h"

///==================================================================
CMeshDlg::CMeshDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSShapeParticle *sp, CParticleDlg  *particleDlg)
	: _Node(ownerNode), _ShapeParticle(sp), _EMMD(NULL), _ParticleDlg(particleDlg)
{
	//{{AFX_DATA_INIT(CMeshDlg)
	//}}AFX_DATA_INIT
}

CMeshDlg::~CMeshDlg()
{
	if (_EMMD)
	{
		_EMMD->DestroyWindow();
		delete _EMMD;
		_EMMD = NULL;
	}
}

///==================================================================
void CMeshDlg::init(CWnd *pParent, sint x, sint y)
{
	Create(IDD_CHOOSE_MESH, pParent);
	RECT r;
	GetClientRect(&r);
	r.top += y; r.bottom += y;
	r.right += x; r.left += x;
	MoveWindow(&r);	

	ShowWindow(SW_SHOW);
}


///==================================================================
void CMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMeshDlg)
	DDX_Control(pDX, IDC_MESH_ERROR, m_MeshErrorMsg);
	DDX_Text(pDX, IDC_SHAPE_NAME, m_ShapeName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMeshDlg, CDialog)
	//{{AFX_MSG_MAP(CMeshDlg)
	ON_BN_CLICKED(IDC_BROWSE_SHAPE, OnBrowseShape)
	ON_BN_CLICKED(IDC_ENABLE_MORPHING, OnEnableMorphing)
	ON_BN_CLICKED(IDC_EDIT_MORPH, OnEditMorph)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///==================================================================
void CMeshDlg::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{	
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}

///==================================================================
void CMeshDlg::OnBrowseShape() 
{
	
	CFileDialog fd(TRUE, _T(".shape"), _T("*.shape"), 0, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		std::string fullPath = NLMISC::tStrToUtf8(fd.GetPathName());
		std::string fname = NLMISC::CFile::getFilenameWithoutExtension(fullPath);
		std::string ext = NLMISC::CFile::getExtension(fullPath);

		// Add search path for the texture
		NLMISC::CPath::addSearchPath (NLMISC::CFile::getPath(fullPath));

		try
		{		
			_ShapeParticle->setShape(fname + "." + ext);		
			m_ShapeName = nlUtf8ToTStr(fname + "." + ext);
			touchPSState();			
		}
		catch (const NLMISC::Exception &e)
		{
			MessageBox(nlUtf8ToTStr(e.what()), _T("shape loading error"));
		}		

		updateMeshErrorString();
	}

	UpdateData(FALSE);
}


///==================================================================
BOOL CMeshDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	if (!dynamic_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle))
	{
		// hide the unused fields
		GetDlgItem(IDC_ENABLE_MORPHING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_MORPH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MORPH_FRAME)->ShowWindow(SW_HIDE);
		m_ShapeName = _ShapeParticle->getShape().c_str();
		UpdateData(FALSE);
	}
	else
	{		
		NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
		if (cm->getNumShapes() > 1)
		{
			((CButton *) GetDlgItem(IDC_ENABLE_MORPHING))->SetCheck(TRUE);
		}
		updateForMorph();
	}
	m_MeshErrorMsg.SetTextColor(RGB(255,  0,  0));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

///==================================================================
void CMeshDlg::updateForMorph()
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (cm)
	{
		BOOL enable = cm->getNumShapes() > 1;						
		GetDlgItem(IDC_EDIT_MORPH)->EnableWindow(enable);
		GetDlgItem(IDC_BROWSE_SHAPE)->EnableWindow(!enable);
		GetDlgItem(IDC_SHAPE_NAME)->EnableWindow(!enable);
		if (!enable)
		{
			m_ShapeName = nlUtf8ToTStr(cm->getShape());
		}
		else
		{
			m_ShapeName.Empty();
		}
	}
	updateMeshErrorString();
	UpdateData(FALSE);
}

///==================================================================
void CMeshDlg::OnEnableMorphing() 
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (((CButton *) GetDlgItem(IDC_ENABLE_MORPHING))->GetCheck())
	{
		// morphing enabled..		
		std::string currName[2] = { cm->getShape(), cm->getShape()};
		cm->setShapes(currName, 2);		
	}
	else
	{
		// morphing disabled
		std::string currName = cm->getShape(0);
		cm->setShape(currName);
	}
	updateForMorph();
	updateModifiedFlag();
	updateMeshErrorString();
}

///==================================================================
void CMeshDlg::OnEditMorph() 
{
	nlassert(_EMMD == NULL);
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	EnableWindow(FALSE);
	_EMMD = new CEditMorphMeshDlg(_Node, cm, this, _ParticleDlg, this);
	_EMMD->init(this);
}

///==================================================================
void CMeshDlg::childPopupClosed(CWnd *child)
{
	nlassert(_EMMD == child);
	_EMMD->DestroyWindow();
	delete _EMMD;
	_EMMD = NULL;
	EnableWindow(TRUE);
	updateMeshErrorString();
}


///==================================================================
BOOL CMeshDlg::EnableWindow( BOOL bEnable)
{
	if (!bEnable)
	{
		GetDlgItem(IDC_EDIT_MORPH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BROWSE_SHAPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENABLE_MORPHING)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHAPE_NAME)->EnableWindow(FALSE);	
	}
	else
	{
		GetDlgItem(IDC_ENABLE_MORPHING)->EnableWindow(TRUE);
		updateForMorph();
	}
	updateMeshErrorString();
	return CDialog::EnableWindow(bEnable);
}

///==================================================================
void CMeshDlg::updateMeshErrorString()
{
	GetDlgItem(IDC_MESH_ERROR)->SetWindowText(_T(""));
	NL3D::CPSConstraintMesh *cm = dynamic_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (!cm) return;
	std::vector<sint> numVerts;
	cm->getShapeNumVerts(numVerts);
	if (numVerts.empty()) return;
	if (numVerts.size() == 1)
	{
		GetDlgItem(IDC_MESH_ERROR)->SetWindowText((LPCTSTR) getShapeErrorString(numVerts[0]));
	}
	else
	{
		// display error msg for morphed meshs
		bool hasError = false;
		for(uint k = 0; k < numVerts.size(); ++k)
		{
			if (numVerts[k] < 0)
			{
				hasError = true;
				break;
			}
		}
		if (hasError)
		{
			CString errorInMorphMesh;
			errorInMorphMesh.LoadString(IDS_ERROR_IN_MORPH_MESH);
			GetDlgItem(IDC_MESH_ERROR)->SetWindowText((LPCTSTR) errorInMorphMesh);
		}
	}
}

//====================================================================
CString CMeshDlg::getShapeErrorString(sint errorCode)
{
	CString str;
	switch(errorCode)
	{
		case NL3D::CPSConstraintMesh::ShapeFileIsNotAMesh: str.LoadString(IDS_SHAPE_FILE_NOT_MESH); break;
		case NL3D::CPSConstraintMesh::ShapeFileNotLoaded: str.LoadString(IDS_SHAPE_NOT_LOADED); break;
		case NL3D::CPSConstraintMesh::ShapeHasTooMuchVertices: str.LoadString(IDS_TOO_MUCH_VERTICES); break;
		default:
		break;
	};
	return str;
}



