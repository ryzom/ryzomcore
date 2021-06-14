<?php

$race = &$_GET['race'];
$faction = &$_GET['faction'];
$shard = &$_GET['shard'];
$lang = &$_GET['lang'];
$user_login = &$_GET['user_login'];
$guild_name=&$_GET['guild_name'];
$guild_name=str_replace('_',' ', $guild_name);


include 'episode2_hof_outils.php';

$include_files = hof_init(array('episode2_valeurs_craft.hdt','episode2_valeurs_seuils.hdt'), 'episode2_hof_noms_%lang%.php');
foreach ($include_files as $include_file)
{
	if (!@include $include_file)
	{
		//die('include file '.$include_file.' not found');
		exit();
	}
}
include 'episode2_hof_titles.php';

echo '<html>';
echo '<head><title>'.$titre.'</title></head>';
echo '<body bgcolor="#30353A">';

echo hof_tab_hgroup($tabs, 'chantier');
echo '<br>';
if ($faction=='karavan')
{
	$tmp_item1_max = ${"maxSocle".$race};
	$tmp_item1_cur = ${"Socle".$race};
	$item1_percent = min($tmp_item1_cur, $tmp_item1_max) * 100 / $tmp_item1_max;
	$tmp_item2_max = ${"maxColonne".$race};
	$tmp_item2_cur = ${"Colonne".$race};
	$item2_percent = min($tmp_item2_cur, $tmp_item2_max) * 100 / $tmp_item2_max;
	$tmp_item3_max = ${"maxComble".$race};
	$tmp_item3_cur = ${"Comble".$race};
	$item3_percent = min($tmp_item3_cur, $tmp_item3_max) * 100 / $tmp_item3_max;
	$tmp_item4_max = ${"maxMuraille".$race};
	$tmp_item4_cur = ${"Muraille".$race};
	$item4_percent = min($tmp_item4_cur, $tmp_item4_max) * 100 / $tmp_item4_max;
	$tmp_item5_max = ${"maxRevetement".$race};
	$tmp_item5_cur = ${"Revetement".$race};
	$item5_percent = min($tmp_item5_cur, $tmp_item5_max) * 100 / $tmp_item5_max;
	$tmp_item6_max = ${"maxOrnement".$race};
	$tmp_item6_cur = ${"Ornement".$race};
	$item6_percent = min($tmp_item6_cur, $tmp_item6_max) * 100 / $tmp_item6_max;
	$tmp_item7_max = ${"maxStatue".$race};
	$tmp_item7_cur = ${"Statue".$race};
	$item7_percent = min($tmp_item7_cur, $tmp_item7_max) * 100 / $tmp_item7_max;
	$tmp_main_max = ${"maxJustice".$race};
	$tmp_main_cur = ${"Justice".$race};
	$main_percent = min($tmp_main_cur, $tmp_main_max) * 100 / $tmp_main_max;
}
else
{
	$tmp_item1_max = ${"maxRacine".$race};
	$tmp_item1_cur = ${"Racine".$race};
	$item1_percent = min($tmp_item1_cur, $tmp_item1_max) * 100 / $tmp_item1_max;
	$tmp_item2_max = ${"maxTronc".$race};
	$tmp_item2_cur = ${"Tronc".$race};
	$item2_percent = min($tmp_item2_cur, $tmp_item2_max) * 100 / $tmp_item2_max;
	$tmp_item3_max = ${"maxFibre".$race};
	$tmp_item3_cur = ${"Fibre".$race};
	$item3_percent = min($tmp_item3_cur, $tmp_item3_max) * 100 / $tmp_item3_max;
	$tmp_item4_max = ${"maxEcorce".$race};
	$tmp_item4_cur = ${"Ecorce".$race};
	$item4_percent = min($tmp_item4_cur, $tmp_item4_max) * 100 / $tmp_item4_max;
	$tmp_item5_max = ${"maxFeuille".$race};
	$tmp_item5_cur = ${"Feuille".$race};
	$item5_percent = min($tmp_item5_cur, $tmp_item5_max) * 100 / $tmp_item5_max;
	$tmp_item6_max = ${"maxFleur".$race};
	$tmp_item6_cur = ${"Fleur".$race};
	$item6_percent = min($tmp_item6_cur, $tmp_item6_max) * 100 / $tmp_item6_max;
	$tmp_item7_max = ${"maxSymbole".$race};
	$tmp_item7_cur = ${"Symbole".$race};
	$item7_percent = min($tmp_item7_cur, $tmp_item7_max) * 100 / $tmp_item7_max;
	$tmp_main_max = ${"maxNoyau".$race};
	$tmp_main_cur = ${"Noyau".$race};
	$main_percent = min($tmp_main_cur, $tmp_main_max) * 100 / $tmp_main_max;
}
$total_percent= ($item1_percent+$item2_percent+$item3_percent+$item4_percent+$item5_percent+$item6_percent+$item7_percent+$main_percent)/8;

echo '<table nowrap cellspacing=10 width=100% align="center"><tr><td height=5 bgcolor="#101112"></td></tr><tr><td><table nowrap cellspacing=10 width=100% align="center">';
echo '<tr><td width=25></td><td width=300 valign="middle" align="right"><H2>'.$txt_progress_batiment.'</H2></td><td width=300 align="left">'.hof_progress_bar_huge($total_percent).'</td><td width=25></td></tr></table><table nowrap cellspacing=10 width=100% align="center">';
if ($noActe == 1)
{
	echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece1.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item1_percent,1).'</td><td width=25></td></tr>';
	echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece2.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item2_percent,1).'</td><td width=25></td></tr>';
}
else
{
	echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece1.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item1_percent,0).'</td><td width=25></td></tr>';
	echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece2.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item2_percent,0).'</td><td width=25></td></tr>';
	if($noActe == 2)
	{
		echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece3.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item3_percent,1).'</td><td width=25></td></tr>';
		echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece4.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item4_percent,1).'</td><td width=25></td></tr>';
	}
	else
	{
		echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece3.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item3_percent,0).'</td><td width=25></td></tr>';
		echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece4.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item4_percent,0).'</td><td width=25></td></tr>';
		if($noActe == 3)
		{
			echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece5.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item5_percent,1).'</td><td width=25></td></tr>';
			echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece6.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item6_percent,1).'</td><td width=25></td></tr>';
			echo '<tr><td width=25></td><td width=300 valign="middle" align="right">'.$txt_piece7.'&nbsp;</td><td width=300 align="left">'.hof_progress_bar($item7_percent,1).'</td><td width=25></td></tr>';
		}
	}
}
echo '<tr><td width=25></td><td width=300 valign="middle" align="right"><b>'.$txt_piece8.'</b>&nbsp;</td><td width=300 align="left">'.hof_progress_bar($main_percent,1).'</td><td width=25></td></tr></table>';
echo '<tr><td height=5 bgcolor="#101112"></td></tr></table>';



echo '</body>';
echo '</html>';

?>
