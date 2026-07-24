/**
 * \file flow_group.cpp
 * \brief zp_flow: auto-wrapping fixed-size tile flow group
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The Ryzom client inventory grid (interface_v3 dbgroup_list_sheet) is the reference
 * mechanism: the column count derives from the group's CURRENT width inside
 * updateCoords, and the EXISTING children reflow into grid slots, with no repopulation and
 * no frame-loop width watch. A window resize invalidates coords and the next layout
 * pass re-wraps naturally. That widget lives in the client, not NLGUI, so this is the
 * minimal standalone equivalent for fixed-size tiles.
 *
 * XML: <group type="zp_flow" tile_w="178" tile_h="106" sizeref="w" w="0" ... />
 * children (spawned TL-TL, e.g. via the board-cell spawnUnder idiom) are laid out in
 * add order, left→right then top→bottom; the group sets its own height to the row
 * count so a stacking CGroupList (scroll_text body) flows around it.
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

#include <algorithm>

#include <libxml/parser.h>

#include <nel/gui/interface_group.h>
#include <nel/misc/class_registry.h>
#include <nel/misc/factory.h>
#include <nel/misc/xml_auto_ptr.h>

namespace ZPUI {

using NLGUI::CViewBase; // DECLARE_UI_CLASS expands an unqualified CViewBase::TCtorParam

class CZPGroupFlow : public NLGUI::CInterfaceGroup
{
public:
	DECLARE_UI_CLASS(CZPGroupFlow)

	CZPGroupFlow(const TCtorParam &param)
		: NLGUI::CInterfaceGroup(param), _TileW(178), _TileH(106)
	{
	}

	virtual bool parse(xmlNodePtr cur, NLGUI::CInterfaceGroup *parentGroup)
	{
		if (!NLGUI::CInterfaceGroup::parse(cur, parentGroup))
			return false;
		CXMLAutoPtr prop((const char *)xmlGetProp(cur, (xmlChar *)"tile_w"));
		if (prop)
			NLMISC::fromString((const char *)prop, _TileW);
		prop = (char *)xmlGetProp(cur, (xmlChar *)"tile_h");
		if (prop)
			NLMISC::fromString((const char *)prop, _TileH);
		if (_TileW < 1) _TileW = 1;
		if (_TileH < 1) _TileH = 1;
		return true;
	}

	virtual void updateCoords()
	{
		// Columns from the current width (last layout pass's real size when sizeref'd,
		// same one-frame convergence as the client's inventory grid), then reflow the
		// existing children into grid slots and size the group to the row count.
		sint32 avail = getWReal();
		if (avail <= 0 && getParent())
			avail = getParent()->getWReal();
		const sint32 cols = std::max((sint32)1, avail / _TileW);
		sint32 i = 0;
		const std::vector<NLGUI::CInterfaceGroup *> &kids = getGroups();
		for (size_t k = 0; k < kids.size(); ++k)
		{
			if (!kids[k] || !kids[k]->getActive())
				continue;
			kids[k]->setX((i % cols) * _TileW);
			kids[k]->setY(-((i / cols) * _TileH));
			++i;
		}
		const sint32 rows = (i + cols - 1) / cols;
		setH(std::max(rows, (sint32)1) * _TileH);
		NLGUI::CInterfaceGroup::updateCoords();
	}

private:
	sint32 _TileW;
	sint32 _TileH;
};

NLMISC_REGISTER_OBJECT(NLGUI::CViewBase, CZPGroupFlow, std::string, "zp_flow");
REGISTER_UI_CLASS(CZPGroupFlow)

} // namespace ZPUI

/* end of file */
