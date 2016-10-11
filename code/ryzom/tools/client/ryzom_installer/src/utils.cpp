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
#include "configfile.h"

#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

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

	return QString::fromUtf8(NLMISC::bytesToHumanReadableUnits(bytes, units).c_str());
}

bool isDirectoryEmpty(const QString &directory, bool recursize)
{
	bool res = true;

	if (!directory.isEmpty())
	{
		QDir dir(directory);

		if (dir.exists())
		{
			// process all files and directories excepted parent and current ones
			QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);

			for (int i = 0; i < list.size(); ++i)
			{
				QFileInfo fileInfo = list.at(i);

				if (fileInfo.isDir())
				{
					// don't consider empty directories as files, but process it recursively if required
					if (recursize) if (!isDirectoryEmpty(fileInfo.absoluteFilePath(), true)) return false;
				}
				else
				{
					// we found a file, directory is not empty
					return false;
				}
			}
		}
	}

	return res;
}

bool isDirectoryWritable(const QString &directory)
{
	// check if directory is writable by current user
	QFile file(directory + "/writable_test_for_ryzom_installer.txt");

	if (!file.open(QFile::WriteOnly)) return false;

	file.close();

	// remove it
	file.remove();

	return true;
}

qint64 getDirectorySize(const QString &directory, bool recursize)
{
	qint64 size = 0;

	if (!directory.isEmpty())
	{
		QDir dir(directory);

		if (dir.exists())
		{
			// process all files and directories excepted parent and current ones
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

bool shortcutExists(const QString &shortcut)
{
	return !shortcut.isEmpty() && NLMISC::CFile::isExists(qToUtf8(appendShortcutExtension(shortcut)));
}

#ifdef Q_OS_WIN32

bool createShortcut(const QString &shortcut, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir)
{
	CCOMHelper comHelper;

	IShellLinkW* psl;

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl);

	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf = NULL;

		// Set the path to the shortcut target and add the description.
		psl->SetPath(qToWide(QDir::toNativeSeparators(executable)));
		psl->SetIconLocation(qToWide(QDir::toNativeSeparators(icon)), 0);
		psl->SetDescription(qToWide(name));
		psl->SetArguments(qToWide(arguments));
		psl->SetWorkingDirectory(qToWide(QDir::toNativeSeparators(workingDir)));

		// Query IShellLink for the IPersistFile interface, used for saving the shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			QString path(shortcut + ".lnk");

			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(qToWide(QDir::toNativeSeparators(path)), TRUE);

			if (FAILED(hres))
			{
				qDebug() << "Unable to create shortcut" << path;
			}

			ppf->Release();
		}

		psl->Release();
	}

	return SUCCEEDED(hres);
}

bool resolveShortcut(const QWidget &window, const QString &shortcut, QString &path)
{
	CCOMHelper comHelper;

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
			hres = ppf->Load(qToWide(QDir::toNativeSeparators(shortcut)), STGM_READ);

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

#elif defined(NL_OS_MAC)

bool createShortcut(const QString &shortcut, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir)
{
	QString appPath(shortcut + ".app");

	// directories
	QString contentsPath(appPath + "/Contents");
	QString binaryPath(contentsPath + "/MacOS");
	QString dataPath(contentsPath + "/Resources");

	// files
	QString binaryFile(binaryPath + "/shortcut.sh");
	QString iconFile(dataPath + "/ryzom.icns");
	QString pkgInfoFile(contentsPath + "/PkgInfo");
	QString plistFile(contentsPath + "/Info.plist");

	// silently create directories
	QDir().mkpath(binaryPath);
	QDir().mkpath(dataPath);

	if (!isDirectoryWritable(binaryPath) || !isDirectoryWritable(dataPath)) return false;

	// write icon
	if (!writeResource(":/icons/ryzom.icns", iconFile)) return false;

	// write PkgInfo
	if (!writeResource(":/templates/PkgInfo", pkgInfoFile)) return false;

	// variables
	QMap<QString, QString> strings;

	// build command
	QString command = QString("exec \"%1\"").arg(executable);
	if (!arguments.isEmpty()) command += " " + arguments;

	strings.clear();
	strings["COMMAND"] = command;

	// write shortcut.sh
	if (!writeResourceWithTemplates(":/templates/shortcut.sh", binaryFile, strings)) return false;

	// set executable flags to .sh
	QFile::setPermissions(binaryFile, QFile::permissions(binaryFile) | QFile::ExeGroup | QFile::ExeUser | QFile::ExeOther);

	CConfigFile *config = CConfigFile::getInstance();

	strings.clear();
	strings["NAME"] = name;
	strings["COPYRIGHT"] = config->getProductPublisher();
	strings["VERSION"] = QApplication::applicationVersion();
	strings["IDENTIFIER"] = "com.winchgate.Ryzom-" + nameToId(name);

	// write Info.plist
	if (!writeResourceWithTemplates(":/templates/Info.plist", plistFile, strings)) return false;

	return true;
}

bool resolveShortcut(const QWidget &window, const QString &pathLink, QString &pathObj)
{
	return false;
}

#else

bool createShortcut(const QString &shortcut, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir)
{
	// build command
	QString command = executable;
	if (!arguments.isEmpty()) command += " " + arguments;

	// variables
	QMap<QString, QString> strings;
	strings["NAME"] = name;
	strings["COMMAND"] = command;
	strings["ICON"] = icon;

	// destination file
	QString path(shortcut + ".desktop");

	// replace strings
	if (!writeResourceWithTemplates(":/templates/template.desktop", path, strings)) return false;

	// set executable flags to .desktop
	QFile::setPermissions(path, QFile::permissions(path) | QFile::ExeGroup | QFile::ExeUser | QFile::ExeOther);

	return true;
}

bool resolveShortcut(const QWidget &window, const QString &pathLink, QString &pathObj)
{
	return false;
}

#endif

bool removeShortcut(const QString &shortcut)
{
	// empty links are invalid
	if (shortcut.isEmpty()) return false;

	// append extension if missing
	QString fullPath = appendShortcutExtension(shortcut);

	// link doesn't exist
	if (!NLMISC::CFile::isExists(qToUtf8(fullPath))) return false;

	// remove it
#if defined(Q_OS_MAC)
	// under OS X, it's a directory
	return QDir(fullPath).removeRecursively();
#else
	// a file under other platforms
	return QFile::remove(fullPath);
#endif
}

QString appendShortcutExtension(const QString &shortcut)
{
	QString extension;

#if defined(Q_OS_WIN32)
	extension = ".lnk";
#elif defined(Q_OS_MAC)
	extension = ".app";
#else
	extension = ".desktop";
#endif

	// already the good extension
	if (shortcut.indexOf(extension) > -1) return shortcut;

	return shortcut + extension;
}

QString getVersionFromExecutable(const QString &path)
{
	// launch executable with --version argument
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(path, QStringList() << "--version", QIODevice::ReadWrite);

	if (!process.waitForStarted()) return "";

	QByteArray data;

	// read all output
	while (process.waitForReadyRead(1000)) data.append(process.readAll());

	if (!data.isEmpty())
	{
		QString versionString = QString::fromUtf8(data);

		// parse version from output (client)
		QRegExp reg("([A-Za-z0-1_.]+) ((DEV|FV) ([0-9.]+))");
		if (reg.indexIn(versionString) > -1) return reg.cap(2);

		// parse version from output (other tools)
		reg.setPattern("([A-Za-z_ ]+) ([0-9.]+)");
		if (reg.indexIn(versionString) > -1) return reg.cap(2);
	}

#ifdef Q_OS_WIN
	// try to parse version of executable in resources
	DWORD verHandle = NULL;
	uint size = 0;
	VS_FIXEDFILEINFO *verInfo = NULL;

	// get size to be allocated
	uint32_t verSize = GetFileVersionInfoSizeW(qToWide(path), &verHandle);
	if (!verSize) return "";

	// allocate buffer
	QScopedArrayPointer<char> verData(new char[verSize]);

	// get pointer on version structure
	if (!GetFileVersionInfoW(qToWide(path), verHandle, verSize, &verData[0])) return "";

	// get pointer on version
	if (!VerQueryValueA(&verData[0], "\\", (void**)&verInfo, &size)) return "";

	// check if version is right
	if (size && verInfo && verInfo->dwSignature == 0xfeef04bd)
	{
		// Doesn't matter if you are on 32 bit or 64 bit,
		// DWORD is always 32 bits, so first two revision numbers
		// come from dwFileVersionMS, last two come from dwFileVersionLS
		return QString("%1.%2.%3.%4")
			.arg((verInfo->dwFileVersionMS >> 16) & 0xffff)
			.arg((verInfo->dwFileVersionMS >> 0) & 0xffff)
			.arg((verInfo->dwFileVersionLS >> 16) & 0xffff)
			.arg((verInfo->dwFileVersionLS >> 0) & 0xffff);
	}
#endif

	return "";
}

bool writeResource(const QString &resource, const QString &path)
{
	// all resources start with :/
	if (!resource.startsWith(":/")) return false;

	// open resource
	QFile file(resource);

	// unable to open it
	if (!file.open(QFile::ReadOnly)) return false;

	QByteArray data(file.readAll());

	file.close();

	// write file
	file.setFileName(path);

	// unable to write it
	if (!file.open(QFile::WriteOnly)) return false;

	// problem writting
	if (file.write(data) != data.length()) return false;

	file.close();

	return true;
}

bool writeResourceWithTemplates(const QString &resource, const QString &path, const QMap<QString, QString> &strings)
{
	// all resources start with :/
	if (!resource.startsWith(":/")) return false;

	// open resource
	QFile file(resource);

	// unable to open it
	if (!file.open(QFile::ReadOnly)) return false;

	// data are UTF-8 text
	QString data = QString::fromUtf8(file.readAll());

	file.close();

	// write file
	file.setFileName(path);

	// unable to write it
	if (!file.open(QFile::WriteOnly)) return false;

	// replace strings
	QMap<QString, QString>::ConstIterator it = strings.begin(), iend = strings.end();

	while (it != iend)
	{
		// replace variables with their value
		data.replace("$" + it.key(), it.value());

		++it;
	}

	// write
	file.write(data.toUtf8());
	file.close();

	return true;
}

CCOMHelper::CCOMHelper()
{
#ifdef Q_OS_WIN
	// to fix the bug with QFileDialog::getExistingDirectory hanging under Windows
	m_mustUninit = SUCCEEDED(CoInitialize(NULL));
#else
	m_mustUninit = false;
#endif
}

CCOMHelper::~CCOMHelper()
{
#ifdef Q_OS_WIN
	// only call CoUninitialize if CoInitialize succeeded
	if (m_mustUninit) CoUninitialize();
#endif
}
