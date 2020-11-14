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

#ifndef RY_ITEM_SERVICE_TYPE_H
#define RY_ITEM_SERVICE_TYPE_H

#include <string>

namespace ITEM_SERVICE_TYPE
{

	enum TItemServiceType
	{
		StableFeedAnimal1,
		StableFeedAnimal2,
		StableFeedAnimal3,
		StableFeedAnimal4,
		StableFeedAllAnimals,
		SpeedUpDPLoss,

		Unknown,
		NbItemServiceType = Unknown
	};

	TItemServiceType fromString(const std::string & str);
	const std::string & toString(TItemServiceType itemServiceType);

} // namespace ITEM_SERVICE_TYPE

#endif // RY_ITEM_SERVICE_TYPE_H
