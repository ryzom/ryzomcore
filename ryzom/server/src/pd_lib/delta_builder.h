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

#ifndef NL_DELTA_BUILDER_H
#define NL_DELTA_BUILDER_H

#include "nel/misc/types_nl.h"

#include "timestamp.h"

#include <vector>


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CDeltaBuilder
{
public:

	/**
	 * Type of Delta to build
	 */
	enum TDelta
	{
		Second,
		Minute,
		Hour
	};

	/**
	 * Build delta
	 * Builds delta files from an stamp interval
	 */
	static bool		build(const std::string& outputPath,
						  const std::string& hoursUpdatePath,
						  const std::string& minutesUpdatePath,
						  const std::string& secondsUpdatePath,
						  const std::string& mintimestamp,
						  const std::string& maxtimestamp,
						  TDelta deltaType);

	/*
	 * Remove older files in update path
	 */
	static bool		removeOlderDeltaInPath(const std::string& keeptimestamp,
										   const std::string& path);

private:

	/// Constructor
	CDeltaBuilder();


	/**
	 * Build delta
	 * Builds delta files from an stamp interval
	 */
	static bool		internalBuild(	const std::string& outputPath,
									const std::string& hoursUpdatePath,
									const std::string& minutesUpdatePath,
									const std::string& secondsUpdatePath,
									const std::string& mintimestamp,
									const std::string& maxtimestamp,
									TDelta deltaType,
									std::vector<std::string>& generatedFiles);

	/// Generate Delta update file
	static bool		generateDeltaFile(const std::string& outputPath,
									  const std::vector<std::string>& updateFiles,
									  const CTimestamp& starttime,
									  const CTimestamp& endtime);

	/// Build update files list
	static bool		updateFilesList(std::vector<std::vector<std::string> >& fileLists,
									std::vector<std::string>& updateFiles,
									const CTimestamp& minstamp,
									const CTimestamp& maxstamp);

};


#endif // NL_DELTA_BUILDER_H

/* End of delta_builder.h */
