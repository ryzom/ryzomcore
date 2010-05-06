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

#ifndef NLGEORGES_FORM_LOADER_H
#define NLGEORGES_FORM_LOADER_H

#include "string_ex.h"
#include "form_file.h"

namespace NLOLDGEORGES
{

// La classe CFormLoader est le point d'entrée des classes CForm pour charger une fiche.
// Il y a deux fonctions load:
//    Sans date: donne directement la dernière fiche historiquement parlant
//    Avec date: donne une fiche composée de la dernière additionnés des historiques postérieurs ou égaux à la date.
class CFormLoader  
{
protected:

public:
	CFormLoader();
	virtual ~CFormLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfilename );
	void LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveForm( CForm& _f, const CStringEx& _sxfilename );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_LOADER_H
