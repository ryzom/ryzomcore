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

class CServer
{
public:
	CServer()
	{
		dataCompressedSize = 0;
		dataUncompressedSize = 0;
	}

	QString id;
	QString name;
	QString displayUrl;
	QString dataDownloadUrl;
	QString dataDownloadFilename;
	qint64 dataCompressedSize;
	qint64 dataUncompressedSize;
	QString clientDownloadUrl;
	QString clientDownloadFilename;
	QString clientFilename;
	QString clientFilenameOld;
	QString configurationFilename;
	QString installerFilename;
	QString comments;

	// helpers
	QString getDirectory() const;
};

extern const CServer NoServer;

typedef QVector<CServer> CServers;

class CProfile
{
public:
	CProfile()
	{
		desktopShortcut = false;
		menuShortcut = false;
	}

	QString id;
	QString name;
	QString server;
	QString executable;
	QString arguments;
	QString comments;
	bool desktopShortcut;
	bool menuShortcut;

	// helpers
	QString getDirectory() const;
};

extern const CProfile NoProfile;

typedef QVector<CProfile> CProfiles;

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

	static CConfigFile* getInstance();

	CServers getServers() const { return m_servers; }
	void setServers(const CServers &servers) { m_servers = servers; }

	int getServersCount() const;
	const CServer& getServer(int i = -1) const;
	const CServer& getServer(const QString &id) const;

	CProfiles getProfiles() const { return m_profiles; }
	void setProfiles(const CProfiles &profiles) { m_profiles = profiles; }

	CProfiles getBackupProfiles() const { return m_backupProfiles; }
	void backupProfiles();

	int getProfilesCount() const;
	CProfile getProfile(int i = -1) const;
	CProfile getProfile(const QString &id) const;
	void setProfile(int i, const CProfile &profile);
	int addProfile(const CProfile &profile);
	void removeProfile(int i);

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

	static bool has64bitsOS();

	// default directories
	static QString getCurrentDirectory();
	static QString getParentDirectory();
	static QString getApplicationDirectory();
	static QString getOldInstallationDirectory();
	static QString getNewInstallationDirectory();
	static QString getOldInstallationLanguage();
	static QString getNewInstallationLanguage();

	// status of installation
	bool isRyzomInstalledIn(const QString &directory) const;
	bool areRyzomDataInstalledIn(const QString &directory) const;
	bool isRyzomClientInstalledIn(const QString &directory) const;

	bool foundTemporaryFiles(const QString &directory) const;
	bool shouldCreateDesktopShortcut() const;

	// installation choices
	bool use64BitsClient() const;
	void setUse64BitsClient(bool on);

	bool shouldUninstallOldClient() const;
	void setShouldUninstallOldClient(bool on);

	QString expandVariables(const QString &str) const;

	QString getClientArch() const;

	QString getProfileClientFullPath(int profileIndex = -1) const;
	QString getServerClientFullPath(const QString &serverId = "") const;
	QString getServerConfigurationFullPath(const QString &serverId = "") const;

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
	int m_defaultServerIndex;
	int m_defaultProfileIndex;

	CServers m_servers;
	CProfiles m_profiles;
	CProfiles m_backupProfiles;

	QString m_installationDirectory;
	QString m_srcDirectory;
	bool m_use64BitsClient;
	bool m_shouldUninstallOldClient;
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
