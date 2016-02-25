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

struct CServer
{
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
	QString comments;
};

extern const CServer NoServer;

struct CConfiguration
{
	CConfiguration()
	{
		id = -1;
	}

	int id;
	QString account;
	QString name;
	QString server;
	QString executable;
	QString parameters;
	QString comments;
};

extern const CConfiguration NoConfiguration;

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
	enum InstallationStep
	{
		DisplayNoServerError,
		ShowWizard,
		DownloadData,
		ExtractDownloadedData,
		DownloadClient,
		ExtractDownloadedClient,
		CopyServerFiles,
		CopyConfigurationFiles,
		ExtractBnpClient,
		CreateConfiguration,
		CreateShortcuts,
		Done
	};

	CConfigFile(QObject *parent = NULL);
	virtual ~CConfigFile();

	bool load();
	bool load(const QString &filename);
	bool save() const;

	static CConfigFile* getInstance();

	int getServersCount() const;
	const CServer& getServer(int i = -1) const;
	const CServer& getServer(const QString &id) const;

	int getConfigurationsCount() const;
	CConfiguration getConfiguration(int i = -1) const;
	void setConfiguration(int i, const CConfiguration &configuration);
	int addConfiguration(const CConfiguration &configuration);
	void removeConfiguration(int i);

	int getDefaultServer() const;
	int getDefaultConfiguration() const;

	bool isRyzomInstallerConfigured() const;

	QString getInstallationDirectory() const;
	void setInstallationDirectory(const QString &directory);

	QString getSrcServerDirectory() const;
	void setSrcServerDirectory(const QString &directory);

	QString getConfigurationDirectory() const;
	QString getSrcConfigurationDirectory() const;

	static bool has64bitsOS();

	// default directories
	static QString getCurrentDirectory();
	static QString getParentDirectory();
	static QString getApplicationDirectory();
	static QString getOldInstallationDirectory();
	static QString getNewInstallationDirectory();

	bool isRyzomInstalledIn(const QString &directory) const;
	bool areRyzomDataInstalledIn(const QString &directory) const;
	bool isRyzomClientInstalledIn(const QString &directory) const;

	// installation choices
	bool use64BitsClient() const;
	void setUse64BitsClient(bool on);

	QString expandVariables(const QString &str);

	QString getClientArch() const;

	QString getClientFullPath() const;

	QString getSrcServerClientBNPFullPath() const;

	InstallationStep getNextStep() const;

private:
	int m_defaultServer;
	int m_defaultConfiguration;

	QVector<CServer> m_servers;
	QVector<CConfiguration> m_configurations;

	QString m_installationDirectory;
	QString m_srcDirectory;
	bool m_use64BitsClient;
	QString m_language;

	QString m_defaultConfigPath;
	QString m_configPath;

	static CConfigFile *s_instance;
};

#endif
