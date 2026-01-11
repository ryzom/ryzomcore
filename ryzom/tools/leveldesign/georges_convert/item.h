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

#ifndef NLGEORGES_ITEM_H
#define NLGEORGES_ITEM_H

#include "string_ex.h"
#include "form.h"

namespace NLOLDGEORGES
{

class CLoader;
class CItemElt;
class CItemEltAtom;
class CItemEltStruct;
class CItemEltList;

class CItem  
{
protected:
	CLoader* pl;
	CItemEltStruct* pitemes;
//	CItemEltList* pitemelparents;
//	CItemEltAtom* pitemeacomments;
	std::vector< std::pair< CStringEx, CStringEx > > vsxparents;
	CStringEx moldfilename;

public:
	CItem();
	virtual ~CItem();

	void Clear();
	void SetLoader( CLoader* const _pl );
	void New( const CStringEx& _sxdfnfilename );
	void Load( const CStringEx& _sxfilename );
	void Load( const CStringEx& _sxfilename, const CStringEx _sxdate );
	void Save( const CStringEx& _sxfilename );
	bool Update();

	// Convert CItem to a CForm (in is this)
	void MakeForm (CForm &out);
	// Convert CForm to CItem (out is this)
	void MakeItem (CForm &in);

	void SetCurrentValue( const unsigned int _index, const CStringEx s );
	uint GetNbElt() const;
	uint GetNbParents() const;
	uint GetNbElt( const unsigned int _index ) const;
	uint GetInfos( const unsigned int _index ) const;
	CStringEx GetName( const unsigned int _index ) const;
	CStringEx GetCurrentResult( const unsigned int _index ) const;
	CStringEx GetCurrentValue( const unsigned int _index ) const;
	CStringEx GetFormula( const unsigned int _index ) const;
	bool IsEnum( const unsigned int _index ) const; 
	bool IsPredef( const unsigned int _index ) const; 
	bool CanEdit( const unsigned int _index ) const; 
	void GetListPredef( const unsigned int _index, std::vector< CStringEx >& _vsx ) const;

	CItemElt* GetElt( const unsigned int _index ) const;
	CItemElt* GetElt( const CStringEx _sxname ) const;

	void AddList( const unsigned int _index ) const;
	void DelListChild( const unsigned int _index ) const;
	void VirtualSaveLoad();

	void AddParent( const unsigned int _index = 0);
	void DelParent( const unsigned int _index );
	void ClearParents ();
	CStringEx GetParent( const unsigned int _index ) const;
	void SetParent( const unsigned int _index, const CStringEx _sx );
	CStringEx GetActivity( const unsigned int _index ) const;
	void SetActivity( const unsigned int _index, const CStringEx _sx );
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_H
