// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef NL_ENUM_BIYSET_H
#define NL_ENUM_BIYSET_H

#include "types_nl.h"
#include "sstring.h"

namespace NLMISC
{
	template <class EnumType, typename BitsetType>
	struct TSimpleEnum
	{
		static BitsetType getValue (EnumType value)
		{
			return BitsetType(value);
		}
	};

	template <class EnumType, typename BitsetType>
	struct TContainedEnum
	{
		static BitsetType getValue (EnumType value)
		{
			return BitsetType(value.getValue());
		}
	};


	/** Utility to build 'ored' bit set from a 2 powered enum.
	 *	The class give to user a conprehensive interface for
	 *	dealing with 'ored' enum value.
	 *
	 *	The class not strictly check that the enum only contains
	 *	power of 2 values because the enum can eventualy
	 *	contains pre 'ored' values.
	 *	For each access, the size in bit of the enumerated value
	 *	passed to the class is checked to not oversize the
	 *	BitsetType capacity.
	 *	By default, the BitsetType is set to 32 bits, but you
	 *	can provide your own type to narrow or expand the
	 *	capacity.
	 *
	 *	You can add an additional checking by setting the maxValue to
	 *	the max value of the enumerated type.
	 *
	 *	For from/to string convertion, you can also provide a delimiter
	 *	char (that is comma by default).
	 *
	 *
	 *	usage:
	 *	enum foo
	 *	{
	 *		value1 = 1,
	 *		value2 = 2,
	 *		value3 = 4,
	 *		value4 = 8,
	 *		maxFoo
	 *	};
	 *
	 *	// Create the enum bit set, using enum foo, coded on an uint32 and limited
	 *	// to the value maxFoo in order to check for invalid values.
	 *	CEnumBitSet<foo, uint32, maxFoo>	myset;
	 *	myset.setEnumValue(value1);
	 *	myset.setEnumValue(value4);
	 *	myset.clearEnumValue(value4);
	 *
	 *	myset.checkEnumValue(value1);	// return true
	 *	myset.checkEnumValue(value2);	// return false
	 *	myset.checkEnumValue(value4);	// return false
	 */
	template <class EnumType,
				typename BitsetType = uint32,
				BitsetType maxValue = UINT_MAX,
				char Delimiter=',',
				class EnumAccessor = TSimpleEnum<EnumType, BitsetType>,
				class SimpleEnumType = EnumType
				>
	struct CEnumBitset
	{
		// Default constructor with no flag set
		CEnumBitset()
			: Bitset(0)
		{
		}

		// Constructor with one flag set
		CEnumBitset(EnumType value)
			: Bitset(0)
		{
			setEnumValue(value);
		}

		// Constructor with a enumerated string
		CEnumBitset(const std::string &valueList)
			: Bitset(0)
		{
			fromString(valueList);
		}

		/// Add a bit
		void addEnumValue(EnumType value)
		{
			nlwarning("Deprecated, please use setEnumValue");
			setEnumValue(value);
	//		nlassert(value < maxValue);
	//		Bitset |= value;
		}

		/// set a bit
		void setEnumValue(EnumType value)
		{
			nlassert(EnumAccessor::getValue(value) < maxValue);
			Bitset |= EnumAccessor::getValue(value);
		}

		/// clear a bit
		void clearEnumValue(EnumType value)
		{
			nlassert(EnumAccessor::getValue(value) < maxValue);
			Bitset &= ~(EnumAccessor::getValue(value));
		}

		bool checkEnumValue(EnumType value)
		{
			return (Bitset & EnumAccessor::getValue(value)) == EnumAccessor::getValue(value);
		}

		std::string toString() const
		{
			CSString result;

			std::string delim;
			delim += Delimiter;

			// count up to 64 bits
			BitsetType value=1;
			uint i=0;
			for (; i<64; value <<= 1, ++i)
			{
				if (Bitset & value)
				{
					if (!result.empty())
						result << delim;
					// this bit is set, add a string
					result << EnumType::toString(SimpleEnumType(value));
				}
			}

			return result;
		}

		bool fromString(const std::string &valueList)
		{
			std::vector<std::string> values;

			std::string delim;
			delim += Delimiter;

			NLMISC::explode(valueList, delim, values, true);

			for( uint i=0; i<values.size(); ++i)
			{
				setEnumValue(EnumType(values[i]));
			}

			return true;
		}

		bool operator == (const CEnumBitset &other) const
		{
			return Bitset == other.Bitset;
		}

		bool operator != (const CEnumBitset &other) const
		{
			return ! operator == (other);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(Bitset);
		}

		BitsetType		Bitset;
	};

} // namespace NLMISC


#endif // NL_ENUM_BIYSET_H
