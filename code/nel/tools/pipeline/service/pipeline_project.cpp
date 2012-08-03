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
#include <nel/georges/u_type.h>
#include <nel/net/service.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>

// Project includes
#include "pipeline_service.h"
#include "workspace_storage.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

CPipelineProject::CPipelineProject(CPipelineWorkspace *workspace, NLGEORGES::UForm *form) : m_Workspace(workspace), m_Form(form), m_ChangedReference(0), m_FileSizeReference(0), m_CRC32(0)
{
	
}

CPipelineProject::~CPipelineProject()
{
	
}

bool CPipelineProject::getValue(std::string &result, const std::string &name)
{
	NLGEORGES::UFormElm *valueElm;
	if (!m_Form->getRootNode().getNodeByName(&valueElm, name.c_str()))
	{
		nlwarning("Node '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	std::string value;
	if (!valueElm->getValue(value))
	{
		nlwarning("Value '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	std::string parsed;
	parseValue(parsed, value);
	std::string typComment = valueElm->getType()->getComment();
	if (typComment == "PIPELINE_PATH")
	{
		parsed = standardizePath(parsed, false);
	}
	else if (typComment == "PIPELINE_PATH_ENDSLASH")
	{
		parsed = standardizePath(parsed, true);
	}
	result = parsed;
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
		NLGEORGES::UFormElm *valueElm;
		if (!elm->getArrayNode(&valueElm, i))
		{
			nlwarning("Array node of node '%s' at '%i' not found in '%s'", name.c_str(), i, m_Form->getFilename().c_str());
			resultAppend.resize(originalSize);
			return false;
		}
		std::string value;
		if (!valueElm->getValue(value))
		{
			nlwarning("Array value of node '%s' at '%i' not found in '%s'", name.c_str(), i, m_Form->getFilename().c_str());
			resultAppend.resize(originalSize);
			return false;
		}
		std::string parsed;
		parseValue(parsed, value);
		std::string typComment = valueElm->getType()->getComment();
		if (typComment == "PIPELINE_PATH")
		{
			parsed = standardizePath(parsed, false);
		}
		else if (typComment == "PIPELINE_PATH_ENDSLASH")
		{
			parsed = standardizePath(parsed, true);
		}
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

bool CPipelineProject::getMacro(std::string &result, const std::string &name)
{
	// TODO: Maybe preload the macros into a map.

	NLGEORGES::UFormElm *elm;
	if (!m_Form->getRootNode().getNodeByName(&elm, "Macros"))
	{
		nlwarning("Node 'Macros' not found in '%s'", m_Form->getFilename().c_str());
		return false;
	}
	uint size;
	if (!elm->getArraySize(size))
	{
		nlwarning("Array size of node 'Macros' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
		return false;
	}
	for (uint i = 0; i < size; ++i)
	{
		NLGEORGES::UFormElm *macro;
		if (!elm->getArrayNode(&macro, i))
		{
			nlwarning("Array node of node 'Macros' at '%i' not found in '%s'", i, m_Form->getFilename().c_str());
			return false;
		}
		std::string macroName;
		if (!macro->getValueByName(macroName, "Name"))
		{
			nlwarning("Macro does not contain value 'Name' at '%i' in '%s'", i, m_Form->getFilename().c_str());
			return false;
		}
		if (macroName == name)
		{
			std::string macroValue;
			if (!macro->getValueByName(macroValue, "Value"))
			{
				nlwarning("Macro does not contain value 'Value' at '%i' in '%s'", i, m_Form->getFilename().c_str());
				return false;
			}
			parseValue(result, macroValue);
			return true;
		}
	}

	nlwarning("Macro '%s' not found in '%s'", name.c_str(), m_Form->getFilename().c_str());
	result = std::string("[&") + name + std::string("]");
	return false;
}

std::string CPipelineProject::getName()
{
	return NLMISC::CFile::getFilenameWithoutExtension(m_Form->getFilename());
}


std::string CPipelineProject::getOutputDirectory()
{
	// return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_PROJECT + getName() + "/";
	return CWorkspaceStorage::getProjectDirectory(getName());
}

std::string CPipelineProject::getTempDirectory()
{
	if (m_TempDirectory.empty())
	{
		std::stringstream ss;
		ss << g_WorkDir;
		ss << PIPELINE_DIRECTORY_PREFIX_PROJECT;
		ss << getName();
		ss << ".";
		ss << NLMISC::CTime::getSecondsSince1970();
		ss << ".";
		ss << rand();
		ss << PIPELINE_DIRECTORY_TEMP_SUFFIX;
		ss << "/";
		NLMISC::CFile::createDirectoryTree(ss.str());
		m_TempDirectory = ss.str();
	}
	return m_TempDirectory;
}

void CPipelineProject::parseValue(std::string &result, const std::string &value)
{
	std::stringstream ss;

	std::string::const_iterator lastEndPP = value.begin();
	std::string::const_iterator findOpen = find(lastEndPP, value.end(), '[');
	ss << std::string(lastEndPP, findOpen);
	while (findOpen != value.end())
	{
		++findOpen;
		switch (*findOpen)
		{
		case '$': // SERVICE CONFIGURATION VALUE
			{
				++findOpen;
				lastEndPP = find(findOpen, value.end(), ']');
				std::string tagName = std::string(findOpen, lastEndPP);
				if (NLNET::IService::getInstance()->ConfigFile.exists(tagName))
				{
					std::string cfgValue = NLNET::IService::getInstance()->ConfigFile.getVar(tagName).asString();
					ss << cfgValue;
				}
				else
				{
					nlwarning("Unknown service configuration value '%s' in '%s'", tagName.c_str(), m_Form->getFilename().c_str());
					ss << "[$";
					ss << tagName;
					ss << "]";
				}
				++lastEndPP;
			}
			break;
		case '!': // SPECIAL PROJECT VALUE
			{
				++findOpen;
				lastEndPP = find(findOpen, value.end(), ']');
				std::string tagName = std::string(findOpen, lastEndPP);
				if (tagName == "OutputDirectory")
				{
					ss << getOutputDirectory();
				}
				else if (tagName == "TempDirectory")
				{
					ss << getTempDirectory();
				}
				else
				{
					nlwarning("Unknown special project value '%s' in '%s'", tagName.c_str(), m_Form->getFilename().c_str());
					ss << "[!";
					ss << tagName;
					ss << "]";
				}
				++lastEndPP;
			}
			break;
		case '&': // PROJECT MACRO VALUE
			{
				++findOpen;
				lastEndPP = find(findOpen, value.end(), ']');
				std::string tagName = std::string(findOpen, lastEndPP);
				std::string macroValue;
				getMacro(macroValue, tagName);
				ss << macroValue;
				++lastEndPP;
			}
			break;
		case '@': // WORKSPACE PROJECT VALUE
			// TODO
			// break;
		case '#': // LEVELDESIGN SHEET VALUE
			// TODO
			// break;
		default:
			lastEndPP = find(findOpen, value.end(), ']');
			--findOpen;
			++lastEndPP;
			std::string unknownTag = std::string(findOpen, lastEndPP);
			ss << unknownTag;
			nlwarning("Unknown tag '%s'", unknownTag.c_str());
			break;
		}
		findOpen = find(lastEndPP, value.end(), '[');
		ss << std::string(lastEndPP, findOpen);
	}

	result = ss.str();
}

} /* namespace PIPELINE */

/* end of file */
