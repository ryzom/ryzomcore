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

#ifndef NL_PERSISTANT_DATA_SERVICE_H
#define NL_PERSISTANT_DATA_SERVICE_H

#include "nel/misc/types_nl.h"
#include "nel/net/service.h"


/**
 * Persistant Data Service Class
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CPersistantDataService : public NLNET::IService
{
public:

	/// Constructor
	CPersistantDataService();



	/// Initialization
	virtual void	init();

	/// Release
	virtual void	release();

	/// Update
	virtual bool	update();

};


#endif // NL_PERSISTANT_DATA_SERVICE_H

/* End of persistant_data_service.h */
