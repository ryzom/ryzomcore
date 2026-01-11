// Configuration.h: interface for the CConfiguration class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGURATION_H__F92DFBDE_7EE5_4077_8D8D_ADDCBE3C302E__INCLUDED_)
#define AFX_CONFIGURATION_H__F92DFBDE_7EE5_4077_8D8D_ADDCBE3C302E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConfiguration  
{
public:
	CConfiguration();
	virtual ~CConfiguration();
	BOOL	Load();
	BOOL	GetValue(CString& csBuffer, CString csKey, CString& csValue, int iIndex = 0);

public:
	CString	m_csHost;
	double	m_dVersion;
	CString	m_csUrlMain;
	CString	m_csUrlRN;
	CString	m_csUrlNews;
	CString	m_csApp;
	CString	m_csExe;
	CString	m_csBasePath;
	CString	m_csAppBasePath;

private:
	void	LoadFromResource();
};

#endif // !defined(AFX_CONFIGURATION_H__F92DFBDE_7EE5_4077_8D8D_ADDCBE3C302E__INCLUDED_)
