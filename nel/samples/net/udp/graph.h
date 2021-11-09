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

// comment this define if you don't want 3d output
//#define USE_3D

#ifdef USE_3D

//
// Includes
//

#include <deque>
#include <string>


#include <nel/misc/rgba.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

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
		: Name(name), X(x), Y(y), Width(width), Height(height), BackColor(backColor), MaxValue(maxValue), Peak(0.0f), Quantum(quantum),
		CurrentQuantumStart(NLMISC::CTime::getLocalTime())
	{
		if (_Graphs == NULL)
		{
			_Graphs = new std::vector<CGraph*>;
		}

		_Graphs->push_back (this);
	}

	void addOneValue (float value = 0.0f);
	void addValue (float value);

	static void render (NL3D::UDriver &driver, NL3D::UTextContext &tc);

	static bool Display;
	static bool DisplayAverageValue;

private:

	static std::vector<CGraph*> *_Graphs;

	void render (NL3D::UDriver *Driver, NL3D::UTextContext *TextContext);
};

#endif

#endif // GRAPH_H

/* End of graph.h */
