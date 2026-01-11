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

#include "brain.h"

CBrain::CBrain(float f,float a,float e,float ha,float hu) : CMood(f,a,e,ha,hu)
{ 
	_UpdateEvery = 0;
}

CBrain::CBrain(CMood &personality) : CMood(personality)
{
	_UpdateEvery = 0;
}

CBrain::CBrain(const CBrain &c) : CMood()
{
	_Personality = c._Personality;
	_RealTime = c._RealTime;
	_UpdateEvery = c._UpdateEvery;
	_LastUpdate = c._LastUpdate;
}

void CBrain::setUpdateEvery(int nb_cycles)
{
	_UpdateEvery = nb_cycles;
}

float CBrain::getFear()
{
	if ( _Personality.getFear() > _RealTime.getFear() )
		return _Personality.getFear();
	else
		return _RealTime.getFear();
}

float CBrain::getAgressivity()
{
	if ( _Personality.getAgressivity() > _RealTime.getAgressivity() )
		return _Personality.getAgressivity();
	else
		return _RealTime.getAgressivity();
}

float CBrain::getEmpathy()
{
	if ( _Personality.getEmpathy() > _RealTime.getEmpathy() )
		return _Personality.getEmpathy();
	else
		return _RealTime.getEmpathy();
}

float CBrain::getHappiness()
{
	if ( _Personality.getHappiness() > _RealTime.getHappiness() )
		return _Personality.getHappiness();
	else
		return _RealTime.getHappiness();

}

float CBrain::getHunger()
{
	if ( _Personality.getHunger() > _RealTime.getHunger() )
		return _Personality.getHunger();
	else
		return _RealTime.getHunger();
}

void CBrain::setInput(CRecord *input)
{
	std::vector<CTree *>::iterator it_t = _Trees.begin();
	while ( it_t != _Trees.end() )
	{
		(*it_t)->getOutput( input );
		it_t++;
	}
}


void CBrain::addField(CField *field)
{
	_Fields.push_back( field );
}

void CBrain::addTree(CTree *tree)
{
	_Trees.push_back( tree );
}

std::vector<std::string> CBrain::getOutputs()
{
	std::vector<std::string> outputs;
	std::vector<CTree *>::iterator it_t = _Trees.begin();
	while ( it_t != _Trees.end() )
	{
		outputs.push_back( _Fields[ (*it_t)->getKey() ]->getName() );
		it_t++;
	}
	return outputs;
}

void CBrain::build()
{
	std::vector<CTree *>::iterator it_tree = _Trees.begin();
	while ( it_tree != _Trees.end() )
	{
		(*it_tree)->rebuild( _Records, _Fields );
		it_tree++;
	}
	_LastUpdate = 0;
}

std::string CBrain::getDebugString()
{
	std::string debug_string;
	std::vector<CTree *>::iterator it_tree = _Trees.begin();
	while ( it_tree != _Trees.end() )
	{
		debug_string += (*it_tree)->getDebugString( _Records, _Fields );
		debug_string += "\n";
		it_tree++;
	}
	return debug_string;
}

void CBrain::forget()
{
	while ( !_Records.empty() )
	{
		delete _Records.front();
		_Records.erase( _Records.begin() );
	}
}

void CBrain::addRecord(CRecord *record)
{
	_Records.push_back( record );
	_LastUpdate++;
	if ( _LastUpdate > _UpdateEvery )
		build();
}
