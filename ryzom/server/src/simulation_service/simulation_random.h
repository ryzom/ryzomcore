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

#ifndef SIMULATION_RANDOM_H
#define SIMULATION_RANDOM_H

// returns a number drawn from the exponential distribution with the given mean
float exponential( float mean = 1.0f );

// returns a number drawn from a gaussian distribution with the
// specified mean and standard deviation
float gaussian( float mean, float stdev );

// This is a simple but pretty good random number generator.
float myRand();

// set the seed (optional)
void myRandSeed(long s);

// Park and Miller's "Minimal Standard" generator using Schrage's
// method to do the computation in 32 bit arithmetic without overflow.
long pmRand();

// Bayes-Durham shuffle
long pmRandShuffle();

// set the seed (optional)
void pmRandSeed(long s);

#endif	// SIMULATION_RANDOM_H
