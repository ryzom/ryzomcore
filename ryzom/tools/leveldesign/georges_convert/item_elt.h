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

#ifndef NLGEORGES_ITEM_ELT_H
#define NLGEORGES_ITEM_ELT_H

#include "string_ex.h"
#include "common.h"

namespace NLOLDGEORGES
{

class CLoader;
class CFormBodyElt;

class CItemElt  
{
protected:
	CLoader*	pl;
	unsigned int	infos;
	CStringEx		sxname;	
	CStringEx		sxparent;	
	CStringEx		sxcurrentresult;	
	CStringEx		sxcurrentvalue;	
	CStringEx		sxoldcurrentvalue;	
	CStringEx		sxparentresult;	
	CStringEx		sxparentvalue;	
	CStringEx		sxoldparentvalue;	
	CItemElt*		listparent;
	bool			bmodified;

public:
	CItemElt( CLoader* const _pl );
	virtual ~CItemElt();

	void SetName( const CStringEx _sxname );
	void AddInfos( const unsigned int _infos );
	void SetListParent( CItemElt* const _listparent );
	CItemElt* GetListParent() const;

	unsigned int GetInfos() const;
	CStringEx GetName() const;
	CStringEx GetParent() const;
	CStringEx GetParentResult() const;
	CStringEx GetParentValue() const;
	CStringEx GetCurrentResult() const;
	CStringEx GetCurrentValue() const;

	virtual CStringEx GetFormula() const;
	virtual void SetParentValue( const CStringEx _sxparentvalue );
	virtual void SetCurrentValue( const CStringEx _sxcurrentvalue );
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();

	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;
	virtual bool SetModified( const unsigned int _index );
	virtual void SetModified( const bool _b );

	virtual unsigned int GetNbChild ();
	virtual CItemElt* GetChild (unsigned int _index);
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_ELT_H
