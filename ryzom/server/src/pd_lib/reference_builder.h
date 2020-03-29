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

#ifndef NL_REFERENCE_BUILDER_H
#define NL_REFERENCE_BUILDER_H

#include "nel/misc/types_nl.h"

#include <vector>

#include "db_delta_file.h"
#include "db_reference_file.h"

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CReferenceBuilder
{
public:

	/**
	 * Build a new reference from a older reference
	 * Apply delta changes so new reference is clean
	 */
	static bool	build(CRefIndex& previous, CRefIndex& next);

	/**
	 * Build a new reference from a older reference
	 * Apply delta changes so new reference is clean
	 */
	static bool	build(const std::string& rootRefPath,
					  const std::string& previousReferencePath,
					  const std::string& nextReferencePath,
					  const std::string& hoursUpdatePath,
					  const std::string& minutesUpdatePath,
					  const std::string& secondsUpdatePath,
					  const std::string& logPath,
					  const std::string& mintimestamp,
					  const std::string& maxtimestamp,
					  volatile bool* stopAsked = NULL);


private:

	struct CUpdateFile
	{
		CTimestamp	StartTime;
		CTimestamp	EndTime;
		std::string	Filename;
	};


	typedef std::vector<CUpdateFile>	TUpdateList;

	/**
	 * Build a new reference from a older reference
	 * Apply delta changes so new reference is clean
	 */
	static bool	internalBuild(const std::string& rootRefPath,
							  const std::string& previousReferencePath,
							  const std::string& nextReferencePath,
							  const std::string& hoursUpdatePath,
							  const std::string& minutesUpdatePath,
							  const std::string& secondsUpdatePath,
							  const std::string& logPath,
							  const std::string& mintimestamp,
							  const std::string& maxtimestamp,
							  volatile bool* stopAsked = NULL);

	/// Apply delta
	static bool	updateReference(std::vector<TUpdateList>& updateList,
								const CTimestamp& baseTimestamp,
								const CTimestamp& endTimestamp,
								const std::string& refRootPath,
								const std::string& refPath,
								volatile bool* stopAsked = NULL);

	/// Build update list
	static bool	buildUpdateList(std::vector<TUpdateList>& updateList, const std::string& filePath);

	/// Build string manager reference
//	static bool	buildStringManagerRef(const std::string& previousReferencePath,
//									  const std::string& nextReferencePath,
//									  const std::string& logPath,
//									  const CTimestamp& baseTimestamp,
//									  const CTimestamp& endTimestamp);


	/// Constructor, hidden, class is only sigleton utility
	CReferenceBuilder();

};


#endif // NL_REFERENCE_BUILDER_H

/* End of reference_builder.h */
