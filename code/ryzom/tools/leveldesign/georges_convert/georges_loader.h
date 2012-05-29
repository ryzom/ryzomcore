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

#ifndef NLGEORGES_LOADER_H
#define NLGEORGES_LOADER_H

#include "form_loader.h"
#include "mold_loader.h"

namespace NLOLDGEORGES
{

class CLoader  
{
	static bool _Initialized;
protected:
	CFormLoader fl;
	CMoldLoader ml;
	CStringEx sxworkdirectory;
	CStringEx sxrootdirectory;
	CStringEx WhereIs( const CStringEx &_sxdirectory, const CStringEx &_sxfilename );

public:
	CLoader();
	virtual ~CLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfullname );
	void LoadForm( CForm& _f, const CStringEx& _sxfullname, const CStringEx& _sxdate );
	void LoadSearchForm( CForm& _f, const CStringEx& _sxfilename );
	void LoadSearchForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate );
	void SaveForm( CForm& _f, const CStringEx& _sxfullename );
	CMoldElt* LoadMold( const CStringEx &_sxfilename );
	CMoldElt* LoadMold( const CStringEx &_sxfilename, const CStringEx &_sxdate ); 

	CStringEx WhereIsDfnTyp( const CStringEx &_sxfilename );
	CStringEx WhereIsForm( const CStringEx &_sxfilename );

	CStringEx GetWorkDirectory() const;
	CStringEx GetRootDirectory() const;

// interface:
	void SetWorkDirectory( const CStringEx &_sxworkdirectory );
	void SetRootDirectory( const CStringEx &_sxrootdirectory );
	void SetTypPredef( const CStringEx& _sxfilename, const std::vector< CStringEx >& _pvsx );

// temporaire:
	void MakeDfn( const CStringEx &_sxfullname, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvdefine = 0 );
	void MakeTyp( const CStringEx &_sxfullname, const CStringEx &_sxtype, const CStringEx &_sxformula, const CStringEx &_sxenum, const CStringEx &_sxlow, const CStringEx &_sxhigh, const CStringEx &_sxdefault, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvpredef = 0, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvparent = 0 );
};

} // NLGEORGES

#endif // NLGEORGES_LOADER_H
