/**
 * \file dllmain.cpp
 * \brief Plugin DLL entry points for the Map Extender drop-in replacement. Pure Max SDK —
 * no NeL context needed (the modifier only replays stored data).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "map_extender.h"

#include <maxversion.h>

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID /* lpvReserved */)
{
	hInstance = hinstDLL;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	}
	return TRUE;
}

__declspec(dllexport) const TCHAR *LibDescription()
{
	return _T("Map Extender modifier (drop-in replacement, RYZOM CORE PIPELINE)");
}

__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

__declspec(dllexport) ClassDesc *LibClassDesc(int i)
{
	switch (i)
	{
	case 0: return GetMapExtenderDesc();
	default: return 0;
	}
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Deferred loading is fine: Max resolves the class on demand by Class_ID.
__declspec(dllexport) ULONG CanAutoDefer()
{
	return 1;
}

/* end of file */
