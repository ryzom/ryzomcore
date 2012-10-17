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




#ifndef NL_MIRROR_MISC_TYPES_H
#define NL_MIRROR_MISC_TYPES_H


#include <nel/misc/types_nl.h>
#include <nel/net/message.h>

#include "mirrored_data_set.h"

#include <map>

typedef std::list<CMirroredDataSet*> TDataSetList;

/**
 *
 */
struct TEntitiesCreatedInEntityType
{
	TEntitiesCreatedInEntityType( sint32 maxnb ) : MaxNb(maxnb), CurrentNb(0) {}

	sint32			MaxNb;
	sint32			CurrentNb;
	TDataSetList	DataSetList;
};


//typedef CHashMap< uint8, TEntitiesCreatedInEntityType, std::hash<uint> > TEntityTypesOwned;
typedef CHashMap< uint8, TEntitiesCreatedInEntityType> TEntityTypesOwned;
#define GET_ENTITY_TYPE_OWNED(it) ((*it).second)


/*
 * Dataset map
 */
class TNDataSets : public CHashMap< std::string, CMirroredDataSet*>
{
public:

	/// Operator [] which does not create an object when the key is not found, but throws EMirror()
	CMirroredDataSet& operator [] ( const std::string& key )
	{
		iterator ids = find( key );
		if ( ids != end() )
		{
			return GET_NDATASET(ids);
		}
		else
		{
			throw EMirror();
		}
	}

	static CMirroredDataSet	*InvalidDataSet;
};


/*
 * TSDataSets map with keys as CSheetId
 */
typedef std::map<NLMISC::CSheetId, CMirroredDataSet> TSDataSets;


/*
 * Buffered mirror transport classes
 */
//typedef std::vector< std::pair<NLNET::CMessage, uint16> > CBufferedMirrorTCs;


#endif // NL_MIRROR_MISC_TYPES_H

/* End of mirror.h */
