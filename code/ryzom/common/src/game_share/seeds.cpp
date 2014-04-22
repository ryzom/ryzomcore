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
#include "seeds.h"


// shortcut to types defined in CSeeds
typedef CSeeds::TUInt    TUInt;
typedef CSeeds::TBigUInt TBigUInt;


/** Add a value and return the overflow
  */
static inline void addAndCheckOverFlow(TUInt &dest, TUInt src, TUInt &overflow)
{
	TBigUInt newValue = (TBigUInt) src + (TBigUInt) dest;
	if (newValue > (TBigUInt) CSeeds::MaxUIntValue) // has an overflow occured ?
	{
		dest= (TUInt) (newValue - (TBigUInt) CSeeds::MaxUIntValue);
		overflow = (TUInt) CSeeds::MaxUIntValue;
	}
	else
	{
		overflow = 0;
		dest     = (TUInt) newValue;
	}
}

//=========================================================
void CSeeds::add(const CSeeds &other, CSeeds &overflow)
{
	// add each component and check overflow
	addAndCheckOverFlow(_LS, other._LS, overflow._LS);
	addAndCheckOverFlow(_MS, other._MS, overflow._MS);
	addAndCheckOverFlow(_BS, other._BS, overflow._BS);
	addAndCheckOverFlow(_VBS, other._VBS, overflow._VBS);
}

//=========================================================
void CSeeds::tradeSubtract(TBigUInt toSubTotal)
{
	nlassert(toSubTotal <= this->getTotal()); // quantity to subtract is too high!
	// We always begin to subtract the lowest seeds
	// Little seeds
	TBigUInt toSub = std::min((TBigUInt) _LS, toSubTotal);
	toSubTotal -= toSub;
	_LS -= (TUInt) toSub;
	if (toSubTotal == 0) return;
	// Medium seeds
	toSub = std::min((TBigUInt) _MS, toSubTotal / 10);
	toSubTotal -= 10 * toSub;
	_MS -= (TUInt) toSub;
	if (toSubTotal == 0) return;
	// Big seeds
	toSub = std::min((TBigUInt) _BS, toSubTotal / 100);
	toSubTotal -= 100 * toSub;
	_BS -= (TUInt) toSub;
	if (toSubTotal == 0) return;
	// Very big seeds
	toSub = std::min((TBigUInt) _VBS, toSubTotal / 1000);
	toSubTotal -= 1000 * toSub;
	_VBS -= (TUInt) toSub;
	if (toSubTotal == 0) return;
	// Give the money back
	nlassert(_LS == 0);
	if (10 * (TBigUInt) _MS > toSubTotal) // enough medium seeds left ?
	{
		_MS -= (TUInt) ((toSubTotal / 10) + 1);
		_LS = (TUInt) (10 - (toSubTotal % 10)); // give back little seeds
		return;
	}
	toSubTotal -= 10 * _MS;
	_MS = 0;
	if (100 * (TBigUInt) _BS > toSubTotal) // enough big seeds left ?
	{
		_BS -= (TUInt) ((toSubTotal / 100) + 1);
		CSeeds overflow;
		CSeeds left;
		left.setTotal(100 - (toSubTotal % 100)); // give back medium & little seeds
		add(left, overflow);
		nlassert(overflow.getTotal() == 0);
		return;
	}
	toSubTotal -= 100 * _BS;
	_BS = 0;
	// there should be enough VBS left
	nlassert(1000 * (TBigUInt) _VBS > toSubTotal);
	_VBS -= (TUInt) ((toSubTotal / 1000) + 1);
	CSeeds overflow;
	CSeeds left;
	left.setTotal(1000 - (toSubTotal % 100)); // give back medium, little & big seeds
	add(left, overflow);
	nlassert(overflow.getTotal() == 0);
}


//=========================================================
void CSeeds::tradeSubtract(const CSeeds &other)
{
	tradeSubtract(other.getTotal());
}

//=========================================================
bool CSeeds::canTradeSubtract(const CSeeds &other) const
{
	return other.getTotal() <= getTotal();
}

//=========================================================
bool CSeeds::canTradeSubtract(TBigUInt rhs) const
{
	return rhs <= getTotal();
}

//=========================================================
void CSeeds::subtract(const CSeeds &other)
{
	nlassert(canSubtract(other));
	_LS -= other._LS;
	_MS -= other._MS;
	_BS -= other._BS;
	_VBS -= other._VBS;
}

//=========================================================
bool CSeeds::canSubtract(const CSeeds &other) const
{
	return _LS >= other._LS &&
		   _MS >= other._MS &&
		   _BS >= other._BS &&
   		   _VBS >= other._VBS;
}


//=========================================================
void CSeeds::setTotal(TBigUInt total)
{
	nlassert(total < (TBigUInt) MaxUIntValue * 1111);
	_LS = (TUInt) (total % 10);
	_MS = (TUInt) ((total / 10) % 10);
	_BS = (TUInt) ((total / 100) % 10);
	_VBS = (TUInt) (total / 1000);
}

//=========================================================
void CSeeds::optimize()
{
	setTotal(getTotal());
}




















