<?php

$race = &$_GET['race'];
$faction = &$_GET['faction'];
$shard = &$_GET['shard'];
$lang = &$_GET['lang'];
$user_login = &$_GET['user_login'];
$guild_name = $_GET['guild_name'];
$guild_name = str_replace('_', ' ', $guild_name);
$type = &$_GET['type'];
$tableau = &$_GET['tableau'];

// Number of entries in the top
$top_len = 10;

// Profile the include time
$WANT_PROFILE = 0;
function microtime_float()
{
	list($usec, $sec) = explode(" ", microtime());
	return ((float)$usec + (float)$sec);
}
if ($WANT_PROFILE)
{
	$time_start = microtime_float();
}

// Includes
include 'episode2_hof_outils.php';

$hdt_files = array(
	'episode2_top10_faction_'.$type.'.hdt',
	'episode2_top10_faction_guild_'.$type.'.hdt',
	'episode2_top10_guild_'.$type.'.hdt',
	'episode2_top10_faction_guild_'.$type.'_'.$race.'.hdt',
	'episode2_top10_faction_acte_guild_'.$type.'_'.$race.'.hdt',
	'episode2_top10_'.$type.'.hdt',
	'episode2_top10_faction_'.$type.'_'.$race.'.hdt',
	'episode2_top10_faction_acte_'.$type.'_'.$race.'.hdt',
	'episode2_valeurs_seuils.hdt'
	);

$include_files = hof_init($hdt_files, 'episode2_hof_noms_%lang%.php');

foreach ($include_files as $include_file)
{
	if (!@include $include_file)
	{
		die('include file '.$include_file.' not found');
	}
}

include 'episode2_hof_titles.php';
include 'episode2_hof_tableaux.php';

if ($WANT_PROFILE)
{
	$time = microtime_float()- $time_start;
	echo '<br>PROFILE: include time = '.$time.'<br>';
}

if ($type == 'fight')
	$type_uppercase = 'Kill';
else
	$type_uppercase = ucfirst($type);

echo '<html>';
echo '<head><title>'.$titre.'</title></head>';
echo '<body bgcolor="#30353A">';

echo '<table width=825><tr><td>';
echo hof_tab_hgroup($tabs,$type);
echo '</td></tr><tr><td height=5 bgcolor="#101112"></td></tr><tr><td height=20></td></tr><tr><td align="center"><H1>'.${'classement_'.$type}.'</H1></td></tr><tr><td>';
echo hof_tab_hgroup_tableaux($tabs_tableaux,$tableau);
echo '</td></tr><tr><td height=5 bgcolor="#101112"></td></tr><tr><td height=20></td></tr><tr><td valign="middle"  align="center"><table border=0  align="center"><tr><td align=center valign="top"><h2>'.$classement_player.'</h2></td></tr><tr><td height=30></td></tr><tr><td align=center valign="top"><h2>'.$main_title.'</h2></td></tr><tr><td valign="middle" align="center"><h3>'.$titre_tableau_player.'</h3></td></tr></table></td></tr><tr><td height=10></td></tr><tr><td align="center">'.$comment_tableau_player.'</td></tr><tr><td height=10></td></tr>';
echo '<tr><td height=3></td></tr><tr><td align="center">';
echo '<tr><td>';

// Top players
////
if ($tableau == 'total')
{
	$j = 0;
	$k = 0;
	$nb_rows_kami = count(${'score_Top10_Best_'.$type_uppercase.'_kami'});
	$nb_rows_karavan = count(${'score_Top10_Best_'.$type_uppercase.'_karavan'});
	$nb_rows = $nb_rows_kami + $nb_rows_karavan;

	$names = array();
	$scores = array();
	$factions = array();
	for ($i = 0; $i < $nb_rows; $i++)
	{
		if ($j < $nb_rows_kami && ($k >= $nb_rows_karavan || ${'score_Top10_Best_'.$type_uppercase.'_kami'}[$j] >= ${'score_Top10_Best_'.$type_uppercase.'_karavan'}[$k]))
		{
			$names[$i] = ${'nom_Top10_Best_'.$type_uppercase.'_kami'}[$j];
			$scores[$i] = ${'score_Top10_Best_'.$type_uppercase.'_kami'}[$j];
			$factions[$i] = 'Kami';
			$j++;
		}
		else if ($k < $nb_rows_karavan && ($j >= $nb_rows_kami || ${'score_Top10_Best_'.$type_uppercase.'_karavan'}[$k] > ${'score_Top10_Best_'.$type_uppercase.'_kami'}[$j]))
		{
			$names[$i] = ${'nom_Top10_Best_'.$type_uppercase.'_karavan'}[$k];
			$scores[$i] = ${'score_Top10_Best_'.$type_uppercase.'_karavan'}[$k];
			$factions[$i] = 'Karavan';
			$k++;
		}
	}
	echo hof_score_table($names, $factions, $scores, $top_len, $user_login);
}
else if ($tableau == 'faction')
{
	$varname_suffix = '_Top10_Best_faction_'.$type_uppercase.'_'.$faction;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $user_login);
}
else if ($tableau == 'chantier')
{
	$varname_suffix = '_Top10_Best_'.$type_uppercase.'_'.$race.'_'.$faction;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $user_login);
}	
else if ($tableau == 'acte')
{
	$varname_suffix = '_Top10_Best_'.$type_uppercase.'_'.$race.'_'.$faction.'_Acte'.$noActe;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $user_login);
}

echo '</td></tr><tr><td height=20></td></tr><tr><td height=5 bgcolor="#101112"></td></tr><tr><td height=20></td></tr><tr><td valign="middle"  align="center"><table border=0  align="center"><tr><td align=center valign="top"><h2>'.$classement_guild.'</h2></td></tr><tr><td height=30></td></tr><tr><tr><td align=center valign="top"><h2>'.$main_title.'</h2></td></tr><tr><td valign="middle" align="center"><h3>'.$titre_tableau_guild.'</h3></td></tr></table></td></tr><tr><td height=10></td></tr><tr><td align="center">'.$comment_tableau_guild.'</td></tr><tr><td height=10></td></tr>';
echo '<tr><td>';

// Top guilds
////
if ($tableau == 'total')
{
	$j = 0;
	$k = 0;
	$nb_rows_kami = count(${'score_Top10_Best_Guild_'.$type_uppercase.'_kami'});
	$nb_rows_karavan = count(${'score_Top10_Best_Guild_'.$type_uppercase.'_karavan'});
	$nb_rows = $nb_rows_kami + $nb_rows_karavan;
	
	$names = array();
	$scores = array();
	$factions = array();
	for ($i = 0; $i < $nb_rows; $i++)
	{
		if ($j < $nb_rows_kami && ($k >= $nb_rows_karavan || ${'score_Top10_Best_Guild_'.$type_uppercase.'_kami'}[$j] >= ${'score_Top10_Best_Guild_'.$type_uppercase.'_karavan'}[$k]))
		{
			$names[$i] = ${'nom_Top10_Best_Guild_'.$type_uppercase.'_kami'}[$j];
			$scores[$i] = ${'score_Top10_Best_Guild_'.$type_uppercase.'_kami'}[$j];
			$factions[$i] = 'Kami';
			$j++;
		}
		else if ($k < $nb_rows_karavan && ($j >= $nb_rows_kami || ${'score_Top10_Best_Guild_'.$type_uppercase.'_karavan'}[$k] > ${'score_Top10_Best_Guild_'.$type_uppercase.'_kami'}[$j]))
		{
			$names[$i] = ${'nom_Top10_Best_Guild_'.$type_uppercase.'_karavan'}[$k];
			$scores[$i] = ${'score_Top10_Best_Guild_'.$type_uppercase.'_karavan'}[$k];
			$factions[$i] = 'Karavan';
			$k++;
		}
	}
	echo hof_score_table($names, $factions, $scores, $top_len, $guild_name);
}
else if ($tableau == 'faction')
{
	$varname_suffix = '_Top10_Best_Guild_faction_'.$type_uppercase.'_'.$faction;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $guild_name);
}
else if ($tableau == 'chantier')
{
	$varname_suffix = '_Top10_Best_Guild_'.$type_uppercase.'_'.$race.'_'.$faction;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $guild_name);
}
else if ($tableau == 'acte')
{
	$varname_suffix = '_Top10_Best_Guild_'.$type_uppercase.'_'.$race.'_'.$faction.'_Acte'.$noActe;
	echo hof_score_table_same_faction(${'nom'.$varname_suffix}, $faction, ${'score'.$varname_suffix}, $top_len, $guild_name);
}

echo '</td></tr><tr><td height=20></td></tr></table>';

if ($WANT_PROFILE)
{
	$time = microtime_float()- $time_start;
	echo '<br>PROFILE: total time = '.$time.'<br>';
}

echo '</body>';
echo '</html>';

?>