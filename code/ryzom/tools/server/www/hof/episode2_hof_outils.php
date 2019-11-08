<?php

include_once('../config.php');

function hof_init($hdt_files, $txt_files)
{
	$include_files = array();

	if (!is_array($hdt_files))
		$hdt_files = array($hdt_files);

	if (!is_array($txt_files))
	{
		if($txt_files!='')
			$txt_files = array($txt_files);
	}

	if (!isset($_GET['shard']))
		die('shard is missing');

	$shard = &$_GET['shard'];
	if (!is_numeric($shard))
		die('shard is invalid');

	foreach ($hdt_files as $hdt_file)
	{
		global	$USERS_DIR;

		$include_files[] = $USERS_DIR.'/'.strtolower($shard).'/'.str_replace('.hdt', '.php', $hdt_file);
	}

	if (isset($_GET['lang']))
	{
		$lang = &$_GET['lang'];
		if ($lang != 'fr' && $lang != 'en' && $lang != 'de')
			die('invalid lang');
	}
	else
		$lang = 'en';

	if (is_array($txt_files))
	{
		foreach ($txt_files as $txt_file)
		{
			$include_files[] = str_replace('%lang%', $lang, $txt_file);
		}
	}

	return $include_files;
}

function hof_score_table($names, $factions, $scores, $max_rows, $highlight_name)
{
	if ($_GET['faction'] == 'kami')
	{
		$color_tab = '#1A807F';
		$color_tab_bis = '#3E9D9C';
		$color_tab_titles = '#242E2E';
		$color_highlight = '#922121';
	}
	else if ($_GET['faction'] == 'karavan')
	{
		$color_tab = '#922121';
		$color_tab_bis = '#A04747';
		$color_tab_titles = '#322727';
		$color_highlight = '#1A807F';
	}
	
	if ($_GET['lang'] == 'fr')
	{
		$txt_position = 'POSITION';
		$txt_name = 'NOM';
		$txt_faction = 'FACTION';
		$txt_score = 'SCORE';
	}
	else if ($_GET['lang'] == 'en')
	{
		$txt_position = 'POSITION';
		$txt_name = 'NAME';
		$txt_faction = 'FACTION';
		$txt_score = 'SCORE';
	}
	else if ($_GET['lang'] == 'de')
	{
		$txt_position = 'PLATZ';
		$txt_name = 'NAME';
		$txt_faction = 'FRAKTION';
		$txt_score = 'PUNKTE';
	}
	
	$nb_rows = count($names);
	if ($nb_rows == 0)
		return '';
	if (count($factions) != $nb_rows || count($scores) != $nb_rows)
		die('wrong params: different nb of rows');

	if ($max_rows > $nb_rows)
		$max_rows = $nb_rows;

	$found_names = array();
	$top_names = array();
	$top_factions = array();
	$top_scores = array();
	$top_colors = array();
	$nb_top_rows = 0;
	$top_index = 0;
	$found_hl_name_in_top = false;
	foreach ($names as $i => $name)
	{
		if ($top_index < $max_rows)
		{
			if (!isset($found_names[strtolower($name)]))
			{
				$found_names[strtolower($name)] = true;
				if (strtolower($name) == strtolower($highlight_name))
				{
					$line_color = $color_highlight;
					$found_hl_name_in_top = true;
				}
				else if ($top_index % 2 == 0)
					$line_color = $color_tab_bis;
				else
					$line_color = $color_tab;
				
				$top_names[$top_index] = $name;
				$top_factions[$top_index] = $factions[$i];
				$top_scores[$top_index] = $scores[$i];
				$top_colors[$top_index] = $line_color;
				$top_index++;
				$nb_top_rows = $top_index;
			}
		}
		else
		{
			if ($found_hl_name_in_top)
				break;
			
			if (!isset($found_names[strtolower($name)]))
			{
				$found_names[strtolower($name)] = true;
				if (strtolower($name) == strtolower($highlight_name))
				{
					$top_names[$top_index] = $name;
					$top_factions[$top_index] = $factions[$i];
					$top_scores[$top_index] = $scores[$i];
					$top_colors[$top_index] = $color_highlight;
					break;
				}
				$top_index++;
			}
		}
	}

	$vsep = '<td width=2 bgcolor="#101112"></td>';
	$hsep = '<tr bgcolor="#101112"><td height=2></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>';

	$res = '<table width=608 bgcolor="#101112" align="center" valign="middle"><tr><td height=8 width=5></td><td width=600></td><td width=5></td></tr>';
	$res .= '<tr><td></td><td align="center"><table bgcolor="'.$color_tab.'" width=591 border=0>';
	$res .= '<tr align="center" valign="middle" bgcolor="'.$color_tab_titles.'">';
	$res .= '<td width=85 height=20><H5>'.$txt_position.'</H5></td>'.$vsep;
	$res .= '<td width=150><H5>'.$txt_name.'</H5></td>'.$vsep;
	$res .= '<td width=150><H5>'.$txt_faction.'</H5></td>'.$vsep;
	$res .= '<td width=200><H5>'.$txt_score.'</H5></td></tr>'.$hsep;

	$previous_i = -1;
	foreach ($top_names as $i => $name)
	{
		$score = $top_scores[$i];
		$faction = $top_factions[$i];
		$line_color = $top_colors[$i];

		if ($previous_i == $nb_top_rows - 1)
		{
			$res .= $hsep;
			if ($i > $nb_top_rows)
			{
				$res .= '<tr bgcolor="'.$color_tab_titles.'" align="center" valign="middle"><td height=20>...</td>'.$vsep.'<td>...</td>'.$vsep.'<td>...</td>'.$vsep.'<td>...</td></tr>';
				$res .= $hsep;
			}
		}

		$res .= '<tr bgcolor="'.$line_color.'" align="center" valign="middle">';
		$res .= '<td height=20>'.($i+1).'</td>'.$vsep.'<td>'.$name.'</td>'.$vsep.'<td>'.$faction.'</td>'.$vsep.'<td>'.$score.'</td>';
		$res .= '</tr>';

		if ($i + 1 < $nb_top_rows)
			$res .= $hsep;
		
		$previous_i = $i;
	}

	$res .= '</table></td><td></td></tr><tr><td height=8 width=5></td><td width=600></td><td width=5></td></tr></table>';
	return $res;
}

function hof_score_table_same_faction($names, $faction, $scores, $max_rows, $highlight_name)
{
	$faction = ucfirst($faction);
	$factions = array();
	foreach ($names as $i => $name)
		$factions[$i] = $faction;
	return hof_score_table($names, $factions, $scores, $max_rows, $highlight_name);
}

function hof_progress_bar($percent, $termine, $width=200, $height=10, $left_color='#0D0D0D', $right_color='#281F1A')
{
	if (is_float($percent) || is_double($percent))
		$percent = (int)$percent;

	if (!is_int($percent))
		die('invalid arg');

	if ($percent > 100)
		$percent = 100;

	$left_text = $percent;
	$right_text = '';
	if ($percent > 0)
	{
		if ($percent < 30)
			$left_color='#9B1515';
		else if ($percent < 70)
			$left_color='#A6AE25';
		else if ($percent < 100)
			$left_color='#108E42';
		else
			$left_color='#1442DA';
		//if ($termine == 0)
		//	$left_color='#0D0D0D';

		$res = '<table width='.$width.'><tr>';
		$res .= '<td bgcolor="'.$left_color.'" width="'.$percent.' %" height='.$height.' valign="middle" align="center">'.$left_text.'%</td>';
		$res .= '<td bgcolor="'.$right_color.'">'.$right_text.'</td>';
		$res .= '</tr></table>';
	}
	else
	{
		$left_color='#281F1A';
		$res = '<table width='.$width.'><tr>';
		$res .= '<td bgcolor="'.$left_color.'" width="100 %" height='.$height.' valign="middle" align="center">0%</td>';
		$res .= '</tr></table>';
	}
	return $res;
}

function hof_progress_bar_huge($percent, $width=200, $height=25, $left_color='#077F64', $right_color='#281F1A')
{
	if (is_float($percent) || is_double($percent))
		$percent = (int)$percent;

	if (!is_int($percent))
		die('invalid arg');

	if ($percent > 100)
		$percent = 100;

	$left_text = $percent;
	$right_text = '';
	if ($percent > 0)
	{
		if ($percent < 30)
			$left_color='#9B1515';
		else if ($percent < 70)
			$left_color='#A6AE25';
		else if ($percent < 100)
			$left_color='#108E42';
		else
			$left_color='#1442DA';

		$res = '<table width='.$width.'><tr>';
		$res .= '<td bgcolor="'.$left_color.'" width="'.$percent.' %" height='.$height.' valign="middle" align="center">'.$left_text.'%</td>';
		$res .= '<td bgcolor="'.$right_color.'">'.$right_text.'</td>';
		$res .= '</tr></table>';
	}
	else
	{
		$left_color='#281F1A';
		$res = '<table width='.$width.'><tr>';
		$res .= '<td bgcolor="'.$left_color.'" width="100 %" height='.$height.' valign="middle" align="center">0%</td>';
		$res .= '</tr></table>';
	}
	return $res;
}

function hof_link($url, $text)
{
	$res = '<a href="'.$url.'">'.$text.'</a>';

	return $res;
}

function hof_margin_table($content, $margin)
{
	$res = '<table border='.$margin.'><tr><td>'.$content.'</td></tr></table>';

	return $res;
}

function hof_tab($url, $text)
{
	$res = array();
	$res['inactive'] = hof_link($url, $text);
	$res['active'] = $text;

	return $res;
}

function hof_tab_hgroup($tabs, $active_tab_id)
{
	$res = '<table valign="middle" width=825 align="center" cellspacing=20><tr><td width=7></td>';
	foreach ($tabs as $tab_id => $tab)
	{
		if ($tab_id == $active_tab_id)
			$content = $tab['active'];
		else
			$content = $tab['inactive'];
		
		$res .= '<td width=120 bgcolor="#101112"><H1>'.hof_margin_table($content, 8).'</H1></td><td width=7></td>';
	}
	$res .= '</tr><td width=7></td></table>';

	return $res;
}

function hof_tab_hgroup_tableaux($tabs, $active_tab_id)
{
	$res = '<table valign="middle" width=825 align="center" cellspacing=20><tr><td width=100></td>';
	foreach ($tabs as $tab_id => $tab)
	{
		if ($tab_id == $active_tab_id)
			$content = $tab['active'];
		else
			$content = $tab['inactive'];
		
		if ($content == 'chantier')
			$res .= '<td width=90 bgcolor="#101112"><H5>'.hof_margin_table($content, 8).'</H5></td><td width=10 bgcolor="#101112"></td>';
		else
			$res .= '<td width=90 bgcolor="#101112"><H5>'.hof_margin_table($content, 8).'</H5></td><td width=10></td>';
	}
	$res .= '</tr><td width=100></td></table>';

	return $res;
}

?>