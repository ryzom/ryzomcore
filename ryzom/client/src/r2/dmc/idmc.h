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

#ifndef DMC_IDMC_H
#define DMC_IDMC_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>
namespace R2
{
	class IDynamicMapClient
	{
	public:
		virtual ~IDynamicMapClient(){}
		// request commands
		virtual void doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value) = 0;
		virtual void doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value) = 0;
		virtual void doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position) = 0;
		virtual void doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition) = 0;
		//
		virtual CObject *find(const std::string& instanceId, const std::string& attrName = "", sint32 position = -1, const std::string &key ="") = 0;
	};
} //namespace DMS

#endif //MC_IDMC_H
