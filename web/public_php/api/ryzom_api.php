<?php
/* Copyright (C) 2009 Winch Gate Property Limited
 *
 * This file is part of ryzom_api.
 * ryzom_api is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ryzom_api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ryzom_api.  If not, see <http://www.gnu.org/licenses/>.
 */

session_start();

/*
function my_session_start()
{
	if (isset($_COOKIE['PHPSESSID'])) {
		$sessid = $_COOKIE['PHPSESSID'];
	} else if (isset($_GET['PHPSESSID'])) {
		$sessid = $_GET['PHPSESSID'];
	} else {
		session_start();
		return false;
	}

	if (!preg_match('/^[a-z0-9]{32}$/', $sessid)) {
		return false;
	}
	session_start();

	return true;
}
*/


// Global defines
if (!defined('ON_IPHONE')) {
	if(isset($_SERVER['HTTP_USER_AGENT']))
		define('ON_IPHONE', strpos($_SERVER['HTTP_USER_AGENT'], 'Ryzom'));
	else
		define('ON_IPHONE', false);
}

require_once("common/config.php");

$includes = array('auth', 'utils', 'user', 'time');

foreach ($includes as $include) {
	if (RYAPI_MODE == 'server') {
		require_once('server/config.php');
		require_once("server/$include.php");
	} else {
		require_once('client/config.php');
		require_once("client/$include.php");
	}

	require_once("common/$include.php");
}

require_once("common/db_lib.php");
require_once("common/db_defs.php");
require_once("common/render.php");

?>
