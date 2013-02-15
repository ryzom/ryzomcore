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

include_once(RYAPI_PATH.'server/guilds.php');


function ryzom_get_user_id($cid, $name, $creation_date) {
	$name = strtolower($name);

	$db = ryDB::getInstance('webig');
	
	$dev_shard = ((isset($_SESSION['dev_shard']) && $_SESSION['dev_shard']) || (isset($_GET['shardid']) && $_GET['shardid'] == RYAPI_DEV_SHARDID))?1:0;
	if ($dev_shard)
		$_SESSION['dev_shard'] = 1;
	$charsWithSameName = $db->query('players', array('name' => $name, 'deleted' => 0, 'dev_shard' => $dev_shard));

	foreach ($charsWithSameName as $charWithSameName) {
		// Another char with same name => delete it
		if (intval($cid) != intval($charWithSameName['cid'])) {
			$db->update('players', array('deleted' => 1), array('id' => $charWithSameName['id']));
		}
	}

	$charProps = $db->querySingle('players', array('cid' => intval($cid), 'deleted' => 0, 'dev_shard' => $dev_shard));
	// new char => create record
	if (!$charProps) {
		$charProps = array('name' => $name, 'cid' => $cid, 'creation_date' => $creation_date, 'deleted' => 0, 'dev_shard' => $dev_shard);
		$charProps['id'] = $db->insert('players', $charProps);
		if (!$charProps['id'])
			die('ryDb New Char Error');
	} else {
		// char deleted and recreated => change to deleted
		if ($charProps['creation_date'] != $creation_date) {
			if (!$db->update('players', array('deleted' => 1), array('id' => $charProps['id'])))
				die('ryDb Delete char Error: '.$db->getErrors());
			$charProps = array('name' => $name, 'cid' => $cid, 'creation_date' => $creation_date, 'deleted' => 0, 'dev_shard' => $dev_shard);
			if (!$charProps['id'] = $db->insert('players', $charProps))
				die('ryDb New Char in Slot Error');
		} else {
			// char renamed => update record
			if ($charProps['name'] != $name)
				if (!$db->update('players', array('name' => $name), array('id' => $charProps['id'])))
					die('ryDb Rename Char Error');
		}
	}
	return $charProps['id'];
}

function ryzom_get_user_gender($id) {
        $db = ryDB::getInstance('webig');
        $player = $db->querySingle('players', array('id' => $id));
        if ($player) {
                if ($player['gender'])
                        return intval($player['gender']);
                $cid = $player['cid'];
                $xml = @simplexml_load_file(RYAPI_PATH.'data/cache/players/public/'.substr($cid, strlen($cid)-1).'/'.$cid.'.xml');
                if ($xml !== false) {
                        $gender = (string)$xml->public->_gender;
                        $db->update('players', array('gender' => intval($gender)+1), array('id' => $id));
                        return $gender+1;
                }
        }
        return false;
}

function ryzom_user_get_info($cid, $webprivs=false, $player_stats=false) {
	// User information
	global $_RYZOM_API_CONFIG;

	if (isset($_SESSION['dev_shard']) && $_SESSION['dev_shard'])
		$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING_DEV);
	else
		$db = new ServerDatabase(RYAPI_NELDB_HOST, RYAPI_NELDB_LOGIN, RYAPI_NELDB_PASS, RYAPI_NELDB_RING);
	$sql = "SELECT char_name, race, civilisation, cult, guild_id, creation_date, last_played_date FROM characters WHERE char_id = $cid";
	$result = $db->query($sql) or die('Could not query on ryzom_user_get_info');
	$found = $db->num_rows($result) >= 1;
	if (!$found)
		return array('char_name' => _t('guest'), 'cid' => $cid, 'ERROR' => 'unknown_user', 'groups' => array('GUEST'));
	$row = $db->fetch_assoc($result);
	$db->free_result($result);
	if ($row) {
		$row['race'] = substr($row['race'], 2);
		$row['cult'] = substr($row['cult'], 2);
		$row['civ'] = substr($row['civilisation'], 2);
		if ($row['guild_id'] != '0') {
		$xml = @simplexml_load_file(ryzom_guild($row['guild_id'], false));
		//	$xml = false;
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
	$priv_row = $db->fetch_row($result, MYSQLI_NUM);
	$priv = $priv_row[0];
	$db->free_result($result);
	$groups = array();

	$row['uid'] = $uid;
	$row['cid'] = $cid;
	$row['slot'] = $cid%16;

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
	if (isset($_SESSION['dev_shard']) && $_SESSION['dev_shard'])
		$groups[] = 'DEV_SHARD';

	if ($webprivs) {
		$db = new ServerDatabase(RYAPI_WEBDB_HOST, RYAPI_WEBDB_LOGIN, RYAPI_WEBDB_PASS, 'webig');
		$sql = 'SELECT web_privs FROM accounts WHERE uid = '.intval($cid/16);
		$result = $db->query($sql) or die("Could not query.".$db->get_error());
		if ($result->num_rows == 0)
			$db->query('INSERT INTO accounts (`uid`, `web_privs`) VALUES ('.intval($cid/16).', \'\')') or die("Could not query.".$db->get_error());
		$priv_row = $db->fetch_row($result, MYSQLI_NUM);
		$privs = $priv_row[0];
		$db->free_result($result);
		$groups = array_merge($groups, explode(':', $privs));
	}
	
	if ($player_stats) {
		include_once(RYAPI_PATH.'server/player_stats.php');
		$row['fames'] = ryzom_player_fames_array($cid);
	}
	$row['groups'] = $groups;
	return $row;
}

?>
