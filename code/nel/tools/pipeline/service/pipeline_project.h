/**
 * \file pipeline_project.h
 * \brief CPipelineProject
 * \date 2012-03-03 11:31GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProject
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

#ifndef PIPELINE_PIPELINE_PROJECT_H
#define PIPELINE_PIPELINE_PROJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>
#include <nel/georges/u_form.h>

// Project includes

namespace PIPELINE {
	class CPipelineWorkspace;

/**
 * \brief CPipelineProject
 * \date 2012-03-03 11:31GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProject
 */
class CPipelineProject
{
	friend class PIPELINE::CPipelineWorkspace;

protected:
	CPipelineWorkspace *m_Workspace;
	NLMISC::CRefPtr<NLGEORGES::UForm> m_Form;
	std::string m_TempDirectory;
	uint32 m_ChangedReference;
	uint32 m_FileSizeReference;
	uint32 m_CRC32;

public:
	CPipelineProject(CPipelineWorkspace *workspace, NLGEORGES::UForm *form);
	virtual ~CPipelineProject();

	bool getValue(std::string &result, const std::string &name);
	bool getValues(std::vector<std::string> &resultAppend, const std::string &name);
	bool getValueNb(uint &result, const std::string &name);

	bool getMacro(std::string &result, const std::string &name);

	/// Gets the project name.
	std::string getName();

	/// Gets the output directory for the project.
	std::string getOutputDirectory();

	/// Gets a temporary directory for the current process. Should be created/deleted by process when (no longer) needed. Temp directories MUST NOT be shared between seperate processes, as these may run in different systems.
	std::string getTempDirectory();

private:
	// Strip all macros and turn all macro paths into real paths.
	void parseValue(std::string &result, const std::string &value);

}; /* class CPipelineProject */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROJECT_H */

/* end of file */
