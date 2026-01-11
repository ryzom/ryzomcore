// PictureHlp.cpp: implementation of the CPictureHlp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nel_launcher.h"
#include "PictureHlp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPictureHlp::CPictureHlp()
{
	m_pPicture	= NULL;
}

CPictureHlp::~CPictureHlp()
{
	if(m_pPicture)
	{
		m_pPicture->Release();
		m_pPicture	= NULL;
	}
}

void CPictureHlp::LoadPictureFile(LPCTSTR szFile)
{
	// open file
	HANDLE hFile	= CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if(!hFile)
	{
		CString csMsg	= "Cannot load picture file ";

		AfxMessageBox(csMsg + " " + szFile + "!");
		return;
	}

	// get file size
	DWORD dwFileSize = GetFileSize(hFile, NULL);

	LPVOID pvData = NULL;
	// alloc memory based on file size
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

	pvData = GlobalLock(hGlobal);

	DWORD dwBytesRead = 0;
	// read file and store in global memory
	BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
	GlobalUnlock(hGlobal);
	CloseHandle(hFile);

	LPSTREAM pstm = NULL;
	// create IStream* from global memory
	HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);

	// Create IPicture from image file
	if(m_pPicture)
		m_pPicture->Release();

	hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&m_pPicture);
	pstm->Release();
}

void CPictureHlp::LoadPicture(int iID)
{
	// Loading the resource
	HINSTANCE	hInst		= AfxGetInstanceHandle();
	HRSRC		hmdbFile	= ::FindResource(hInst, MAKEINTRESOURCE(iID), "PICTURE_RESOURCE");
	if(!hmdbFile)
	{
		AfxMessageBox("FindResource failed");
		return;
	}
	HGLOBAL		hRes	= ::LoadResource(hInst, hmdbFile);

	if(hRes)
	{
		DWORD		dwResSize	= ::SizeofResource(hInst, hmdbFile);
		UINT FAR*	lpnRes		= (UINT FAR*)::LockResource(hRes);
		
		LPVOID pvData = NULL;
		// alloc memory based on file size
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwResSize);

		pvData = GlobalLock(hGlobal);
		memcpy(pvData, lpnRes, dwResSize);
		GlobalUnlock(hGlobal);

		LPSTREAM pstm = NULL;
		// create IStream* from global memory
		HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);

		// Create IPicture from image file
		if(m_pPicture)
			m_pPicture->Release();

		hr = ::OleLoadPicture(pstm, dwResSize, FALSE, IID_IPicture, (LPVOID *)&m_pPicture);
		pstm->Release();

		::FreeResource(hRes);
	}
	else
		AfxMessageBox("LoadResource failed");
}

void CPictureHlp::Display(CDC& dc, CRect& r, int iXdest, int iYdest) 
{
	if(m_pPicture)
	{
		// get width and height of picture
		long	hmWidth, hmHeight;
		int		nWidth, nHeight;

		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);
		
		// convert himetric to pixels
		nWidth	= MulDiv(hmWidth, GetDeviceCaps(dc.m_hDC, LOGPIXELSX), HIMETRIC_INCH);
		nHeight	= MulDiv(hmHeight, GetDeviceCaps(dc.m_hDC, LOGPIXELSY), HIMETRIC_INCH);

		// display picture using IPicture::Render
		m_pPicture->Render(dc.m_hDC, iXdest, iYdest, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, &r);
	}
}
