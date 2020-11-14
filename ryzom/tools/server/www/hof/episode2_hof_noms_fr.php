<?php

$txt_progress_batiment = 'Avancement du chantier : ';
$titre='Stèle de valeur - Episode II : Les demeures des dieux';

$position_name='VOTRE CLASSEMENT';
$position_guild_name='LE CLASSEMENT DE VOTRE GUILDE';
$rang_name='VOTRE RANG';
$points_name='VOS POINTS';
$rang_guild_name='LE RANG';
$points_guild_name='LES POINTS';

$txt_tab_chantier = 'CHANTIER';
$txt_tab_craft = 'ARTISANAT';
$txt_tab_harvest = 'RECOLTE';
$txt_tab_fight = 'COMBAT';

$classement_craft = 'CLASSEMENT ARTISANAT';
$classement_harvest = 'CLASSEMENT RECOLTE';
$classement_fight = 'CLASSEMENT COMBAT';

$txt_tab_total = 'TOTAL';
$txt_tab_faction = 'FACTION';
$txt_tab_chantier = 'CHANTIER';
$txt_tab_acte = 'ACTE';

$txt_tab_name_craft = 'ARTISAN';
$txt_tab_name_harvest = 'RECOLTEUR';
$txt_tab_name_fight = 'COMBATTANT';


$faction = &$_GET['faction'];
$race = &$_GET['race'];
$tableau = &$_GET['tableau'];
$type=&$_GET['type'];

if($type=='craft')
$classement_player='CLASSEMENT DES ARTISANTS';
if($type=='harvest')
$classement_player='CLASSEMENT DES RECOLTEURS';
if($type=='fight')
$classement_player='CLASSEMENT DES COMBATTANTS';
$classement_guild='CLASSEMENT DES GUILDES';


if ($faction == 'karavan')
{
	
	$txt_progress_batiment = 'Construction du temple : ';
	$txt_piece1 = 'Socle :';
	$txt_piece2 = 'Colonne :';
	$txt_piece3 = 'Combles :';
	$txt_piece4 = 'Murailles :';
	$txt_piece5 = 'Revêtement :';
	$txt_piece6 = 'Ornement :';
	$txt_piece7 = 'Statue :';
	$txt_piece8 = 'Colonne de Justice :';
	switch ($race){
		case "fyros" : 
			$place = 'Temple Karavanier de Pyr';
		break;
		case "matis" : 
			$place = 'Temple Karavanier d\'Yrkanis';
		break;
		case "zorai" : 
			$place = 'Temple Karavanier de Zora';
		break;
		case "tryker" : 
			$place = 'Temple Karavanier de Fairhaven';
		break;
	}
}
else if ($faction == 'kami')
{
	
	$txt_progress_batiment = 'Construction du Sanctuaire : ';
	$txt_piece1 = 'Racine :';
	$txt_piece2 = 'Tronc :';
	$txt_piece3 = 'Fibres :';
	$txt_piece4 = 'Ecorce :';
	$txt_piece5 = 'Feuille :';
	$txt_piece6 = 'Fleur :';
	$txt_piece7 = 'Symbole :';
	$txt_piece8 = 'Noyau :';
	switch ($race){
		case "fyros" : 
			$place = 'Sanctuaire Kamiste de Pyr';
		break;
		case "matis" : 
			$place = 'Sanctuaire Kamiste d\'Yrkanis';
		break;
		case "zorai" : 
			$place = 'Sanctuaire Kamiste de Zora';
		break;
		case "tryker" : 
			$place = 'Sanctuaire Kamiste de Fairhaven';
		break;
	}
}


//Creation des titres

switch ($race){
	case "fyros" : 
		$city_title='DE PYR';
		$city_de='de Pyr';
		$city_a='à Pyr';
	break;
	case "matis" : 
		$city_title='D\'YRKANIS';
		$city_de='d\'Yrkanis';
		$city_a='à Yrkanis.';
	break;
	case "zorai" : 
		$city_title='DE ZORA';
		$city_de='de Zora';
		$city_a='à Zora.';
	break;
	case "tryker" : 
		$city_title='DE FAIRHAVEN';
		$city_de='de Fairhaven';
		$city_a='à Fairhaven.';
	break;
}

switch ($type){
	case "craft" : 
		$type_de='d\'artisans';
		$type_norm='artisans';
	break;
	case "harvest" : 
		$type_de='de recolteurs';
		$type_norm='recolteurs';
	break;
	case "fight" : 
		$type_de='de combattants';
		$type_norm='combattants';
	break;
}


if ($tableau=='total')
{
	$main_title='LES DEMEURES DES DIEUX';
	$titre_tableau_player='Les grands '.$type_norm;
	$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' d\'Atys ayant participé à la construction des monuments des dieux.';
	$titre_tableau_guild='Les grandes guilde '.$type_de;
	$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' d\'Atys ayant participé à la construction des monuments des dieux.';
}

if ($tableau=='faction')
{
	if($faction=='kami')
	{
		$main_title='LES DEMEURES DES DIEUX';
		$titre_tableau_player='Les grands '.$type_norm.' de Ma-Duk';
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' kamistes ayant participé à la construction des sanctuaires de Ma-Duk.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' de Ma-Duk';
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' kamistes ayant participé à la construction des sanctuaires de Ma-Duk.';
	}
	if($faction=='karavan')
	{
		$main_title='LES DEMEURES DES DIEUX';
		$titre_tableau_player='Les grands '.$type_norm.' de Jena';
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' karavaniers ayant participé à la construction des temples de Jena.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' de Jena';
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' karavaniers ayant participé à la construction des temples de Jena.';
	}
}


if($tableau=='chantier')
{
	
	if($faction=='kami')
	{
		$main_title='LE SANCTUAIRE KAMISTE '.$city_title;
		$titre_tableau_player='Les grands '.$type_norm.' kamistes '.$city_de;
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' kamistes ayant participé à la construction du sanctuaire de Ma-Duk '.$city_a.'.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' kamistes '.$city_de;
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' kamistes ayant participé à la construction du sanctuaire de Ma-Duk '.$city_a.'.';
	}
	if($faction=='karavan')
	{
		$main_title='LE TEMPLE KARAVANIER '.$city_title;
		$titre_tableau_player='Les grands '.$type_norm.' karavaniers '.$city_de;
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' karavaniers ayant participé à la construction du temple de Jena '.$city_a.'.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' karavaniers '.$city_de;
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' karavaniers ayant participé à la construction du temple de Jena '.$city_a.'.';
	
	}
	
}
if($tableau=='acte')
{
	if($noActe==1)
	{
	
		$ilot_title='LES DUNES D\'AELIUS';
		$ilot='les dunes d\'Aelius';
		$acte='I';
	}	
	if($noActe==2)
	{
		$ilot_title='LE LAC D\'OLKERN';
		$ilot='le lac d\'Olkern';
		$acte='II';
	}
	if($noActe==3)
	{
		$ilot_title='LE BOIS D\'ALMATI';
		$ilot='le bois d\'Almati';
		$acte='III';
	}
	if($faction=='kami')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='Les grands '.$type_norm.' kamistes '.$city_de.' ayant travaillé dans '.$ilot;
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' kamistes ayant participé à la construction du sanctuaire de Ma-Duk '.$city_a.' pendant l\'Acte '.$acte.'.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' kamistes '.$city_a.' ayant travaillé dans '.$ilot;
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' kamistes ayant participé à la construction du sanctuaire de Ma-Duk '.$city_a.' pendant l\'Acte '.$acte.'.';
	}
	if($faction=='karavan')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='Les grands '.$type_norm.' karavaniers '.$city_de.' ayant travaillé dans '.$ilot;
		$comment_tableau_player='Classement des dix meilleurs '.$type_norm.' karavaniers ayant participé à la construction du sanctuaire de Ma-Duk '.$city_a.' pendant l\'Acte '.$acte.'.';
		$titre_tableau_guild='Les grandes guildes '.$type_de.' karavaniers '.$city_a.' ayant travaillé dans '.$ilot;
		$comment_tableau_guild='Classement des dix meilleures guildes '.$type_de.' karavaniers ayant participé à la construction du temple de Jena '.$city_a.' pendant l\'Acte '.$acte.'.';
	}
}

?>