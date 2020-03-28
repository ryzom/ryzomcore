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
#include "people_pd.h"

namespace EGSPD
{

static const struct { const char* Name; CPeople::TPeople Value; } TPeopleConvert[] =
{
	{ "Humanoid", CPeople::Humanoid },
	{ "Playable", CPeople::Playable },
	{ "Fyros", CPeople::Fyros },
	{ "Matis", CPeople::Matis },
	{ "Tryker", CPeople::Tryker },
	{ "Zorai", CPeople::Zorai },
	{ "EndPlayable", CPeople::EndPlayable },
	{ "Karavan", CPeople::Karavan },
	{ "Tribe", CPeople::Tribe },
	{ "Common", CPeople::Common },
	{ "EndHumanoid", CPeople::EndHumanoid },
	{ "Creature", CPeople::Creature },
	{ "Fauna", CPeople::Fauna },
	{ "Arma", CPeople::Arma },
	{ "Balduse", CPeople::Balduse },
	{ "Bul", CPeople::Bul },
	{ "Capryni", CPeople::Capryni },
	{ "Chonari", CPeople::Chonari },
	{ "Clapclap", CPeople::Clapclap },
	{ "Cococlaw", CPeople::Cococlaw },
	{ "Cute", CPeople::Cute },
	{ "Dag", CPeople::Dag },
	{ "Diranak", CPeople::Diranak },
	{ "Estrasson", CPeople::Estrasson },
	{ "Filin", CPeople::Filin },
	{ "Frahar", CPeople::Frahar },
	{ "Gibbai", CPeople::Gibbai },
	{ "Hachtaha", CPeople::Hachtaha },
	{ "Jungler", CPeople::Jungler },
	{ "Kakty", CPeople::Kakty },
	{ "Kalab", CPeople::Kalab },
	{ "Kami", CPeople::Kami },
	{ "Kazoar", CPeople::Kazoar },
	{ "Kitin", CPeople::Kitin },
	{ "Kitins", CPeople::Kitins },
	{ "Kitifly", CPeople::Kitifly },
	{ "Kitihank", CPeople::Kitihank },
	{ "Kitiharak", CPeople::Kitiharak },
	{ "Kitikil", CPeople::Kitikil },
	{ "Kitimandib", CPeople::Kitimandib },
	{ "Kitinagan", CPeople::Kitinagan },
	{ "Kitinega", CPeople::Kitinega },
	{ "Kitinokto", CPeople::Kitinokto },
	{ "EndKitins", CPeople::EndKitins },
	{ "Lightbird", CPeople::Lightbird },
	{ "Mektoub", CPeople::Mektoub },
	{ "MektoubPacker", CPeople::MektoubPacker },
	{ "MektoubMount", CPeople::MektoubMount },
	{ "Pucetron", CPeople::Pucetron },
	{ "Regus", CPeople::Regus },
	{ "Ryzerb", CPeople::Ryzerb },
	{ "Ryzoholo", CPeople::Ryzoholo },
	{ "Ryzoholok", CPeople::Ryzoholok },
	{ "Vampignon", CPeople::Vampignon },
	{ "Varinx", CPeople::Varinx },
	{ "Yber", CPeople::Yber },
	{ "Zerx", CPeople::Zerx },
	{ "race_c1", CPeople::race_c1 },
	{ "race_c2", CPeople::race_c2 },
	{ "race_c3", CPeople::race_c3 },
	{ "race_c4", CPeople::race_c4 },
	{ "race_c5", CPeople::race_c5 },
	{ "race_c6", CPeople::race_c6 },
	{ "race_c7", CPeople::race_c7 },
	{ "race_h1", CPeople::race_h1 },
	{ "race_h2", CPeople::race_h2 },
	{ "race_h3", CPeople::race_h3 },
	{ "race_h4", CPeople::race_h4 },
	{ "race_h5", CPeople::race_h5 },
	{ "race_h6", CPeople::race_h6 },
	{ "race_h7", CPeople::race_h7 },
	{ "race_h8", CPeople::race_h8 },
	{ "race_h9", CPeople::race_h9 },
	{ "race_h10", CPeople::race_h10 },
	{ "race_h11", CPeople::race_h11 },
	{ "race_h12", CPeople::race_h12 },
	{ "EndFauna", CPeople::EndFauna },
	{ "Flora", CPeople::Flora },
	{ "Cephaloplant", CPeople::Cephaloplant },
	{ "Electroalgs", CPeople::Electroalgs },
	{ "Phytopsy", CPeople::Phytopsy },
	{ "SapEnslaver", CPeople::SapEnslaver },
	{ "SpittingWeeds", CPeople::SpittingWeeds },
	{ "Swarmplants", CPeople::Swarmplants },
	{ "EndFlora", CPeople::EndFlora },
	{ "Goo", CPeople::Goo },
	{ "GooFauna", CPeople::GooFauna },
	{ "GooArma", CPeople::GooArma },
	{ "GooBalduse", CPeople::GooBalduse },
	{ "GooBul", CPeople::GooBul },
	{ "GooCapryni", CPeople::GooCapryni },
	{ "GooChonari", CPeople::GooChonari },
	{ "GooClapclap", CPeople::GooClapclap },
	{ "GooCococlaw", CPeople::GooCococlaw },
	{ "GooCute", CPeople::GooCute },
	{ "GooDag", CPeople::GooDag },
	{ "GooDiranak", CPeople::GooDiranak },
	{ "GooEstrasson", CPeople::GooEstrasson },
	{ "GooFilin", CPeople::GooFilin },
	{ "GooFrahar", CPeople::GooFrahar },
	{ "GooGibbai", CPeople::GooGibbai },
	{ "GooHachtaha", CPeople::GooHachtaha },
	{ "GooJungler", CPeople::GooJungler },
	{ "GooKakty", CPeople::GooKakty },
	{ "GooKalab", CPeople::GooKalab },
	{ "GooKami", CPeople::GooKami },
	{ "GooKazoar", CPeople::GooKazoar },
	{ "GooKitifly", CPeople::GooKitifly },
	{ "GooKitihank", CPeople::GooKitihank },
	{ "GooKitiharak", CPeople::GooKitiharak },
	{ "GooKitikil", CPeople::GooKitikil },
	{ "GooKitimandib", CPeople::GooKitimandib },
	{ "GooKitin", CPeople::GooKitin },
	{ "GooKitinagan", CPeople::GooKitinagan },
	{ "GooKitinega", CPeople::GooKitinega },
	{ "GooKitinokto", CPeople::GooKitinokto },
	{ "GooLightbird", CPeople::GooLightbird },
	{ "GooMektoub", CPeople::GooMektoub },
	{ "GooMektoubPacker", CPeople::GooMektoubPacker },
	{ "GooMektoubMount", CPeople::GooMektoubMount },
	{ "GooPucetron", CPeople::GooPucetron },
	{ "GooRegus", CPeople::GooRegus },
	{ "GooRyzerb", CPeople::GooRyzerb },
	{ "GooRyzoholo", CPeople::GooRyzoholo },
	{ "GooRyzoholok", CPeople::GooRyzoholok },
	{ "GooVampignon", CPeople::GooVampignon },
	{ "GooVarinx", CPeople::GooVarinx },
	{ "GooYber", CPeople::GooYber },
	{ "GooZerx", CPeople::GooZerx },
	{ "Goorace_c1", CPeople::Goorace_c1 },
	{ "Goorace_c2", CPeople::Goorace_c2 },
	{ "Goorace_c3", CPeople::Goorace_c3 },
	{ "Goorace_c4", CPeople::Goorace_c4 },
	{ "Goorace_c5", CPeople::Goorace_c5 },
	{ "Goorace_c6", CPeople::Goorace_c6 },
	{ "Goorace_c7", CPeople::Goorace_c7 },
	{ "Goorace_h1", CPeople::Goorace_h1 },
	{ "Goorace_h2", CPeople::Goorace_h2 },
	{ "Goorace_h3", CPeople::Goorace_h3 },
	{ "Goorace_h4", CPeople::Goorace_h4 },
	{ "Goorace_h5", CPeople::Goorace_h5 },
	{ "Goorace_h6", CPeople::Goorace_h6 },
	{ "Goorace_h7", CPeople::Goorace_h7 },
	{ "Goorace_h8", CPeople::Goorace_h8 },
	{ "Goorace_h9", CPeople::Goorace_h9 },
	{ "Goorace_h10", CPeople::Goorace_h10 },
	{ "Goorace_h11", CPeople::Goorace_h11 },
	{ "Goorace_h12", CPeople::Goorace_h12 },
	{ "EndGooFauna", CPeople::EndGooFauna },
	{ "GooPlant", CPeople::GooPlant },
	{ "GooCephaloplant", CPeople::GooCephaloplant },
	{ "GooElectroalgs", CPeople::GooElectroalgs },
	{ "GooPhytopsy", CPeople::GooPhytopsy },
	{ "GooSapEnslaver", CPeople::GooSapEnslaver },
	{ "GooSpittingWeeds", CPeople::GooSpittingWeeds },
	{ "GooSwarmplants", CPeople::GooSwarmplants },
	{ "EndGooPlant", CPeople::EndGooPlant },
	{ "EndGoo", CPeople::EndGoo },
	{ "EndCreature", CPeople::EndCreature }
};
/* -----------------------------------------
* Static Implementation of CPeople
* ----------------------------------------- */
void							CPeople::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(142);
	uint	i;
	for (i=0; i<159; ++i)
	{
		_StrTable[TPeopleConvert[i].Value] = TPeopleConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TPeopleConvert[i].Name))] = TPeopleConvert[i].Value;
	}
	_Initialised = true;
}
bool							CPeople::_Initialised = false;
std::string						CPeople::_UnknownString = "Unknown";
std::vector<std::string>		CPeople::_StrTable;
std::map<std::string, CPeople::TPeople>	CPeople::_ValueMap;
// End of static implementation of CPeople

static const struct { const char* Name; CClassificationType::TClassificationType Value; } TClassificationTypeConvert[] =
{
	{ "TypeHumanoid", CClassificationType::TypeHumanoid },
	{ "TypeHomin", CClassificationType::TypeHomin },
	{ "TypeDegenerated", CClassificationType::TypeDegenerated },
	{ "TypeFauna", CClassificationType::TypeFauna },
	{ "TypeFlora", CClassificationType::TypeFlora },
	{ "TypeDog", CClassificationType::TypeDog },
	{ "TypeRunner", CClassificationType::TypeRunner },
	{ "TypeHorse", CClassificationType::TypeHorse },
	{ "TypeBird", CClassificationType::TypeBird },
	{ "TypeKitin", CClassificationType::TypeKitin },
	{ "TypeLandKitin", CClassificationType::TypeLandKitin },
	{ "TypeFlyingKitin", CClassificationType::TypeFlyingKitin },
	{ "TypeFish", CClassificationType::TypeFish },
	{ "TypeRyzomian", CClassificationType::TypeRyzomian },
	{ "TypeGreatRyzomian", CClassificationType::TypeGreatRyzomian },
	{ "TypePachyderm", CClassificationType::TypePachyderm },
	{ "TypeShellfish", CClassificationType::TypeShellfish },
	{ "TypeKami", CClassificationType::TypeKami },
	{ "TypeKaravan", CClassificationType::TypeKaravan },
	{ "TypeAll", CClassificationType::TypeAll },
};
/* -----------------------------------------
* Static Implementation of CClassificationType
* ----------------------------------------- */
void							CClassificationType::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(20);
	uint	i;
	for (i=0; i<20; ++i)
	{
		_StrTable[TClassificationTypeConvert[i].Value] = TClassificationTypeConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TClassificationTypeConvert[i].Name))] = TClassificationTypeConvert[i].Value;
	}
	_Initialised = true;
}
bool							CClassificationType::_Initialised = false;
std::string						CClassificationType::_UnknownString = "Unknown";
std::vector<std::string>		CClassificationType::_StrTable;
std::map<std::string, CClassificationType::TClassificationType>	CClassificationType::_ValueMap;
// End of static implementation of CClassificationType


} // End of EGSPD
