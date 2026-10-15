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

include_once(RYAPI_PATH.'server/guilds.php');

function ryzom_authenticate_with_serverkey($cid, $name, $authserver, $authkey) {
	$rawkey = RYAPI_COOKIE_KEY.$name.$cid.$authserver;
	$authkey = md5($rawkey);
	if ($authkey != $authkey) return false;
	return true;
}

function ryzom_authenticate_ingame($shardid, $cid, $name, $authkey) {
	$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING);
	$uid = intval($cid / 16);
	$sql = "SELECT cookie FROM ring_users WHERE user_id = $uid";
	$row = $db->query_single_row($sql);

	$rawkey = $shardid.$name.$cid.'\''.trim($row['cookie']).'\'';
	$md5rawkey = md5($rawkey);
	return $authkey == $md5rawkey;
}

// take the character name and the account password and check if it's valid
function ryzom_authenticate_with_char_and_password($character, $password, &$cid) {
	$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING);
	$char = $db->escape_string($character);
	$schar = explode('@', $char);
	$_SESSION['dev_shard'] = 0;
	if (count($schar) == 2 && $schar[1] == RYAPI_DEV_SHARD) {
		$_SESSION['dev_shard'] = 1;
		$char = $schar[0];
		$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING_DEV);
	}
	$sql = "SELECT char_id, char_name, user_id, home_mainland_session_id FROM characters WHERE char_name = '$char'";
	$row = $db->query_single_row($sql);
	$character = $row['char_name'];
	$cid = $row['char_id'];
	$uid = $row['user_id'];
	$db->select_db('nel');
	$sql = "SELECT Password FROM user WHERE UId = $uid";
	$row = $db->query_single_row($sql);
	$ok = $row['Password'] == crypt($password, $row['Password']);
	return $ok;
}

function ryzom_authenticate_with_session(&$name, &$cid, &$error_message) {
	$c = '';

	$action = ryzom_get_param('action');
	if ($action == 'logout') {
		unset($_SESSION['name']);
		unset($_SESSION['cid']);
		unset($_SESSION['dev_shard']);
	}

	if (isset($_SESSION['name']) && ($name == '' || $_SESSION['name'] == $name)) {
		$name = $_SESSION['cid'];
		$cid = $_SESSION['cid'];
		return true;
	}

	$char = ryzom_get_param('char');
	$password = ryzom_get_param('password');

	if ($char && $password) {
		// check credentials
		if (ryzom_authenticate_with_char_and_password($char, $password, $cid)) {
			$_SESSION['name'] = $char;
			$_SESSION['cid'] = $cid;
			return true;
		} else {
			$error_message = 'bad_auth';
		}
	} else
		return NULL;

	return false;
}

?>






























