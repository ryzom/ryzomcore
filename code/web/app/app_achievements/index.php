<?php

define('APP_NAME', 'app_achievements');

include_once('../config.php');
include_once('../lang.php');
include_once('lang.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, true);

if($user['ig']) {
	include_once("include/ach_render_ig.php");
}
else {
	include_once("include/ach_render_web.php");
}
include_once("include/ach_render_common.php");


// Update user acces on Db
/*$db = ryDB::getInstance(APP_NAME);
$db->setDbDefs('test', array('id' => SQL_DEF_INT, 'num_access' => SQL_DEF_INT));

$num_access = $db->querySingleAssoc('test', array('id' => $user['id']));
if ($num_access)
	$db->update('test', array('num_access' => ++$num_access['num_access']), array('id' => $user['id']));
else
	$db->insert('test', array('num_access' => $num_access['num_access']=1, 'id' => $user['id']));

// Content
$c = _t('access', $num_access['num_access']).'<br/>';*/

$c = "<center><table>
	<tr>
		<td valign='top'>awesome menu</td>
		<td width='645px'>";

for($i=0;$i<15;$i++) {
	$c .= ach_render_box_done("Bejeweled");
}

$c .= "</td>
	</tr>
</table></center>";

echo ryzom_app_render("achievements", $c, $user['ig']);

?>
