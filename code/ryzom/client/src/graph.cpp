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

#include "graph.h"

#include <nel/misc/command.h>

extern NL3D::UTextContext *TextContext;

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
UDriver			*CGraph::_Driver = NULL;
vector<CGraph*> *CGraph::_Graphs = NULL;


//
// Classes
//

void CGraph::render (uint page)
{
	if (!Display) return;

	if (_Driver == NULL) return;
	if (_Graphs == NULL) return;

	for (uint i = 0; i < _Graphs->size(); i++)
	{
		// Graph in all info pages
		if ((*_Graphs)[i]->Page == page)
			(*_Graphs)[i]->renderGraph ();
	}
}

void CGraph::renderGraph ()
{
	if (_Driver == NULL)
		return;

	// Display the background
	uint32 w, h;
	_Driver->getWindowSize (w, h);
	float ScreenWidth = (float) w;
	float ScreenHeight = (float) h;
	_Driver->setMatrixMode2D (CFrustum (0.0f, ScreenWidth, 0.0f, ScreenHeight, -1.0f, 1.0f, false));
	_Driver->drawQuad (X, Y, X+Width, Y+Height, BackColor);

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

		_Driver->drawLine (vect1.x, vect1.y, pos, PrevY, lineCol);

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
	_Driver->drawLine (X, peakval, X+Width, peakval, frontCol);

	// Display average
	float average = sum / favoid0((float)Values.size());
	value = average * Height / MaxValue;
	if (value > Height) value = Height;
	float avrval = Y+value;
	_Driver->drawLine (X, avrval, X+Width, avrval, frontCol);

	if (TextContext != NULL)
	{
		TextContext->setShaded (false);
		TextContext->setHotSpot (UTextContext::MiddleLeft);
		TextContext->setColor (frontCol);
		TextContext->setFontSize (10);
		TextContext->printfAt ((X+Width+2)/ScreenWidth, peakval/ScreenHeight, "%.2f", Peak);

		if (DisplayAverageValue)
		{
			// don't display the average value if they are superposed
			if (avrval+5<peakval-5 || avrval-5>peakval+5)
			{
				TextContext->printfAt ((X+Width+2)/ScreenWidth, avrval/ScreenHeight, "%.2f", average);
			}
		}

		// Display name
		TextContext->setHotSpot (UTextContext::TopLeft);
		TextContext->printfAt ((X+1)/ScreenWidth, (Y+Height-1)/ScreenHeight, Name.c_str());
	}
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
	TTime currentTime = ryzomGetLocalTime ();

	while (Values.size () == 0 || currentTime > CurrentQuantumStart + Quantum)
	{
		CurrentQuantumStart += Quantum;
		addOneValue ();
	}

	Values.back() += value;

//	if (Values.back() > Peak)
//		Peak = Values.back();
}
