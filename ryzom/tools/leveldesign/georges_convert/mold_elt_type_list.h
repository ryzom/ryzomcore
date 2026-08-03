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

#ifndef NLGEORGES_MOLD_ELT_TYPE_LIST_H
#define NLGEORGES_MOLD_ELT_TYPE_LIST_H

#include "mold_elt_type.h"

namespace NLOLDGEORGES
{

class CLoader;
class CTypeUnit;

class CMoldEltTypeList : public CMoldEltType  
{
protected:
	CMoldEltType* pmet;

public:
	CMoldEltTypeList( CLoader* const _pl, CMoldEltType* const _pmet );
	virtual ~CMoldEltTypeList() NL_OVERRIDE;

	virtual void Load( const CStringEx _sxfullname ) NL_OVERRIDE;
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate ) NL_OVERRIDE;

	virtual CMoldElt* GetMold() NL_OVERRIDE; 
	virtual CStringEx	GetDefaultValue() const NL_OVERRIDE;															// Give the default value of the type
	virtual CStringEx	CalculateResult( const CStringEx _sxvalue, const CStringEx _sxbasevalue ) const NL_OVERRIDE;	// Calculate the final result
	virtual CStringEx	Format( const CStringEx _sxvalue ) const NL_OVERRIDE;											// Transform any value in the good format					
	virtual CStringEx	GetPredefSubstitute( const CStringEx _sxdesignation ) const NL_OVERRIDE;
	virtual CStringEx	GetPredefDesignation( const CStringEx _sxdesignation ) const NL_OVERRIDE;						// Give the value corresponding to the string
	virtual CStringEx	GetPredefDesignation( const unsigned int _index ) const NL_OVERRIDE;
	virtual	CStringEx	GetFormula() NL_OVERRIDE;																		// Give the formula of the type : int(0,100)
	virtual unsigned int GetType() const NL_OVERRIDE;																	// Give the type ( typ or dfn or nothing )	
};

} // NLGEORGES

#endif // NLGEORGES_MOLD_ELT_TYPE_LIST_H
