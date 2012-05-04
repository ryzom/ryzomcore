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

require_once(RYAPI_PATH.'client/config.php');

// Og (non-ryzom.com) method
function ryzom_authenticate_with_serverkey($cid, $name, $authserver, $authkey) {
	global $_RYZOM_API_CONFIG;
	$fn = $_RYZOM_API_CONFIG['auth_script'].'?name='.$name.'&cid='.$cid.'&authkey='.$authkey.'&authserver='.$authserver;

	$res = file_get_contents($fn);
	return $res == '1';
}

// Ig method
function ryzom_authenticate_ingame($cid, $name, $authkey) {
	global $_RYZOM_API_CONFIG;
	$fn = $_RYZOM_API_CONFIG['auth_script'].'?name='.$name.'&cid='.$cid.'&authkey='.$authkey.'&ig=1';

	$res = file_get_contents($fn);
	echo $res;
	return $res == '1';
}

// Session method
function ryzom_authenticate_with_session($name, $redirect) {
	global $_RYZOM_API_CONFIG;
	$fn = $_RYZOM_API_CONFIG['auth_script'].'?name='.$name;

	$res = file_get_contents($fn);
	return $res == '1';
}

?>
