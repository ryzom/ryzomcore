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
#include "projectile_stats.h"


static NLMISC::TTime lastTime = 0;
static bool firstSample = true;
static uint numProjFired = 0;

const uint PROJ_STATS_REPORT_TIME = 60 * 1000;

void projStatsTime(NLMISC::TTime time)
{
	if (firstSample)
	{
		firstSample = false;
	}
	else
	{
		if (time - lastTime > PROJ_STATS_REPORT_TIME)
		{			
// The following removed for now by Sadge because it causes spam!
//			nlinfo("PROJECTILE_STATS : %d projectile fired in %.2f second -> %.2f projectile per second",
//				   (int) numProjFired, 
//				   (time - lastTime) / 1000.f,
//				   (1000.f * numProjFired) / (time - lastTime));
			lastTime = time;
			numProjFired = 0;
		}
	}
}


void projStatsIncrement()
{
	++ numProjFired;
}
