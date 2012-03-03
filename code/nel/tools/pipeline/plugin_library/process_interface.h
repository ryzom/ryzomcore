/**
 * \file process_interface.h
 * \brief IProcessInterface
 * \date 2012-03-03 09:22GMT
 * \author Jan Boon (Kaetemi)
 * IProcessInterface
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_PROCESS_INTERFACE_H
#define PIPELINE_PROCESS_INTERFACE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace PIPELINE {

/**
 * \brief IProcessInterface
 * \date 2012-03-03 09:22GMT
 * \author Jan Boon (Kaetemi)
 * IProcessInterface
 */
class IProcessInterface
{
public:
	IProcessInterface();
	virtual ~IProcessInterface();

	static IProcessInterface *getInstance();

	// ***************** PROCESS FUNCTIONS (EASILY EXPOSABLE TO SCRIPTS ETCETERA) *****************

	/// Get a parsed georges sheets value from the current project. (example: Interface.DstDirectory)
	virtual std::string getProjectValue(const std::string &name) = 0;

	/// Get the temporary directory for the current process. The directory must be deleted when the process ends. May return random temporary directories if no process is running.
	virtual std::string getTempDir() = 0;
}; /* class IProcessInterface */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_INTERFACE_H */

/* end of file */
