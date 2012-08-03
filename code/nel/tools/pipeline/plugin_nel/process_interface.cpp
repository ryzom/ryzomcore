/**
 * \file process_interface.cpp
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

#include <nel/misc/types_nl.h>
#include "process_interface.h"

// STL includes
#include <sstream>

// NeL includes
#include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CProcessInterface::CProcessInterface()
{
	
}

CProcessInterface::~CProcessInterface()
{
	
}

void CProcessInterface::buildAtlas(const std::vector<std::string> &srcDirectories, const std::string &dstFile)
{
	nldebug("Build atlas '%s'", dstFile.c_str());
	std::stringstream ss;
	ss << "build_interface " << dstFile;
	for (std::vector<std::string>::const_iterator it = srcDirectories.begin(), end = srcDirectories.end(); it != end; ++it)
	{
		const std::string &path = *it;
		ss << " " << path;
	}
	system(ss.str().c_str());
}

void CProcessInterface::build()
{
	nldebug("Building process interface!");

	{
		uint nb;
		if (m_PipelineProcess->getValueNb(nb, "Interface.Atlas"))
		{
			for (uint i = 0; i < nb; ++i)
			{
				std::vector<std::string> srcDirectories;
				{
					std::stringstream ss;
					ss << "Interface.Atlas[" << i << "].SrcDirectories";
					if (!m_PipelineProcess->getValues(srcDirectories, ss.str())) break;
				}
				std::string dstFile;
				{
					std::stringstream ss;
					ss << "Interface.Atlas[" << i << "].DstFile";
					if (!m_PipelineProcess->getValue(dstFile, ss.str())) break;
				}
				if (m_PipelineProcess->needsToBeRebuilt(srcDirectories, dstFile))
				{
					m_PipelineProcess->makePaths(dstFile);
					buildAtlas(srcDirectories, dstFile);
					m_PipelineProcess->parseToolLog("", "", false);
				}
				if (m_PipelineProcess->needsExit()) return;
			}
		}
	}

	m_PipelineProcess->setExit(FINISH_ERROR, "Not yet implemented");
}

} /* namespace PIPELINE */

/* end of file */
