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

// mission_compiler_feDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mission_compiler_fe.h"
#include "mission_compiler_feDlg.h"
#include "CompilDialog.h"

#include "nel/misc/path.h"
#include "nel/ligo/primitive.h"
#include "../mission_compiler_lib/mission_compiler.h"
#include "nel/misc/config_file.h"
#include "AddPathDlg.h"

using namespace NLMISC;
using namespace NLLIGO;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeDlg dialog

CMissionCompilerFeDlg::CMissionCompilerFeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMissionCompilerFeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissionCompilerFeDlg)
	m_filter = _T("");
	m_dataDirectory = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMissionCompilerFeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissionCompilerFeDlg)
	DDX_Control(pDX, IDC_VALIDATE, m_validateBtn);
	DDX_Control(pDX, IDC_COMPILE, m_compileBtn);
	DDX_Control(pDX, IDC_PUBLISH, m_publishBtn);
	DDX_Control(pDX, IDC_LIST_DST, m_listDst);
	DDX_Control(pDX, IDC_LIST_SRC, m_listSrc);
	DDX_Text(pDX, IDC_FILTER, m_filter);
	DDX_Text(pDX, IDC_DATA_DIRECTORY, m_dataDirectory);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMissionCompilerFeDlg, CDialog)
	//{{AFX_MSG_MAP(CMissionCompilerFeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADD_ALL, OnAddAll)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_EN_CHANGE(IDC_FILTER, OnChangeFilter)
	ON_BN_CLICKED(IDC_COMPILE, OnCompile)
	ON_COMMAND(ID_SPECIAL_RUNCOMPILERTEST, OnSpecialRuncompilertest)
	ON_LBN_DBLCLK(IDC_LIST_SRC, OnDblclkListSrc)
	ON_LBN_DBLCLK(IDC_LIST_DST, OnDblclkListDst)
	ON_BN_CLICKED(IDC_PUBLISH, OnPublish)
	ON_COMMAND(ID_SPECIAL_VALIDATE_MISSIONS, OnSpecialValidateMissions)
	ON_BN_CLICKED(IDC_VALIDATE, OnValidate)
	ON_BN_CLICKED(IDC_CHANGE_DIR, OnChangeDir)
	ON_EN_CHANGE(IDC_DATA_DIRECTORY, OnChangeDataDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionCompilerFeDlg message handlers

BOOL CMissionCompilerFeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	switch(Mode)
	{
	case mode_publish:
		m_publishBtn.EnableWindow(TRUE);
		m_compileBtn.EnableWindow(FALSE);
		m_validateBtn.EnableWindow(FALSE);
		break;
	case mode_compile:
		m_publishBtn.EnableWindow(FALSE);
		m_compileBtn.EnableWindow(TRUE);
		m_validateBtn.EnableWindow(TRUE);
		break;

	}

	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	if (!readConfigFile())
		return TRUE;

	// Initial Data directory
	m_dataDirectory = CPath::standardizeDosPath(SearchPaths[0]).c_str();

	// Print a warning if there is more than one path
	if (SearchPaths.size() > 1)
	{
		CWnd* pWnd = GetDlgItem(IDC_PATH_WARNING);
		pWnd->SetWindowText("Warning ! config file contains more than one path ! Only the first is shown ! ");
	}

	UpdateData(FALSE);
	fillSourceList();
	CenterWindow();
	ShowWindow(SW_SHOW);
	updateFileList();

	// Check if the file "tmptool.txt" exists in system temp directory
	// It's that way world_editor pass arguments to the program
	// The file contains a list of primitives to select
	char tmpPath[MAX_PATH];
	GetEnvironmentVariable("TMP", tmpPath, MAX_PATH);
	strcat(tmpPath, "\\tmptool.txt");

	if (NLMISC::CFile::fileExists(tmpPath))
	{
		FILE *f = fopen(tmpPath, "r");
		if (f == NULL)
		{
			nlinfo("Can't open the file for reading !\n%s", tmpPath);
			return TRUE;
		}

		// read the list of selected primitives from world_editor
		vector<string> files;
		char filePath[MAX_PATH];
		while (fgets(filePath, MAX_PATH, f))
		{
			// remove endline
			filePath[strlen(filePath)-1] = '\0';
			files.push_back(filePath);
		}

		fclose(f);

		// delete temp file
		NLMISC::CFile::deleteFile(tmpPath);

		// Test to check if the primitive is in src list
		// If it is, erase it form src and add it to dest
		for (uint i=0 ; i<files.size() ; i++)
		{
			// search the file in src list
			string filename = NLMISC::CFile::getFilename(files[i]);
			TFileList::iterator it = _SrcList.find(filename);

			if (it != _SrcList.end())
			{
				// We found the same filename : check the path
				string srcPath = it->second;
				srcPath  = toUpper(CPath::standardizeDosPath(srcPath));
				files[i] = toUpper(CPath::standardizeDosPath(files[i]));
				if (srcPath != files[i])
				{
					::MessageBox(NULL, "Primitive path and working directory are not the same !",
									   "Mission compiler", MB_OK|MB_ICONEXCLAMATION);
				}
				else
				{
					_DstList.insert(make_pair(filename, files[i]));
					_SrcList.erase(filename);
				}
			}
			else
			{
				char buffer[1024];
				sprintf(buffer, "Can't find primitive in the directory !\n%s\n", files[i].c_str(), m_dataDirectory);
				::MessageBox(NULL, buffer, "Mission compiler", MB_OK|MB_ICONEXCLAMATION);
			}
		}

		updateFileList();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMissionCompilerFeDlg::fillSourceList()
{
	CAddPathDlg addPath;

	CPath::clearMap();
	_SrcList.clear();

	addPath.Create(IDD_DIALOG_ADD_PATH, this);
	addPath.CenterWindow();
	addPath.ShowWindow(SW_SHOW);

	string log(addPath.m_addPathLog);
	log += "Reading config file\r\n";
	addPath.m_addPathLog = log.c_str();
	addPath.UpdateData(FALSE);
	addPath.RedrawWindow();

	// Fill the src list with available primitives
	for (uint i=0; i<SearchPaths.size(); ++i)
	{
		log = addPath.m_addPathLog;
		log += SearchPaths[i]+"\r\n";
		addPath.m_addPathLog = log.c_str();
		addPath.UpdateData(FALSE);
		addPath.RedrawWindow();
		CPath::addSearchPath(SearchPaths[i], true, false);
	}
	addPath.DestroyWindow();
	vector<string>	paths;
	CPath::getFileList("primitive", paths);
	
	// fill the src file list
	set<string> files;
	for (uint i=0; i<paths.size(); ++i)
	{
		string path = CPath::lookup(paths[i]);
		_SrcList.insert(make_pair(NLMISC::CFile::getFilename(paths[i]), path));
	}
}

bool CMissionCompilerFeDlg::readConfigFile()
{
	// Read configuration file
	CConfigFile cf;
	cf.load("mission_compiler.cfg");

	CConfigFile::CVar *var;
	var = cf.getVarPtr("SearchPath");
	if (var)
	{
		for (uint i=0; i<var->size(); ++i)
			SearchPaths.push_back(var->asString(i));
		updateSearchPath();
	}
	else
	{
		AfxMessageBox("Can't find configuration var 'SearchPath', fatal", MB_OK);
		PostQuitMessage(-1);
		return false;
	}

	var = cf.getVarPtr("LigoConfig");
	if (var)
		LigoConfigFile = var->asString();
	else
	{
		AfxMessageBox("Can't find configuration var 'ligo_config', fatal", MB_OK);
		PostQuitMessage(-1);
		return false;
	}

	var = cf.getVarPtr("DefaultFilter");
	if (var)
		DefaultFilter = var->asString();

	m_filter = DefaultFilter.c_str();

	var = cf.getVarPtr("TestPrimitive");
	if (var)
		TestPrimitive = var->asString();
	else
		TestPrimitive = "test_compilateur.primitive";

	var = cf.getVarPtr("TestPrimitiveDest");
	if (var)
		TestPrimitiveDest = var->asString();
	else
		TestPrimitiveDest = "test_compilateur_gn.primitive";

	var = cf.getVarPtr("ReferenceScript");
	if (var)
		ReferenceScript = var->asString();
	else
		ReferenceScript = "test_compilateur.script";

	var = cf.getVarPtr("LocalTextPath");
	if (var)
		LocalTextPath = var->asString();

	// read data to publish
	CConfigFile::CVar *pathsPrim = cf.getVarPtr("ServerPathPrim");
	CConfigFile::CVar *pathsText = cf.getVarPtr("ServerPathText");
	CConfigFile::CVar *names = cf.getVarPtr("ServerName");
	if (pathsPrim && pathsText && names)
	{
		if ((pathsPrim->size() != names->size()) || (pathsText->size() != names->size()))
		{
			AfxMessageBox("Config file : ServerPathPrim, ServerPathText and ServerName are different in size !", MB_OK);
			PostQuitMessage(-1);
			return false;
		}

		if (pathsPrim->size() > 0)
			GetDlgItem(IDC_PUBLISH)->EnableWindow(TRUE);

		// activate servers if present in config file
		for (uint i=0 ; i<pathsPrim->size() ; i++)
		{
			CWnd* pWnd = GetDlgItem(IDC_CHECK_SRV1 + i);
			pWnd->EnableWindow(TRUE);
			pWnd->SetWindowText(names->asString(i).c_str());

			ServerName.push_back(names->asString(i));
			ServerPathPrim.push_back(pathsPrim->asString(i));
			ServerPathText.push_back(pathsText->asString(i));
		}
	}

	// init ligo
	NLLIGO::Register();
	LigoConfig.readPrimitiveClass(LigoConfigFile.c_str(), false);
	CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;

	return true;
}

void CMissionCompilerFeDlg::updateFileList()
{
	while (m_listSrc.GetCount() != 0)
		m_listSrc.DeleteString(0);
	while (m_listDst.GetCount() != 0)
		m_listDst.DeleteString(0);

	// fill the src list
	{
		TFileList::iterator first(_SrcList.begin()), last(_SrcList.end());
		for (uint i=0; first != last; ++first)
		{
			if (m_filter.GetLength() == 0)
				m_listSrc.InsertString(i++, first->first.c_str());
			else
			{
				// check the filter
				string filter(m_filter.LockBuffer());
				m_filter.UnlockBuffer();

				if (first->first.find(filter) != string::npos)
					m_listSrc.InsertString(i++, first->first.c_str());
			}
		}
	}
	// fill the dst list
	{
		TFileList::iterator first(_DstList.begin()), last(_DstList.end());
		for (uint i=0; first != last; ++first,++i)
		{
			m_listDst.InsertString(i, first->first.c_str());
		}
	}
}

void CMissionCompilerFeDlg::compile(BOOL publish)
{
	CCompilDialog dlg;
	dlg.Create(IDD_COMPIL, this);
	EnableWindow(FALSE);
	dlg.m_okBtn.EnableWindow(FALSE);
	dlg.ShowWindow(SW_SHOW);
	string	compileLog;
	uint nbMission = 0;
	TFileList::iterator first(_DstList.begin()), last(_DstList.end());
	for (; first != last; ++first)
	{
		compileLog += "Compiling '"+first->first+"'...\r\n";
		dlg.m_compileLog = compileLog.c_str();
		dlg.UpdateData(FALSE);
		dlg.RedrawWindow();

		NLLIGO::CPrimitives primDoc;
		CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		NLLIGO::loadXmlPrimitiveFile(primDoc, first->second, LigoConfig);
		CPrimitiveContext::instance().CurrentPrimitive = NULL;
		try
		{
			CMissionCompiler mc;
			mc.compileMissions(primDoc.RootNode, first->first);
			compileLog += toString("Found %u valid missions\r\n",mc.getMissionsCount());
			dlg.m_compileLog = compileLog.c_str();
			dlg.UpdateData(FALSE);
			dlg.RedrawWindow();
			mc.installCompiledMission(LigoConfig, first->first);
			nbMission += mc.getMissionsCount();

			// publish files to selected servers
			if (publish)
			for (uint i=0 ; i<ServerPathPrim.size() ; i++)
			{
				if (IsDlgButtonChecked(IDC_CHECK_SRV1 + i) != BST_CHECKED)
					continue;

				compileLog += toString("\r\nPublishing to %s ...\r\n", ServerName[i].c_str());
				for (uint j=0 ; j<mc.getFileToPublishCount() ; j++)
					compileLog += toString("   %s\r\n", (NLMISC::CFile::getFilename(mc.getFileToPublish(j))).c_str());
				mc.publishFiles(ServerPathPrim[i], ServerPathText[i], LocalTextPath);
			}
		}
		catch(EParseException e)
		{
			string msg;
			msg + "\r\n";
			if (e.Primitive != NULL)
			{
				msg += "In '"+buildPrimPath(e.Primitive)+"'\r\n";
			}
			msg += "Error while compiling '"+first->first+"' :\r\n"+e.Why+"\r\n";
			compileLog += msg;
			dlg.m_compileLog = compileLog.c_str();
			dlg.UpdateData(FALSE);
//			dlg.m_compileLogCtrl.SetSel(compileLog.size(), compileLog.size(), FALSE);
			dlg.RedrawWindow();
			AfxMessageBox(msg.c_str());
			break;
		}
	}
	compileLog += toString("\r\nCompiled and installed %u missions", nbMission);
	dlg.m_compileLog = compileLog.c_str();
	dlg.UpdateData(FALSE);
	dlg.m_compileLogCtrl.SetSel(compileLog.size(), compileLog.size(), FALSE);
	dlg.RedrawWindow();

	dlg.m_okBtn.EnableWindow(TRUE);
	dlg.RunModalLoop();
	dlg.DestroyWindow();
	EnableWindow(TRUE);
}

void CMissionCompilerFeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMissionCompilerFeDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMissionCompilerFeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMissionCompilerFeDlg::OnAdd() 
{
	// read all selected prim in src, move them to dst
	vector<int>	sel;
	sel.resize(m_listSrc.GetSelCount());
	m_listSrc.GetSelItems(sel.size(), &sel[0]);

	for (uint i=0; i<sel.size(); ++i)
	{
		CString tmp;
		m_listSrc.GetText(sel[i], tmp);
		string file(tmp.LockBuffer());
		tmp.UnlockBuffer();
		_DstList.insert(make_pair(file, _SrcList[file]));
		_SrcList.erase(file);
	}

	updateFileList();

}

void CMissionCompilerFeDlg::OnAddAll() 
{
	while (m_listSrc.GetCount() != 0)
	{
		CString tmp;
		m_listSrc.GetText(0, tmp);
		string file(tmp.LockBuffer());
		tmp.UnlockBuffer();

		_DstList.insert(make_pair(file, _SrcList[file]));
		_SrcList.erase(file);

		m_listSrc.DeleteString(0);
	}

	updateFileList();
}

void CMissionCompilerFeDlg::OnRemove() 
{
	// read all selected prim in src, move them to dst
	vector<int>	sel;
	sel.resize(m_listDst.GetSelCount());
	m_listDst.GetSelItems(sel.size(), &sel[0]);

	for (uint i=0; i<sel.size(); ++i)
	{
		CString tmp;
		m_listDst.GetText(sel[i], tmp);
		string file(tmp.LockBuffer());
		tmp.UnlockBuffer();
		_SrcList.insert(make_pair(file, _DstList[file]));
		_DstList.erase(file);
	}

	updateFileList();
}

void CMissionCompilerFeDlg::OnRemoveAll() 
{
	_SrcList.insert(_DstList.begin(), _DstList.end());
	_DstList.clear();

	updateFileList();
}

void CMissionCompilerFeDlg::OnChangeFilter() 
{
	UpdateData(TRUE);
	updateFileList();
}

void CMissionCompilerFeDlg::OnCompile() 
{
	compile(FALSE);
}

void CMissionCompilerFeDlg::OnDblclkListSrc() 
{
	OnAdd();	
}

void CMissionCompilerFeDlg::OnDblclkListDst() 
{
	OnRemove();
}

void CMissionCompilerFeDlg::OnSpecialRuncompilertest() 
{
	string sourceDocName = TestPrimitive;
	string dstDocName = TestPrimitiveDest;

	CPrimitives		primDoc;
	CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
	nlinfo("Test compiler, opening file '%s'", sourceDocName.c_str());
	loadXmlPrimitiveFile(primDoc, sourceDocName, LigoConfig);
	CPrimitiveContext::instance().CurrentPrimitive = NULL;

	try
	{
		CMissionCompiler mc;
		if (!mc.compileMissions(primDoc.RootNode, sourceDocName))
		{
			AfxMessageBox("Error while compiling the test primitive, correct error(s) first");
			return;
		}
		TMissionDataPtr testMission = mc.getMission(0);

		CSString script = testMission->generateMissionScript(sourceDocName);
		script += "======================================================"+NL;
		script += testMission->generatePhraseFile();
		script += "======================================================"+NL;
		script += testMission->generateDotScript();

		// add the compiled in mission primitive
		mc.installCompiledMission(LigoConfig, sourceDocName);
		char *buffer = NULL;
		CIFile dst(dstDocName);
		buffer = new char[dst.getFileSize()+1];
		dst.serialBuffer((uint8*)buffer, dst.getFileSize());
		buffer[dst.getFileSize()] = 0;
		script += "======================================================"+NL;
		script += buffer;
		delete [] buffer;

		// cleanup CR/LF with a single \n
		script = script.replace(NL.c_str(), "\n");

		char *tmp = ::getenv("TEMP");
	
		FILE *fp = ::fopen((string(tmp)+"/compiled_mission.script").c_str(), "w");
		::fwrite(script.data(), script.size(), 1, fp);
		::fclose(fp);

		system((string("\"C:\\Program Files\\Beyond Compare 2\\bc2.exe\" ")+string(tmp)+"/compiled_mission.script "+ReferenceScript).c_str());
	}
	catch(EParseException e)
	{
		string msg = "In primitive ";
		msg += buildPrimPath(e.Primitive) +" : "+e.Why;
		AfxMessageBox(msg.c_str());
	}
	
}

void CValidationFile::loadMissionValidationFile(string filename)
{
	using namespace std;
	
	// load the configuration file
	CConfigFile cf;
	string pathName = CPath::lookup(filename, false);
	
	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no mission will be valid", filename.c_str());
		return;
	}
	cf.load(pathName);
	
	// get the variable
	CConfigFile::CVar* var = cf.getVarPtr("AuthorizedStates");
	if (var)
	{
		for (uint i=0; i<var->size(); ++i)
			_AuthorizedStates.push_back(var->asString(i));
	}
	int missionStatesFields = 3;
	var = cf.getVarPtr("MissionStatesFields");
	if (var)
	{
		missionStatesFields = var->asInt();
	}
	else
	{
		nlwarning("Mission validation file does not contain MissionStatesFields variable. Parsing may fail and corrupt data.");
	}
	var = cf.getVarPtr("MissionStates");
	if (var)
	{
		for (uint i=0; i<var->size()/missionStatesFields; ++i)
		{
			string mission = var->asString(i*missionStatesFields);
			string stateName = var->asString(i*missionStatesFields+1);
			string hashKey = var->asString(i*missionStatesFields+2);
			
		//	if (_AuthorizedStates.empty() || _AuthorizedStates.find(state)!=_AuthorizedStates.end())
				_MissionStates.insert(make_pair(mission, CMissionState(mission, stateName, hashKey)));
		}
	}
}

void CValidationFile::saveMissionValidationFile(string filename)
{
	using namespace std;
	
	// load the configuration file
	string pathName = CPath::lookup(filename, false);
	
	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no mission will be valid", filename.c_str());
		return;
	}
	FILE* file = fopen(pathName.c_str(), "w");
	nlassert(file!=NULL);
	
	// AuthorizedStates
	fprintf(file, "%s",
		"// AuthorizedStates contains the list of authorized states. EGS mission\n"
		"// manager can accept any number of states. Default state is the first one.\n"
		"AuthorizedStates = {\n");
	deque<string>::iterator itAuth, itAuthEnd = _AuthorizedStates.end();
	for (itAuth=_AuthorizedStates.begin(); itAuth!=itAuthEnd; ++itAuth)
		fprintf(file, "\t\"%s\",\n", itAuth->c_str());
	fprintf(file, "%s", "};\n\n");
	
	// MissionStatesFields
	fprintf(file, "%s",
		"// MissionStatesFields contains the number of fields in MissionStates, for\n"
		"// future compatibility purpose.\n"
		"MissionStatesFields = ");
	fprintf(file, "%d", 3); // 3 fields: name, state, hash key
	fprintf(file, "%s", ";\n\n");
	
	// MissionStates
	fprintf(file, "%s",
		"// MissionStates contains a list of mission with for each the state of the\n"
		"// mission and its hash key. The tool will add new missions with the default\n"
		"// state. It will flag missions with a modified hash key with default state to\n"
		"// prevent untested modified missions to be published.\n"
		"// :NOTE: You can add a field to this structure without the need to modify EGS\n"
		"// code. Simply update MissionStatesFields.\n"
		"MissionStates = {\n");
	TMissionStateContainer::iterator itMission, itMissionEnd = _MissionStates.end();
	for (itMission=_MissionStates.begin(); itMission!=itMissionEnd; ++itMission)
		fprintf(file, "\t%-42s %-12s \"%s\",\n", ("\""+itMission->second.name+"\",").c_str(), ("\""+itMission->second.state+"\",").c_str(), itMission->second.hashKey.c_str());
	fprintf(file, "};\n\n");

	fclose(file);
}

// :NOTE: This function exists in mission_template.cpp. If you change it here modify the other file.
std::string buildHashKey(std::string const& content)
{
	uint32 sum = 0;
	size_t size = content.length()/4;
	for (size_t i=0; i<size; ++i)
	{
		uint32 val = 0;
		for (int j=0; j<4; ++j)
			val += content[4*i+j]<<8*j;
		sum += val;
		if (sum&1)
			sum = sum>>1 | 0x80000000;
		else
			sum = sum>>1;
	}
	return NLMISC::toString("0x%08X", sum);
}

bool CMission::parsePrim(NLLIGO::IPrimitive const* prim)
{
	// init default values
	vector<string>* params;
	// get the mission script
	if (!prim->getPropertyByName("script", params) || !params)
	{
		nlwarning("ERROR : cant find mission script!!!!!!");
		return false;
	}
	
	// parse them
	string content;
	vector<string>::iterator itParam, itParamEnd = params->end();
	for (itParam=params->begin(); itParam!=itParamEnd; ++itParam)
	{
		content += *itParam + "\n";
	}
	hashKey = buildHashKey(content);
	return true;
}

void CMissionCompilerFeDlg::OnSpecialValidateMissions() 
{
	CCompilDialog dlg;
	dlg.Create(IDD_COMPIL, this);
	EnableWindow(FALSE);
	dlg.m_okBtn.EnableWindow(FALSE);
	dlg.ShowWindow(SW_SHOW);
	string	compileLog;
	
	// Load existing validation
	CValidationFile validation;
	validation.loadMissionValidationFile("mission_validation.cfg");
	// Find real paths
	typedef map<string, string> TFileContainer;
	TFileContainer files;
	TFileList::iterator itDst, itDstEnd = _DstList.end();
	for (itDst=_DstList.begin(); itDst!=itDstEnd; ++itDst)
		files.insert(make_pair(itDst->first, itDst->second));
	
	// Parse primitives to get missions
	TFileContainer::iterator itFile, itFileEnd = files.end();
	for (itFile=files.begin(); itFile!=itFileEnd; ++itFile)
	{
		compileLog += "Parsing '"+itFile->first+"'...\r\n";
		dlg.m_compileLog = compileLog.c_str();
		dlg.UpdateData(FALSE);
		dlg.RedrawWindow();
		
		string filename = itFile->second;
		
		TMissionContainer missions;
		
		// Load primitive
		NLLIGO::CPrimitives primDoc;
		CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		NLLIGO::loadXmlPrimitiveFile(primDoc, filename, LigoConfig);
		CPrimitiveContext::instance().CurrentPrimitive = NULL;
		parsePrimForMissions(primDoc.RootNode, missions);
		
		// Parse missions to check modification
		std::map<std::string, CMission>::iterator itMission, itMissionEnd = missions.end();
		for (itMission=missions.begin(); itMission!=itMissionEnd; ++itMission)
		{
			CValidationFile::TMissionStateContainer::iterator itMissionValidation = validation._MissionStates.find(itMission->first);
			if (itMissionValidation!=validation._MissionStates.end())
			{
				// Mission already registered, check hash key
				if (itMissionValidation->second.hashKey!=itMission->second.hashKey)
				{
					itMissionValidation->second.hashKey = itMission->second.hashKey;
					itMissionValidation->second.state = validation.defaultState();
				}
			}
			else
			{
				// New mission
				validation.insertMission(itMission->first, itMission->second.hashKey);
			}
			compileLog += "Mission: '"+itMission->first+"->"+itMission->second.hashKey+"\r\n";
			dlg.m_compileLog = compileLog.c_str();
			dlg.UpdateData(FALSE);
			dlg.RedrawWindow();
		}
	}
	validation.saveMissionValidationFile("mission_validation.cfg");
	
	compileLog += toString("Validation finished");
	dlg.m_compileLog = compileLog.c_str();
	dlg.UpdateData(FALSE);
	dlg.m_compileLogCtrl.SetSel(compileLog.size(), compileLog.size(), FALSE);
	dlg.RedrawWindow();
	
	dlg.m_okBtn.EnableWindow(TRUE);
	dlg.RunModalLoop();
	dlg.DestroyWindow();
	EnableWindow(TRUE);
}

bool CMissionCompilerFeDlg::parsePrimForMissions(NLLIGO::IPrimitive const* prim, TMissionContainer& missions)
{
	string value;
	// if the node is a mission parse it
	if (prim->getPropertyByName("class",value) && !nlstricmp(value.c_str(),"mission") )
	{
		string name;
		prim->getPropertyByName("name",name);
		
		// parse the mission and put it in our manager
		CMission mission(value, "");
		if (!mission.parsePrim(prim) )
		{
			nlwarning("Previous errors in mission '%s'", name.c_str());
		//	MissionLog.Log->displayNL(" \n");
			return false;
		}
		missions.insert(make_pair(name, mission));
		return true;
	}
	else
	{
		//this is not a mission node, so lookup recursively in the children
		bool ok = true;
		for (uint i=0;i<prim->getNumChildren();++i)	
		{
			const IPrimitive *child;
			if ( !prim->getChild(child,i) || !parsePrimForMissions(child, missions) )
				ok = false;
		}
		return ok;
	}
}

void CMissionCompilerFeDlg::OnValidate() 
{
	OnSpecialValidateMissions();
}

void CMissionCompilerFeDlg::OnPublish() 
{
	compile(TRUE);
}

void CMissionCompilerFeDlg::updateSearchPath() 
{
	for (uint i=0; i<SearchPaths.size(); ++i)
	{		
		NLMISC::CPath::addSearchPath(SearchPaths[i], true, false);
	}	
}

int CALLBACK dataDirBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch (uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

void CMissionCompilerFeDlg::OnChangeDir()
{
	UpdateData();

	// open a dialog to choose a directory
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	// fill the structure with options
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;
	bi.lpszTitle = "Choose the data directory";
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, m_dataDirectory);
	bi.lParam = (LPARAM)sDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);

	if (SHGetPathFromIDList(pidl, str)) 
	{
		m_dataDirectory = str;
		SearchPaths[0] = str;
		fillSourceList();
		updateFileList();
		UpdateData(FALSE);
	}
}

void CMissionCompilerFeDlg::OnChangeDataDir() 
{
	UpdateData(TRUE);
	SearchPaths[0] = m_dataDirectory;
	fillSourceList();
	updateFileList();
	UpdateData(FALSE);
}