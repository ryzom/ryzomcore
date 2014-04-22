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



#ifndef SEEDS_H
#define SEEDS_H


/** Weight of a seed in kilograms (all kind of seeds have the same weight)
  */
const double SeedWeigth = 0.01;

/** Money of Ryzom (seeds)
  * There are 4 kind of seeds :
  * - little seeds (LS)
  * - medium seeds (MS). One MS is worth 10 LS.
  * - big seeds (BS). One BS is worth 10 MS.
  * - very big seeds (VBS). One VBS is worth 10 BS.
  *
  * Overflow is managed.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSeeds
{
public:
	typedef uint32 TUInt;    // type of integer to count number of seeds for a slot.
	typedef uint64 TBigUInt; // type of integer to count the total number of seeds.
	enum { MaxUIntValue = INT_MAX };
public:
	// ctruct with the given quantity of money
	CSeeds(TUInt ls = 0, TUInt ms = 0, TUInt bs = 0, TUInt vbs = 0) : _LS(ls), _MS(ms), _BS(bs), _VBS(vbs)
	{
	}
	// set the given total by using the smallest number of seeds
	void  setTotal(TBigUInt total);
	// gets
	TUInt getLS() const { return _LS; }
	TUInt getMS() const { return _MS; }
	TUInt getBS() const { return _BS; }
	TUInt getVBS() const { return _VBS; }

	// sets
	void setLS(TUInt quantity)  { _LS = quantity; }
	void setMS(TUInt quantity)  { _MS = quantity; }
	void setBS(TUInt quantity) { _BS = quantity; }
	void setVBS(TUInt quantity)  { _VBS = quantity; }

	// get money total (expressed in little seeds)
	TBigUInt getTotal() const { return (TBigUInt) _LS + 10 * (TBigUInt) _MS + 100 * (TBigUInt) _BS + 1000 * (TBigUInt) _VBS; }
	/** Add a number of seeds.
	  * If an overflow is detected, only the maximum value is added, & overflow is filled with the difference
	  */
	void     add(const CSeeds &other, CSeeds &overflow);
	/** Subtract a number of seeds. Begin to subtract the littlest seeds.
	  * Always works provided that the total of the subtracted seeds is < to the total of the target.
	  * Should test it yourself otherwise an assert is raised.
	  * This is typically used when you buy something to a merchant
	  */
	void     tradeSubtract(const CSeeds &other);
	void     tradeSubtract(TBigUInt rhs);
	//
	bool     canTradeSubtract(const CSeeds &other) const;
	bool     canTradeSubtract(TBigUInt rhs) const;
	//
	// Subtract the given quantity to this obj on a per seeds basis. Must ensure that it is possible
	void     subtract(const CSeeds &other);
	// Test if the given quantity can be subtracted to this obj
	bool     canSubtract(const CSeeds &other) const;
	// Optimize a quantity of money so that the number of seeds is minimum
	void	 optimize();
	// serial
	void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
	{
		f.serial( _LS, _MS, _BS, _VBS);
	}
	// get the weight of all seeds
	double getWeight() const { return SeedWeigth * (_LS + _MS + _BS + _VBS); }
/////////////////////////////////////////////////////
private:
	TUInt _LS;   // little seeds
	TUInt _MS;   // medium seeds
	TUInt _BS;   // big seeds
	TUInt _VBS;  // very big seeds
};





#endif




















