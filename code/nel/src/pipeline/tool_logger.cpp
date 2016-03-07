/**
 * \file tool_logger.cpp
 * \brief CToolLogger
 * \date 2012-02-19 10:33GMT
 * \author Jan Boon (Kaetemi)
 * CToolLogger
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

#include "nel/pipeline/tool_logger.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

namespace NLPIPELINE {

// Tool logger is fully implemented in header so small tools do not need to link to this library unnecessarily.
void dummy_tool_logger_cpp() { }

} /* namespace NLPIPELINE */

/* end of file */
