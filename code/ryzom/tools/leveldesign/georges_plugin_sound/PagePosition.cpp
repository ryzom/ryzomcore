// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// PagePosition.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "georges_plugin_sound.h"
#include "listener_view.h"
#include "sound_dialog.h"
#include "nel/georges/u_form_elm.h"
#include "sound_document_plugin.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/sound.h"
#include "nel/sound/simple_sound.h"

#include "PagePosition.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CPagePosition property page

#undef new
IMPLEMENT_DYNCREATE(CPagePosition, CPageBase)
#define new NL_NEW

CPagePosition::CPagePosition(NLGEORGES::CSoundDialog *soundDialog) : CPageBase(soundDialog, CPagePosition::IDD)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPagePosition)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPagePosition::~CPagePosition()
{
}

void CPagePosition::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPagePosition)
	DDX_Control(pDX, IDC_PSEUDO_PICTURE, _Picture);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPagePosition, CPropertyPage)
	//{{AFX_MSG_MAP(CPagePosition)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPagePosition message handlers

BOOL CPagePosition::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	// insert the listener position view inside the pseudo picture control.
	WINDOWPLACEMENT place;
	_Picture.GetWindowPlacement(&place);

	
#undef new
	_ListenerView = new NLGEORGES::CListenerView();
#define new NL_NEW
	_ListenerView->init(SoundDialog->getSoundPlugin(), CRect(place.rcNormalPosition), this);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPagePosition::onDocChanged()
{
/*	// the document have been modified, update the dialog
	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();
*/
	bool valid = false;
/*
	if (pdoc != NULL)
	{
		string type, dfnName;

		pdoc->getForm()->getRootNode().getDfnName(dfnName);
		if (dfnName == "sound.dfn")
		{
*/			NLSOUND::CSound *sound = SoundDialog->getSoundPlugin()->getSound();

			if (sound != 0)
			{
				// the sound is available !
				float minDist = 1.0f;
				_ListenerView->setAngles(uint32(180 * sound->getConeInnerAngle() / NLMISC::Pi), uint32(180 * sound->getConeOuterAngle() / NLMISC::Pi));
				if (sound->getSoundType() == NLSOUND::CSound::SOUND_SIMPLE)
				{
					_ListenerView->setAlpha(static_cast<NLSOUND::CSimpleSound*>(sound)->getAlpha());
					_ListenerView->setShowAlpha(true);
					minDist = static_cast<NLSOUND::CSimpleSound*>(sound)->getMinDistance();
				}
				else
					_ListenerView->setShowAlpha(false);

				_ListenerView->setMinMaxDistances(minDist, sound->getMaxDistance());
				_ListenerView->setActive(true); 

				valid = true;
			}
/*		}
	}
*/
	if (!valid)
	{
		// not a sound file
		_ListenerView->setActive(false);
	}
}
