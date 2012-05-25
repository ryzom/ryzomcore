<?php

define('APP_NAME', 'app_achievements');

require_once('../webig/config.php');
include_once('../webig/lang.php');
include_once('lang.php');
require_once('conf.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, true);

if($user['ig']) {
	require_once("include/ach_render_ig.php");
}
else {
	require_once("include/ach_render_web.php");
}
require_once("include/ach_render_common.php");

require_once("include/AchMenu_class.php");
require_once("include/AchSummary_class.php");
require_once("include/AchCategory_class.php");
require_once("include/AchCommon_class.php");


// Update user acces on Db
$db = ryDB::getInstance(APP_NAME);
/*$db->setDbDefs('test', array('id' => SQL_DEF_INT, 'num_access' => SQL_DEF_INT));

$num_access = $db->querySingleAssoc('test', array('id' => $user['id']));
if ($num_access)
	$db->update('test', array('num_access' => ++$num_access['num_access']), array('id' => $user['id']));
else
	$db->insert('test', array('num_access' => $num_access['num_access']=1, 'id' => $user['id']));

// Content
$c = _t('access', $num_access['num_access']).'<br/>';*/

$c = var_export($user,true);

$c .= "<center><table>
	<tr>
		<td valign='top'>";
		
		$menu = new AchMenu($_REQUEST['mid'],$user['lang']);

		$c .= ach_render_menu($menu);
		
$c .= "</td>
		<td width='645px'>";

/*for($i=0;$i<15;$i++) {
	$c .= ach_render_box_done("Bejeweled");
}*/

if($menu->isSelected()) {
	$cat = new AchCategory($menu->getCat(),$user['lang']);
}
else {
	$cat = new AchSummary(12,$user['lang']);
}

$c .= ach_render_category($cat);

$c .= "</td>
	</tr>
</table></center>";

echo ryzom_app_render("achievements", $c, $user['ig']);

?>
