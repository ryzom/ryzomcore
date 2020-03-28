// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "nel/3d/scene_group.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"
#include <string>
#include "stdio.h"

using namespace NL3D;
using namespace NLMISC;
using namespace std;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("usage : %s file.ig\n", argv[0]);
		return -1;
	}
	try
	{
		uint k;
		CInstanceGroup ig;
		CIFile inputStream;
		if (!inputStream.open(string(argv[1])))
		{
			printf("unable to open %s\n", argv[1]);
			return -1;
		}
		ig.serial(inputStream);
		printf("Origine\n");
		printf("---------\n");
		CVector gpos = ig.getGlobalPos();
		printf("global pos : x = %.1f, y = %.1f, z =%.1f\n", gpos.x, gpos.y, gpos.z);
		printf("Instances\n");
		printf("---------\n");
		for(k = 0; k < ig._InstancesInfos.size(); ++k)
		{
			printf("instance %s : x = %.1f, y = %.1f, z = %.1f, sx = %.1f, sy = %.1f, sz = %.1f\n", ig._InstancesInfos[k].Name.c_str(), ig._InstancesInfos[k].Pos.x + gpos.x, ig._InstancesInfos[k].Pos.y + gpos.y, ig._InstancesInfos[k].Pos.z + gpos.z, ig._InstancesInfos[k].Scale.x, ig._InstancesInfos[k].Scale.y, ig._InstancesInfos[k].Scale.z);
		}
		printf("\n");
		printf("Lights\n");
		printf("---------\n");
		for(k = 0; k < ig.getNumPointLights(); ++k)
		{
			const CPointLightNamed &pl = ig.getPointLightNamed(k);
			printf("light group = %d, anim = \"%s\" x = %.1f, y = %.1f, z = %.1f\n", pl.LightGroup, pl.AnimatedLight.c_str(), pl.getPosition().x + gpos.x, pl.getPosition().y + gpos.y, pl.getPosition().z + gpos.z);
		}
		printf("---------\n");
		printf("Realtime sun contribution = %s\n", ig.getRealTimeSunContribution() ? "on" : "off");
		printf("---------\n");
		// IGSurfaceLight info.
		const CIGSurfaceLight::TRetrieverGridMap	&rgm= ig.getIGSurfaceLight().getRetrieverGridMap();
		printf("IGSurfaceLighting: CellSize: %f. NumGridRetriever: %u\n", 
			ig.getIGSurfaceLight().getCellSize(), (uint)rgm.size() );
		uint	rgmInst= 0;
		uint	totalCells= 0;
		CIGSurfaceLight::TRetrieverGridMap::const_iterator	it= rgm.begin();
		for(;it!=rgm.end();it++)
		{
			for(uint i=0;i<it->second.Grids.size();i++)
			{
				printf("grid(%d, %d): %dx%d\n", rgmInst, i, it->second.Grids[i].Width, it->second.Grids[i].Height );
				totalCells+= it->second.Grids[i].Cells.size();
			}
			rgmInst++;
		}
		printf("TotalCells: %d\n", totalCells);

	}
	catch (const std::exception &e)
	{
		printf("%s\n", e.what());
	}
}
