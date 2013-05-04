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
#include "type_unit_double.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitDouble::CTypeUnitDouble( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxll, _sxhl, _sxdv, _sxf )
{
	dlowlimit = atof( sxlowlimit.c_str() ); 
	dhighlimit = atof( sxhighlimit.c_str() ); 
	ddefaultvalue = atof( sxdefaultvalue.c_str() ); 
	if( sxformula.empty() )
		sxformula.format( "double(%s,%s)", FormatDouble( dlowlimit ).c_str(), FormatDouble( dhighlimit ).c_str() );
}

CTypeUnitDouble::~CTypeUnitDouble()
{
}

CStringEx CTypeUnitDouble::FormatDouble( const double dvalue ) const
{
	int  decimal, sign;
	char *buffer;

	if( dvalue == 0 )
		return( CStringEx( "0.0" ));

#ifdef NL_OS_WINDOWS
	buffer = _fcvt( dvalue, 5, &decimal, &sign );
#else
	buffer = fcvt( dvalue, 5, &decimal, &sign ); 
#endif // NL_OS_WINDOWS
	CStringEx sx( buffer );
	if( decimal <= 0 )
	{
		sx = (std::string)(CStringEx( "0." ) + CStringEx( '0', -decimal ) + sx);
	}
	else
		sx.insert(decimal,".");

	while( sx[sx.length()-1] == '0' )
		sx.left( (int)sx.length() -1 );
	
	if( sx[sx.length()-1] == '.' )
		sx += '0';
	
	if( sign )
		sx = CStringEx( "-" + sx );

	return sx;
}

/*
CStringEx CTypeUnitDouble::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	double dvalue = atof( _sxvalue.c_str() );
	if( dvalue < dlowlimit )
		dvalue = dlowlimit;
	if( dvalue > dhighlimit )
		dvalue = dhighlimit;
	return( FormatDouble( dvalue ) );
}
*/
CStringEx CTypeUnitDouble::Format( const CStringEx _sxvalue ) const
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
		CStringEx sxoperande = value.get_mid( 2, (int)pos-2);
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
		double dvalue = atof( _sxvalue.c_str() );
		if( dvalue < dlowlimit )
			dvalue = dlowlimit;
		if( dvalue > dhighlimit )
			dvalue = dhighlimit;
		return( FormatDouble( dvalue ) );
	}
}
/*									
CStringEx CTypeUnitDouble::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	return( Format( _sxvalue ) );
}
*/
CStringEx CTypeUnitDouble::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
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
		double dr = atof( _sxbasevalue.c_str() );
		for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = modificationValues.begin(); it != modificationValues.end(); ++it )
		{
			double dvalue = atof( it->second.c_str() );
			if( it->first == "+" )
				dr += dvalue;
			else if( it->first == "*" )
					dr *= dvalue;
				else if( it->first == "-" )
						dr -= dvalue;
					else if( it->first == "/" )
							dr /= dvalue;   
						else if( it->first == "^" )
								dr = pow( dr, dvalue );   
		}
		if( dr < dlowlimit )
			dr = dlowlimit;
		if( dr > dhighlimit )
			dr = dhighlimit;
		return( FormatDouble( dr ) );
	}
	else
	{
		double dvalue = atof( _sxvalue.c_str() );
		if( dvalue < dlowlimit )
			dvalue = dlowlimit;
		if( dvalue > dhighlimit )
			dvalue = dhighlimit;
		return( FormatDouble( dvalue ) );
	}

	return( Format( _sxvalue ) );
}

void CTypeUnitDouble::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
	ddefaultvalue = atof( sxdefaultvalue.c_str() ); 
}

void CTypeUnitDouble::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
	dlowlimit = atof( sxlowlimit.c_str() ); 
}

void CTypeUnitDouble::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
	dhighlimit = atof( sxhighlimit.c_str() ); 
}

}
