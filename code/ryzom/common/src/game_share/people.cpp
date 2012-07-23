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
#include "people.h"

using namespace std;

namespace EGSPD
{




BODY::TBodyType getBodyType(CPeople::TPeople people)
{
	/// TODO
	/// change the enum to optimize this test

	if ( people >= CPeople::Humanoid && people < CPeople::EndHumanoid )
		return BODY::Homin;

	if ( people >= CPeople::Flora && people <= CPeople::EndFlora )
		return BODY::Plant;

	// other races, use a switch case :(
	switch(people)
	{
	// birds
	case CPeople:: Kazoar:
	case CPeople:: Lightbird:
	case CPeople:: Yber:
	case CPeople:: race_c1:
	case CPeople:: GooKazoar:
	case CPeople:: GooLightbird:
	case CPeople:: GooYber:
		return BODY::Bird;

	// flying kitins
	case CPeople:: Kitifly:
	case CPeople:: Kitikil:
	case CPeople:: GooKitifly:
	case CPeople:: GooKitikil:
		return BODY::FlyingKitin;

	// humanoids, degenerated, kamis
	case CPeople:: Kalab:
	case CPeople:: Cute:
	case CPeople:: Gibbai:
	case CPeople:: Frahar:
	case CPeople:: Kami:
	case CPeople:: Tribe:
	case CPeople:: Karavan:
	case CPeople:: GooKalab:
	case CPeople:: GooCute:
	case CPeople:: GooGibbai:
	case CPeople:: GooFrahar:
		return BODY::Homin;

	// runners
	case CPeople:: Capryni:
	case CPeople:: Filin:
		return BODY::Quadruped;

	// dogs
	case CPeople:: Dag:
	case CPeople:: Chonari:
	case CPeople:: Jungler:
	case CPeople:: Regus:
	case CPeople:: Varinx:
		return BODY::Quadruped;

	// creature pass 2
	case CPeople:: race_h1:
	case CPeople:: race_h2:
	case CPeople:: race_h3:
	case CPeople:: race_h4:
	case CPeople:: race_h5:
	case CPeople:: race_h6:
	case CPeople:: race_h7:
	case CPeople:: race_h8:
	case CPeople:: race_h9:
	case CPeople:: race_h10:
	case CPeople:: race_h11:
	case CPeople:: race_h12:
	case CPeople:: race_c2:
	case CPeople:: race_c3:
	case CPeople:: race_c4:
	case CPeople:: race_c6:
	case CPeople:: race_c7:
	case CPeople:: Goorace_c1:
	case CPeople:: Goorace_c2:
	case CPeople:: Goorace_c3:
	case CPeople:: Goorace_c4:
	case CPeople:: Goorace_c5:
	case CPeople:: Goorace_c6:
	case CPeople:: Goorace_c7:
	case CPeople:: Goorace_h1:
	case CPeople:: Goorace_h2:
	case CPeople:: Goorace_h3:
	case CPeople:: Goorace_h4:
	case CPeople:: Goorace_h5:
	case CPeople:: Goorace_h6:
	case CPeople:: Goorace_h7:
	case CPeople:: Goorace_h8:
	case CPeople:: Goorace_h9:
	case CPeople:: Goorace_h10:
	case CPeople:: Goorace_h11:
	case CPeople:: Goorace_h12:
		return BODY::Quadruped;

	// "horses"
	case CPeople:: Mektoub :
	case CPeople:: MektoubPacker:
	case CPeople:: MektoubMount:
	case CPeople:: GooMektoub :
	case CPeople:: GooMektoubPacker:
	case CPeople:: GooMektoubMount:
		return BODY::Quadruped;

	// ryzomians
	case CPeople:: Kakty:
	case CPeople:: Ryzoholo:
	case CPeople:: Zerx:
	case CPeople:: GooKakty:
	case CPeople:: GooRyzoholo:
	case CPeople:: GooZerx:
		return BODY::Quadruped;

	// great ryzomian
	case CPeople:: Ryzerb:
	case CPeople:: Ryzoholok:
	case CPeople:: GooRyzerb:
	case CPeople:: GooRyzoholok:
		return BODY::Quadruped;

	// Pachyderms
	case CPeople:: Arma:
	case CPeople:: Bul:
	case CPeople:: Vampignon:
	case CPeople:: GooArma:
	case CPeople:: GooBul:
	case CPeople:: GooVampignon:
		return BODY::Quadruped;

	// Shellfish
	case CPeople:: Cococlaw:
	case CPeople:: Diranak:
	case CPeople:: Estrasson:
	case CPeople:: Hachtaha:
	case CPeople:: GooCococlaw:
	case CPeople:: GooDiranak:
	case CPeople:: GooEstrasson:
	case CPeople:: GooHachtaha:
		return BODY::LandKitin;

	// kitins
	case CPeople:: Kitihank:
	case CPeople:: Kitiharak:
	case CPeople:: Kitimandib:
	case CPeople:: Kitinagan:
	case CPeople:: Kitinega:
	case CPeople:: Kitinokto:
	case CPeople:: Pucetron:
	case CPeople:: GooKitihank:
	case CPeople:: GooKitiharak:
	case CPeople:: GooKitimandib:
	case CPeople:: GooKitin:
	case CPeople:: GooKitinagan:
	case CPeople:: GooKitinega:
	case CPeople:: GooKitinokto:
	case CPeople:: GooPucetron:
		return BODY::LandKitin;

	// fishs
	case CPeople:: Balduse:
	case CPeople:: Clapclap:
	case CPeople:: GooBalduse:
	case CPeople:: GooClapclap:
		return BODY::Fish;

	default:
		return BODY::UnknownBodyType;
	};
}


bool testClassificationType(CPeople::TPeople people, CClassificationType::TClassificationType type)
{
	if (type >= CClassificationType::EndClassificationType)
		return true;

	switch(people)
	{
	// birds
	case CPeople:: Kazoar:
	case CPeople:: Lightbird:
	case CPeople:: Yber:
	case CPeople:: race_c1:
	case CPeople:: GooKazoar:
	case CPeople:: GooLightbird:
	case CPeople:: GooYber:
	case CPeople:: Goorace_c1:
		return (type == CClassificationType::TypeBird);

	// flying kitins
	case CPeople:: Kitifly:
	case CPeople:: Kitikil:
	case CPeople:: GooKitifly:
	case CPeople:: GooKitikil:
		return (type == CClassificationType::TypeFlyingKitin || type == CClassificationType::TypeKitin);

	// humanoids
	case CPeople:: Kalab:
	case CPeople:: Fyros:
	case CPeople:: Matis:
	case CPeople:: Tryker:
	case CPeople:: Zorai:
	case CPeople:: Tribe:
	case CPeople:: GooKalab:
		return (type == CClassificationType::TypeHomin || type == CClassificationType::TypeHumanoid);

	case CPeople::Karavan:
		return (type == CClassificationType::TypeHomin || type == CClassificationType::TypeHumanoid || type == CClassificationType::TypeKaravan);

	// degenerated
	case CPeople:: Cute:
	case CPeople:: Gibbai:
	case CPeople:: Frahar:
	case CPeople:: GooCute:
	case CPeople:: GooGibbai:
	case CPeople:: GooFrahar:
		return (type == CClassificationType::TypeDegenerated || type == CClassificationType::TypeHumanoid);

	// runners
	case CPeople:: Capryni:
	case CPeople:: Filin:
	case CPeople:: GooCapryni:
	case CPeople:: GooFilin:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeRunner);

	// dogs
	case CPeople:: Dag:
	case CPeople:: Chonari:
	case CPeople:: Jungler:
	case CPeople:: Regus:
	case CPeople:: Varinx:
	case CPeople:: GooDag:
	case CPeople:: GooChonari:
	case CPeople:: GooJungler:
	case CPeople:: GooRegus:
	case CPeople:: GooVarinx:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeDog);

	// "horses"
	case CPeople:: Mektoub :
	case CPeople:: MektoubPacker:
	case CPeople:: MektoubMount:
	case CPeople:: GooMektoub :
	case CPeople:: GooMektoubPacker:
	case CPeople:: GooMektoubMount:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeHorse);

	// ryzomians
	case CPeople:: Kakty:
	case CPeople:: Ryzoholo:
	case CPeople:: Zerx:
	case CPeople:: GooKakty:
	case CPeople:: GooRyzoholo:
	case CPeople:: GooZerx:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeRyzomian);

	// great ryzomian
	case CPeople:: Ryzerb:
	case CPeople:: Ryzoholok:
	case CPeople:: GooRyzerb:
	case CPeople:: GooRyzoholok:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeGreatRyzomian);

	// Pachyderms
	case CPeople:: Arma:
	case CPeople:: Bul:
	case CPeople:: Vampignon:
	case CPeople:: GooArma:
	case CPeople:: GooBul:
	case CPeople:: GooVampignon:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypePachyderm);

	// Shellfish
	case CPeople:: Cococlaw:
	case CPeople:: Diranak:
	case CPeople:: Estrasson:
	case CPeople:: Hachtaha:
	case CPeople:: GooCococlaw:
	case CPeople:: GooDiranak:
	case CPeople:: GooEstrasson:
	case CPeople:: GooHachtaha:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeShellfish);

	// kitins
	case CPeople:: Kitihank:
	case CPeople:: Kitiharak:
	case CPeople:: Kitimandib:
	case CPeople:: Kitinagan:
	case CPeople:: Kitinega:
	case CPeople:: Kitinokto:
	case CPeople:: Pucetron:
	case CPeople:: race_c5:
	case CPeople:: GooKitihank:
	case CPeople:: GooKitiharak:
	case CPeople:: GooKitimandib:
	case CPeople:: GooKitin:
	case CPeople:: GooKitinagan:
	case CPeople:: GooKitinega:
	case CPeople:: GooKitinokto:
	case CPeople:: GooPucetron:
	case CPeople:: Goorace_c5:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeKitin || type == CClassificationType::TypeLandKitin);

	// fishs
	case CPeople:: Balduse:
	case CPeople:: Clapclap:
	case CPeople:: GooBalduse:
	case CPeople:: GooClapclap:
		return (type == CClassificationType::TypeFauna || type == CClassificationType::TypeFish);

	// flora
	case CPeople:: Cephaloplant:
	case CPeople:: Electroalgs:
	case CPeople:: Phytopsy:
	case CPeople:: SapEnslaver:
	case CPeople:: SpittingWeeds:
	case CPeople:: Swarmplants:
	case CPeople:: GooCephaloplant:
	case CPeople:: GooElectroalgs:
	case CPeople:: GooPhytopsy:
	case CPeople:: GooSapEnslaver:
	case CPeople:: GooSpittingWeeds:
	case CPeople:: GooSwarmplants:
		return (type == CClassificationType::TypeFlora);

	// Kami
	case CPeople::Kami:
		return (type == CClassificationType::TypeKami);

	//
	case CPeople:: race_h1:
	case CPeople:: race_h2:
	case CPeople:: race_h3:
	case CPeople:: race_h4:
	case CPeople:: race_h5:
	case CPeople:: race_h6:
	case CPeople:: race_h7:
	case CPeople:: race_h8:
	case CPeople:: race_h9:
	case CPeople:: race_h10:
	case CPeople:: race_h11:
	case CPeople:: race_h12:
	case CPeople:: race_c2:
	case CPeople:: race_c3:
	case CPeople:: race_c4:
	case CPeople:: race_c6:
	case CPeople:: race_c7:
	case CPeople:: Goorace_c2:
	case CPeople:: Goorace_c3:
	case CPeople:: Goorace_c4:
	case CPeople:: Goorace_c6:
	case CPeople:: Goorace_c7:
	case CPeople:: Goorace_h1:
	case CPeople:: Goorace_h2:
	case CPeople:: Goorace_h3:
	case CPeople:: Goorace_h4:
	case CPeople:: Goorace_h5:
	case CPeople:: Goorace_h6:
	case CPeople:: Goorace_h7:
	case CPeople:: Goorace_h8:
	case CPeople:: Goorace_h9:
	case CPeople:: Goorace_h10:
	case CPeople:: Goorace_h11:
	case CPeople:: Goorace_h12:
		return (type == CClassificationType::TypeFauna);

	default:
		return false;
	};
}

void getMatchingClassificationType(CPeople::TPeople people, vector<CClassificationType::TClassificationType> &types)
{
	types.clear();
	types.push_back(CClassificationType::TypeAll);

	switch(people)
	{
	// birds
	case CPeople:: Kazoar:
	case CPeople:: Lightbird:
	case CPeople:: Yber:
	case CPeople:: GooKazoar:
	case CPeople:: GooLightbird:
	case CPeople:: GooYber:
	case CPeople:: race_c1:
	case CPeople:: Goorace_c1:
		types.push_back(CClassificationType::TypeBird);
		break;

	// flying kitins
	case CPeople:: Kitifly:
	case CPeople:: Kitikil:
	case CPeople:: GooKitifly:
	case CPeople:: GooKitikil:
		types.push_back(CClassificationType::TypeFlyingKitin);
		types.push_back(CClassificationType::TypeKitin);
		break;

	// humanoids
	case CPeople:: Kalab:
	case CPeople:: Fyros:
	case CPeople:: Matis:
	case CPeople:: Tryker:
	case CPeople:: Zorai:
	case CPeople:: Tribe:
	case CPeople:: GooKalab:
		types.push_back(CClassificationType::TypeHomin);
		types.push_back(CClassificationType::TypeHumanoid);
		break;


	case CPeople::Karavan:
		types.push_back(CClassificationType::TypeHomin);
		types.push_back(CClassificationType::TypeHumanoid);
		types.push_back(CClassificationType::TypeKaravan);
		break;


	// degenerated
	case CPeople:: Cute:
	case CPeople:: Gibbai:
	case CPeople:: Frahar:
	case CPeople:: GooCute:
	case CPeople:: GooGibbai:
	case CPeople:: GooFrahar:
		types.push_back(CClassificationType::TypeDegenerated);
		types.push_back(CClassificationType::TypeHumanoid);
		break;

	// runners
	case CPeople:: Capryni:
	case CPeople:: Filin:
	case CPeople:: GooCapryni:
	case CPeople:: GooFilin:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeRunner);
		break;


	// dogs
	case CPeople:: Dag:
	case CPeople:: Chonari:
	case CPeople:: Jungler:
	case CPeople:: Regus:
	case CPeople:: Varinx:
	case CPeople:: GooDag:
	case CPeople:: GooChonari:
	case CPeople:: GooJungler:
	case CPeople:: GooRegus:
	case CPeople:: GooVarinx:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeDog);
		break;


	// "horses"
	case CPeople:: Mektoub :
	case CPeople:: MektoubPacker:
	case CPeople:: MektoubMount:
	case CPeople:: GooMektoub :
	case CPeople:: GooMektoubPacker:
	case CPeople:: GooMektoubMount:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeHorse);
		break;

	// ryzomians
	case CPeople:: Kakty:
	case CPeople:: Ryzoholo:
	case CPeople:: Zerx:
	case CPeople:: GooKakty:
	case CPeople:: GooRyzoholo:
	case CPeople:: GooZerx:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeRyzomian);
		break;

	// great ryzomian
	case CPeople:: Ryzerb:
	case CPeople:: Ryzoholok:
	case CPeople:: GooRyzerb:
	case CPeople:: GooRyzoholok:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeGreatRyzomian);
		break;

	// Pachyderms
	case CPeople:: Arma:
	case CPeople:: Bul:
	case CPeople:: Vampignon:
	case CPeople:: GooArma:
	case CPeople:: GooBul:
	case CPeople:: GooVampignon:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypePachyderm);
		break;

	// Shellfish
	case CPeople:: Cococlaw:
	case CPeople:: Diranak:
	case CPeople:: Estrasson:
	case CPeople:: Hachtaha:
	case CPeople:: GooCococlaw:
	case CPeople:: GooDiranak:
	case CPeople:: GooEstrasson:
	case CPeople:: GooHachtaha:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeShellfish);
		break;

	// kitins
	case CPeople:: Kitihank:
	case CPeople:: Kitiharak:
	case CPeople:: Kitimandib:
	case CPeople:: Kitinagan:
	case CPeople:: Kitinega:
	case CPeople:: Kitinokto:
	case CPeople:: Pucetron:
	case CPeople:: GooKitihank:
	case CPeople:: GooKitiharak:
	case CPeople:: GooKitimandib:
	case CPeople:: GooKitin:
	case CPeople:: GooKitinagan:
	case CPeople:: GooKitinega:
	case CPeople:: GooKitinokto:
	case CPeople:: GooPucetron:
	case CPeople:: race_c5:
	case CPeople:: Goorace_c5:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeKitin);
		types.push_back(CClassificationType::TypeLandKitin);
		break;

	// fishs
	case CPeople:: Balduse:
	case CPeople:: Clapclap:
	case CPeople:: GooBalduse:
	case CPeople:: GooClapclap:
		types.push_back(CClassificationType::TypeFauna);
		types.push_back(CClassificationType::TypeFish);
		break;

	// flora
	case CPeople:: Cephaloplant:
	case CPeople:: Electroalgs:
	case CPeople:: Phytopsy:
	case CPeople:: SapEnslaver:
	case CPeople:: SpittingWeeds:
	case CPeople:: Swarmplants:
	case CPeople:: GooCephaloplant:
	case CPeople:: GooElectroalgs:
	case CPeople:: GooPhytopsy:
	case CPeople:: GooSapEnslaver:
	case CPeople:: GooSpittingWeeds:
	case CPeople:: GooSwarmplants:
		types.push_back(CClassificationType::TypeFlora);
		break;

	// Kami
	case CPeople::Kami:
		types.push_back(CClassificationType::TypeKami);
		break;

	//
	case CPeople:: race_h1:
	case CPeople:: race_h2:
	case CPeople:: race_h3:
	case CPeople:: race_h4:
	case CPeople:: race_h5:
	case CPeople:: race_h6:
	case CPeople:: race_h7:
	case CPeople:: race_h8:
	case CPeople:: race_h9:
	case CPeople:: race_h10:
	case CPeople:: race_h11:
	case CPeople:: race_h12:
	case CPeople:: race_c2:
	case CPeople:: race_c3:
	case CPeople:: race_c4:
	case CPeople:: race_c6:
	case CPeople:: race_c7:
	case CPeople:: Goorace_c2:
	case CPeople:: Goorace_c3:
	case CPeople:: Goorace_c4:
	case CPeople:: Goorace_c6:
	case CPeople:: Goorace_c7:
	case CPeople:: Goorace_h1:
	case CPeople:: Goorace_h2:
	case CPeople:: Goorace_h3:
	case CPeople:: Goorace_h4:
	case CPeople:: Goorace_h5:
	case CPeople:: Goorace_h6:
	case CPeople:: Goorace_h7:
	case CPeople:: Goorace_h8:
	case CPeople:: Goorace_h9:
	case CPeople:: Goorace_h10:
	case CPeople:: Goorace_h11:
	case CPeople:: Goorace_h12:
		types.push_back(CClassificationType::TypeFauna);
		break;

	default:;
	};
}

};// namespace GSPEOPLE
