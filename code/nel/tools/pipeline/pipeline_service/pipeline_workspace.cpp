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
}

CPipelineWorkspace::~CPipelineWorkspace()
{
	
}

} /* namespace PIPELINE */

/* end of file */
