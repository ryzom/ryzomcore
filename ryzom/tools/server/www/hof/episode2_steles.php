<?php

if (isset($_GET['lang']))
{
	$lang = &$_GET['lang'];
	if ($lang != 'fr' && $lang != 'en' && $lang != 'de')
		die('invalid lang');
}

$faction = &$_GET['faction'];
$race = &$_GET['race'];
$self = &$_SERVER['PHP_SELF'];
$page_en_cours = &$_GET['page_en_cours'];
if ($page_en_cours =='')
	$page_en_cours = 0;


include 'episode2_textes_'.$faction.'_'.$lang.'.php';

function stele_menu_item($label, $page, $page_active, $page_en_cours)
{
	global $self, $lang, $race, $faction;
	if ($page_en_cours!=$page_active)
	return '<a href="episode2_steles.php?lang='.$lang.'&page_en_cours='.$page_active.'&faction='.$faction.'&race='.$race.'&page='.$page.'">'.$label.'</a>';
	else return $label;
	
}
if (isset($_GET['page']))
	$texte = $$_GET['page'];
else
	$texte = $texteDefault;

echo '<html>';
echo '<head><title>'.$titre.'</title></head>';
echo '<body bgcolor="#30353A">';

if ($faction=='kami')
	$couleur='1A807F';
else 
	$couleur='922121';

echo '<table height=365><tr><td height=15></td></tr><tr><td width=200 align="center"><H1>'.${'TITLE_'.$faction}.'</H1></td></tr><tr><td height=30></td></tr><tr><td><table><tr><td height=10 bgcolor=#101112></td><td height=10 bgcolor=#101112><td height=10 bgcolor=#101112></td></td><td bgcolor=#101112></td><td bgcolor=#101112></td></tr><tr><td><table><tr><td width=10 height=328 bgcolor=#101112></td></tr><tr><td></td></tr></table></td></tr><td height=280 valign="top"><table height=285 align="center">';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center" width=290><H4>'.stele_menu_item($menu1, 'texte_stele_'.$faction.'_1',1 ,$page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu2, 'texte_stele_'.$faction.'_2',2, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu3, 'texte_stele_'.$faction.'_3',3, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu4, 'texte_stele_'.$faction.'_4',4, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu5, 'texte_stele_'.$faction.'_5',5, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu6, 'texte_stele_'.$faction.'_6',6, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu7, 'texte_stele_'.$faction.'_7',7, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item(${'menu8'.$race}, 'texte_stele_'.$faction.'_8'.$race,8, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu9, 'texte_stele_'.$faction.'_9',9, $page_en_cours).'</H4></td></tr><tr><td height=2 bgcolor=#101112></td></tr>';
echo '<tr><td bgcolor=#'.$couleur.' valign="middle" height=30 align="center"><H4>'.stele_menu_item($menu10, 'texte_stele_'.$faction.'_10',10, $page_en_cours).'</H4></td></tr>';
echo '<tr><td height=10 bgcolor=#101112></td></tr><tr><td height=150></td></tr></table></td><td width=10 bgcolor=#101112></td>';
echo '<td width=500 align="center"><table><tr><td height=15></td></tr><tr>';
if ($page_en_cours!=8 && $page_en_cours!=0)
	echo '<td align="center" height=10><H2>'.${'menu'.$page_en_cours}.'</H2><br></td>';
else if ($page_en_cours!=0)
	echo '<td align="center" height=10><H2>'.${'menu'.$page_en_cours.$race}.'</H2><br></td>';
echo '</tr><tr><td height=255 align="center" valign="top"><table><tr><td width=20></td><td>'.$texte.'</td><td width=20></td></table></td></tr></table></td><td width=10 bgcolor=#101112></td></tr>';
echo '<tr><td height=10 ></td><td height=10 ></td><td height=10 bgcolor=#101112></td><td bgcolor=#101112></td><td bgcolor=#101112></td></tr></table></td></tr></table>';


echo '</body>';
echo '</html>';

?>