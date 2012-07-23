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

#include "delta_builder_task.h"


/*
 * Constructor
 */
CDeltaBuilderTask::CDeltaBuilderTask()
{
}

/*
 * Destructor
 */
CDeltaBuilderTask::~CDeltaBuilderTask()
{
}


/*
 * Setup Task
 */
void	CDeltaBuilderTask::setup(const std::string& outputPath,
								 const std::string& hoursUpdatePath,
								 const std::string& minutesUpdatePath,
								 const std::string& secondsUpdatePath,
								 const std::string& mintimestamp,
								 const std::string& maxtimestamp,
								 CDeltaBuilder::TDelta type,
								 const std::string& keeptimestamp)
{
	_OutputPath = outputPath;
	_HoursUpdatePath = hoursUpdatePath;
	_MinutesUpdatePath = minutesUpdatePath;
	_SecondsUpdatePath = secondsUpdatePath;
	_Mintimestamp = mintimestamp;
	_Maxtimestamp = maxtimestamp;
	_Type = type;
	_KeepTimestamp = keeptimestamp;
}



/*
 * Run task
 */
bool	CDeltaBuilderTask::execute()
{
	// generate new reference
	if (!CDeltaBuilder::build(	_OutputPath,
								_HoursUpdatePath,
								_MinutesUpdatePath,
								_SecondsUpdatePath,
								_Mintimestamp,
								_Maxtimestamp,
								_Type))
	{
		nlwarning("CDeltaBuilder: failed to build delta");
		return false;
	}
	else
	{
		nlinfo("CDeltaBuilder: built delta between '%s' and '%s'", _Mintimestamp.c_str(), _Maxtimestamp.c_str());

		// remove older files
		if (_Type == CDeltaBuilder::Hour)
			CDeltaBuilder::removeOlderDeltaInPath(_KeepTimestamp, _MinutesUpdatePath);
		if (_Type == CDeltaBuilder::Hour || _Type == CDeltaBuilder::Minute)
			CDeltaBuilder::removeOlderDeltaInPath(_KeepTimestamp, _SecondsUpdatePath);
	}

	return true;
}
