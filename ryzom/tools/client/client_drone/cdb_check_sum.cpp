/** \file cdb_check_sum.cpp
 * <File description>
 *
 * $Id$
 */



//#include "stdpch.h"
#include "cdb_check_sum.h"


/*
 * Constructor
 */

CCDBCheckSum::CCDBCheckSum()
{
	//arbitrary values
	_Sum = 0;
	_Factor = 55665;
	_Const1 = 52845;
	_Const2 = 22719;
};

///add an uint8 to the sum
void CCDBCheckSum::add(uint8 el)
{
	uint32 cipher = (el ^ (_Factor >> 8));
	_Factor = (cipher + _Factor) * _Const1 + _Const2;
	_Sum += cipher;	
}
