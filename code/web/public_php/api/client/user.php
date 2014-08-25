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

function ryzom_user_get_info($cid) { 
	if (isset($_SESSION['user']))
		return $_SESSION['user'];
	
	$user = unserialize(base64_decode(ryzom_get_param('user')));
	$_SESSION['user'] = $user;
	return $user;
}

function ryzom_get_user_id($cid, $name, $creation_date) {
	if (isset($_SESSION['user']))
		return $_SESSION['user']['id'];
	
	$user = unserialize(base64_decode(ryzom_get_param('user')));
	$_SESSION['user'] = $user;

	return $user['id'];
}


?>
