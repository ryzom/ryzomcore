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
#include "located_bindable_dialog.h"



#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_mesh.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_zone.h"
#include "nel/3d/particle_system_model.h"



#include "texture_chooser.h"
#include "attrib_dlg.h"
#include "precomputed_rotations_dlg.h"
#include "tail_particle_dlg.h"
#include "mesh_dlg.h"
#include "texture_anim_dlg.h"
#include "particle_dlg.h"
#include "constraint_mesh_dlg.h"
#include "constraint_mesh_tex_dlg.h"
#include "ribbon_dlg.h"


using NL3D::CPSLocatedBindable;


/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog


//***********************************************************************************
CLocatedBindableDialog::CLocatedBindableDialog(CParticleWorkspace::CNode *ownerNode, NL3D::CPSLocatedBindable *bindable)
	: _Node(ownerNode), _Bindable(bindable), _SizeCtrl(NULL)
{
	//{{AFX_DATA_INIT(CLocatedBindableDialog)
	m_IndependantSizes = FALSE;
	//}}AFX_DATA_INIT
}

/// dtor
//***********************************************************************************
CLocatedBindableDialog::~CLocatedBindableDialog()
{
	if (_SizeCtrl)
	{
		_SizeCtrl->DestroyWindow();
		delete _SizeCtrl;
	}
}

//***********************************************************************************
void CLocatedBindableDialog::init(CParticleDlg* pParent)
{
	Create(IDD_LOCATED_BINDABLE, pParent);
	ShowWindow(SW_SHOW);
	_ParticleDlg = pParent;

	NL3D::CParticleSystem *ps = _Bindable->getOwner()->getOwner();
	if (ps->isSharingEnabled())
	{
		GetDlgItem(IDC_NO_AUTO_LOD)->ShowWindow(TRUE);
		if (ps->isAutoLODEnabled() == false)
		{
			GetDlgItem(IDC_NO_AUTO_LOD)->EnableWindow(FALSE);
		}
		else
		{
			((CButton *) GetDlgItem(IDC_NO_AUTO_LOD))->SetCheck(NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->isAutoLODDisabled());
		}
	}
	else
	{
		GetDlgItem(IDC_NO_AUTO_LOD)->ShowWindow(FALSE);
	}

	uint yPos = 60;
	const uint xPos = 5;
	RECT rect;


	// control at the top of the sheet are not available for meshs & constraint meshes, so use that extra space
	if (dynamic_cast<NL3D::CPSMesh *>(_Bindable) || dynamic_cast<NL3D::CPSConstraintMesh *>(_Bindable))
	{
		yPos = 0;
	}

	// has the particle a material ?
	if (dynamic_cast<NL3D::CPSMaterial *>(_Bindable))
	{
		NL3D::CPSMaterial *material = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
		// blending mode
		m_BlendingMode.SetCurSel((uint) material->getBlendingMode() );
		// z-test
		((CButton *) GetDlgItem(IDC_ZTEST))->SetCheck(material->isZTestEnabled() ? BST_CHECKED : BST_UNCHECKED);
		// z-bias
		GetDlgItem(IDC_ZBIAS)->SetWindowText(nlUtf8ToTStr(NLMISC::toString("%.2f", -material->getZBias())));
	}
	else
	{
		m_BlendingMode.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BLENDING_MODE_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ZTEST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ZBIAS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ZBIAS_TEXT)->ShowWindow(SW_HIDE);
	}
	GetDlgItem(IDC_ALIGN_ON_MOTION)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_ZALIGN)->ShowWindow(SW_HIDE);
	// enable disable z-test
	//
	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable;

		// check support for lighting
		if (p->supportGlobalColorLighting())
		{
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->ShowWindow(SW_SHOW);
			// if global color lighting is forced for all objects, don't allow to modify
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->EnableWindow(ps->getForceGlobalColorLightingFlag() ? FALSE : TRUE);
			((CButton *) GetDlgItem(ID_GLOBAL_COLOR_LIGHTING))->SetCheck(p->usesGlobalColorLighting() ? 1 : 0);
		}
		else
		{
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->ShowWindow(SW_HIDE);
		}
		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{

			CAttribDlgRGBA *ad = new CAttribDlgRGBA("PARTICLE_COLOR", _Node);
			pushWnd(ad);

			_ColorWrapper.S = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable);
			ad->setWrapper(&_ColorWrapper);
			ad->setSchemeWrapper(&_ColorWrapper);

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR));
			ad->init(bmh, xPos, yPos, this);
			ad->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// init size ctrl
		_SizeCtrlX = xPos;
		_SizeCtrlY = yPos;
		yPos += updateSizeControl();

		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{

			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_ANGLE2D", _Node, 0.f, 256.f);
			pushWnd(ad);

			_Angle2DWrapper.S = dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable);
			ad->setWrapper(&_Angle2DWrapper);
			ad->setSchemeWrapper(&_Angle2DWrapper);

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_ANGLE));
			ad->init(bmh, xPos, yPos, this);
			ad->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}



		CAttribDlgPlaneBasis *pb = NULL;

		// check support for plane basis
		if (dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable))
		{
			pb = new CAttribDlgPlaneBasis("PARTICLE_PLANE_BASIS", _Node);
			pushWnd(pb);
			_PlaneBasisWrapper.S = dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable);
			pb->setWrapper(&_PlaneBasisWrapper);
			pb->setSchemeWrapper(&_PlaneBasisWrapper);
			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BASIS));
			pb->init(bmh, xPos, yPos, this);
			pb->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}

		// check support for precomputed rotations
		if (dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable))
		{
			CPrecomputedRotationsDlg *pr = new CPrecomputedRotationsDlg(_Node, dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable), pb);
			pushWnd(pr);
			pr->init(this, xPos, yPos);
			pr->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// if we're dealing with a face look at, motion blur can be tuned
		if (dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable))
		{
			NL3D::CPSFaceLookAt *fla = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			CEditableRangeFloat *mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_COEFF"), _Node, 0, 5);
			pushWnd(mbc);
			_MotionBlurWnd.push_back(mbc);
			_MotionBlurCoeffWrapper.P = fla;
			mbc->setWrapper(&_MotionBlurCoeffWrapper);
			mbc->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			_MotionBlurWnd.push_back(s);
			s->Create(_T("Fake motion blur coeff."), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);
			mbc->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_THRESHOLD"), _Node, 0, 5);
			pushWnd(mbc);
			_MotionBlurWnd.push_back(mbc);
			_MotionBlurThresholdWrapper.P = fla;
			mbc->setWrapper(&_MotionBlurThresholdWrapper);
			mbc->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			_MotionBlurWnd.push_back(s);
			s->Create(_T("Fake motion blur threshold."), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);
			mbc->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			GetDlgItem(IDC_ALIGN_ON_MOTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ZALIGN)->ShowWindow(SW_SHOW);
			((CButton *) GetDlgItem(IDC_ALIGN_ON_MOTION))->SetCheck(fla->getAlignOnMotion());
			((CButton *) GetDlgItem(IDC_ZALIGN))->SetCheck(fla->getAlignOnZAxis());
			updateValidWndForAlignOnMotion(fla->getAlignOnMotion());
		}

		// if we're dealing with a shockwave, we add dlg for the radius cut, and the number of segments
		if (dynamic_cast<NL3D::CPSShockWave *>(_Bindable))
		{
			NL3D::CPSShockWave *sw = static_cast<NL3D::CPSShockWave *>(_Bindable);
			CEditableRangeFloat *rc = new CEditableRangeFloat(std::string("RADIUS CUT"), _Node, 0, 1);
			pushWnd(rc);
			_RadiusCutWrapper.S = sw;
			rc->setWrapper(&_RadiusCutWrapper);
			rc->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			s->Create(_T("Radius cut."), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);


			rc->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CEditableRangeUInt *snbs = new CEditableRangeUInt(std::string("SHOCK WAVE NB SEG"), _Node, 3, 24);
			pushWnd(snbs);
			_ShockWaveNbSegWrapper.S = sw;
			snbs->enableLowerBound(3, false);
			snbs->setWrapper(&_ShockWaveNbSegWrapper);
			snbs->init(xPos + 140, yPos, this);

			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Nb segs"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->ShowWindow(SW_SHOW);

			snbs->GetClientRect(&rect);
			yPos += rect.bottom + 3;


			CEditableRangeFloat *uvd = new CEditableRangeFloat(std::string("TEX UFACTOR"), _Node, 0, 5);
			pushWnd(uvd);
			_ShockWaveUFactorWrapper.S = sw;
			uvd->setWrapper(&_ShockWaveUFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Texture U factor :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}

		// fanlight
		if (dynamic_cast<NL3D::CPSFanLight *>(_Bindable))
		{
			CEditableRangeUInt *nbf = new CEditableRangeUInt(std::string("NB_FANS"), _Node, 3, 127);
			pushWnd(nbf);
			nbf->enableLowerBound(3, false);
			nbf->enableUpperBound(128, true);
			_FanLightWrapper.P = dynamic_cast<NL3D::CPSFanLight *>(_Bindable);
			nbf->setWrapper(&_FanLightWrapper);
			nbf->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			s->Create(_T("Nb fan lights :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			nbf = new CEditableRangeUInt(std::string("PHASE_SMOOTHNESS"), _Node, 0, 31);
			pushWnd(nbf);
			nbf->enableUpperBound(32, true);
			_FanLightSmoothnessWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbf->setWrapper(&_FanLightSmoothnessWrapper);
			nbf->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Phase smoothnes:"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CEditableRangeFloat *nbfp = new CEditableRangeFloat(std::string("FAN_LIGHT_PHASE"), _Node, 0, 4.f);
			pushWnd(nbfp);
			_FanLightPhaseWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbfp->setWrapper(&_FanLightPhaseWrapper);
			nbfp->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Fan light speed :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			nbfp = new CEditableRangeFloat(std::string("FAN_LIGHT_INTENSITY"), _Node, 0, 4.f);
			pushWnd(nbfp);
			_FanLightIntensityWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbfp->setWrapper(&_FanLightIntensityWrapper);
			nbfp->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Fan light intensity:"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}


		// tail particle
		if (dynamic_cast<NL3D::CPSTailParticle *>(_Bindable))
		{
			CEditableRangeUInt *nbs;
			if (!dynamic_cast<NL3D::CPSRibbonLookAt *>(_Bindable))
			{
				nbs = new CEditableRangeUInt(std::string("TAIL_NB_SEGS_"), _Node, 2, 16);
				nbs->enableLowerBound(1, true);
			}
			else
			{
				nbs = new CEditableRangeUInt(std::string("LOOKAT_RIBBON_TAIL_NB_SEGS_"), _Node, 2, 16);
				nbs->enableLowerBound(1, true);
			}

			pushWnd(nbs);

			if (dynamic_cast<NL3D::CPSTailDot *>(_Bindable))
			{
				nbs->enableUpperBound(256, true);
			}


			_TailParticleWrapper.P = dynamic_cast<NL3D::CPSTailParticle *>(_Bindable);
			nbs->setWrapper(&_TailParticleWrapper);
			nbs->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			s->Create(_T("Nb segs :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);

			nbs->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CTailParticleDlg *tpd = new CTailParticleDlg(_Node, dynamic_cast<NL3D::CPSTailParticle *>(_Bindable));
			pushWnd(tpd);
			tpd->init(this, xPos, yPos);

			tpd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}


		// shape particle
		if (dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable))
		{
			CMeshDlg *md = new CMeshDlg(_Node, dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable), _ParticleDlg);
			md->init(this, xPos, yPos);
			md->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			pushWnd(md);
		}

		// constraint mesh particle
		if (dynamic_cast<NL3D::CPSConstraintMesh *>(_Bindable))
		{
			CConstraintMeshDlg *cmd = new CConstraintMeshDlg(static_cast<NL3D::CPSConstraintMesh *>(_Bindable));
			cmd->init(xPos, yPos, this);
			cmd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			CConstraintMeshTexDlg *cmtd = new CConstraintMeshTexDlg(static_cast<NL3D::CPSConstraintMesh *>(_Bindable),
																	this);
			cmtd->init(xPos, yPos, this);
			cmtd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			pushWnd(cmd);
			pushWnd(cmtd);
		}



		// check support for animated texture
		if (dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable))
		{
			CTextureAnimDlg *td = new CTextureAnimDlg(_Node,
													  dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable),
													  dynamic_cast<NL3D::CPSMultiTexturedParticle *>(_Bindable)
													 );
			pushWnd(td);

			td->init(xPos, yPos, this);
			td->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// unanimated texture
		if (dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable))
		{
			NL3D::CPSTexturedParticleNoAnim *tp = dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable);
			_TextureNoAnimWrapper.TP = tp;
			CTextureChooser *tc = new CTextureChooser(dynamic_cast<NL3D::CPSMultiTexturedParticle *>(_Bindable), _Node);
			tc->enableRemoveButton();
			tc->setWrapper(&_TextureNoAnimWrapper);
			pushWnd(tc);

			tc->init(xPos, yPos, this);
			tc->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// ribbon texture (doesn't support texture animation for now)
		if (dynamic_cast<NL3D::CPSRibbon *>(_Bindable))
		{

			// add dialog for uv tuning with ribbon
			CEditableRangeFloat *uvd = new CEditableRangeFloat(std::string("RIBBON UFACTOR"), _Node, 0, 5);
			pushWnd(uvd);
			_RibbonUFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			uvd->setWrapper(&_RibbonUFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			s->Create(_T("Texture U factor :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			uvd = new CEditableRangeFloat(std::string("RIBBON VFACTOR"), _Node, 0, 5);
			pushWnd(uvd);
			_RibbonVFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			uvd->setWrapper(&_RibbonVFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			s = new CStatic;
			pushWnd(s);
			s->Create(_T("Texture V factor :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}

		if (dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable))
		{

			// add dialog for uv tuning with ribbon
			CEditableRangeFloat *sd = new CEditableRangeFloat(std::string("SEGMENT DURATION"), _Node, 0.05f, 0.5f);
			sd->enableLowerBound(0, true);
			pushWnd(sd);
			_SegDurationWrapper.R = static_cast<NL3D::CPSRibbonLookAt *>(_Bindable);
			sd->setWrapper(&_SegDurationWrapper);
			sd->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;
			pushWnd(s);
			s->Create(_T("Seg Duration :"), SS_LEFT, CRect(xPos, yPos + 16, xPos + 139, yPos + 48), this);
			s->SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
			s->ShowWindow(SW_SHOW);
			sd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// 'look at' independant sizes
		bool isLookAt = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable) != NULL;
		GetDlgItem(IDC_INDE_SIZES)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SIZE_WIDTH)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SIZE_HEIGHT)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_WIDTH_HEIGHT_BOX)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		((CButton *) GetDlgItem(IDC_SIZE_WIDTH))->SetCheck(1);
		if (isLookAt)
		{
			NL3D::CPSFaceLookAt *la = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			m_IndependantSizes = la->hasIndependantSizes();
			((CButton *) GetDlgItem(IDC_INDE_SIZES))->SetCheck(m_IndependantSizes);
			updateIndependantSizes();
		}

		/// new ribbon base class
		if (dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable))
		{
			CRibbonDlg *rd = new CRibbonDlg(_Node, static_cast<NL3D::CPSRibbonBase *>(_Bindable),
											this);
			rd->init(this, xPos, yPos);
			pushWnd(rd);
			rd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}
	}
	UpdateData();
}


//***********************************************************************************
void CLocatedBindableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedBindableDialog)
	DDX_Control(pDX, IDC_ZBIAS, m_ZBias);
	DDX_Control(pDX, IDC_BLENDING_MODE, m_BlendingMode);
	DDX_Check(pDX, IDC_INDE_SIZES, m_IndependantSizes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedBindableDialog, CDialog)
	//{{AFX_MSG_MAP(CLocatedBindableDialog)
	ON_CBN_SELCHANGE(IDC_BLENDING_MODE, OnSelchangeBlendingMode)
	ON_BN_CLICKED(IDC_INDE_SIZES, OnIndeSizes)
	ON_BN_CLICKED(IDC_SIZE_WIDTH, OnSizeWidth)
	ON_BN_CLICKED(IDC_SIZE_HEIGHT, OnSizeHeight)
	ON_BN_CLICKED(IDC_NO_AUTO_LOD, OnNoAutoLod)
	ON_BN_CLICKED(ID_GLOBAL_COLOR_LIGHTING, OnGlobalColorLighting)
	ON_BN_CLICKED(IDC_ALIGN_ON_MOTION, OnAlignOnMotion)
	ON_BN_CLICKED(IDC_ZTEST, OnZtest)
	ON_EN_CHANGE(IDC_ZBIAS, OnChangeZbias)
	ON_EN_KILLFOCUS(IDC_ZBIAS, OnKillfocusZbias)
	ON_BN_CLICKED(IDC_ZALIGN, OnZalign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog message handlers

//====================================================================
void CLocatedBindableDialog::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}


//***********************************************************************************
void CLocatedBindableDialog::OnSelchangeBlendingMode()
{
	UpdateData();
	NL3D::CPSMaterial *m = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
	nlassert(m);
	m->setBlendingMode( (NL3D::CPSMaterial::TBlendingMode) m_BlendingMode.GetCurSel());
	touchPSState();
	updateModifiedFlag();
}


//***********************************************************************************
void CLocatedBindableDialog::updateIndependantSizes()
{
	UpdateData();
	// make sure we are dealing with 'LookAt' for now
	nlassert(dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable));

	GetDlgItem(IDC_SIZE_WIDTH)->EnableWindow(m_IndependantSizes);
	GetDlgItem(IDC_SIZE_HEIGHT)->EnableWindow(m_IndependantSizes);
}


//***********************************************************************************
// user asked for independant sizes
void CLocatedBindableDialog::OnIndeSizes()
{
	UpdateData();
	// make sure we are dealing with 'LookAt' for now
	nlassert(dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable));
	NL3D::CPSFaceLookAt *la = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	la->setIndependantSizes(m_IndependantSizes ? true : false /* VCC warning*/);
	updateIndependantSizes();
	updateSizeControl();
	updateModifiedFlag();
}

//***********************************************************************************
uint CLocatedBindableDialog::updateSizeControl()
{
	HBITMAP bmh;
	if (!dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable)) return 0;
	// if a previous control was there, remove it
	if (_SizeCtrl)
	{
		_SizeCtrl->DestroyWindow();
		delete _SizeCtrl;
		_SizeCtrl = NULL;
	}

	NL3D::CPSFaceLookAt *fla = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	// LookAt case
	if (fla && fla->hasIndependantSizes())
	{
		int editWidth = (((CButton *) GetDlgItem(IDC_SIZE_WIDTH))->GetCheck());
		_SizeCtrl = new CAttribDlgFloat(editWidth ? "PARTICLE_WIDTH" : "PARTICLE_HEIGHT", _Node, 0.f, 1.f);


		if (editWidth) // wrap to the wanted size
		{
			_SizeWrapper.S = fla;
		}
		else
		{
			_SizeWrapper.S = &fla->getSecondSize();
		}
		_SizeCtrl->setWrapper(&_SizeWrapper);
		_SizeCtrl->setSchemeWrapper(&_SizeWrapper);

		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(editWidth ? IDB_PARTICLE_WIDTH : IDB_PARTICLE_HEIGHT));
	}
	else // general case. Wrap to the size interface and the appropriate dialog
	{
		_SizeCtrl = new CAttribDlgFloat("PARTICLE_SIZE", _Node, 0.f, 1.f);

		_SizeWrapper.S = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable);
		_SizeCtrl->setWrapper(&_SizeWrapper);
		_SizeCtrl->setSchemeWrapper(&_SizeWrapper);

		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_SIZE));
	}
	RECT rect;
	_SizeCtrl->init(bmh, _SizeCtrlX, _SizeCtrlY, this);
	_SizeCtrl->GetClientRect(&rect);
	return rect.bottom + 3;
}

//***********************************************************************************
void CLocatedBindableDialog::OnSizeWidth()
{
	updateSizeControl();
}

//***********************************************************************************
void CLocatedBindableDialog::OnSizeHeight()
{
	updateSizeControl();
}

//***********************************************************************************
void CLocatedBindableDialog::OnNoAutoLod()
{
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	p->disableAutoLOD(((CButton *) GetDlgItem(IDC_NO_AUTO_LOD))->GetCheck() != 0);
	updateModifiedFlag();
}

//***********************************************************************************
void CLocatedBindableDialog::OnGlobalColorLighting()
{
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	p->enableGlobalColorLighting(((CButton *) GetDlgItem(ID_GLOBAL_COLOR_LIGHTING))->GetCheck() == 1);
	touchPSState();
	updateModifiedFlag();
}

//***********************************************************************************
void CLocatedBindableDialog::OnAlignOnMotion()
{
	bool align = ((CButton *) GetDlgItem(IDC_ALIGN_ON_MOTION))->GetCheck() != 0;
	NL3D::CPSFaceLookAt *fla = NLMISC::safe_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	fla->setAlignOnMotion(align);
	updateValidWndForAlignOnMotion(align);
	updateModifiedFlag();
}

//***********************************************************************************
void CLocatedBindableDialog::updateValidWndForAlignOnMotion(bool align)
{
	BOOL enable = align ? FALSE : TRUE;
	for(uint k = 0; k < _MotionBlurWnd.size(); ++k)
	{
		CEditAttribDlg *ead = dynamic_cast<CEditAttribDlg *>(_MotionBlurWnd[k]);
		if (ead)
		{
			ead->EnableWindow(enable); // enable window not virtual in CWnd ...
		}
		else
		{
			_MotionBlurWnd[k]->EnableWindow(enable);
		}
	}
	GetDlgItem(IDC_ZALIGN)->EnableWindow(enable);
}

//***********************************************************************************
void CLocatedBindableDialog::OnZtest()
{
	UpdateData();
	NL3D::CPSMaterial *mat = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
	nlassert(mat);
	mat->enableZTest(((CButton *) GetDlgItem(IDC_ZTEST))->GetCheck() != BST_UNCHECKED);
	updateModifiedFlag();
}

// TODO: factor this code
static	void concatEdit2Lines(CEdit &edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	TCHAR	tmp0[2*lineLen];
	TCHAR	tmp1[lineLen];
	n= edit.GetLine(0, tmp0, lineLen);	tmp0[n]= 0;
	n= edit.GetLine(1, tmp1, lineLen);	tmp1[n]= 0;
	// concat and update the CEdit.
	edit.SetWindowText(_tcscat(tmp0, tmp1));
}

//***************************************************************************************************************************
void CLocatedBindableDialog::updateZBias()
{
	CString value;
	m_ZBias.GetWindowText(value);
	float zbias = 0.f;
	int dummy; // to test if end of string as no not wanted extra characters
	if (_stscanf((LPCTSTR)(value + _T("\n0")), _T("%f\n%d"), &zbias, &dummy) == 2)
	{
		NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->setZBias(-zbias);
	}
	else
	{
		CString caption;
		CString mess;
		caption.LoadString(IDS_CAPTION_ERROR);
		mess.LoadString(IDS_BAD_ZBIAS);
		m_ZBias.SetWindowText(_T("0.00"));
		MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_ICONERROR);
		NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->setZBias(0);
		updateModifiedFlag();
	}
}

//***************************************************************************************************************************
void CLocatedBindableDialog::OnChangeZbias()
{
	UpdateData();
	// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
	// user has press enter.
	if(m_ZBias.GetLineCount()>1)
	{
		// must ccat 2 lines of the CEdit.
		concatEdit2Lines(m_ZBias);
		updateZBias();
	}
}

//***************************************************************************************************************************
void CLocatedBindableDialog::OnKillfocusZbias()
{
	updateZBias();
}

//***************************************************************************************************************************
void CLocatedBindableDialog::OnZalign()
{
	bool align = ((CButton *) GetDlgItem(IDC_ZALIGN))->GetCheck() != 0;
	NL3D::CPSFaceLookAt *fla = NLMISC::safe_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	fla->setAlignOnZAxis(align);
	updateModifiedFlag();
}
