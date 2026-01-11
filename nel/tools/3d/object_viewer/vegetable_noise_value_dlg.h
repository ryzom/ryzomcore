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

#if !defined(AFX_VEGETABLE_NOISE_VALUE_DLG_H__9845CF5F_B7BB_4D55_9153_443BDC89613C__INCLUDED_)
#define AFX_VEGETABLE_NOISE_VALUE_DLG_H__9845CF5F_B7BB_4D55_9153_443BDC89613C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_noise_value_dlg.h : header file
//


#include <string>
#include "edit_attrib_dlg.h"
#include "vegetable_refresh.h"


class	CDirectEditableRangeFloat;
namespace NLMISC
{
	class	CNoiseValue;
};

/////////////////////////////////////////////////////////////////////////////
// CVegetableNoiseValueDlg dialog

class CVegetableNoiseValueDlg : public CEditAttribDlg
{
public:
	// Approximate Height of this control
	enum	{ControlHeight= 120};

// Construction
public:
	/// noiseValueName must be unique in the app. Used for Title too.
	CVegetableNoiseValueDlg(const std::string &noiseValueName);   // standard constructor
	~CVegetableNoiseValueDlg();

	// After construction, and before OnInitDialog(), setup those Range/Default
	void		setDefaultRangeAbs(float defRangeMin, float defRangeMax);
	void		setDefaultRangeRand(float defRangeMin, float defRangeMax);
	void		setDefaultRangeFreq(float defRangeMin, float defRangeMax);


// Dialog Data
	//{{AFX_DATA(CVegetableNoiseValueDlg)
	enum { IDD = IDD_VEGETABLE_NOISE_VALUE_DLG };
	CStatic	StaticScaleMarker;
	CSliderCtrl	SliderNoiseValue;
	CButton	NoiseValueName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableNoiseValueDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	///	CEditAttribDlg implementation
	virtual	BOOL	EnableWindow( BOOL bEnable);
	virtual	void	init(uint32 x, uint32 y, CWnd *pParent);


	/// setup the NoiseValue to edit, and update view.
	void			setNoiseValue(NLMISC::CNoiseValue	*nv, IVegetableRefresh *vegetRefresh);


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVegetableNoiseValueDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSliderVegetableScaleNoise(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	std::string					_TitleName;
	// The noise value setuped.
	NLMISC::CNoiseValue			*_NoiseValue;
	IVegetableRefresh			*_VegetableRefresh;

	// ScaleSlider mgt.
	bool						_EnteringScalerSlider;
	float						_BkupAbsValue;
	float						_BkupRandValue;
	void						applyScaleSlider(sint scrollValue);


	CDirectEditableRangeFloat	*_AbsValue;
	CDirectEditableRangeFloat	*_RandValue;
	CDirectEditableRangeFloat	*_Frequency;

	float		_DefAbsRangeMin, _DefAbsRangeMax;
	float		_DefRandRangeMin, _DefRandRangeMax;
	float		_DefFreqRangeMin, _DefFreqRangeMax;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_NOISE_VALUE_DLG_H__9845CF5F_B7BB_4D55_9153_443BDC89613C__INCLUDED_)
