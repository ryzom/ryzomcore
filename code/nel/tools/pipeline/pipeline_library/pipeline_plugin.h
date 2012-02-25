/**
 * \file pipeline_plugin.h
 * \brief IPipelinePlugin
 * \date 2012-02-25 10:27GMT
 * \author Jan Boon (Kaetemi)
 * IPipelinePlugin
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

#ifndef PIPELINE_PIPELINE_PLUGIN_H
#define PIPELINE_PIPELINE_PLUGIN_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace PIPELINE {

/**
 * \brief IPipelinePlugin
 * \date 2012-02-25 10:27GMT
 * \author Jan Boon (Kaetemi)
 * A pipeline plugin (dll) exposes multiple process plugins.
 */
class IPipelinePlugin
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	IPipelinePlugin();
	virtual ~IPipelinePlugin();
}; /* class IPipelinePlugin */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PIPELINE_PLUGIN_H */

/* end of file */
