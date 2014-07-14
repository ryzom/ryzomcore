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


class ryUser {
	private $infos;

	function __construct($infos) {
		$this->infos = $infos;
	}

	function __get($name)
	{
		if (array_key_exists($name, $this->infos)) {
			return $this->infos[$name];
		} else {
			/** TODO **/
			return NULL;
		}
	}

	function inGroup($groups) {
		$groups = explode(':', $groups);
		foreach ($groups as $group) {
			if ($group != '') {
				if (in_array($group, $this->groups))
					return true;
				if ('P_'.$this->id == $group)
					return true;
				if ('G_'.$this->guild_id == $group)
					return true;
				if ($group == '*')
					return true;
			}
		}
		return false;
	}
}

function ryzom_auth_user($ask_login=true, $welcome_message='') {
	global $user, $_USER;

	$result = ryzom_app_authenticate($user, $ask_login, $welcome_message, true);
	$_USER = new RyUser($user);
	return $result;
}

function _user() {
	global $_USER;
	return $_USER;
}

?>
