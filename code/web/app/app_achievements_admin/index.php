<?php

error_reporting(E_ALL ^ E_NOTICE);
ini_set("display_errors","1");

define('APP_NAME', 'app_achievements_admin');

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
$user['admin'] = true;

require_once("class/RyzomAdmin_class.php");
$_USER = new RyzomAdmin($user);

if($_USER->isIG()) {
	die("IG disabled for admin tool!");
}

$DBc = ryDB::getInstance("app_achievements");


echo ryzom_app_render("achievements admin", $c, $_USER->isIG());

?>
