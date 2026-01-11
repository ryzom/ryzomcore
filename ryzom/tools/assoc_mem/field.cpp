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

#include "field.h"
#include "cond_node.h"

CField::CField()
{
	_Name = std::string("<unnamed>");
}

CField::CField(std::string name)
{
	_Name = name;
}

const std::vector<IValue *> &CField::getPossibleValues() const
{
	return _PossibleValues;
}

void CField::addPossibleValue(IValue *value)
{
	_PossibleValues.push_back( value );
}

CField::~CField()
{
	std::vector<IValue *>::iterator it_val = _PossibleValues.begin();
	while ( ! _PossibleValues.empty() )
	{
		delete _PossibleValues.front();
		_PossibleValues.erase( _PossibleValues.begin() );
		
	}
}

const std::string &CField::getName() const
{
	return _Name;
}

/////////////////////////////////////////////////////////////////////////////////////////////

CBoolField::CBoolField() : CField()
{
	_PossibleValues.push_back( new CValue<bool>(true) );
	_PossibleValues.push_back( new CValue<bool>(false) );
}

CBoolField::CBoolField(std::string name) : CField( name )
{
	_PossibleValues.push_back( new CValue<bool>(true) );
	_PossibleValues.push_back( new CValue<bool>(false) );
}

ICondNode *CBoolField::createNode(int key, int pos, std::vector<CRecord *> &)
{
	ICondNode *node = new CEqualNode<bool>(this, pos);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////////

CStringField::CStringField() : CField()
{
}

CStringField::CStringField(std::string name, std::vector<std::string> &values) : CField(name)
{
	std::vector<std::string>::iterator it_val = values.begin();
	while ( it_val != values.end() )
	{
		_PossibleValues.push_back( new CValue<std::string>( *it_val ) );
		it_val++;
	}
}

ICondNode *CStringField::createNode(int key, int pos, std::vector<CRecord *> &)
{
	return new CEqualNode<std::string>(this, pos);
}


/////////////////////////////////////////////////////////////////////////////////////////////

CIntField::CIntField() : CField()
{
}

CIntField::CIntField(std::string name, std::vector<int> &values) : CField( name )
{
	std::vector<int>::iterator it_val = values.begin();
	while ( it_val != values.end() )
	{
		_PossibleValues.push_back( new CValue<int>( *it_val ) );
		it_val++;
	}
}

ICondNode *CIntField::createNode(int key, int pos, std::vector<CRecord *> &)
{
	return new CEqualNode<int>(this, pos);
}


/////////////////////////////////////////////////////////////////////////////////////////////

/*
CRealField::CRealField() : CField()
{
}

CRealField::CRealField(std::string name, std::vector<double> &values) : CField( name )
{
	std::vector<double>::iterator it_val = values.begin();
	while ( it_val != values.end() )
	{
		_PossibleValues.push_back( new CValue<double>( *it_val ) );
		it_val++;
	}
}

ICondNode *CRealField::createNode(int key, int pos, std::vector<CRecord *> &)
{
	return new CEqualNode<int>(this, pos);
}

void CRealField::computeRanges(int key, int pos, std::vector<CRecord *> &records)
{
	int		max_gain_attrib = -1;
	double	max_gain = 0;

	std::vector<CRecord *>::iterator it_r = records.begin();
	while ( it_r != records.end() )
	{
		
		it_r++;
	}
}
*/