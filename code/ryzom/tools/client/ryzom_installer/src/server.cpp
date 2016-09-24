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
#include "server.h"
#include "configfile.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

const CServer NoServer;

void CServer::loadFromSettings(const QSettings &settings)
{
	id = settings.value("id").toString();
	name = settings.value("name").toString();
	displayUrl = settings.value("display_url").toString();
	filesListUrl = settings.value("files_list_url").toString();
	dataDownloadUrl = settings.value("data_download_url").toString();
	dataDownloadFilename = settings.value("data_download_filename").toString();
	dataCompressedSize = settings.value("data_compressed_size").toULongLong();
	dataUncompressedSize = settings.value("data_uncompressed_size").toULongLong();
	clientDownloadUrl = settings.value("client_download_url").toString();
	clientDownloadFilename = settings.value("client_download_filename").toString();
#if defined(Q_OS_WIN)
	clientFilename = settings.value("client_filename_windows").toString();
	clientFilenameOld = settings.value("client_filename_old_windows").toString();
	configurationFilename = settings.value("configuration_filename_windows").toString();
	installerFilename = settings.value("installer_filename_windows").toString();
#elif defined(Q_OS_MAC)
	clientFilename = settings.value("client_filename_osx").toString();
	clientFilenameOld = settings.value("client_filename_old_osx").toString();
	configurationFilename = settings.value("configuration_filename_osx").toString();
	installerFilename = settings.value("installer_filename_osx").toString();
#else
	clientFilename = settings.value("client_filename_linux").toString();
	clientFilenameOld = settings.value("client_filename_old_linux").toString();
	configurationFilename = settings.value("configuration_filename_linux").toString();
	installerFilename = settings.value("installer_filename_linux").toString();
#endif
	comments = settings.value("comments").toString();
}

void CServer::saveToSettings(QSettings &settings) const
{
	settings.setValue("id", id);
	settings.setValue("name", name);
	settings.setValue("display_url", displayUrl);
	settings.setValue("files_list_url", filesListUrl);
	settings.setValue("data_download_url", dataDownloadUrl);
	settings.setValue("data_download_filename", dataDownloadFilename);
	settings.setValue("data_compressed_size", dataCompressedSize);
	settings.setValue("data_uncompressed_size", dataUncompressedSize);
	settings.setValue("client_download_url", clientDownloadUrl);
	settings.setValue("client_download_filename", clientDownloadFilename);
#if defined(Q_OS_WIN)
	settings.setValue("client_filename_windows", clientFilename);
	settings.setValue("client_filename_old_windows", clientFilenameOld);
	settings.setValue("configuration_filename_windows", configurationFilename);
	settings.setValue("installer_filename_windows", installerFilename);
#elif defined(Q_OS_MAC)
	settings.setValue("client_filename_osx", clientFilename);
	settings.setValue("client_filename_old_osx", clientFilenameOld);
	settings.setValue("configuration_filename_osx", configurationFilename);
	settings.setValue("installer_filename_osx", installerFilename);
#else
	settings.setValue("client_filename_linux", clientFilename);
	settings.setValue("client_filename_old_linux", clientFilenameOld);
	settings.setValue("configuration_filename_linux", configurationFilename);
	settings.setValue("installer_filename_linux", installerFilename);
#endif
	settings.setValue("comments", comments);
}

QString CServer::getDirectory() const
{
	return CConfigFile::getInstance()->getInstallationDirectory() + "/" + id;
}

QString CServer::getClientFullPath() const
{
	if (clientFilename.isEmpty()) return "";

	return getDirectory() + "/" + clientFilename;
}

QString CServer::getConfigurationFullPath() const
{
	if (configurationFilename.isEmpty()) return "";

	return getDirectory() + "/" + configurationFilename;
}

QString CServer::getDefaultClientConfigFullPath() const
{
	return getDirectory() + "/client_default.cfg";
}
