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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "../world_editor/plugin_interface.h"
#include <nel/3d/text_context.h>



class CDialogFlags;

class CPlugin : public IPluginCallback,
                public IPrimitiveDisplayer
{
public:	
	CPlugin();
	~CPlugin();
	// from IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);			
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root) {}
	virtual void		positionMoved(const NLMISC::CVector &position) {}
	virtual void		lostPositionControl() {}	
	virtual void		onIdle() {}	
	virtual void		postRender(CDisplay &display);	
	virtual std::string&	getName();	
	virtual bool		isActive();
	virtual bool		activatePlugin();
	virtual bool		closePlugin();
	// from IPrimitiveDisplayer
	virtual void		drawPrimitive(const NLLIGO::IPrimitive *primitive, const TRenderContext &renderContext);
	IPluginAccess       *getPluginAccess() const { return _PluginAccess; }
	//
private:
	std::set<const NLLIGO::IPrimitive *> _FaunaGroups;
	bool								 _PluginActive;
	private:
	IPluginAccess						 *_PluginAccess;
	NL3D::CTextContext					 _TextContext;
	CPrimTexture						 *_FaunaFlagIcons;
	CDialogFlags						 *_DialogFlags;
private:	
	void drawArrow(CDisplay &display, const NLMISC::CVector &start, const NLMISC::CVector &end);
	void drawFaunaGraph(CDisplay &display, const NLLIGO::IPrimitive &grp);
	void pushIcon(CDisplay &display, sint stepX, sint stepY, NLMISC::CVector &currPos, uint srcX, uint srcY, const CPrimTexture &pt);
};




extern "C"
{
	/// Export the C factory method for dynamic linking..
	__declspec( dllexport ) void *createPlugin();
}

















#endif