/**
 * \file process_plugin.h
 * \brief IProcessPlugin
 * \date 2012-02-25 10:19GMT
 * \author Jan Boon (Kaetemi)
 * IProcessPlugin
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

#ifndef PIPELINE_PROCESS_PLUGIN_H
#define PIPELINE_PROCESS_PLUGIN_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_registry.h>

// Project includes

namespace PIPELINE {

/**
 * \brief IProcessPlugin
 * \date 2012-02-25 10:19GMT
 * \author Jan Boon (Kaetemi)
 * /// REJECTED.A /// A process plugin handles process sheets with specified sheet extention.
 * /// REJECTED.A /// If more than one process plugin handles a sheet extention, all of them will be run.
 * A process sheet may define one or more process plugins to handle the process.
 * The master service may dispatch these seperately to different slave services to executa a single process sheets using multiple process plugins.
 * This enables creating seperate plugins for different file formats for the same process, to allow handling files from different modeling packages on different build servers.
 */
class IProcessPlugin : public NLMISC::IClassable
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	IProcessPlugin();
	virtual ~IProcessPlugin();
}; /* class IProcessPlugin */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PROCESS_PLUGIN_H */

/* end of file */
