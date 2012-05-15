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
	// User information
	global $_RYZOM_API_CONFIG;
	$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING);
	$sql = "SELECT char_name, race, civilisation, cult, guild_id, creation_date, last_played_date FROM characters WHERE char_id = $cid";
	$result = $db->query($sql) or die('Could not query on ryzom_user_get_info');
	$found = $db->num_rows($result) >= 1;
	if (!$found)
		die('Could not found on ryzom_user_get_info');
	$row = $db->fetch_assoc($result);
	$db->free_result($result);
	if ($row) {
		$row['race'] = substr($row['race'], 2);
		$row['cult'] = substr($row['cult'], 2);
		$row['civ'] = substr($row['civilisation'], 2);
		if ($row['guild_id'] != '0') {
			//$xml = @simplexml_load_file(ryzom_guild($row['guild_id'], false));
			$xml = false;
			if ($xml !== false) {
				$row['guild_icon'] = (string)$xml->icon;
				$row['guild_name'] = (string)$xml->name;
				$result = $xml->xpath("/guild/members/member[cid=$cid]");
				while(list( , $item) = each($result))
					$row['grade'] = (string)$item->grade;
			} else {
				$row['guild_name'] = 'UNKNOWN_GUILD_'.$row['guild_id']; // Unknow name (normal in yubo shard)
			}
		}
	}
	
	$uid = intval($cid / 16);
	$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_NEL);
	$sql = "SELECT Privilege FROM user WHERE UId = $uid";
	$result = $db->query($sql) or die("Could not query.");
	$priv_row = $db->fetch_row($result, MYSQL_NUM);
	$priv = $priv_row[0];
	$db->free_result($result);
	$groups = array();
	
	if (strpos($priv, ':DEV:') !== false) {
		$groups[] = 'DEV';
		$groups[] = 'SGM';
		$groups[] = 'GM';
		$groups[] = 'EM';
		$groups[] = 'EG';
		$groups[] = 'VG';
		$groups[] = 'G';
	}
	
	if (strpos($priv, ':SGM:') !== false) {
		$groups[] = 'SGM';
		$groups[] = 'GM';
		$groups[] = 'VG';
		$groups[] = 'G';
	}
	
	if (strpos($priv, ':GM:') !== false) {
		$groups[] = 'GM';
		$groups[] = 'VG';
		$groups[] = 'G';
	}

	if (strpos($priv, ':VG:') !== false) {
		$groups[] = 'VG';
		$groups[] = 'G';
	}

	if (strpos($priv, ':G:') !== false) {
		$groups[] = 'G';
	}
	
	if (strpos($priv, ':SEM:') !== false) {
		$groups[] = 'SEM';
		$groups[] = 'EM';
		$groups[] = 'EG';
	}
	
	if (strpos($priv, ':EM:') !== false) {
		$groups[] = 'EM';
		$groups[] = 'EG';
	}
	
	if (strpos($priv, ':EG:') !== false) {
		$groups[] = 'EG';
	}

	$groups[] = 'PLAYER';
	$row['groups'] = $groups;
	
	return $row;
}

?>
