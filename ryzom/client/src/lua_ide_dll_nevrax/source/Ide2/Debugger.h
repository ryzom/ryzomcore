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

// Debugger.h: interface for the CDebugger class.
//
//////////////////////////////////////////////////////////////////////

extern "C" 
{
	#include "lua.h" 
} 


#if !defined(AFX_DEBUGGER_H__344DB359_4B39_4DAF_BF7D_B2BFE030E7DB__INCLUDED_)
#define AFX_DEBUGGER_H__344DB359_4B39_4DAF_BF7D_B2BFE030E7DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "LuaHelper.h"




class CLuaEditor;
class CProject;

#include "DebuggerMessages.h"

#define DMOD_NONE					0
#define DMOD_STEP_INTO				1
#define DMOD_STEP_OVER				2
#define DMOD_STEP_OUT				3
#define DMOD_RUN_TO_CURSOR			4

#define DMOD_BREAK					10
#define DMOD_STOP					11

typedef struct
{
	const char* szDesc;
	const char* szFile;
	int nLine;
} StackTrace;

typedef struct
{
	const char* szName;
	const char* szType;
	const char* szValue;
} Variable;

class CDebugger  
{
public:
	void SetExternalDebugging();
	BOOL IsExternalDebugging() const { return m_ExternalDebugging; }
	void Execute();
	CString Eval(CString strCode);
	static void EndThread();
	BOOL GetCalltip(const char* szWord, char* szCalltip);
	void AddLocalVariable(const char* name, const char* type, const char* value);
	void ClearLocalVariables();
	void AddGlobalVariable(const char* name, const char* type, const char* value);
	void ClearGlobalVariables();
	void StackLevelChanged();
	void Break();
	void Stop();
	void DebugBreak(const char* szFile, int nLine);
	void LineHook(const char* szFile, int nLine);
	void FunctionHook(const char* szFile, int nLine, BOOL bCall);
	void Write(const char* szMsg);
	BOOL Start();
	BOOL Prepare(lua_realloc_t reallocfunc, lua_free_t freefunc);
	CDebugger();
	virtual ~CDebugger();

	void Go();
	void StepInto();
	void StepOver();
	void StepOut();
	void RunToCursor();

	void ClearStackTrace();
	void AddStackTrace(const char* strDesc, const char* strFile, int nLine);
	int GetStackTraceLevel();

	static CDebugger* GetDebugger() { return m_pDebugger; };
	lua_State* GetLuaState() { return m_lua.GetState(); };
	CLuaHelper &GetLuaHelper() { return m_lua; };

	HWND GetMainWnd();

	int GetMode() const { return m_nMode; }
protected:
	static UINT StartDebugger( LPVOID pParam );	
	UINT StartDebugger();	

	CLuaHelper m_lua;
	HWND m_hWndMainFrame;
	CEvent m_event;
	
	bool   m_monoThreadEvent;

	int m_nMode;
	CString m_strPathName;
	int m_nLine, m_nLevel;
	CWinThread* m_pThread;

	bool m_ExternalDebugging;

	bool m_EvaluatingCondition;

	void ResetEvent();
	void SetEvent();

	static CDebugger* m_pDebugger;
};

#endif // !defined(AFX_DEBUGGER_H__344DB359_4B39_4DAF_BF7D_B2BFE030E7DB__INCLUDED_)
