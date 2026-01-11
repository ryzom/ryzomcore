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

#include "reference_builder.h"
#include "pds_table_buffer.h"
//#include "pd_string_manager.h"

#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>

using namespace std;
using namespace NLMISC;


/*
 * Constructor
 */
CReferenceBuilder::CReferenceBuilder()
{
}



/*
 * Build a new reference from a older reference
 * Apply delta changes so new reference is clean
 */
bool	CReferenceBuilder::build(CRefIndex& previous, CRefIndex& next)
{
	return build(previous.getRootPath(),
				 previous.getPath(),
				 next.getPath(),
				 previous.getRootPath()+"hours",
				 previous.getRootPath()+"minutes",
				 previous.getRootPath()+"seconds",
				 previous.getRootPath()+"logs",
				 previous.Timestamp.toString(), 
				 next.Timestamp.toString());
}

/*
 * Build a new reference from a older reference
 * Apply delta changes so new reference is clean
 */
bool	CReferenceBuilder::build(	const std::string& rootRefPath,
									const std::string& previousReferencePath,
									const std::string& nextReferencePath,
									const std::string& hoursUpdatePath,
									const std::string& minutesUpdatePath,
									const std::string& secondsUpdatePath,
									const std::string& logPath,
									const std::string& mintimestamp,
									const std::string& maxtimestamp,
									volatile bool* stopAsked)
{
	if (!internalBuild(	rootRefPath,
						previousReferencePath,
						nextReferencePath,
						hoursUpdatePath,
						minutesUpdatePath,
						secondsUpdatePath,
						logPath,
						mintimestamp,
						maxtimestamp,
						stopAsked))
	{
		// clean up if failed...
		// delete all files generated in the next reference path
		vector<string>	files;
		NLMISC::CPath::getPathContent(nextReferencePath, false, false, true, files);

		uint	i;
		for (i=0; i<files.size(); ++i)
			if (!CFile::deleteFile(files[i]))
				nlwarning("CReferenceBuilder::build(): after failure, failed to delete generated file '%s'", files[i].c_str());

		return false;
	}

	return true;
}

/*
 * Build a new reference from a older reference
 * Apply delta changes so new reference is clean
 */
bool	CReferenceBuilder::internalBuild(const std::string& rootRefPath,
										 const std::string& previousReferencePath,
										 const std::string& nextReferencePath,
										 const std::string& hoursUpdatePath,
										 const std::string& minutesUpdatePath,
										 const std::string& secondsUpdatePath,
										 const std::string& logPath,
										 const std::string& mintimestamp,
										 const std::string& maxtimestamp,
										 volatile bool* stopAsked)
{
	PDS_LOG_DEBUG(1)("CReferenceBuilder::build()");

	string	root = NLMISC::CPath::standardizePath(rootRefPath);
	string	previous = NLMISC::CPath::standardizePath(previousReferencePath);
	string	next = NLMISC::CPath::standardizePath(nextReferencePath);
	string	hourspath = NLMISC::CPath::standardizePath(hoursUpdatePath);
	string	minutespath = NLMISC::CPath::standardizePath(minutesUpdatePath);
	string	secondspath = NLMISC::CPath::standardizePath(secondsUpdatePath);

	CTimestamp	minstamp(mintimestamp);
	CTimestamp	maxstamp(maxtimestamp);

	// copy xml description from previous directory to new
	if (!CFile::copyFile(next+"description.xml", previous+"description.xml", true))
	{
		nlwarning("CReferenceBuilder::build(): failed to copy 'description.xml' from '%s' to '%s'", previous.c_str(), next.c_str());
		return false;
	}

	uint	i;

	// copy previous reference
	vector<string>	files;
	NLMISC::CPath::getPathContent(previous, false, false, true, files);

	for (i=0; i<files.size(); ++i)
	{
		if (stopAsked != NULL && *stopAsked)
		{
			nlwarning("CReferenceBuilder::build(): stop asked, operation incomplete");
			return false;
		}

		string	ext = CFile::getExtension(files[i]);

		// is ref file?
		if (ext != CDBReferenceFile::getRefFileExt())
			continue;

		string	file = CFile::getFilename(files[i]);

		// copy from old to new directory
		if (!CFile::copyFile(next+file, previous+file, true))
		{
			nlwarning("CReferenceBuilder::build(): failed to copy '%s' from '%s' to '%s'", file.c_str(), previous.c_str(), next.c_str());
			return false;
		}
	}

	vector<TUpdateList>	updateList;

	if (!buildUpdateList(updateList, hourspath) ||
		!buildUpdateList(updateList, minutespath) ||
		!buildUpdateList(updateList, secondspath))
	{
		nlwarning("CReferenceBuilder::build(): failed to build update list for new reference '%s'", next.c_str());
		return false;
	}

	// first apply hours, then minutes and eventually seconds delta updates
	if (!updateReference(updateList, minstamp, maxstamp, rootRefPath, next))
	{
		nlwarning("CReferenceBuilder::build(): failed to build next reference '%s'", next.c_str());
		return false;
	}

//	buildStringManagerRef(previousReferencePath, nextReferencePath, 
//						  logPath,
//						  minstamp, maxstamp);

	return true;
}



/*
 * Apply delta
 */
bool	CReferenceBuilder::updateReference(std::vector<TUpdateList>& updateList,
										   const CTimestamp& baseTimestamp,
										   const CTimestamp& endTimestamp,
										   const string& refRootPath,
										   const string& refPath,
										   volatile bool* stopAsked)
{
	if (updateList.empty())
		return true;
	
	uint	i, j;
	for (i=0; i<updateList.size(); ++i)
	{
		const TUpdateList&	tableList = updateList[i];

		if (tableList.empty())
			continue;

		CTableBuffer	tableBuffer;
		tableBuffer.init(i, refRootPath, refPath);

		if (!tableBuffer.openAllRefFilesWrite())
		{
			nlwarning("CReferenceBuilder::updateReference(): failed to preopen all reference files for table '%d' in reference '%s'", i, refPath.c_str());
			return false;
		}

		for (j=0; j<tableList.size(); ++j)
		{
			const CUpdateFile&	update = tableList[j];

			if (update.EndTime < baseTimestamp || update.StartTime >= endTimestamp)
				continue;

			if (!tableBuffer.applyDeltaChanges(update.Filename))
			{
				nlwarning("CReferenceBuilder::updateReference(): failed to apply delta file '%s'", update.Filename.c_str());
				return false;
			}

			PDS_LOG_DEBUG(1)("CReferenceBuilder::updateReference(): updated reference with file '%s'", update.Filename.c_str());
		}
	}

	return true;
}

/*
 * Build update list
 */
bool	CReferenceBuilder::buildUpdateList(std::vector<TUpdateList>& updateList, const std::string& filePath)
{
	vector<string>	fileList;
	NLMISC::CPath::getPathContent(filePath, false, false, true, fileList);

	if (fileList.empty())
		return true;

	// sort table by id first then date
	sort(fileList.begin(), fileList.end());

	uint	i;
	for (i=0; i<fileList.size(); ++i)
	{
		uint32		tableId;
		CTimestamp	timestamp;

		if (!CDBDeltaFile::isDeltaFileName(fileList[i], tableId, timestamp))
			continue;

		CDBDeltaFile	delta;
		delta.setup(fileList[i], 0, CTimestamp(), CTimestamp());

		if (!delta.preload())
		{
			nlwarning("CReferenceBuilder::buildUpdateList(): failed to preload file '%s', left as is but built reference may be corrupted", fileList[i].c_str());
			continue;
		}

		CTimestamp	starttime, endtime;

		starttime.fromTime(delta.getStartTimestamp());
		endtime.fromTime(delta.getEndTimestamp());

		if (updateList.size() <= tableId)
			updateList.resize(tableId+1);

		TUpdateList&			list = updateList[tableId];
		TUpdateList::iterator	it;

		bool					insert = true;

		for (it=list.begin(); it!=list.end(); ++it)
		{
			CUpdateFile&	update = *it;

			// ok, insert here
			if (endtime < update.StartTime)
				break;

			// discard any overlap
			if ((starttime >= update.StartTime && starttime < update.EndTime) ||
				(endtime > update.StartTime && endtime <= update.EndTime))
			{
				insert = false;
				break;
			}
		}

		if (!insert)
			continue;

		CUpdateFile	update;

		update.StartTime = starttime;
		update.EndTime = endtime;
		update.Filename = fileList[i];

		list.insert(it, update);
	}

	return true;
}


/* Build string manager reference
 *
 */
//bool	CReferenceBuilder::buildStringManagerRef(const std::string& previousReferencePath,
//												 const std::string& nextReferencePath,
//												 const std::string& logPath,
//												 const CTimestamp& baseTimestamp,
//												 const CTimestamp& endTimestamp)
//{
//	RY_PDS::CPDStringManager	sm;
//
//	if (!sm.load(previousReferencePath))
//	{
//		nlwarning("CReferenceBuilder::build(): failed to load previous reference string manager");
//		return false;
//	}
//
//	vector<string>	filelist;
//	NLMISC::CPath::getPathContent(logPath, false, false, true, filelist);
//
//	vector<string>	updateList;
//
//	uint	i;
//	for (i=0; i<filelist.size(); ++i)
//	{
//		CTimestamp	stamp;
//		if (RY_PDS::CPDStringManager::isLogFileName(filelist[i], stamp) &&
//			stamp >= baseTimestamp && stamp <= endTimestamp)
//		{
//			updateList.push_back(filelist[i]);
//		}
//	}
//
//	sort(updateList.begin(), updateList.end());
//
//	for (i=0; i<updateList.size(); ++i)
//	{
//		CIFile	file;
//		CIXml	xml;
//		if (!file.open(updateList[i]) || !xml.init(file))
//		{
//			nlwarning("CReferenceBuilder::buildStringManagerRef(): failed to load string manager log '%s', ignored", updateList[i].c_str());
//			continue;
//		}
//
//		if (!sm.applyLog(xml))
//		{
//			nlwarning("CReferenceBuilder::buildStringManagerRef(): failed to apply string manager log '%s', ignored", updateList[i].c_str());
//			continue;
//		}
//	}
//
//	if (!sm.save(nextReferencePath))
//	{
//		nlwarning("CReferenceBuilder::build(): failed to save next reference string manager");
//		return false;
//	}
//
//	return true;
//}
