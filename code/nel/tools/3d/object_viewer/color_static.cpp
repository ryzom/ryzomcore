// This file was created on March 28th 2001 by Robert Brault.
// I created this Class to be able change the Color of your Static Text.
// This is Derived from CStatic.
//
// There are three functions available Currently:
// SetBkColor(COLORREF crColor)
// SetTextColor(COLORREF crColor)
//
// How To Use:
// Add three files to your project
// ColorStatic.cpp, ColorStatic.h and Color.h
// Color.h has (#define)'s for different colors (add any color you desire).
//
// Add #include "ColorStatic.h" to your Dialogs Header file.
// Declare an instance of CColorStatic for each static text being modified.
// Ex. CColorStatic m_stText;
//
// In your OnInitDialog() add a SubclassDlgItem for each CColorStatic member variable.
// Ex. m_stText.SubclassDlgItem(IDC_ST_TEXT, this);
// In this same function initialize your color for each piece of text unless you want the default.


// ColorStatic.cpp : implementation file
//

#include "std_afx.h"
#include "color_static.h"


/////////////////////////////////////////////////////////////////////////////
// CColorStatic

CColorStatic::CColorStatic()
{
	m_crBkColor = ::GetSysColor(COLOR_3DFACE); // Initializing the Background Color to the system face color.
	m_crTextColor = 0; // Initializing the text to Black
	m_brBkgnd.CreateSolidBrush(m_crBkColor); // Create the Brush Color for the Background.
}

CColorStatic::~CColorStatic()
{
}


BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
	//{{AFX_MSG_MAP(CColorStatic)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorStatic message handlers

HBRUSH CColorStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	HBRUSH hbr;
	hbr = (HBRUSH)m_brBkgnd; // Passing a Handle to the Brush
	pDC->SetBkColor(m_crBkColor); // Setting the Color of the Text Background to the one passed by the Dialog
	pDC->SetTextColor(m_crTextColor); // Setting the Text Color to the one Passed by the Dialog

	if (nCtlColor)       // To get rid of compiler warning
      nCtlColor += 0;

	return hbr;
	
}

void CColorStatic::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor; // Passing the value passed by the dialog to the member varaible for Backgound Color
	m_brBkgnd.DeleteObject(); // Deleting any Previous Brush Colors if any existed.
	m_brBkgnd.CreateSolidBrush(crColor); // Creating the Brush Color For the Static Text Background
	RedrawWindow();
}

void CColorStatic::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor; // Passing the value passed by the dialog to the member varaible for Text Color
	RedrawWindow();
}
