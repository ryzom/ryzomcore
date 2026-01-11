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

#ifndef GRAPH_H
#define GRAPH_H

//
// Includes
//

#include <deque>
#include <string>

#include <nel/misc/rgba.h>
#include <nel/misc/time_nl.h>

namespace SBCLIENT {

//
// External classes
//

class CGraph
{
public:
	std::string Name;
	float X, Y, Width, Height;
	NLMISC::CRGBA BackColor;
	float MaxValue;
	float Peak;

	std::deque<float> Values;

	NLMISC::TTime Quantum;

	NLMISC::TTime CurrentQuantumStart;

	CGraph (std::string name, float x, float y, float width, float height, NLMISC::CRGBA backColor, NLMISC::TTime quantum, float maxValue)
		: Name(name), X(x), Y(y), Width(width), Height(height), BackColor(backColor), MaxValue(maxValue),
		Peak(0.0f), Quantum(quantum), CurrentQuantumStart(NLMISC::CTime::getLocalTime())
	{
	}

	void render ();
	void addOneValue (float value = 0.0f);
	void addValue (float value);
};

//
// External variables
//

extern CGraph FpsGraph;
extern CGraph SpfGraph;
extern CGraph DownloadGraph;
extern CGraph UploadGraph;

//
// External functions
//

void initGraph ();
void updateGraph ();
void releaseGraph ();

} /* namespace SBCLIENT */

#endif // GRAPH_H

/* End of graph.h */

/* end of file */
