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

function ryzom_authenticate_with_serverkey($cid, $name, $authserver, $authkey) {
	$rawkey = RYAPI_COOKIE_KEY.$name.$cid.$authserver;
	$authkey = md5($rawkey);
	if ($authkey != $authkey) return false;
	return true;
}

function ryzom_authenticate_ingame($cid, $name, $authkey) {
	return file_get_contents(RYAPI_AUTH_SCRIPT) == '1';
}

// take the character name and the account password and check if it's valid
function ryzom_authenticate_with_char_and_password($character, $password, &$cid) {
	$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING);
	$char = mysql_real_escape_string($character, $db->_connection);
	$sql = "SELECT char_id, char_name, user_id, home_mainland_session_id FROM characters WHERE char_name = '$char'";
	$row = $db->query_single_row($sql);
	$character = $row['char_name'];
	$cid = $row['char_id'];
	$uid = $row['user_id'];
	mysql_select_db('nel', $db->_connection);
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
	}

	return false;
}



function ryzom_get_user_id($cid, $name, $creation_date) {
	$name = strtolower($name);

	$db = ryDB::getInstance('webig');
	$db->setDbDefs('players', array('id' => SQL_DEF_INT, 'cid' => SQL_DEF_INT, 'name' => SQL_DEF_TEXT, 'creation_date' => SQL_DEF_DATE, 'deleted' => SQL_DEF_BOOLEAN));

	$charProps = $db->querySingle('players', array('cid' => intval($cid), 'deleted' => 0));
	// new char => create record
	if (!$charProps) {
		$charProps = array('name' => $name, 'cid' => $cid, 'creation_date' => $creation_date, 'deleted' => 0);
		$charProps['id'] = $db->insert('players', $charProps);
		if (!$charProps['id'])
			die('ryDb New Char Error');
	} else {
		// char renamed => update record
		if ($charProps['name'] != $name)
			if (!$db->update('players', array('name' => $name), array('id' => $charProps['id'])))
				die('ryDb Rename Char Error');

		// char deleted and recreated => change to deleted
		if ($charProps['creation_date'] != $creation_date) {
			if (!$db->update('players', array('deleted' => 1), array('id' => $charProps['id'])))
				die('ryDb Delete char Error: '.$db->getErrors());
			$charProps = array('name' => $name, 'cid' => $cid, 'creation_date' => $creation_date, 'deleted' => 0);
			if (!$charProps['id'] = $db->insert('players', $charProps))
				die('ryDb New Char in Slot Error');
		}
	}
	return $charProps['id'];
}

?>
