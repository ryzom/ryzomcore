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
#include "operation.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

QString stepToString(OperationStep step)
{
	static QStringList s_operations;
	
	if (s_operations.isEmpty())
	{
		s_operations << "DisplayNoServerError"
			<< "ShowInstallWizard"
			<< "ShowMigrateWizard"
			<< "DownloadData"
			<< "ExtractDownloadedData"
			<< "DownloadClient"
			<< "ExtractDownloadedClient"
			<< "CopyDataFiles"
			<< "CopyProfileFiles"
			<< "CleanFiles"
			<< "ExtractBnpClient"
			<< "CopyInstaller"
			<< "LaunchInstalledInstaller"
			<< "UninstallOldClient"
			<< "CreateProfile"
			<< "CreateProfileShortcuts"
			<< "CreateAddRemoveEntry"
			<< "Done";
	}
	
	QString res;
	
	if (step >= 0 && step < s_operations.size()) res += s_operations[step];

	return QString("%1 (%2)").arg(res).arg(step);
}
