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

#include "stdpch.h"
#include "utils.h"

QString qFromUtf8(const std::string &str)
{
	return QString::fromUtf8(str.c_str());
}

std::string qToUtf8(const QString &str)
{
	return str.toUtf8().constData();
}

QString qFromUtf16(const ucstring &str)
{
	return QString::fromUtf16(str.c_str());
}

ucstring qToUtf16(const QString &str)
{
	return ucstring::makeFromUtf8(qToUtf8(str));
}

QString qFromWide(const wchar_t *str)
{
	return QString::fromUtf16((ushort*)str);
}

wchar_t* qToWide(const QString &str)
{
	return (wchar_t*)str.utf16();
}

#ifdef Q_OS_WIN32

// CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces
//              to create and store a shortcut to the specified object.
//
// Returns the result of calling the member functions of the interfaces.
//
// Parameters:
// lpszPathObj  - Address of a buffer that contains the path of the object,
//                including the file name.
// lpszPathLink - Address of a buffer that contains the path where the
//                Shell link is to be stored, including the file name.
// lpszDesc     - Address of a buffer that contains a description of the
//                Shell link, stored in the Comment field of the link
//                properties.

HRESULT CreateLink(const QString &pathObj, const QString &pathLink, const QString &desc)
{
	IShellLinkW* psl;

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Set the path to the shortcut target and add the description.
		psl->SetPath(qToWide(pathObj));
		psl->SetDescription(qToWide(desc));
		psl->SetArguments(L"--profil ");
		psl->SetWorkingDirectory(L"");

		// Query IShellLink for the IPersistFile interface, used for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			// Add code here to check return value from MultiByteWideChar
			// for success.

			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(qToWide(pathLink), TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	return hres;
}

// ResolveIt - Uses the Shell's IShellLink and IPersistFile interfaces
//             to retrieve the path and description from an existing shortcut.
//
// Returns the result of calling the member functions of the interfaces.
//
// Parameters:
// hwnd         - A handle to the parent window. The Shell uses this window to
//                display a dialog box if it needs to prompt the user for more
//                information while resolving the link.
// lpszLinkFile - Address of a buffer that contains the path of the link,
//                including the file name.
// lpszPath     - Address of a buffer that receives the path of the link
//                target, including the file name.
// lpszDesc     - Address of a buffer that receives the description of the
//                Shell link, stored in the Comment field of the link
//                properties.

HRESULT ResolveIt(HWND hwnd, const QString &linkFile, QString &path)
{
	IShellLinkW* psl;
	WIN32_FIND_DATAW wfd;

	path.clear(); // Assume failure

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface.
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

		if (SUCCEEDED(hres))
		{
			// Add code here to check return value from MultiByteWideChar
			// for success.

			// Load the shortcut.
			hres = ppf->Load(qToWide(linkFile), STGM_READ);

			if (SUCCEEDED(hres))
			{
				// Resolve the link.
				hres = psl->Resolve(hwnd, 0);

				if (SUCCEEDED(hres))
				{
					WCHAR szGotPath[MAX_PATH];

					// Get the path to the link target.
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATAW*)&wfd, SLGP_SHORTPATH);

					if (SUCCEEDED(hres))
					{
						WCHAR szDescription[MAX_PATH];

						// Get the description of the target.
						hres = psl->GetDescription(szDescription, MAX_PATH);

						if (SUCCEEDED(hres))
						{
							// Handle success
							path = qFromWide(szGotPath);
						}
						else
						{
						}
					}
				}
			}

			// Release the pointer to the IPersistFile interface.
			ppf->Release();
		}

		// Release the pointer to the IShellLink interface.
		psl->Release();
	}

	return hres;
}

#endif
