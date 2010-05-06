#if !defined(AFX_VEGETABLEAPPERANCEPAGE_H__BCF1C014_7271_4F83_9E62_F45C434B64F7__INCLUDED_)
#define AFX_VEGETABLEAPPERANCEPAGE_H__BCF1C014_7271_4F83_9E62_F45C434B64F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VegetableApperancePage.h : header file
//


#include "vegetable_list_color.h"


namespace	NL3D
{
	class	CVegetable;
}


class	CVegetableNoiseValueDlg;
class	CVegetableDlg;


/////////////////////////////////////////////////////////////////////////////
// CVegetableApperancePage dialog

class CVegetableApperancePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CVegetableApperancePage)

// Construction
public:
	CVegetableApperancePage();
	~CVegetableApperancePage();
	void	initVegetableDlg(CVegetableDlg *vegetableDlg) {_VegetableDlg= vegetableDlg;}

// Dialog Data
	//{{AFX_DATA(CVegetableApperancePage)
	enum { IDD = IDD_VEGETABLE_APPEARANCE_DLG };
	CVegetableListColor	ColorList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CVegetableApperancePage)
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
	//{{AFX_MSG(CVegetableApperancePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonVegetableAdd();
	afx_msg void OnButtonVegetableGetother();
	afx_msg void OnButtonVegetableInsert();
	afx_msg void OnButtonVegetableRemove();
	afx_msg void OnDblclkListVegetableColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// The "father" VegetableDlg.
	CVegetableDlg					*_VegetableDlg;


	// The NoiseValue edition.
	CVegetableNoiseValueDlg			*_BendPhaseDlg;
	CVegetableNoiseValueDlg			*_BendFactorDlg;
	CVegetableNoiseValueDlg			*_ColorDlg;


	// The vegetable to edit.
	NL3D::CVegetable				*_Vegetable;


	void	writeToVegetableColor(NL3D::CVegetable *vegetable);
	void	readFromVegetableColor(NL3D::CVegetable *vegetable);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLEAPPERANCEPAGE_H__BCF1C014_7271_4F83_9E62_F45C434B64F7__INCLUDED_)
