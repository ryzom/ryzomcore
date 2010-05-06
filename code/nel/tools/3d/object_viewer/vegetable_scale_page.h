#if !defined(AFX_VEGETABLE_SCALE_PAGE_H__420A39C3_E7DD_4C7F_9527_9A7B8DB6E101__INCLUDED_)
#define AFX_VEGETABLE_SCALE_PAGE_H__420A39C3_E7DD_4C7F_9527_9A7B8DB6E101__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_scale_page.h : header file
//

namespace	NL3D
{
	class	CVegetable;
}


class	CVegetableNoiseValueDlg;
class	CDirectEditableRangeFloat;
class	CVegetableDlg;


/////////////////////////////////////////////////////////////////////////////
// CVegetableScalePage dialog

class CVegetableScalePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CVegetableScalePage)

// Construction
public:
	CVegetableScalePage();
	~CVegetableScalePage();
	void	initVegetableDlg(CVegetableDlg *vegetableDlg) {_VegetableDlg= vegetableDlg;}

// Dialog Data
	//{{AFX_DATA(CVegetableScalePage)
	enum { IDD = IDD_VEGETABLE_SCALE_DLG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVegetableScalePage)
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
	//{{AFX_MSG(CVegetableScalePage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// The "father" VegetableDlg.
	CVegetableDlg					*_VegetableDlg;


	// The NoiseValue edition.
	CVegetableNoiseValueDlg			*_SxyDlg;
	CVegetableNoiseValueDlg			*_SzDlg;

	// BendFreqFactor
	CDirectEditableRangeFloat		*_BendFreqFactorDlg;


	// The vegetable to edit.
	NL3D::CVegetable				*_Vegetable;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_SCALE_PAGE_H__420A39C3_E7DD_4C7F_9527_9A7B8DB6E101__INCLUDED_)
