/**
 * \file pipeline_workspace.cpp
 * \brief CPipelineWorkspace
 * \date 2012-02-18 17:23GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineWorkspace
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
#include "pipeline_workspace.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>

// Project includes

using namespace std;
// using namespace NLMISC;
using namespace NLGEORGES;

namespace PIPELINE {

CPipelineWorkspace::CPipelineWorkspace(NLGEORGES::UFormLoader *formLoader, const std::string &sheetName) : m_FormLoader(formLoader)
{
	m_Form = formLoader->loadForm(sheetName.c_str());
	std::string description;
	m_Form->getRootNode().getValueByName(description, "Description");
	nlinfo("Loading pipeline workspace: '%s'", description.c_str());

	{
		UFormElm *plugins;
		if (m_Form->getRootNode().getNodeByName(&plugins, "Plugins"))
		{
			uint nb;
			plugins->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				std::string pluginSheet;
				if (plugins->getArrayValue(pluginSheet, i))
				{
					m_Plugins.push_back(formLoader->loadForm(pluginSheet.c_str()));
				}
				else
				{
					nlwarning("Array error in '%s'", m_Form->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'Plugins' in '%s'", m_Form->getFilename().c_str());
		}
	}

	{
		UFormElm *projects;
		if (m_Form->getRootNode().getNodeByName(&projects, "Projects"))
		{
			uint nb;
			projects->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				std::string projectSheet;
				if (projects->getArrayValue(projectSheet, i))
				{
					m_Projects.push_back(formLoader->loadForm(projectSheet.c_str()));
				}
				else
				{
					nlwarning("Array error in '%s'", m_Form->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'Projects' in '%s'", m_Form->getFilename().c_str());
		}
	}
}

CPipelineWorkspace::~CPipelineWorkspace()
{
	
}

void CPipelineWorkspace::getProcessPlugins(std::vector<CProcessPluginInfo> &result, const std::string &process)
{
	for (std::vector<NLMISC::CRefPtr<NLGEORGES::UForm> >::iterator it = m_Plugins.begin(), end = m_Plugins.end(); it != end; ++it)
	{
		UFormElm *processHandlers;
		if ((*it)->getRootNode().getNodeByName(&processHandlers, "ProcessHandlers"))
		{
			uint nb;
			processHandlers->getArraySize(nb);
			for (uint i = 0; i < nb; ++i)
			{
				UFormElm *handler;
				if (processHandlers->getArrayNode(&handler, i))
				{
					std::string handlerProcess;
					if (handler->getValueByName(handlerProcess, "Process"))
					{
						if (handlerProcess == process)
						{
							CProcessPluginInfo processPlugin;
							uint32 handlerType;
							uint32 infoType;
							if (handler->getValueByName(handlerType, "HandlerType")
								&& handler->getValueByName(processPlugin.Handler, "Handler")
								&& handler->getValueByName(infoType, "InfoType")
								&& handler->getValueByName(processPlugin.Info, "Info"))
							{
								processPlugin.HandlerType = (TPluginType)handlerType;
								processPlugin.InfoType = (TPluginType)infoType;
								result.push_back(processPlugin);

								nldebug("Found '%s': '%s', '%s'", process.c_str(), processPlugin.Handler.c_str(), processPlugin.Info.c_str());
							}
							else
							{
								nlwarning("Missing value in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
							}
						}
					}
					else
					{
						nlwarning("Missing 'Process' in '%s' at 'ProcessHandlers' at '%i'", (*it)->getFilename().c_str(), i);
					}
				}
				else
				{
					nlwarning("Array error in '%s'", (*it)->getFilename().c_str());
				}
			}
		}
		else
		{
			nlwarning("Missing 'ProcessHandlers' in '%s'", (*it)->getFilename().c_str());
		}
	}
}

} /* namespace PIPELINE */

/* end of file */
