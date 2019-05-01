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
#include "sound_anim_dlg.h"
#include "pick_sound.h"
#include "sound_system.h"

#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/sound_anim_marker.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;



// ********************************************************

CSoundAnimDlg::CSoundAnimDlg(CObjectViewer* objView, CAnimationDlg* animDlg, CWnd* pParent/*=NULL*/)
	: CDialog(CSoundAnimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundAnimDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	_ObjView = objView;
	_AnimationDlg = animDlg;
	_SelectedMarker = 0;
}

// ********************************************************

int CSoundAnimDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	RECT r;
	r.top = 3;
	r.bottom = 53;
	r.left = 150;
	r.right = 674;

	_AnimView.Create(_ObjView, _AnimationDlg, this, r);        

	return 0;
}

// ********************************************************

BOOL CSoundAnimDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE; 
}

// ********************************************************

void CSoundAnimDlg::handle()
{
	float sec = _AnimationDlg->getTime();
	std::string text = toString("time: %.3f", sec);
	GetDlgItem(IDC_SOUNDANIMINFO)->SetWindowText(nlUtf8ToTStr(text));

	_AnimView.updateCursor(); 
}

// ********************************************************

void CSoundAnimDlg::setAnimTime(float animStart, float animEnd)
{
	nlwarning("START=%.3f - END=%.3f", animStart, animEnd);
	_AnimView.setAnimTime(animStart, animEnd);
}

// ********************************************************

void CSoundAnimDlg::updateScroll(uint pos, uint min, uint max)
{
	CScrollBar* scroll = (CScrollBar*) GetDlgItem(IDC_SOUND_ANIM_SCROLLBAR);

	scroll->SetScrollRange(min, max, FALSE);
	scroll->SetScrollPos(pos, TRUE);
}

// ********************************************************

void CSoundAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundAnimDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ********************************************************

void CSoundAnimDlg::selectMarker(CSoundAnimMarker* marker)
{
	_SelectedMarker = marker;
	updateSounds();	
}

// ********************************************************

void CSoundAnimDlg::updateSounds()
{
	if (_SelectedMarker != 0)
	{
		vector<NLMISC::CSheetId> sounds;

		_SelectedMarker->getSounds(sounds);

		CListBox* list = (CListBox*) GetDlgItem(IDC_SOUND_ANIM_LIST);
		list->ResetContent();

		vector<NLMISC::CSheetId>::iterator iter;

		for (iter = sounds.begin(); iter != sounds.end(); iter++)
		{
			list->AddString(nlUtf8ToTStr((*iter).toString()));
		}

		list->UpdateData();
	}
}

// ********************************************************

void CSoundAnimDlg::OnAddSound()
{
	if (_SelectedMarker != 0)
	{
//		CPickSound::TNameVect names;
		vector<NLMISC::CSheetId>	names;
		

		NLSOUND::UAudioMixer *audioMixer = CSoundSystem::getAudioMixer();
		if (audioMixer)
		{
			audioMixer->getSoundNames(names);
		}

		CPickSound ps(names, this);

		if (ps.DoModal() == IDOK)
		{
//			string name = ps.getName();
			_SelectedMarker->addSound(ps.getName());
			updateSounds();	
		}
	}
}

// ********************************************************

void CSoundAnimDlg::OnRemoveSound()
{
	if (_SelectedMarker != 0)
	{
		TCHAR s[256];
		CListBox* list = (CListBox*) GetDlgItem(IDC_SOUND_ANIM_LIST);

		if (list->GetText(list->GetCurSel(), s) != LB_ERR)
		{
			_SelectedMarker->removeSound(NLMISC::CSheetId(tStrToUtf8(s), "sound"));
			updateSounds();
		}
	}
}

// ********************************************************

void CSoundAnimDlg::OnSave()
{
	_AnimView.save();
}

// ********************************************************

void CSoundAnimDlg::OnZoomOut()
{
	_AnimView.zoomOut();
}

// ********************************************************

void CSoundAnimDlg::OnZoomIn()
{
	_AnimView.zoomIn();
}

// ********************************************************

void CSoundAnimDlg::OnMark()
{
	_AnimView.mark();
}

// ********************************************************

void CSoundAnimDlg::OnDelete()
{
	_AnimView.deleteMarker();
}

// ********************************************************

void CSoundAnimDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

	nlwarning("CSoundAnimDlg::OnHScroll");
	
   // Get the minimum and maximum scroll-bar positions.
   int minpos;
	int maxpos;
	pScrollBar->GetScrollRange(&minpos, &maxpos); 
	maxpos = pScrollBar->GetScrollLimit();

	// Get the current position of scroll box.
	int curpos = pScrollBar->GetScrollPos();

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:	  // Scroll to far left.
		curpos = minpos;
	  break;

	case SB_RIGHT:		// Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:	// End scroll.
		break;

	case SB_LINELEFT:		// Scroll left.
		curpos -= 5;
		if (curpos < minpos)
		{
		 curpos = minpos;
		}
		break;

	case SB_LINERIGHT:	// Scroll right.
		curpos += 5;
		if (curpos > maxpos)
		{
		 curpos = maxpos;
		}
		break;

	case SB_PAGELEFT:	// Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO	info;
		pScrollBar->GetScrollInfo(&info, SIF_ALL);

		if (curpos > minpos)
		{
			curpos = std::max(minpos, curpos - (int) info.nPage);
		}
	}
		break;

	case SB_PAGERIGHT:		// Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO	info;
		pScrollBar->GetScrollInfo(&info, SIF_ALL);

		if (curpos < maxpos)
		{
			curpos = std::min(maxpos, curpos + (int) info.nPage);
		}
	}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;		// of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:	 // Drag scroll box to specified position. nPos is the
		curpos = nPos;	 // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	pScrollBar->SetScrollPos(curpos);

	_AnimView.changeScroll(curpos);

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ********************************************************

BEGIN_MESSAGE_MAP(CSoundAnimDlg, CDialog)
	//{{AFX_MSG_MAP(CSoundAnimDlg)
	ON_BN_CLICKED(IDC_ANIM_SOUND_ADD, OnAddSound)
	ON_BN_CLICKED(IDC_ANIM_SOUND_REMOVE, OnRemoveSound)
	ON_BN_CLICKED(IDC_ANIM_SOUND_SAVE, OnSave)
	ON_BN_CLICKED(IDC_ANIM_SOUND_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED(IDC_ANIM_SOUND_ZOOMOUT, OnZoomOut)
	ON_BN_CLICKED(IDC_ANIM_SOUND_MARK, OnMark)
	ON_BN_CLICKED(IDC_ANIM_SOUND_DELETE, OnDelete)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



