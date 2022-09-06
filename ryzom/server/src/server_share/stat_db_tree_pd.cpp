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


#include "stdpch.h"
#include "stat_db_tree_pd.h"


using namespace std;
using namespace NLMISC;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


#define PERSISTENT_MACROS_AUTO_UNDEF

// ****************************************************************************
#define PERSISTENT_CLASS CStatDBValueLeafPD

#define PERSISTENT_DATA\
	PROP(string,Path)\
	PROP(sint32,Value)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

// ****************************************************************************
#define PERSISTENT_CLASS CStatDBTableLeafPD

#define PERSISTENT_DATA\
	PROP(string,Path)\
	PROP_MAP(CEntityId,sint32,PlayerValues)\
	PROP_MAP(EGSPD::TGuildId,sint32,GuildValues)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

// ****************************************************************************
#define PERSISTENT_CLASS CStatDBValueLeavesPD

#define PERSISTENT_DATA\
	STRUCT_VECT(ValueLeavesPD)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
