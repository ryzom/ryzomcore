<?php

error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors","1");

define('APP_NAME', 'app_achievements');

require_once('../config.php');
include_once('../lang.php');
include_once('lang.php');
require_once('conf.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, false);

$user = array();
$user['id'] = 1;
$user['lang'] = 'en';
$user['name'] = 'Talvela';
$user['race'] = "r_matis";
$user['civilization'] = "c_neutral";
$user['cult'] = "c_neutral";

require_once("class/RyzomUser_class.php");
$_USER = new RyzomUser($user);

if($_USER->isIG()) {
	require_once("include/ach_render_ig.php");
}
else {
	require_once("include/ach_render_web.php");
}
require_once("include/ach_render_common.php");

require_once("class/RenderNodeIteraor_abstract.php");
require_once("class/AchList_abstract.php");

require_once("class/AchMenu_class.php");
require_once("class/AchSummary_class.php");
require_once("class/AchCategory_class.php");
require_once("class/AchAchievement_class.php");
require_once("class/AchPerk_class.php");
require_once("class/AchObjective_class.php");



// Update user acces on Db
//$db = ryDB::getInstance(APP_NAME);
$DBc = ryDB::getInstance(APP_NAME);
/*$db->setDbDefs('test', array('id' => SQL_DEF_INT, 'num_access' => SQL_DEF_INT));

$num_access = $db->querySingleAssoc('test', array('id' => $user['id']));
if ($num_access)
	$db->update('test', array('num_access' => ++$num_access['num_access']), array('id' => $user['id']));
else
	$db->insert('test', array('num_access' => $num_access['num_access']=1, 'id' => $user['id']));

// Content
$c = _t('access', $num_access['num_access']).'<br/>';*/

#$c = var_export($user,true);

$c = "<center><table>
	<tr>
		<td colspan='2'>".ach_render_yubopoints(1)."</td>
	</tr>
	<tr>
		<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
		#$_REQUEST['mid'] = 1;
		
		$menu = new AchMenu($_REQUEST['cat']);

		$c .= ach_render_menu($menu);
		
$c .= "</div></td>
		<td width='645px' valign='top'>";

/*for($i=0;$i<15;$i++) {
	$c .= ach_render_box_done("Bejeweled");
}*/

$open = $menu->getOpenCat();

if($open != 0) {
	$cat = new AchCategory($open,$_REQUEST['cult'],$_REQUEST['civ']);
}
else {
	$cat = new AchSummary($menu,8);
	$c .= ach_render_summary_header();
}

$c .= ach_render_category($cat);
if($open == 0) {
	$c .= ach_render_summary_footer($cat,1);
}

$c .= "</td>
	</tr>
</table></center>";

echo ryzom_app_render("achievements", $c, $_USER->isIG());

?>
