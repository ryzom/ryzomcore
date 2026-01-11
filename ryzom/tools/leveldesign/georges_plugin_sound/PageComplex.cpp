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

// PageComplex.cpp : implementation file
//

#include "std_sound_plugin.h"
#include "sound_document_plugin.h"
#include <string>

#include "PageComplex.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CPageComplex property page

#undef new
IMPLEMENT_DYNCREATE(CPageComplex, CPageBase)
#define new NL_NEW

CPageComplex::CPageComplex(NLGEORGES::CSoundDialog *soundDialog) : CPageBase(soundDialog, CPageComplex::IDD)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//{{AFX_DATA_INIT(CPageComplex)
	_SequenceSize = 0;
	_MaxDelay = 0;
	_MinDelay = 0;
	//}}AFX_DATA_INIT
}

CPageComplex::~CPageComplex()
{
}

void CPageComplex::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageComplex)
	DDX_Control(pDX, IDC_BTN_RANDOM_SOUND, _BtnRandomSound);
	DDX_Control(pDX, IDC_BTN_RANDOM_DELAY, _BtnRandomDelay);
	DDX_Text(pDX, IDC_EDIT_SEQ_SIZE, _SequenceSize);
	DDX_Text(pDX, IDC_EDIT_MAX_DELAY, _MaxDelay);
	DDX_Text(pDX, IDC_EDIT_MIN_DELAY, _MinDelay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageComplex, CPropertyPage)
	//{{AFX_MSG_MAP(CPageComplex)
	ON_BN_CLICKED(IDC_BTN_RANDOM_DELAY, OnBtnRandomDelay)
	ON_BN_CLICKED(IDC_BTN_RANDOM_SOUND, OnBtnRandomSound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageComplex message handlers

void CPageComplex::OnBtnRandomDelay() 
{
	UpdateData(TRUE);
	nldebug("Random sound : size = %u, min = %u, max = %u", _SequenceSize, _MinDelay, _MaxDelay);

	if (_MinDelay < 0)
	{
		_MinDelay = 0;
		UpdateData(FALSE);
	}

	if (_MaxDelay < _MinDelay)
	{
		_MaxDelay = _MinDelay+1;
		UpdateData(FALSE);
	}

	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();

	if (pdoc != NULL)
	{
		// create a delay list.
		string seq;

		for (sint i=0; i<_SequenceSize; ++i)
		{
			char tmp[200];
			sprintf(tmp, "%u", _MinDelay + rand()%((_MaxDelay+1)-_MinDelay));
			seq += string(tmp)+";";
		}
		pdoc->setValue(seq.c_str(), ".SoundType.DelaySeq");
		pdoc->refreshView();
	}
}

void CPageComplex::OnBtnRandomSound() 
{
	UpdateData(TRUE);
	nldebug("Random sound : size = %u, min = %u, max = %u", _SequenceSize, _MinDelay, _MaxDelay);

	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();

	if (pdoc != NULL)
	{
		// create a sound list.
		NLGEORGES::UFormElm	*psoundsArray;
		pdoc->getForm()->getRootNode().getNodeByName(&psoundsArray, ".SoundType.SoundList");

		if (psoundsArray != NULL)
		{
			uint size;
			psoundsArray->getArraySize(size);

			string seq;

			for (sint i=0; i<_SequenceSize; ++i)
			{
				char tmp[200];
				sprintf(tmp, "%u", rand()%size);
				seq += string(tmp)+";";
			}
			pdoc->setValue(seq.c_str(), ".SoundType.SoundSeq");
			pdoc->refreshView();
		}
	}
}

void CPageComplex::onDocChanged()
{
	// the document have been modified, update the dialog
	NLGEORGES::IEditDocument *pdoc = SoundDialog->getSoundPlugin()->getActiveDocument();

	bool valid = false;

	if (pdoc != NULL)
	{
		string type, dfnName;
		NLGEORGES::UFormElm *psoundType;

		pdoc->getForm()->getRootNode().getNodeByName(&psoundType, ".SoundType");

		if (psoundType != NULL)
		{
			psoundType->getDfnName(dfnName);
			if (dfnName == "complex_sound.dfn")
			{
				_BtnRandomDelay.EnableWindow(TRUE);
				_BtnRandomSound.EnableWindow(TRUE);
				valid = true;
			}
		}
	}

	if (!valid)
	{
		_BtnRandomDelay.EnableWindow(FALSE);
		_BtnRandomSound.EnableWindow(FALSE);
	}
}