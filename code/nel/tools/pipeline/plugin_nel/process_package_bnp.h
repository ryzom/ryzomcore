/**
 * \file process_package_bnp.h
 * \brief CProcessPackageBNP
 * \date 2012-08-04 18:54GMT
 * \author Jan Boon (Kaetemi)
 * CProcessPackageBNP
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_PROCESS_PACKAGE_BNP_H
#define PIPELINE_PROCESS_PACKAGE_BNP_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../plugin_library/process_handler.h"
#include "../plugin_library/process_info.h"

namespace PIPELINE {

/**
 * \brief CProcessPackageBNP
 * \date 2012-08-04 18:54GMT
 * \author Jan Boon (Kaetemi)
 * CProcessPackageBNP
 */
class CProcessPackageBNP : public IProcessHandler
{
public:
	CProcessPackageBNP() { }
	virtual ~CProcessPackageBNP() { }

	virtual void build();

	NLMISC_DECLARE_CLASS(CProcessPackageBNP)
}; /* class CProcessPackageBNP */

/**
 * \brief CProcessPackageBNPInfo
 * \date 2012-08-04 18:54GMT
 * \author Jan Boon (Kaetemi)
 * CProcessPackageBNPInfo
 */
class CProcessPackageBNPInfo : public IProcessInfo
{
public:
	CProcessPackageBNPInfo() { }
	virtual ~CProcessPackageBNPInfo() { }

	virtual void getDependentDirectories(std::vector<std::string> &resultAppend);
	virtual void getDependentFiles(std::vector<std::string> &resultAppend);

	NLMISC_DECLARE_CLASS(CProcessPackageBNPInfo)
}; /* class CProcessPackageBNPInfo */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_PACKAGE_BNP_H */

/* end of file */
