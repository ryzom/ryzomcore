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

QString qBytesToHumanReadable(qint64 bytes)
{
	static std::vector<std::string> units;

	if (units.empty())
	{
		units.push_back(QObject::tr("B").toUtf8().constData());
		units.push_back(QObject::tr("KiB").toUtf8().constData());
		units.push_back(QObject::tr("MiB").toUtf8().constData());
		units.push_back(QObject::tr("GiB").toUtf8().constData());
		units.push_back(QObject::tr("TiB").toUtf8().constData());
		units.push_back(QObject::tr("PiB").toUtf8().constData());
	}

	return QString::fromUtf8(NLMISC::bytesToHumanReadable(bytes).c_str());
}

qint64 getDirectorySize(const QString &directory, bool recursize)
{
	qint64 size = 0;

	if (!directory.isEmpty())
	{
		QDir dir(directory);

		if (dir.exists())
		{
			QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);

			for (int i = 0; i < list.size(); ++i)
			{
				QFileInfo fileInfo = list.at(i);

				if (fileInfo.isDir())
				{
					if (recursize) size += getDirectorySize(fileInfo.absoluteFilePath(), true);
				}
				else
				{
					size += fileInfo.size();
				}
			}
		}
	}

	return size;
}

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

bool createLink(const QString &link, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir)
{
	IShellLinkW* psl;

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Set the path to the shortcut target and add the description.
		psl->SetPath(qToWide(QDir::toNativeSeparators(executable)));
		psl->SetIconLocation(qToWide(QDir::toNativeSeparators(icon)), 0);
		psl->SetDescription(qToWide(name));
		psl->SetArguments(qToWide(arguments));
		psl->SetWorkingDirectory(qToWide(QDir::toNativeSeparators(workingDir)));

		// Query IShellLink for the IPersistFile interface, used for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			// Add code here to check return value from MultiByteWideChar
			// for success.

			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(qToWide(QDir::toNativeSeparators(link)), TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}

bool resolveLink(const QWidget &window, const QString &linkFile, QString &path)
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
			hres = ppf->Load(qToWide(QDir::toNativeSeparators(linkFile)), STGM_READ);

			if (SUCCEEDED(hres))
			{
				// Resolve the link.
				hres = psl->Resolve((HWND)window.winId(), 0);

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
							path = QDir::fromNativeSeparators(qFromWide(szGotPath));
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

	return SUCCEEDED(hres);
}

#else

bool createLink(const QString &link, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir)
{
	// open template
	QFile file(":/templates/template.desktop");

	if (!file.open(QFile::ReadOnly)) return false;

	QString data = QString::fromUtf8(file.readAll());

	file.close();

	// build command
	QString command = executable;
	if (!arguments.isEmpty()) command += " " + arguments;

	// replace strings
	data.replace("$NAME", name);
	data.replace("$COMMAND", command);
	data.replace("$ICON", icon);

	// write file
	file.setFileName(link);

	if (!file.open(QFile::WriteOnly)) return false;

	file.write(data.toUtf8());
	file.close();

	return true;
}

bool resolveLink(const QWidget &window, const QString &pathLink, QString &pathObj)
{
	return false;
}

#endif
