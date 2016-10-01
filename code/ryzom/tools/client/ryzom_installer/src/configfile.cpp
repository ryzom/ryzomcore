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
#include "configfile.h"
#include "utils.h"

#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CConfigFile *CConfigFile::s_instance = NULL;

CConfigFile::CConfigFile(QObject *parent):QObject(parent), m_version(-1),
	m_defaultServerIndex(0), m_defaultProfileIndex(0), m_use64BitsClient(false), m_shouldUninstallOldClient(true)
{
	s_instance = this;

	// only keep language ISO 639 code
	m_language = QLocale::system().name().left(2);

	// default config file in included in resources
	m_defaultConfigPath = ":/templates/ryzom_installer.ini";

	// the config file we'll write
	m_configPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/ryzom_installer.ini";
}

CConfigFile::~CConfigFile()
{
	s_instance = NULL;
}

bool CConfigFile::load()
{
	// load default values
	if (!load(m_defaultConfigPath)) return false;

	// ignore return value, since we'll always have valid values
	load(m_configPath);

	return true;
}

bool CConfigFile::load(const QString &filename)
{
	if (!QFile::exists(filename)) return false;

	QSettings settings(filename, QSettings::IniFormat);

	int defaultVersion = m_version;
	int currentVersion = settings.value("version", 0).toInt();

	bool useDefaultValues = defaultVersion > currentVersion;

	// set default version from default config
	if (defaultVersion == -1) m_version = currentVersion;

	if (useDefaultValues)
	{
		// make a backup of custom ryzom_installer.ini
		QFile::copy(filename, filename + ".bak");
	}

	// custom choices, always keep them
	settings.beginGroup("common");
	m_language = settings.value("language", m_language).toString();
	m_srcDirectory = settings.value("source_directory").toString();
	m_installationDirectory = settings.value("installation_directory").toString();
	m_use64BitsClient = settings.value("use_64bits_client", true).toBool();
	m_shouldUninstallOldClient = settings.value("should_uninstall_old_client", true).toBool();
	settings.endGroup();

	if (!useDefaultValues)
	{
		settings.beginGroup("product");
		m_productName = settings.value("name").toString();
		m_productPublisher = settings.value("publisher").toString();
		m_productAboutUrl = settings.value("url_about").toString();
		m_productUpdateUrl = settings.value("url_update").toString();
		m_productHelpUrl = settings.value("url_help").toString();
		m_productComments = settings.value("comments").toString();
		settings.endGroup();

		settings.beginGroup("servers");
		int serversCount = settings.value("size").toInt();
		m_defaultServerIndex = settings.value("default").toInt();
		settings.endGroup();

		m_servers.resize(serversCount);

		for (int i = 0; i < serversCount; ++i)
		{
			CServer &server = m_servers[i];

			settings.beginGroup(QString("server_%1").arg(i));
			server.loadFromSettings(settings);
			settings.endGroup();
		}
	}

	// custom choices, always keep them
	settings.beginGroup("profiles");
	int profilesCounts = settings.value("size").toInt();
	m_defaultProfileIndex = settings.value("default").toInt();
	settings.endGroup();
	
	m_profiles.resize(profilesCounts);

	for(int i = 0; i < profilesCounts; ++i)
	{
		CProfile &profile = m_profiles[i];

		settings.beginGroup(QString("profile_%1").arg(i));
		profile.loadFromSettings(settings);
		settings.endGroup();
	}

	return !m_servers.isEmpty();
}

bool CConfigFile::save() const
{
	QSettings settings(m_configPath, QSettings::IniFormat);

	settings.setValue("version", m_version);

	settings.beginGroup("common");
	settings.setValue("language", m_language);
	settings.setValue("source_directory", m_srcDirectory);
	settings.setValue("installation_directory", m_installationDirectory);
	settings.setValue("use_64bits_client", m_use64BitsClient);
	settings.setValue("should_uninstall_old_client", m_shouldUninstallOldClient);
	settings.endGroup();

	settings.beginGroup("product");
	settings.setValue("name", m_productName);
	settings.setValue("publisher", m_productPublisher);
	settings.setValue("url_about", m_productAboutUrl);
	settings.setValue("url_update", m_productUpdateUrl);
	settings.setValue("url_help", m_productHelpUrl);
	settings.setValue("comments", m_productComments);
	settings.endGroup();

	settings.beginGroup("servers");
	settings.setValue("size", m_servers.size());
	settings.setValue("default", m_defaultServerIndex);
	settings.endGroup();

	for(int i = 0; i < m_servers.size(); ++i)
	{
		const CServer &server = m_servers[i];

		settings.beginGroup(QString("server_%1").arg(i));
		server.saveToSettings(settings);
		settings.endGroup();
	}

	settings.beginGroup("profiles");
	settings.setValue("size", m_profiles.size());
	settings.setValue("default", m_defaultProfileIndex);
	settings.endGroup();

	for(int i = 0; i < m_profiles.size(); ++i)
	{
		const CProfile &profile = m_profiles[i];

		settings.beginGroup(QString("profile_%1").arg(i));
		profile.saveToSettings(settings);
		settings.endGroup();
	}

	return true;
}

bool CConfigFile::remove()
{
	return QFile::remove(m_configPath);
}

CConfigFile* CConfigFile::getInstance()
{
	return s_instance;
}

int CConfigFile::getServersCount() const
{
	return m_servers.size();
}

const CServer& CConfigFile::getServer(int i) const
{
	if (i < 0) i = m_defaultServerIndex;

	if (i >= m_servers.size()) return NoServer;

	return m_servers.at(i);
}

const CServer& CConfigFile::getServer(const QString &id) const
{
	for(int i = 0; i < m_servers.size(); ++i)
	{
		if (m_servers[i].id == id) return m_servers[i];
	}

	// default server
	return getServer();
}

void CConfigFile::backupProfiles()
{
	m_backupProfiles = m_profiles;
}

int CConfigFile::getProfilesCount() const
{
	return m_profiles.size();
}

CProfile CConfigFile::getProfile(int i) const
{
	if (i < 0) i = m_defaultProfileIndex;

	if (i >= m_profiles.size()) return NoProfile;

	return m_profiles.at(i);
}

CProfile CConfigFile::getProfile(const QString &id) const
{
	for (int i = 0; i < m_profiles.size(); ++i)
	{
		if (m_profiles[i].id == id) return m_profiles[i];
	}

	// default profile
	return getProfile();
}

void CConfigFile::setProfile(int i, const CProfile &profile)
{
	m_profiles[i] = profile;
}

int CConfigFile::addProfile(const CProfile &profile)
{
	m_profiles.append(profile);

	return m_profiles.size()-1;
}

void CConfigFile::removeProfile(int i)
{
	m_profiles.removeAt(i);
}

void CConfigFile::removeProfile(const QString &id)
{
	for (int i = 0; i < m_profiles.size(); ++i)
	{
		if (m_profiles[i].id == id) removeProfile(i);
	}
}

QString CConfigFile::getDesktopDirectory() const
{
	return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

QString CConfigFile::getMenuDirectory() const
{
	return QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/" + QApplication::applicationName();
}

bool CConfigFile::has64bitsOS()
{
#ifdef Q_OS_WIN32
	// 64 bits only supported under Vista and up
	if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA) return false;
#endif

	return QSysInfo::currentCpuArchitecture() == "x86_64";
}

int CConfigFile::getDefaultServerIndex() const
{
	return m_defaultServerIndex;
}

void CConfigFile::setDefaultServerIndex(int index)
{
	m_defaultServerIndex = index;
}

int CConfigFile::getDefaultProfileIndex() const
{
	return m_defaultProfileIndex;
}

void CConfigFile::setDefaultProfileIndex(int index)
{
	m_defaultProfileIndex = index;
}

bool CConfigFile::isRyzomInstallerConfigured() const
{
	return m_profiles.size() > 0;
}

QString CConfigFile::getInstallationDirectory() const
{
	return m_installationDirectory;
}

void CConfigFile::setInstallationDirectory(const QString &directory)
{
	m_installationDirectory = directory;
}

QString CConfigFile::getSrcServerDirectory() const
{
	return m_srcDirectory;
}

void CConfigFile::setSrcServerDirectory(const QString &directory)
{
	m_srcDirectory = directory;
}

QString CConfigFile::getProfileDirectory() const
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString CConfigFile::getSrcProfileDirectory() const
{
	QString filename = "client.cfg";

	QStringList paths;

	// same path as client
	paths << getSrcServerDirectory();

	// profile path root
	paths << getProfileDirectory();

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
	// specific path under Linux
	paths << QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ryzom";
#endif

	// search config file in these locations
	foreach(const QString &path, paths)
	{
		if (QFile::exists(path + "/" + filename)) return path;
	}

	return "";
}

bool CConfigFile::use64BitsClient() const
{
	return m_use64BitsClient;
}

void CConfigFile::setUse64BitsClient(bool on)
{
	m_use64BitsClient = on;
}

bool CConfigFile::shouldUninstallOldClient() const
{
	return m_shouldUninstallOldClient;
}

void CConfigFile::setShouldUninstallOldClient(bool on)
{
	m_shouldUninstallOldClient = on;
}

bool CConfigFile::uninstallingOldClient() const
{
	return QFile::exists(getInstallationDirectory() + "/ryzom_installer_uninstalling_old_client");
}

void CConfigFile::setUninstallingOldClient(bool on) const
{
	QString filename = getInstallationDirectory() + "/ryzom_installer_uninstalling_old_client";

	if (on)
	{
		// writing a file to avoid asking several times when relaunching installer
		QFile file(filename);

		if (file.open(QFile::WriteOnly))
		{
			file.write("empty");
			file.close();
		}
	}
	else
	{
		// deleting the temporary file
		if (QFile::exists(filename)) QFile::remove(filename);
	}
}

QString CConfigFile::expandVariables(const QString &str) const
{
	QString res = str;

	res.replace("$TIMESTAMP", QString::number(QDateTime::currentDateTime().toTime_t()));
	res.replace("$LANG", m_language);
	res.replace("$ARCH", getClientArch());
	res.replace("$PRODUCT_NAME", m_productName);
	res.replace("$PRODUCT_PUBLISHER", m_productPublisher);
	res.replace("$PRODUCT_ABOUT_URL", m_productAboutUrl);
	res.replace("$PRODUCT_HELP_URL", m_productHelpUrl);
	res.replace("$PRODUCT_COMMENTS", m_productComments);

	return res;
}

QString CConfigFile::getClientArch() const
{
#if defined(Q_OS_WIN)
	return QString("win%1").arg(m_use64BitsClient ? 64:32);
#elif defined(Q_OS_MAC)
	// only 64 bits clients under OS X, because there not any 32 bits OS X version anymore
	return "osx";
#else
	return QString("linux%1").arg(m_use64BitsClient ? 64:32);
#endif
}

QString CConfigFile::getCurrentDirectory()
{
	return QDir::current().absolutePath();
}

QString CConfigFile::getParentDirectory()
{
	QDir current = QDir::current();
	current.cdUp();
	return current.absolutePath();
}

QString CConfigFile::getApplicationDirectory()
{
	return QApplication::applicationDirPath();
}

QString CConfigFile::getOldInstallationDirectory()
{
	// HKEY_CURRENT_USER/SOFTWARE/Nevrax/RyzomInstall/InstallId=1917716796 (string)
#if defined(Q_OS_WIN)
	// NSIS previous official installer
#ifdef Q_OS_WIN64
	// use WOW6432Node in 64 bits (64 bits OS and 64 bits Installer) because Ryzom old installer was in 32 bits
	QSettings settings("HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\Nevrax\\Ryzom", QSettings::NativeFormat);
#else
	QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Nevrax\\Ryzom", QSettings::NativeFormat);
#endif

	if (settings.contains("Ryzom Install Path"))
	{
		return QDir::fromNativeSeparators(settings.value("Ryzom Install Path").toString());
	}

	// check default directory if registry key not found
	return CConfigFile::has64bitsOS() ? "C:/Program Files (x86)/Ryzom":"C:/Program Files/Ryzom";
#elif defined(Q_OS_MAC)
	return "/Applications/Ryzom.app";
#else
	return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.ryzom";
#endif
}

QString CConfigFile::getOldInstallationLanguage()
{
#if defined(Q_OS_WIN)
	// NSIS previous official installer
#ifdef Q_OS_WIN64
	// use WOW6432Node in 64 bits (64 bits OS and 64 bits Installer) because Ryzom old installer was in 32 bits
	QSettings settings("HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\Nevrax\\Ryzom", QSettings::NativeFormat);
#else
	QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Nevrax\\Ryzom", QSettings::NativeFormat);
#endif

	QString key = "Language";

	if (settings.contains(key))
	{
		QString languageCode = settings.value(key).toString();

		// 1036 = French (France), 1033 = English (USA), 1031 = German
		if (languageCode == "1036") return "fr";
		if (languageCode == "1031") return "de";
		if (languageCode == "1033") return "en";
	}
#endif

	return "";
}

QString CConfigFile::getNewInstallationLanguage()
{
#if defined(Q_OS_WIN)
	// NSIS new official installer
	QSettings settings("HKEY_CURRENT_USER\\Software\\Winch Gate\\Ryzom", QSettings::NativeFormat);

	QString key = "Language";

	if (settings.contains(key))
	{
		QString languageCode = settings.value(key).toString();

		// 1036 = French (France), 1033 = English (USA), 1031 = German
		if (languageCode == "1036") return "fr";
		if (languageCode == "1031") return "de";
		if (languageCode == "1033") return "en";
	}
#endif

	return "";
}

QString CConfigFile::getNewInstallationDirectory()
{
#if defined(Q_OS_WIN)
	// NSIS new official installer
	QSettings settings("HKEY_CURRENT_USER\\Software\\Winch Gate\\Ryzom", QSettings::NativeFormat);

	QString key = "Ryzom Install Path";

	if (settings.contains(key))
	{
		return QDir::fromNativeSeparators(settings.value(key).toString());
	}
#endif

	// default location
	return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

bool CConfigFile::isRyzomInstalledIn(const QString &directory) const
{
	// check client and data
	return isRyzomClientInstalledIn(directory) && areRyzomDataInstalledIn(directory);
}

bool CConfigFile::areRyzomDataInstalledIn(const QString &directory) const
{
	if (directory.isEmpty()) return false;

	QDir dir(directory);

	// directory doesn't exist
	if (!dir.exists()) return false;

	if (!dir.cd("data") || !dir.exists()) return false;

	// at least 200 BNP in data directory
	if (dir.entryList(QStringList() << "*.bnp", QDir::Files).size() < 200) return false;

	//  ryzom.ttf or fonts.bnp is required
	if (!dir.exists("fonts/ryzom.ttf") && !dir.exists("fonts.bnp")) return false;

	//  gamedev.bnp is required
	if (!dir.exists("gamedev.bnp")) return false;

	//  interfaces.bnp is required
	if (!dir.exists("interfaces.bnp")) return false;

	// TODO: more checks

	return true;
}

bool CConfigFile::isRyzomClientInstalledIn(const QString &directory) const
{
	if (directory.isEmpty()) return false;

	QDir dir(directory);

	// directory doesn't exist
	if (!dir.exists()) return false;

	// current server
	CServer server = getServer();

	QString clientFilename = server.clientFilename;

	// check if new client is defined and exists
	if (!clientFilename.isEmpty() && !dir.exists(clientFilename))
	{
		clientFilename = server.clientFilenameOld;

		// check if old client is defined and exists
		if (!dir.exists(clientFilename)) return false;

		// client 2.1-
	}
	else
	{
		// client 3.0+

		// client_default.cfg doesn't exist
		if (!dir.exists("client_default.cfg")) return false;

		// TODO: more checks
	}

	return true;
}

bool CConfigFile::foundTemporaryFiles(const QString &directory) const
{
	if (directory.isEmpty()) return false;

	QDir dir(directory);

	// directory doesn't exist
	if (!dir.exists()) return false;

	if (!dir.cd("data")) return false;

	QStringList filter;
	filter << "*.string_cache";

	// certificate should be in gamedev.bnp now
	filter << "*.pem";

	// only .ref files should be there
	filter << "exedll*.bnp";

	if (dir.exists("packedsheets.bnp"))
	{
		filter << "*.packed_sheets";
		filter << "*.packed";
	}

	// temporary files
	if (!dir.entryList(filter, QDir::Files).isEmpty()) return true;

	// temporary directories
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	// fonts directory is not needed anymore if fonts.bnp exists
	if (!dir.exists("fonts.bnp")) dirs.removeAll("fonts");

	return !dirs.isEmpty();
}

bool CConfigFile::shouldCreateDesktopShortcut() const
{
	const CProfile &profile = getProfile();

	if (!profile.desktopShortcut) return false;

	QString shortcut = profile.getClientDesktopShortcutFullPath();

	return !shortcut.isEmpty() && !NLMISC::CFile::isExists(qToUtf8(appendLinkExtension(shortcut)));
}

bool CConfigFile::shouldCreateMenuShortcut() const
{
	const CProfile &profile = getProfile();

	if (!profile.menuShortcut) return false;

	QString shortcut = profile.getClientMenuShortcutFullPath();

	return !shortcut.isEmpty() && !NLMISC::CFile::isExists(qToUtf8(appendLinkExtension(shortcut)));
}

bool CConfigFile::shouldCopyInstaller() const
{
	const CProfile &p = getProfile();
	const CServer &s = getServer(p.server);

	QString installerDst = getInstallationDirectory() + "/" + s.installerFilename;

	// if installer not found in installation directory, extract it from BNP
	if (!QFile::exists(installerDst)) return true;

	QString installedVersion = getVersionFromExecutable(installerDst);
	QString newVersion = QApplication::applicationVersion();

	QVersionNumber installedVer = QVersionNumber::fromString(installedVersion);
	QVersionNumber newVer = QVersionNumber::fromString(newVersion);

	// if version is greater, copy it
	return newVer > installedVer;
}

QString CConfigFile::getInstallerCurrentFilePath() const
{
	// installer is always run from TEMP under Windows
	return QApplication::applicationFilePath();
}

QString CConfigFile::getInstallerCurrentDirPath() const
{
	// installer is always run from TEMP under Windows
	return QApplication::applicationDirPath();
}

QString CConfigFile::getInstallerOriginalFilePath() const
{
	return getInstallerOriginalDirPath() + "/" + QFileInfo(QApplication::applicationFilePath()).fileName();
}

QString CConfigFile::getInstallerOriginalDirPath() const
{
	return m_installationDirectory;
}

QString CConfigFile::getInstallerMenuLinkFullPath() const
{
	// don't put extension
	return QString("%1/%2/%2 Installer").arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(QApplication::applicationName());
}

QStringList CConfigFile::getInstallerRequiredFiles() const
{
	// list of all files required by installer (and its executable too)
	QStringList files;

#ifdef Q_OS_WIN

	// VC++ runtimes
#if _MSC_VER == 1900
	// VC++ 2015
	files << "msvcp140.dll";
	files << "msvcr140.dll";
#elif _MSC_VER == 1800
	// VC++ 2013
	files << "msvcp120.dll";
	files << "msvcr120.dll";
#elif _MSC_VER == 1700
	// VC++ 2012
	files << "msvcp110.dll";
	files << "msvcr110.dll";
#elif _MSC_VER == 1600
	// VC++ 2010
	files << "msvcp100.dll";
	files << "msvcr100.dll";
#elif _MSC_VER == 1500
	// VC++ 2008
	files << "msvcp90.dll";
	files << "msvcr90.dll";
#else
	// unsupported compiler
#endif

#elif defined(Q_OS_MAC)
	// TODO: for OS X
#else
	// icon under Linux
	files << "ryzom_installer.png";
#endif

	// include current executable
	files << QFileInfo(QApplication::applicationFilePath()).fileName();

	return files;
}

QString CConfigFile::getSrcServerClientBNPFullPath() const
{
	return QString("%1/unpack/exedll_%2.bnp").arg(getSrcServerDirectory()).arg(getClientArch());
}

OperationStep CConfigFile::getInstallNextStep() const
{
	// get last used profile
	const CProfile &profile = getProfile();

	// get server used by it or default server
	CServer server = getServer(profile.server);

	// no or wrong profile
	if (server.id.isEmpty())
	{
		// get last used server
		server = getServer();
	}

	// no or wrong server
	if (server.id.isEmpty())
	{
		// get first server
		server = getServer(0);
	}

	// no server defined, shouldn't happen
	if (server.id.isEmpty())
	{
		return DisplayNoServerError;
	}

	// only show wizard if installation directory undefined
	if (getInstallationDirectory().isEmpty())
	{
		// if launched from current directory, it means we just patched files
		QString currentDirectory = getCurrentDirectory();

		if (!isRyzomInstalledIn(currentDirectory))
		{
			// Ryzom is in the same directory as Ryzom Installer
			currentDirectory = getApplicationDirectory();

			if (!isRyzomInstalledIn(currentDirectory))
			{
				currentDirectory.clear();
			}
		}

		// install or migrate depending if Ryzom was found in current directory
		return currentDirectory.isEmpty() ? ShowInstallWizard:ShowMigrateWizard;
	}

	QString serverDirectory = server.getDirectory();

	if (getSrcServerDirectory().isEmpty())
	{
		// user decided to download files

		// downloaded files are kept in server directory
		QString dataFile = getInstallationDirectory() + "/" + server.dataDownloadFilename;
		QString clientFile = getInstallationDirectory() + "/" + expandVariables(server.clientDownloadFilename);

		// data are not copied
		if (!areRyzomDataInstalledIn(serverDirectory))
		{
			// when file is not finished, it has .part extension
			if (!QFile::exists(dataFile))
			{
				return DownloadData;
			}

			return ExtractDownloadedData;
		}

		if (!isRyzomClientInstalledIn(serverDirectory))
		{
			// when file is not finished, it has .part extension
			if (!QFile::exists(clientFile))
			{
				return DownloadClient;
			}

			return ExtractDownloadedClient;
		}
	}
	else
	{
		// user decided to copy files

		// data are not copied
		if (!areRyzomDataInstalledIn(serverDirectory))
		{
			// selected directory contains Ryzom files (shouldn't fail)
			if (areRyzomDataInstalledIn(getSrcServerDirectory()))
			{
				return CopyDataFiles;
			}
			else
			{
				return ShowInstallWizard;
			}
		}

		// client is not extracted from BNP
		if (!isRyzomClientInstalledIn(serverDirectory))
		{
			if (foundTemporaryFiles(serverDirectory))
			{
				return CleanFiles;
			}

			if (QFile::exists(getSrcServerClientBNPFullPath()))
			{
				return ExtractBnpClient;
			}

			QString clientFile = getInstallationDirectory() + "/" + expandVariables(server.clientDownloadFilename);

			// when file is not finished, it has .part extension
			if (!QFile::exists(clientFile))
			{
				return DownloadClient;
			}

			return ExtractDownloadedClient;
		}
	}

	if (shouldCopyInstaller()) return CopyInstaller;

	// no default profile
	if (profile.id.isEmpty())
	{
		return CreateProfile;
	}

	QString clientCfg = QString("%1/%2/client.cfg").arg(getProfileDirectory()).arg(profile.id);

	// migration profile
	if (!getSrcServerDirectory().isEmpty() && QFile::exists(getSrcProfileDirectory() + "/client.cfg") && !QFile::exists(clientCfg))
	{
		return CopyProfileFiles;
	}

	if (shouldCreateDesktopShortcut() || shouldCreateMenuShortcut())
	{
		return CreateProfileShortcuts;
	}

#ifdef Q_OS_WIN
	// check that Add/Remove entry is created under Windows
	QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);

	if (!settings.contains("InstallLocation")) return CreateAddRemoveEntry;
#endif

	if (m_shouldUninstallOldClient && !getSrcServerDirectory().isEmpty())
	{
		// if old client must be uninstalled
		if (!uninstallingOldClient() && QFile::exists(getSrcServerDirectory() + "/Uninstall.exe"))
		{
			return UninstallOldClient;
		}

		// if old client has been uninstalled
		if (uninstallingOldClient() && !QFile::exists(getSrcServerDirectory() + "/Uninstall.exe"))
		{
			setUninstallingOldClient(false);
		}
	}

	return Done;
}

QString CConfigFile::getProductName() const
{
	return m_productName;
}

QString CConfigFile::getProductPublisher() const
{
	return m_productPublisher;
}

QString CConfigFile::getProductAboutUrl() const
{
	return expandVariables(m_productAboutUrl);
}

QString CConfigFile::getProductUpdateUrl() const
{
	return expandVariables(m_productUpdateUrl);
}

QString CConfigFile::getProductHelpUrl() const
{
	return expandVariables(m_productHelpUrl);
}

QString CConfigFile::getProductComments() const
{
	return m_productComments;
}
