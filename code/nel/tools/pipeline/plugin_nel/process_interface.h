/**
 * \file process_interface.h
 * \brief CProcessInterface
 * \date 2012-03-03 10:10GMT
 * \author Jan Boon (Kaetemi)
 * CProcessInterface
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
#include "../plugin_library/process_handler.h"

namespace PIPELINE {

/**
 * \brief CProcessInterface
 * \date 2012-03-03 10:10GMT
 * \author Jan Boon (Kaetemi)
 * CProcessInterface
 */
class CProcessInterface : public IProcessHandler
{
public:
	CProcessInterface();
	virtual ~CProcessInterface();

	void buildAtlas(const std::vector<std::string> &srcDirectories, const std::string &dstFile);

	virtual void build();

	NLMISC_DECLARE_CLASS(CProcessInterface)
}; /* class CProcessInterface */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_INTERFACE_H */

/* end of file */
