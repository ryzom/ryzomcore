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



#ifndef RY_BYPASS_CHECK_FLAGS_H
#define RY_BYPASS_CHECK_FLAGS_H



namespace CHECK_FLAG_TYPE
{
	enum TCheckFlagType
	{
		WhileSitting = 0,
		InWater,
		OnMount,
		Fear,
		Sleep,
		Invulnerability,
		Stun,

		Unknown,
	};

	const std::string &toString(TCheckFlagType type);
	
	TCheckFlagType fromString(const std::string &str);
}

/**
 * CBypassCheckFlags used to enable/disable check for canEntityUseAction() method of CEntityBase
 * \author Fleury David
 * \author Nevrax France
 * \date 2004
 */
struct CBypassCheckFlags
{
public:
	union
	{
		uint8 RawFlags;

		struct
		{
			uint8	WhileSitting : 1;
			uint8	InWater : 1;
			uint8	OnMount : 1;
			uint8	Fear : 1;
			uint8	Sleep : 1; // = Mezz
			uint8	Invulnerability : 1;
			uint8	Stun : 1;

			uint8	Unused : 1;
		} Flags;
	};

	inline CBypassCheckFlags()
	{
		RawFlags = 0;
	}

	// set/reset flag from enum
	void setFlag( CHECK_FLAG_TYPE::TCheckFlagType type, bool on );

	static CBypassCheckFlags NoFlags;
};

#endif // RY_BYPASS_CHECK_FLAGS_H
