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

#ifndef NL_DELTA_BUILDER_TASK_H
#define NL_DELTA_BUILDER_TASK_H

#include "nel/misc/types_nl.h"
#include "reference_builder_service.h"

#include "pd_lib/delta_builder.h"


/**
 * This task builds a delta from other delta updates
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDeltaBuilderTask : public IRefTask
{
public:

	/// Constructor
	CDeltaBuilderTask();

	/// Destructor
	~CDeltaBuilderTask();

	/// Setup Task
	void		setup(const std::string& outputPath,
					  const std::string& hoursUpdatePath,
					  const std::string& minutesUpdatePath,
					  const std::string& secondsUpdatePath,
					  const std::string& mintimestamp,
					  const std::string& maxtimestamp,
					  CDeltaBuilder::TDelta type,
					  const std::string& keeptimestamp);



	/// Run task
	virtual bool	execute();


private:

	std::string		_OutputPath;
	std::string		_HoursUpdatePath;
	std::string		_MinutesUpdatePath;
	std::string		_SecondsUpdatePath;
	std::string		_Mintimestamp;
	std::string		_Maxtimestamp;
	CDeltaBuilder::TDelta	_Type;
	std::string		_KeepTimestamp;
};


#endif // NL_DELTA_BUILDER_TASK_H

/* End of delta_builder_task.h */
