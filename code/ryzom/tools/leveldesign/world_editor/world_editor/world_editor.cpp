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

// world_editor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "world_editor.h"
#include <nel/3d/nelu.h>
#include <nel/misc/dynloadlib.h>

#include "main_frm.h"
#include "world_editor_doc.h"
#include "world_editor_view.h"
#include "editor_primitive.h"
#include "dialog_properties.h"
#include "primitive_configuration_dlg.h"
#include "splash_screen.h"
#include "file_dialog_ex.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

bool	DontUse3D = false;

//CSplashScreen* splashScreen=new CSplashScreen;

// ***************************************************************************
// CWorldEditorApp

BEGIN_MESSAGE_MAP(CWorldEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

// ***************************************************************************
// CWorldEditorApp construction

CWorldEditorApp::CWorldEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CWorldEditorApp::~CWorldEditorApp()
{	
}

void CWorldEditorApp::deletePlugins()
{
	for(std::vector<IPluginCallback*>::iterator it = Plugins.begin(); it != Plugins.end(); ++it)
	{
		delete *it;
	}
}



// ***************************************************************************
// The one and only CWorldEditorApp object

CWorldEditorApp theApp;

// ***************************************************************************
// CWorldEditorApp initialization

BOOL CWorldEditorApp::InitInstance()
{
	// init context
	new CApplicationContext;
	// Register class

	// Filter the log
	NLMISC::createDebug();
	NLMISC::WarningLog->addNegativeFilter ("in not a string");
	CSplashScreen	splashScreen;
	splashScreen.Create(IDD_SPLASHSCREEN, theApp.GetMainWnd());
	splashScreen.ShowWindow(TRUE);
	splashScreen.addLine(string("Register my classes for ligo"));

	// Register my classes for ligo
	NLMISC::CClassRegistry::registerClass("CPrimPoint", CPrimPointEditor::creator, typeid(CPrimPointEditor).name());
	NLMISC::CClassRegistry::registerClass("CPrimNode", CPrimNodeEditor::creator, typeid(CPrimNodeEditor).name());
	NLMISC::CClassRegistry::registerClass("CPrimPath", CPrimPathEditor::creator, typeid(CPrimPathEditor).name());
	NLMISC::CClassRegistry::registerClass("CPrimZone", CPrimZoneEditor::creator, typeid(CPrimZoneEditor).name());
	NLMISC::CClassRegistry::registerClass("CPrimAlias", CPrimAliasEditor::creator, typeid(CPrimAliasEditor).name());

	splashScreen.addLine(string("Init the quad tree"));
	
	// Init the quad tree
	InitQuadGrid ();

	NLMISC_REGISTER_CLASS(CPrimBitmap);

	splashScreen.addLine(string("Init primitive class manager"));
	

	// Init primitive class manager
	if (!initPath ("world_editor_script.xml", splashScreen))
		return FALSE;

	splashScreen.addLine(string("Exe path"));
	
	// Exe path
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
	ExePath = CPath::standardizePath (NLMISC::CFile::getPath (ExePath));

	splashScreen.addLine(string("Load the Ligoscape.cfg"));
	
	// Load the Ligoscape.cfg
	try
	{
		string sConfigFileName = ExePath;
		sConfigFileName += "ligoscape.cfg";

		splashScreen.addLine(string("Load the config file"));
		
		// Load the config file
		if (!Config.readConfigFile (sConfigFileName.c_str(), true))
			return FALSE;

		// set the primitive context
		CPrimitiveContext::instance().CurrentLigoConfig = &Config;
	}
	catch (Exception& e)
	{
		::MessageBox (NULL, e.what(), "Warning", MB_OK|MB_ICONEXCLAMATION);

		// Can't found the module put some default values
		Config.CellSize = 160.0f;
		Config.Snap = 1.0f;
	}

	AfxEnableControlContainer();

	splashScreen.addLine(string("Standard initialization"));
	

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	splashScreen.addLine(string("Change the registry key"));
	
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Nevrax"));

	LoadStdProfileSettings(15);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWorldEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWorldEditorView));
	AddDocTemplate(pDocTemplate);

	splashScreen.addLine(string("Enable DDE Execute open"));
	

	// Enable DDE Execute open
	EnableShellOpen();
	RegDeleteKey (HKEY_CLASSES_ROOT, "Worldeditor.Document\\DefaultIcon");				
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	splashScreen.addLine(string("Fill image list"));
	
	// Fill image list
	ImageList.create (16, 16);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_POINT_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_POINT_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_POINT_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_LINE_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_LINE_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_LINE_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ZONE_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ZONE_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ZONE_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_FOLDER_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_FOLDER_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_FOLDER_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_PROPERTY_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_PROPERTY_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_PROPERTY_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ROOT_HIDDEN);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ROOT_CLOSED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ROOT_OPENED);
	ImageList.addResourceIcon (theApp.m_hInstance, IDI_ERROR_STRUCTURE);

	// Get icones 
	vector<string> result;
	CPath::getPathContent ("ui", true, false, true, result);
	for (uint i=0; i<result.size (); i++)
	{
		// This is an icon ?
		if (strlwr (NLMISC::CFile::getExtension (result[i])) == "ico")
			ImageList.addResourceIcon (result[i].c_str ());
	}

	splashScreen.addLine(string("Load the plugins"));
	

	// Load the plugins
	loadPlugins();

	splashScreen.addLine(string("Init the plugin menu"));
	
	//Init the plugin menu
	
	CMenu* dynamic_menu;
	
	dynamic_menu=getMainFrame()->GetMenu();

	CMenu* menu = new CMenu();
	menu->CreatePopupMenu();
	dynamic_menu->InsertMenu(6, MF_BYPOSITION | MF_POPUP, (UINT)menu->GetSafeHmenu(), "Plugins");

	for(uint k=0;k<Plugins.size();k++)
	{
		IPluginCallback* test=Plugins[k];
		string retest=test->getName();
		menu->InsertMenu( k, MF_BYPOSITION | MF_POPUP, ID_WINDOWS_PLUGINS+1 + k, retest.c_str() );
		menu->CheckMenuItem(ID_WINDOWS_PLUGINS+1 +k, MF_CHECKED);				
	}
	
	getMainFrame()->DrawMenuBar();
	getMainFrame()->onMenuModeLogic();

	// Reset the primitive configurations
	reloadPrimitiveConfiguration ();
	splashScreen.ShowWindow(false);
	splashScreen.DestroyWindow();

	return TRUE;
}

void CWorldEditorApp::loadPlugins()
{
	// 1st load the plugin configuration file
//	char curDir[MAX_PATH];
//	GetCurrentDirectory (MAX_PATH, curDir);

	PluginConfig.load((ExePath+"/world_editor_plugin.cfg").c_str());

	// enumerate the plugin variable.
	CConfigFile::CVar *plugins = PluginConfig.getVarPtr("PluginsLibrary");

	if (plugins != 0)
	{
		for (uint i=0; i<plugins->size(); ++i)
		{
			string libname = plugins->asString(i)+nlLibSuffix+".dll";

			//vl HMODULE h = AfxLoadLibrary(libname.c_str());
			NL_LIB_HANDLE h = nlLoadLibrary(libname.c_str());
			if (h == NULL)
				continue;

			//FCreateSoundPlugin *pf = (FCreateSoundPlugin*) GetProcAddress(h, "createSoundPlugin");
			//JC: switch to the generic call as soon as possible
			//vl FCreatePlugin *pf = (FCreatePlugin*) GetProcAddress(h, "createPlugin");
			FCreatePlugin *pf = (FCreatePlugin*) nlGetSymbolAddress(h, "createPlugin");

			if (pf == NULL)
				continue;

			// ok, the plugin is loaded.
			IPluginCallback *pcb = (IPluginCallback*) pf();

			if (pcb == 0)
				continue;

			nlinfo("Plugins '%s' loaded", libname.c_str());
			Plugins.push_back(pcb);

			// init the plugin.
			pcb->init(getMainFrame());
		}
	}
}

// ***************************************************************************

bool CWorldEditorApp::reloadPrimitiveConfiguration ()
{
	// Reset
	Config.resetPrimitiveConfiguration ();

	// Load
	if (!Config.readPrimitiveClass ("world_editor_primitive_configuration.xml", true))
		return false;

	// Init configurations
	Configurations.clear ();
	Configurations.resize (theApp.Config.getPrimitiveConfiguration().size());

	// Create the PrimitiveConfigurationDlg
	PrimitiveConfigurationDlg.destroy ();
	PrimitiveConfigurationDlg.Create (IDD_PRIMITIVE_CONFIGURATION, m_pMainWnd);
	return true;
}

// ***************************************************************************
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_videoMem;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_videoMem = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_STATIC_VIDEO_MEM, m_videoMem);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWorldEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

bool CWorldEditorApp::yesNoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	return MessageBox (m_pMainWnd?m_pMainWnd->m_hWnd:NULL, buffer, "NeL World Editor", MB_YESNO|MB_ICONQUESTION) == IDYES;
}

void CWorldEditorApp::errorMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	MessageBox (m_pMainWnd?m_pMainWnd->m_hWnd:NULL, buffer, "NeL World Editor", MB_OK|MB_ICONEXCLAMATION);
}

void CWorldEditorApp::infoMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	MessageBox (m_pMainWnd?m_pMainWnd->m_hWnd:NULL, buffer, "NeL World Editor", MB_OK|MB_ICONINFORMATION);
}

void CWorldEditorApp::syntaxError (const char *filename, xmlNodePtr xmlNode, const char *format, ...)
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	errorMessage ("(%s), node (%s), line (%d) :\n%s", filename, xmlNode->name, (int)xmlNode->content, buffer);
}

bool CWorldEditorApp::getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName)
{
	// Call the CIXml version
	if (!CIXml::getPropertyString (result, xmlNode, propName))
	{
		// Output a formated error
		syntaxError (filename, xmlNode, "Missing XML node property (%s)", propName);
		return false;
	}
	return true;
}

bool CWorldEditorApp::initPath (const char *filename, CSplashScreen &splashScreen)
{
	// The context strings
	set<string> contextStrings;

	// Read the document
	CIFile file;
	if (file.open (filename))
	{
		try
		{
			// XML stream
			CIXml xml;
			xml.init (file);

			// Get the root node
			xmlNodePtr root = xml.getRootNode ();
			nlassert (root);
			
			// Check the header
			if (strcmp ((const char*)root->name, "NEL_WORLD_EDITOR_CONFIG") == 0)
			{
				// Get the first search_path description
				xmlNodePtr search_path = CIXml::getFirstChildNode (root, "SEARCH_PATH");
				if (search_path)
				{
					do
					{
						// Get the search_path name
						std::string path;
						if (getPropertyString (path, filename, search_path, "PATH"))
						{
							bool	recurse = true;
							string noRecurse;
							if (CIXml::getPropertyString (noRecurse, search_path, "NO_RECURSE"))
							{
								if (toLower(noRecurse) == "true")
									recurse = false;
							}

							// Add the search path
							CPath::addSearchPath (path, recurse, true, &splashScreen);
						}
						else
							goto failed;
					}
					while (search_path = CIXml::getNextChildNode (search_path, "SEARCH_PATH"));
				}

				// Get the documentation path
				xmlNodePtr doc_path = CIXml::getFirstChildNode (root, "DOC_PATH");
				if (doc_path)
				{
					std::string path;
					if (getPropertyString (path, filename, doc_path, "PATH"))
					{
						string fullPath = CPath::getFullPath(path);
						// Store the doc path
						DocPath = fullPath;
					}
					else
						goto failed;
				}

				// Ok
				return true;
			}
			else
			{
				syntaxError (filename, root, "Wrong root node, should be NEL_WORLD_EDITOR_CONFIG");
			}
		}
		catch (Exception &e)
		{
			errorMessage ("File read error (%s):%s", filename, e.what ());
		}
	}
	else
	{
		errorMessage ("Can't open the file %s for reading.", filename);
	}

failed:
	return false;
}

// ***************************************************************************

sint CWorldEditorApp::getActiveConfiguration (const NLLIGO::IPrimitive &primitive, uint searchBegin) const
{
	const std::vector<CPrimitiveConfigurations> &configurations = theApp.Config.getPrimitiveConfiguration();

	uint i;
	for (i=searchBegin; i<configurations.size(); i++)
	{
		if (Configurations[i].Activated && configurations[i].belong (primitive))
			return i;
	}
	return -1;
}

// ***************************************************************************

class CMainFrame *getMainFrame ()
{
	return (CMainFrame *)theApp.m_pMainWnd;
}

// ***************************************************************************
// CWorldEditorApp message handlers


// ***************************************************************************

std::string standardizePath (const char *str)
{
	return NLMISC::strlwr (NLMISC::CPath::standardizePath (str, true));
}

// ***************************************************************************

std::string formatString (const char *str)
{
	string copy = NLMISC::strlwr (str);
	return copy;
}

// ***************************************************************************

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *wnd = GetDlgItem (IDC_TITLE_VERSION);
	nlassert (wnd);

	uint32 mem;
	if (DontUse3D)
		mem = 0;
	else
		mem = CNELU::Driver->profileAllocatedTextureMemory();

	m_videoMem = NLMISC::toString("%u Mo", mem/(1024*1024)).c_str();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void invalidateLeftView ()
{
	CMainFrame *mf = getMainFrame ();
	if (mf)
	{
		mf->m_wndSplitter.GetPane(0,0)->Invalidate ();
	}
}

// ***************************************************************************

std::string numberize (const char *oldString, uint value)
{
	int i=strlen (oldString)-1;
	while ((i>=0) && (((oldString[i]<='9') && (oldString[i]>='0')) || (oldString[i]==' ')))
	{
		// again
		i--;
	}

	// String new value
	string newValue = oldString;

	// Found ?
	if (i<0)
	{
		newValue.clear ();
	}
	else
	{
		newValue = newValue.substr (0, i+1);
		// Append a space if there is no space nor underscore
		if (newValue[i]!='_' && newValue[i]!=' ')
		{
			newValue += " ";
		}
	}

	// Put the value
	newValue += toString (value);
	return newValue;
}

// ***************************************************************************

bool getZoneNameFromXY (sint32 x, sint32 y, std::string &zoneName)
{
	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return false;
	zoneName = toString(-y) + "_";
	zoneName += ('A' + (x/26));
	zoneName += ('A' + (x%26));
	return true;
}

// ***************************************************************************

bool openFile (const char *filename)
{
    char key[MAX_PATH + MAX_PATH];

	// Extension
	string extension = NLMISC::CFile::getExtension (filename);

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, "open", filename, NULL,NULL, SW_SHOW);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) 
	{

        if (getRegKey(HKEY_CLASSES_ROOT, ("."+extension).c_str (), key) == ERROR_SUCCESS) 
		{
            lstrcat(key, "\\shell\\open\\command");

            if (getRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
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
                lstrcat(pos, filename);
                result = (HINSTANCE) WinExec(key, SW_SHOW);
				return ((UINT)result) >= 31;
            }
        }
    }
	else
		return true;
	return false;
}

// ***************************************************************************

uint getRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
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

// ***************************************************************************

CNoInteraction::CNoInteraction ()
{
	// Get the main frame
	CMainFrame *mainFrame = getMainFrame ();
	if (mainFrame)
		mainFrame->interaction (false);
}

// ***************************************************************************

CNoInteraction::~CNoInteraction ()
{
	// Get the main frame
	CMainFrame *mainFrame = getMainFrame ();
	if (mainFrame)
		mainFrame->interaction (true);
}

// ***************************************************************************

void CMyLigoConfig::errorMessage (const char *format, ... )
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	theApp.errorMessage (buffer);
}

// ***************************************************************************

void setEditTextMultiLine (CEdit &edit, const char *text)
{
	string temp;
	uint size = strlen (text);
	temp.reserve (2*size);
	bool previousR=false;
	for (uint c=0; c<size; c++)
	{
		if ((text[c] == '\n') && (!previousR))
			temp += "\r\n";
		else
			temp += text[c];
		previousR = (text[c] == '\r');
	}
	setWindowTextUTF8 (edit, temp.c_str ());
}	

// ***************************************************************************

void setEditTextMultiLine (CEdit &edit, const std::vector<std::string> &vect)
{
	string temp;
	uint i;
	for (i=0; i<vect.size (); i++)
	{
		temp += vect[i];
		if (i != (vect.size ()-1))
			temp += "\n";
	}
	setEditTextMultiLine (edit, temp.c_str());
}

/*
void setEditTextMultiLine (ColorEditWnd &edit, const char *text)
{
	string temp;
	uint size = strlen (text);
	temp.reserve (2*size);
	bool previousR=false;
	for (uint c=0; c<size; c++)
	{
		if ((text[c] == '\n') && (!previousR))
			temp += "\r\n";
		else
			temp += text[c];
		previousR = (text[c] == '\r');
	}
	edit.LoadText(CString(temp.c_str()));
}	

void setEditTextMultiLine (ColorEditWnd &edit, const std::vector<std::string> &vect)
{
	string temp;
	uint i;
	for (i=0; i<vect.size (); i++)
	{
		temp += vect[i];
		if (i != (vect.size ()-1))
			temp += "\n";
	}
	edit.LoadText(CString(temp.c_str()));
}	
*/
// ***************************************************************************

/*void setEditTextMultiLine (CListBox &listBox, const char *text)
{
	listBox.ResetContent();
	nlverify (listBox.InsertString( -1, text) !=  LB_ERR);
}*/

// ***************************************************************************

void setEditTextMultiLine (CListBox &listBox, const std::vector<std::string> &vect)
{
	listBox.ResetContent();
	uint i;
	for (i=0; i<vect.size (); i++)
		nlverify (listBox.InsertString( -1, vect[i].c_str()) !=  LB_ERR);
}

// ***************************************************************************

bool setWindowTextUTF8 (HWND hwnd, const char *textUtf8)
{
	ucstring str;

	str.fromUtf8 (textUtf8);
	return SetWindowTextW (hwnd, (WCHAR*)str.c_str()) != FALSE;
}

// ***************************************************************************

bool getWindowTextUTF8 (HWND hwnd, CString &textUtf8)
{
	ucstring text;
	text.resize (GetWindowTextLengthW (hwnd));
	if (GetWindowTextW (hwnd, (WCHAR*)text.c_str(), text.size()))
	{
		textUtf8 = text.toUtf8 ().c_str();
		return true;
	}
	else
		return false;
}

// ***************************************************************************

HTREEITEM insertItemUTF8 (HWND hwnd, const char *textUtf8, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	ucstring str;
	str.fromUtf8 (textUtf8);
	TVINSERTSTRUCTW insert;
	insert.hParent = hParent;
	insert.hInsertAfter = hInsertAfter;
	insert.item.mask = TVIF_TEXT;
	insert.item.pszText = (WCHAR*)str.c_str();
	return (HTREEITEM)::SendMessage (hwnd, TVM_INSERTITEMW, 0, (LPARAM)&insert);
}

// ***************************************************************************

HTREEITEM insertItemUTF8 (HWND hwnd, const char *textUtf8, int nImage, int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	ucstring str;
	str.fromUtf8 (textUtf8);
	TVINSERTSTRUCTW insert;
	insert.hParent = hParent;
	insert.hInsertAfter = hInsertAfter;
	insert.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	insert.item.pszText = (WCHAR*)str.c_str();
	insert.item.iImage = nImage;
	insert.item.iSelectedImage = nSelectedImage;
	return (HTREEITEM)::SendMessage (hwnd, TVM_INSERTITEMW, 0, (LPARAM)&insert);
}

// ***************************************************************************

bool setItemTextUTF8 ( HWND hwnd, HTREEITEM hItem, LPCTSTR lpszItem )
{
	ucstring str;
	str.fromUtf8 (lpszItem);
	TVITEMW item;
	item.mask = TVIF_TEXT;
	item.pszText = (WCHAR*)str.c_str();
	item.hItem = hItem;
	return SendMessage (hwnd, TVM_SETITEMW, 0, (LPARAM)&item) != FALSE;
}

// ***************************************************************************

bool isPrimitiveVisible (const NLLIGO::IPrimitive *primitive)
{
	const CPrimitiveClass *primClass = theApp.Config.getPrimitiveClass (*primitive);
	if ( (primClass == NULL || primClass->Visible) && !getPrimitiveEditor (primitive)->getHidden())
		return true;
	return false;
}

// ***************************************************************************

void CWorldEditorApp::OnFileOpen() 
{
	static char BASED_CODE szFilter[] = "NeL World Editor Files (*.worldedit)|*.worldedit|All Files (*.*)|*.*||";
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "worldedit", TRUE, "worldedit", NULL, 0, szFilter);
	if (dialog.DoModal() == IDOK)
	{
		CDocument &doc = *getDocument ();
		if (doc.OnOpenDocument (dialog.GetPathName()))
			doc.SetPathName (dialog.GetPathName());
	}
}

// ***************************************************************************

void CWorldEditorApp::OnFileSave() 
{
	CDocument &doc = *getDocument ();
	if (doc.IsModified ())
	{
		CString pathName = doc.GetPathName( );
		if (pathName == "")
			OnFileSaveAs ();
		else
			doc.OnSaveDocument (pathName);
	}
}

// ***************************************************************************

void CWorldEditorApp::OnFileSaveAs() 
{
	static char BASED_CODE szFilter[] = "NeL World Editor Files (*.worldedit)|*.worldedit|All Files (*.*)|*.*||";
	CFileDialogEx dialog (BASE_REGISTRY_KEY, "worldedit", FALSE, "worldedit", NULL, 0, szFilter);
	if (dialog.DoModal() == IDOK)
	{
		CWorldEditorDoc &doc = *getDocument ();
		doc.modifyProject ();
		if (doc.OnSaveDocument (dialog.GetPathName()))
			doc.SetPathName (dialog.GetPathName());
	}
}

// ***************************************************************************

std::string getTextureFile(const std::string &filename)
{
	if(NLMISC::CFile::fileExists(filename+".tga"))
		return filename+".tga";
	if(NLMISC::CFile::fileExists(filename+".png"))
		return filename+".png";
	else
	{
		nlwarning("%s doesn't exist in tga or in png", filename.c_str());
		return filename;
	}
}
