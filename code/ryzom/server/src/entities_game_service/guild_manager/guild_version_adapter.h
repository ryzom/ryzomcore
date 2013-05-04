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

#ifndef RY_GUILD_VERSION_ADAPTER_H
#define RY_GUILD_VERSION_ADAPTER_H

class CGuild;
/**
 * class used to adapt previous guild versions to new ones
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildVersionAdapter
{	

public:
	/// getInstance
	static inline CGuildVersionAdapter *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CGuildVersionAdapter();
		
		return _Instance;
	}
	/// get current version number
	uint32 currentVersionNumber() const;

	/// adapt character from given version
	void adaptGuildFromVersion( CGuild &guild ) const;
	
private:
	/// adapter methods
	void adaptToVersion1(CGuild &guild) const;
	void adaptToVersion2(CGuild &guild) const;
	void adaptToVersion3(CGuild &guild) const;
	void adaptToVersion4(CGuild &guild) const;

private:
	CGuildVersionAdapter(){}
	/// unique instance
	static CGuildVersionAdapter*			_Instance;
};

#endif // RY_GUILD_VERSION_ADAPTER_H

/* End of guild_version_adapter.h */
