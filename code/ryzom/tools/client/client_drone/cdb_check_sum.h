/** \file cdb_check_sum.h
 * <File description>
 *
 * $Id: cdb_check_sum.h,v 1.1 2005/09/19 09:50:17 boucher Exp $
 */



#ifndef NL_CDB_CHECK_SUM_H
#define NL_CDB_CHECK_SUM_H

#include "nel/misc/types_nl.h"
#include <vector>

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
		for (typename std::vector<T>::const_iterator it = vect.begin(); it != vect.end(); it++)
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
