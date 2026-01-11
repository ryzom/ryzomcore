<?php
// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

function retrieveTable($tableName, $key, $whereClause="", $selectClause="*")
{
	$result = sqlquery("SELECT $selectClause FROM $tableName".($whereClause=="" ? "" : " WHERE ".$whereClause));
	while ($result && ($arr = sqlfetch($result)))
		$dataArray[$arr[$key]] = $arr;
	return $dataArray;
}


// retrieve users info
$usersData = retrieveTable("user", "uid");
//$usersData = retrieveTable("user", "uid", "uid='$uid' OR uid='$gid'");
$userData = $usersData[$uid];
$userData['groupname'] = $usersData[$userData['gid']]['login'];

// retrieve variables info
$variableData = retrieveTable("variable", "vid");

// retrieve user variables
$uservariableData = retrieveTable("user_variable", "vid", "uid='$uid' OR uid='$gid'");

// retrieve shard list info
$shardList = retrieveTable("service", "shard", "", "DISTINCT shard");

// retrieve shard access info
$shardAccess = retrieveTable("shard_access", "shard", "uid='$uid' OR uid='$gid'", "DISTINCT shard");


function getUserVariableRights($uid, $gid)
{
	// get default variable state
	$result = sqlquery("SELECT vid, state FROM variable");
	while ($result && ($array = sqlfetch($result)))
	{
		$uservariablerights[$array["vid"]][0] = 1;
		$uservariablerights[$array["vid"]][1] = $array["state"];
	}
	
	// override from group settings
	$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='$gid'");
	while ($result && ($array = sqlfetch($result)))
	{
		$uservariablerights[$array["vid"]][0] = 2;
		$uservariablerights[$array["vid"]][2] = $array["privilege"];
	}
	
	// override from user settings
	$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='$uid'");
	while ($result && ($array = sqlfetch($result)))
	{
		$uservariablerights[$array["vid"]][0] = 3;
		$uservariablerights[$array["vid"]][3] = $array["privilege"];
	}

	return $uservariablerights;
}

$userUserVariableRights = getUserVariableRights($uid, $gid);

function hasAccessToVariable($vid)
{
	global	$userUserVariableRights;
	
	$var = &$userUserVariableRights[$vid];
	return isset($var) && $var[$var[0]] != "none";
}

function getVariableRight($vid)
{
	global	$userUserVariableRights;
	
	$var = &$userUserVariableRights[$vid];
	return $var[$var[0]];
}

function getShardLockState()
{
	global	$shardLockState, $uid, $REMOTE_ADDR, $enablelock, $shardList;
	global	$ASHost, $ASPort;
	
	$shardLockState = array();

	if (count($shardList) > 0)
	{
		foreach ($shardList as $shard => $s)
		{
			$shardLockState[$shard]['lock_state'] = ($enablelock ? 0 : 1);
		}
	}

	$result = sqlquery("SELECT * FROM shard_annotation");
	while ($result && ($arr=sqlfetch($result)))
	{
		if ($enablelock)
		{
			if ($arr['lock_user'] == 0)
			{
				$lockState = 0;	// unlocked
			}
			else if ($arr['lock_user'] == $uid && $arr['lock_ip'] == $REMOTE_ADDR)
			{
				$lockState = 1;	// locked by user
			}
			else
			{
				$lockState = 2;	// locked by another user
			}
		}
		else
		{
			$lockState = 1;
		}

		$shardLockState[$arr['shard']] = array(	'user_annot' 	=> $arr['user'],
												'annot'  		=> htmlentities($arr['annotation'], ENT_QUOTES),
												'post_date'  	=> $arr['post_date'],
												'lock_user'  	=> $arr['lock_user'],
												'lock_ip'  		=> $arr['lock_ip'],
												'lock_date'  	=> $arr['lock_date'],
												'lock_state'  	=> $lockState,
												'ASAddr'		=> $arr['ASAddr'],
												'alias'			=> $arr['alias']);
	}
}

getShardLockState();

?>