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

// LuaHelper.h: interface for the CLuaHelper class.
//
//////////////////////////////////////////////////////////////////////

extern "C" 
{
	#include "lua.h" 
} 


#if !defined(AFX_LUAHELPER_H__FE39E254_5793_42CE_B1CA_C38E7437E9AD__INCLUDED_)
#define AFX_LUAHELPER_H__FE39E254_5793_42CE_B1CA_C38E7437E9AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct lua_State;
struct Proto;
struct lua_Debug;

class CLuaHelper  
{
public:
	void SetExternalDebugging() { m_ExternalDebugging = true; }
	void RestoreGlobals();
	void CoverGlobals();
	void Describe(char* szRet, int nIndex);
	BOOL Eval(const char* szCode, char* szRet);
	BOOL CheckSyntax(const CString &code, CString &errors);
	BOOL GetCalltip(const char *szWord, char *szCalltip);
	void DrawGlobalVariables();
	void DrawLocalVariables();
	const char* GetSource();

	static BOOL ErrorStringToFileLine(CString strError, CString &strPathName, int &nLine);
	static BOOL LoadDebugLines(CProjectFile* pPF);

	CLuaHelper();
	virtual ~CLuaHelper();

// debugger functions
	BOOL PrepareDebugger(lua_realloc_t reallocfunc, lua_free_t freefunc);
	BOOL StartDebugger();	
	void StopDebugger();

	void DrawStackTrace();

	lua_State* GetState() { return L; };

	void Free();

protected:
	static CLuaHelper* m_pThis;

	static int OutputTop(lua_State* L);
	static int errormessage(lua_State* L);
	static int lua_loadlib(lua_State* L);
	static void line_hook (lua_State *L, lua_Debug *ar);
	static void func_hook (lua_State *L, lua_Debug *ar);
	static void hook (lua_State *L, lua_Debug *ar);

	lua_State* L;
	lua_Debug* m_pAr;
	HMODULE m_hLoaded[8192];
	int m_nLoaded;

	bool m_ExternalDebugging;
};

#endif // !defined(AFX_LUAHELPER_H__FE39E254_5793_42CE_B1CA_C38E7437E9AD__INCLUDED_)
