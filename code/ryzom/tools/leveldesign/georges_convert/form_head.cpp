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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdgeorgesconvert.h"
#include "form_head.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormHead::CFormHead()
{
	sxuser = "Anonymous";
	sxversion = "0.0";
	sxdate = "unknow";
}

CFormHead::CFormHead( const CFormHead& _fh )
{
	sxuser = _fh.sxuser;
	sxversion = _fh.sxversion;
	sxdate = _fh.sxdate;
}

CFormHead::~CFormHead()
{
}

CStringEx CFormHead::GetUser() const
{
	return( sxuser );
}

CStringEx CFormHead::GetVersion() const
{
	return( sxversion );
}

CStringEx CFormHead::GetDate() const
{
	return( sxdate );
}

void CFormHead::SetUser( const CStringEx _sxuser )
{
	sxuser = _sxuser;
}

void CFormHead::SetVersion( const CStringEx _sxversion )
{
	sxversion = _sxversion;
}

void CFormHead::SetDate( const CStringEx _sxdate )
{
	sxdate = _sxdate;
}

void CFormHead::serial( NLMISC::IStream& s )
{

	s.xmlPushBegin( "Head" );
		s.xmlSetAttrib( "User_name" );
			s.serial( sxuser );
		s.xmlSetAttrib( "Version_number" );
			s.serial( sxversion );
		s.xmlSetAttrib( "Date" );
			s.serial( sxdate );
	s.xmlPushEnd();
	s.xmlPop();
}

CFormHead&	CFormHead::operator =( const CFormHead& _fh )
{
	sxuser = _fh.sxuser;
	sxversion = _fh.sxversion;
	sxdate = _fh.sxdate;
	return( *this );
}

}