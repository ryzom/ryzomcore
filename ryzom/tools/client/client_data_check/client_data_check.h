// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// client_data_check.h : main header file for the CLIENT_DATA_CHECK application
//

#if !defined(AFX_CLIENT_DATA_CHECK_H__3B7BC465_4902_4E07_B8B2_A5240E493164__INCLUDED_)
#define AFX_CLIENT_DATA_CHECK_H__3B7BC465_4902_4E07_B8B2_A5240E493164__INCLUDED_

#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
namespace NL3D
{
	class CMeshMRM;
	class CMeshMRMSkinned;
}

// main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkApp:
// See client_data_check.cpp for the implementation of this class
//

class CClient_data_checkApp : public CWinApp
{
public:
	CClient_data_checkApp();

	void	cancel();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClient_data_checkApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClient_data_checkApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	class CClient_data_checkDlg	*_CheckDlg;
	enum	TState
	{
		Init= 0,
		CheckSum,
		CancelCheckSum,
		MRMCheck,
		NearEnd,
		End,
	};
	TState						_StateProcess;
	ucstring					_LastDataScanLog;

	// MRM Check
	std::vector<std::string>			_BnpList;
	std::vector<std::string>			_CurrentBnpFileList;
	uint32						_BnpIndex;
	uint32						_FileIndex;

	void		setLog();
	void		addLog(const std::string &logAdd);
	void		setStatus(const std::string &status);

	bool		processBnpMRM();

	void		checkMeshMrm(const std::string &fileName, NL3D::CMeshMRM *meshMrm);
	void		checkMeshMrmSkinned(const std::string &fileName, NL3D::CMeshMRMSkinned	*meshMrmSkinned);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENT_DATA_CHECK_H__3B7BC465_4902_4E07_B8B2_A5240E493164__INCLUDED_)
