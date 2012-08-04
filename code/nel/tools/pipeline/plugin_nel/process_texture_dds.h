/**
 * \file process_texture_dds.h
 * \brief CProcessTextureDDS
 * \date 2012-08-04 12:50GMT
 * \author Jan Boon (Kaetemi)
 * CProcessTextureDDS
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

#ifndef PIPELINE_PROCESS_TEXTURE_DDS_H
#define PIPELINE_PROCESS_TEXTURE_DDS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../plugin_library/process_handler.h"
#include "../plugin_library/process_info.h"

namespace PIPELINE {

/**
 * \brief CProcessTextureDDS
 * \date 2012-08-04 12:50GMT
 * \author Jan Boon (Kaetemi)
 * CProcessTextureDDS
 */
class CProcessTextureDDS : public IProcessHandler
{
public:
	CProcessTextureDDS() { }
	virtual ~CProcessTextureDDS() { }

	void buildDDS(const std::string &dependLog, const std::string &errorLog, const std::string &srcFile, const std::string &dstFile, const std::string &algorithm, bool createMipMap, uint reduceFactor, bool checkUserColor);

	virtual void build();

	NLMISC_DECLARE_CLASS(CProcessTextureDDS)
}; /* class CProcessTextureDDS */

/**
 * \brief CProcessTextureDDS
 * \date 2012-08-04 12:50GMT
 * \author Jan Boon (Kaetemi)
 * CProcessTextureDDS
 */
class CProcessTextureDDSInfo : public IProcessInfo
{
public:
	CProcessTextureDDSInfo() { }
	virtual ~CProcessTextureDDSInfo() { }

	virtual void getDependentDirectories(std::vector<std::string> &resultAppend);	
	virtual void getDependentFiles(std::vector<std::string> &resultAppend);

	NLMISC_DECLARE_CLASS(CProcessTextureDDSInfo)
}; /* class CProcessTextureDDS */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_TEXTURE_DDS_H */

/* end of file */
