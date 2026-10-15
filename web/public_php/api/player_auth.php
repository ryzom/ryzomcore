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

require_once('ryzom_api.php');
$cid = ryzom_get_param('cid');
$name = ryzom_get_param('name');
$authserver = ryzom_get_param('authserver');
$authkey = ryzom_get_param('authkey');

if ($authserver) {
	if (ryzom_authenticate_with_serverkey($cid, $name, $authserver, $authkey))
		die('1');
	die('0');
}

if ((isset($_SERVER['HTTP_USER_AGENT']) && strpos($_SERVER['HTTP_USER_AGENT'], 'Ryzom')) || ryzom_get_param('ig')) {
		echo 'ig';
	if (ryzom_authenticate_ingame($cid, $name, $authkey)) {
		echo 'ok';
		$user_infos = ryzom_user_get_info($cid);
		echo ryzom_get_user_id($cid, $name, $user_infos['creation_date']);
		die('1');
	}
	die('0');
} else  {
	echo ryzom_authenticate_with_session($name, $cid, $error_message);
}

?>
