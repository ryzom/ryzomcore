/*
 * Workaround from the MFC CFileDialog Multi Select bug
 * Thanks to Jens Bohlmann (bohly@ki.comcity.de)
 */

#include "file_dialog.h"

CMultiFileDialog::CMultiFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
    CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.lpstrFile = m_strFileName.GetBuffer(FILENAME_BUFFERSIZE);
	m_ofn.nMaxFile = FILENAME_BUFFERSIZE;
	m_ofn.Flags |= OFN_ALLOWMULTISELECT;
}
