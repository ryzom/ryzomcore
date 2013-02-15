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

#include "std_afx.h"

#include "graph.h"



//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;

//
// Variables
//


bool			CGraph::Display = true;
bool			CGraph::DisplayAverageValue = true;



void CGraph::renderGraph ()
{


	// Display the background
	uint32 w, h;
	CNELU::Driver->getWindowSize (w, h);
	float ScreenWidth = (float) w;
	float ScreenHeight = (float) h;
	if (w == 0 || h == 0) return;
	float iw = 1.f / w;
	float ih = 1.f / h;
	NL3D::CViewport vp;	
	CDRU::drawQuad(X * iw, Y * ih, (X+Width) * iw, (Y+Height) * ih, *CNELU::Driver, BackColor, vp);

	Peak = 0.0f;
	float sum = 0.0f;

	CRGBA lineCol;
	if ( LineMode )
	{
		lineCol.set (BackColor.R, BackColor.G, BackColor.B, 255);
	}
	else
	{
		lineCol.set (255,255,255,BackColor.A);
	}

	float pos = X+Width-1;
	uint i = 0;
	for (deque<float>::reverse_iterator it = Values.rbegin(); it != Values.rend(); it++)
	{
		float value = (*it) * Height / MaxValue;
		if (value > Height) value = Height;

		CVector vect1;
		if ( LineMode )
		{
			vect1.x = pos-1;
			vect1.y = PrevY;
		}
		else
		{
			vect1.x = pos;
			vect1.y = Y;
		}
		PrevY = Y + value;

		CDRU::drawLine(vect1.x * iw, vect1.y * ih, pos * iw, PrevY * ih, *CNELU::Driver, lineCol);

		pos--;
		if ((*it) > Peak) Peak = *it;
		sum += *it;
		i++;
	}


	// Display max
	float value = Peak * Height / MaxValue;
	if (value > Height) value = Height;
	float peakval = Y+value;
	CRGBA frontCol (min(BackColor.R*2,255),min(BackColor.G*2,255),min(BackColor.B*2,255),min(BackColor.A*2,255));
	CDRU::drawLine(X * iw, peakval * ih, (X+Width) * iw, peakval * ih, *CNELU::Driver, frontCol);

	// Display average
	float average = sum / (float)Values.size();
	value = average * Height / MaxValue;
	if (value > Height) value = Height;
	float avrval = Y+value;
	CDRU::drawLine(X * iw, avrval * ih, (X+Width) * iw, avrval * ih, *CNELU::Driver, frontCol);

}


void CGraph::addOneValue (float value)
{
	if (value < 0.0f) value = 0.0f;

	Values.push_back (value);
	while (Values.size () > Width)
		Values.pop_front ();

//	if (Values.back() > Peak)
//		Peak = Values.back();
}


void CGraph::addValue (float value)
{
	TTime currentTime = (uint64) (1000 * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()));

	while (Values.size () == 0 || currentTime > CurrentQuantumStart + Quantum)
	{
		CurrentQuantumStart += Quantum;
		addOneValue ();
	}

	Values.back() += value;

//	if (Values.back() > Peak)
//		Peak = Values.back();
}
