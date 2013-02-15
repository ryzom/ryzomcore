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

#ifndef STDPCH_H
#define STDPCH_H



//----------------------------------------------------------------
// external files
//----------------------------------------------------------------

// this is up top because it contains a certain number of #pragmas to
// control compiler warnings with stlport

#include "nel/misc/types_nl.h"


//----------------------------------------------------------------
// std libs

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>


//----------------------------------------------------------------
// stl

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
//#include <sstream>
#include <exception>
#include <utility>
#include <deque>
#include <limits>
#include <iterator>


//----------------------------------------------------------------
// nel

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/random.h"
#include "nel/misc/smart_ptr.h"

#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"

#include "nel/net/message.h"
#include "nel/net/unified_network.h"

// NeL/ligo
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_configuration.h"


//----------------------------------------------------------------
// nel net
#include "nel/net/service.h"
//----------------------------------------------------------------
// service basics

#define	FOREACH(__itvar,__conttype,__contvar)	\
for (__conttype::iterator __itvar(__contvar.begin()),__itvar##end(__contvar.end()); __itvar!=__itvar##end; ++__itvar)

#define	FOREACH_NOINC(__itvar,__conttype,__contvar)	\
for (__conttype::iterator __itvar(__contvar.begin()),__itvar##end(__contvar.end()); __itvar!=__itvar##end;)

#define	FOREACHC(__itvar,__conttype,__contvar)	\
for (__conttype::const_iterator __itvar(__contvar.begin()),__itvar##end(__contvar.end()); __itvar!=__itvar##end; ++__itvar)

#define	FOREACHC_NOINC(__itvar,__conttype,__contvar)	\
for (__conttype::const_iterator __itvar(__contvar.begin()),__itvar##end(__contvar.end()); __itvar!=__itvar##end; )


class	CStringWriter
		:public	NLMISC::CRefCount
{
public:
	CStringWriter()
	{}
	virtual ~CStringWriter()
	{}
	virtual	void	append(const std::string	&str) = 0;
};

class	CTrashStringWriter
:public	CStringWriter
{
public:
	CTrashStringWriter(NLMISC::CLog *log=NLMISC::InfoLog)
	{}
	virtual ~CTrashStringWriter()
	{}
	void	append(const std::string	&str)
	{}
};

class	CLogStringWriter
	:public	CStringWriter
{
public:
	CLogStringWriter(NLMISC::CLog *log=NLMISC::InfoLog)
		:_Log(log)
	{}
	virtual ~CLogStringWriter()
	{}
	void	append(const std::string	&str)
	{
#if !FINAL_VERSION
		nlassert(_Log);
#endif
		if (_Log)
			_Log->displayNL(str.c_str());
	}
	NLMISC::CLog *_Log;
};


class	CArrayStringWriter
	:public	CStringWriter
{
public:
	CArrayStringWriter(std::vector<std::string>	&stringVector)
		:_StringVector(stringVector)
	{}
	virtual ~CArrayStringWriter()
	{}
	void	append(const std::string	&str)
	{
		_StringVector.push_back(str);
	}
	std::vector<std::string>	&_StringVector;
};


namespace MULTI_LINE_FORMATER {
	void pushTitle(std::vector<std::string>& container, std::string const& text);
	void pushEntry(std::vector<std::string>& container, std::string const& text);
	void pushFooter(std::vector<std::string>& container);
}

//----------------------------------------------------------------
// game share

#include "game_share/ryzom_entity_id.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/player_visual_properties.h"
#include "ai_share/ai_event.h"
#include "server_share/msg_ai_service.h"

//----------------------------------------------------------------
// ai share


#include "ai_share/ai_share.h"
#include "ai_share/ai_types.h"
#include "ai_share/ai_alias_description_node.h"
#include "ai_share/ai_event_description.h"
#include "ai_share/ai_coord.h"
#include "ai_share/ai_vector.h"
#include "ai_share/angle.h"
#include "ai_share/world_map.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

#endif /*STDPCH_H*/
