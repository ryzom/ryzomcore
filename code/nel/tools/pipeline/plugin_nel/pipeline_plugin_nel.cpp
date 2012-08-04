/**
 * \file pipeline_plugin_nel.cpp
 * \brief CPipelinePluginNeL
 * \date 2012-03-03 10:09GMT
 * \author Jan Boon (Kaetemi)
 * CPipelinePluginNeL
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
#include "pipeline_plugin_nel.h"

// STL includes
#ifdef NL_OS_WINDOWS
#	include <windows.h>
#endif

// NeL includes
#include "nel/misc/dynloadlib.h"
#include "nel/misc/debug.h"

// Project includes
#include "../plugin_library/pipeline_interface.h"
#include "process_interface.h"
#include "process_texture_dds.h"
#include "process_package_bnp.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

// ******************************************************************

class CPipelinePluginNeLNelLibrary : public NLMISC::INelLibrary 
{ 
	void onLibraryLoaded(bool /* firstTime */) 
	{
		nldebug("Library loaded: CPipelinePluginNeL");
		PIPELINE_REGISTER_CLASS(CProcessInterface);
		PIPELINE_REGISTER_CLASS(CProcessInterfaceInfo);
		PIPELINE_REGISTER_CLASS(CProcessTextureDDS);
		PIPELINE_REGISTER_CLASS(CProcessTextureDDSInfo);
		PIPELINE_REGISTER_CLASS(CProcessPackageBNP);
		PIPELINE_REGISTER_CLASS(CProcessPackageBNPInfo);
	} 
	void onLibraryUnloaded(bool /* lastTime */) 
	{ 
		nldebug("Library unloaded: CPipelinePluginNeL"); 
	}  
};
NLMISC_DECL_PURE_LIB(CPipelinePluginNeLNelLibrary)

#ifdef NL_OS_WINDOWS
HINSTANCE CPipelinePluginNeLDllHandle = NULL;
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
	CPipelinePluginNeLDllHandle = (HINSTANCE)hModule;
	return TRUE;
}
#endif

// ******************************************************************

/*
CPipelinePluginNeL::CPipelinePluginNeL()
{
	
}

CPipelinePluginNeL::~CPipelinePluginNeL()
{
	
}
*/

} /* namespace PIPELINE */

/* end of file */
