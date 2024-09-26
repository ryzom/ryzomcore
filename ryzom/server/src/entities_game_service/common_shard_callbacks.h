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



#ifndef RY_COMMON_SHARD_CALLBACKS_H
#define RY_COMMON_SHARD_CALLBACKS_H



/**
 * class used to manage net callbacks that are not linked to a particular game manager
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CCommonShardCallbacks
{
public:

	static void init();
	static void release();
};


#endif // RY_COMMON_SHARD_CALLBACKS_H

/* End of common_shard_callbacks.h */
