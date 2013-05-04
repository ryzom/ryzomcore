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
#include "pick_sound.h"
#include "sound_system.h"

#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"
#include "nel/sound/u_source.h"

using namespace std;


/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog


//========================================================================================
CPickSound::CPickSound(const CPickSound::TNameVect &names, CWnd* pParent /*=NULL*/)
	: CDialog(CPickSound::IDD, pParent), _Names(names), _CurrSource(NULL)
{
	
	//{{AFX_DATA_INIT(CPickSound)
	//}}AFX_DATA_INIT
}


//========================================================================================
void CPickSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickSound)
	DDX_Control(pDX, IDC_LIST1, m_NameList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickSound, CDialog)
	//{{AFX_MSG_MAP(CPickSound)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchange)
	ON_BN_CLICKED(IDC_BUTTON1, OnPlaySound)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList)
	ON_BN_CLICKED(IDC_PLAY_SOUND, OnPlaySound)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickSound message handlers

//========================================================================================
BOOL CPickSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData();
	
	for (TNameVect::iterator it = _Names.begin(); it	!= _Names.end(); ++it)
	{
		m_NameList.AddString((*it).toString().c_str());
	}

	_Timer = SetTimer (1, 100, NULL);

	// store value
	if (CSoundSystem::getAudioMixer())
	{
		_BackupGain = CSoundSystem::getAudioMixer()->getListener ()->getGain();
		CSoundSystem::getAudioMixer()->getListener ()->getVelocity(_BackupVel);

		CSoundSystem::getAudioMixer()->getListener ()->setGain(1.0f);
		CSoundSystem::getAudioMixer()->getListener ()->setVelocity(NLMISC::CVector(0,0,0));
	}
	else
		_BackupGain  = 1.0f;

	// set new value

	if(!_Timer)
	{
		nlwarning ("Can't create the timer to update the sound system");
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//========================================================================================
void CPickSound::OnSelchange() 
{
	UpdateData();
	char str[1024];
	nlassert(m_NameList.GetTextLen(m_NameList.GetCurSel()) < 1024);
	
	m_NameList.GetText(m_NameList.GetCurSel(), str);
	_CurrName = NLMISC::CSheetId(str, "sound"); 
	
}

//========================================================================================
void CPickSound::OnPlaySound() 
{
	int curSel = m_NameList.GetCurSel();
	if (curSel == LB_ERR) return;
	stopCurrSource();
	CString sName;
	m_NameList.GetText(curSel, sName);
	CSoundSystem::create(std::string( (LPCTSTR) sName));
}

//========================================================================================
void CPickSound::OnTimer(UINT_PTR nIDEvent) 
{
	CSoundSystem::poll();

	CDialog::OnTimer(nIDEvent);
}

//========================================================================================
void CPickSound::OnDestroy() 
{
	stopCurrSource();
	CDialog::OnDestroy();

	if(_Timer != 0)
		KillTimer (_Timer);

	// restore old value
	if (CSoundSystem::getAudioMixer())
	{
		CSoundSystem::getAudioMixer()->getListener ()->setGain(_BackupGain);
		CSoundSystem::getAudioMixer()->getListener ()->setVelocity(_BackupVel);
	}
}

//========================================================================================
void CPickSound::OnDblclkList() 
{
	int curSel = m_NameList.GetCurSel();
	if (curSel == LB_ERR) return;
	stopCurrSource();
	CString sName;
	m_NameList.GetText(curSel, sName);
	_CurrSource = CSoundSystem::create(std::string( (LPCTSTR) sName));	
}

//========================================================================================
void CPickSound::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	stopCurrSource();
	CDialog::OnClose();
}

//========================================================================================
void CPickSound::stopCurrSource()
{
	delete _CurrSource;
	_CurrSource = NULL;
}
