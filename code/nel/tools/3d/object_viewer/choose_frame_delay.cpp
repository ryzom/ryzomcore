// choose_frame_delay.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_frame_delay.h"
#include "particle_dlg.h"
#include "nel/3d/particle_system.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay dialog


//*****************************************************************************************************
CChooseFrameDelay::CChooseFrameDelay(CObjectViewer *objectViewer, CWnd* pParent)
	: CDialog(CChooseFrameDelay::IDD, pParent), _LockToPS(false)
{
	nlassert(objectViewer);
	_CFDWrapper.OV = objectViewer;
	//{{AFX_DATA_INIT(CChooseFrameDelay)	
	//}}AFX_DATA_INIT
	_ER = new CEditableRangeUInt("CHOOSE_FRAME_DELAY", NULL, 0, 500);
	_ER->enableLowerBound(0, false);
	_ER->enableUpperBound(10000, false);	
	_ER->setWrapper(&_CFDWrapper);
	//	
	objectViewer->registerMainLoopCallBack(this);
}

CChooseFrameDelay::~CChooseFrameDelay()
{
	lockToPS(false);
	CObjectViewer *ov = _CFDWrapper.OV;
	ov->removeMainLoopCallBack(this);
	_ER->DestroyWindow();
	delete _ER;	
}

//*****************************************************************************************************
void CChooseFrameDelay::lockToPS(bool lock)
{
	if (lock == _LockToPS) return;
	_ER->EnableWindow(!lock);
	_LockToPS = lock;
	if (!lock)
	{
		_CFDWrapper.set(0);
		_ER->update();
	}
}

//*****************************************************************************************************
void CChooseFrameDelay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFrameDelay)	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFrameDelay, CDialog)
	//{{AFX_MSG_MAP(CChooseFrameDelay)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay message handlers

//*****************************************************************************************************
BOOL CChooseFrameDelay::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT r;
	GetDlgItem(IDC_FRAME_DELAY)->GetWindowRect(&r);
	ScreenToClient(&r);
	_ER->init(r.left, r.top, this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

uint32 CChooseFrameDelay::CCFDWrapper::get(void) const { return OV->getFrameDelay(); }
void   CChooseFrameDelay::CCFDWrapper::set(const uint32 &value) { OV->setFrameDelay(value); }

//*****************************************************************************************************
void CChooseFrameDelay::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_CHOOSE_FRAME_DELAY_DLG);
	CDialog::OnDestroy();		
}

//*****************************************************************************************************
void CChooseFrameDelay::goPreRender()
{
	CObjectViewer *ov = _CFDWrapper.OV;
	if (_LockToPS && ov->getParticleDialog()->getActiveNode())
	{
		CParticleWorkspace::CNode *activeNode = ov->getParticleDialog()->getActiveNode();
		nlassert(activeNode->isLoaded());
		// get integration step from ps
		CObjectViewer *ov = _CFDWrapper.OV;
		NL3D::CParticleSystem *ps = activeNode->getPSPointer();
		uint integrationTime = (uint) (1000.f * ps->getTimeTheshold());
		if (!ps->getBypassMaxNumIntegrationSteps())
		{
			integrationTime *= ps->getMaxNbIntegrations();
		}
		if (integrationTime != _CFDWrapper.get())
		{
			_CFDWrapper.set(integrationTime);
			_ER->update();
		}		
	}
}
