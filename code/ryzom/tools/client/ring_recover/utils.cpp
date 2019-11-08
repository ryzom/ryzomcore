//#include <afx.h>

#include <shlobj.h>
#include <objbase.h>
#include <shlguid.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include "nel/misc/path.h"

/*
	Determine the directory containing the program
	----------------------------------------------

	Use the following function to determine the directory path containing the running program: 
*/

static std::string GetExeDir()
{
	std::string dir= NLMISC::CPath::standardizeDosPath(NLMISC::CPath::getFullPath(NLMISC::CFile::getPath(__argv[0])));
	return dir;
}

/*
	Determine the path of special folders
	-------------------------------------

	Use the following code to determine the path of special folders (e.g. the Desktop folder, the Start Menu folder, etc).
	Read the on-line help topics entitled "CSIDL Values" to determine the nFolder argument value that applies to the folder
	of interest. For example, call CShellUtils::GetSpecialFolderLocation with an argument of CSIDL_DESKTOP will return the
	Windows Desktop folder path.
*/

static std::string GetSpecialFolderLocation(int nFolder)
{
	std::string Result;
	
	LPITEMIDLIST pidl;
	HRESULT hr = SHGetSpecialFolderLocation(NULL, nFolder, &pidl);
	
	if (SUCCEEDED(hr))
	{
		// Convert the item ID list's binary
		// representation into a file system path
		char szPath[_MAX_PATH];

		if (SHGetPathFromIDList(pidl, szPath))
		{
			Result = szPath;
		}
		else
		{
			printf("%s: %d: ERROR: Failed to get special folder location",__FILE__,__LINE__);
			return "";
		}
	}
	else
	{
		printf("%s: %d: ERROR: Failed to get special folder location",__FILE__,__LINE__);
		return "";
	}
	
	return Result;
}



/*
	Create shortcuts (on the Desktop, in the Start Menu, etc.)
	----------------------------------------------------------

	Use the following code to create shortcuts. The ExePath argument should be the full path of an executable program file,
	e.g. "e:\Vault\Vault.exe". The LinkFilename should not be a full path, e.g. "Vault.lnk". The "Description" is free-form
	text (I don't know how to display this text after the shortcut is created. If you do, please let me know.

	Read the on-line help topics entitled "CSIDL Values" to determine the nFolder argument value corresponding to the kind of
	shortcut that you want to create. For example, use an nFolder argument of CSIDL_DESKTOP to create a Desktop shortcut.

	Note: Be sure to call CoInitialize before calling CShellUtils::CreateShortcut, and be sure to call CoUninitialize after
	calling CoInitialize. You can call CoInitialize(NULL); in your CWinApp-derivative's constructor and call CoUninitialize();
	in its destructor. 
*/

static void CreateShortcut(const std::string& ExePath, const std::string& LinkFilename, const std::string& WorkingDirectory, const std::string& Description, int nFolder)
{
	// Must have called CoInitalize before this function is called!

    IShellLink* psl; 
	const std::string PathLink = GetSpecialFolderLocation(nFolder) + "\\" + LinkFilename;
	
    // Get a pointer to the IShellLink interface. 
    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
									IID_IShellLink, (PVOID *) &psl); 
	
    if (SUCCEEDED(hres)) 
	{ 
        IPersistFile* ppf; 
		
        // Set the path to the shortcut target and add the 
        // description. 
        psl->SetPath((LPCSTR) ExePath.c_str()); 
		psl->SetWorkingDirectory((LPCSTR) WorkingDirectory.c_str());
        psl->SetDescription((LPCSTR) Description.c_str()); 
		
		// Query IShellLink for the IPersistFile interface for saving the 
		// shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (PVOID *) &ppf); 
		
        if (SUCCEEDED(hres)) 
		{ 
            WORD wsz[MAX_PATH]; 
			
            // Ensure that the string is ANSI. 
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) PathLink.c_str(), -1, wsz, MAX_PATH); 
			
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(wsz, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
}

void addShortcutToDesktop()
{
	CoInitialize(NULL);

	std::string dir= GetExeDir();
	printf("Exe is installed in directory: %s\n",dir.c_str());
	CreateShortcut(dir+"ring\\client_ryzom_dev_rd.exe","Ryzom Ring.lnk",dir+"ring\\","launch the ryzom ring",CSIDL_DESKTOP);
	CreateShortcut(dir+"ring\\client_ryzom_dev_rd.exe","Ryzom Ring.lnk",dir+"ring\\","launch the ryzom ring",CSIDL_PROGRAMS);

	CoUninitialize();
}