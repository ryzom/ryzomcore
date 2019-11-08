<?php

$txt_progress_batiment = 'Fortgang der Baustelle : ';
$titre='Ruhmessäule - Episode II : Der Wohnsitz der Götter';

$position_name='IHR EINORDNEN';
$position_guild_name='IHR GUILDSEINORDNEN';
$rang_name='IHRE STELLE';
$points_name='IHRE PUNKTE';
$rang_guild_name='DIE STELLE';
$points_guild_name='DIE PUNKTE';

$txt_tab_chantier = 'BAUSTELLE';
$txt_tab_craft = 'HANDWERK';
$txt_tab_harvest = 'ROHSTOFFABBAU';
$txt_tab_fight = 'KAMPF';

$classement_craft = 'RANGLISTE HANDWERK';
$classement_harvest = 'RANGLISTE ROHSTOFFABBAU';
$classement_fight = 'RANGLISTE KAMPF';

$txt_tab_total = 'GESAMT';
$txt_tab_faction = 'FRAKTION';
$txt_tab_chantier = 'BAUSTELLE';
$txt_tab_acte = 'ETAPPE';

$txt_tab_name_craft = 'HANDWERKER';
$txt_tab_name_harvest = 'ERNTER';
$txt_tab_name_fight = 'KAMPFER';


$faction = &$_GET['faction'];
$race = &$_GET['race'];
$tableau = &$_GET['tableau'];
$type=&$_GET['type'];

if($type=='craft')
$classement_player='RANGLISTE DER HANDWERKER';
if($type=='harvest')
$classement_player='RANGLISTE ROHSTOFFABBAU';
if($type=='fight')
$classement_player='RANGLISTE DER KÄMPFER';
$classement_guild='RANGLISTE DER GILDEN';


if ($faction == 'karavan')
{
	
	$txt_progress_batiment = 'Bau des Tempels : ';
	$txt_piece1 = 'Sockel :';
	$txt_piece2 = 'Säule :';
	$txt_piece3 = 'Gipfel :';
	$txt_piece4 = 'Mauern :';
	$txt_piece5 = 'Deckblatt :';
	$txt_piece6 = 'Ornament :';
	$txt_piece7 = 'Statue :';
	$txt_piece8 = 'Rechtssäule :';
	switch ($race){
		case "fyros" : 
			$place = 'Karavaneer Temples von Pyr';
		break;
		case "matis" : 
			$place = 'Karavaneer Temples von Yrkanis';
		break;
		case "zorai" : 
			$place = 'Karavaneer Temples von Zora';
		break;
		case "tryker" : 
			$place = 'Karavaneer Temples von Fairhaven';
		break;
	}
}
else if ($faction == 'kami')
{
	
	$txt_progress_batiment = 'Bau des Heiligtumes : ';
	$txt_piece1 = 'Wurzel :';
	$txt_piece2 = 'Rumpf :';
	$txt_piece3 = 'Fasern :';
	$txt_piece4 = 'Rinde :';
	$txt_piece5 = 'Blatt :';
	$txt_piece6 = 'Blume :';
	$txt_piece7 = 'Symbol :';
	$txt_piece8 = 'Herz :';
	switch ($race){
		case "fyros" : 
			$place = 'Kamist Heiligtum von Pyr';
		break;
		case "matis" : 
			$place = 'Kamist Heiligtum von Yrkanis';
		break;
		case "zorai" : 
			$place = 'Kamist Heiligtum von Zora';
		break;
		case "tryker" : 
			$place = 'Kamist Heiligtum von Fairhaven';
		break;
	}
}


//Creation des titres

switch ($race){
	case "fyros" : 
		$city_title='VON PYR';
		$city_de='von Pyr';
		$city_a='in Pyr';
	break;
	case "matis" : 
		$city_title='VON YRKANIS';
		$city_de='von Yrkanis';
		$city_a='in Yrkanis';
	break;
	case "zorai" : 
		$city_title='VON ZORA';
		$city_de='von Zora';
		$city_a='in Zora';
	break;
	case "tryker" : 
		$city_title='VON FAIRHAVEN';
		$city_de='von Fairhaven';
		$city_a='in Fairhaven';
	break;
}

switch ($type){
	case "craft" : 
		$type_de='Handwerks';
		$type_de_low='handwerks';
		$type_norm='Handwerker';
		$type_norm_low='handwerker';
	break;
	case "harvest" : 
		$type_de='Rohstoffabbau';
		$type_de_low='rohstoffabbau';
		$type_norm='Rohstoffabbauer';
		$type_norm_low='rohstoffabbauer';
	break;
	case "fight" : 
		$type_de='Kämpfer';
		$type_de_low='kämpfer';
		$type_norm='Kämpfer';
		$type_norm_low='kämpfer';
	break;
}


if ($tableau=='total')
{
	$main_title='DER WOHNSITZ DER GÖTTER';
	$titre_tableau_player='Die großen '.$type_norm;
	$comment_tableau_player='Rangliste der zehn besten '.$type_norm.' von Atys, welche am Bau der Momumente der Götter beteiligt waren.';
	$titre_tableau_guild='Die großen '.$type_de.'gilden';
	$comment_tableau_guild='Rangliste der zehn besten '.$type_de.'gilden von Atys, welche am Bau der Monumente der Götter beteiligt waren.';
}

if ($tableau=='faction')
{
	if($faction=='kami')
	{
		$main_title='DER WOHNSITZ DER GÖTTER';
		$titre_tableau_player='Die großen '.$type_norm.' von Ma-Duk';
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Kami, welche am Bau des Heiligtumes von Ma-Duk beteiligt waren.';
		$titre_tableau_guild='Die großen '.$type_de.'gilden von Ma-Duk';
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Kami, welche am Bau des Heiligtumes von Ma-Duk beteiligt waren.';
	}
	if($faction=='karavan')
	{
		$main_title='DER WOHNSITZ DER GÖTTER';
		$titre_tableau_player='Die großen '.$type_norm.' von Jena';
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Karavan, welche am Bau des Tempels von Jena beteiligt waren.';
		$titre_tableau_guild='Die großen '.$type_de.'gilden von Jena';
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Karavan, welche am Bau des Tempels von Jena beteiligt waren.';
	}
}

if($tableau=='chantier')
{
	if($faction=='kami')
	{
		$main_title='DAS KAMIST HEILIGTUM '.$city_title;
		$titre_tableau_player='Die großen '.$type_norm_low.' der Kami '.$city_de;
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Kami, welche am Bau des Heiligtumes von Ma-Duk '.$city_a.' beteiligt waren.';
		$titre_tableau_guild='Die großen '.$type_de_low.'gilden der Kami '.$city_de;
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Kami, welche am Bau des Heiligtumes von Ma-Duk '.$city_a.' beteiligt waren.';
	}
	if($faction=='karavan')
	{
		$main_title='DER KARAVANEER TEMPEL '.$city_title;
		$titre_tableau_player='Die großen '.$type_norm_low.' der Karavan '.$city_de;
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Karavan, welche am Bau des Tempels von Jena '.$city_a.' beteiligt waren.';
		$titre_tableau_guild='Die großen '.$type_de_low.'gilden der Karavan '.$city_de;
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Karavan, welche am Bau des Tempels von Jena '.$city_a.' beteiligt waren.';
	}
}

if($tableau=='acte')
{
	if($noActe==1)
	{
	
		$ilot_title='DIE DÜNEN VON ELIUS';
		$ilot='Die Dünen von Aelius';
		$acte='I';
	}	
	if($noActe==2)
	{
		$ilot_title='DER SEE VON OLKERN';
		$ilot='der See von Olkern';
		$acte='II';
	}
	if($noActe==3)
	{
		$ilot_title='DAS WALD VON ALMATI';
		$ilot='das Wald von Almati';
		$acte='III';
	}
	if($faction=='kami')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='Die großen '.$type_norm_low.' der Kami '.$city_de.', die in '.$ilot.' gearbeitet haben';
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Kami,  welche bei Etappe '.$acte." für den Bau von Ma-Duk's Heiligtum ".$city_a.' gearbeitet haben.';
		$titre_tableau_guild='Die großen '.$type_de_low.'gilden der Kami '.$city_a.', die in '.$ilot.' gearbeitet haben';
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Kami,  welche bei Etappe '.$acte." für den Bau von Ma-Duk's Heiligtum ".$city_a.' gearbeitet haben.';
	}
	if($faction=='karavan')
	{
		$main_title=$ilot_title;
		$titre_tableau_player='Die großen '.$type_norm_low.' der Karavan '.$city_de.', die in '.$ilot.' gearbeitet haben';
		$comment_tableau_player='Rangliste der zehn besten '.$type_norm_low.' der Karavan,  welche bei Etappe '.$acte.' für den Bau des Tempels von Jena '.$city_a.' gearbeitet haben.';
		$titre_tableau_guild='Die großen '.$type_de_low.'gilden der Karavan '.$city_a.', die in '.$ilot.' gearbeitet haben';
		$comment_tableau_guild='Rangliste der zehn besten '.$type_de_low.'gilden der Karavan,  welche bei Etappe '.$acte.' für den Bau des Tempels von Jena '.$city_a.' gearbeitet haben.';
	}
}

?>
