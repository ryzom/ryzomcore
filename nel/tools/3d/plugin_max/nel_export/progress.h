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

#ifndef __PROGRESS__H
#define __PROGRESS__H

#include <string>
#include "../nel_mesh_lib/export_nel.h"


// ---------------------------------------------------------------------------

class CProgressBar : public IProgress
{
	
public:

	bool bCancelCalculation;
	float rRatioCalculated;
	double rTimeBegin;
	uint32 nNbTotalMeshes;
	std::string sInfoProgress[14];
	HWND hWndProgress;

public:

	CProgressBar();
	~CProgressBar();

	void initProgressBar (sint32 nNbMesh, Interface &ip);
	void uninitProgressBar();
	void updateProgressBar( sint32 NMeshNb );
	bool isCanceledProgressBar();

	// Interface implemenation
	virtual void setLine (uint32 LineNumber, std::string &LineText);
	virtual void update();
};

// ---------------------------------------------------------------------------



#endif