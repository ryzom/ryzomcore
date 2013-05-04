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


#include "std_afx.h"
#include "object_viewer.h"
#include "texture_anim_dlg.h"
#include "attrib_dlg.h"
#include "texture_chooser.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/texture_grouped.h"
#include "nel/3d/texture_file.h"


#include "nel/misc/smart_ptr.h"
#include "multi_tex_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CTextureAnimDlg dialog


CTextureAnimDlg::CTextureAnimDlg(CParticleWorkspace::CNode *ownerNode, 
								 NL3D::CPSTexturedParticle *p,
								 NL3D::CPSMultiTexturedParticle *mtp /*= NULL*/) 
								: _Node(ownerNode),
								  _EditedParticle(p),
								  _TextureChooser(NULL),
								  _TextureIndexDialog(NULL),
								  _MTP(mtp),
								  _MultiTexDlg(NULL)
{	
	nlassert(p);
	//{{AFX_DATA_INIT(CTextureAnimDlg)	
	m_EnableTextureAnim = p->getTextureGroup() ? TRUE : FALSE;
	m_MultiTexEnable = FALSE;
	//}}AFX_DATA_INIT
}

CTextureAnimDlg::~CTextureAnimDlg()
{	
	if (_MultiTexDlg)
	{
		_MultiTexDlg->DestroyWindow();
		delete _MultiTexDlg;
	}
	cleanCtrl();	
}



BOOL CTextureAnimDlg::EnableWindow( BOOL bEnable)
{
	if (_TextureChooser) _TextureChooser->EnableWindow(bEnable);	
	GetDlgItem(IDC_CHOOSE_TEXTURES)->EnableWindow(bEnable & m_EnableTextureAnim);
	GetDlgItem(IDC_ENABLE_TEXTURE_ANIM)->EnableWindow(bEnable);
	if (_MTP)
	{		
		((CButton *) GetDlgItem(IDC_MULTITEX))->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_MULTITEX)->EnableWindow(bEnable & (_MTP->isMultiTextureEnabled() ? 1 : 0));			
	}
	if (_TextureIndexDialog)
	{
		_TextureIndexDialog->EnableWindow(bEnable);
	}
	return CDialog::EnableWindow(bEnable);
}

void CTextureAnimDlg::init(sint x, sint y, CWnd *pParent)
{
	Create(IDD_TEXTURE_ANIM, pParent); 
	RECT r;
	GetClientRect(&r);
	MoveWindow(x, y, x + r.right, y + r.bottom);
	setupCtrl();
	if (!_MTP)
	{
		GetDlgItem(IDC_MULTITEX)->ShowWindow(FALSE);
		GetDlgItem(IDC_EDIT_MULTITEX)->ShowWindow(FALSE);
		GetDlgItem(IDC_MULTITEX_BORDER)->ShowWindow(FALSE);
	}
	ShowWindow(SW_SHOW);
}

void CTextureAnimDlg::cleanCtrl(void)
{
	if (_TextureChooser)
	{
		_TextureChooser->DestroyWindow();
		delete _TextureChooser;
		_TextureChooser = NULL;
	}
	if (_TextureIndexDialog)
	{
		_TextureIndexDialog->DestroyWindow();
		delete _TextureIndexDialog;
		_TextureIndexDialog = NULL;
	}
}


void CTextureAnimDlg::setupCtrl(void)
{
	// is there an animation ?
	if (_EditedParticle->getTextureGroup())
	{
		if (!_TextureIndexDialog)
		{
			
			_TextureIndexDialog = new CAttribDlgInt("TEXTURE_INDEX", _Node, 0, _EditedParticle->getTextureGroup()->getNbTextures() - 1);			
			_TextureIndexWrapper.P = _EditedParticle;
			_TextureIndexDialog->setWrapper(&_TextureIndexWrapper );			
			_TextureIndexDialog->setSchemeWrapper(&_TextureIndexWrapper );

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ANIM_SEQUENCE));
			_TextureIndexDialog->init(bmh, 0, 30, this);		
			m_ChooseTextures.EnableWindow(TRUE);
		}
		if (_MTP)
		{
			int display = _MTP->isMultiTextureEnabled() ? 1 : 0;
			m_MultiTexEnable = display;
			GetDlgItem(IDC_EDIT_MULTITEX)->EnableWindow(display);		
			GetDlgItem(IDC_MULTITEX)->EnableWindow(TRUE);
		}	
	}
	else // no animation, just show a texture chooser
	{
		_TextureChooser = new CTextureChooser(_MTP, _Node);			
		_TextureWrapper.P = _EditedParticle;
		_TextureChooser->setWrapper(&_TextureWrapper);
		_TextureChooser->init(0, 30, this);
		m_ChooseTextures.EnableWindow(FALSE);

		if (_MTP)
		{
			
			((CButton *) GetDlgItem(IDC_MULTITEX))->SetCheck(0);
			GetDlgItem(IDC_EDIT_MULTITEX)->EnableWindow(0);		
			GetDlgItem(IDC_MULTITEX)->EnableWindow(0);
		}	
	}

	UpdateData(FALSE);
}




void CTextureAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureAnimDlg)
	DDX_Control(pDX, IDC_CHOOSE_TEXTURES, m_ChooseTextures);
	DDX_Check(pDX, IDC_ENABLE_TEXTURE_ANIM, m_EnableTextureAnim);
	DDX_Check(pDX, IDC_MULTITEX, m_MultiTexEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CTextureAnimDlg)
	ON_BN_CLICKED(IDC_CHOOSE_TEXTURES, OnChooseTextures)
	ON_BN_CLICKED(IDC_ENABLE_TEXTURE_ANIM, OnEnableTextureAnim)
	ON_BN_CLICKED(IDC_MULTITEX, OnMultiTex)
	ON_BN_CLICKED(IDC_EDIT_MULTITEX, OnEditMultitex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureAnimDlg message handlers

void CTextureAnimDlg::OnChooseTextures() 
{
	
	_GradientInterface.P = _EditedParticle;	
	CValueGradientDlg vd(&_GradientInterface, _Node, false, this, NULL, false, 1);	
	_GradientInterface.Dlg = &vd;
	vd.DoModal();	
}

void CTextureAnimDlg::OnEnableTextureAnim() 
{
	UpdateData();
	if (!m_EnableTextureAnim)
	{
		_EditedParticle->setTexture(NULL);
	}
	else
	{
		// put a dummy texture as a first texture
		NLMISC::CSmartPtr<NL3D::ITexture> tex = (NL3D::ITexture *) new NL3D::CTextureFile(std::string(""));
		NL3D::CTextureGrouped *tg = new NL3D::CTextureGrouped;
		tg->setTextures(&tex, 1);
		_EditedParticle->setTextureGroup(tg);
		_EditedParticle->setTextureIndex(0);		
	}
	cleanCtrl();
	setupCtrl();
	updateModifiedFlag();
}


////////////////////////////////////////////////////////
// CTextureAnimDlg::CGradientInterface implementation //
////////////////////////////////////////////////////////



CEditAttribDlg *CTextureAnimDlg::CGradientInterface::createDialog(uint index, CValueGradientDlg *grad, CParticleWorkspace::CNode *ownerNode)
{
	CTextureChooser *tc = new CTextureChooser(NULL, ownerNode);
	_TextureWrapper.P = P;
	_TextureWrapper.Dlg = Dlg;
	_TextureWrapper.Index = index;
	_TextureWrapper.OwnerNode = ownerNode;
	tc->setWrapper(&_TextureWrapper);
	return tc;
}
void CTextureAnimDlg::CGradientInterface::modifyGradient(TAction action, uint index)
{
	nlassert(P);
	nlassert(P->getTextureGroup());
	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(P->getTextureGroup()->getNbTextures());
	P->getTextureGroup()->getTextures(&textureList[0]);

	switch(action)
	{
		case IValueGradientDlgClient::Add:
		{
			// we duplicate the last texture, so that they have the same size
			NLMISC::CSmartPtr<NL3D::ITexture> lastTex = textureList[textureList.size() - 1];
			textureList.push_back(lastTex);		
		}
		break;
		case IValueGradientDlgClient::Insert:
		{
			// we duplicate the current texture, so that they have the same size
			NLMISC::CSmartPtr<NL3D::ITexture> tex = textureList[index];
			textureList.insert(textureList.begin() + index, tex);			
		}			
		break;
		case IValueGradientDlgClient::Delete:		
			textureList.erase(textureList.begin() + index);						
		break;
	}

	P->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
}
void CTextureAnimDlg::CGradientInterface::displayValue(CDC *dc, uint index, sint x, sint y)
{
	const uint tSize = 20;
	NLMISC::CSmartPtr<NL3D::ITexture> tex = P->getTextureGroup()->getTexture(index);
	tex->generate();

	// make copy of the texture
	NLMISC::CBitmap cb(* ((NL3D::ITexture *) tex));

	cb.convertToType(NLMISC::CBitmap::RGBA);
	cb.resample(tSize, tSize);
	

	uint32 *dat  = (uint32 *) &(cb.getPixels()[0]);

	HBITMAP myBitmap = ::CreateBitmap(tSize, tSize, 1, 32, dat);
	
	HDC bitmapDc = ::CreateCompatibleDC(dc->m_hDC);
	HGDIOBJ old = ::SelectObject(bitmapDc, myBitmap);

	// display the texture
	::BitBlt(dc->m_hDC, x + 20, y + 10, tSize, tSize, bitmapDc, 0, 0, SRCCOPY);

	// free resources
	::SelectObject(bitmapDc, old);
	::DeleteDC(bitmapDc);
	::DeleteObject(myBitmap);

}
uint32 CTextureAnimDlg::CGradientInterface::getSchemeSize(void) const
{
	nlassert(P->getTextureGroup());
	return P->getTextureGroup()->getNbTextures();
}
uint32 CTextureAnimDlg::CGradientInterface::getNbSteps(void) const
{
	return 1;
}
void CTextureAnimDlg::CGradientInterface::setNbSteps(uint32 value)
{
	// this should never be called, as we don't allow nbsteps to be called
	nlassert(false);
}


///////////////////////////////////////////////////////////////////////////
// CTextureAnimDlg::CGradientInterface::CTextureWrapper implementation //
///////////////////////////////////////////////////////////////////////////


NL3D::ITexture *CTextureAnimDlg::CGradientInterface::CTextureWrapper::get(void)
{
	nlassert(P);
	nlassert(P->getTextureGroup());
	return P->getTextureGroup()->getTexture(Index);
}
void CTextureAnimDlg::CGradientInterface::CTextureWrapper::set(NL3D::ITexture *t)
{
	nlassert(P);
	nlassert(P->getTextureGroup());

	// if a texture is added, it must have the same size than other textures
	if (P->getTextureGroup()->getNbTextures() > 1)
	{
		NLMISC::CSmartPtr<NL3D::ITexture> tex = P->getTextureGroup()->getTexture(0);
		tex->generate();
		t->generate();

		if (t->getWidth() != tex->getWidth() || t->getHeight() != tex->getHeight())
		{
			::MessageBox(NULL, "All textures must have the same size !", "error", MB_OK);
			return;
		}

		if (t->PixelFormat != tex->PixelFormat)
		{
			::MessageBox(NULL, "All textures must have the same pixel format !", "error", MB_OK);
			return;
		}
	}

	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(P->getTextureGroup()->getNbTextures());
	P->getTextureGroup()->getTextures(&textureList[0]);

	textureList[Index] = t;


	P->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
	
	Dlg->invalidateGrad();
}

void CTextureAnimDlg::OnMultiTex() 
{
	UpdateData();
	_MTP->enableMultiTexture(m_MultiTexEnable ? true : false /* VC WARNING */);
	setupCtrl();
	updateModifiedFlag();
}

void CTextureAnimDlg::childPopupClosed(CWnd *child)
{
	nlassert(_MultiTexDlg == child);
	_MultiTexDlg->DestroyWindow();
	delete _MultiTexDlg;
	_MultiTexDlg = NULL;
	EnableWindow(TRUE);
}

void CTextureAnimDlg::OnEditMultitex() 
{	
	EnableWindow(FALSE);
	_MultiTexDlg = new 	CMultiTexDlg(_Node, _MTP, this, this);
	_MultiTexDlg->init(this);
}
