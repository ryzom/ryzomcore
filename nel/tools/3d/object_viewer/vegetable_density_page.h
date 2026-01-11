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

#if !defined(AFX_VEGETABLE_DENSITY_PAGE_H__12320F77_5179_4727_A551_F5A8FAE7EB3A__INCLUDED_)
#define AFX_VEGETABLE_DENSITY_PAGE_H__12320F77_5179_4727_A551_F5A8FAE7EB3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_density_page.h : header file
//

class	CDirectEditableRangeFloat;
class	CVegetableNoiseValueDlg;
class	CVegetableDlg;

namespace	NL3D
{
	class	CVegetable;
}

/////////////////////////////////////////////////////////////////////////////
// CVegetableDensityPage dialog

class CVegetableDensityPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CVegetableDensityPage)

// Construction
public:
	CVegetableDensityPage();
	~CVegetableDensityPage();
	void	initVegetableDlg(CVegetableDlg *vegetableDlg) {_VegetableDlg= vegetableDlg;}

// Dialog Data
	//{{AFX_DATA(CVegetableDensityPage)
	enum { IDD = IDD_VEGETABLE_DENSITY_DLG };
	CStatic	StaticVegetableShape;
	CButton	MaxDensityStaticText;
	CSliderCtrl	AngleMinSlider;
	CSliderCtrl	AngleMaxSlider;
	CEdit	AngleMinEdit;
	CEdit	AngleMaxEdit;
	CComboBox	DistTypeCombo;
	CButton	MaxDensityCheckBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVegetableDensityPage)
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
	//{{AFX_MSG(CVegetableDensityPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckMaxDensity();
	afx_msg void OnSelchangeComboDistType();
	afx_msg void OnRadioAngleCeiling();
	afx_msg void OnRadioAngleFloor();
	afx_msg void OnRadioAngleWall();
	afx_msg void OnReleasedcaptureSliderAngleMax(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSliderAngleMin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditAngleMin();
	afx_msg void OnKillfocusEditAngleMax();
	afx_msg void OnChangeEditAngleMin();
	afx_msg void OnChangeEditAngleMax();
	afx_msg void OnButtonVegetableBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// The "father" VegetableDlg.
	CVegetableDlg					*_VegetableDlg;


	// Density
	CVegetableNoiseValueDlg			*_DensityDlg;

	// MaxDensity;
	CDirectEditableRangeFloat		*_MaxDensityDlg;


	// The vegetable to edit.
	NL3D::CVegetable				*_Vegetable;

private:

	void		enableAngleEdit(uint radioId);
	// update slider and edit text.
	void		updateViewAngleMin();
	void		updateViewAngleMax();
	// update vegetable and view from edit text
	void		updateAngleMinFromEditText();
	void		updateAngleMaxFromEditText();

	float		_PrecMaxDensityValue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_DENSITY_PAGE_H__12320F77_5179_4727_A551_F5A8FAE7EB3A__INCLUDED_)
