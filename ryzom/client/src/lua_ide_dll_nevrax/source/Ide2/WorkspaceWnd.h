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

// WorkspaceWnd.h: interface for the CWorkspaceWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORKSPACEWND_H__1609FA7C_19A2_49B5_A86E_65E866932D97__INCLUDED_)
#define AFX_WORKSPACEWND_H__1609FA7C_19A2_49B5_A86E_65E866932D97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTreeViewFiles;

class CWorkspaceWnd : public CCJTabCtrlBar
{
public:
	void Enable(BOOL bEnable);
	CTreeViewFiles* GetTreeViewFiles();
	virtual BOOL Create(CWnd* pParentWnd, UINT nID, LPCTSTR lpszWindowName = NULL, CSize sizeDefault = CSize(200,100), DWORD dwStyle = CBRS_LEFT);
	CWorkspaceWnd();
	virtual ~CWorkspaceWnd();

protected:
	CImageList	m_tabImages;

};

#endif // !defined(AFX_WORKSPACEWND_H__1609FA7C_19A2_49B5_A86E_65E866932D97__INCLUDED_)
