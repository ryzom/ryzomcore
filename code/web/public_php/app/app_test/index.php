<?php

define('APP_NAME', 'app_test');

include_once('../config.php');
include_once('../lang.php');
include_once('lang.php');

// Ask to authenticate user (using ingame or session method) and fill $user with all information
ryzom_app_authenticate($user, true);

// Enable debug logs only for DEVS
if (in_array('DEV', $user['groups']))
	ryLogger::getInstance()->enable = true;

// Debug log
p($user);

// Update user acces on Db
$db = ryDB::getInstance(APP_NAME);
$db->setDbDefs('test', array('id' => SQL_DEF_INT, 'num_access' => SQL_DEF_INT));

$num_access = $db->querySingleAssoc('test', array('id' => $user['id']));
if ($num_access)
	$db->update('test', array('num_access' => ++$num_access['num_access']), array('id' => $user['id']));
else
	$db->insert('test', array('num_access' => $num_access['num_access']=1, 'id' => $user['id']));

// Content
$c = _t('access', $num_access['num_access']).'<br/>';

echo ryzom_app_render(APP_NAME, $c, $user['ig']);

?>
