/**
 * \file pipeline_service.h
 * \brief CPipelineService
 * \date 2012-02-18 17:25GMT
 * \author Jan Boon (Kaetemi)
 * CPipelineService
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

#ifndef PIPELINE_PIPELINE_SERVICE_H
#define PIPELINE_PIPELINE_SERVICE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>

// NeL includes

// Project includes

namespace PIPELINE {

extern bool g_IsMaster;
extern std::string g_DatabaseDirectory;
extern std::string g_PipelineDirectory;

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_SERVICE_H */

/* end of file */
