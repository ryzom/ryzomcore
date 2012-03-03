/**
 * \file pipeline_process_impl.h
 * \brief CPipelineProcessImpl
 * \date 2012-03-03 09:33GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProcessImpl
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

#ifndef PIPELINE_PIPELINE_PROCESS_IMPL_H
#define PIPELINE_PIPELINE_PROCESS_IMPL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../plugin_library/pipeline_process.h"

namespace PIPELINE {

/**
 * \brief CPipelineProcessImpl
 * \date 2012-03-03 09:33GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineProcessImpl
 */
class CPipelineProcessImpl : public IPipelineProcess
{
public:
	CPipelineProcessImpl();
	virtual ~CPipelineProcessImpl();

	virtual std::string getProjectValue(const std::string &name);
	virtual std::string getTempDir();
}; /* class CPipelineProcessImpl */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PROCESS_IMPL_H */

/* end of file */
