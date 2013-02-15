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

#ifndef PEOPLE_PD_H
#define PEOPLE_PD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <vector>
#include <map>

// User #includes

namespace EGSPD
{

//
// Forward declarations
//



//
// Typedefs & Enums
//

/** TPeople
 * defined at game_share/pd_scripts/people.pds:6
 */
class CPeople
{

public:

	/// \name Enum values
	// @{

	enum TPeople
	{
		Humanoid = 0,
		Playable = 0,
		Fyros = 0,
		Matis = 1,
		Tryker = 2,
		Zorai = 3,
		EndPlayable = 4,
		Karavan = 4,
		Tribe = 5,
		Common = 6,
		EndHumanoid = 7,
		Creature = 7,
		Fauna = 7,
		Arma = 7,
		Balduse = 8,
		Bul = 9,
		Capryni = 10,
		Chonari = 11,
		Clapclap = 12,
		Cococlaw = 13,
		Cute = 14,
		Dag = 15,
		Diranak = 16,
		Estrasson = 17,
		Filin = 18,
		Frahar = 19,
		Gibbai = 20,
		Hachtaha = 21,
		Jungler = 22,
		Kakty = 23,
		Kalab = 24,
		Kami = 25,
		Kazoar = 26,
		Kitin = 27,
		Kitins = 28,
		Kitifly = 28,
		Kitihank = 29,
		Kitiharak = 30,
		Kitikil = 31,
		Kitimandib = 32,
		Kitinagan = 33,
		Kitinega = 34,
		Kitinokto = 35,
		EndKitins = 36,
		Lightbird = 36,
		Mektoub = 37,
		MektoubPacker = 38,
		MektoubMount = 39,
		Pucetron = 40,
		Regus = 41,
		Ryzerb = 42,
		Ryzoholo = 43,
		Ryzoholok = 44,
		Vampignon = 45,
		Varinx = 46,
		Yber = 47,
		Zerx = 48,
		race_c1 = 49,
		race_c2 = 50,
		race_c3 = 51,
		race_c4 = 52,
		race_c5 = 53,
		race_c6 = 54,
		race_c7 = 55,
		race_h1 = 56,
		race_h2 = 57,
		race_h3 = 58,
		race_h4 = 59,
		race_h5 = 60,
		race_h6 = 61,
		race_h7 = 62,
		race_h8 = 63,
		race_h9 = 64,
		race_h10 = 65,
		race_h11 = 66,
		race_h12 = 67,
		EndFauna = 68,
		Flora = 68,
		Cephaloplant = 68,
		Electroalgs = 69,
		Phytopsy = 70,
		SapEnslaver = 71,
		SpittingWeeds = 72,
		Swarmplants = 73,
		EndFlora = 74,
		Goo = 74,
		GooFauna = 74,
		GooArma = 74,
		GooBalduse = 75,
		GooBul = 76,
		GooCapryni = 77,
		GooChonari = 78,
		GooClapclap = 79,
		GooCococlaw = 80,
		GooCute = 81,
		GooDag = 82,
		GooDiranak = 83,
		GooEstrasson = 84,
		GooFilin = 85,
		GooFrahar = 86,
		GooGibbai = 87,
		GooHachtaha = 88,
		GooJungler = 89,
		GooKakty = 90,
		GooKalab = 91,
		GooKami = 92,
		GooKazoar = 93,
		GooKitifly = 94,
		GooKitihank = 95,
		GooKitiharak = 96,
		GooKitikil = 97,
		GooKitimandib = 98,
		GooKitin = 99,
		GooKitinagan = 100,
		GooKitinega = 101,
		GooKitinokto = 102,
		GooLightbird = 103,
		GooMektoub = 104,
		GooMektoubPacker = 105,
		GooMektoubMount = 106,
		GooPucetron = 107,
		GooRegus = 108,
		GooRyzerb = 109,
		GooRyzoholo = 110,
		GooRyzoholok = 111,
		GooVampignon = 112,
		GooVarinx = 113,
		GooYber = 114,
		GooZerx = 115,
		Goorace_c1 = 116,
		Goorace_c2 = 117,
		Goorace_c3 = 118,
		Goorace_c4 = 119,
		Goorace_c5 = 120,
		Goorace_c6 = 121,
		Goorace_c7 = 122,
		Goorace_h1 = 123,
		Goorace_h2 = 124,
		Goorace_h3 = 125,
		Goorace_h4 = 126,
		Goorace_h5 = 127,
		Goorace_h6 = 128,
		Goorace_h7 = 129,
		Goorace_h8 = 130,
		Goorace_h9 = 131,
		Goorace_h10 = 132,
		Goorace_h11 = 133,
		Goorace_h12 = 134,
		EndGooFauna = 135,
		GooPlant = 135,
		GooCephaloplant = 135,
		GooElectroalgs = 136,
		GooPhytopsy = 137,
		GooSapEnslaver = 138,
		GooSpittingWeeds = 139,
		GooSwarmplants = 140,
		EndGooPlant = 141,
		EndGoo = 141,
		EndCreature = 141,
		___TPeople_useSize = 142,
		Unknown = 142,
		EndPeople = 142,
	};

	// @}


public:

	/// \name Conversion methods
	// @{

	/**
	 * Use these methods to convert from enum value to string (and vice versa)
	 */

	static const std::string&		toString(TPeople v);
	static CPeople::TPeople			fromString(const std::string& v);

	// @}


private:

	/// \name Enum initialisation
	// @{

	static void						init();
	static bool						_Initialised;
	static std::string				_UnknownString;
	static std::vector<std::string>	_StrTable;
	static std::map<std::string, TPeople>	_ValueMap;

	// @}

};

/** TClassificationType
 * defined at game_share/pd_scripts/people.pds:186
 */
class CClassificationType
{

public:

	/// \name Enum values
	// @{

	enum TClassificationType
	{
		TypeHumanoid = 0,
		TypeHomin = 1,
		TypeDegenerated = 2,
		TypeFauna = 3,
		TypeFlora = 4,
		TypeDog = 5,
		TypeRunner = 6,
		TypeHorse = 7,
		TypeBird = 8,
		TypeKitin = 9,
		TypeLandKitin = 10,
		TypeFlyingKitin = 11,
		TypeFish = 12,
		TypeRyzomian = 13,
		TypeGreatRyzomian = 14,
		TypePachyderm = 15,
		TypeShellfish = 16,
		TypeKami = 17,
		TypeKaravan = 18,
		TypeAll = 19,
		___TClassificationType_useSize = 20,
		Unknown = 20,
		EndClassificationType = 20,
	};

	// @}


public:

	/// \name Conversion methods
	// @{

	/**
	 * Use these methods to convert from enum value to string (and vice versa)
	 */

	static const std::string&		toString(TClassificationType v);
	static CClassificationType::TClassificationType	fromString(const std::string& v);

	// @}


private:

	/// \name Enum initialisation
	// @{

	static void						init();
	static bool						_Initialised;
	static std::string				_UnknownString;
	static std::vector<std::string>	_StrTable;
	static std::map<std::string, TClassificationType>	_ValueMap;

	// @}

};


} // End of EGSPD


//
// Inline implementations
//

#include "people_pd_inline.h"

#endif
