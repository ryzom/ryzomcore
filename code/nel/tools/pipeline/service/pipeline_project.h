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
protected:
	CPipelineWorkspace *m_Workspace;
	NLMISC::CRefPtr<NLGEORGES::UForm> m_Form;
public:
	CPipelineProject(CPipelineWorkspace *workspace, NLGEORGES::UForm *form);
	virtual ~CPipelineProject();

}; /* class CPipelineProject */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROJECT_H */

/* end of file */
