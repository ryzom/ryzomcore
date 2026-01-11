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

/* Basic constants */
/* 1 IG hour = 3 IRL minutes = 1800 ticks */
define('RYTIME_HOUR_TICKS', 1800);
define('RYTIME_DAY_HOURS', 24);
define('RYTIME_SEASON_DAYS', 90);
define('RYTIME_MONTH_DAYS', 30);
define('RYTIME_CYCLE_MONTHS', 12);
define('RYTIME_JY_CYCLES', 4);
define('RYTIME_WEEK_DAYS', 6);
/* 0 = spring, 1 = summer, 2 = automn, 3 = winter */
define('RYTIME_CYCLE_SEASONS', 4);
/* Tick is offset on server of 61 days. */
define('RYTIME_TICK_OFFSET', 61 * RYTIME_DAY_HOURS * RYTIME_HOUR_TICKS);

define('RYTIME_START_JY', 2568);

/* Helpers */
define('RYTIME_CYCLE_DAYS',RYTIME_CYCLE_MONTHS * RYTIME_MONTH_DAYS);
define('RYTIME_JY_DAYS', RYTIME_CYCLE_DAYS * RYTIME_JY_CYCLES);
define('RYTIME_JY_MONTHS', RYTIME_CYCLE_MONTHS * RYTIME_JY_CYCLES);

// Takes a server tick and returns a computed array
function ryzom_time_array($tick) {
	$out = array();
	$out["server_tick"] = $tick;

	$time_in_hours = ($tick-RYTIME_TICK_OFFSET) / RYTIME_HOUR_TICKS;
	$day = $time_in_hours / RYTIME_DAY_HOURS;

	$out["jena_year"] = floor($day / RYTIME_JY_DAYS) + RYTIME_START_JY;
	if ($day < 0) $day = RYTIME_JY_DAYS - abs($day) % RYTIME_JY_DAYS;
	$out["day_of_jy"] = $day % RYTIME_JY_DAYS;
	$out["month_of_jy"] = floor($out["day_of_jy"] / RYTIME_MONTH_DAYS);

	$out["cycle"] = floor($out["day_of_jy"] / RYTIME_CYCLE_DAYS);
	$out["day_of_cycle"] = $day % RYTIME_CYCLE_DAYS;
	$out["month_of_cycle"] = $out["month_of_jy"] % RYTIME_CYCLE_MONTHS;

	$out["day_of_month"] = $out["day_of_jy"] % RYTIME_MONTH_DAYS;
	$out["day_of_week"] = $day % RYTIME_WEEK_DAYS;

	$out["season"] = ($day / RYTIME_SEASON_DAYS) % RYTIME_CYCLE_SEASONS;
	$out["day_of_season"] = $day % RYTIME_SEASON_DAYS;

	$out["time_of_day"] = abs($time_in_hours) % RYTIME_DAY_HOURS;
	if ($time_in_hours < 0 && $out["time_of_day"]) $out["time_of_day"] = RYTIME_DAY_HOURS - $out["time_of_day"];

	return $out;
}

function ryzom_time_xml_without_cache($rytime) {
	$out = new SimpleXMLElement('<shard_time/>');
	foreach($rytime as $key => $value) {
		$out->addChild($key, $value);
	}
	return $out;
}

/**
 * Take number of the month (0-11) and returns its name
 */
function ryzom_month_name($month_number) {
	if ($month_number < 0 || $month_number > 11) return "bad month";

	$RYTIME_MONTHS = array(
		'Winderly', 'Germinally', 'Folially', 'Floris',
		'Medis', 'Thermis', 'Harvestor', 'Frutor',
		'Fallenor', 'Pluvia', 'Mystia', 'Nivia'
	);

	return $RYTIME_MONTHS[(int)$month_number];
}


/**
 * Take number of the day of week (0-5) and returns its name
 */
function ryzom_day_name($day_number) {
	if ($day_number < 0 || $day_number > 5) return "bad day of week";

	$RYTIME_DAYS = array(
		'Prima', 'Dua', 'Tria',
		'Quarta', 'Quinteth', 'Holeth'
	);

	return $RYTIME_DAYS[(int)$day_number];
}

/**
 * Take a computed ryzom time array and returns the formatted date
 * (Official 2004 Format without trailing JY)
 */
function ryzom_time_txt($rytime, $lang = "en") {
	if ($lang != "en" && $lang != "fr" && $lang != "de") $lang = "en";

	$RYTIME_AC = array(
		"de" => array("1. AZ", "2. AZ", "3. AZ", "4. AZ"),
		"en" => array("1st AC", "2nd AC", "3rd AC", "4th AC"),
		"fr" => array("1er CA", "2e CA", "3e CA", "4e CA")
	);

	# Day, Month DayOfMonth, CycleNth AC JY
	return sprintf("%sh - %s, %s %d, %s %d",
		$rytime["time_of_day"],
		ryzom_day_name($rytime["day_of_week"]),
		ryzom_month_name($rytime["month_of_cycle"]),
		$rytime["day_of_month"] + 1,
		$RYTIME_AC[$lang][$rytime["cycle"]],
		$rytime["jena_year"]);
}

?>