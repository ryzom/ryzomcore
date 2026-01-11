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


function ryzom_name_to_file($name)
{
	$r = '';
	for ($i=0; $i<strlen($name); ++$i)
	{
		if ($name[$i] == ' ')
			$r .= '_';
		else if ($name[$i] == '%' || $name[$i] <= chr(32) || $name[$i] >= chr(127))
			$r .= sprintf("%%%02x", ord($name[$i]));
		else
			$r .= $name[$i];
	}
	return $r;
}

// -------------------------------------
// get user home directory
// -------------------------------------
function ryzom_get_user_dir($user)
{
	global $_RYZOM_API_CONFIG;

	if ($user == "")
		die("INTERNAL ERROR CODE 1");

	$user = ryzom_name_to_file($user);
	return $_RYZOM_API_CONFIG['cookie_path'].'/'.$_RYZOM_API_CONFIG['shardid'].'/'.substr(strtolower($user), 0, 2).'/'.strtolower($user).'/';
}

?>
