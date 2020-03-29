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

#include "ref_builder_task.h"
#include "pd_lib/reference_builder.h"
#include "pd_lib/delta_builder.h"


/*
 * Constructor
 */
CRefBuilderTask::CRefBuilderTask()
{
}


/*
 * Destructor
 */
CRefBuilderTask::~CRefBuilderTask()
{
}



/*
 * Setup Task
 */
void	CRefBuilderTask::setup(	const std::string& rootRefPath,
								const std::string& previousReferencePath,
								const std::string& nextReferencePath,
								const std::string& hoursUpdatePath,
								const std::string& minutesUpdatePath,
								const std::string& secondsUpdatePath,
								const std::string& logUpdatePath,
								const std::string& mintimestamp,
								const std::string& maxtimestamp,
								const std::string& keeptimestamp)
{
	_RootRefPath = rootRefPath;
	_PreviousReferencePath = previousReferencePath;
	_NextReferencePath = nextReferencePath;
	_HoursUpdatePath = hoursUpdatePath;
	_MinutesUpdatePath = minutesUpdatePath;
	_SecondsUpdatePath = secondsUpdatePath;
	_LogUpdatePath = logUpdatePath;
	_Mintimestamp = mintimestamp;
	_Maxtimestamp = maxtimestamp;
	_KeepTimestamp = keeptimestamp;
}


/*
 * Run task
 */
bool	CRefBuilderTask::execute()
{
	// generate new reference
	if (!CReferenceBuilder::build(	_RootRefPath,
									_PreviousReferencePath,
									_NextReferencePath,
									_HoursUpdatePath,
									_MinutesUpdatePath,
									_SecondsUpdatePath,
									_LogUpdatePath,
									_Mintimestamp,
									_Maxtimestamp,
									&AskedToStop ))
	{
		nlwarning("CRefBuilderTask: failed to build new reference, from '%s' to '%s'", _PreviousReferencePath.c_str(), _NextReferencePath.c_str());
		return false;
	}
	else
	{
		nlinfo("CRefBuilderTask: built new reference '%s'", _NextReferencePath.c_str());

		// remove older files
		CDeltaBuilder::removeOlderDeltaInPath(_KeepTimestamp, _HoursUpdatePath);
		CDeltaBuilder::removeOlderDeltaInPath(_KeepTimestamp, _MinutesUpdatePath);
		CDeltaBuilder::removeOlderDeltaInPath(_KeepTimestamp, _SecondsUpdatePath);
	}

	return true;
}

/*
 * Compute next timestamp
 */
std::string	CRefBuilderTask::getNextTimestamp(const std::string& timestamp)
{
	uint	year, month, day, hour, minute, second;

	if (sscanf(timestamp.c_str(), "%d.%d.%d.%d.%d.%d", &year, &month, &day, &hour, &minute, &second) != 6)
	{
		nlwarning("getNextTimestamp: failed to compute next timestamp, '%s' can't be decoded", timestamp.c_str());
		return "";
	}

	// go on a daily basis
	hour = 0;
	minute = 0;
	second = 0;

	++day;

	bool		bissextile = ((year%4) == 0) && (((year%100) != 0) || ((year%400) == 0));

	static uint	maxdays[12][2] = {	{ 31, 31 }, { 28, 29 }, { 31, 31 }, { 30, 30 },
									{ 31, 31 }, { 30, 30 }, { 31, 31 }, { 31, 31 },
									{ 30, 30 }, { 31, 31 }, { 30, 30 }, { 31, 31 } };

	if ( (day > maxdays[month-1][bissextile ? 1 : 0]) )
	{
		day = 1;
		++month;
	}

	if (month > 12)
	{
		month = 1;
		++year;
	}

	return NLMISC::toString("%04d.%02d.%02d.%02d.%02d.%02d", year, month, day, hour, minute, second);
}
