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


#ifndef NLVARIANT_H
#define NLVARIANT_H

#include "nel/misc/stream.h"
#include <string>

#define VARIANT_VVAL_END 16

namespace NLMISC
{
	class CVariant : public IStreamable
	{
	public:

		enum EVarType
		{
			Float,
			Double,
			Int,
			UInt,
			String,
			Vector4,
			Matrix4
		};

		CVariant();
		~CVariant();

		std::string getClassName(){ return "CVariant"; }

		void serial( IStream &f );

		void setDouble( double d ){
			type = Double;
			uvalue.dval = d;
		}

		void setFloat( float f ){
			type = Float;
			uvalue.fval = f;
		}

		void setInt( int i ){
			type = Int;
			uvalue.ival = i;
		}

		void setUInt( unsigned int u ){
			type = UInt;
			uvalue.uval = u;
		}

		void setString( const std::string &s ){
			type = String;
			sval = s;
		}

		void setVector4( const float *v );
		void setMatrix4( const float *m );

		double toDouble() const{ return uvalue.dval; }
		float toFloat() const{ return uvalue.fval; }
		int toInt() const{ return uvalue.ival; }
		unsigned int toUInt() const{ return uvalue.uval; }
		std::string toString() const{ return sval; }
		void getVector4( float *v ) const;
		void getMatrix4( float *m ) const;

		bool valueAsString( std::string &s ) const;
		void fromString( const std::string &s, EVarType t );

		EVarType getType() const{ return type; }

	private:

		union{
			double dval;
			float  fval;
			int    ival;
			unsigned int uval;
			float vval[ VARIANT_VVAL_END ];
		}uvalue;

		std::string sval;

		EVarType type;
	};
}


#endif

