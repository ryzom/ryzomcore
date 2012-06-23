// PictureHlp.h: interface for the CPictureHlp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PICTUREHLP_H__84D3E504_630B_45D5_9F81_ED53E062F8FB__INCLUDED_)
#define AFX_PICTUREHLP_H__84D3E504_630B_45D5_9F81_ED53E062F8FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HIMETRIC_INCH	2540
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_INCH/2) / HIMETRIC_INCH )

class CPictureHlp  
{
public:
	CPictureHlp();
	virtual ~CPictureHlp();
	void LoadPictureFile(LPCTSTR szFile);
	void LoadPicture(int iID);
	void Display(CDC& dc, CRect& r, int iXdest = 0, int iYdest = 0);

private:
	LPPICTURE	m_pPicture;
};

#endif // !defined(AFX_PICTUREHLP_H__84D3E504_630B_45D5_9F81_ED53E062F8FB__INCLUDED_)
