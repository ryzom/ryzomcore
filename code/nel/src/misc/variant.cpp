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


#include "nel/misc/variant.h"
#include <sstream>


namespace NLMISC
{
	CVariant::CVariant()
	{
		std::fill( uvalue.vval, uvalue.vval + VARIANT_VVAL_END, 0.0f );
		type = String;
	}

	CVariant::~CVariant()
	{
	}

	void CVariant::serial( IStream &f )
	{
		uint v = f.serialVersion( 1 );

		f.xmlPush( "type" );

		if( !f.isReading() )
		{
			uint t = type;
			f.serial( t );
		}
		else
		{
			uint t;
			f.serial( t );
			type = EVarType( t );
		}

		f.xmlPop();

		f.xmlPush( "value" );

		switch( type )
		{
		case Double:
			{
				f.serial( uvalue.dval );
				break;
			}

		case Float:
			{
				f.serial( uvalue.fval );
				break;
			}

		case Int:
			{
				f.serial( uvalue.ival );
				break;
			}

		case UInt:
			{
				f.serial( uvalue.uval );
				break;
			}

		case String:
			{
				f.serial( sval );
				break;
			}

		}


		if( !f.isReading() )
		{
			switch( type )
			{
			case Vector4:
				{
					float fval;
					for( int i = 0; i < 4; i++ )
					{
						fval = uvalue.vval[ i ];
						f.serial( fval );
					}
					break;
				}

			case Matrix4:
				{
					float fval;
					for( int i = 0; i < 16; i++ )
					{
						fval = uvalue.vval[ i ];
						f.serial( fval );
					}
					break;
				}
			}
			
		}
		else
		{
			switch( type )
			{

			case Vector4:
				{
					float fval;
					for( int i = 0; i < 4; i++ )
					{
						f.serial( fval );
						uvalue.vval[ i ] = fval;
					}
					break;
				}

			case Matrix4:
				{
					float fval;
					for( int i = 0; i < 16; i++ )
					{
						f.serial( fval );
						uvalue.vval[ i ] = fval;
					}
					break;
				}
			}
		}

		f.xmlPop();
	}

	void CVariant::setVector4( const float *v )
	{
		for( int i = 0; i < 4; i++ )
			uvalue.vval[ i ] = v[ i ];
		type = Vector4;
	}

	void CVariant::setMatrix4( const float *m )
	{
		for( int i = 0; i < 16; i++ )
			uvalue.vval[ i ] = m[ i ];
		type = Matrix4;
	}

	void CVariant::getVector4( float *v ) const
	{
		for( int i = 0; i < 4; i++ )
			v[ i ] = uvalue.vval[ i ];
	}

	void CVariant::getMatrix4( float *m ) const
	{
		for( int i = 0; i < 16; i++ )
			m[ i ] = uvalue.vval[ i ];
	}

	bool CVariant::valueAsString( std::string &s ) const
	{
		std::stringstream ss;

		switch( type )
		{
		case Double:
			{
				ss << uvalue.dval;
				s = ss.str();
				break;
			}

		case Float:
			{
				ss << uvalue.fval;
				s = ss.str();
				break;
			}

		case Int:
			{
				ss << uvalue.ival;
				s = ss.str();
				break;
			}

		case UInt:
			{
				ss << uvalue.uval;
				s = ss.str();
				break;
			}

		case String:
			{
				s = sval;
				break;
			}

		case Vector4:
			{
				for( int i = 0; i < 4; i++ )
					ss << uvalue.vval[ i ] << " ";
				s = ss.str();
				s.resize( s.size() - 1 );
				break;
			}

		case Matrix4:
			{
				for( int i = 0; i < 16; i++ )
					ss << uvalue.vval[ i ] << " ";
				s = ss.str();
				s.resize( s.size() - 1 );
				break;
			}

		default:
			{
				return false;
				break;
			}
		}

		return true;
	}


	void CVariant::fromString( const std::string &s, EVarType t )
	{
		type = t;
		sval = "";
		std::fill( uvalue.vval, uvalue.vval + VARIANT_VVAL_END, 0.0 );

		if( s.empty() )
			return;

		switch( t )
		{
		case Double:
			{
				uvalue.dval = strtod( s.c_str(), NULL );
				break;
			}

		case Float:
			{
				uvalue.fval = strtod( s.c_str(), NULL );
				break;
			}

		case Int:
			{
				uvalue.ival = strtod( s.c_str(), NULL );
				break;
			}

		case UInt:
			{
				uvalue.uval = strtod( s.c_str(), NULL );
				break;
			}

		case String:
			{
				sval = s;
				break;
			}

		case Vector4:
			{
				std::stringstream ss( s );

				for( int i = 0; i < 4; i++ )
				{
					ss >> uvalue.vval[ i ];
					if( !ss.good() )
						break;
				}

				break;
			}

		case Matrix4:
			{
				std::stringstream ss( s );

				for( int i = 0; i < 16; i++ )
				{
					ss >> uvalue.vval[ i ];
					if( !ss.good() )
						break;
				}
				break;
			}
		}

	}

}



