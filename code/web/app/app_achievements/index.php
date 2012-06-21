<?php

error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors","1");

define('APP_NAME', 'app_achievements');

require_once('../webig/config.php');
include_once('../webig/lang.php');
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
$user['ig'] = ($_REQUEST['ig']==1);

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
require_once("class/Tieable_inter.php");

require_once("class/AchMenu_class.php");
require_once("class/AchSummary_class.php");
require_once("class/AchCategory_class.php");
require_once("class/AchAchievement_class.php");
require_once("class/AchPerk_class.php");
require_once("class/AchObjective_class.php");



// Update user acces on Db
//$db = ryDB::getInstance(APP_NAME);
$DBc = ryDB::getInstance("ahufler");
/*$db->setDbDefs('test', array('id' => SQL_DEF_INT, 'num_access' => SQL_DEF_INT));

$num_access = $db->querySingleAssoc('test', array('id' => $user['id']));
if ($num_access)
	$db->update('test', array('num_access' => ++$num_access['num_access']), array('id' => $user['id']));
else
	$db->insert('test', array('num_access' => $num_access['num_access']=1, 'id' => $user['id']));

// Content
$c = _t('access', $num_access['num_access']).'<br/>';*/

#$c = var_export($user,true);



echo ryzom_app_render("achievements", ach_render(), $_USER->isIG());

?>
