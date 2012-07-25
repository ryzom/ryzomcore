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

#ifndef PRIMITIVE_PLUGIN_H
#define PRIMITIVE_PLUGIN_H

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <nel/misc/types_nl.h>

#undef min
#undef max

#include <nel/misc/sheet_id.h>
#include <nel/georges/u_form.h>

#include "../world_editor/plugin_interface.h"

class CPrimitivePlugin : public IPluginCallback, public IPrimitiveDisplayer
{
public:
	
	virtual bool		isActive();

	virtual bool		activatePlugin();

	virtual bool		closePlugin();

	virtual std::string& getName();

	CPrimitivePlugin();

	virtual void		positionMoved(const NLMISC::CVector &position) {}
	virtual void		lostPositionControl() {}
	virtual void		onIdle() {}

	virtual void drawPrimitive(const NLLIGO::IPrimitive *primitive, const TRenderContext &renderContext);


private:

	struct TCreatureInfo
	{
		bool	HaveRadius;
		float	Radius;
		bool	HaveBox;
		float	Width;
		float	Length;

		void	readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

		void	serial (NLMISC::IStream &s);

		static uint getVersion ();

		void removed() {}
	};

	std::map<NLMISC::CSheetId, TCreatureInfo>	_CreatureInfos;

	// @{
	// \name Overload for IPluginCallback
	virtual void		init(IPluginAccess *pluginAccess);

	/// The current region has changed.
	virtual void		primitiveChanged(const NLLIGO::IPrimitive *root);
	// @}

	bool	_PluginActive;

	bool	m_Initialized;

	// The plugin access
	IPluginAccess	*_PluginAccess;

};

extern "C"
{
	/// Export the C factory method for dynamic linking..
	__declspec( dllexport ) void *createPlugin();
}


#endif
