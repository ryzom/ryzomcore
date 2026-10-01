<?php

error_reporting(E_ERROR | E_PARSE);

include_once('../config.php');
include_once('../libs/login_translations.php');
include_once('../libs/login_service_itf.php');
include_once('../libs/nel_message.php');
include_once('../libs/domain_info.php');
include_once('../ring/join_shard.php');
include_once('modules/r2_login_db.php');
include_once('modules/r2_login_logs.php');
include_once('modules/r2_login_logincb.php');
include_once('modules/r2_login_domain.php');
include_once('modules/r2_login_user.php');

if (!isset($_GET['cmd']))
	die(errorMsgBlock(3002));

set_error_handler('err_callback');
session_start();
// For error handling, buffer all output
ob_start('ob_callback_r2login');


$submittedLang = isset($_GET['lg']) ? $_GET['lg'] : 'unknown';
$submittedLang = addslashes(substr($submittedLang, 0, 10));
$cmd = isset($_GET['cmd'])?addslashes(substr($_GET['cmd'], 0, 7)):'';
$login = isset($_GET['login'])?addslashes(substr($_GET['login'], 0, 20)):'';
$password = isset($_GET['password'])?$_GET['password']:'';
$clientApplication = isset($_GET['clientApplication'])?addslashes(substr($_GET['clientApplication'], 0, 20)):'';

if ($clientApplication == 'ryzom_live')
	define('STATUS', trim(file_get_contents('status/server_open_status')));
else
	define('STATUS', 'ds_ok');


// Inits
$RingDb = new LoginDB(DB_NEL_HOST, DB_NEL_USER, DB_NEL_PASS, DB_NEL_NAME);
$User = new RingUser($RingDb, $login, $password, $clientApplication, $submittedLang);

switch($cmd) {
	case 'ask':
		// client ask for a login salt
		$User->askSalt();
		die();

	case 'login':
		// client sent is login info
		$ServerDomain = new ServerDomain($RingDb, $clientApplication);
		if ($User->checkValidity()) {

			if (AUTO_CREATE_RING_INFO)
				$User->createRingInfo($ServerDomain);

			// Check Status of Server Domain and block accesses (ds_close, ds_restricted, ...)
			if ($clientApplication == 'ryzom_live' || $clientApplication == 'ryzom_beta')
				$ServerDomain->checkStatus(STATUS, $User);

			$LSaddr = explode(":", $ServerDomain->domainInfo['login_address']);
			// ask for a session cookie to the login service
			$Login = new LoginCb();
			$Login->init($RingDb, $ServerDomain);

			$res = "";
			$Login->connect($LSaddr[0], $LSaddr[1], $res);
			//die(errorMsgBlock(3003, $LSaddr[0]."/".$LSaddr[1]."/".$_SERVER['REMOTE_ADDR']));
			$Login->login($User->uid, $_SERVER['REMOTE_ADDR'], $ServerDomain->id);

			if (!$Login->waitCallback())
				die(errorMsgBlock(3003, $ServerDomain->domainInfo['login_address']));

		} else
			die(errorMsgBlock(3003, $ServerDomain->domainInfo['login_address']));

		die();
}

