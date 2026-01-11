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

#ifndef RY_ADMIN_PROPERTIES_H
#define RY_ADMIN_PROPERTIES_H

#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_types.h"

class CCharacter;

/**
 * properties for CSR characters. Use the init method to init this property for CS characters
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CAdminProperties
{
	NL_INSTANCE_COUNTER_DECL(CAdminProperties);
public:

	CAdminProperties()
		:_Data(NULL){}
	~CAdminProperties()
	{
		if (_Data)
		{
			delete _Data;
			_Data = NULL;
		}
	}

	void init()
	{
		_Data = new CData;
	}

	void setMissionMonitoredUser(const TDataSetRow & row)
	{
		if ( !_Data ) return;
		_Data->MissionUser = row;
	}

	TDataSetRow getMissionMonitoredUser() const
	{
		if ( !_Data ) return TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW );
		return _Data->MissionUser;
	}

	CMission * getMission( uint indexInJournal ) const;

	static void updateCSRJournal( CCharacter * user, CMission * mission,uint8 idx );

private:
	struct CData
	{
		TDataSetRow MissionUser;
	};

	CData * _Data;
};


#endif // RY_ADMIN_PROPERTIES_H

/* End of admin_properties.h */
