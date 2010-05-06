
#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <winsock2.h>
#	include <windows.h>
#endif // NL_OS_WINDOWS

#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>

#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <utility>
#include <cstdlib>
#include <algorithm>
#include <exception>

#include "nel/misc/debug.h"

#include "nel/misc/common.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/hierarchical_timer.h"
