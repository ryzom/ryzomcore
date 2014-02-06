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

//

#include "stdafx.h"

#include "nel/misc/path.h"
#include "nel/misc/config_file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/dynloadlib.h"

#include "georges_edit.h"
#include "reg_shell_ext.h"

#include "main_frm.h"
#include "child_frm.h"
#include "georges_edit_doc.h"
#include "left_view.h"

#include "splash_screen.h"

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


const char* TypeFilter = "Type Files (*.typ)|*.typ|All Files (*.*)|*.*||";
const char* DfnFilter = "Form Definition Files (*.dfn)|*.dfn|All Files (*.*)|*.*||";
CSplashScreen* splashScreen=new CSplashScreen;
/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditApp

BEGIN_MESSAGE_MAP(CGeorgesEditApp, CWinApp)
	//{{AFX_MSG_MAP(CGeorgesEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_FILE_SAVE_ALL, OnFileSaveAll)
	ON_COMMAND(ID_FILE_CLOSE_ALL, OnFileCloseAll)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_ALL, OnUpdateFileSaveAll)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditApp construction

CGeorgesEditApp::CGeorgesEditApp() : MemStream (false, false, 1024*1024)
{
	Superuser = true;
	DefaultType = "default.typ";
	DefaultDfn = "default.dfn";

	m_pMainWnd = NULL;
	RememberListSize = 10;
	MaxUndo = 20;
	ResizeMain = true;
	ExeStandalone = false;
	StartExpanded = true;

	FormClipBoardFormatStruct = RegisterClipboardFormat ("GeorgesFormStruct");
	FormClipBoardFormatVirtualStruct = RegisterClipboardFormat ("GeorgesFormVirtualStruct");
	FormClipBoardFormatArray = RegisterClipboardFormat ("GeorgesFormArray");
	FormClipBoardFormatType = RegisterClipboardFormat ("GeorgesFormType");
	nlassert (FormClipBoardFormatStruct);
	nlassert (FormClipBoardFormatVirtualStruct);
	nlassert (FormClipBoardFormatArray);
	nlassert (FormClipBoardFormatType);

	_TemplateForm = NULL;
	_TemplateType = NULL;
	_TemplateDfn = NULL;
}

CGeorgesEditApp::~CGeorgesEditApp ()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeorgesEditApp object

CGeorgesEditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditApp initialization

BOOL CGeorgesEditApp::InitInstance()
{
	return TRUE;
}

CMyMultiDocTemplate::CMyMultiDocTemplate (const char *ext, UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) : 
	  CMultiDocTemplate ( nIDResource, pDocClass, pFrameClass, pViewClass )
{
	_Ext = ext;
	_ExtMaj = _Ext;
	_Ext = strlwr (_Ext);
	_ExtMaj[0] = toupper (_ExtMaj[0]);
}

void CMyMultiDocTemplate::getDfnName (string &dfnName)
{
	dfnName = _Ext + ".dfn";
}

BOOL CMyMultiDocTemplate::GetDocString(CString& rString, enum DocStringIndex index) const
{
	switch (index)
	{
	case fileNewName:
	case windowTitle:
	case docName:
		rString = (_ExtMaj + " Form").c_str ();
		return TRUE;
	case filterName:
		rString = (_ExtMaj + " Form Filename (*." + _Ext+ ")").c_str ();
		return TRUE;
	case filterExt:
		rString = ("." + _Ext).c_str ();
		return TRUE;
	default:
		return CMultiDocTemplate::GetDocString(rString, index);
	}
}

BOOL CGeorgesEditApp::initInstance (int nCmdShow, bool exeStandalone, int x, int y, int cx, int cy)
{
	// Some things have just to be done once per launch of the application that is
	// completely different from an opening/closing of the main frame
	static bool isInitialized = false;
	ExeStandalone = exeStandalone;

	
	splashScreen->Create(IDD_SPLASHSCREEN, theApp.GetMainWnd());
	splashScreen->ShowWindow(TRUE);
	splashScreen->addLine(string("Georges Initialization"));
	// Filter addSearchPath
	splashScreen->addLine(string("Filter addSearchPath"));
	NLMISC::createDebug();
	InfoLog->addNegativeFilter("adding the path");
	WarningLog->addNegativeFilter("Exception will be launched");

	if (!isInitialized && exeStandalone)
	{
		m_nCmdShow = nCmdShow;
		ExePath = GetCommandLine ();
		if (ExePath.size()>0)
		{
			if (ExePath[0] == '\"')
			{
				uint end=ExePath.find ('\"', 1);
				if (end != string::npos)
				{
					ExePath = ExePath.substr (1, end-1);
				}
				else
				{
					nlassert (0);	// no!
				}
			}
			else
			{
				uint end=ExePath.find (' ', 1);
				ExePath = ExePath.substr (0, end);
			}
		}

		// Init CPath
		loadCfg ();
		initCfg ();
	}

	if (!isInitialized)
	{
		// Fill the image list
		splashScreen->addLine(string("Fill the image list"));
		ImageList.create (16, 16);
		ImageList.addResourceIcon (m_hInstance, IDR_ARRAY);
		ImageList.addResourceIcon (m_hInstance, IDR_HEADER);
		ImageList.addResourceIcon (m_hInstance, IDR_HOLD);
		ImageList.addResourceIcon (m_hInstance, IDR_ROOT);
		ImageList.addResourceIcon (m_hInstance, IDR_STRUCT);
		ImageList.addResourceIcon (m_hInstance, IDR_VSTRUCT);
		ImageList.addResourceIcon (m_hInstance, IDR_TYPEDFN);
		ImageList.addResourceIcon (m_hInstance, IDR_TYPEFORM);
		ImageList.addResourceIcon (m_hInstance, IDR_TYPETYPE);

		// Add icon
		splashScreen->addLine(string("Add icon"));
		std::vector<std::string> result;
		string iconPathName = RootSearchPath+TypeDfnSubDirectory;
		if (!iconPathName.empty ())
		{
			CPath::getPathContent (iconPathName, true, false, true, result);
			uint i;
			for (i=0; i<result.size (); i++)
			{
				// Icon ?
				uint ii = result[i].rfind ('.');
				if ((ii != string::npos) && (stricmp (result[i].c_str ()+ii, ".ico") == 0) )
				{
					ImageList.addResourceIcon (result[i].c_str ());
				}
			}
		}
	}

	// Standard initialization
	splashScreen->addLine(string("Standard initialization"));
	if (!isInitialized)
	{

// 		const char *open = "Open(\"%1\")";
// 		const char *copy = "Georges Copy(\"%1\")";
// 		const char *derive = "Derive(\"%1\")";
// 		const char *createForm = "CreateForm(\"%1\")";
// 		const char *notepad = "notepad.exe \"%1\"";

		// *** Registry

		// MFC settings
		splashScreen->addLine(string("MFC settings"));
		SetRegistryKey(_T("Nevrax"));
		LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)
//		EnableShellOpen();

		// Application path
		string appPath = "\"";
		appPath += ExePath;
		appPath += "\"";

		bool okRegister = true;
		if (ExeStandalone)
		{
			splashScreen->addLine(string("Associate Georges file extensions"));
			okRegister &= RegisterApp ("Georges.Type", "Georges Type Files", appPath.c_str (), 3);
			okRegister &= RegisterApp ("Georges.Dfn", "Georges Dfn Files", appPath.c_str (), 3);
			okRegister &= RegisterApp ("Georges.Form", "Georges Form Files", appPath.c_str (), 3);
			okRegister &= RegisterShellFileExt (".typ", "Georges.Type");
			okRegister &= RegisterShellFileExt (".dfn", "Georges.Dfn");

			// Register Type and DFN for superuser
/*			if (Superuser)
			{
				// Register the application
				splashScreen->addLine(string("Register the application"));
				okRegister &= RegisterApp ("Georges.Type", "Georges Type Files", appPath.c_str (), 1);
				okRegister &= RegisterApp ("Georges.Dfn", "Georges Dfn Files", appPath.c_str (), 2);
				okRegister &= RegisterApp ("Georges.Form", "Georges Form Files", appPath.c_str (), 3);

				// Register the command
				splashScreen->addLine(string("Register the command"));
				okRegister &= RegisterAppCommand ("Georges.Type", "Open", (appPath+" %1").c_str ());
				okRegister &= RegisterAppCommand ("Georges.Dfn", "Open", (appPath+" %1").c_str ());
				okRegister &= RegisterAppCommand ("Georges.Dfn", "Create Form", (appPath+" /dde").c_str ());

				okRegister &= RegisterAppCommand ("Georges.Type", "Open With Notepad", notepad);
				okRegister &= RegisterAppCommand ("Georges.Dfn", "Open With Notepad", notepad);

				// Register the DDE command (avoid strange Visual compile error..)
				splashScreen->addLine(string("Register the DDE command"));
 				okRegister &= RegisterDDECommand ("Georges.Type", "Open", open, m_pszExeName);
 				okRegister &= RegisterDDECommand ("Georges.Dfn", "Open", open, m_pszExeName);
				okRegister &= RegisterDDECommand ("Georges.Dfn", "Create Form", createForm, m_pszExeName);

				// Register the file extension
				splashScreen->addLine(string("Register the file extension"));
				okRegister &= RegisterShellFileExt (".typ", "Georges.Type");
				okRegister &= RegisterShellFileExt (".dfn", "Georges.Dfn");
			}
			else
			{
				// Register the command
				splashScreen->addLine(string("Register the command"));
				UnregisterAppCommand ("Georges.Dfn", "Open");
				okRegister &= RegisterAppCommand ("Georges.Dfn", "Create Form", (appPath+" /dde").c_str ());
				UnregisterAppCommand ("Georges.Dfn", "Open With Notepad");

				// Register the DDE command (avoid strange Visual compile error..)
				splashScreen->addLine(string("Register the DDE command"));
				okRegister &= RegisterDDECommand ("Georges.Dfn", "Create Form", createForm, m_pszExeName);

				// Register the file extension
				splashScreen->addLine(string("Register the file extension"));
				okRegister &= RegisterShellFileExt (".dfn", "Georges.Dfn");

				// Unregister type and dfn
				splashScreen->addLine(string("Unregister type and dfn"));
				UnregisterApp ("Georges.Type");
			}

			// Register non super user commands
			splashScreen->addLine(string("Register non super user commands"));
			okRegister &= RegisterAppCommand ("Georges.Form", "Open", (appPath+" %1").c_str ());
			okRegister &= RegisterAppCommand ("Georges.Form", "Georges Copy", (appPath+" /dde").c_str ());
			okRegister &= RegisterAppCommand ("Georges.Form", "Derive", (appPath+" /dde").c_str ());
			okRegister &= RegisterAppCommand ("Georges.Form", "Open With Notepad", notepad);

			// Register dde commands
			splashScreen->addLine(string("Register dde commands"));
			okRegister &= RegisterDDECommand ("Georges.Form", "Open", open, m_pszExeName);
			okRegister &= RegisterDDECommand ("Georges.Form", "Georges Copy", copy, m_pszExeName);
			okRegister &= RegisterDDECommand ("Georges.Form", "Derive", derive, m_pszExeName);
*/
		}

		// Ok register ?
		if (!okRegister)
			nlwarning ("Register Georges: problem during Georges registration. Do you have administrator privileges ?");

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		// Are we a superuser ?
		if (Superuser)
		{
			// Form
			_TemplateForm = new CMultiDocTemplate(
				IDR_TYPEFORM,
				RUNTIME_CLASS(CGeorgesEditDocForm),
				RUNTIME_CLASS(CChildFrame), // custom MDI child frame
				RUNTIME_CLASS(CLeftView));
			AddDocTemplate(_TemplateForm);

			// Type
			_TemplateType = new CMultiDocTemplate(
				IDR_TYPETYPE,
				RUNTIME_CLASS(CGeorgesEditDocType),
				RUNTIME_CLASS(CChildFrame), // custom MDI child frame
				RUNTIME_CLASS(CLeftView));
			AddDocTemplate(_TemplateType);

			// Dfn
			_TemplateDfn = new CMultiDocTemplate(
				IDR_TYPEDFN,
				RUNTIME_CLASS(CGeorgesEditDocDfn),
				RUNTIME_CLASS(CChildFrame), // custom MDI child frame
				RUNTIME_CLASS(CLeftView));
			AddDocTemplate(_TemplateDfn);
		}
		else
		{
			// For each form
			uint i;
			for (i=0; i<theApp.UserTypes.size (); i++)
			{
				AddDocTemplate(new CMyMultiDocTemplate (UserTypes[i].c_str (),
					IDR_TYPEFORM,
					RUNTIME_CLASS(CGeorgesEditDocForm),
					RUNTIME_CLASS(CChildFrame), // custom MDI child frame
					RUNTIME_CLASS(CLeftView)));
			}
		}
	}

	// create main MDI Frame window
	splashScreen->addLine(string("create main MDI Frame window"));
	CMainFrame* pMainFrame = new CMainFrame;
	nlassert (pMainFrame);
	((CMainFrame*)pMainFrame)->createX = x;
	((CMainFrame*)pMainFrame)->createY = y;
	((CMainFrame*)pMainFrame)->createCX = cx;
	((CMainFrame*)pMainFrame)->createCY = cy;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	if (!isInitialized)
	{
		// Enable DDE Execute open
//		EnableShellOpen();
//		RegisterShellFileTypes(TRUE);
		isInitialized = true;
	}

	// Parse command line for standard shell commands, DDE, file open
	splashScreen->addLine(string("Parse command line"));
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	splashScreen->addLine(string("Dispatch commands specified"));
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	/*if (!ProcessShellCommand(cmdInfo))
		return FALSE;*/

	// Load and init plugins
	splashScreen->addLine(string("Load and init plugins"));
	loadPlugins ();

	// Init windows
	ResizeMain = (x==-1);
	loadState ();

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(SW_SHOW /*m_nCmdShow*/);
	pMainFrame->UpdateWindow();

	splashScreen->ShowWindow(false);
	return TRUE;
}

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
		// No message handlers
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

// App command to run the dialog
void CGeorgesEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CGeorgesEditApp::outputError (const char* message)
{
	if (m_pMainWnd)
		m_pMainWnd->MessageBox (message, "Georges Edit", MB_OK|MB_ICONEXCLAMATION);
	else
		MessageBox (NULL, message, "Georges Edit", MB_OK|MB_ICONEXCLAMATION);
}

void CGeorgesEditApp::getConfigFilePath (std::string &output)
{
	// Get the config file path
	char sDrive[MAX_PATH];
	char sDir[MAX_PATH];
	char sPath[MAX_PATH];
	_splitpath (theApp.ExePath.c_str (), sDrive, sDir, NULL, NULL);
	_makepath (sPath, sDrive, sDir, "georges", ".cfg");
	output = sPath;
}

bool CGeorgesEditApp::loadCfg ()
{
	// Open the config file
	std::string configPath;
	getConfigFilePath (configPath);
	CConfigFile &cf = ConfigFile;;


	// Root search path
	try
	{
		cf.load (configPath);

		CConfigFile::CVar *rootDirectory = cf.getVarPtr ("RootSearchDirectory");
		if (rootDirectory)
		{
			RootSearchPath = rootDirectory->asString ();
			if (RootSearchPath.size ())
			{
				char c = RootSearchPath[RootSearchPath.size ()-1];
				if ((c != '\\') && (c != '/'))
					RootSearchPath += "\\";
			}
		}

		// Type and Dfn sub directory
		CConfigFile::CVar *typeDfnSubDirectory = cf.getVarPtr ("TypDfnSubFolder");
		if (typeDfnSubDirectory)
		{
			// Last char is good ?
			string copy = typeDfnSubDirectory->asString ();
			const char *ptr = copy.c_str ();
			while ((*ptr == '/') || (*ptr == '\\'))
				ptr++;
			TypeDfnSubDirectory = ptr;
			if (TypeDfnSubDirectory.size ())
			{
				char c = TypeDfnSubDirectory[TypeDfnSubDirectory.size ()-1];
				if ((c != '\\') && (c != '/'))
					TypeDfnSubDirectory += "\\";
			}
		}

		// RememberListSize
		CConfigFile::CVar *remember = cf.getVarPtr ("RememberListSize");
		if (remember)
			RememberListSize = remember->asInt ();

		// StartExpanded
		CConfigFile::CVar *start_expanded = cf.getVarPtr ("StartExpanded");
		if (start_expanded)
			StartExpanded = start_expanded->asInt () != 0;

		// MaxUndo
		CConfigFile::CVar *max_undo = cf.getVarPtr ("MaxUndo");
		if (max_undo)
			MaxUndo = max_undo->asInt ();

		// DefaultType
		CConfigFile::CVar *defaultType = cf.getVarPtr ("DefaultType");
		if (defaultType)
			DefaultType = defaultType->asString ();

		// DefaultDfn
		CConfigFile::CVar *defaultDfn = cf.getVarPtr ("DefaultDfn");
		if (defaultDfn)
			DefaultDfn = defaultDfn->asString ();

		// Plugins
		CConfigFile::CVar *plugins = cf.getVarPtr ("Plugins");
		if (plugins)
		{
			PluginsNames.reserve (plugins->size());
			uint i;
			for (i=0; i<(uint)plugins->size(); i++)
				PluginsNames.push_back (plugins->asString (i));
		}

		// User types
		CConfigFile::CVar *user_type = cf.getVarPtr ("UserType");
		if (user_type)
		{
			UserTypes.reserve (user_type->size());
			uint i;
			for (i=0; i<(uint)user_type->size(); i++)
				UserTypes.push_back (user_type->asString (i));
		}

		// Super user
		CConfigFile::CVar *superuser = cf.getVarPtr ("SuperUser");
		if (superuser)
			Superuser = superuser->asInt () != 0;
	}
	catch (Exception &)
	{
		char message[512];
		smprintf (message, 512, "Can't load georges.cfg config file.");
		outputError (message);
	}

	return true;
}

bool CGeorgesEditApp::saveCfg ()
{
	// Config path
	std::string configPath;
	getConfigFilePath (configPath);
	CConfigFile &cf = ConfigFile;

	// Root search path
	try
	{
		cf.load (configPath);

		CConfigFile::CVar *rootDirectory = cf.getVarPtr ("RootSearchDirectory");
		if (rootDirectory)
			rootDirectory->setAsString (RootSearchPath.c_str ());

		// Type and Dfn sub directory
		CConfigFile::CVar *typeDfnSubDirectory = cf.getVarPtr ("TypDfnSubFolder");
		if (typeDfnSubDirectory)
			typeDfnSubDirectory->setAsString (TypeDfnSubDirectory.c_str ());

		// RememberListSize
		CConfigFile::CVar *remember= cf.getVarPtr ("RememberListSize");
		if (remember)
			remember->setAsInt (RememberListSize);

		// StartExpanded
		CConfigFile::CVar *start_expanded= cf.getVarPtr ("StartExpanded");
		if (start_expanded)
			start_expanded->setAsInt (StartExpanded);

		// MaxUndo
		CConfigFile::CVar *max_undo= cf.getVarPtr ("MaxUndo");
		if (max_undo)
			max_undo->setAsInt (MaxUndo);

		// DefaultType
		CConfigFile::CVar *defaultType = cf.getVarPtr ("DefaultType");
		if (defaultType)
			defaultType->setAsString  (DefaultType.c_str ());

		// DefaultDfn
		CConfigFile::CVar *defaultDfn = cf.getVarPtr ("DefaultDfn");
		if (defaultDfn)
			defaultDfn->setAsString  (DefaultDfn.c_str ());

		// Save the list
		try
		{
			cf.save ();
		}
		catch (Exception &)
		{
			char message[512];
			smprintf (message, 512, "Can't save georges.cfg config file.");
			outputError (message);
		}
	}
	catch (Exception &)
	{
		char message[512];
		smprintf (message, 512, "Can't load georges.cfg config file.");
		outputError (message);
	}

	return true;
}

void CGeorgesEditApp::initCfg ()
{
	// Clear the search path list
	CPath::removeAllAlternativeSearchPath ();

	// Add search path
	CPath::addSearchPath (RootSearchPath, true, true,(IProgressCallback*)splashScreen);
}

bool CGeorgesEditApp::getColor (NLMISC::CRGBA &color)
{
	// Get custom colors
	COLORREF arrayColor[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, GEORGES_EDIT_BASE_REG_KEY"\\Custom Colors", 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=sizeof(arrayColor);
		DWORD type;
		RegQueryValueEx (hKey, "", 0, &type, (LPBYTE)(arrayColor), &len);
		RegCloseKey (hKey);
	}

	// Reset the struct
	CHOOSECOLOR cc;
	memset (&cc, 0, sizeof(CHOOSECOLOR));

	// Fill the struct
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.rgbResult=RGB (color.R, color.G, color.B);
	cc.lpCustColors=arrayColor;
	cc.Flags=CC_RGBINIT|CC_ANYCOLOR|CC_FULLOPEN;

	// Open it
	if (ChooseColor (&cc))
	{
		color = CRGBA (GetRValue (cc.rgbResult), GetGValue (cc.rgbResult), GetBValue (cc.rgbResult));

		// Save the custom colors
		HKEY hKey;
		if (RegCreateKey(HKEY_CURRENT_USER, GEORGES_EDIT_BASE_REG_KEY"\\Custom Colors", &hKey)==ERROR_SUCCESS)
		{
			RegSetValueEx (hKey, "", 0, REG_BINARY, (LPBYTE)(arrayColor), sizeof(arrayColor));
			RegCloseKey (hKey);
		}
		return true;
	}
	return false;
}

bool CGeorgesEditApp::yesNo (const char* message)
{
	if (m_pMainWnd)
		return m_pMainWnd->MessageBox (message, "Georges Edit", MB_YESNO|MB_ICONQUESTION) != IDNO;
	else
		return MessageBox (NULL, message, "Georges Edit", MB_YESNO|MB_ICONQUESTION)  != IDNO;
}

void CGeorgesEditApp::loadPlugins ()
{
	uint i;
	for (i=0; i<PluginsNames.size (); i++)
	{
		// Load the dll
		HINSTANCE hModule = AfxLoadLibrary (PluginsNames[i].c_str ());
		if (hModule)
		{
			// Get the proc adrdess
			NLGEORGES::IGeorgesEditGetInterface procGet = (NLGEORGES::IGeorgesEditGetInterface) ::GetProcAddress (hModule, "IGeorgesEditGetInterface");
			if (procGet)
			{
				// Get the interface pointer
				IEditPlugin *plugin = procGet (NLGEORGES_PLUGIN_INTERFACE_VERSION, this, INelContext::getInstance());
				if (plugin)
				{
					// Valid pointer ?
					PluginArray.push_back (CPlugin (hModule, plugin));

					// Init the plugin uI
					 plugin->dialogInit (*m_pMainWnd);

					// Hide it
					plugin->activate (false);
				}
			}
			else
			{
				char message[512];
				smprintf (message, 512, "Bad interface in plugin \"%s\"", PluginsNames[i].c_str ());
				outputError (message);
			}
		}
		else
		{
			char message[512];
			smprintf (message, 512, "Can't found plugin \"%s\"", PluginsNames[i].c_str ());
			outputError (message);
		}
	}
}

void CGeorgesEditApp::releasePlugins ()
{
	uint i;
	for (i=0; i<PluginArray.size (); i++)
	{
		// Delete the object interface
		delete PluginArray[i].PluginInterface;

		// Free
		nlverify ( FreeLibrary (PluginArray[i].PluginModule) );
	}

	PluginArray.clear ();
}

IEditDocument *CGeorgesEditApp::getActiveDocument ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_pMainWnd)
	{
		CGeorgesEditDoc *doc = (CGeorgesEditDoc*)(((CMainFrame*)m_pMainWnd)->GetActiveDocument ());
		if (doc)
		{
			if (doc->isForm ())
				return doc;
			else
				return NULL;
		}
		else
			return NULL;
	}
	else
		return NULL;
}

void CGeorgesEditApp::getSearchPath (std::string &searchPath)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	searchPath = RootSearchPath;
}

NLGEORGES::IEditDocument *CGeorgesEditApp::createDocument (const char *dfnName, const char *pathName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Get the form templaet
	CMultiDocTemplate *docTemplate = getFormDocTemplate (dfnName);

	if (docTemplate)
	{
		// Create a new document
		CGeorgesEditDocForm *doc = (CGeorgesEditDocForm*)docTemplate->CreateNewDocument();
		nlassert (doc);
		nlassert (doc->isForm ());

		// Create the frame
		CFrameWnd* pFrame = docTemplate->CreateNewFrame(doc, NULL);
		nlassert (pFrame);

		// Set default title
		docTemplate->SetDefaultTitle (doc);

		// Init the document
		if (doc->initDocument (dfnName, true))
		{
			// Set the filename
			if (strcmp (pathName, "") != 0)
			{
				doc->SetPathName ( pathName, FALSE );

				// Create the file
				doc->OnSaveDocument( pathName );
			}

			// Init the frame
			docTemplate->InitialUpdateFrame (pFrame, doc, TRUE);

			doc->changeSubSelection ((CGeorgesEditDocSub *)NULL, NULL);
			doc->UpdateAllViews (NULL);
			return doc;
		}
		else
		{
			docTemplate->RemoveDocument( doc );
			doc = NULL;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesEditApp message handlers



int CGeorgesEditApp::ExitInstance() 
{
	// Save the app profile (because it is a DLL)
	if (m_pCmdInfo == NULL ||
		m_pCmdInfo->m_nShellCommand != CCommandLineInfo::AppUnregister)
	{
		if (!afxContextIsDLL)
			SaveStdProfileSettings();
	}
	
	return CWinApp::ExitInstance();
}

CGeorgesEditApp::CPlugin::CPlugin (HINSTANCE hModule, IEditPlugin *plugin)
{
	PluginModule = hModule;
	PluginInterface = plugin;
	Activated = false;
}

void CGeorgesEditApp::onNewDocView (CGeorgesEditDoc *doc)
{
	if (doc->isForm ())
	{
		uint i;
		for (i=0; i<PluginArray.size (); i++)
		{
			PluginArray[i].PluginInterface->onCreateDocument (doc);
		}

	}
}

bool CGeorgesEditApp::isPluginActivated (NLGEORGES::IEditPlugin *plugin) const
{
	uint i;
	for (i=0; i<PluginArray.size(); i++)
	{
		if (PluginArray[i].PluginInterface == plugin)
			return PluginArray[i].Activated;
	}
	nlwarning ("Plugin not found");
	return false;
}

LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) 
	{
        long datasize = MAX_PATH;
        char data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}

void CGeorgesEditApp::gotoURL (LPCTSTR url)
{
    char key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, "open", url, NULL,NULL, SW_SHOW);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) {

        if (GetRegKey(HKEY_CLASSES_ROOT, ".htm", key) == ERROR_SUCCESS) 
		{
            lstrcat(key, "\\shell\\open\\command");

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
			{
                char *pos;
                pos = strstr(key, "\"%1\"");
                if (pos == NULL) 
				{                     // No quotes found
                    pos = strstr(key, "%1");       // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, " ");
                lstrcat(pos, url);
                result = (HINSTANCE) WinExec(key, SW_SHOW);
            }
        }
    }
}

void CGeorgesEditApp::WinHelp(DWORD dwData, UINT nCmd) 
{
	char drive[512];
	char dir[512];
	char path[512];
	_splitpath (ExePath.c_str (), drive, dir, NULL, NULL);
	_makepath (path, drive, dir, "georges_edit", ".html");
	gotoURL (path);
}

void CGeorgesEditApp::OnViewRefresh() 
{
	if (m_pMainWnd)
	{
		((CMainFrame*)m_pMainWnd)->FileBrowserDlg.refresh ();
	}
}

void CGeorgesEditApp::saveWindowState (const CWnd *wnd, const char *name, bool controlBar)
{
	HKEY hKey;
	nlverify (RegCreateKey (HKEY_CURRENT_USER, GEORGES_EDIT_BASE_REG_KEY"\\Windows states", &hKey) == ERROR_SUCCESS);

	// Get the position
	WINDOWPLACEMENT wndpl;
	wnd->GetWindowPlacement( &wndpl );

	// Set the registry
	nlverify (RegSetValueEx (hKey, name, 0, REG_BINARY, (BYTE*)&wndpl, sizeof(WINDOWPLACEMENT)) == ERROR_SUCCESS);

	// Close the key
	nlverify (RegCloseKey (hKey) == ERROR_SUCCESS);
}

void CGeorgesEditApp::loadWindowState (CWnd *wnd, const char *name, bool mdiChildWnd, bool controlBar)
{
	HKEY hKey;
	if (RegOpenKey (HKEY_CURRENT_USER, GEORGES_EDIT_BASE_REG_KEY"\\Windows states", &hKey) == ERROR_SUCCESS)
	{
		// Get the value
		WINDOWPLACEMENT wndpl;
		DWORD type;
		DWORD len = sizeof (WINDOWPLACEMENT);

		// Get from the registry
		if (RegQueryValueEx (hKey, name, 0, &type, (BYTE*)&wndpl, &len) == ERROR_SUCCESS)
		{
			// Length ok ?
			if (len == sizeof (WINDOWPLACEMENT))
			{
				// Set window placement
				
				// Other children
				if (mdiChildWnd)
				{
					wndpl.showCmd = SW_SHOW;
					wnd->SetWindowPos (NULL, 0, 0, wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left, 
						wndpl.rcNormalPosition.bottom-wndpl.rcNormalPosition.top, SWP_NOMOVE|SWP_NOZORDER|SWP_NOOWNERZORDER);
				}
				else
				{
					wnd->SetWindowPlacement( &wndpl );
				}
			}
		}

		nlverify (RegCloseKey (hKey) == ERROR_SUCCESS);
	}
}

void CGeorgesEditApp::saveState ()
{
	// Save the main window state
	nlassert (m_pMainWnd);
	if (ResizeMain)
		saveWindowState (m_pMainWnd, "main", false);

	saveWindowState (&((CMainFrame*)m_pMainWnd)->FileBrowser, "browser", true);
	saveWindowState (&((CMainFrame*)m_pMainWnd)->OutputConsole, "output", true);
}

void CGeorgesEditApp::loadState ()
{
	nlassert (m_pMainWnd);
	if (ResizeMain)
		loadWindowState (m_pMainWnd, "main", false, false);

	loadWindowState (&((CMainFrame*)m_pMainWnd)->FileBrowser, "browser", false, true);
	loadWindowState (&((CMainFrame*)m_pMainWnd)->OutputConsole, "output", false, true);
}

void CGeorgesEditApp::OnFileSaveAll() 
{
	SaveAllModified  ();
}

void CGeorgesEditApp::OnFileCloseAll() 
{
	SaveAllModified  ();
	CloseAllDocuments (FALSE);
}

void CGeorgesEditApp::OnUpdateFileSaveAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (getActiveDocument () != NULL);
}

bool CGeorgesEditApp::SerialIntoMemStream (const char *formName, CGeorgesEditDoc *doc, uint slot, bool copyToClipboard)
{
	// Ok, get the node
	const CFormDfn *parentDfn;
	uint lastElement;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	UFormDfn::TEntryType type;
	bool array;
	bool parentVDfnArray;
	nlverify (((const CFormElm*)(doc->getRootNode (slot)))->getNodeByName ( formName, &parentDfn, lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));

	// Output mode
	if (MemStream.isReading ())
		MemStream.invert ();

	// Init the stream
	MemStream.clear ();

	// Create a serial
	COXml serialOutput;
	serialOutput.init (&MemStream);

	// Catch some prb
	try
	{
		// Serial the string
		string name;
		if (parentDfn)
			name = parentDfn->getEntry(lastElement).getFilename ();
		else
		{
			// Get the string
			CString str = doc->GetPathName ();
			name = str;
			uint pos = name.rfind ('.');
			if (pos != string::npos)
			{
				// Keep the extension
				name = name.substr (pos+1);
				name += ".dfn";
			}
		}
		MemStream.serial (name);

		// Data presents ?
		bool presents = node != NULL;

		// Serial the flag
		MemStream.serial (presents);

		if (presents)
		{
			// Serial the buffer
			xmlNodePtr nodeXml = xmlNewDocNode (serialOutput.getDocument (), NULL, (const xmlChar*)"CLIPBOARD", NULL);
			xmlDocSetRootElement (serialOutput.getDocument (), nodeXml);

			// Struct type ?
			if (node)
			{
				if (array)
				{
					(safe_cast<const CFormElmArray*> (node))->write (nodeXml, doc->getFormPtr(), NULL);
				}
				else
				{
					if (type == UFormDfn::EntryDfn)
					{
						(safe_cast<const CFormElmStruct*> (node))->write (nodeXml, doc->getFormPtr(), NULL);
					}
					else if (type == UFormDfn::EntryVirtualDfn)
					{
						(safe_cast<const CFormElmVirtualStruct*> (node))->write (nodeXml, doc->getFormPtr(), NULL);
					}
					else if (type == UFormDfn::EntryType)
					{
						(safe_cast<const CFormElmAtom*> (node))->write (nodeXml, doc->getFormPtr(), NULL);
					}
				}
			}
			else
			{
				(safe_cast<const CFormElmVirtualStruct*> (node))->write (nodeXml, doc->getFormPtr(), NULL);
			}
		}

		// Flush the XML stream
		serialOutput.flush ();

		// Debug test
		/* FILE *output = fopen ("c:\\toto.txt", "wb");
		fwrite ((void*)MemStream.buffer (), 1, MemStream.length (), output);
		fclose (output); */

		// Write to clipboard
		if (copyToClipboard)
		{
			// Is an array ?
			if (array)
			{
				((CMainFrame*)m_pMainWnd)->dataToClipboard (FormClipBoardFormatArray, (void*)MemStream.buffer (),
					MemStream.length ());
			}
			else
			{
				// Dfn ?
				if (type == UFormDfn::EntryDfn )
				{
					((CMainFrame*)m_pMainWnd)->dataToClipboard (FormClipBoardFormatStruct, (void*)MemStream.buffer (),
						MemStream.length ());
				}
				// Virtual Dfn ?
				else if (parentDfn && type == UFormDfn::EntryVirtualDfn && !array)
				{
					((CMainFrame*)m_pMainWnd)->dataToClipboard (FormClipBoardFormatVirtualStruct, (void*)MemStream.buffer (),
						MemStream.length ());
				}
				// Type ?
				else // if (parentDfn && type == UFormDfn::EntryType && !array)
				{
					((CMainFrame*)m_pMainWnd)->dataToClipboard (FormClipBoardFormatType, (void*)MemStream.buffer (),
						MemStream.length ());
				}
			}
		}

		// Ok
		return true;
	}
	catch (Exception &)
	{
		nlstop;
	}

	// Not ok
	return false;
}

void CGeorgesEditApp::FillMemStreamWithBuffer (const uint8 *buffer, uint size)
{
	// Input mode
	if (!theApp.MemStream.isReading ())
		theApp.MemStream.invert ();

	// Init the stream
	theApp.MemStream.clear ();

	// Buffer to fill
	uint8 *bufferPtr = theApp.MemStream.bufferToFill (size);
	memcpy (bufferPtr, buffer, size);
}

bool CGeorgesEditApp::FillMemStreamWithClipboard (const char *formName, CGeorgesEditDoc *doc, uint slot)
{
	// Input mode
	if (!theApp.MemStream.isReading ())
		theApp.MemStream.invert ();

	// The form pointer
	CForm *form = doc->getFormPtr ();

	// Ok, get the node
	const CFormDfn *parentDfn;
	uint lastElement;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	UFormDfn::TEntryType type;
	bool array;
	bool parentVDfnArray;
	nlverify (((CFormElm*)doc->getRootNode(slot))->getNodeByName ( formName, &parentDfn, 
		lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));

	// Get the clipboard type
	UINT clipboardType;
	if (array)
		clipboardType = theApp.FormClipBoardFormatArray;
	else if (type == UFormDfn::EntryType)
		clipboardType = theApp.FormClipBoardFormatType;
	else if (type == UFormDfn::EntryDfn)
		clipboardType = theApp.FormClipBoardFormatStruct;
	else // if (type == UFormDfn::EntryVirtualDfn)
		clipboardType = theApp.FormClipBoardFormatVirtualStruct;

	// Get the clipboard size
	uint size;

	// Get the clipboard size
	if (((CMainFrame*)theApp.m_pMainWnd)->clipboardSize (clipboardType, size))
	{
		// Init the stream
		theApp.MemStream.clear ();

		// Fill the input buffer
		uint8 *bufferPtr = theApp.MemStream.bufferToFill (size);
		nlverify (((CMainFrame*)theApp.m_pMainWnd)->dataFromClipboard (clipboardType, bufferPtr));
		return true;
	}
	else
		return false;
}

bool CGeorgesEditApp::SerialFromMemStream (const char *formName, CGeorgesEditDoc *doc, uint slot)
{
	// The form pointer
	CForm *form = doc->getFormPtr ();

	// Ok, get the node
	const CFormDfn *parentDfn;
	uint lastElement;
	const CFormDfn *nodeDfn;
	const CType *nodeType;
	CFormElm *node;
	UFormDfn::TEntryType type;
	bool array;
	bool parentVDfnArray;
	nlverify (((CFormElm*)(doc->getRootNode (slot)))->getNodeByName ( formName, &parentDfn, 
		lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND));

	// Get the clipboard type
	UINT clipboardType;
	if (array)
		clipboardType = theApp.FormClipBoardFormatArray;
	else if (type == UFormDfn::EntryType)
		clipboardType = theApp.FormClipBoardFormatType;
	else if (type == UFormDfn::EntryDfn)
		clipboardType = theApp.FormClipBoardFormatStruct;
	else // if (type == UFormDfn::EntryVirtualDfn)
		clipboardType = theApp.FormClipBoardFormatVirtualStruct;

	// Catch some prb
	try
	{
		// Serial the string
		string nameParent;
		string name;
		if (parentDfn)
			nameParent = parentDfn->getEntry(lastElement).getFilename ();
		else
		{
			// Get the string
			CString str = doc->GetPathName ();
			nameParent = str;
			uint pos = nameParent.rfind ('.');
			if (pos != string::npos)
			{
				// Keep the extension
				nameParent = nameParent.substr (pos+1);
				nameParent += ".dfn";
			}
		}
		theApp.MemStream.serial (name);

		// Same DFN ?
		if (name == nameParent)
		{
			// Is in the clipboard
			bool present;
			theApp.MemStream.serial (present);

			// Is present
			if (present)
			{
				// Create the node
				bool created;
				nlverify (((CFormElm*)(doc->getRootNode (slot)))->createNodeByName ( formName, &parentDfn, 
					lastElement, &nodeDfn, &nodeType, &node, type, array, created));

				// For struct
				if (clipboardType == theApp.FormClipBoardFormatStruct)
				{
					// Struct pointer
					CFormElmStruct *formStruct = safe_cast<CFormElmStruct*>((CFormElm*)node);

					// Create a serial
					CIXml serialInput;
					serialInput.init (theApp.MemStream);

					// Clean the node
					formStruct->clean ();

					// Serial it

					// Get first struct node
					xmlNodePtr nodeXml = CIXml::getFirstChildNode (serialInput.getRootNode (), "STRUCT");
					if (nodeXml)
					{
						formStruct->read (nodeXml, doc->FormLoader, (CFormDfn*)nodeDfn, form);
					}
				}
				// For array
				else if (clipboardType == theApp.FormClipBoardFormatArray)
				{
					// Struct pointer
					CFormElmArray *formArray = safe_cast<CFormElmArray*>((CFormElm*)node);

					// Create a serial
					CIXml serialInput;
					serialInput.init (theApp.MemStream);

					// Clean the node
					formArray->clean ();

					// Serial it

					// Get first struct node
					xmlNodePtr nodeXml = CIXml::getFirstChildNode (serialInput.getRootNode (), "ARRAY");
					if (nodeXml)
					{
						// Should be a type or a DFN
						nlassert ((nodeType) || (nodeDfn));
						formArray->read (nodeXml, doc->FormLoader, form);
					}
				}
				// For atom
				else if (clipboardType == theApp.FormClipBoardFormatType)
				{
					// Struct pointer
					CFormElmAtom *formAtom = safe_cast<CFormElmAtom*>((CFormElm*)node);

					// Create a serial
					CIXml serialInput;
					serialInput.init (theApp.MemStream);

					// Serial it

					// Get first struct node
					xmlNodePtr nodeXml = CIXml::getFirstChildNode (serialInput.getRootNode (), "ATOM");
					if (nodeXml)
					{
						// Should be a type
						nlassert (nodeType);
						formAtom->read (nodeXml, doc->FormLoader, (CType*)nodeType, form);
					}
				}
				// For virtual struct
				else if (clipboardType == theApp.FormClipBoardFormatVirtualStruct)
				{
					// Struct pointer
					CFormElmVirtualStruct *vitualStruct = safe_cast<CFormElmVirtualStruct*>((CFormElm*)node);

					// Create a serial
					CIXml serialInput;
					serialInput.init (theApp.MemStream);

					// Clean the node
					vitualStruct->clean ();

					// Serial it

					// Get first struct node
					xmlNodePtr nodeXml = CIXml::getFirstChildNode (serialInput.getRootNode (), "VSTRUCT");
					if (nodeXml)
					{
						vitualStruct->read (nodeXml, doc->FormLoader, form);
					}
				}
			} // if (present)
			else
			{
				// Does the node exist ?
				// Ok, get the node
				const CFormDfn *parentDfn;
				uint lastElement;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *node;
				UFormDfn::TEntryType type;
				bool array;
				bool parentVDfnArray;
				if (((CFormElm*)(doc->getRootNode (slot)))->getNodeByName ( formName, &parentDfn, 
					lastElement, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND))
				{
					// Delete the node if it exist in this form
					if (node && (node->getForm () == doc->getFormPtr ()) )
					{
						nlverify (((CFormElm*)(doc->getRootNode (slot)))->deleteNodeByName ( formName, &parentDfn, 
							lastElement, &nodeDfn, &nodeType, &node, type, array));
					}
				}									
			}

			// Not ok
			return true;
		}
	}
	catch (Exception &e)
	{
		nlwarning ("Error while paste: %s", e.what());
	}

	// Not ok
	return false;
}

BOOL CGeorgesEditApp::OnDDECommand(LPTSTR lpszCommand) 
{
	if (strncmp (lpszCommand, "Open", 4) == 0)
	{
		string name = lpszCommand;
		name = name.substr (6, name.size ()-8);
		OpenDocumentFile  (name.c_str ());
	}
	else if (strncmp (lpszCommand, "Georges Copy", 4) == 0)
	{
		// Get ext name
		string name = lpszCommand;
		name = name.substr (6, name.size ()-8);

		// Get the extension
		char ext[MAX_PATH];
		_splitpath (name.c_str (), NULL, NULL, NULL, ext);
		string dfnName = string (ext+1) + ".dfn";

		// Get the doc template
		CMultiDocTemplate *docTemplate = getFormDocTemplate (dfnName.c_str ());

		if (docTemplate)
		{
			// Create a new document
			CGeorgesEditDocForm *doc = (CGeorgesEditDocForm*)docTemplate->CreateNewDocument();
			nlassert (doc);
			nlassert (doc->isForm ());

			// Create the frame
			CFrameWnd* pFrame = docTemplate->CreateNewFrame(doc, NULL);
			nlassert (pFrame);
			if (doc->loadFormFile (name.c_str ()))
			{

				doc->updateDocumentStructure ();

				// Init the frame
				docTemplate->InitialUpdateFrame (pFrame, doc, TRUE);

				// Init the document
				if (doc->initDocument (dfnName.c_str (), false))
				{
					doc->changeSubSelection ((CGeorgesEditDocSub *)NULL, NULL);
					doc->UpdateAllViews (NULL);
				}
				else
				{
					docTemplate->RemoveDocument( doc );
					doc = NULL;
					char tmp[512];
					smprintf (tmp, 512, "Can't open document '%s'.", name.c_str ());
					outputError (tmp);
				}
			}
			else
			{
				docTemplate->RemoveDocument( doc );
				doc = NULL;
				char tmp[512];
				smprintf (tmp, 512, "Can't open document '%s'.", name.c_str ());
				outputError (tmp);
			}
		}
		else
		{
			char tmp[512];
			smprintf (tmp, 512, "Can't open document '%s'.", name.c_str ());
			outputError (tmp);
		}
	}
	else if (strncmp (lpszCommand, "Derive", 6) == 0)
	{
		// Get ext name
		string name = lpszCommand;
		name = name.substr (8, name.size ()-10);

		// Get the extension
		char ext[MAX_PATH];
		_splitpath (name.c_str (), NULL, NULL, NULL, ext);
		string dfnName = string (ext+1) + ".dfn";

		// Create a document
		CGeorgesEditDocForm *doc = (CGeorgesEditDocForm*)createDocument (dfnName.c_str (), "");
		if (doc)
		{
			char nameFile[MAX_PATH];
			char extFile[MAX_PATH];
			_splitpath (name.c_str (), NULL, NULL, nameFile, extFile);
			doc->addParent ((string (nameFile) + extFile).c_str ());
			doc->updateDocumentStructure ();
			doc->UpdateAllViews (NULL);
		}
		else
		{
			char tmp[512];
			smprintf (tmp, 512, "Can't derive from document '%s'.", name.c_str ());
			outputError (tmp);
		}
	}
	else if (strncmp (lpszCommand, "CreateForm", 10) == 0)
	{
		// Get ext name
		string name = lpszCommand;
		name = name.substr (10, name.size ()-12);

		// Get the extension
		char name2[MAX_PATH];
		char ext[MAX_PATH];
		_splitpath (name.c_str (), NULL, NULL, name2, ext);
		string dfnName = name2;
		dfnName += ext;

		// Create a document
		CGeorgesEditDocForm *doc = (CGeorgesEditDocForm*)createDocument (dfnName.c_str (), "");
		if (doc)
		{
			doc->updateDocumentStructure ();
			doc->UpdateAllViews (NULL);
		}
		else
		{
			char tmp[512];
			smprintf (tmp, 512, "Can't create a file from dfn '%s'.", name.c_str ());
			outputError (tmp);
		}
	}
	return CWinApp::OnDDECommand(lpszCommand);
}

CMultiDocTemplate	*CGeorgesEditApp::getFormDocTemplate (const char *dfnName)
{
	if (Superuser)
	{
		return _TemplateForm;
	}
	else
	{
		POSITION pos = GetFirstDocTemplatePosition ();
		while (pos)
		{
			// Get the template
			CMyMultiDocTemplate *docTemp = safe_cast<CMyMultiDocTemplate*> (GetNextDocTemplate(pos));

			// Check its dfn name
			std::string dfnNameTemplate;
			docTemp->getDfnName (dfnNameTemplate);
			if (dfnName == dfnNameTemplate)
				return docTemp;
		}
	}

	// Not found
	return NULL;
}