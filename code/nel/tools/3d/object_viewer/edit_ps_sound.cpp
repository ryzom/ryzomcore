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


//

#include "std_afx.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/u_particle_system_sound.h"
#include "nel/3d/particle_system.h"
#include "object_viewer.h"
#include "edit_ps_sound.h"
#include "attrib_dlg.h"
#include "pick_sound.h"
#include "sound_system.h"
#include "editable_range.h"

using namespace std;

/// particle system sound system initialisation
void setPSSoundSystem(NLSOUND::UAudioMixer *am)
{
	NL3D::UParticleSystemSound::setPSSound(am);	
}

void releasePSSoundSystem(void)
{
	// nothing to do for now
}







/////////////////////////////////////////////////////////////////////////////
// CEditPSSound dialog



CEditPSSound::CEditPSSound(CParticleWorkspace::CNode *ownerNode, NL3D::CPSSound *sound) 
                          : _Node(ownerNode),
						    _Sound(sound), 
							_GainDlg(NULL),
							_PitchDlg(NULL),
							_PercentDlg(NULL)
{
	nlassert(sound);
}

CEditPSSound::~CEditPSSound()
{
	if (_GainDlg)
	{
		_GainDlg->DestroyWindow();
		delete _GainDlg;
	}
	if (_PitchDlg)
	{
		_PitchDlg->DestroyWindow();
		delete _PitchDlg;
	}
	if (_PercentDlg)
	{
		_PercentDlg->DestroyWindow();
		delete _PercentDlg;
	}
}

void CEditPSSound::init(CWnd* pParent /*= NULL*/)
{
	Create(IDD_SOUND, pParent);
	
	const uint posX = 3;
	uint	   posY = 52;
	RECT	   r;
	
	nlassert(_Sound);

	_PercentDlg = new CEditableRangeFloat(std::string("SOUND_EMISSION_PERCENT"), _Node, 0, 1);
	_EmissionPercentWrapper.S = _Sound;
	_PercentDlg->setWrapper(&_EmissionPercentWrapper);
	_PercentDlg->init(posX + 95, posY, this);

	posY += 35;

	_GainWrapper.S = _Sound;
	_GainDlg = new CAttribDlgFloat(std::string("SOUND VOLUME"), _Node, 0, 1);
	_GainDlg->setWrapper(&_GainWrapper);
	_GainDlg->setSchemeWrapper(&_GainWrapper);	
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_VOLUME));
	_GainDlg->init(bmh, posX, posY, this);
	_GainDlg->GetClientRect(&r);
	posY += r.bottom + 3;	


	_PitchWrapper.S = _Sound;
	_PitchDlg = new CAttribDlgFloat(std::string("SOUND PITCH"), _Node, 0.001f, 5);
	_PitchDlg->setWrapper(&_PitchWrapper);
	_PitchDlg->setSchemeWrapper(&_PitchWrapper);	
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_FREQ));
	_PitchDlg->init(bmh, posX, posY, this);
	_PitchDlg->GetClientRect(&r);
	posY += r.bottom + 3;	

	m_Spawn = _Sound->getSpawn();
	m_Mute  = _Sound->getMute();
	m_KeepOriginalPitch = _Sound->getUseOriginalPitchFlag();
	_PitchDlg->EnableWindow(!m_KeepOriginalPitch);
	
	ShowWindow(SW_SHOW); 
	UpdateData(FALSE);
}





void CEditPSSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditPSSound)
	DDX_Text(pDX, IDC_SOUND_NAME, m_SoundName);
	DDX_Check(pDX, IDC_SPAWN, m_Spawn);
	DDX_Check(pDX, IDC_MUTE, m_Mute);	
	DDX_Check(pDX, IDC_KEEP_ORIGINAL_PITCH, m_KeepOriginalPitch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditPSSound, CDialog)
	//{{AFX_MSG_MAP(CEditPSSound)
	ON_BN_CLICKED(IDC_BROWSE_SOUND, OnBrowseSound)
	ON_EN_CHANGE(IDC_SOUND_NAME, OnChangeSoundName)
	ON_BN_CLICKED(IDC_SPAWN, OnSpawn)
	ON_BN_CLICKED(IDC_BUTTON1, OnPlaySound)
	ON_BN_CLICKED(IDC_MUTE, OnMute)
	ON_BN_CLICKED(IDC_KEEP_ORIGINAL_PITCH, OnKeepOriginalPitch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditPSSound message handlers

void CEditPSSound::OnBrowseSound() 
{
//	CPickSound::TNameVect names;
	vector<NLMISC::TStringId>	names;
	

	NLSOUND::UAudioMixer *audioMixer = CSoundSystem::getAudioMixer();
	if (audioMixer)
	{
		audioMixer->getSoundNames(names);
	}

	CPickSound ps(names, this);

	if (ps.DoModal() == IDOK)
	{
		m_SoundName = NLMISC::CStringMapper::unmap(ps.getName()).c_str();
		_Sound->setSoundName(ps.getName());
		updateModifiedFlag();
		UpdateData(FALSE);
	}
}

BOOL CEditPSSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	nlassert(_Sound);
	m_SoundName = NLMISC::CStringMapper::unmap(_Sound->getSoundName()).c_str();
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditPSSound::OnChangeSoundName() 
{
	nlassert(_Sound);
	UpdateData();
	_Sound->setSoundName(NLMISC::CStringMapper::map(NLMISC::tStrToUtf8(m_SoundName)));
	 updateModifiedFlag();
}

void CEditPSSound::OnSpawn() 
{
	UpdateData(TRUE);
	_Sound->setSpawn(m_Spawn ? true : false /* to avoid VCC warning*/);	
	 updateModifiedFlag();
}

// play the currently selected sound
void CEditPSSound::OnPlaySound() 
{
	CSoundSystem::play(NLMISC::tStrToUtf8(m_SoundName));
}

void CEditPSSound::OnMute() 
{
	UpdateData(TRUE);
	_Sound->setMute(m_Mute ? true : false /* to avoid VCC warning*/);
	updateModifiedFlag();
}

void CEditPSSound::OnKeepOriginalPitch() 
{
	UpdateData(TRUE);
	bool hadScheme = _PitchWrapper.getScheme() != NULL;
	_Sound->setUseOriginalPitchFlag(m_KeepOriginalPitch ? true : false /* to avoid VCC warning*/);	
	nlassert(_PitchDlg);
	if (m_KeepOriginalPitch)
	{		
		if (hadScheme) _PitchDlg->update();
		_PitchDlg->closeEditWindow();		
	}
	_PitchDlg->EnableWindow(!m_KeepOriginalPitch);
	updateModifiedFlag();
}
