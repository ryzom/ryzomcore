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

// Project.cpp: implementation of the CProject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ide2.h"
#include "Project.h"

#include "WorkspaceWnd.h"
#include "TreeViewFiles.h"
#include "LuaDoc.h"
#include "LuaView.h"
#include "MainFrame.h"
#include "ProjectProperties.h"
#include "ScintillaView.h"
#include "Executor.h"
#include "ProjectNew.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProject::CProject()
{
	SetModifiedFlag(FALSE);
	m_LineToBPModified = true;
}

CProject::~CProject()
{
	RemoveProjectFiles();
}

//--------------------------------------------------------------------------------------------------
//- file and directory functions
//--------------------------------------------------------------------------------------------------

CString CProject::GetName()
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( m_strPathName, drive, dir, fname, ext );
	return CString(fname);
}

CString CProject::GetNameExt()
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( m_strPathName, drive, dir, fname, ext );
	return CString(fname)+ext;
}

BOOL CProject::CreateIntermediateDir()
{
	DWORD Attrib=GetFileAttributes(m_strIntermediateDirRoot);
	if(Attrib == 0xFFFFFFFF) 
		return CreateDirectory(m_strIntermediateDirRoot, NULL); 
	else if(!(Attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
 
	return TRUE;
}

BOOL CProject::CreateOutputDir()
{
	DWORD Attrib=GetFileAttributes(m_strOutputDirRoot);
	if(Attrib == 0xFFFFFFFF) 
		return CreateDirectory(m_strOutputDirRoot, NULL);
	else if(!(Attrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
 
	return TRUE;
}

CString CProject::GetProjectDir()
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( m_strPathName, drive, dir, fname, ext );
	if ( dir[strlen(dir)-1]=='\\' )
		dir[strlen(dir)-1] = '\0';
	return CString(drive)+dir;	
}

//--------------------------------------------------------------------------------------------------
//- project files functions
//--------------------------------------------------------------------------------------------------

void CProject::RedrawFilesTree()
{
	CWorkspaceWnd* pWorkspace = ((CMainFrame*)AfxGetMainWnd())->GetWorkspaceWnd();
	CTreeViewFiles* pTree = pWorkspace->GetTreeViewFiles();
	pTree->RemoveAll();

	pTree->AddRoot(GetName());

	int nFiles = m_files.GetSize();
	for ( int i=0; i<nFiles; ++i )
	{
		m_files[i]->m_TreeItem = pTree->AddProjectFile(m_files[i]->GetNameExt(), (long)m_files[i]);
	}
}


CProjectFile* CProject::GetProjectFile(CString strPathName)
{
	int nSize = m_files.GetSize();
	for ( int i=0; i<nSize; ++i )
		if ( m_files[i]->HasFile(strPathName) )
			return m_files[i];

	return NULL;
}

CProjectFile* CProject::GetProjectFile(UINT index)
{
	assert(index < m_files.GetSize());	
	return m_files[index];
}


void CProject::AddFiles()
{
	CFileDialog fd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_ALLOWMULTISELECT, 
		"Lua files (*.lua)|*.lua|All files (*.*)|*.*||", AfxGetMainWnd());

	if ( fd.DoModal()==IDOK )
	{
		POSITION pos = fd.GetStartPosition();
		while (pos)
		{
			CString strPathName = fd.GetNextPathName(pos);
			AddFile(strPathName);
		}
	}

}

void CProject::AddFile(CString strPathName)
{
 	CProjectFile* pPF;
	if ( (pPF=GetProjectFile(strPathName)) != NULL )
		return;

	pPF = new CProjectFile;
	pPF->SetPathName(strPathName);

	AddFile(pPF);
}

void CProject::AddFile(CProjectFile* pPF)
{
	m_files.Add(pPF);

	CWorkspaceWnd* pWorkspace = ((CMainFrame*)AfxGetMainWnd())->GetWorkspaceWnd();
	CTreeViewFiles* pTree = pWorkspace->GetTreeViewFiles();
	pPF->m_TreeItem = pTree->AddProjectFile(pPF->GetNameExt(), (long)pPF);

	SetModifiedFlag(TRUE);
}

void CProject::RemoveFile(CProjectFile *pPF)
{
	int nSize = m_files.GetSize();
	for ( int i=0; i<nSize; ++i )
		if ( m_files[i] == pPF )
		{
			m_files.RemoveAt(i);
			break;
		}

	SetModifiedFlag(TRUE);
}

void CProject::RemoveProjectFiles()
{
	int nSize = m_files.GetSize();
	for ( int i=0; i<nSize; ++i )
	{
		CLuaView *pView = theApp.FindProjectFilesView(m_files[i]);
		if ( pView )
			pView->CloseFrame();
	}

	for ( i=0; i<nSize; ++i )
		delete m_files[i];

	m_files.RemoveAll();
}

void CProject::Properties()
{
	CProjectProperties dlg;

	dlg.m_bGenerateListing = m_bGenerateListing;
	dlg.m_strIntermediateDir = m_strIntermediateDir;
	dlg.m_strOutputDir = m_strOutputDir;
	dlg.m_strOutputPrefix = m_strOutputPrefix;

	if ( dlg.DoModal() == IDOK )
	{
		m_bGenerateListing = dlg.m_bGenerateListing;
		m_strIntermediateDir = dlg.m_strIntermediateDir;
		m_strOutputDir = dlg.m_strOutputDir;
		m_strOutputPrefix = dlg.m_strOutputPrefix;

		UpdateDirectories();
		SetModifiedFlag(TRUE);
	}
}

void CProject::UpdateDirectories()
{
	if ( !PathIsRelative(m_strOutputDir) )
	{
		PathRemoveBackslash(m_strOutputDir.GetBuffer(0));
		m_strOutputDir.ReleaseBuffer();
		m_strOutputDirRoot = m_strOutputDir;
	}
	else
	{
		m_strOutputDirRoot = GetProjectDir();
		PathAppend(m_strOutputDirRoot.GetBuffer(MAX_PATH), m_strOutputDir);
		m_strOutputDirRoot.ReleaseBuffer();
		PathRemoveBackslash(m_strOutputDirRoot.GetBuffer(0));
		m_strOutputDirRoot.ReleaseBuffer();
	}

	if ( !PathIsRelative(m_strIntermediateDir) )
	{
		PathRemoveBackslash(m_strIntermediateDir.GetBuffer(0));
		m_strIntermediateDir.ReleaseBuffer();
		m_strIntermediateDirRoot = m_strIntermediateDir;
	}
	else
	{
		m_strIntermediateDirRoot = GetProjectDir();
		PathAppend(m_strIntermediateDirRoot.GetBuffer(MAX_PATH), m_strIntermediateDir);
		m_strIntermediateDirRoot.ReleaseBuffer();
		PathRemoveBackslash(m_strIntermediateDirRoot.GetBuffer(0));
		m_strIntermediateDirRoot.ReleaseBuffer();
	}
}

//--------------------------------------------------------------------------------------------------
//- project new/save/load/close functions
//--------------------------------------------------------------------------------------------------

void CProject::NewProject()
{
	GetCurrentDirectory(MAX_PATH, m_strPathName.GetBuffer(MAX_PATH));
	m_strPathName.ReleaseBuffer();
	m_strPathName += "\\project1.lpr";

	m_strOutputDir = "Output";
	m_strIntermediateDir = "Intermediate";
	m_strOutputPrefix = "project1";
	m_bGenerateListing = FALSE;

	UpdateDirectories();
	RedrawFilesTree();

	SetModifiedFlag(FALSE);
}

BOOL CProject::New()
{
	SaveModified();

	CProjectNew pn;
	if ( pn.DoModal()!=IDOK )
		return FALSE;

	m_strPathName = pn.GetProjectPathName();
	m_strOutputPrefix = pn.GetProjectName();

	m_strOutputDir = "Output";
	m_strIntermediateDir = "Intermediate";
	m_bGenerateListing = FALSE;

	UpdateDirectories();
	RedrawFilesTree();

	pn.CreateByType(this);

	Save();
	SetModifiedFlag(FALSE);

	return TRUE;
}

void CProject::InitForExternalDebug(const char *tmpProjectFile)
{
	m_strPathName = tmpProjectFile;
	m_strOutputPrefix = tmpProjectFile;
	
	m_bGenerateListing = FALSE;

	UpdateDirectories();
	RedrawFilesTree();

	

	Save();
	SetModifiedFlag(FALSE);

}

BOOL CProject::Close()
{
	SaveModified();

	m_strPathName = "";

	RemoveProjectFiles();

	CWorkspaceWnd* pWorkspace = ((CMainFrame*)AfxGetMainWnd())->GetWorkspaceWnd();
	CTreeViewFiles* pTree = pWorkspace->GetTreeViewFiles();
	pTree->RemoveAll();

	return TRUE;
}

BOOL CProject::Load()
{
	SaveModified();

	CFileDialog fd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST, 
		"Project files (*.lpr)|*.lpr|All files (*.*)|*.*||", AfxGetMainWnd());

	if ( fd.DoModal()!=IDOK )
		return FALSE;

	CFile fin;
	if ( !fin.Open(fd.GetPathName(), CFile::modeRead) )
	{
		AfxMessageBox("Unable to open project file");
		return FALSE;
	}

	m_strPathName = fd.GetPathName();

	CArchive ar(&fin, CArchive::load);

	RemoveProjectFiles();
	
	BOOL bResult = Load(ar);

	SetModifiedFlag(FALSE);

	RedrawFilesTree();
	UpdateDirectories();

	return bResult;
}

BOOL CProject::Load(CArchive &ar)
{
	ar >> m_strPathName;
	ar >> m_strOutputDir;
	ar >> m_strIntermediateDir;
	ar >> m_strOutputPrefix;
	ar >> m_bGenerateListing;

	long nFiles;
	ar >> nFiles;

	for ( int i=0; i<nFiles; ++i )
	{
		CProjectFile *pPF = new CProjectFile;
		pPF->Load(ar);

		AddFile(pPF);
	}

	return TRUE;
}

BOOL CProject::Save()
{
	return Save(m_strPathName);
}

BOOL CProject::Save(CString strPathName)
{
	CFile fout;
	if ( !fout.Open(strPathName, CFile::modeCreate|CFile::modeWrite) )
	{
		AfxMessageBox("Unable to open project file");
		return FALSE;
	}

	CArchive ar(&fout, CArchive::store);

	BOOL bResult = Save(ar);

	SetModifiedFlag(FALSE);

	return bResult;
}

BOOL CProject::Save(CArchive &ar)
{
	ar << m_strPathName;
	ar << m_strOutputDir;
	ar << m_strIntermediateDir;
	ar << m_strOutputPrefix;
	ar << m_bGenerateListing;

	long nFiles = m_files.GetSize();
	ar << nFiles;

	for ( int i=0; i<nFiles; ++i )
		m_files[i]->Save(ar);

	return TRUE;
}

BOOL CProject::SaveAs()
{
	CFileDialog fd(FALSE, "lpr", m_strPathName, OFN_PATHMUSTEXIST, 
		"Project files (*.lpr)|*.lpr|All files (*.*)|*.*||", AfxGetMainWnd());

	if ( fd.DoModal()!=IDOK )
		return FALSE;

	m_strPathName = fd.GetPathName();

	BOOL bResult = Save(m_strPathName);

	UpdateDirectories();

	return bResult;
}


void CProject::SaveModified()
{
	if ( m_bModified )
	{
		if ( AfxMessageBox("Your project has changed. Do you want to save it?", MB_YESNO)==IDYES )
			Save();
	}
}

//--------------------------------------------------------------------------------------------------
//- compile/build functions
//--------------------------------------------------------------------------------------------------

BOOL CProject::PositionBreakPoints()
{
	BOOL bModified = FALSE;
	int nFiles = m_files.GetSize();
	for ( int i=0; i<nFiles; ++i )
	{
		if ( m_files[i]->PositionBreakPoints() )
		{
			bModified = TRUE;
			CLuaView* pView = theApp.FindProjectFilesView(m_files[i]);
			if ( pView )
				m_files[i]->SetBreakPointsIn(pView->GetEditor());
		}
	}

	if ( bModified )
		SetModifiedFlag(TRUE);

	return bModified;
}


BOOL CProject::BuildIntermediateFiles()
{
	long nFiles = m_files.GetSize();
	int nErrors = 0;
	for ( int i=0; i<nFiles; ++i )
	{
		if ( m_files[i]->IsModified() )
		{
			if ( !m_files[i]->Compile() )
				++nErrors;
		}
	}

	return nErrors;
}

BOOL CProject::CheckBeforeBuild()
{
	COutputWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	pWnd->SetActiveOutput(COutputWnd::outputBuild);
	CScintillaView* pOutput = pWnd->GetOutput(COutputWnd::outputBuild);

	pOutput->Clear();

	CString strMsg;
	strMsg.Format("------------------ Project %s -------------------\n", GetName());
	pOutput->Write(strMsg);

	if ( !CreateIntermediateDir() )
	{
		pOutput->Write("Cannot create intermediate directory\n");
		return FALSE;
	}

	if ( !CreateOutputDir() )
	{
		pOutput->Write("Cannot create output directory\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CProject::Compile(CProjectFile *pPF)
{
	if ( !CheckBeforeBuild() )
		return FALSE;

	return pPF->Compile();
}

BOOL CProject::BuildOutputFiles()
{
	COutputWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	CScintillaView* pOutput = pWnd->GetOutput(COutputWnd::outputBuild);

	CExecutor m_exe;
	CString strCmdLine, strOutput;

	long nFiles = m_files.GetSize();
	for ( int i=0; i<nFiles; ++i )
		if ( theApp.FirstFileIsNewer(m_files[i]->GetOutputPathNameExt(), GetDebugPathNameExt()) )
		{
			pOutput->Write("Linking\n");

			strCmdLine.Format("\"%s\" -o \"%s\"", 
				theApp.GetModuleDir() + "\\" + "luac.exe", GetDebugPathNameExt());

			long nFiles = m_files.GetSize();
			for ( int i=0; i<nFiles; ++i )
				strCmdLine += " \"" + m_files[i]->GetOutputPathNameExt() + "\"";


			m_exe.Execute(strCmdLine);
			strOutput = m_exe.GetOutputString();
			if ( !strOutput.IsEmpty() )
			{
				pOutput->Write(strOutput);
				return FALSE;
			}

			break;
		}

	if ( theApp.FirstFileIsNewer(GetDebugPathNameExt(), GetReleasePathNameExt()) )
	{
		strCmdLine.Format("\"%s\" -s -o \"%s\" \"%s\"", 
			theApp.GetModuleDir() + "\\" + "luac.exe", GetReleasePathNameExt(), GetDebugPathNameExt());

		m_exe.Execute(strCmdLine);
		strOutput = m_exe.GetOutputString();
		if ( !strOutput.IsEmpty() )
		{
			pOutput->Write(strOutput);
			return FALSE;
		}
	}

	if ( m_bGenerateListing && theApp.FirstFileIsNewer(GetDebugPathNameExt(), GetListingPathNameExt()) )
	{
		strCmdLine.Format("\"%s\" -p -l \"%s\"", 
			theApp.GetModuleDir() + "\\" + "luac.exe", GetDebugPathNameExt());

		m_exe.Execute(strCmdLine);
		m_exe.SaveOutput(GetListingPathNameExt());
	}

	pOutput->Write("Link complete\n");
	return TRUE;
}

BOOL CProject::Build()
{
	COutputWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	CScintillaView* pOutput = pWnd->GetOutput(COutputWnd::outputBuild);

	CString strMsg;
	strMsg.Format("------------------ Building project %s -------------------\n", GetName());
	pOutput->Write(strMsg);

	if ( !CheckBeforeBuild() )
		return FALSE;

	int nErrors = BuildIntermediateFiles();
	
	if ( nErrors!=0 )
	{
		strMsg.Format("%d error(s), build terminated\n", nErrors);
		pOutput->Write(strMsg);
		return FALSE;
	}

	return BuildOutputFiles();
}


void CProject::CleanIntermediateAndOutputFiles()
{
	COutputWnd* pWnd = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	CScintillaView* pOutput = pWnd->GetOutput(COutputWnd::outputBuild);

	pOutput->Clear();

	pOutput->Write("Deleting intermediate files\n");
	long nFiles = m_files.GetSize();
	for ( int i=0; i<nFiles; ++i )
		m_files[i]->DeleteIntermediateFiles();

	pOutput->Write("Deleting output files\n");

	DeleteFile(GetDebugPathNameExt());
	DeleteFile(GetReleasePathNameExt());
}



BOOL CProject::HasBreakPoint(const char* szFile,int nLine)
{
	CProjectFile *pPF = GetProjectFile(szFile);
	if ( pPF==NULL )
		return FALSE;

	return pPF->HasBreakPoint(nLine);
}

const BOOL CProject::GetBreakPoint(const char* szFile,int nLine,CProjectFile::CBreakPoint &bp)
{
	CProjectFile *pPF = GetProjectFile(szFile);
	if ( pPF==NULL )
		return FALSE;
	return pPF->GetBreakPoint(nLine, bp);
}


void CProject::SetModifiedFlag(BOOL bModified)
{
	m_bModified = bModified;
	m_LineToBPModified = TRUE;
}

BOOL CProject::IsBreakPointPossibleAtLine(int line)
{
	if (line < 0)
	{
		return false;
	}
	if (m_LineToBPModified)
	{
		m_BPVect.clear();
		std::vector<int> lineWithBP;
		for (int k = 0; k < m_files.GetSize(); ++k)
		{
			m_files[k]->GetBreakPointLines(lineWithBP);
			for (UINT l = 0; l < lineWithBP.size(); ++l)
			{
				if (lineWithBP[l] >= (int) m_BPVect.size())
				{
					m_BPVect.resize(lineWithBP[l] + 1, FALSE);
					m_BPVect[lineWithBP[l]] = TRUE;
				}
			}
		}
		m_LineToBPModified = false;
	}	
	if (line >= (int) m_BPVect.size()) return FALSE;
	return m_BPVect[line];
}
