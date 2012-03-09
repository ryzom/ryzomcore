/**
 * \file pipeline_workspace.h
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

#ifndef PIPELINE_PIPELINE_WORKSPACE_H
#define PIPELINE_PIPELINE_WORKSPACE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <map>

// NeL includes
#include <nel/georges/u_form_loader.h>
#include <nel/georges/u_form.h>

// Project includes

namespace PIPELINE {
	class CPipelineProject;

enum TPluginType
{
	PLUGIN_REGISTERED_CLASS, 
	PLUGIN_LUA_SCRIPT, 
};

struct CProcessPluginId
{
	union
	{
		struct 
		{
			uint16 Plugin;
			uint16 Handler;
		} Sub;
		uint32 Global;
	};
};

struct CProcessPluginInfo
{
	CProcessPluginId Id;
	TPluginType HandlerType;
	std::string Handler;
	TPluginType InfoType;
	std::string Info;
};

/**
 * \brief CPipelineWorkspace
 * \date 2012-02-18 17:23GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineWorkspace
 */
class CPipelineWorkspace
{
	friend class CPipelineProject;

protected:
	NLGEORGES::UFormLoader *m_FormLoader;
	NLMISC::CRefPtr<NLGEORGES::UForm> m_Form;
	std::vector<NLMISC::CRefPtr<NLGEORGES::UForm> > m_Plugins;
	std::map<std::string, CPipelineProject *> m_Projects;

public:
	CPipelineWorkspace(NLGEORGES::UFormLoader *formLoader, const std::string &sheetName);
	virtual ~CPipelineWorkspace();

	void getProcessPlugins(std::vector<CProcessPluginInfo> &resultAppend, const std::string &process);
	CPipelineProject *getProject(const std::string &project);
	bool getProcessPlugin(CProcessPluginInfo &result, uint32 globalId);

	inline const std::map<std::string, CPipelineProject *> &getProjects() { return m_Projects; }

	/// Makes a list of the global id of all available plugins to this service
	void listAvailablePlugins(std::vector<uint32> &result);

}; /* class CPipelineWorkspace */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_WORKSPACE_H */

/* end of file */
