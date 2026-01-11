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



#ifndef GRAPH_H
#define GRAPH_H

#include "nel/misc/types_nl.h"

#include <nel/misc/rgba.h>
#include <nel/misc/time_nl.h>

#include <deque>
#include <string>

#include "time_client.h"

namespace NL3D
{
	class UDriver;
	class UTextContext;
}


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
	uint Page;

	std::deque<float> Values;

	NLMISC::TTime Quantum;

	NLMISC::TTime CurrentQuantumStart;

	/// Init driver and text context
	static void init (NL3D::UDriver *driver)
	{
		_Driver = driver;
	}

	/// Render all available graph
	static void render (uint page);

	/// release material
	virtual ~CGraph()
	{
		if (_Graphs != NULL)
		{
			for (uint i = 0; i < _Graphs->size(); i++)
			{
				if ((*_Graphs)[i] == this)
				{
					_Graphs->erase (_Graphs->begin()+i);
					break;
				}
			}
		}
	}


	/// Constructor (CGraph::init() must have been called before)
	CGraph (std::string name,
			float x, float y, float width, float height,
			NLMISC::CRGBA backColor,
			NLMISC::TTime quantum,
			float maxValue, uint page,
			bool lineMode = false)
		: Name(name), X(x), Y(y), Width(width), Height(height), BackColor(backColor), MaxValue(maxValue),
		  Peak(0.0f), LineMode(lineMode), PrevY(y), Page(page), Quantum(quantum), CurrentQuantumStart(ryzomGetLocalTime ())
	{
		if (_Graphs == NULL)
		{
			_Graphs = new std::vector<CGraph*>;
		}

		_Graphs->push_back (this);
	}

	/// Add one value
	void addOneValue (float value = 0.0f);

	/// Add value
	void addValue (float value);

	static bool					DisplayAverageValue;
	static bool					Display;

private:

	/// Render a specific graph
	void renderGraph ();

	static NL3D::UDriver		*_Driver;

	static std::vector<CGraph*> *_Graphs;
};


/*
 * Graph relative to a particular entity
 */
class CSlotGraph : public CGraph
{
public:

	/// Constructor
	CSlotGraph( std::string name,
				float x, float y, float width, float height,
				NLMISC::CRGBA backColor,
				NLMISC::TTime quantum,
				float maxValue,
				bool lineMode,
				uint8 slot )
				: CGraph( name, x, y, width, height, backColor, quantum, maxValue, lineMode ), _Slot( slot )
				{}

	/// Add one value only for the stored slot
	void	addOneValue( uint8 slot, float value )
	{
		if ( slot == _Slot )
		{
			CGraph::addOneValue( value );
		}
	}

private:

	uint8	_Slot;
};

#endif // GRAPH_H

/* End of graph.h */
