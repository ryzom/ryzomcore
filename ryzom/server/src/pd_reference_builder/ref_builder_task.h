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

#ifndef NL_REF_BUILDER_TASK_H
#define NL_REF_BUILDER_TASK_H

#include <nel/misc/types_nl.h>
#include <nel/misc/thread.h>

#include "reference_builder_service.h"

/**
 * This task builds a new reference from a previous reference and various delta updates
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CRefBuilderTask : public IRefTask
{
public:

	/// Constructor
	CRefBuilderTask();

	/// Destructor
	~CRefBuilderTask();


	/// Setup Task
	void		setup(const std::string& rootRefPath,
					  const std::string& previousReferencePath,
					  const std::string& nextReferencePath,
					  const std::string& hoursUpdatePath,
					  const std::string& minutesUpdatePath,
					  const std::string& secondsUpdatePath,
					  const std::string& logUpdatePath,
					  const std::string& mintimestamp,
					  const std::string& maxtimestamp,
					  const std::string& keeptimestamp);



	/// Run task
	virtual bool	execute();


private:

	std::string		_RootRefPath;
	std::string		_PreviousReferencePath;
	std::string		_NextReferencePath;
	std::string		_HoursUpdatePath;
	std::string		_MinutesUpdatePath;
	std::string		_SecondsUpdatePath;
	std::string		_LogUpdatePath;
	std::string		_Mintimestamp;
	std::string		_Maxtimestamp;
	std::string		_KeepTimestamp;

public:

	/// Compute next timestamp
	static std::string	getNextTimestamp(const std::string& timestamp);
};


#endif // NL_REF_BUILDER_TASK_H

/* End of ref_builder_task.h */
