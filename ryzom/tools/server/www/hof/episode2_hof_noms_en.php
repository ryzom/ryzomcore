<?php

$txt_progress_batiment = 'Progress of the construction site : ';
$titre='Stele of Valor - Episode II : The Houses of the Gods';

$position_name='YOUR POSITION';
$position_guild_name='THE POSITION OF YOUR GUILD';
$rang_name='YOUR RANK';
$points_name='YOUR POINTS';
$rang_guild_name='THE RANK';
$points_guild_name='THE POINTS';

$txt_tab_chantier = 'SITE';
$txt_tab_craft = 'CRAFT';
$txt_tab_harvest = 'HARVEST';
$txt_tab_fight = 'FIGHT';

$classement_craft = 'CRAFT RANKING';
$classement_harvest = 'HARVEST RANKING';
$classement_fight = 'FIGHT RANKING';

$txt_tab_total = 'TOTAL';
$txt_tab_faction = 'FACTION';
$txt_tab_chantier = 'SITE';
$txt_tab_acte = 'ACTE';

$txt_tab_name_craft = 'CRAFTER';
$txt_tab_name_harvest = 'HARVESTER';
$txt_tab_name_fight = 'FIGHTER';


$faction = &$_GET['faction'];
$race = &$_GET['race'];
$tableau = &$_GET['tableau'];
$type=&$_GET['type'];

if($type=='craft')
$classement_player='RANKING OF THE CRAFTERS';
if($type=='harvest')
$classement_player='RANKING OF THE HARVESTERS';
if($type=='fight')
$classement_player='RANKING OF THE FIGHTERS';
$classement_guild='RANKING OF THE GUILDS';


if ($faction == 'karavan')
{
	
	$txt_progress_batiment = 'Construction of the temple : ';
	$txt_piece1 = 'Pedastal :';
	$txt_piece2 = 'Column :';
	$txt_piece3 = 'Attic :';
	$txt_piece4 = 'Wall :';
	$txt_piece5 = 'Cover :';
	$txt_piece6 = 'Ornament :';
	$txt_piece7 = 'Statue :';
	$txt_piece8 = 'Column of Justice :';
	switch ($race){
		case "fyros" : 
			$place = 'Karavaneer Temple of Pyr';
		break;
		case "matis" : 
			$place = 'Karavaneer Temple of Yrkanis';
		break;
		case "zorai" : 
			$place = 'Karavaneer Temple of Zora';
		break;
		case "tryker" : 
			$place = 'Karavaneer Temple of Fairhaven';
		break;
	}
}
else if ($faction == 'kami')
{
	
	$txt_progress_batiment = 'Construction of the Sanctuary : ';
	$txt_piece1 = 'Root :';
	$txt_piece2 = 'Trunk :';
	$txt_piece3 = 'Fiber :';
	$txt_piece4 = 'Bark :';
	$txt_piece5 = 'Leaf :';
	$txt_piece6 = 'Flower :';
	$txt_piece7 = 'Symbol :';
	$txt_piece8 = 'Core :';
	switch ($race){
		case "fyros" : 
			$place = 'Kamist Sanctuary of Pyr';
		break;
		case "matis" : 
			$place = 'Kamist Sanctuary of Yrkanis';
		break;
		case "zorai" : 
			$place = 'Kamist Sanctuary of Zora';
		break;
		case "tryker" : 
			$place = 'Kamist Sanctuary of Fairhaven';
		break;
	}
}


//Creation des titres

switch ($race){
	case "fyros" : 
		$city_title='OF PYR';
		$city_de='of Pyr';
		$city_a='in Pyr';
	break;
	case "matis" : 
		$city_title='OF YRKANIS';
		$city_de='of Yrkanis';
		$city_a='in Yrkanis.';
	break;
	case "zorai" : 
		$city_title='OF ZORA';
		$city_de='of Zora';
		$city_a='in Zora.';
	break;
	case "tryker" : 
		$city_title='OF FAIRHAVEN';
		$city_de='of Fairhaven';
		$city_a='in Fairhaven.';
	break;
}

switch ($type){
	case "craft" : 
		$type_de='crafter';
		$type_norm='crafters';
	break;
	case "harvest" : 
		$type_de='harvester';
		$type_norm='harvesters';
	break;
	case "fight" : 
		$type_de='fighter';
		$type_norm='fighters';
	break;
}


if ($tableau=='total')
{
	$main_title='THE HOUSES OF THE GODS';
	$titre_tableau_player='The great '.$type_norm;
	$comment_tableau_player='Ranking of the ten best '.$type_norm.' of Atys who participated in the construction of the gods\' monuments.';
	$titre_tableau_guild='The great guilds of '.$type_norm;
	$comment_tableau_guild='Ranking of the ten best '.$type_norm.' guilds of Atys who participated in the construction of the gods\' monuments.';
}

if ($tableau=='faction')
{
	if($faction=='kami')
	{
		$main_title='THE HOUSES OF THE GODS';
		$titre_tableau_player='The great '.$type_norm.' of Ma-Duk';
		$comment_tableau_player='Ranking of the ten best kamist '.$type_norm.' who participated in the construction of the sanctuaries of Ma-Duk.';
		$titre_tableau_guild='The great '.$type_norm.' guilds of Ma-Duk';
		$comment_tableau_guild='Ranking of the ten best kamist  '.$type_norm.' guilds who participated in the construction of the sanctuaries of Ma-Duk.';
	}
	if($faction=='karavan')
	{
		$main_title='THE HOUSES OF THE GODS';
		$titre_tableau_player='The great '.$type_norm.' of Jena';
		$comment_tableau_player='Ranking of the ten best karavaneer '.$type_norm.' who participated in the construction of the temples of Jena.';
		$titre_tableau_guild='The great '.$type_norm.' guilds of Jena';
		$comment_tableau_guild='Ranking of the ten best karavaneer '.$type_norm.' guilds who participated in the construction of the temples of Jena.';
	}
}


if($tableau=='chantier')
{
	
	if($faction=='kami')
	{
		$main_title='THE KAMIST SANCTUARY '.$city_title;
		$titre_tableau_player='The great kamist '.$type_norm.' '.$city_de;
		$comment_tableau_player='Ranking of the ten best kamist '.$type_norm.' who participated in the construction of the sanctuary of Ma-Duk '.$city_a.'.';
		$titre_tableau_guild='The great kamist '.$type_norm.' guilds  '.$city_de;
		$comment_tableau_guild='Ranking of the ten best kamist '.$type_de.' guilds who participated in the construction of the sanctuary of Ma-Duk '.$city_a.'.';
	}
	if($faction=='karavan')
	{
		$main_title='THE KARAVANEER TEMPLE '.$city_title;
		$titre_tableau_player='The great karavaneer '.$type_norm.' '.$city_de;
		$comment_tableau_player='Ranking of the ten best karavaneer '.$type_norm.' who participated in the construction of the temple of Jena '.$city_a.'.';
		$titre_tableau_guild='The great  karavaneer '.$type_norm.' guilds  '.$city_de;
		$comment_tableau_guild='Ranking of the ten best karavanner '.$type_de.' guilds who participated in the construction of the temple of Jena '.$city_a.'.';
	
	}
	
}
if($tableau=='acte')
{
	if($noActe==1)
	{
	
		$ilot_title='THE DUNES OF AELIUS';
		$ilot='the dunes of Aelius';
		$acte='Ist';
	}	
	if($noActe==2)
	{
		$ilot_title='THE LAKE OF OLKERN';
		$ilot='the lake of Olkern';
		$acte='IInd';
	}
	if($noActe==3)
	{
		$ilot_title='THE WOODS OF ALMATI';
		$ilot='the woods of Almati';
		$acte='IIIrd';
	}
	if($faction=='kami')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='The great Kamist '.$type_norm.' '.$city_de.' who worked in '.$ilot;
		$comment_tableau_player='Ranking of the ten best kamist '.$type_norm.' who participated in the construction of the sanctuary of Ma-Duk '.$city_a.' during the '.$acte.' act.';
		$titre_tableau_guild='The great kamist '.$type_de.' guilds '.$city_a.' who worked in '.$ilot;
		$comment_tableau_guild='Ranking of the ten best kamist '.$type_de.' guilds who participated in the construction of the sanctuary of Ma-Duk '.$city_a.' during the '.$acte.' act.';
	}
	if($faction=='karavan')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='The great karavanner '.$type_norm.' '.$city_de.' who worked in '.$ilot;
		$comment_tableau_player='Ranking of the ten best karavaneer '.$type_norm.' who participated in the construction of the temple of Jena '.$city_a.' during the '.$acte.' act.';
		$titre_tableau_guild='The great karavaneer '.$type_de.' guilds '.$city_a.' who worked in '.$ilot;
		$comment_tableau_guild='Ranking of the ten best karavaneer '.$type_de.' guilds who participated in the construction of the temple of Jena '.$city_a.' during the '.$acte.' act.';
	}
}

?>