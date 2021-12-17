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

// Executor.h: interface for the CExecutor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXECUTOR_H__B467D883_9302_4A7E_810B_058B60881E4A__INCLUDED_)
#define AFX_EXECUTOR_H__B467D883_9302_4A7E_810B_058B60881E4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CExecutor  
{
public:
	BOOL SaveOutput(CString strPathName);
	CString GetOutputString();
	CExecutor();
	virtual ~CExecutor();
	BOOL Execute(CString strCmdLine, CMemFile* pInput=NULL);

protected:
	void Close();
	void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut, HANDLE hChildStdIn, HANDLE hChildStdErr,
											 CString strCmdLine);
	BOOL Write(LPBYTE lpData, int nSize);
	static DWORD WINAPI ReadAndHandleOutput(LPVOID lpvThreadParam);

	HANDLE m_hOutputRead, m_hInputWrite;
	HANDLE m_hThread;
	CMemFile m_output;
};

#endif // !defined(AFX_EXECUTOR_H__B467D883_9302_4A7E_810B_058B60881E4A__INCLUDED_)
