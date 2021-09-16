/*
 * Workaround from the MFC CFileDialog Multi Select bug
 * Thanks to Jens Bohlmann (bohly@ki.comcity.de)
 */

#include "afxdlgs.h"

#define FILENAME_BUFFERSIZE 64000

class CMultiFileDialog : public CFileDialog
{
public:

	CMultiFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
					 DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd);

private:

	CString m_strFileName;
};
