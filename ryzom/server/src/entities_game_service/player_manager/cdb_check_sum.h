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



#ifndef NL_CDB_CHECK_SUM_H
#define NL_CDB_CHECK_SUM_H


/**
 * class implementing check sum for the client database
 * these check sum can be used to ensure that linked properties have all been modified
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CCDBCheckSum
{
public:
	///constructor
	CCDBCheckSum();

	//clear the sum
	void clear()
	{
		_Sum = 0;
	};

	///add an uint8 to the sum
	void add(uint8 el);

	///add a value to the check sum
	template <class T>
	void add(const T & el)
	{
		T value = el;
		for (uint8 i=0; i< sizeof(T); i++)
		{
			uint8 tmp = (uint8)(value & 0xFF);
			add(tmp);
			value >>=8;
		}
	}

	///add a vector to the sum
	template <class T>
	void addVector(const std::vector<T> &  vect)
	{
		for (typename std::vector<T>::const_iterator it = vect.begin(); it != vect.end(); ++it)
			add(*it);
	}
	
	uint32 getSum()
	{
		return _Sum;
	}
private:
	///the checsum result
	uint32 _Sum;

	///the following values are used in the check algorithm
	uint32 _Factor;
    uint32 _Const1;
    uint32 _Const2;
    
};



#endif // NL_CDB_CHECK_SUM_H

/* End of cdb_check_sum.h */
