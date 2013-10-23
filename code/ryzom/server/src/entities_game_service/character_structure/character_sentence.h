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



#ifndef RY_CHARACTER_SENTENCE_H
#define RY_CHARACTER_SENTENCE_H


typedef std::vector<NLMISC::CSheetId>	TSheetIdVector;

/// Class for sentences
class CCharacterSentence
{
public:
	std::string					Name;
	TSheetIdVector				BricksIds;
	std::vector<uint8>			BricksIndexInSentence;
	
	/// Serialisation
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( Name );
		f.serialCont( BricksIds );
		f.serialCont( BricksIndexInSentence );
	}
};

static bool operator==(const CCharacterSentence &a, const CCharacterSentence &b)
{
	return a.BricksIds == b.BricksIds;
}


#endif // RY_CHARACTER_SENTENCE_H
/* character_sentence.h */
