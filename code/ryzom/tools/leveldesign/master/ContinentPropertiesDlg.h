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

#ifndef CONTINENT_PROPERTIES_DLG_H
#define CONTINENT_PROPERTIES_DLG_H

/////////////////////////////////////////////////////////////////////////////
// ContinentProperties dialog

class CContinentPropertiesDlg : public CDialog
{
// Construction
public:
	CContinentPropertiesDlg(CWnd* pParent = NULL);

// Dialog Data
	enum { IDD = IDD_CONTINENT_PROPERTIES };

	CString ContinentName;
	CString LandFile;
	CString LandDir;

	CString	LandBankFile;
	CString	LandFarBankFile;
	CString	LandTileNoiseDir;
	CString	LandZoneWDir;
	CString	OutIGDir;

	CString DfnDir;
	CString GameElemDir;


	afx_msg void OnButtonLandFile ();
	afx_msg void OnButtonLandDir ();
	afx_msg void OnButtonDfnDir ();
	afx_msg void OnButtonGameElemDir ();
	afx_msg void OnButtonLandBankFile ();
	afx_msg void OnButtonLandFarBankFile ();
	afx_msg void OnButtonLandTileNoiseDir ();
	afx_msg void OnButtonLandZoneW ();
	afx_msg void OnButtonOutIGDir ();


protected:
	virtual void DoDataExchange (CDataExchange* pDX);

// Implementation
protected:

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};


#endif
