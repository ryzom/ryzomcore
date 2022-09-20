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

#ifndef RYAI_KNAPSACK_SOLVER_H
#define RYAI_KNAPSACK_SOLVER_H

//////////////////////////////////////////////////////////////////////////////
// IKnapsackContext                                                          //
//////////////////////////////////////////////////////////////////////////////

class IKnapsackContext
{
public:
	virtual ~IKnapsackContext() { }
	virtual float weight(size_t i) = 0;
	virtual float value(size_t i) = 0;
	virtual size_t size() = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CKnapsackSolver                                                          //
//////////////////////////////////////////////////////////////////////////////

/// This class resolves the knapsack problem, in french "probleme du sac a
/// dos"
class CKnapsackSolver
{
public:
	enum Algorithm
	{
		UndefinedAlgorithm,
		Optimal,
		FullAddCheck,
		AddCheck,
		FastAddCheck,
		FullSingleReplace,
		SingleReplace,
		FastSingleReplace,
		VeryFastSingleReplace,
	//- /!\ Algorithms below don't respect constraints
		TakeAll
	};
	static std::string toString(Algorithm a);
	static CKnapsackSolver::Algorithm fromString(std::string const& a);
private:
	IKnapsackContext* _Context;
	bool* _Take;
	bool _AllocatedTake;
	float _WBest;
	float _VBest;
	float _WMax;
public:
	explicit CKnapsackSolver(IKnapsackContext* context, bool* take = NULL);
	virtual ~CKnapsackSolver();
	bool take(size_t i);
	float totalWeight();
	float totalValue();
	float totalFreeWeight();
	
	/// @param wMax Max weight that can be taken
	/// @note Not thread safe
	void optimize(float wMax, Algorithm algorithm = Optimal);
private:
	float weight(size_t i);
	float value(size_t i);
	size_t size();
	/// @name Algorithms entry points
	//@{
	void optimizeOptimal();
	void optimizeFullAddCheck();
	void optimizeAddCheck();
	void optimizeFastAddCheck();
	void optimizeFullSingleReplace();
	void optimizeSingleReplace();
	void optimizeFastSingleReplace();
	void optimizeVeryFastSingleReplace();
	void optimizeTakeAll();
	//@}
	/// @name Algorithms helper functions
	//@{
	void optimizeOptimalRec(int i, float w, float v, bool* take);
	//@}
};

//////////////////////////////////////////////////////////////////////////////
// CKnapsackContext                                                         //
//////////////////////////////////////////////////////////////////////////////

class CKnapsackContext
: public IKnapsackContext
{
	size_t _Size;
	float* _Weights;
	float* _Values;
public:
	CKnapsackContext(size_t size, float* weights, float* values);
	virtual float weight(size_t i);
	virtual float value(size_t i);
	virtual size_t size();
};

#endif
