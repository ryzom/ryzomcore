/**
 * \file pipeline_interface.h
 * \brief IPipelineInterface
 * \date 2012-02-25 12:10GMT
 * \author Jan Boon (Kaetemi)
 * IPipelineInterface
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

#ifndef PIPELINE_PIPELINE_INTERFACE_H
#define PIPELINE_PIPELINE_INTERFACE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_registry.h>

// Project includes

namespace NLMISC {
	class CConfigFile;
}

namespace PIPELINE {

/**
 * \brief IPipelineInterface
 * \date 2012-02-25 12:10GMT
 * \author Jan Boon (Kaetemi)
 * IPipelineInterface
 */
class IPipelineInterface
{
public:
	IPipelineInterface() { }
	virtual ~IPipelineInterface() { }

	static IPipelineInterface *getInstance();

	virtual NLMISC::CConfigFile &getConfigFile() = 0;
	virtual void registerClass(const std::string &className, NLMISC::IClassable* (*creator)(), const std::string &typeidCheck) throw(NLMISC::ERegistry) = 0;

}; /* class IPipelineInterface */

#define	PIPELINE_REGISTER_CLASS(_class_) PIPELINE::IPipelineInterface::getInstance()->registerClass(#_class_, _class_::creator, typeid(_class_).name());

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_INTERFACE_H */

/* end of file */
