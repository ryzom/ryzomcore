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



#ifndef EGS_STATIC_EMOT_H
#define EGS_STATIC_EMOT_H

//Nel georges
#include "nel/georges/u_form.h"


/**
 * class used to store emote animation list
 * \author Jerome Vuarand
 * \author Nevrax France
 * \date 2005
 */
class CStaticEmot
{
public:
	/// ctor
	CStaticEmot() { }

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serialCont( _Anims );
		if (f.isReading())
			buildAnimIdMap();
	}
	
	uint16 getAnimIndex(const std::string& animId) const
	{
		std::map<std::string, size_t>::const_iterator it = _AnimIdMap.find(animId);
		if (it!=_AnimIdMap.end())
			return (uint16)it->second;
		return (uint16)~0;
	}
	
	/// Removed
	void removed() { }
	
private:
	void buildAnimIdMap();
	
private:		
	/// all emote animations, ordered by (behav) integer id
	std::vector< std::string > _Anims;
	/// mapping of string id to integer id (index in _Anims)
	std::map<std::string, size_t> _AnimIdMap;
};



#endif
