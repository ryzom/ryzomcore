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

#include "stdpacs.h"

#include "nel/pacs/retrievable_surface.h"

using namespace std;
using namespace NLMISC;

/*
float			NLPACS::Models[NumCreatureModels][NumModelCharacteristics] =
{
	{ 0.5f, 1.0f, -1.0f },
	{ 0.8f, 2.0f, -0.5f },
	{ 2.0f, 4.0f, +0.5f },
	{ 4.0f, 8.0f, +0.707f }
};
*/

float			NLPACS::Models[NumMaxCreatureModels][NumModelCharacteristics] =
{
	{ 0.5f, 1.0f, -1.0f },
	{ 4.0f, 8.0f, +0.5f },
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 0.0f, -1.0f },
};

void	NLPACS::CRetrievableSurface::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	Version 1:
		- absolute water height and flag
	Version 2:
		- no more topologies in stream (obsolete)
		- no more height quad
		- quantized height (_QuantHeight)
	*/
	sint	ver= f.serialVersion(2);

	if (ver < 2)
		throw EOlderStream();

	uint	i;
	f.serial(_NormalQuanta);
	f.serial(_OrientationQuanta);
	f.serial(_Material);
	f.serial(_Character);
	f.serial(_Level);
	f.serialCont(_Chains);
	f.serialCont(_Loops);
	if (ver <= 1)
	{
		f.serial(_Quad);
		for (i=0; i<NumMaxCreatureModels; ++i)
			f.serial(_Topologies[i]);
	}
	f.serial(_Center);
	f.serial(_IsFloor, _IsCeiling);
	f.serial(_Flags);

	if (ver >= 1)
	{
		f.serial(_WaterHeight);
	}

	if (ver >= 2)
	{
		f.serial(_QuantHeight);
	}
}

void	NLPACS::CRetrievableSurface::TLoop::serial(NLMISC::IStream &f)
{
	f.serialCont(*this);
	f.serial(Length);
}
