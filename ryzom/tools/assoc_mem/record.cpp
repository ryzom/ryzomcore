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

#include "record.h"

CRecord::CRecord()
{
}

CRecord::CRecord(std::vector<IValue *> &)
{
}

CRecord::~CRecord()
{
/*	std::vector<IValue *>::iterator it_val = _Values.begin();
	while ( it_val != _Values.end() )
	{
		IValue *val = *it_val;
		it_val++;
		delete val;
	}
*/	
}

void CRecord::addValue(IValue *value)
{
	_Values.push_back( value );
}

void CRecord::addValue(std::string &str)
{
	_Values.push_back( new CValue<std::string>(str) );
}

void CRecord::addValue(bool b)
{
	_Values.push_back( new CValue<bool>(b) );
}

void CRecord::addValue(int val)
{
	_Values.push_back( new CValue<int>(val) );
}

const std::vector<IValue *> &CRecord::getValues()
{
	return _Values;
}

const IValue *CRecord::operator[](int index)
{
	return _Values[index];
}

int CRecord::size()
{
	return (int)_Values.size();
}