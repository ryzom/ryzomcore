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



#ifndef RYAI_AI_INLINE_H
#define RYAI_AI_INLINE_H


//-------------------------------------------------------------------
// Important boolean showing true when the singleton has been initialised
// and other pseudo-constants initialised at service init time

/*inline bool CAIS::initialised()
{
	return _initialised;
}
*/
//-------------------------------------------------------------------
// Interface to the random number generator
inline sint32 CAIS::randPlusMinus(uint16 mod)	{ return _random.randPlusMinus(mod); }
inline float CAIS::frand(double mod)			{ return _random.frand(mod); }
inline float CAIS::frandPlusMinus(double mod)	{ return _random.frandPlusMinus(mod); }

inline uint32 CAIS::rand32()
{ 
	return ((uint32(_random.rand()))<<16)+uint32(_random.rand());
}
inline uint32 CAIS::rand32(uint32 mod)
{ 
	if (mod==0)
		return	0;
	return rand32()%mod;
}
inline uint32 CAIS::rand16(uint32 mod)
{ 
	if (mod==0)
		return	0;
	return _random.rand()%mod;
}

//-------------------------------------------------------------------
// Interface to the vision management matrices

// read accessors for getting hold of the vision matrices
inline const CAIEntityMatrixIteratorTblRandom	*CAIS::matrixIterator2x2()	{ return &_matrixIterator2x2; }
inline const CAIEntityMatrixIteratorTblRandom	*CAIS::matrixIterator3x3()	{ return &_matrixIterator3x3; }


#endif


