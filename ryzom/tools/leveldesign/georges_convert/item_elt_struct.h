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

#ifndef NLGEORGES_ITEM_ELT_STRUCT_H
#define NLGEORGES_ITEM_ELT_STRUCT_H

#include "item_elt.h"

namespace NLOLDGEORGES
{

class CMoldElt;
class CMoldEltDefine;
class CFormBodyEltStruct;

class CItemEltStruct : public CItemElt  
{
protected:
	CMoldEltDefine*	pmed;
	std::vector< CItemElt* > vpie;
	void Clear();

public:
	CItemEltStruct( CLoader* const _pl );
	virtual ~CItemEltStruct() NL_OVERRIDE;
	
	void BuildItem( CMoldElt* const _pme );
	
	virtual CStringEx GetFormula() const NL_OVERRIDE;
	virtual void FillParent( const CFormBodyElt* const _pfbe ) NL_OVERRIDE;
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe ) NL_OVERRIDE;
	virtual CItemElt* Clone() NL_OVERRIDE;
	virtual CFormBodyElt* BuildForm() NL_OVERRIDE;
	void BuildForm( CFormBodyEltStruct* const _pfbes );
	void BuildForm( CFormBodyEltStruct* const _pfbes, const std::vector< std::pair< CStringEx, CStringEx > >& _vsxparents );
	virtual unsigned int GetNbElt() const NL_OVERRIDE;
	virtual CItemElt* GetElt( const unsigned int _index ) const NL_OVERRIDE;
	virtual CItemElt* GetElt( const CStringEx sxname ) const NL_OVERRIDE;
	virtual bool SetModified( const unsigned int _index ) NL_OVERRIDE;
	virtual void SetModified( const bool _b ) NL_OVERRIDE;

	virtual unsigned int GetNbChild () NL_OVERRIDE;
	virtual CItemElt* GetChild (unsigned int _index) NL_OVERRIDE;
	unsigned int GetNbStructElt ();
	CItemElt* GetStructElt (unsigned int _index);
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_STRUCT_H
