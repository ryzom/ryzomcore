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
}

// Ig method
function ryzom_authenticate_ingame($cid, $name, $authkey) {
	if (isset($_SESSION['user']))
		return true;

	if (ryzom_get_param('user'))
		return true;
	
	return false;
}

// Session method
function ryzom_authenticate_with_session($name, $redirect) {
	if (isset($_SESSION['user']))
		return true;

	if (ryzom_get_param('user'))
		return true;
	
	return false;
}

?>
