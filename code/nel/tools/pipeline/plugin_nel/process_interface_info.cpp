/**
 * \file process_interface_info.cpp
 * \brief CProcessInterfaceInfo
 * \date 2012-03-03 10:10GMT
 * \author Jan Boon (Kaetemi)
 * CProcessInterfaceInfo
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

#include <nel/misc/types_nl.h>
#include "process_interface_info.h"

// STL includes
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CProcessInterfaceInfo::CProcessInterfaceInfo()
{
	
}

CProcessInterfaceInfo::~CProcessInterfaceInfo()
{
	
}

void CProcessInterfaceInfo::getDependentDirectories(std::vector<std::string> &resultAppend)
{
	{
		uint nb;
		if (m_PipelineProcess->getValueNb(nb, "Interface.Atlas"))
		{
			for (uint i = 0; i < nb; ++i)
			{
				std::stringstream ss;
				ss << "Interface.Atlas[" << i << "].SrcDirectories";
				m_PipelineProcess->getValues(resultAppend, ss.str());
			}
		}
	}
	{
		uint nb;
		if (m_PipelineProcess->getValueNb(nb, "Interface.AtlasDxtc"))
		{
			for (uint i = 0; i < nb; ++i)
			{
				std::stringstream ss;
				ss << "Interface.AtlasDxtc[" << i << "].SrcDirectories";
				m_PipelineProcess->getValues(resultAppend, ss.str());
			}
		}
	}
	{
		std::stringstream ss;
		ss << "Interface.Fullscreen.SrcDirectories";
		m_PipelineProcess->getValues(resultAppend, ss.str());
	}
	{
		std::stringstream ss;
		ss << "Interface.3D.SrcDirectories";
		m_PipelineProcess->getValues(resultAppend, ss.str());
	}
}

void CProcessInterfaceInfo::getDependentFiles(std::vector<std::string> &resultAppend)
{
	
}

} /* namespace PIPELINE */

/* end of file */
