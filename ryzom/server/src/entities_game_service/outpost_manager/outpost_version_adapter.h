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


#ifndef OUTPOST_VERSION_ADAPTER_H
#define OUTPOST_VERSION_ADAPTER_H

class COutpost;

/**
 * Singleton class used to adapt different version of COutpost
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class COutpostVersionAdapter
{	
	NL_INSTANCE_COUNTER_DECL(COutpostVersionAdapter);
public:

	/// get singleton instance
	static inline COutpostVersionAdapter * getInstance()
	{
		if (_Instance == NULL)
			_Instance = new COutpostVersionAdapter();

		return _Instance;
	}

	/// get current version number
	uint32 currentVersionNumber() const;

	/// adapt outpost from given version
	void adaptOutpostFromVersion(COutpost & outpost, uint32 version) const;

private:
	/// adapter methods
	//void adaptToVersion1(COutpost & outpost) const;

private:
	/// singleton instance
	static COutpostVersionAdapter * _Instance;
};

#endif // OUTPOST_VERSION_ADAPTER_H
