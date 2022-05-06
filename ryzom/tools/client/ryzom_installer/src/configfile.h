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

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "operation.h"
#include "server.h"
#include "profile.h"

/**
 * Config file management and other stuff related to location of files/directories.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CConfigFile : public QObject
{
	Q_OBJECT

public:
	CConfigFile(QObject *parent = NULL);
	virtual ~CConfigFile();

	bool load();
	bool load(const QString &filename);
	bool save() const;
	bool remove();

	static CConfigFile* getInstance();

	CServers getServers() const { return m_servers; }
	void setServers(const CServers &servers) { m_servers = servers; }

	int getServersCount() const;
	const CServer& getServer(int i = -1) const;
	const CServer& getServer(const QString &id) const;

	CProfiles getProfiles() const { return m_profiles; }
	void setProfiles(const CProfiles &profiles) { m_profiles = profiles; }

	CProfiles getBackupProfiles() const { return m_backupProfiles; }
	CProfile getBackupProfile(const QString &id) const;
	void backupProfiles();

	QString getLanguage() const { return m_language; }

	bool getInstallerCopied() const { return m_installerCopied; }
	void setInstallerCopied(bool copied) { m_installerCopied = copied; }

	int getProfilesCount() const;
	CProfile getProfile(int i = -1) const;
	CProfile getProfile(const QString &id) const;
	void setProfile(int i, const CProfile &profile);
	int addProfile(const CProfile &profile);
	void removeProfile(int i);
	void removeProfile(const QString &id);

	int getDefaultServerIndex() const;
	void setDefaultServerIndex(int index);

	int getDefaultProfileIndex() const;
	void setDefaultProfileIndex(int index);

	bool isRyzomInstallerConfigured() const;

	QString getInstallationDirectory() const;
	void setInstallationDirectory(const QString &directory);

	QString getSrcServerDirectory() const;
	void setSrcServerDirectory(const QString &directory);

	QString getProfileDirectory() const;
	QString getSrcProfileDirectory() const;

	QString getDesktopDirectory() const;
	QString getMenuDirectory() const;

	static bool has64bitsOS();

	// default directories
	static QString getCurrentDirectory();
	static QString getParentDirectory();
	static QString getNewInstallationDirectory();
	static QString getNewInstallationLanguage();

	// status of installation
	bool isRyzomInstalledIn(const QString &directory) const;
	bool areRyzomDataInstalledIn(const QString &directory) const;
	bool isRyzomClientInstalledIn(const QString &directory) const;

	bool foundTemporaryFiles(const QString &directory) const;
	bool shouldCreateDesktopShortcut() const;
	bool shouldCreateMenuShortcut() const;
	int compareInstallersVersion() const;

	// installation choices
	bool use64BitsClient() const;
	void setUse64BitsClient(bool on);

	bool shouldUninstallOldClient() const;
	void setShouldUninstallOldClient(bool on);

	bool ignoreFreeDiskSpaceChecks() const;
	void setIgnoreFreeDiskSpaceChecks(bool on);

	bool uninstallingOldClient() const;
	void setUninstallingOldClient(bool on) const;

	QString getInstallerFilename() const { return m_installerFilename; }

	QString expandVariables(const QString &str) const;

	QString getClientArch() const;

	QString getInstallerCurrentFilePath() const;
	QString getInstallerCurrentDirPath() const;
	QString getInstallerInstalledFilePath() const;
	QString getInstallerInstalledDirPath() const;

	QString getInstallerMenuShortcutFullPath() const;
	QString getInstallerDesktopShortcutFullPath() const;

	QStringList getInstallerRequiredFiles() const;

	QString getSrcServerClientBNPFullPath() const;

	OperationStep getInstallNextStep() const;

	// product details
	QString getProductName() const;
	QString getProductPublisher() const;
	QString getProductAboutUrl() const;
	QString getProductUpdateUrl() const;
	QString getProductHelpUrl() const;
	QString getProductComments() const;

private:
	int m_version;
	int m_defaultServerIndex;
	int m_defaultProfileIndex;
	bool m_installerCopied;

	CServers m_servers;
	CProfiles m_profiles;
	CProfiles m_backupProfiles;

	QString m_installationDirectory;
	QString m_srcDirectory;
	bool m_use64BitsClient;
	bool m_shouldUninstallOldClient;
	bool m_ignoreFreeDiskSpaceChecks;
	QString m_installerFilename;
	QString m_language;

	QString m_defaultConfigPath;
	QString m_configPath;

	// product
	QString m_productName;
	QString m_productPublisher;
	QString m_productAboutUrl;
	QString m_productUpdateUrl;
	QString m_productHelpUrl;
	QString m_productComments;

	static CConfigFile *s_instance;
};

#endif
