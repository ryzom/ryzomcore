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

#include "nel/misc/types_nl.h"

#include <nel/misc/rgba.h>
#include <nel/misc/time_nl.h>

#include <deque>
#include <string>



/**
 * Graph class for network statistics
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CGraph
{
public:
	std::string Name;
	float X, Y, Width, Height;
	NLMISC::CRGBA BackColor;
	float MaxValue;
	float Peak;
	bool LineMode;
	float PrevY;	

	std::deque<float> Values;

	NLMISC::TTime Quantum;

	NLMISC::TTime CurrentQuantumStart;

		

	/// release material
	~CGraph()
	{	
	}


	/// Constructor (CGraph::init() must have been called before)
	CGraph (std::string name,
			float x, float y, float width, float height,
			NLMISC::CRGBA backColor,
			NLMISC::TTime quantum,
			float maxValue,
			bool lineMode = false)
		: Name(name), X(x), Y(y), Width(width), Height(height), BackColor(backColor), Quantum(quantum),
		MaxValue(maxValue), Peak(0.0f), LineMode(lineMode), PrevY(y)
	{
		CurrentQuantumStart = (uint64) (1000 * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()));		
	}

	/// Add one value
	void addOneValue (float value = 0.0f);

	/// Add value
	void addValue (float value);

	static bool					DisplayAverageValue;
	static bool					Display;

	void renderGraph ();	
	
private:
	
	
	
};



#endif // GRAPH_H

/* End of graph.h */
