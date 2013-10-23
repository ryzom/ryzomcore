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


#ifndef CL_UNBLOCK_TITLES_SHEET_H
#define CL_UNBLOCK_TITLES_SHEET_H

// Application
#include "entity_sheet.h"
// Game Share
#include "game_share/skills.h"
#include "game_share/character_title.h"

class CUnblockTitlesSheet : public CEntitySheet
{
public :

	struct STitleUnblock
	{
		bool										Reserved;			// Title unblock only by server message
		std::vector<std::vector<std::string> >		SkillsNeeded;
		std::vector<std::vector<sint32> >			SkillsLevelNeeded;
		std::vector<NLMISC::CSheetId>				BricksNeeded;
		std::vector<uint32>							MinFames;
		std::vector<sint32>							MinFameLevels;
		std::vector<uint32>							MaxFames;
		std::vector<sint32>							MaxFameLevels;
		std::string									CivNeeded;
		std::string									CultNeeded;
		std::string									CharOldness;
		std::string									CharPlayedTime;
		std::string									AccountOldness;
		uint32										AuthorRating;
		uint32										AMRating;
		uint32										OrganizerRating;
		std::vector<std::vector<NLMISC::CSheetId> >	ItemsNeeded;
		std::vector<std::vector<sint32> >			ItemsQualityNeeded;

		// -----------------------------------------------------

		STitleUnblock() : Reserved(false)
		{
			AuthorRating = 0;
			AMRating = 0;
			OrganizerRating = 0;
		}

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			uint i,j;

			f.serial (Reserved);

			// skills
			uint16 size;
			if (!f.isReading())
				size = uint16(SkillsNeeded.size());
			f.serial (size);
			if (f.isReading())
				SkillsNeeded.resize (size);
			for (i = 0; i < size; ++i)
			{
				uint16 sizeTmp;
				if (!f.isReading())
					sizeTmp = uint16(SkillsNeeded[i].size());
				f.serial(sizeTmp);
				if (f.isReading())
					SkillsNeeded[i].resize (sizeTmp);
				for (j = 0; j < sizeTmp; ++j)
					f.serial (SkillsNeeded[i][j]);
			}

			if (!f.isReading())
				size = uint16(SkillsLevelNeeded.size());
			f.serial (size);
			if (f.isReading())
				SkillsLevelNeeded.resize (size);
			for (i = 0; i < size; ++i)
				f.serialCont (SkillsLevelNeeded[i]);

			// bricks
			if (!f.isReading())
				size = uint16(BricksNeeded.size());
			f.serial (size);
			if (f.isReading())
				BricksNeeded.resize (size);
			for (i = 0; i < size; ++i)
				f.serial (BricksNeeded[i]);

			// fames
			if (!f.isReading())
				size = uint16(MinFames.size());
			f.serial (size);
			if (f.isReading())
				MinFames.resize (size);
			for (i = 0; i < size; ++i)
				f.serial (MinFames[i]);

			if (!f.isReading())
				size = uint16(MinFameLevels.size());
			f.serial (size);
			if (f.isReading())
				MinFameLevels.resize (size);
			for (i = 0; i < size; ++i)
				f.serial (MinFameLevels[i]);

			if (!f.isReading())
				size = uint16(MaxFames.size());
			f.serial (size);
			if (f.isReading())
				MaxFames.resize (size);
			for (i = 0; i < size; ++i)
				f.serial (MaxFames[i]);

			if (!f.isReading())
				size = uint16(MaxFameLevels.size());
			f.serial (size);
			if (f.isReading())
				MaxFameLevels.resize (size);
			for (i = 0; i < size; ++i)
				f.serial (MaxFameLevels[i]);

			// civ & cult (allegiances)
			f.serial(CivNeeded);
			f.serial(CultNeeded);

			// char and account time properties
			f.serial(CharOldness);
			f.serial(CharPlayedTime);
			f.serial(AccountOldness);

			// ring ratings
			f.serial(AuthorRating);
			f.serial(AMRating);
			f.serial(OrganizerRating);

			// items
			if (!f.isReading())
				size = uint16(ItemsNeeded.size());
			f.serial (size);
			if (f.isReading())
				ItemsNeeded.resize (size);
			for (i = 0; i < size; ++i)
			{
				uint16 sizeTmp;
				if (!f.isReading())
					sizeTmp = uint16(ItemsNeeded[i].size());
				f.serial(sizeTmp);
				if (f.isReading())
					ItemsNeeded[i].resize (sizeTmp);
				for (j = 0; j < sizeTmp; ++j)
					f.serial (ItemsNeeded[i][j]);
			}

			if (!f.isReading())
				size = uint16(ItemsQualityNeeded.size());
			f.serial (size);
			if (f.isReading())
				ItemsQualityNeeded.resize (size);
			for (i = 0; i < size; ++i)
				f.serialCont (ItemsQualityNeeded[i]);
		}
	};


public:

	CUnblockTitlesSheet()
	{
		Type = UNBLOCK_TITLES;
	}
	/// destructor
	virtual ~CUnblockTitlesSheet() {}

	virtual void build(const NLGEORGES::UFormElm &item);

	/// serialize
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont(TitlesUnblock);
	}

public:

	std::vector< STitleUnblock > TitlesUnblock; // There is NB_CHARACTER_TITLE TitleUnblock
};

#endif // CL_UNBLOCK_TITLES_SHEET_H
