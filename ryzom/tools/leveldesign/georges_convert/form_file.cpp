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
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "form_file.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormFile::CFormFile()
{
}

CFormFile::~CFormFile()
{
}

void CFormFile::serial( NLMISC::IStream& s )
{
	s.xmlPush( "File_of_forms" );
		s.serialCheck( (uint32)'FORM' );
		s.serialVersion( 0 );
		s.serialCont( lform );
	s.xmlPop();
}

void CFormFile::Load( const CStringEx _sxfullname )
{
	NLMISC::CIFile fileIn;
	fileIn.open( CStringEx( _sxfullname ) );
	NLMISC::CIXml input;											
	input.init( fileIn );
	serial( input );
}																	// Exception if fileIn.close();

void CFormFile::Save( const CStringEx _sxfullname )
{
	lform.front().SetDate( "Temporary out of date" );
	NLMISC::COFile fileOut;
	fileOut.open( CStringEx( _sxfullname ) );
	NLMISC::COXml output;											
	output.init( &fileOut );
	serial( output );
	output.flush();
	fileOut.close();
}																	// Exception if fileOut.close();

void CFormFile::GetForm( CForm& _f ) const 
{
	_f = lform.front();												// The first form is copying
}

void CFormFile::GetForm( CForm& _f, const CStringEx& _sxdate ) const 
{
	std::list< CForm >::const_iterator cit = lform.begin();
	_f = *cit;														// The first form is copying
	while( (cit != lform.end())&&( _sxdate <= cit->GetDate() ) )
		_f += *(++cit);												// While history's date is after the date, add history's form.
}

void CFormFile::SetForm( CForm& _f ) 
{
	if( lform.empty() )					
	{
		lform.push_front( _f );
		return;
	}

	if( ( !_f.GetModified() )||( _f == lform.front() ) )
		return;														// It's the same! Don't save...

	CForm f = lform.front();										// Copy the old first form
	f -= _f;														// overwrite with differences
	lform.pop_front();												// Delete the old fist form
	lform.push_front( f );											// Replace it by differences
	lform.push_front( _f );											// Place the new form in first position
}

}