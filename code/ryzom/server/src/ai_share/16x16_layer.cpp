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

#include "16x16_layer.h"

using namespace std;
using namespace NLMISC;

/*
 * Loads a 16x16Layer and returns a pointer to it. Layer is automatically allocated.
 */
I16x16Layer		*I16x16Layer::load(NLMISC::IStream &f)
{
	uint8	type = 0;
	f.serial(type);

	I16x16Layer	*layer = NULL;

	switch (type)
	{
	case 1:	layer = new CFull16x16Layer(); break;
	case 2:	layer = new C8Bits16x16Layer(); break;
	case 3:	layer = new C4Bits16x16Layer(); break;
	case 4:	layer = new C2Bits16x16Layer(); break;
	case 5:	layer = new C1Bit16x16Layer(); break;
	case 6:	layer = new CWhite16x16Layer(); break;
	}

	if (layer != NULL)
		layer->serial(f);

	return layer;
}

/*
 * Saves a 16x16Layer.
 */
void			I16x16Layer::save(NLMISC::IStream &f, I16x16Layer *layer)
{
	uint8	type = 0;

	if		(dynamic_cast<CFull16x16Layer*>(layer))		type = 1;
	else if	(dynamic_cast<C8Bits16x16Layer*>(layer))	type = 2;
	else if	(dynamic_cast<C4Bits16x16Layer*>(layer))	type = 3;
	else if	(dynamic_cast<C2Bits16x16Layer*>(layer))	type = 4;
	else if	(dynamic_cast<C1Bit16x16Layer*>(layer))		type = 5;
	else if	(dynamic_cast<CWhite16x16Layer*>(layer))	type = 6;

	f.serial(type);
	if (type != 0)
		layer->serial(f);
	else if (layer != NULL)
		nlwarning("Unknown 16x16 layer type %s, aborted", typeid(*layer).name());
}


/*
 * Compresses a 16x16Layer. Returns a new layer if compression was successful, otherwise returns same layer.
 * If compression was successful, previous layer is deleted.
 */
I16x16Layer		*I16x16Layer::compress(I16x16Layer *layer, sint32 blank)
{
	CFull16x16Layer	*flayer = dynamic_cast<CFull16x16Layer*>(layer);
	if (flayer == NULL)
	{
		nlwarning("Can't compressed layer, already compressed");
		return layer;
	}

	sint32	min = 0x7fffffff, max = 0x80000000;
	uint	i, j;

	map<sint32, uint>	count;

	for (i=0; i<256; ++i)
	{
		sint32	val = flayer->Array[0][i];

		if (val == blank)
			continue;

		if (val < min)	min = val;
		if (val > max)	max = val;

		map<sint32, uint>::iterator	it = count.find(val);
		if (it == count.end())
			count.insert(make_pair<sint32, uint>(val, 1));
		else
			++((*it).second);
	}

	uint	msize = (uint)count.size();

	if (msize == 1)
	{
		// white 16x16 layer
		CWhite16x16Layer	*nlayer = new CWhite16x16Layer();
		nlayer->set(0, 0, min);
		delete layer;
		return nlayer;
	}
	else if (msize == 2)
	{
		// 1 bit 16x16 layer, 2 values
		C1Bit16x16Layer	*nlayer = new C1Bit16x16Layer();
		nlayer->Values[0] = min;
		nlayer->Values[1] = max;
		for (i=0; i<16; ++i)
			for (j=0; j<16; ++j)
				nlayer->set(i, j, flayer->get(i, j));

		compare(layer, nlayer, 0x7fffffff);
		delete layer;
		return nlayer;
	}
	else if (msize <= 4)
	{
		// 2 bits 16x16 layer, 4 values
		C2Bits16x16Layer	*nlayer = new C2Bits16x16Layer();
		map<sint32, uint>::iterator	it;
		i=0;
		for (it=count.begin(); it!=count.end(); ++it)
			nlayer->Values[i++] = (*it).first;
		for (i=0; i<16; ++i)
			for (j=0; j<16; ++j)
				nlayer->set(i, j, flayer->get(i, j));

		compare(layer, nlayer, 0x7fffffff);
		delete layer;
		return nlayer;
	}
	else if (max-min < 16)
	{
		// 4 bits 16x16 layer, 16 values in range
		C4Bits16x16Layer	*nlayer = new C4Bits16x16Layer();
		nlayer->Mean = min;
		for (i=0; i<16; ++i)
			for (j=0; j<16; ++j)
				nlayer->set(i, j, flayer->get(i, j));

		compare(layer, nlayer, 0x7fffffff);
		delete layer;
		return nlayer;
	}
	else if (max-min < 256)
	{
		// 8 bits 16x16 layer, 256 values in range
		C8Bits16x16Layer	*nlayer = new C8Bits16x16Layer();
		nlayer->Mean = min;
		for (i=0; i<16; ++i)
			for (j=0; j<16; ++j)
				nlayer->set(i, j, flayer->get(i, j));

		compare(layer, nlayer, 0x7fffffff);
		delete layer;
		return nlayer;
	}

	return layer;
}

bool	I16x16Layer::compare(I16x16Layer *original, I16x16Layer *copy, sint32 avoid)
{
	uint	i, j;
	bool	failed = false;
	for (i=0; i<16; ++i)
	{
		for (j=0; j<16; ++j)
		{
			if (original->get(i, j) != avoid && original->get(i, j) != copy->get(i, j))
			{
				nlwarning("Different values !");
				failed = true;
			}
		}
	}

	return failed;
}
