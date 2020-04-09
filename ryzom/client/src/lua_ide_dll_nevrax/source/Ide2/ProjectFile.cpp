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

// ProjectFile.cpp: implementation of the CProjectFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "ProjectFile.h"

#include "LuaEditor.h"
#include "MainFrame.h"
#include "Executor.h"
#include "ScintillaView.h"
#include "LuaHelper.h"
#include "TreeViewFiles.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectFile::CProjectFile()
{
	RemoveAllDebugLines();
	RemoveAllBreakPoints();

	GetSystemTime(&m_timeCompiled);
	m_Modified = FALSE;
}

CProjectFile::~CProjectFile()
{

}

void CProjectFile::RemoveAllDebugLines()
{
	m_nMinDebugLine = 2147483647;
	m_nMaxDebugLine = 0;

	m_debugLines.RemoveAll();
}

void CProjectFile::AddDebugLine(int nLine)
{
	m_debugLines[nLine] = 1;
	if ( nLine<m_nMinDebugLine )
		m_nMinDebugLine = nLine;
	if ( nLine>m_nMaxDebugLine )
		m_nMaxDebugLine = nLine;
}

void CProjectFile::RemoveAllBreakPoints()
{
	m_nMinBreakPoint = 2147483647;
	m_nMaxBreakPoint = 0;

	m_breakPoints.RemoveAll();

	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	pProject->SetModifiedFlag(TRUE);
}

void CProjectFile::AddBreakPoint(int nLine)
{
	AddBreakPoint(CBreakPoint(nLine));	
}

void CProjectFile::AddBreakPoint(const CBreakPoint &bp)
{
	m_breakPoints[bp.m_Line] = bp;
	if ( bp.m_Line<m_nMinBreakPoint)
		m_nMinBreakPoint = bp.m_Line;
	if ( bp.m_Line>m_nMaxBreakPoint )
		m_nMaxBreakPoint = bp.m_Line;

	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	pProject->SetModifiedFlag(TRUE);
}


void CProjectFile::RemoveBreakPoint(int nLine)
{
	m_breakPoints.RemoveKey(nLine);

	m_nMinBreakPoint = 2147483647;
	m_nMaxBreakPoint = 0;

	POSITION pos = m_breakPoints.GetStartPosition();
	CBreakPoint nTemp;
	while (pos != NULL)
	{
		m_breakPoints.GetNextAssoc( pos, nLine, nTemp );
		if ( nLine<m_nMinBreakPoint)
			m_nMinBreakPoint = nLine;
		if ( nLine>m_nMaxBreakPoint )
			m_nMaxBreakPoint = nLine;
	}

	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	pProject->SetModifiedFlag(TRUE);
}

int CProjectFile::GetNextDebugLine(int nLine)
{
	int nTemp;
	++nLine;

	while ( nLine<=m_nMaxDebugLine )
		if ( m_debugLines.Lookup(nLine, nTemp) )
			return nLine;
		else
			++nLine;

	return 0;
}

int CProjectFile::GetPreviousDebugLine(int nLine)
{
	int nTemp;
	--nLine;

	while ( nLine>=m_nMinDebugLine )
		if ( m_debugLines.Lookup(nLine, nTemp) )
			return nLine;
		else
			--nLine;

	return 0;
}

int CProjectFile::GetNearestDebugLine(int nLine)
{
	int nTemp, nNearest;
	if ( m_debugLines.Lookup(nLine, nTemp) )
		return nLine;

	if ( (nNearest=GetNextDebugLine(nLine)) > 0 )
		return nNearest;

	if ( (nNearest=GetPreviousDebugLine(nLine)) > 0 )
		return nNearest;

	return 0;
}

BOOL CProjectFile::PositionBreakPoints()
{
	if ( !CLuaHelper::LoadDebugLines(this) )
		return FALSE;

	BOOL bModified = FALSE;
	POSITION pos = m_breakPoints.GetStartPosition();
	CBreakPoint temp;
	int nLine, nNearest;
	while (pos != NULL)
	{
		m_breakPoints.GetNextAssoc( pos, nLine, temp );
		nNearest = GetNearestDebugLine(nLine);
		if ( nNearest == 0 )
		{
			m_breakPoints.RemoveKey(nLine);
			bModified = TRUE;
		}
		else if ( nLine != nNearest )
		{
			m_breakPoints.RemoveKey(nLine);
			m_breakPoints.SetAt(nNearest, 1);
			bModified = TRUE;
		}
	}

	return bModified;
}

CString CProjectFile::GetName()
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( m_strPathName, drive, dir, fname, ext );
	return CString(fname);
}

CString CProjectFile::GetNameExt()
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( m_strPathName, drive, dir, fname, ext );
	return CString(fname)+ext;
}


BOOL CProjectFile::GetBreakPoint(int nLine, CBreakPoint &bp)
{
	return m_breakPoints.Lookup(nLine, bp);
}

BOOL CProjectFile::HasBreakPoint(int nLine)
{
	CBreakPoint bp;
	return m_breakPoints.Lookup(nLine, bp);
}


void CProjectFile::SetBreakPointsIn(CLuaEditor *pEditor)
{
	pEditor->ClearAllBreakpoints();

	POSITION pos = m_breakPoints.GetStartPosition();
	int nLine;
	CBreakPoint temp;
	while (pos != NULL)
	{
		m_breakPoints.GetNextAssoc( pos, nLine, temp );
		pEditor->SetBreakpoint(nLine);
	}
}

BOOL CProjectFile::HasFile(CString strPathName)
{
	// TMP TMP : compare with no path only ...	
	char fnamel[_MAX_FNAME];
	char fextl[_MAX_FNAME];
	char fnamer[_MAX_FNAME];
	char fextr[_MAX_FNAME];
							
	_splitpath((LPCTSTR) strPathName, NULL, NULL, fnamel, fextl);
	_splitpath((LPCTSTR) m_strPathName, NULL, NULL, fnamer, fextr);
	if (_stricmp(fextl, fextr) == 0 && _stricmp(fnamel, fnamer) == 0)
		return TRUE;

	return FALSE;

	/*
 	if(!m_strPathName.CompareNoCase(strPathName))
 		return TRUE;
 
 	//should actually search using the luasearch path
 	DWORD n=MAX_PATH;
 	CString sFullPath;	
 	::GetFullPathName(strPathName,n,sFullPath.GetBuffer(n),NULL);
 	sFullPath.ReleaseBuffer();
 
 	if(!m_strPathName.CompareNoCase(sFullPath))
 		return TRUE;
 	return FALSE;
	*/
}


BOOL CProjectFile::Load(CArchive &ar)
{
	RemoveAllDebugLines();
	RemoveAllBreakPoints();

	ar >> m_strRelPathName;

	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();
	m_strPathName = pProject->GetProjectDir();
	PathAppend(m_strPathName.GetBuffer(MAX_PATH), m_strRelPathName);

	ar >> m_nMinBreakPoint;
	ar >> m_nMaxBreakPoint;

	int nBreakPoints;
	ar >> nBreakPoints;

	for ( int i=0; i<nBreakPoints; ++i )
	{
		int nLine;
		ar >> nLine;

		m_breakPoints[nLine] = 1;
	}

	return TRUE;
}

BOOL CProjectFile::Save(CArchive &ar)
{
	ar << m_strRelPathName;
	ar << m_nMinBreakPoint;
	ar << m_nMaxBreakPoint;

	int nBreakPoints = m_breakPoints.GetCount();
	ar << nBreakPoints;

	POSITION pos = m_breakPoints.GetStartPosition();
	int nLine;
	CBreakPoint temp;
	while (pos != NULL)
	{
		m_breakPoints.GetNextAssoc( pos, nLine, temp );
		ar << nLine;
	}

	return TRUE;
}

BOOL CProjectFile::IsModified()
{
	WIN32_FILE_ATTRIBUTE_DATA sourceFile, compiledFile;

	if (! ::GetFileAttributesEx(m_strPathName, GetFileExInfoStandard, &sourceFile) )
		return TRUE;

	if (! ::GetFileAttributesEx(GetOutputPathNameExt(), GetFileExInfoStandard, &compiledFile) )
		return TRUE;

	ULARGE_INTEGER sourceTime, compiledTime;
	sourceTime.LowPart = sourceFile.ftLastWriteTime.dwLowDateTime;
	sourceTime.HighPart = sourceFile.ftLastWriteTime.dwHighDateTime;
	compiledTime.LowPart = compiledFile.ftLastWriteTime.dwLowDateTime;
	compiledTime.HighPart = compiledFile.ftLastWriteTime.dwHighDateTime;

	return ( sourceTime.QuadPart > compiledTime.QuadPart );
}

BOOL CProjectFile::Compile()
{
	CExecutor m_exe;

	COutputWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	CScintillaView* pOutput = pWnd->GetOutput(COutputWnd::outputBuild);

	pOutput->Write(GetNameExt() + "\n");

	CString strCmdLine;
	strCmdLine.Format("\"%s\" -o \"%s\" \"%s\"", 
		theApp.GetModuleDir() + "\\" + "luac.exe", GetOutputPathNameExt(), GetPathName());

	m_exe.Execute(strCmdLine);
	CString strOutput = m_exe.GetOutputString();
	if ( !strOutput.IsEmpty() )
	{
		pOutput->Write(strOutput);
		return FALSE;
	}

	return TRUE;
}

void CProjectFile::DeleteIntermediateFiles()
{
	DeleteFile(GetOutputPathNameExt());
}

CString CProjectFile::GetOutputPathNameExt()
{
	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();	
	return pProject->GetIntermediateDir() + "\\" + GetOutputNameExt();
}

void CProjectFile::UpdateRelPathName()
{
	CProject* pProject = ((CMainFrame*)AfxGetMainWnd())->GetProject();

	PathRelativePathTo(m_strRelPathName.GetBuffer(MAX_PATH), 
		pProject->GetProjectDir(), FILE_ATTRIBUTE_DIRECTORY,
		m_strPathName, 0);
	m_strRelPathName.ReleaseBuffer();
}

void CProjectFile::GetBreakPointLines(std::vector<int> &dest) const
{	
	POSITION p = m_breakPoints.GetStartPosition();
	while (p)
	{
		int key;
		CBreakPoint value;
		m_breakPoints.GetNextAssoc(p, key, value);		
		dest.push_back(key);		
	}
}

void CProjectFile::GetBreakPoints(std::vector<CBreakPoint> &dest) const
{	
	POSITION p = m_breakPoints.GetStartPosition();
	while (p)
	{
		int key;
		CBreakPoint value;
		m_breakPoints.GetNextAssoc(p, key, value);		
		dest.push_back(value);		
	}
}


void CProjectFile::SetPathName(CString strPathName)
{
	m_strPathName=strPathName;
	UpdateRelPathName();	
}

void CProjectFile::SetModifiedFlag(BOOL modified)
{
	if (modified == m_Modified) return;		
	m_Modified = modified;
	CString name = GetNameExt();
	if (m_Modified)
	{
		name +=" *";
	}
	theApp.GetMainFrame()->GetWorkspaceWnd()->GetTreeViewFiles()->GetTreeCtrl().SetItemText(m_TreeItem, name);
}

