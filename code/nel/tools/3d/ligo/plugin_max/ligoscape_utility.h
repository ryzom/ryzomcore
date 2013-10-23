/**********************************************************************
 *<
	FILE: ligoscape_utility.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __PLUGIN_MAX__H
#define __PLUGIN_MAX__H

#include <assert.h>
#include "Max.h"
#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>

#include <utilapi.h>


#undef min
#undef max

#include "resource.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __PLUGIN_MAX__H
