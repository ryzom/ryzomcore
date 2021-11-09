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

#include "stdafx.h"
#include "../../include/lua_ide_dll/ide_interface.h"
#include "../ide2/ide2.h"
#include "../ide2/MainFrame.h"
#include "../ide2/WorkspaceWnd.h"
#include "../ide2/TreeViewFiles.h"
#include "../ide2/LuaView.h"
#include "../ide2/OutputWnd.h"
#include "../ide2/ScintillaView.h"
#include <string>

class CLuaIDEInterfaceImpl : public ILuaIDEInterface
{
public:
	CLuaIDEInterfaceImpl();
	~CLuaIDEInterfaceImpl();
	virtual void prepareDebug(const char *tmpProjectFile, lua_realloc_t reallocfunc, lua_free_t freefunc, HWND mainWnd);
	virtual void stopDebug();
	virtual void showDebugger(bool visible = true);
	virtual bool isBreaked();
	virtual void doMainLoop();
	virtual void addFile(const char *filename);	
	virtual lua_State *getLuaState();
	virtual void	   expandProjectTree();
	virtual void	   sortFiles();
	virtual void	   setBreakPoint(const char *name, int line);
	virtual void	   debugInfo(const char *msg);
	virtual void					setDebuggedAppMainLoop(IDebuggedAppMainLoop *mainAppLoop);
	virtual IDebuggedAppMainLoop	*setDebuggedAppMainLoop() const;
};

CLuaIDEInterfaceImpl::CLuaIDEInterfaceImpl()
{	
}

CLuaIDEInterfaceImpl::~CLuaIDEInterfaceImpl()
{	
}


void CLuaIDEInterfaceImpl::setDebuggedAppMainLoop(IDebuggedAppMainLoop *mainAppLoop)
{
	theApp.m_DebuggedAppMainLoop = mainAppLoop;
}

IDebuggedAppMainLoop *CLuaIDEInterfaceImpl::setDebuggedAppMainLoop() const
{
	return theApp.m_DebuggedAppMainLoop;
}


void CLuaIDEInterfaceImpl::prepareDebug(const char *tmpProjectFile, lua_realloc_t reallocfunc, lua_free_t freefunc, HWND mainWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->InitExternalDebug(tmpProjectFile, reallocfunc, freefunc);
	theApp.m_EmbeddingAppWnd = mainWnd;
}

void CLuaIDEInterfaceImpl::stopDebug()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->GetDebugger()->GetLuaHelper().Free();
	theApp.GetMainFrame()->ShowWindow(SW_HIDE);
}


void CLuaIDEInterfaceImpl::showDebugger(bool visible /*=true*/)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->ShowWindow(visible ? SW_MAXIMIZE: SW_HIDE);
}

bool CLuaIDEInterfaceImpl::isBreaked()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.GetMainFrame()->appMode == CMainFrame::modeDebugBreak;
}

void CLuaIDEInterfaceImpl::doMainLoop()
{	
	theApp.MainLoop();
}


void CLuaIDEInterfaceImpl::addFile(const char *filename)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CProject *proj = theApp.GetMainFrame()->GetProject();
	if (proj->GetProjectFile(filename))	
		return;

	theApp.GetMainFrame()->GetProject()->AddFile(filename);	
	CDocument *doc = theApp.OpenDocumentFile(filename);
	if (doc)
	{
		POSITION p = doc->GetFirstViewPosition();
		for (;;)
		{
			CView *v = doc->GetNextView(p);
			if (!v) break;
			theApp.GetMainFrame()->MDIActivate(v);
			BOOL maximized;
			CMDIChildWnd* child = theApp.GetMainFrame()->MDIGetActive(&maximized);
			if (child && (!maximized))
			{
				child->MDIMaximize();
			}
		}
	}
}

void CLuaIDEInterfaceImpl::setBreakPoint(const char *name, int line)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CProjectFile *pf = theApp.GetMainFrame()->GetProject()->GetProjectFile(name);
	if (pf)
	{		
		CLuaView *view = theApp.FindProjectFilesView(pf);
		if (view)
		{
			view->ToggleBreakPoint(line);
		}
	}
}

void CLuaIDEInterfaceImpl::expandProjectTree()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->GetWorkspaceWnd()->GetTreeViewFiles()->Expand();
}

void CLuaIDEInterfaceImpl::sortFiles()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->GetWorkspaceWnd()->GetTreeViewFiles()->Sort();
}

void CLuaIDEInterfaceImpl::debugInfo(const char *msg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.GetMainFrame()->GetOutputWnd()->GetOutput(COutputWnd::outputDebug)->Write((std::string(msg) +  "\n").c_str());
}



static CLuaIDEInterfaceImpl Impl;

LUA_IDE_API ILuaIDEInterface *GetLuaIDEInterface()
{
	return &Impl;
}

LUA_IDE_API int GetLuaIDEInterfaceVersion()
{
	return LUA_IDE_INTERFACE_VERSION;
}

lua_State *CLuaIDEInterfaceImpl::getLuaState()
{
	return theApp.GetMainFrame()->GetDebugger()->GetLuaState();
}






