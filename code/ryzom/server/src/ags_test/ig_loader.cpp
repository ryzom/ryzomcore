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

#include "nel/misc/common.h"
#include <string>

#include "ig_loader.h"
#include "ig_list.h"

using namespace NL3D;
using namespace NLMISC;
using namespace std;

void CIGLoader::load()
{
	for (int i=sizeof(IGFiles)/sizeof(IGFiles[0]);i--;)
	{
		try
		{
			uint k;
			CInstanceGroup ig;
			CIFile inputStream;
			if (!inputStream.open(IGFiles[i]))
			{
				nlinfo("unable to open %s", argv[1]);
				return -1;
			}
			ig.serial(inputStream);
			CVector gpos = ig.getGlobalPos(); // the origin for the ig
			for(k = 0; k < ig._InstancesInfos.size(); ++k)
			{
				nlinfo("instance %s : x = %.1f, y = %.1f, z = %.1f, sx = %.1f, sy = %.1f, sz = %.1f", ig._InstancesInfos[k].Name.c_str(), ig._InstancesInfos[k].Pos.x + gpos.x, ig._InstancesInfos[k].Pos.y + gpos.y, ig._InstancesInfos[k].Pos.z + gpos.z, ig._InstancesInfos[k].Scale.x, ig._InstancesInfos[k].Scale.y, ig._InstancesInfos[k].Scale.z);
			}
		}
		catch (std::exception &e)
		{
			nlinfo(e.what());
		}
	}
}
