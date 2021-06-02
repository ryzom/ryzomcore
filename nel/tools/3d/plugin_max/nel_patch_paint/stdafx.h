#pragma warning (disable : 4786)

// max.h include uses min/max in 3dsmax 2010 sdk
#include <assert.h>
#include "Max.h"
#ifdef min
#	undef min
#endif
#ifdef max
#	undef max
#endif

#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include "locale.h"
#include "dllentry.h"
#include "decomp.h"
#include "buildver.h"
#include <iparamb2.h>
