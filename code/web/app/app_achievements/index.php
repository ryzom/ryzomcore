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

if($user['ig']) {
	require_once("include/ach_render_ig.php");
}
else {
	require_once("include/ach_render_web.php");
}
require_once("include/ach_render_common.php");

require_once("include/AchCommon_class.php");
require_once("include/AchMenu_class.php");
require_once("include/AchSummary_class.php");
require_once("include/AchCategory_class.php");

require_once("include/AchAchievement_class.php");
require_once("include/AchPerk_class.php");
require_once("include/AchObjective_class.php");



// Update user acces on Db
//$db = ryDB::getInstance(APP_NAME);
$db = ryDB::getInstance(APP_NAME);
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
		<td valign='top'><div style='width:230px;font-weight:bold;font-size:14px;'>";
		#$_REQUEST['mid'] = 1;
		
		$menu = new AchMenu($_REQUEST['cat'],$user['lang']);

		$c .= ach_render_menu($menu);
		
$c .= "</div></td>
		<td width='645px' valign='top'>";

/*for($i=0;$i<15;$i++) {
	$c .= ach_render_box_done("Bejeweled");
}*/

$open = $menu->getOpenCat();

if($open != 0) {
	$cat = new AchCategory($open,1,$user['lang']);
}
else {
	$cat = new AchSummary($menu,1,8,$user['lang']);
	$c .= ach_render_summary_header($user['lang']);
}

$c .= ach_render_category($cat);
if($open == 0) {
	$c .= ach_render_summary_footer($user['lang'],$cat,1);
}

$c .= "</td>
	</tr>
</table></center>";

echo ryzom_app_render("achievements", $c, $user['ig']);

?>
