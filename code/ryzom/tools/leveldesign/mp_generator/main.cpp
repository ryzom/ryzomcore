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

#include "moulinette.h"
#include "utils.h"
#include "nel/misc/algo.h"

TRMItem currentDocItem;
CRMData SortableData;


// Preloaded Files
CSString	FamilyTypContent;
CSString	GroupTypContent;
CSString	WKContent;


// Assigne une nouvelle MP à une creature
void AssignerMP( const CSString& creatureName, const CSString& materialName )
{
	// on regarde si la créature est dégénérée ou non
	if ( ( creatureName.c_str()[3] != 'c' ) && ( creatureName.c_str()[3] != 'd' ) 
		&& ( creatureName.c_str()[3] != 'f' ) && ( creatureName.c_str()[3] != 'j' )
		&& ( creatureName.c_str()[3] != 'l' ) && ( creatureName.c_str()[3] != 'p' ) )
	{
	}
	else
	{
		// lecture du fichier d'assignement
		CSString fileName = toString( "%s//_%s_mp.creature", RAW_MATERIAL_ASSIGN.c_str(), creatureName.c_str() ); 
		CSString data;

		// création si le fichier n'existe pas
		if(!CFile::fileExists(fileName))
		{
			CSString	str;
			str = "<?xml version=\"1.0\"?>\r\n";
			str+= "<FORM Version=\"0.0\" State=\"modified\">\r\n";
			str+= "  <STRUCT>\r\n";
			str+= "    <STRUCT Name=\"Harvest\">\r\n";
			str+= "    </STRUCT>\r\n";
			str+= "  </STRUCT>\r\n";
			str+= "  <STRUCT/>\r\n";
			str+= "  <STRUCT/>\r\n";
			str+= "  <STRUCT/>\r\n";
			str+= "  <STRUCT/>\r\n";
			str+= "</FORM>\r\n";
			str.writeToFile( fileName );
		}

		// lecture
		data.readFromFile( fileName );

		if ( !data.contains( materialName.c_str() ) )
		{	
			// on recherche le premier numéro de MP non utilisé
			CSString str = data;
			int nb= 0;
			while ( str.contains( "Name=\"MP" ) )
			{
				str = str.splitFrom( "Name=\"MP" );
				nb = str.firstWord().atoi();
			}

			// on insère la nouvelle MP
			str = "      <STRUCT Name=\"MP";
			str += toString( "%d\">\r\n        <ATOM Name=\"AssociatedItem\"", nb+1 );
			str += toString( " Value=\"%s\"/>\r\n      </STRUCT>\r\n    </STRUCT>\r\n  </STRUCT>\r\n", materialName.c_str() );
			
			data = data.replace( "    </STRUCT>\r\n  </STRUCT>\r\n", str.c_str() );
			data.writeToFile( fileName );
		}
	}
}


// enregistrement des differentes  
// caractéristiques de chaque craft part
void LoadCraftParts()
{
	CSString data, ligne, info;
	data.readFromFile( ITEM_MP_PARAM_DFN );
	int index;

	printf( "-- LOADING CRAFT PARTS --\n" );
	do 
	{
		data = data.splitFrom( "Name=\"" );
		int index;
		index = data.c_str()[0] - 'A';
		craftParts[index].Desc = data.splitTo( "\"" );

	} while ( data.c_str()[0] != 'Z' );

	if ( ! CFile::fileExists( "rm_item_parts.csv" ) )
	{
		nlError( "rm_item_parts.csv not found\n");
		exit( 1 );
	}
	
	data.readFromFile( "rm_item_parts.csv" );
	
	while ( !data.empty() )
	{
		ligne = data.splitTo( "\n", true );
		
		// on recherche la ligne correspondant à notre craft part
		info = ligne.splitTo( ";", true );
		if ( !info.empty() )
		{
			index = info.c_str()[0] - 'A';

			for ( int i=0; i<6; i++ )
				ligne.splitTo( ";", true );

			// parcours des différentes caractèristiques
			for ( int i=0; i<NumMPStats; i++ )
			{
				info = ligne.splitTo( ";", true ).left(1);

				// si la cellule n'est pas vide, cette caractèristique
				// est générée par la craft part
				if ( ( info == "C" ) || ( info == "D" ) || ( info == "X" ) )
					craftParts[index].Carac[i] = true;
				else
					craftParts[index].Carac[i] = false;
			}
		}
	}
}


// Enregistre les noms des différents fichiers de creature dans le bestiaire
void LoadCreatureFiles()
{
	printf( "-- REGISTERING CREATURE FILES --\n" );
	CSString inputSheetPath = LEVEL_DESIGN_PATH + "leveldesign\\Game_elem\\Creature\\Fauna\\bestiary";
	CPath::addSearchPath( inputSheetPath, true, false );

	vector<string> files;
	// on parcours les fichiers dans le repertoire
	CPath::getPathContent ( inputSheetPath, true, false, true, files );

	for (uint32 i=0; i<files.size(); ++i)
	{
		string filename = files[i];
		string filebase = CFile::getFilenameWithoutExtension( filename );
		creatureFiles[ filebase ] = filename;
	}
}


// Enregistre pour chaque créature la liste des différents
// niveaux d'item à générer
void InitCreatureMP()
{
	CSString data, ligneN, ligneM;
	CSString nom, code;
	map<string,string>::const_iterator it;

	if ( ! CFile::fileExists( "creature_models.csv" ) )
	{
		nlError( "creature_models.csv not found\n");
		exit( 1 );
	}
		
	data.readFromFile( "creature_models.csv" );

	while ( !data.empty() )
	{
		ligneN = data.splitTo( "\n", true );
		ligneM = data.splitTo( "\n", true );
		
		// on vérifie que la ligne est valide
		if ( !ligneN.splitTo( ";", true ).empty() )
		{
			ligneM.splitTo( ";", true );

			// on se déplace jusqu'au données
			ligneN.splitTo( ";", true );
			ligneM.splitTo( ";", true );
			ligneN.splitTo( ";", true );
			ligneM.splitTo( ";", true );

			while ( !ligneN.empty() )
			{
				ListeCreatureMP listeCreatureMP;
				
				if ( ligneN.contains( ";" ) )
				{
					code = ligneN.splitTo( ";", true );
					nom = ligneM.splitTo( ";", true );
				}
				else
				{
					code = ligneN.firstWord();
					nom = ligneM.firstWord();
					ligneN = "";
				}

				// pour chaque nouveau nom de créature trouvé,
				// on enregistre les items à générer
				for ( char eco='a'; eco<='z'; eco++ )
				{
					for ( int level=0; level<10; level++ )
					{
						for ( int creatureLevel=1; creatureLevel<=8; creatureLevel++ )
						{
							CSString fileName = toString( "c%s%c%c%d", code.toLower().c_str(), eco, 
								                          'a' + level, creatureLevel );
							// on recherche si une créature correspond à ces données
							it = creatureFiles.find( fileName );

							if ( it != creatureFiles.end() )
							{			
								// si oui, on signale que cet item devra être généré
								CreatureMPItem* creatureMP = new CreatureMPItem;
								creatureMP->creatureLevel = creatureLevel;
								creatureMP->eco = eco;
								creatureMP->itemLevel = level;
								creatureMP->codeCreature = code;
								creatureMP->creatureFileName = fileName;
								listeCreatureMP.push_back( creatureMP );
							}	
						}
					}
				}

				itemsAGenerer[ nom ] = listeCreatureMP;
			}
		}
	}
}


// Retourne la qualité maximum pour un level d'item
int GetMaxQuality( int level )
{
	if ( level == 0 )
		return 50;

	return ( level * 50 );
}


// Retourne l'énergie maximum pour un level d'item
int GetStatEnergy( int level )
{
	if ( level == 0 )
		return 20;
	
	return ( 20 + ( level-1 ) * 15 );
}


// Retourne le nom de classe de l'item selon ses caractèristiques
void GetItemClass( int level, bool mission, bool creature, CSString& outClassName )
{
	static CSString missionClasses[] = { "Plain", "Average", "Prime", "Select", 
		                                 "Superb", "Magnificient" };

	static CSString nonMissionClasses[] = { "", "Basic", "Fine", "Choice", 
		                                    "Excellent", "Supreme" };

	if ( mission )
		outClassName = missionClasses[ level ];
	else if ( creature )
		outClassName = nonMissionClasses[ level+1 ];
	else
		outClassName = nonMissionClasses[ level ];
}


// Retourne la couleur de l'item
void GetItemColor( int color, char eco, int level, CSString& outStr )
{
	CSString commonColors [4] = { "Beige", "Green", "Turquoise", "Violet" };
	CSString rareColors [2] = { "Red", "Blue" };
	CSString primeRootColors [2] = { "White", "Black" };
	
	if ( eco == 'p' )
		outStr = primeRootColors[ color % 2 ];
	else if ( ( eco != 'c' ) && ( level >= 2 ) )
		outStr = rareColors[ color % 2 ];
	else if ( ( eco == 'c' ) && ( level >= 3 ) )
		outStr = rareColors[ color % 2 ];
	else
		outStr = commonColors[ color % 4 ];
} 


bool endsWith( const CSString& s, const CSString& substring )
{
	return (s.right( (uint)substring.size() ) == substring);
}


// Génére les noms d'un item
void GenerateItemNames( const CSString& nomMP, char eco, int level, bool mission, bool creature, CSString& outStr )
{
	CSString itemClass, prefix, singularWithNoPrefix;
	CSString ia = "a";

	GetItemClass( level, mission, creature, itemClass );
	currentDocItem.push( DtStatQuality, itemClass );

	if ( !mission )
	{
		if ( ( creature && ( level == 3 ) ) || ( !creature && ( level == 4 ) ) )
			ia = "an";
	}

	singularWithNoPrefix = itemClass + " ";

	// selection de l'eco-systeme
	switch ( eco )
	{
	case 'c' :
		break;

	case 'd' :
		singularWithNoPrefix += "Desert ";
		break;

	case 'f' :
		singularWithNoPrefix += "Forest ";
		break;

	case 'j' :
		singularWithNoPrefix += "Jungle ";
		break;

	case 'l' :
		singularWithNoPrefix += "Lake ";
		break;

	case 'p' :
		singularWithNoPrefix += "Prime Root ";
		break;
	}

	singularWithNoPrefix += nomMP;

	// Contenant
	if ( endsWith( nomMP, "Wood" ) )
		prefix = "Bundle";
	else if ( endsWith( nomMP, "Bark" ) || endsWith( nomMP, "Moss" ) || endsWith( nomMP, "Sawdust" ) ||
			  endsWith( nomMP, "Straw" ) || endsWith( nomMP, "Dust" ) || endsWith( nomMP, "Soil" ) ||
			  endsWith( nomMP, "Cereal" ) )
		prefix = "Handful";
	else if ( endsWith( nomMP, "Resin" ) || endsWith( nomMP, "Wax" ) )
		prefix = "Portion";
	else if ( endsWith( nomMP, "Whiskers" ) || endsWith( nomMP, "Hairs" ) )
		prefix = "Tuft";
	else if ( endsWith( nomMP, "Silk" ) )
		prefix = "Ball";
	else if ( endsWith( nomMP, "Sap" ) || endsWith( nomMP, "Residue" ) || endsWith( nomMP, "Honey" ) ||
			  endsWith( nomMP, "Blood" ) )
		prefix = "Phial";
	else if ( endsWith( nomMP, "Fruit" ) )
		prefix = "Piece";
	else if ( endsWith( nomMP, "Flesh" ) )
		prefix = "Morsel";
	else if ( endsWith( nomMP, "Saliva" ) )
		prefix = "Sample";
	else if ( endsWith( nomMP, "Pollen" ) || endsWith( nomMP, "Fiber" ) || endsWith( nomMP, "Amber" ) ||
			  endsWith( nomMP, "Leather" ) || endsWith( nomMP, "Oil" ) )
		ia = "some";
	else if ( endsWith( nomMP, "Pelvis" ) || endsWith( nomMP, "Eye" ) || endsWith( nomMP, "Spine" ) ||
			  endsWith( nomMP, "Hoof" ) || endsWith( nomMP, "Mandible" ) || endsWith( nomMP, "Claw") ||
			  endsWith( nomMP, "Tail" ) || endsWith( nomMP, "Trunk" ) || endsWith( nomMP, "Shell" ) ||
			  endsWith( nomMP, "Sting" ) || endsWith( nomMP, "Skin" ) || endsWith( nomMP, "Beak" ) ||
			  endsWith( nomMP, "Wing" ) || endsWith( nomMP, "Horn" ) || endsWith( nomMP, "Rostrum" ) ||
			  endsWith( nomMP, "Skull" ) || endsWith( nomMP, "Pistil" ) )
		prefix = "Fragment"; // number-limited creature objects

	if ( ! prefix.empty() )
	{
		outStr = prefix + " of " + singularWithNoPrefix;
		ia = "a";
	}
	else
		outStr = singularWithNoPrefix;

	CSString singular = outStr;
	currentDocItem.push( DtTitle, outStr );

	// A, The
	outStr += "\t" + ia + "\tthe\t";

	// Plural
	if ( prefix.empty() )
		outStr += singular + "s";
	else
		outStr += prefix + "s" + " of " + singularWithNoPrefix;

	outStr += "\t\tthe";
}


// Retourne le numéro de la famille passée en paramètre
int GetNumeroMP( const CSString& nomMP )
{
	CSString result;
	char buffer[100];
	char buffer2[100];
	int res;

	// *** Get the family number, and add it to faimly.typ if not already done
	// on recherche si la MP est présente
	// dans le fichier item_mp_family.typ
	sprintf( buffer, "%s\" Value=\"", nomMP.c_str() );
	result = FamilyTypContent.splitFrom( buffer );

	// si oui, on retourne son numéro de MP
	if ( !result.empty() )
		res = result.splitTo( "\"" ).atoi();
	else
	{
		// sinon, on génère un nouveau numéro :
		// on recupère le dernier numéro de MP (le max)
		result = FamilyTypContent.splitTo( "<LOG>" ).right(10);
		result.splitTo( "\"", true );
		result = result.splitTo( "\"" );

		// on ajoute 1 pour avoir un numéro non utilisé
		res = result.atoi() + 1;

		// on ajoute la nouvelle MP :
		// dans le fichier item_mp_family.typ
		sprintf( buffer, "  <DEFINITION Label=\"%s\" Value=\"%d\"/>\n<LOG>", nomMP.c_str(), res );
		FamilyTypContent= FamilyTypContent.replace( "<LOG>", buffer );
		FamilyTypContent.writeToFile( ITEM_MP_FAMILY_TYP );
	}

	// *** Add the text in wk.uxt (if not done)
	// Exist in wk.uxt ???
	sprintf( buffer, "mpfam%d\t", res );
	sprintf( buffer2, "mpfam%d ", res );
	// if not found
	if ( !WKContent.contains(buffer) && !WKContent.contains(buffer2) )
	{
		// add it at end
		sprintf( buffer, "mpfam%d\t\t\t[%s]\n\r\nmpgroup0", res, nomMP.c_str() );
		WKContent= WKContent.replace( "\r\nmpgroup0", buffer );
		WKContent.writeToFile( WK_UXT );
	}

	return res;
}


// Retourne le numéro du groupe passé en paramètre
int GetNumeroGroupe( const CSString& groupe )
{
	CSString result;
	char buffer[100];
	char buffer2[100];
	int res;

	// *** Get the group number, and add it to group.typ if not already done
	// on recherche si le groupe est présent
	// dans le fichier item_mp_group.typ
	sprintf( buffer, "%s\" Value=\"", groupe.c_str() );
	result = GroupTypContent.splitFrom( buffer );

	// si oui, on retourne son numéro de groupe
	if ( !result.empty() )
		res = result.splitTo( "\"" ).atoi();
	else
	{
		// sinon, on génère un nouveau numéro :
		// on recupère le dernier numéro de groupe (le max)
		result = GroupTypContent.splitTo( "<LOG>" ).right(10);
		result.splitTo( "\"", true );
		result = result.splitTo( "\"" );

		// on ajoute 1 pour avoir un numéro non utilisé
		res = result.atoi() + 1;

		// on ajoute la nouvelle MP :
		// dans le fichier item_mp_group.typ
		sprintf( buffer, "<DEFINITION Label=\"%s\" Value=\"%d\"/>\n<LOG>", groupe.c_str(), res );
		GroupTypContent= GroupTypContent.replace( "<LOG>", buffer );
		GroupTypContent.writeToFile( ITEM_MP_GROUPE_TYP );
	}


	// *** Add the text in wk.uxt (if not done)
	// Exist in wk.uxt ???
	sprintf( buffer, "mpgroup%d\t", res );
	sprintf( buffer2, "mpgroup%d ", res );
	// if not found
	if ( !WKContent.contains(buffer) && !WKContent.contains(buffer2) )
	{
		// add it at end
		sprintf( buffer, "mpgroup%d\t\t\t[%s]\n\r\nmpSource", res, groupe.c_str() );
		WKContent= WKContent.replace( "\r\nmpSource", buffer );
		WKContent.writeToFile( WK_UXT );
	}

	return res;
}


// Génère l'item parent pour une MP
void CreateParentSItem( int numMP, 
					    const CSString& nomMP,
						const CSString& groupe,
						bool dropOrSell,
						const CSString& icon,
						const CSString& overlay )
{
	CSString output;
	CSString outputFileName;

	// nom du fichier de sortie
	outputFileName = toString( "%s_parent\\_m%04d.sitem", MP_DIRECTORY.c_str(), numMP );

	// entete xml
	output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
	
	// basics
	output += "  <STRUCT>\n    <STRUCT Name=\"basics\">\n";
	output += "      <ATOM Name=\"Drop or Sell\" Value=\"";

	if ( !dropOrSell )
		// il s'agit d'un item de mission, non vendable
		output += "false\"/>\n";
	else
		// sinon, on peut le vendre
		output += "true\"/>\n";

	output += "      <ATOM Name=\"Bulk\" Value=\"0.5\"/>\n    </STRUCT>\n";

	// mp
	output += "    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"Family\" Value=\"";
	output += nomMP;
	output += "\"/>\n      <ATOM Name=\"Group\" Value=\"";
	output += groupe;
	output += "\"/>\n    </STRUCT>\n";

	// 3d
	output += "    <STRUCT Name=\"3d\">\n";
	
	if ( !icon.empty() )
	{
		output += "      <ATOM Name=\"icon\" Value=\"";
		output += icon;
		output += "\"/>\n";
	}

	if ( !overlay.empty() )
	{
		output += "      <ATOM Name=\"text overlay\" Value=\"";
		output += overlay;
		output += "\"/>\n";
	}

	output += "    </STRUCT>\n  </STRUCT>\n";

	// fin du fichier
	output += "  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";

	// écriture finale
	output.writeToFile( outputFileName );

}


// Remplit les différents informations concernant le craft par un item
void FillCraftData( char craft, int eco, int level, bool creature, int bestStat, 
				    int worstStat1, int worstStat2, CSString& outStr )
{
	CSString data;
	char buf[10];
	int index;
	static CSString carac[] = { "Durability", "Weight", "SapLoad", "DMG", "Speed",
			                    "Range", "DodgeModifier", "ParryModifier", 
					            "AdversaryDodgeModifier", "AdversaryParryModifier",
						        "ProtectionFactor", "MaxSlashingProtection",
						        "MaxBluntProtection", "MaxPiercingProtection",
						        "ElementalCastingTimeFactor", "ElementalPowerFactor",
						        "OffensiveAfflictionCastingTimeFactor", 
						        "OffensiveAfflictionPowerFactor", "HealCastingTimeFactor",
						        "HealPowerFactor", "DefensiveAfflictionCastingTimeFactor",
						        "DefensiveAfflictionPowerFactor",
								"AcidProtection",
								"ColdProtection",
								"FireProtection",
								"RotProtection",
								"ShockWaveProtection",
								"PoisonProtection",
								"ElectricityProtection",
								"DesertResistance",
								"ForestResistance",
								"LacustreResistance",
								"JungleResistance",
								"PrimaryRootResistance",
							};
	nlctassert((sizeof(carac)/sizeof(carac[0]))==NumMPStats);

	static int mediumStatsByStatQuality[] = { 20, 35, 50, 65, 80 };
	int stat, remaining, nbToRaise, ajout;
	int stats[NumMPStats];

	index = craft - 'A';

	outStr = "        <STRUCT Name=\"";
	outStr += craftParts[index].Desc;
	outStr += "\">\n";

	nbToRaise = 0;
	remaining = 0;
	ajout = 0;

	currentDocItem.push( DtCraftSlotName, craftParts[index].Desc.splitFrom( "(" ).splitTo( ")" ) );

	// enregistrements des stats de chaque
	// caractèristique
	for ( int i=0; i<NumMPStats; i++ )
	{
		if ( craftParts[index].Carac[i]  )
		{
			if ( !creature )
				stat = mediumStatsByStatQuality[ level-1 ];
			else
				stat = mediumStatsByStatQuality[ level ];
			
			// gestion des points forts/faibles de la MP
			if ( i == bestStat )
			{
				if ( worstStat2 == -1 )
					stat += 20;
				else
					stat += 40;

				if ( stat > 100 )
				{
					// si une des stats dépasse 100, les autres
					// stats sont augmentées
					remaining = stat - 100;
					stat = 100;
				}
			}
			else if ( ( i == worstStat1 ) || ( i == worstStat2 ) )
			{
				stat -= 20;

				// durabilité minimum
				if ( ( i == 0 ) && ( stat < 1 ) )
					stat = 1;
			}
			else
				nbToRaise++;

			stats[i] = stat;
		}
		else 
			stats[i] = -1;
	}

	if ( nbToRaise != 0 )
		ajout = remaining/nbToRaise;
	
	// ajout des informations pour chaque carac
	for ( int i=0; i<NumMPStats; i++ )
	{
		if ( stats[i] != -1 )
		{
			if ( ( i != bestStat ) && ( i != worstStat1 ) && ( i != worstStat2 ) )
				stats[i] += ajout;
		
			outStr += "          <ATOM Name=\"";
			outStr += carac[i];
			outStr += "\" Value=\"";
			
			sprintf( buf, "%d", stats[i] );

			outStr += buf;
			outStr += "\"/>\n";
		}
	}

	// CraftCivSpec en fonction de l'écosystème
	outStr += "          <ATOM Name=\"CraftCivSpec\" Value=\"";
	CSString craftCiv;

	switch ( eco )
	{
	case 'c' :
		outStr += "common";
		craftCiv = "All";
		break;

	case 'd' :
		outStr += "fyros";
		craftCiv = "Fyros";
		break;

	case 'f' :
		outStr += "matis";
		craftCiv = "Matis";
		break;

	case 'j' :
		outStr += "zorai";
		craftCiv = "Zorai";
		break;

	case 'l' :
		outStr += "tryker";
		craftCiv = "Tryker";
		break;

	case 'p' :
		outStr += "common";
		craftCiv = "All";
		break;
	}

	currentDocItem.push( DtCraftCivSpec, craftCiv );

	outStr += "\"/>\n        </STRUCT>\n";
}


// Création d'une fiche d'item
void CreateSheet( int numMP, const CSString& nomMP,
				  const CSString& code, char eco,  
				  int level, const MPCraftStats& craftStats,
				  bool specialItem = false, int variation = 1 )
{
	CSString output, directory, itemName, craftInfo, color;
	CSString outputFileName, ecoStr;
	char chaineNum[5];
	bool creature = ( code != "dxa" ) && ( code != "cxx" ) && ( code != "ixx" );

	// Creation du nom de fichier
	sprintf( chaineNum, "%04d", numMP );

	switch ( eco )
	{
	case 'd' :
		directory = "desert";
		ecoStr = "Desert";
		break;

	case 'f' :
		directory = "forest";
		ecoStr = "Forest";
		break;

	case 'j' :
		directory = "jungle";
		ecoStr = "Jungle";
		break;

	case 'l' :
		directory = "lacustre";
		ecoStr = "Lacustre";
		break;

	case 'p' :
		directory = "prime_roots";
		ecoStr = "PrimeRoots";
		break;

	default :
		directory = "common";
		ecoStr = "Common";
		eco = 'c';
		break;
	}

	if ( ( eco == 'c' ) && creature && ( craftStats.Craft.empty() ) )
		return;

	outputFileName = toString( "m%04d%s%c%c%02d.sitem", numMP, code.c_str(), eco, 
		                     'a' + level, variation );

	if ( craftStats.Craft.empty() )
	{
		CSString levelZone = toString( "%c", 'a' + level );
		currentDocItem.push( DtLevelZone, levelZone.toUpper() );
	}
	else
		currentDocItem.push( DtLevelZone, "-" );
	
	// remplissage des informations de la fiche
	output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
	output += "  <PARENT Filename=\"_m";
	output += eco;
	output += ".sitem\"/>\n  <PARENT Filename=\"_m";
	output += chaineNum;
	output += ".sitem\"/>\n";

	// le code est celui d'une créature, on doit donc rajouter le parent
	if ( creature )
	{
		output += "  <PARENT Filename=\"_m"; 
		output += code;
		output += ".sitem\"/>\n";
		creature = true;
	}

	output += "  <STRUCT>\n";


    output += "    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"MpColor\" Value=\"";
	
	// materiaux de missions toujours Beige
	if ( craftStats.Craft.empty() )
	{
		output += "Beige\"/>\n";
		if(craftStats.UsedAsCraftRequirement)
			output += "      <ATOM Name=\"UsedAsCraftRequirement\" Value=\"true\"/>\n";
	}
	else
	{
		// on récupère la couleur
		GetItemColor( craftStats.color, eco, level, color );
		output += color;
		output += "\"/>\n";

		currentDocItem.push( DtColor, color );

		// on ajoute les données de craft
		output += "      <STRUCT Name=\"MpParam\">\n";
		for ( uint i=0; i<craftStats.Craft.size(); i++ )
		{
			int bestStat, worstStat1, worstStat2;

			if ( i == 0 )
			{
				bestStat = craftStats.bestStatA;
				worstStat1 = craftStats.worstStatA1;
				worstStat2 = craftStats.worstStatA2;
			}
			else if ( i == 1 )
			{
				bestStat = craftStats.bestStatB;
				worstStat1 = craftStats.worstStatB1;
				worstStat2 = craftStats.worstStatB2;
			}
			else
			{
				bestStat = -1;
				worstStat1 = -1;
				worstStat2 = -1;
			}

			currentDocItem.push( DtProp, toString( "%c", craftStats.Craft.c_str()[i] ).c_str() );
			
			FillCraftData( craftStats.Craft.c_str()[i], eco, level, creature, bestStat, 
					   worstStat1, worstStat2, craftInfo );
			output += craftInfo;
		}

		output += "      </STRUCT>\n";
	}

	output += "      <ATOM Name=\"MaxQuality\" Value=\"";
	if ( creature || specialItem || ( code == "cxx" ) )
	{
		CSString maxQuality = toString( "%d", 250 ); 
		output += maxQuality;
		currentDocItem.push( DtMaxLevel, maxQuality );
	}
	else
	{
		CSString maxQuality = toString( "%d", GetMaxQuality( level ) );
		output += maxQuality;
		currentDocItem.push( DtMaxLevel, maxQuality );
	}

	output += "\"/>\n      <ATOM Name=\"StatEnergy\" Value=\"";

	CSString statEnergy;
	if ( ( variation == 2 ) && ( numMP == 695 ) ) // cas particulier pour le kitin trophy (beurk)
		statEnergy = "0";
	else if ( !creature || ( craftStats.Craft.empty() ) )
		statEnergy = toString( "%d", GetStatEnergy( level ) );
	else if ( variation < 2 )
		statEnergy = toString( "%d", GetStatEnergy( level + 1 ) );
		
	output += statEnergy;
	currentDocItem.push( DtAverageEnergy, statEnergy );

	output += "\"/>\n    </STRUCT>\n  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";

	output.writeToFile( toString( "%s%s\\%s", MP_DIRECTORY.c_str(), directory.c_str(), outputFileName.c_str() ) );

	// Génération des noms
	if ( !specialItem )
	{
		outputFileName = toString( "m%04d%s%c%c%02d", numMP, code.c_str(), eco, 'a' + level, variation );
		output = outputFileName;

		GenerateItemNames( nomMP, eco, level, ( craftStats.Craft.empty() ), creature, itemName );
		output += "\t" + itemName;
		itemNames.insert( output );
	}

	currentDocItem.push( DtEcosystem, ecoStr );
	currentDocItem.push( DtName, outputFileName );

	SortableData.updateItemAppend( currentDocItem, DtName );
	currentDocItem.reset( DtName );
	currentDocItem.reset( DtEcosystem );
	currentDocItem.reset( DtMaxLevel );
	currentDocItem.reset( DtAverageEnergy );
	currentDocItem.reset( DtTitle );
	currentDocItem.reset( DtLevelZone );
	currentDocItem.reset( DtStatQuality );
	currentDocItem.reset( DtCraftSlotName );
	currentDocItem.reset( DtCraftCivSpec );
	currentDocItem.reset( DtColor );
}


// Pour une MP se trouvant dans des déposits, génération de ses items
void GenerateDepositItems( int numMP, const CSString& nomMP, const MPCraftStats& craftStats, const CSString& loc )
{
	CSString code;

	if ( loc.left(1) == "I" )
		code = "ixx";
	else if ( loc.left(1) == "D" )
		code = "dxa";
	else
		code = "cxx";

	// pas de craft = items de mission
	if ( craftStats.Craft.empty() )
	{
		if ( loc != "G" )
			CreateSheet( numMP, nomMP, code, 'c', 0, craftStats );
		
		for ( int i=1; i<6; i++ )
		{
			CreateSheet( numMP, nomMP, code, 'c', i, craftStats );
		}
	}
	else
	{
		// 2 items dans common
		CreateSheet( numMP, nomMP, code, 'c', 1, craftStats );
		CreateSheet( numMP, nomMP, code, 'c', 2, craftStats );

		// 3 items par zone
		for ( int i=0; i<3; i++ )
		{
			CreateSheet( numMP, nomMP, code, 'd', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'f', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'j', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'l', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'p', 3+i, craftStats );
		}
	}
}


// Pour une MP se trouvant sur un créature, génération de ses items
void GenerateCreatureItems( int numMP, CSString& nomMP, const MPCraftStats& craftStats )
{
	map<CSString, ListeCreatureMP>::const_iterator itLCMP;
	int quality;
	static int statQuality[] = { 0, 1, 0, 1, 3, 6, 4, 2 };

	// On obtient la liste des niveau d'item à generer pour la créature
	itLCMP = itemsAGenerer.find( nomMP.firstWord() );

	if ( itLCMP != itemsAGenerer.end() )
	{
		ListeCreatureMP::const_iterator itMP = (*itLCMP).second.begin();

		// pour chaque niveau d'item à générer
		while ( itMP != (*itLCMP).second.end() )
		{
			// on enregistre ses caractéristiques
			char eco = (*itMP)->eco;
			int creatureLevel = (*itMP)->creatureLevel;
			int itemLevel = (*itMP)->itemLevel;
			CSString creatureFileName = "c";
			creatureFileName += (*itMP)->codeCreature.toLower();
			
			if ( !craftStats.Craft.empty() )
			{
				quality = statQuality[creatureLevel-1];
				if ( quality != 6 )
				{						
					if ( quality < 2 )
					{
						CreateSheet( numMP, nomMP, creatureFileName, 'c', quality, craftStats );

						AssignerMP( (*itMP)->creatureFileName, 
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), 'c', 'a' + quality ) );
					}
					else
					{
						CreateSheet( numMP, nomMP, creatureFileName, eco, quality, craftStats );

						AssignerMP( (*itMP)->creatureFileName, 
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), eco, 'a' + quality ) );
					}

					currentDocItem.push( DtCreature, (*itMP)->creatureFileName );
				}
			}
			else
			{
				// pas de MP de mission pour les boss
				if ( creatureLevel < 5 )
				{
					CreateSheet( numMP, nomMP, creatureFileName, eco, itemLevel, craftStats ); 

					AssignerMP( (*itMP)->creatureFileName, 
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), eco, 'a' + itemLevel ) );

					currentDocItem.push( DtCreature, (*itMP)->creatureFileName );
				}
			}

			itMP++;
		}
	}
}


// Génération d'un item spécial
void GenerateSpecialItem( int numMP, const CSString& nomMP, const MPCraftStats& craftStats, 
						  const CSString& loc,CSString& itemData, int variation )
{
	CSString info, code, name;
	info = itemData.splitTo( "/", true ).toLower();

	if ( loc.left(1) == "I" )
		code = "ixx";
	else if ( loc.left(1) == "D" )
		code = "dxa";
	else
		code = "cxx";

	CreateSheet( numMP, nomMP, code, info.c_str()[0], info.c_str()[1]-'a', craftStats, true, variation );

	name = toString( "m%04d%s%s%02d\t", numMP, code.c_str(), info.c_str(), variation ); 
	name += itemData.splitTo( "/", true ); // singulier
	name += "\t";
	name += itemData.splitTo( "/", true ); // article indéfini
	name += "\t";
	name += itemData.splitTo( "/", true ); // article défini
	name += "\t";
	name += itemData.splitTo( "/", true ); // pluriel
	name += "\t\t";
	name += itemData.splitTo( "/", true ); // article pluriel

	itemNames.insert( name );
}


// Special Attrib parsing. craftStats must be good for extraInfo init
void	parseSpecialAttributes(const CSString &specialAttributes, MPCraftStats &craftStats, CExtraInfo &extraInfo)
{
	// evaluate DropOrSell according to CraftStats: can DropOrSell if it is a MP Craft
	extraInfo.DropOrSell= craftStats.Craft != "";

	// parse attributes
	vector<string> strArray;
	splitString(specialAttributes, "-", strArray);
	for(uint i=0;i<strArray.size();i++)
	{
		if(nlstricmp(strArray[i], "R")==0)
			craftStats.UsedAsCraftRequirement= true;
		if(nlstricmp(strArray[i], "D0")==0)
			extraInfo.DropOrSell= false;
		if(nlstricmp(strArray[i], "D1")==0)
			extraInfo.DropOrSell= true;
	}
}


// Nouvelle MP à traiter
void NewMP( CSString& ligne )
{
	CSString nomMP, groupe, loc, icon, overlay, special, stat, specialAttributes;
	MPCraftStats craftStats;
	CExtraInfo	extraInfo;
	int numMP;
	bool specialOnly = false;
	CSortedStringSet specialNames;

	// nouveau nom de famille
	nomMP = ligne.splitTo( ";", true );
	if ( nomMP.empty() )
	{
		// cette ligne ne contient pas d'info
		return;
	}
	
	// récupération des infos
	groupe = ligne.splitTo( ";", true );
	craftStats.Craft = ligne.splitTo( ";", true );
	specialAttributes= ligne.splitTo( ";" , true );
	parseSpecialAttributes(specialAttributes, craftStats, extraInfo);
	ligne.splitTo( ";" , true );
	loc = ligne.splitTo( ";" , true );
	icon = ligne.splitTo( ";", true );
	ligne.splitTo( ";", true );
	ligne.splitTo( ";", true );
	ligne.splitTo( ";", true );

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.bestStatA = stat.atoi();
	else
		craftStats.bestStatA = -1;

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.worstStatA1 = stat.atoi();
	else
		craftStats.worstStatA1 = -1;

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.worstStatA2 = stat.atoi();
	else
		craftStats.worstStatA2 = -1;

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.bestStatB = stat.atoi();
	else
		craftStats.bestStatB = -1;

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.worstStatB1 = stat.atoi();
	else
		craftStats.worstStatB1 = -1;

	stat = ligne.splitTo( ";", true );
	if ( !stat.firstWord().empty() )
		craftStats.worstStatB2 = stat.atoi();
	else
		craftStats.worstStatB2 = -1;

	stat = ligne.splitTo( ";", true );
	craftStats.color = stat.firstWord().atoi();

	stat = ligne.splitTo( ";", true );
	specialOnly = stat.firstWord().contains( "x" );
	
	// cas particuliers
	while ( !ligne.empty() )
	{
		if ( !ligne.contains( ";" ) ) 
		{
			special = ligne;
			if ( !special.firstWord().empty() )
				specialNames.insert( special );
			ligne = "";
		}
		else
		{
			special = ligne.splitTo( ";", true );
			if ( !special.empty() )
				specialNames.insert( special );
		}
	}


	currentDocItem.push( DtRMFamily, nomMP );
	currentDocItem.push( DtGroup, groupe );
	
	// récupréation du numéro de MP
	numMP = GetNumeroMP( nomMP );
	printf( "    Processing Family %d : %s\n", numMP, nomMP.c_str() );

	GetNumeroGroupe( groupe );


	// Add the MPFamily into the list
	if(numMP>=(sint)MPFamilies.size())
		MPFamilies.resize(numMP+1);
	MPFamilies[numMP].Name= nomMP;
	MPFamilies[numMP].Icon= icon;

	
	// MP trouvées dans les déposits ou dans la goo
	if ( loc.left(1) != "C" )
	{
		if ( !specialOnly )
		{
			// Génération des items
			GenerateDepositItems( numMP, nomMP, craftStats, loc );
		}
		
		// on enregistre les items se trouvant dans les deposits
		if ( loc.left(1) == "D" )
		{
			CSString output;
			output.writeToFile( toString( "%s%s_%d.mp", DEPOSIT_MPS.c_str(), nomMP.toLower().replace( " ", "_" ).c_str(), numMP ) );
		}

		overlay = nomMP.firstWord().toUpper().left(6);
	}
	// MP trouvées sur les creature
	else
	{
		GenerateCreatureItems( numMP, nomMP, craftStats );
	}


	// items spéciaux
	CSString codeSpecial, nouveauCode;
	int variation = 1;
	CSortedStringSet::const_iterator it = specialNames.begin();

	while ( it != specialNames.end() )
	{
		CSString name = (*it);
		
		nouveauCode = name.left(2).toLower();

		if ( nouveauCode == codeSpecial )
			variation++;
		else
			variation = 1;

		GenerateSpecialItem( numMP, nomMP, craftStats, loc, name, variation );
		codeSpecial = nouveauCode;
		it++;
	}
		
		
	// Création de la fiche parente pour la MP
	CreateParentSItem( numMP, nomMP, groupe, extraInfo.DropOrSell, icon, overlay );

	currentDocItem.reset( DtRMFamily );
	currentDocItem.reset( DtGroup );
	currentDocItem.reset( DtProp );
	currentDocItem.reset( DtCreature );
}


// Génération du Primitive Necklace (objet particulier)
void CreatePrimitiveNecklace()
{
	CSString output;
	
	output = "<?xml version=\"1.0\"?>\n";
	output += "<FORM Version=\"0.0\" State=\"modified\">\n";
	output += "  <PARENT Filename=\"_mc.sitem\"/>\n";
	output += "  <PARENT Filename=\"_m0696.sitem\"/>\n";
	output += "  <STRUCT>\n    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"MpColor\" Value=\"Beige\"/>\n";
	output += "      <ATOM Name=\"MaxQuality\" Value=\"250\"/>\n";
	output += "      <ATOM Name=\"StatEnergy\" Value=\"0\"/>\n";
	output += "    </STRUCT>\n  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n";
	output += "  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";
	output.writeToFile( toString( "%scommon\\m0696ixxcc01.sitem", MP_DIRECTORY.c_str() ) );
	
	itemNames.insert( "m0696ixxcc01	Primitive Necklace	a	the	Primitive Necklaces		the" );

}


// Enregistre les différents noms
void ItemNamesSave()
{
	printf( "-- SAVING ITEM NAMES --\n");
	CSString data, output;

	FILE* file;
	file = fopen( ITEM_WORDS_WK.c_str(), "rb" );

	char c;
	fread( &c, 1, 1, file );
	while ( !feof( file ) )
	{
		data += toString( "%c", c );
		fread( &c, 1, 1, file );
	}

	fclose( file );

	data.splitTo( "i", true );
	output = "i";
	output += data.splitTo( "prospector", true );

	CSortedStringSet::const_iterator it = itemNames.begin();

	while ( it != itemNames.end() )
	{
		if ( !output.contains( (*it).left(5).c_str() ) )
		{
			output += (*it);
			output += "\r\n";
		}
		it++;
	}

	output += "p";

	output += data;

	output.writeToFile( ITEM_WORDS_WK.c_str() );
}


// Charge les différentes Customized Properties 
// définies dans raw_material_generation.cfg
void LoadCustomizedProperties()
{
	CSString data, name, prop, val;
	
	printf( "-- REGISTERING CUSTOMIZED PROPERTIES --\n" );
	
	data.readFromFile( "raw_material_generation.cfg" );
	data = data.splitFrom( "{\r\n\t" );
	CPath::addSearchPath( MP_DIRECTORY, true, false );

	while ( data.contains( "};" ) )
	{
		name = data.splitTo( ",", true ).replace( "\"", "" );
		prop = data.splitTo( ",", true ).replace( "\"", "" ).replace( " ", "" );

		val = data.splitTo( ",", true ).replace( "\"", "" ).replace( " ", "" );
		if ( val.contains( "\r\n" ) )
			val = val.splitTo( "\r\n", true );

		TRMItem item;
		item.push( DtName, name );
		item.push( DtCustomizedProperties, prop );
		SortableData.updateItemAppend( item, DtName );
		
		data.splitTo( "\t", true );

		CSString fileName, str, output;
		fileName = CPath::lookup( name, false, false, true );
		
		// on vérifie que le fichier concerné existe
		if ( !fileName.empty() )
		{
			CSString zone = prop.splitTo( ".", true );
			str.readFromFile( fileName );

			if ( !str.contains( zone.c_str() ) )
			{
				output = "<STRUCT>\n    <STRUCT Name=\"";
				output += toString( "%s\">\n      <ATOM Name=\"", zone.c_str() );
				output += toString( "%s\" Value=\"%s\"/>\n    </STRUCT>", prop.c_str(), val.c_str() );

				// on vérifie que la propriétés n'a pas déjà été insérée
				if ( !str.contains( output.c_str() ) )
				{
					str = str.replace( "<STRUCT>", output.c_str() );
					str.writeToFile( fileName );
				}
			}
			else
			{
				output = toString( "    <STRUCT Name=\"%s\">\n", zone.c_str() );
				output += toString( "      <ATOM Name=\"" );
				output += toString( "%s\" Value=\"%s\"/>\n", prop.c_str(), val.c_str() );

				// on vérifie que la propriétés n'a pas déjà été insérée
				if ( !str.contains( toString( "%s\" Value=\"%s\"/>\n", prop.c_str(), val.c_str() ).c_str() ) )
				{
					str = str.replace( toString( "    <STRUCT Name=\"%s\">\n", zone.c_str() ).c_str(), output.c_str() );
					str.writeToFile( fileName );
				}
			}
		}
	}
}

// Generate _ic_families.forage_source()
void	SaveFamiliesForageSource()
{
	CSString output;

	printf( "-- GROUP ICONS FOR _ic_groups.forage_source --\n");
	
	output = "<?xml version=\"1.0\"?>\n";
	output+= "<FORM Revision=\"$Revision: 1.9 $\" State=\"modified\">\n";
	output+= "    <STRUCT>\n";
	output+= "    <ARRAY Name=\"Icons\">\n";

	for ( uint i=0; i!=MPFamilies.size(); ++i )
	{
		output+= toString("      <ATOM Value=\"%s\"/>\n", MPFamilies[i].Icon.c_str());
	}

    output+= "    </ARRAY>\n";
	output+= "  </STRUCT>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <LOG></LOG>\n";
	output+= "</FORM>\n";

	output.writeToFile( IC_FAMILIES_FORAGE_SOURCE.c_str() );
}


// Génère la documentation
void GenerateDoc()
{
	CProducedDocHtml			MainDoc;
	CFile::createDirectory("doc");
	MainDoc.open( "doc\\rm.html", "Raw materials by generation order", true );
	MainDoc.write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
	MainDoc.write( "<tr>" );
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		MainDoc.write( "<td><b><a href=\"rm_" + string(DataColStr[c]) + ".html\">" + string(DataColStr[c]) + "</a></b></td>" );
	}
	MainDoc.write( "</tr>" );
	for ( CRMData::CItems::const_iterator isd=SortableData.items().begin(); isd!=SortableData.items().end(); ++isd )
	{
		MainDoc.write( (*isd).toHTMLRow() );
	}
	MainDoc.write( "</tbody><table>\n" );
	
	// Produce alt docs
	CProducedDocHtml			AltDocs[DtNbCols];
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		AltDocs[c].open( "doc\\rm_" + string(DataColStr[c]) + ".html", "Raw materials by " + string(DataColStr[c]), true );
		AltDocs[c].write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
		AltDocs[c].write( "<tr>" );
		for ( uint32 cc=0; cc!=DtNbCols; ++cc )
			if ( cc == c )
				AltDocs[c].write( "<td><b>" + string(DataColStr[cc]) + "</b></td>" );
			else
				AltDocs[c].write( "<td><b><a href=\"rm_" + string(DataColStr[cc]) + ".html\">" + string(DataColStr[cc]) + "</a></b></td>" );
		AltDocs[c].write( "</tr>" );
		string previousKey = "[NO PREVIOUS]"; // not a blank string, because it may be a valid value
		string previousName;
		for ( CRMData::CLookup::const_iterator isd=SortableData.lookup( c ).begin(); isd!=SortableData.lookup( c ).end(); ++isd )
		{
			const TRMItem& item = SortableData.getRow( (*isd).second );
			AltDocs[c].write( item.toHTMLRow( c, (*isd).first, previousKey, DtName, previousName ) );

			previousKey = (*isd).first;
			previousName = item.Fields[DtName][0];
		}
		AltDocs[c].write( "</tbody><table>\n" );
		AltDocs[c].save();
	}
}


// Initialise les repertoires à partir de raw_material_generation.cfg
void SetupDirectories()
{
	CSString data;
	
	if ( ! CFile::fileExists( "raw_material_generation.cfg" ) )
	{
		nlError( "raw_material_generation.cfg not found\n");
		exit( 1 );
	}
		
	data.readFromFile( "raw_material_generation.cfg" );

	LEVEL_DESIGN_PATH = data.splitFrom( "LevelDesignPath = \"").splitTo( "\"" );
	TRANSLATION_PATH = data.splitFrom( "TranslationPath = \"" ).splitTo( "\"" );

	printf( "Level Design Path : %s\nTranslation Path : %s\n\n", LEVEL_DESIGN_PATH.c_str(), TRANSLATION_PATH.c_str() );
	
	ITEM_MP_FAMILY_TYP = LEVEL_DESIGN_PATH + "leveldesign\\DFN\\game_elem\\_item\\item_mp_family.typ";
	ITEM_MP_GROUPE_TYP = LEVEL_DESIGN_PATH + "leveldesign\\DFN\\game_elem\\_item\\item_mp_group.typ";
	ITEM_MP_PARAM_DFN = LEVEL_DESIGN_PATH + "leveldesign\\DFN\\game_elem\\_item\\_item_mp_param.dfn";
	MP_DIRECTORY = LEVEL_DESIGN_PATH + "leveldesign\\game_element\\sitem\\raw_material\\";
	DEPOSIT_MPS = LEVEL_DESIGN_PATH + "leveldesign\\game_element\\deposit_system\\mps\\"; 

	RAW_MATERIAL_ASSIGN = LEVEL_DESIGN_PATH + "leveldesign\\Game_elem\\Creature\\raw_material_assignment\\";

	IC_FAMILIES_FORAGE_SOURCE = LEVEL_DESIGN_PATH + "leveldesign\\game_element\\forage_source\\_ic_families.forage_source";
	
	WK_UXT = TRANSLATION_PATH + "work\\wk.uxt";
	ITEM_WORDS_WK = TRANSLATION_PATH + "work\\item_words_wk.txt";
}


// Parcours les différentes familles de MP pour effectuer
// les traitements nécessaires
void LoadFamillesMP()
{
	printf( "-- LOADING RAW MATERIAL FAMILIES --\n" );

	// Preload wk.uxt, item_mp_family.typ, and item_mp_group.typ (avoid to reload them each time)
	FamilyTypContent.readFromFile( ITEM_MP_FAMILY_TYP );
	GroupTypContent.readFromFile( ITEM_MP_GROUPE_TYP );
	WKContent.readFromFile( WK_UXT );
	
	// avoid huge resize of vector<string>
	MPFamilies.reserve(1000);

	CSString fileData, ligne;

	if ( ! CFile::fileExists( "rm_fam_prop.csv" ) )
	{
		nlError( "rm_fam_prop.csv not found\n");
		exit( 1 );
	}


	fileData.readFromFile( "rm_fam_prop.csv" );

	ligne = fileData.splitTo( "\n", true );

	while ( !ligne.empty() )
	{
		NewMP( ligne );	
		ligne = fileData.splitTo( "\n", true );
	} 

	// le Primitive Necklace n'étant pas spécifié dans la liste des MP
	// on le rajoute à la main
	CreatePrimitiveNecklace();
}


// Programme principal
int main( int argc, char* argv[] )
{
	new CApplicationContext;
	SortableData.init( true );
	
	SetupDirectories();

	LoadCraftParts();

	LoadCreatureFiles();

	InitCreatureMP();

	LoadFamillesMP();
	
	ItemNamesSave();

	LoadCustomizedProperties();

	SaveFamiliesForageSource();
	
	printf( "-- GENERATING DOCUMENTATION --\n" );
	try
	{
		GenerateDoc();
	}
	catch(const Exception &e)
	{
		nlwarning(e.what());
		nlwarning("HTML Doc generation failed\n");
	}

	printf( "-- DONE --\n" );
	
	return 0;
}
