#include <nel/misc/types_nl.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
//#include <sstream>
#include <exception>
#include <utility>
#include <deque>
#include <fstream>

#include <nel/misc/common.h>
#include <nel/misc/debug.h>

#include <nel/misc/stream.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/path.h>
#include <nel/misc/rgba.h>
#include <nel/misc/log.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/sheet_id.h>

#ifdef NL_OS_WINDOWS
#define NOMINMAX
#include	<WinSock2.h>
#include	<Windows.h>
#endif // NL_OS_WINDOWS
