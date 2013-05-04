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
 
// Get the guild from backup and return the filename of the final xml
function ryzom_guild($gid, $gz=false) {
	$out_filename = RYAPI_PATH."data/cache/guilds/guild_$gid.xml";
	if($gz) $out_filename .= '.gz';

	if(!file_exists($out_filename)) return ''; //ryzom_die('File not found', $gz?'xmlgz':'txt');

	return $out_filename;
}

// Get the guilds from backup and return the filename of the final xml
function ryzom_guilds($gz=false) {
	$out_filename = RYAPI_PATH.'data/cache/guilds/guilds_atys.xml';
	if($gz) $out_filename .= '.gz';

	if(!file_exists($out_filename)) return ''; //ryzom_die('File not found', $gz?'xmlgz':'txt');

	return $out_filename;
}

function ryzom_guild_icon($icon, $size) {
	$filename = RYAPI_PATH."data/cache/guild_icons/${icon}_$size.png";

	if($size != 'b' && $size != 's') die('Bad size parameter');
	if($icon == '') die('icon parameter cannot be empty');

	if(!file_exists($filename)) {
		$command = RYAPI_PATH."server/scripts/generate_guild_icon.sh $icon $size 2>&1";
		exec($command, $output, $result);
		if ($result != 0)
	    {
			echo $command;
			print_r($output);
			die('Cannot generate guild icon');
		}
	}
	return file_get_contents($filename);
}

function ryzom_guild_icon_url($icon, $size) {
	ryzom_guild_icon($icon, $size);
	return RYAPI_URL."data/cache/guild_icons/${icon}_$size.png";
}

?>
