// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// DebuggerInterface.h: interface for the CDebuggerInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGGERMESSAGES_H__9940175C_4956_4E67_9B1B_0F7DD334F8E3__INCLUDED_)
#define AFX_DEBUGGERMESSAGES_H__9940175C_4956_4E67_9B1B_0F7DD334F8E3__INCLUDED_

#define _DMSG_FIRST_MSG			DMSG_WRITE_DEBUG
#define DMSG_WRITE_DEBUG			WM_USER+1
#define DMSG_HAS_BREAKPOINT			WM_USER+2
#define DMSG_GOTO_FILELINE			WM_USER+3
#define DMSG_DEBUG_START			WM_USER+4
#define DMSG_DEBUG_BREAK			WM_USER+5
#define DMSG_DEBUG_END				WM_USER+6
#define DMSG_CLEAR_STACKTRACE		WM_USER+7
#define DMSG_ADD_STACKTRACE			WM_USER+8
#define DMSG_GOTO_STACKTRACE_LEVEL	WM_USER+9
#define DMSG_GET_STACKTRACE_LEVEL	WM_USER+10
#define DMSG_CLEAR_LOCALVARIABLES	WM_USER+11
#define DMSG_ADD_LOCALVARIABLE		WM_USER+12
#define DMSG_CLEAR_GLOBALVARIABLES	WM_USER+13
#define DMSG_ADD_GLOBALVARIABLE		WM_USER+14
#define DMSG_REDRAW_WATCHES			WM_USER+15
#define _DMSG_LAST_MSG			DMSG_REDRAW_WATCHES

#endif // !defined(AFX_DEBUGGERMESSAGES_H__9940175C_4956_4E67_9B1B_0F7DD334F8E3__INCLUDED_)
