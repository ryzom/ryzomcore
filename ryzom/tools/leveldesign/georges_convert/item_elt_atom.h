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

#ifndef NLGEORGES_ITEM_ELT_ATOM_H
#define NLGEORGES_ITEM_ELT_ATOM_H

#include "item_elt.h"

namespace NLOLDGEORGES
{

class CMoldElt;
class CMoldEltType;

class CItemEltAtom : public CItemElt  
{
protected:
	CMoldEltType*	pmet;

public:
	CItemEltAtom( CLoader* const _pl );
	virtual ~CItemEltAtom();
	
	void BuildItem( CMoldElt* const _pme );

	virtual CStringEx GetFormula() const;
	virtual void SetParentValue( const CStringEx _sxparentvalue );
	virtual void SetCurrentValue( const CStringEx _sxcurrentvalue );
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	virtual unsigned int GetNbElt() const;
	
	CMoldEltType* GetMoldType() const;

	virtual unsigned int GetNbChild ();
	virtual CItemElt* GetChild (unsigned int _index);
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_ATOM_H
