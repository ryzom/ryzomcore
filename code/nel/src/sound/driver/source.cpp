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

#include "stdsound_lowlevel.h"

#include "nel/sound/driver/source.h"

using namespace NLMISC;

namespace NLSOUND
{

// common method used only with OptionManualRolloff. return the volume in 1/100th DB ( = mB) modified
sint32 ISource::computeManualRollOff(sint32 volumeMB, sint32 mbMin, sint32 mbMax, double alpha, float sqrdist, float distMin, float distMax)
{
	if (sqrdist < distMin * distMin)
	{
		// no attenuation
		return volumeMB;
	}
	else if (sqrdist > distMax * distMax)
	{
		// full attenuation
		return mbMin;
	}
	else
	{
		double dist = (double) sqrt(sqrdist);

		// linearly descending volume on a dB scale
		double db1 = mbMin * (dist - distMin) / (distMax - distMin);

		if (alpha == 0.0) {
			volumeMB += (sint32) db1;

		} else if (alpha > 0.0) {
			double amp2 = 0.0001 + 0.9999 * (distMax - dist) / (distMax - distMin); // linear amp between 0.00001 and 1.0
			double db2 = 2000.0 * log10(amp2); // convert to 1/100th decibels
			volumeMB += (sint32) ((1.0 - alpha) * db1 + alpha * db2);

		} else if (alpha < 0.0) {
			double amp3 = distMin / dist; // linear amplitude is 1/distance
			double db3 = 2000.0 * log10(amp3); // convert to 1/100th decibels
			volumeMB += (sint32) ((1.0 + alpha) * db1 - alpha * db3);
		}
		
		clamp(volumeMB, mbMin, mbMax);
		return volumeMB;
	}
}

// common method used only with OptionManualRolloff. return the rolloff in amplitude ratio (gain)
float ISource::computeManualRolloff(double alpha, float sqrdist, float distMin, float distMax)
{
	static const sint32 mbMin = -10000;
	static const sint32 mbMax = 0;
	sint32 rolloffMb = ISource::computeManualRollOff(mbMax, mbMin, mbMax, alpha, sqrdist, distMin, distMax);
	float rolloffGain = (float)pow(10.0, (double)rolloffMb / 2000.0);
	clamp(rolloffGain, 0.0f, 1.0f);
	return rolloffGain;
}

} // NLSOUND

/* end of file */
