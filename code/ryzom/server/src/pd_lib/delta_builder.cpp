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

#include "delta_builder.h"

#include "pd_utils.h"
#include "db_delta_file.h"

using namespace std;
using namespace NLMISC;

/*
 * Constructor
 */
CDeltaBuilder::CDeltaBuilder()
{
}


/*
 * Build delta
 */
bool	CDeltaBuilder::build(const std::string& outputPath,
							 const std::string& hoursUpdatePath,
							 const std::string& minutesUpdatePath,
							 const std::string& secondsUpdatePath,
							 const std::string& mintimestamp,
							 const std::string& maxtimestamp,
							 TDelta deltaType)
{
	vector<string>	files;

	if (!internalBuild(	outputPath,
						hoursUpdatePath,
						minutesUpdatePath,
						secondsUpdatePath,
						mintimestamp,
						maxtimestamp,
						deltaType,
						files) )
	{
		// if failed, delete generated files
		uint	i;
		for (i=0; i<files.size(); ++i)
			if (!CFile::deleteFile(files[i]))
				nlwarning("CDeltaBuilder::build(): after failure, failed to delete generated file '%s'", files[i].c_str());

		return false;
	}

	return true;
}

/*
 * Build delta
 */
bool	CDeltaBuilder::internalBuild(	const std::string& outputPath,
										const std::string& hoursUpdatePath,
										const std::string& minutesUpdatePath,
										const std::string& secondsUpdatePath,
										const std::string& mintimestamp,
										const std::string& maxtimestamp,
										TDelta deltaType,
										std::vector<std::string>& generatedFiles)
{
	string	output = NLMISC::CPath::standardizePath(outputPath);
	string	hourspath = NLMISC::CPath::standardizePath(hoursUpdatePath);
	string	minutespath = NLMISC::CPath::standardizePath(minutesUpdatePath);
	string	secondspath = NLMISC::CPath::standardizePath(secondsUpdatePath);

	vector<string>	hours;
	vector<string>	minutes;
	vector<string>	seconds;

	// get delta in all directories
	NLMISC::CPath::getPathContent(hourspath, false, false, true, hours);
	NLMISC::CPath::getPathContent(minutespath, false, false, true, minutes);
	NLMISC::CPath::getPathContent(secondspath, false, false, true, seconds);

	vector<vector<string> >		tableUpdates;

	CTimestamp	minstamp(mintimestamp);
	CTimestamp	maxstamp(maxtimestamp);

	updateFilesList(tableUpdates, hours, minstamp, maxstamp);
	updateFilesList(tableUpdates, minutes, minstamp, maxstamp);
	updateFilesList(tableUpdates, seconds, minstamp, maxstamp);

	uint	i;
	for (i=0; i<tableUpdates.size(); ++i)
	{
		if (tableUpdates[i].empty())
			continue;

		string	deltaFilename;
		if (deltaType == Hour)
			deltaFilename = output+CDBDeltaFile::getHourDeltaFileName(i, maxstamp);
		else if (deltaType == Minute)
			deltaFilename = output+CDBDeltaFile::getMinuteDeltaFileName(i, maxstamp);
		else
			deltaFilename = output+CDBDeltaFile::getDeltaFileName(i, maxstamp);

		// add filename to list of generated files
		generatedFiles.push_back(deltaFilename);

		if (!generateDeltaFile(deltaFilename, tableUpdates[i], minstamp, maxstamp))
		{
			nlwarning("CDeltaBuilder:build(): failed to build delta file '%s'", deltaFilename.c_str());
			return false;
		}
	}

	return true;
}


/*
 * Build update files list
 */
bool	CDeltaBuilder::updateFilesList(std::vector<std::vector<std::string> >& fileLists,
									   std::vector<std::string>& files,
									   const CTimestamp& minstamp,
									   const CTimestamp& maxstamp)
{
	sort(files.begin(), files.end());

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		nlinfo("updateFilesList: found file '%s'", files[i].c_str());

		uint32		tableId;
		CTimestamp	stamp;

		if (!CDBDeltaFile::isDeltaFileName(files[i], tableId, stamp))
			continue;

		// check delta file in stamp interval
		if (stamp < minstamp || stamp > maxstamp)
			continue;

		if (fileLists.size() <= tableId)
			fileLists.resize(tableId+1);

		if (fileLists[tableId].empty())
		{
			fileLists[tableId].push_back(files[i]);
			continue;
		}

		string	lastupdate = CFile::getFilenameWithoutExtension(fileLists[tableId].back());
		string	thisupdate = CFile::getFilenameWithoutExtension(files[i]);

		// is this update newer than last update?
		if (thisupdate > lastupdate)
			fileLists[tableId].push_back(files[i]);
	}

	return true;
}


/*
 * Generate Delta update file
 */
bool	CDeltaBuilder::generateDeltaFile(const std::string& outputPath,
										 const std::vector<std::string>& updateFiles,
										 const CTimestamp& starttime,
										 const CTimestamp& endtime)
{
	CDBDeltaFile		output;
	output.setup(outputPath, 0, starttime, endtime);

	uint	i;
	for (i=0; i<updateFiles.size(); ++i)
	{
		CDBDeltaFile	delta;
		const std::string&	file = updateFiles[i];

		// load delta file, so we can get row size, if needed
		delta.setup(file, 0, CTimestamp(), CTimestamp());

		if (!output.concat(delta, starttime, endtime))
		{
			nlwarning("CDeltaBuilder::generateDeltaFile(): failed to concat file '%s' to delta '%s'", updateFiles[i].c_str(), outputPath.c_str());
			return false;
		}
		else if (RY_PDS::PDVerbose)
		{
			PDS_LOG_DEBUG(1)("CDeltaBuilder::generateDeltaFile(): concatted '%s' into '%s'", file.c_str(), outputPath.c_str());
		}
	}

	return true;
}

/*
 * Remove older files in update path
 */
bool	CDeltaBuilder::removeOlderDeltaInPath(const std::string& keeptimestamp,
											  const std::string& path)
{
	vector<string>	files;
	NLMISC::CPath::getPathContent(path, false, false, true, files);

	CTimestamp	keepstamp(keeptimestamp);

	uint	i, n=0;
	for (i=0; i<files.size(); ++i)
	{
		uint32		tableId;
		CTimestamp	stamp;

		if (!CDBDeltaFile::isDeltaFileName(files[i], tableId, stamp) || keepstamp <= stamp)
			continue;

		if (!CFile::deleteFile(files[i]))
		{
			nlwarning("CDeltaBuilder::removeOlderDeltaInPath(): failed to delete older file '%s'", files[i].c_str());
		}
		else
		{
			++n;
		}
	}
	PDS_LOG_DEBUG(1)("CDeltaBuilder::removeOlderDeltaInPath(): deleted %d files in directory '%s'", n, path.c_str());
	return true;
}
