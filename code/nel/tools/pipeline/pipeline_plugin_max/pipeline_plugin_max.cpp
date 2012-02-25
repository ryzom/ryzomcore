/**
 * \file pipeline_plugin_max.cpp
 * \brief CPipelinePluginMax
 * \date 2012-02-25 10:39GMT
 * \author Jan Boon (Kaetemi)
 * CPipelinePluginMax
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
#include "pipeline_plugin_max.h"

// STL includes

// NeL includes
#include "nel/misc/dynloadlib.h"
#include "nel/misc/debug.h"

// Project includes
#include "../pipeline_library/pipeline_interface.h"
#include "process_max_shape.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

// ******************************************************************

class CPipelinePluginMaxNelLibrary : public NLMISC::INelLibrary 
{ 
	void onLibraryLoaded(bool /* firstTime */) 
	{
		nldebug("Library loaded: CPipelinePluginMax");
		PIPELINE_REGISTER_CLASS(CProcessMaxShape);
	} 
	void onLibraryUnloaded(bool /* lastTime */) 
	{ 
		nldebug("Library unloaded: CPipelinePluginMax"); 
	}  
};
NLMISC_DECL_PURE_LIB(CPipelinePluginMaxNelLibrary)

HINSTANCE CPipelinePluginMaxDllHandle = NULL;
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
	CPipelinePluginMaxDllHandle = (HINSTANCE)hModule;
	return TRUE;
}

// ******************************************************************

CPipelinePluginMax::CPipelinePluginMax()
{
	
}

CPipelinePluginMax::~CPipelinePluginMax()
{
	
}

} /* namespace PIPELINE */

/* end of file */
