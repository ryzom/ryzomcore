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
