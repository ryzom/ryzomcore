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
#include "form_loader.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormLoader::CFormLoader()
{
}

CFormLoader::~CFormLoader()
{
}

void CFormLoader::LoadForm( CForm& _f, const CStringEx& _sxfilename )
{
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f );
	delete pff;
}

void CFormLoader::LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f, _sxdate );
	delete pff;
}

void CFormLoader::SaveForm( CForm& _f, const CStringEx& _sxfilename )
{
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->SetForm( _f );
	pff->Save( _sxfilename );
	delete pff;
}

}