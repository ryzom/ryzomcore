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
#include "entity_matrix.h"

// static data
std::vector<CEntityMatrixPatternSymetrical*> CEntityMatrixPatternSymetrical::_DiscPatterns;


//--------------------------------------------------------------------------
// DATA TABLES FOR ENTITY MATRIX
//--------------------------------------------------------------------------

// a series of tables giving the minimum iterator table forms for entity matrix iterators for all sizes up to 127m
static uint32 EntityMatrixDiscTbl0[]   = { 3,  3,  3};
static uint32 EntityMatrixDiscTbl16[]  = { 3,  5,  5,  5,  3};
static uint32 EntityMatrixDiscTbl23[]  = { 5,  5,  5,  5,  5};
static uint32 EntityMatrixDiscTbl32[]  = { 3,  5,  7,  7,  7,  5,  3};
static uint32 EntityMatrixDiscTbl36[]  = { 5,  7,  7,  7,  7,  7,  5};
static uint32 EntityMatrixDiscTbl46[]  = { 7,  7,  7,  7,  7,  7,  7};
static uint32 EntityMatrixDiscTbl48[]  = { 3,  7,  7,  9,  9,  9,  7,  7,  3};
static uint32 EntityMatrixDiscTbl51[]  = { 5,  7,  9,  9,  9,  9,  9,  7,  5};
static uint32 EntityMatrixDiscTbl58[]  = { 7,  9,  9,  9,  9,  9,  9,  9,  7};
static uint32 EntityMatrixDiscTbl64[]  = { 3,  7,  9,  9, 11, 11, 11,  9,  9,  7,  3};
static uint32 EntityMatrixDiscTbl66[]  = { 5,  7,  9, 11, 11, 11, 11, 11,  9,  7,  5};
static uint32 EntityMatrixDiscTbl68[]  = { 5,  9,  9, 11, 11, 11, 11, 11,  9,  9,  5};
static uint32 EntityMatrixDiscTbl72[]  = { 7,  9, 11, 11, 11, 11, 11, 11, 11,  9,  7};
static uint32 EntityMatrixDiscTbl80[]  = { 3,  9, 11, 11, 11, 13, 13, 13, 11, 11, 11,  9,  3};
static uint32 EntityMatrixDiscTbl82[]  = { 5,  9, 11, 11, 13, 13, 13, 13, 13, 11, 11,  9,  5};
static uint32 EntityMatrixDiscTbl87[]  = { 7,  9, 11, 13, 13, 13, 13, 13, 13, 13, 11,  9,  7};
static uint32 EntityMatrixDiscTbl91[]  = { 7, 11, 11, 13, 13, 13, 13, 13, 13, 13, 11, 11,  7};
static uint32 EntityMatrixDiscTbl94[]  = { 9, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11,  9};
static uint32 EntityMatrixDiscTbl96[]  = { 3,  9, 11, 13, 13, 13, 15, 15, 15, 13, 13, 13, 11,  9,  3};
static uint32 EntityMatrixDiscTbl98[]  = { 5,  9, 11, 13, 13, 15, 15, 15, 15, 15, 13, 13, 11,  9,  5};
static uint32 EntityMatrixDiscTbl102[] = { 7,  9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 13, 11,  9,  7};
static uint32 EntityMatrixDiscTbl103[] = { 7, 11, 13, 13, 15, 15, 15, 15, 15, 15, 15, 13, 13, 11,  7};
static uint32 EntityMatrixDiscTbl108[] = { 9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 11,  9};
static uint32 EntityMatrixDiscTbl112[] = { 3,  9, 11, 13, 15, 15, 15, 17, 17, 17, 15, 15, 15, 13, 11,  9,  3};
static uint32 EntityMatrixDiscTbl114[] = { 5,  9, 13, 13, 15, 15, 17, 17, 17, 17, 17, 15, 15, 13, 13,  9,  5};
static uint32 EntityMatrixDiscTbl116[] = { 5, 11, 13, 15, 15, 15, 17, 17, 17, 17, 17, 15, 15, 15, 13, 11,  5};
static uint32 EntityMatrixDiscTbl117[] = { 7, 11, 13, 15, 15, 17, 17, 17, 17, 17, 17, 17, 15, 15, 13, 11,  7};
static uint32 EntityMatrixDiscTbl122[] = { 9, 11, 13, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 15, 13, 11,  9};
static uint32 EntityMatrixDiscTbl125[] = { 9, 13, 15, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 15, 15, 13,  9};

// a few larger special case matrices
static uint32 EntityMatrixDiscTblUpTo150[] = { 7, 11, 15, 17, 17, 19, 19, 21, 21, 21, 21, 21, 21, 21, 19, 19, 17, 17, 15, 11, 7};
static uint32 EntityMatrixDiscTblUpTo200[] = { 9, 13, 17, 19, 21, 23, 23, 25, 25, 27, 27, 27, 27, 27, 27, 27, 27, 27, 25, 25, 23, 23, 21, 19, 17, 13, 9};
static uint32 EntityMatrixDiscTblUpTo250[] = {11, 15, 19, 23, 25, 27, 27, 29, 29, 31, 31, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 31, 31, 29, 29, 27, 27, 25, 23, 19, 15, 11};

void CEntityMatrixPatternSymetrical::initMatrixPatterns()
{
	// initialise the vectors with the first tables
	_DiscPatterns.push_back(new CEntityMatrixPatternSymetrical(&(EntityMatrixDiscTbl0[0]),3));

	// local macros undefined at end of function
#define ADD_TBL(d) \
	{				   \
	while (_DiscPatterns.size()<d) \
		_DiscPatterns.push_back(_DiscPatterns[_DiscPatterns.size()-1]); \
	_DiscPatterns.push_back(new CEntityMatrixPatternSymetrical(EntityMatrixDiscTbl##d,sizeof(EntityMatrixDiscTbl##d)/sizeof(EntityMatrixDiscTbl##d[0]))); \
	}

	
	// setup the disc tables ...
		ADD_TBL(16)	ADD_TBL(23)	
		ADD_TBL(32)	ADD_TBL(36)	ADD_TBL(46)
		ADD_TBL(48)	ADD_TBL(51)	ADD_TBL(58)	
		ADD_TBL(64)	ADD_TBL(66)	ADD_TBL(68)	ADD_TBL(72)
		ADD_TBL(80)	ADD_TBL(82)	ADD_TBL(87)	ADD_TBL(91)	ADD_TBL(94)	
		ADD_TBL(96)	ADD_TBL(98)	ADD_TBL(102) ADD_TBL(103) ADD_TBL(108)
		ADD_TBL(112) ADD_TBL(114) ADD_TBL(116) ADD_TBL(117) ADD_TBL(122) ADD_TBL(125)
		
#undef ADD_TBL
}



