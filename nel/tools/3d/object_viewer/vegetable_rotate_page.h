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

#if !defined(AFX_VEGETABLE_ROTATE_PAGE_H__EE4CBECA_CB0B_418B_8F72_CC02AB397E10__INCLUDED_)
#define AFX_VEGETABLE_ROTATE_PAGE_H__EE4CBECA_CB0B_418B_8F72_CC02AB397E10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_rotate_page.h : header file
//

namespace	NL3D
{
	class	CVegetable;
}


class	CVegetableNoiseValueDlg;
class	CVegetableDlg;


/////////////////////////////////////////////////////////////////////////////
// CVegetableRotatePage dialog

class CVegetableRotatePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CVegetableRotatePage)

// Construction
public:
	CVegetableRotatePage();
	~CVegetableRotatePage();
	void	initVegetableDlg(CVegetableDlg *vegetableDlg) {_VegetableDlg= vegetableDlg;}

// Dialog Data
	//{{AFX_DATA(CVegetableRotatePage)
	enum { IDD = IDD_VEGETABLE_ROTATE_DLG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVegetableRotatePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	/** set the vegetble to edit. NULL will disable all the controls.
	 *	Called by CVegetableDlg.
	 */
	void			setVegetableToEdit(NL3D::CVegetable *vegetable);


// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CVegetableRotatePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// The "father" VegetableDlg.
	CVegetableDlg					*_VegetableDlg;


	// The NoiseValue edition.
	CVegetableNoiseValueDlg			*_RxDlg;
	CVegetableNoiseValueDlg			*_RyDlg;
	CVegetableNoiseValueDlg			*_RzDlg;


	// The vegetable to edit.
	NL3D::CVegetable				*_Vegetable;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_ROTATE_PAGE_H__EE4CBECA_CB0B_418B_8F72_CC02AB397E10__INCLUDED_)
