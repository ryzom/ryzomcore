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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef _NLGEORGES_SNDDOCPLUG_H
#define _NLGEORGES_SNDDOCPLUG_H

#include "std_sound_plugin.h"
#include "../georges_dll/plugin_interface.h"
#include "sound_plugin.h"
#include "nel/misc/vector.h"


namespace NLGEORGES
{

class CSoundPlugin;

class CSoundDocumentPlugin : public IEditDocumentPlugin
{
public:
	CSoundDocumentPlugin(CSoundPlugin *plugin, IEditDocument *document) : _Plugin(plugin), _Document(document), _Filename() {}
	virtual ~CSoundDocumentPlugin() {}

	virtual void		dialogInit(HWND documentView);
	virtual bool		pretranslateMessage(MSG *pMsg);
	virtual void		activate(bool activated);
	virtual void		onValueChanged(const char *formName);
	virtual void		onNodeChanged();

private:

	void				updateInfo(bool stopSound);

	CSoundPlugin		*_Plugin;
	IEditDocument		*_Document;

	std::string			_Filename;
	uint32				_InnerAngle;
	uint32				_OuterAngle;
	bool				_Loop;
	sint32				_Gain;
	sint32				_ExternalGain;
	sint32				_Transpose;
};


} // namespace NLGEORGES

#endif // _NLGEORGES_SNDDOCPLUG_H