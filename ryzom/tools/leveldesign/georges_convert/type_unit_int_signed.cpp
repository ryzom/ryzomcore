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

#include "stdgeorgesconvert.h"
#include "type_unit_int_signed.h"
#include <nel/misc/common.h>

using namespace NLMISC;


namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitIntSigned::CTypeUnitIntSigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxll, _sxhl, _sxdv, _sxf )
{
	ilowlimit = atoiInt64( sxlowlimit.c_str() ); 
	ihighlimit = atoiInt64( sxhighlimit.c_str() ); 
	idefaultvalue = atoiInt64( sxdefaultvalue.c_str() ); 
	if( sxformula.empty() )
		sxformula = CStringEx( "sint(" +_sxll +"," +_sxhl +")" );
}

CTypeUnitIntSigned::~CTypeUnitIntSigned()
{
}

CStringEx CTypeUnitIntSigned::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	std::vector< std::pair< CStringEx, CStringEx > > modificationValues;
	CStringEx value( _sxvalue );
	value.purge();
	while( value[0] == '<' )
	{
		std::string::size_type pos = value.find( '>' );
		if( pos == std::string::npos )
			break;
		CStringEx sxoperateur = value.get_mid( 1, 1 );
		CStringEx sxoperande = value.get_mid( 2, pos-2);
		value.right( (int)(value.size()-pos-1) );
		modificationValues.push_back( std::make_pair( sxoperateur, sxoperande ) );
	}
	if( modificationValues.size() )
	{
		CStringEx sxr;
		for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = modificationValues.begin(); it != modificationValues.end(); ++it )
		{
			sxr += CStringEx( "<" );
			sxr += it->first;
			sxr += it->second;
			sxr += CStringEx( ">" );
		}
		return( sxr );
	}
	else
	{
		sint64 ivalue = atoiInt64( _sxvalue.c_str() );
		if( ivalue < ilowlimit )
			ivalue = ilowlimit;
		if( ivalue > ihighlimit )
			ivalue = ihighlimit;

		char pc[256];
		itoaInt64( ivalue, pc, 10 );
		return( CStringEx( pc ) );
	}
}
									
CStringEx CTypeUnitIntSigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );

	std::vector< std::pair< CStringEx, CStringEx > > modificationValues;
	CStringEx value( _sxvalue );
	value.purge();
	while( value[0] == '<' )
	{
		std::string::size_type pos = value.find( '>' );
		if( pos == std::string::npos )
			break;
		CStringEx sxoperateur = value.get_mid( 1, 1 );
		CStringEx sxoperande = value.get_mid( 2, (int)pos-2);
		value.right( (int)(value.size()-pos-1) );
		modificationValues.push_back( std::make_pair( sxoperateur, sxoperande ) );
	}
	if( modificationValues.size() )
	{
		sint64 ir = atoiInt64( _sxbasevalue.c_str() );
		for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = modificationValues.begin(); it != modificationValues.end(); ++it )
		{
			sint64 ivalue = atoiInt64( it->second.c_str() );
			if( it->first == "+" )
				ir += ivalue;
			else if( it->first == "*" )
					ir *= ivalue;
				else if( it->first == "-" )
						ir -= ivalue;
					else if( it->first == "/" )
							ir /= ivalue;   
						else if( it->first == "^" )
								ir = (sint64)( pow( (double)(ir), (double)(ivalue) ) );   
		}
		if( ir < ilowlimit )
			ir = ilowlimit;
		if( ir > ihighlimit )
			ir = ihighlimit;
		char pc[256];
		itoaInt64( ir, pc, 10 );
		return( CStringEx( pc ) );
	}
	else
	{
		sint64 ivalue = atoiInt64( _sxvalue.c_str() );
		if( ivalue < ilowlimit )
			ivalue = ilowlimit;
		if( ivalue > ihighlimit )
			ivalue = ihighlimit;
		char pc[256];
		itoaInt64( ivalue, pc, 10 );
		return( CStringEx( pc ) );
	}

	return( Format( _sxvalue ) );
}

void CTypeUnitIntSigned::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
	idefaultvalue = atoiInt64( sxdefaultvalue.c_str() ); 
}

void CTypeUnitIntSigned::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
	ilowlimit = atoiInt64( sxlowlimit.c_str() ); 
}

void CTypeUnitIntSigned::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
	ihighlimit = atoiInt64( sxhighlimit.c_str() ); 
}

}
