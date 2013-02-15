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

$tick_cache_timeout = 60;

function ryzom_time_tick() {
	$fn = RYAPI_URL.'data/cache/game_cycle.ticks';
	$handle = fopen($fn, "r");
	$version = fread($handle, 1);
	$raw_tick = fread($handle, 4);
	fclose($handle);
	$arr = unpack("V", $raw_tick);
	$tick = $arr[1];
	return sprintf("%u", $tick & 0xffffffff);
}


/**
 * Takes a computed ryzom time array and returns a SimpleXMLElement
 */
function ryzom_time_xml($rytime) {
	global $tick_cache_timeout;
	$out = ryzom_time_xml_without_cache($rytime);
	$filename = RYAPI_URL.'data/cache/game_cycle.ticks';
	$cache = $out->addChild('cache');
	$cache->addAttribute('created', filemtime($filename));
	$cache->addAttribute('expire', (filemtime($filename)+$tick_cache_timeout));
	return $out;
}

?>