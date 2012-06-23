/**
 * \file process_handler.h
 * \brief IProcessHandler
 * \date 2012-03-03 10:14GMT
 * \author Jan Boon (Kaetemi)
 * IProcessHandler
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

#ifndef PIPELINE_PROCESS_HANDLER_H
#define PIPELINE_PROCESS_HANDLER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_registry.h>

// Project includes

namespace PIPELINE {

/**
 * \brief IProcessHandler
 * \date 2012-03-03 10:14GMT
 * \author Jan Boon (Kaetemi)
 * IProcessHandler
 * A process handler is executed by the PLS SLAVE services. Processes can have multiple handlers. These are configured under the workspace plugins sheet.
 */
class IProcessHandler : public NLMISC::IClassable
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	IProcessHandler();
	virtual ~IProcessHandler();
}; /* class IProcessHandler */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_HANDLER_H */

/* end of file */
