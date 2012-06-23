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

#ifndef NLGEORGES_MOLD_ELT_TYPE_H
#define NLGEORGES_MOLD_ELT_TYPE_H

#include "mold_elt.h"

namespace NLOLDGEORGES
{

class CLoader;
class CTypeUnit;

class CMoldEltType : public CMoldElt  
{
protected:
	CTypeUnit*	ptu;
	std::vector< std::pair< CStringEx, CStringEx > > vpredef;							
	CStringEx sxtype;
	CStringEx sxformula;
	CStringEx sxenum;

public:
	CMoldEltType( CLoader* const _pl );
	virtual ~CMoldEltType();

	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );

	virtual CMoldElt* GetMold(); 
	virtual	CStringEx	GetDefaultValue() const;															// Give the default value of the type
	virtual	CStringEx	CalculateResult( const CStringEx _sxvalue, const CStringEx _sxbasevalue ) const;	// Calculate the final result
	virtual	CStringEx	Format( const CStringEx _sxvalue ) const;											// Transform any value in the good format					
	virtual	CStringEx	GetPredefSubstitute( const CStringEx _sxdesignation ) const;
	virtual	CStringEx	GetPredefDesignation( const CStringEx _sxdesignation ) const;						// Give the value corresponding to the string
	virtual	CStringEx	GetPredefDesignation( const unsigned int _index ) const;
	virtual	CStringEx	GetFormula();																		// Give the formula of the type : int(0,100)
	virtual unsigned int GetType() const;																	// Give the type ( typ or dfn or nothing )	
	void SetTypPredef( const std::vector< CStringEx >& _pvsx );
	void Save();
};

} // NLGEORGES

#endif // NLGEORGES_MOLD_ELT_TYPE_H
