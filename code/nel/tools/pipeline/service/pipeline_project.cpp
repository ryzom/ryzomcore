/**
 * \file pipeline_project.cpp
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

#include <nel/misc/types_nl.h>
#include "pipeline_project.h"

// STL includes
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineProject::CPipelineProject(CPipelineWorkspace *workspace, NLGEORGES::UForm *form) : m_Workspace(workspace), m_Form(form)
{
	
}

CPipelineProject::~CPipelineProject()
{
	
}

bool CPipelineProject::getValue(std::string &result, const std::string &name)
{
	std::string value;
	if (!m_Form->getRootNode().getValueByName(value, name.c_str()))
	{
		nlwarning("Value '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	parseValue(result, value);
	return true;
}

bool CPipelineProject::getValues(std::vector<std::string> &resultAppend, const std::string &name)
{
	NLGEORGES::UFormElm *elm;
	if (!m_Form->getRootNode().getNodeByName(&elm, name.c_str()))
	{
		nlwarning("Node '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	uint size;
	if (!elm->getArraySize(size))
	{
		nlwarning("Array size of node '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	std::vector<std::string>::size_type originalSize = resultAppend.size();
	resultAppend.reserve(resultAppend.size() + (std::vector<std::string>::size_type)size);
	for (uint i = 0; i < size; ++i)
	{
		std::string value;
		if (!elm->getArrayValue(value, i))
		{
			nlwarning("Array value of node '%s' at '%i' not found in '%s'", name.c_str(), i, m_Form->getFilename().c_str());
			resultAppend.resize(originalSize);
			return false;
		}
		std::string parsed;
		parseValue(parsed, value);
		resultAppend.push_back(parsed);
	}
	return true;
}

bool CPipelineProject::getValueNb(uint &result, const std::string &name)
{
	NLGEORGES::UFormElm *elm;
	if (!m_Form->getRootNode().getNodeByName(&elm, name.c_str()))
	{
		nlwarning("Node '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	if (!elm->getArraySize(result))
	{
		nlwarning("Array size of node '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	return true;
}

void CPipelineProject::parseValue(std::string &result, const std::string &value)
{
	std::stringstream ss;

	std::string::const_iterator lastEnd = value.begin();
	std::string::const_iterator findOpen = find(lastEnd, value.end(), '[');
	ss << std::string(lastEnd, findOpen);
	while (findOpen != value.end())
	{
		++findOpen;
		switch (*findOpen)
		{
		case '$':
			// TODO
			break;
		case '@':
			// TODO
			break;
		case '#':
			// TODO
			break;
		default:			
			// TODO
			break;
		}
	}

	result = ss.str();
}

} /* namespace PIPELINE */

/* end of file */
