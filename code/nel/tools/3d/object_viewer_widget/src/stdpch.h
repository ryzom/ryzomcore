/*
Object Viewer Qt Widget
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef NL_STDPCH_H
#define NL_STDPCH_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#undef realloc
	#undef free
#endif

#include <QtCore/QtCore>
#include <QtGui/QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define USE_QT5
#endif

#ifdef USE_QT5
#include <QtWidgets/QtWidgets>
#include <QtConcurrent/QtConcurrent>
#endif

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>

#if defined(NL_OS_WINDOWS)
#define NOMINMAX
#include <Windows.h>
#elif defined(NL_OS_MAC)
#else
#include <X11/Xlib.h>
#endif

#endif
