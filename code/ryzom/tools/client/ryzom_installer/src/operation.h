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

#ifndef OPERATION_H
#define OPERATION_H

class IOperationProgressListener
{
public:
	virtual ~IOperationProgressListener() {}

	virtual void operationPrepare() =0;
	virtual void operationInit(qint64 current, qint64 total) =0;
	virtual void operationStart() =0;
	virtual void operationStop() =0;
	virtual void operationProgress(qint64 current, const QString &filename) =0;
	virtual void operationSuccess(qint64 total) =0;
	virtual void operationFail(const QString &error) =0;

	virtual bool operationShouldStop() =0;
};

struct SUninstallComponents
{
	SUninstallComponents()
	{
		installer = true;
	}

	QStringList servers;
	QStringList profiles;

	bool installer;
};

enum OperationStep
{
	DisplayNoServerError,
	ShowInstallWizard,
	ShowMigrateWizard,
	DownloadData,
	ExtractDownloadedData,
	DownloadClient,
	ExtractDownloadedClient,
	CopyDataFiles,
	CopyProfileFiles,
	CleanFiles,
	ExtractBnpClient,
	CopyInstaller,
	UninstallOldClient,
	CreateProfile,
	CreateShortcuts,
	CreateAddRemoveEntry,
	Done
};

enum OperationType
{
	OperationNone,
	OperationMigrate,
	OperationUpdateProfiles,
	OperationInstall,
	OperationUninstall
};

#endif
