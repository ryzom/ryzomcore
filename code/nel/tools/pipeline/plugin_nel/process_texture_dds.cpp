/**
 * \file process_texture_dds.cpp
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

#include <nel/misc/types_nl.h>
#include "process_texture_dds.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

void CProcessTextureDDS::build()
{
	nldebug("Build process plugin: CProcessTextureDDS");
	m_PipelineProcess->setExit(FINISH_ERROR, "Not implemented");
}

void CProcessTextureDDSInfo::getDependentDirectories(std::vector<std::string> &resultAppend)
{
	{
		uint nb;
		if (m_PipelineProcess->getValueNb(nb, "Texture.DDS"))
		{
			for (uint i = 0; i < nb; ++i)
			{
				std::stringstream ss;
				ss << "Texture.DDS[" << i << "].SrcDirectories";
				m_PipelineProcess->getValues(resultAppend, ss.str());
			}
		}
	}
}

void CProcessTextureDDSInfo::getDependentFiles(std::vector<std::string> &resultAppend)
{
	
}

} /* namespace PIPELINE */

/* end of file */
