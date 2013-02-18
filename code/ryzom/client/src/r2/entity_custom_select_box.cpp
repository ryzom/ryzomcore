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
//
#include "entity_custom_select_box.h"
#include "nel/gui/lua_object.h"
//
#include "nel/misc/vector.h"

using namespace NLMISC;

namespace R2
{

// *********************************************************************************************************
void CEntityCustomSelectBox::toTable(CLuaObject &table)
{
	//H_AUTO(R2_CEntityCustomSelectBox_toTable)
	if (!table.isTable())
	{
		nlwarning("trying to store bbox in an object that is not a table");
		return;
	}
	table.setValue("Enabled", Enabled);
	//
	table.setValue("XMin", (double) Box.getMin().x);
	table.setValue("YMin", (double) Box.getMin().y);
	table.setValue("ZMin", (double) Box.getMin().z);
	//
	table.setValue("XMax", (double) Box.getMax().x);
	table.setValue("YMax", (double) Box.getMax().y);
	table.setValue("ZMax", (double) Box.getMax().z);


}

// *********************************************************************************************************
void CEntityCustomSelectBox::fromTable(CLuaObject &table)
{
	//H_AUTO(R2_CEntityCustomSelectBox_fromTable)
	if (!table.isTable())
	{
		nlwarning("trying to rerieve bbox from an object that is not a table");
		return;
	}
	Enabled = table["Enabled"].toBoolean();
	Box.setMinMax(CVector((float) table["XMin"].toNumber(),
		                  (float) table["YMin"].toNumber(),
						  (float) table["ZMin"].toNumber()),
				  CVector((float) table["XMax"].toNumber(),
		                  (float) table["YMax"].toNumber(),
						  (float) table["ZMax"].toNumber()));
}

} // R2
