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
#include "knapsack_solver.h"

//////////////////////////////////////////////////////////////////////////////
// CKnapsackSolver                                                          //
//////////////////////////////////////////////////////////////////////////////

std::string CKnapsackSolver::toString(Algorithm a)
{
	switch (a)
	{
	case Optimal:				return "Optimal";
	case FullAddCheck:			return "FullAddCheck";
	case AddCheck:				return "AddCheck";
	case FastAddCheck:			return "FastAddCheck";
	case FullSingleReplace:		return "FullSingleReplace";
	case SingleReplace:			return "SingleReplace";
	case FastSingleReplace:		return "FastSingleReplace";
	case VeryFastSingleReplace:	return "VeryFastSingleReplace";
	case TakeAll:				return "TakeAll";
	default:					return "undefined";
	}
}

CKnapsackSolver::Algorithm CKnapsackSolver::fromString(std::string const& a)
{
	if (a=="Optimal")				return Optimal;
	if (a=="FullAddCheck")			return FullAddCheck;
	if (a=="AddCheck")				return AddCheck;
	if (a=="FastAddCheck")			return FastAddCheck;
	if (a=="FullSingleReplace")		return FullSingleReplace;
	if (a=="SingleReplace")			return SingleReplace;
	if (a=="FastSingleReplace")		return FastSingleReplace;
	if (a=="VeryFastSingleReplace")	return VeryFastSingleReplace;
	if (a=="TakeAll")				return TakeAll;
	return UndefinedAlgorithm;
}

/// Find the set that fit the specified maximum weight which have the maximal
/// value total, using an already defined good solution.
void CKnapsackSolver::optimize(float wMax, Algorithm algorithm)
{
	H_AUTO(CKnapsackSolver_optimize);
	_WMax = wMax;
	// Solve the problem
	switch (algorithm)
	{
	case FullAddCheck:			optimizeFullAddCheck();				break;
	case AddCheck:				optimizeAddCheck();					break;
	case FastAddCheck:			optimizeFastAddCheck();				break;
	case FullSingleReplace:		optimizeFullSingleReplace();		break;
	case SingleReplace:			optimizeSingleReplace();			break;
	default:
	case FastSingleReplace:		optimizeFastSingleReplace();		break;
	case VeryFastSingleReplace:	optimizeVeryFastSingleReplace();	break;
	case Optimal:				optimizeOptimal();					break;
	case TakeAll:				optimizeTakeAll();					break;
	}
}

/// Algorithm is taken from http://eleves.ensmp.fr/P00/00rouaul/sacados/sacados_swp.html
/// This algorithm complexity is O(N^2)
void CKnapsackSolver::optimizeOptimal()
{
	H_AUTO(CKnapsackSolver_optimizeOptimal);
	// :FIXME: Not thread safe
	// Allocate temporary solution
	bool* take = new bool[size()];
	for (size_t i=0; i<size(); ++i)
		take[i] = false;
	// Run the optimization recursion
	optimizeOptimalRec((int)size()-1, _WMax, 0, take);
	// Delete temporary solution
	delete [] take;
}

/// @param i take[j] for j>i are already determined by the recursion
/// @param w Free weight to fill
/// @param v Sum of the taken values (ie all value(j) where take[j] is true and j > i)
/// @param take Current examined partial solution
void CKnapsackSolver::optimizeOptimalRec(int i, float w, float v, bool* take)
{
	nlassert(i>=-1);
	if (i==-1)
	{
		if (v > _VBest)
		{
			_WBest = _WMax - w;
			_VBest = v;
			std::copy(take, take+size(), _Take);
		}
	}
	else
	{
		take[i] = false;
		optimizeOptimalRec(i-1, w, v, take);
		if (weight(i) <= w)
		{
			take[i] = true;
			optimizeOptimalRec(i - 1, w - weight(i), v + value(i), take);
		}
	}
}

/// Here we consider already defined solution has lots of take[i] that are
/// true. We just find the first i for which take[i] is false and that don't
/// exceed maximal weight. If we find one we take it which gives a better
/// solution.
/// Note: We start at the end since CTargetable puts candidate at end.
/// This algorithm complexity is O(N)
void CKnapsackSolver::optimizeAddCheck()
{
	H_AUTO(CKnapsackSolver_optimizeAddCheck);
	int i = (int)size()-1;
	float w = _WMax - _WBest;
	while (i>=0)
	{
		if (!_Take[i] && weight(i) <= w)
		{
			_Take[i] = true;
			_WBest += weight(i);
			_VBest += value(i);
			break;
		}
		--i;
	}
}

/// Same as AddCheck, except that we consider all false take[i], even if we already took some.
/// This algorithm complexity is Theta(N)
void CKnapsackSolver::optimizeFullAddCheck()
{
	H_AUTO(CKnapsackSolver_optimizeFullAddCheck);
	int i = (int)size()-1;
	float w = _WMax - _WBest;
	while (i>=0)
	{
		if (!_Take[i] && weight(i) <= w)
		{
			_Take[i] = true;
			_WBest += weight(i);
			_VBest += value(i);
		}
		--i;
	}
}

/// Same as AddCheck, except that we examine only a single false take[i], event if it cannot be taken.
/// This algorithm complexity is O(N), but O(1) when used by CTargetable
void CKnapsackSolver::optimizeFastAddCheck()
{
	H_AUTO(CKnapsackSolver_optimizeFastAddCheck);
	int i = (int)size()-1;
	float w = _WMax - _WBest;
	while (i>=0)
	{
		if (!_Take[i])
		{
			if (weight(i) <= w)
			{
				_Take[i] = true;
				_WBest += weight(i);
				_VBest += value(i);
			}
			break;
		}
		--i;
	}
}

/// First try FullAddCheck, then try to replace the already taken elements with not taken ones.
/// This algorithm complexity is Theta(N^2)
void CKnapsackSolver::optimizeFullSingleReplace()
{
	optimizeFullAddCheck();
	H_AUTO(CKnapsackSolver_optimizeFullSingleReplace);
	int i = (int)size()-1;
	while (i>=0)
	{
		// For each not taken ith element
		if (!_Take[i])
		{
			float w = weight(i);
			float v = value(i);
			int worst = i;
			// Find the worst element that ith element can replace
			int j = (int)size()-1;
			while (j>=0)
			{
				if (i!=j && _Take[j] && w<=weight(j) && v>value(j))
				{
					worst = j;
					w = weight(j);
					v = value(j);
				}
				--j;
			}
			// If we find one untake it and take ith.
			if (worst!=i)
			{
				_Take[worst] = false;
				_WBest -= weight(worst);
				_VBest -= value(worst);
				_Take[i] = true;
				_WBest += weight(i);
				_VBest += value(i);
			}
		}
		--i;
	}
}

/// First try FastAddCheck, and if it fails optimizing try to replace a not
/// taken one with an already taken element (the worst one) until a
/// replacement occurs.
/// This algorithm complexity is Theta(N^2) and O(N^2) for CTargetable
void CKnapsackSolver::optimizeSingleReplace()
{
	float vBest = _VBest;
	optimizeAddCheck();
	if (_VBest > vBest)
		return;
	H_AUTO(CKnapsackSolver_optimizeSingleReplace);
	int i = (int)size()-1;
	while (i>=0)
	{
		// For each not taken ith element
		if (!_Take[i])
		{
			float w = weight(i);
			float v = value(i);
			int worst = i;
			// Find the worst element that ith element can replace
			int j = (int)size()-1;
			while (j>=0)
			{
				if (i!=j && _Take[j] && w<=weight(j) && v>value(j))
				{
					worst = j;
					w = weight(j);
					v = value(j);
				}
				--j;
			}
			// If we find one untake it and take ith.
			if (worst!=i)
			{
				_Take[worst] = false;
				_WBest -= weight(worst);
				_VBest -= value(worst);
				_Take[i] = true;
				_WBest += weight(i);
				_VBest += value(i);
				break;
			}
		}
		--i;
	}
}

/// First try FastAddCheck, and if it fails optimizing try to replace the
/// first not taken element with an already taken element (the worst one).
/// This algorithm complexity is O(N^2) and Theta(N) for CTargetable
void CKnapsackSolver::optimizeFastSingleReplace()
{
	float vBest = _VBest;
	optimizeFastAddCheck();
	if (_VBest > vBest)
		return;
	H_AUTO(CKnapsackSolver_optimizeFastSingleReplace);
	int i = (int)size()-1;
	while (i>=0)
	{
		// For each not taken ith element
		if (!_Take[i])
		{
			float w = weight(i);
			float v = value(i);
			int worst = i;
			// Find the worst element that ith element can replace
			int j = (int)size()-1;
			while (j>=0)
			{
				if (i!=j && _Take[j] && w<=weight(j) && v>value(j))
				{
					worst = j;
					w = weight(j);
					v = value(j);
				}
				--j;
			}
			// If we find one untake it and take ith.
			if (worst!=i)
			{
				_Take[worst] = false;
				_WBest -= weight(worst);
				_VBest -= value(worst);
				_Take[i] = true;
				_WBest += weight(i);
				_VBest += value(i);
			}
			break;
		}
		--i;
	}
}

/// First try FastAddCheck, and if it fails optimizing try to replace the
/// first not taken element with an already taken one (the first worst that
/// the not taken one).
/// This algorithm complexity is O(N^2) and O(N) for CTargetable
void CKnapsackSolver::optimizeVeryFastSingleReplace()
{
	float vBest = _VBest;
	optimizeFastAddCheck();
	if (_VBest > vBest)
		return;
	H_AUTO(CKnapsackSolver_optimizeVeryFastSingleReplace);
	int i = (int)size()-1;
	while (i>=0)
	{
		// For each not taken ith element
		if (!_Take[i])
		{
			float w = weight(i);
			float v = value(i);
			int worst = i;
			// Find the worst element that ith element can replace
			int j = (int)size()-1;
			while (j>=0)
			{
				if (i!=j && _Take[j] && w<=weight(j) && v>value(j))
				{
					worst = j;
					w = weight(j);
					v = value(j);
					break;
				}
				--j;
			}
			// If we find one untake it and take ith.
			if (worst!=i)
			{
				_Take[worst] = false;
				_WBest -= weight(worst);
				_VBest -= value(worst);
				_Take[i] = true;
				_WBest += weight(i);
				_VBest += value(i);
			}
			break;
		}
		--i;
	}
}

void CKnapsackSolver::optimizeTakeAll()
{
	H_AUTO(CKnapsackSolver_optimizeTakeAll);
	_WBest = 0;
	_VBest = 0;
	int i = (int)size()-1;
	while (i>=0)
	{
		_Take[i] = true;
		_WBest += weight(i);
		_VBest += value(i);
		--i;
	}
}

CKnapsackSolver::CKnapsackSolver(IKnapsackContext* context, bool* _take)
: _Context(context)
, _Take(_take)
, _AllocatedTake(_take==NULL)
, _WBest(0.f)
, _VBest(0.f)
, _WMax(0.f)
{
	if (_take==NULL && size()!=0)
	{
		_Take = new bool[size()];
		for (size_t i=0; i<size(); ++i)
			_Take[i] = false;
	}
	else
	{
		for (size_t i=0; i<size(); ++i)
		{
			if (take(i))
			{
				_WBest += weight(i);
				_VBest += value(i);
			}
		}
		_WMax = _WBest;
	}
}

CKnapsackSolver::~CKnapsackSolver()
{
	if (_AllocatedTake)
		delete [] _Take;
}

float CKnapsackSolver::weight(size_t i)
{
	if (_Context!=0)
		return _Context->weight(i);
	else
		return 0.f;
}

float CKnapsackSolver::value(size_t i)
{
	if (_Context!=0)
		return _Context->value(i);
	else
		return 0.f;
}

size_t CKnapsackSolver::size()
{
	if (_Context!=0)
		return _Context->size();
	else
		return 0;
}

bool CKnapsackSolver::take(size_t i)
{
	if (_Take!=0 && i>=0 && i<size())
		return _Take[i];
	else
		return false;
}

float CKnapsackSolver::totalWeight()
{
	return _WBest;
}

float CKnapsackSolver::totalValue()
{
	return _VBest;
}

float CKnapsackSolver::totalFreeWeight()
{
	return _WMax - _WBest;
}

//////////////////////////////////////////////////////////////////////////////
// CKnapsackContext                                                         //
//////////////////////////////////////////////////////////////////////////////

CKnapsackContext::CKnapsackContext(size_t size, float* weights, float* values)
: _Size(size)
, _Weights(weights)
, _Values(values)
{
}

float CKnapsackContext::weight(size_t i)
{
	if (i>=0 && i<_Size)
		return _Weights[i];
	else
		return 0.f;
}

float CKnapsackContext::value(size_t i)
{
	if (i>=0 && i<_Size)
		return _Values[i];
	else
		return 0.f;
}

size_t CKnapsackContext::size()
{
	return _Size;
}
