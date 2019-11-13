/* Precomp.h -- StdAfx
2013-11-12 : Igor Pavlov : Public domain */

#ifndef __7Z_PRECOMP_H
#define __7Z_PRECOMP_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#define _7ZIP_ST

#include "Compiler.h"
/* #include "7zTypes.h" */

#endif
