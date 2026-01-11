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
#include "form_body_elt_struct.h"
#include "form_body_elt_list.h"
#include "form_body_elt_atom.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyEltStruct::CFormBodyEltStruct()
{
}

CFormBodyEltStruct::CFormBodyEltStruct( const CFormBodyEltStruct& _fbes )
{
	sxname = _fbes.sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = _fbes.vpbodyelt.begin(); it != _fbes.vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
}

CFormBodyEltStruct::~CFormBodyEltStruct()
{
	Clear();
}

void CFormBodyEltStruct::Clear()
{
	for( std::vector< CFormBodyElt* >::iterator it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( *it )
			delete *it;
	vpbodyelt.clear();
}

void CFormBodyEltStruct::serial( NLMISC::IStream& s)
{
	s.xmlPushBegin( "Body" );
		s.xmlSetAttrib( "Name" );
			s.serial( sxname );
	s.xmlPushEnd();
		s.serialContPolyPtr( vpbodyelt );
	s.xmlPop();
}

std::vector< CFormBodyElt* >::iterator CFormBodyEltStruct::Find( const CStringEx _sxname ) 
{
  std::vector< CFormBodyElt* >::iterator it;
	for( it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

std::vector< CFormBodyElt* >::const_iterator CFormBodyEltStruct::Find( const CStringEx _sxname ) const 
{
  std::vector< CFormBodyElt* >::const_iterator it;
	for( it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

CFormBodyElt& CFormBodyEltStruct::operator =( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	Clear();
	sxname = pfbes->sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator =( const CFormBodyEltStruct& _fbes )
{
	Clear();
	sxname = _fbes.sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = _fbes.vpbodyelt.begin(); it != _fbes.vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator +=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::iterator iu = Find( (*it)->GetName() );
		if( iu == vpbodyelt.end() )
		{
			vpbodyelt.push_back( (*it)->Clone() );
			iu = vpbodyelt.end();									
			--iu;
		}
		**iu += **it;	
	}
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator -=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::iterator iu = Find( (*it)->GetName() );
		if( iu == vpbodyelt.end() )
			continue;
		**iu -= **it;
		if( (*iu)->Empty() )
		{
			delete( *iu );
			vpbodyelt.erase( iu );
		}
	}
	return( *this );
}

bool CFormBodyEltStruct::operator ==( const CFormBodyElt& _fbe ) const
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	if( vpbodyelt.size() != pfbes->vpbodyelt.size() )
		return( false );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::const_iterator iu =  Find( (*it)->GetName() );
		if( ( iu == vpbodyelt.end() )||( !(**iu == **it) ) )
			return false;
	}
	return( true );
}

CFormBodyElt* CFormBodyEltStruct::Clone() const 
{
	return( new CFormBodyEltStruct( *this ) );
}

bool CFormBodyEltStruct::Empty() const
{
	return( vpbodyelt.empty() );
}

uint32 CFormBodyEltStruct::GetNbElt () const
{
	return (uint32)vpbodyelt.size();
}

CFormBodyElt* CFormBodyEltStruct::GetElt( const unsigned int _index ) const
{
	if( _index >= vpbodyelt.size() )
		return( 0 );
	return( vpbodyelt[_index] ); 
}

CFormBodyElt* CFormBodyEltStruct::GetElt( const CStringEx _sxname ) const
{
	std::vector< CFormBodyElt* >::const_iterator it = Find( _sxname );
	if( it == vpbodyelt.end() ) 
		return( 0 );
	return( *it );
}

CStringEx CFormBodyEltStruct::GetComment() const
{
	std::vector< CFormBodyElt* >::const_iterator it =  Find( SXCOMMENT );
	if( it == vpbodyelt.end() )
	  {
	    CStringEx object;
		return( object );
	  }							
	const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( *it );      
	nlassert( pfbea );
	return pfbea->GetValue();
}

void CFormBodyEltStruct::SetComment( const CStringEx _sxcomment )
{
	// todo
}

CStringEx CFormBodyEltStruct::GetParent( unsigned int _index ) const
{
	std::vector< CFormBodyElt* >::const_iterator it =  Find( SXPARENTS );
	if( it == vpbodyelt.end() )
	  {
	    CStringEx object;
		return( object );
	  }									
	const CFormBodyEltList* pfbel = dynamic_cast< CFormBodyEltList* >( *it );      
	if( !pfbel )
	  {
	    CStringEx object;
		return( object );
	  }									
	const CFormBodyEltStruct* pfbes = dynamic_cast< CFormBodyEltStruct* >( pfbel->GetElt( _index ) );      
	if( !pfbes )
	  {
	    CStringEx object;
		return( object );
	  }									
/*

	const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Activity" ) );      
	if( !pfbea )
		return( CStringEx() );
	CStringEx sxactivity = pfbea->GetValue();
	if( sxactivity != "true" )
		return( CStringEx(" ") );
*/
		const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Filename" ) );      
//	pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Filename" ) );
	if( !pfbea )
	  {
	    CStringEx object;
	    return( object );
	  }
	return pfbea->GetValue();

}

CStringEx CFormBodyEltStruct::GetActivity( unsigned int _index ) const
{
	std::vector< CFormBodyElt* >::const_iterator it =  Find( SXPARENTS );
	if( it == vpbodyelt.end() )
	  {
	    CStringEx object;
		return( object );
	  }									
	const CFormBodyEltList* pfbel = dynamic_cast< CFormBodyEltList* >( *it );      
	if( !pfbel )
	  {
	    CStringEx object;
		return( object );
	  }									
	const CFormBodyEltStruct* pfbes = dynamic_cast< CFormBodyEltStruct* >( pfbel->GetElt( _index ) );      
	if( !pfbes )
	  {
	    CStringEx object;
		return( object );
	  }									
	const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Activity" ) );      
	if( !pfbea )
	  {
	    CStringEx object;
		return( object );
	  }									
/*
	CStringEx sxactivity = pfbea->GetValue();
	if( sxactivity != "true" )
		return( CStringEx(" ") );
	pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Filename" ) );
	if( !pfbea )
		return( CStringEx() );
*/
	return pfbea->GetValue();
}

void CFormBodyEltStruct::AddElt( CFormBodyElt* const pfbe )
{
	if( !pfbe )
		return;
	vpbodyelt.push_back( pfbe );
}

}
