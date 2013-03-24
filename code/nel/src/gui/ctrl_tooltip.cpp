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


#include "stdpch.h"
#include "nel/gui/ctrl_tooltip.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

NLMISC_REGISTER_OBJECT(CViewBase, CCtrlToolTip, std::string, "tooltip");

REGISTER_UI_CLASS(CCtrlToolTip)

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	void CCtrlToolTip::draw ()
	{
	}

	// ----------------------------------------------------------------------------
	bool CCtrlToolTip::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (CCtrlBase::handleEvent(event)) return true;
		return false;
	}


	xmlNodePtr CCtrlToolTip::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "tooltip" );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CCtrlToolTip::parse(xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if (!CCtrlBase::parse(cur, parentGroup)) return false;
		return true;
	}

	// ----------------------------------------------------------------------------
	void CCtrlToolTip::serial(NLMISC::IStream &f)
	{
		CCtrlBase::serial(f);
	}

}

