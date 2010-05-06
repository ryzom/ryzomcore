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

#ifndef NLGEORGES_MOLD_ELT_DEFINE_LIST_H
#define NLGEORGES_MOLD_ELT_DEFINE_LIST_H

#include "mold_elt_define.h"

namespace NLOLDGEORGES
{

class CLoader;

class CMoldEltDefineList : public CMoldEltDefine  
{
protected:
	CMoldEltDefine* pmed;

public:
	CMoldEltDefineList( CLoader* const _pl, CMoldEltDefine* const _pmed );
	virtual ~CMoldEltDefineList();

	virtual CMoldElt* GetMold(); 
	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
};

} // NLGEORGES

#endif // NLGEORGES_MOLD_ELT_DEFINE_LIST_H
