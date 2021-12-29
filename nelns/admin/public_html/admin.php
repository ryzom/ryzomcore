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

	global $HTTP_POST_VARS, $HTTP_GET_VARS;
	$publicAccess = false;
	include('authenticate.php');

	// -----------------------------
	// page commands
	
	unset($error);

	// remove user, ask for confirmation
	if (isset($rmUid) && isset($uuid) && $uuid!=$uid)
	{
		$result = sqlquery("SELECT login FROM user WHERE uid='$uuid'");
		if ($result && mysql_num_rows($result) == 1)
		{
			htmlProlog($_SERVER['PHP_SELF'], "Administration");
			$arr = mysql_fetch_array($result);
			echo "You are about to delete user ".$arr["login"]." ($uid)<br>\n";
			echo "Are you sure ?<br>\n";
			echo "<font size=+6><a href='".$_SERVER['PHP_SELF']."?confirmRmUid=$uuid'>YES</a> | <a href='".$_SERVER['PHP_SELF']."?editUsers=true'>NO</a>\n";
			htmlEpilog();
			die;
		}
	}
	// remove effectively user
	else if (isset($confirmRmUid) && $confirmRmUid!=$uid)
	{
		sqlquery("DELETE FROM user WHERE uid='$confirmRmUid'");
		$numUserDeleted = mysql_affected_rows();
		sqlquery("DELETE FROM user_variable WHERE uid='$confirmRmUid'");

		$result = sqlquery("SELECT tid FROM view_table WHERE uid='$confirmRmUid'");
		sqlquery("DELETE FROM view_table WHERE uid='$confirmRmUid'");

		while ($result && ($arr=mysql_fetch_array($result)))
		{
			sqlquery("DELETE FROM view_rows WHERE tid='".$arr["tid"]."'");
		}
		$editUsers = true;
	}
	// force user password
	else if (isset($forcePass))
	{
		sqlquery("UPDATE user SET password='".crypt($forcedPass, "NL")."' WHERE uid='$forcePass'");
	}
	// update user variables
	else if (isset($updVars) && isset($editUser))
	{
		foreach ($HTTP_POST_VARS as $var => $value)
		{
			if (strncmp($var, "avv_", 4) != 0)
				continue;
			
			$vid = (int)substr($var, 4);
			$ovar = "aovv_$vid";
			if (!isset($HTTP_POST_VARS[$ovar]))
				continue;

			$ovalue = $HTTP_POST_VARS[$ovar];
			if ($value == $ovalue)
				continue;

			sqlquery("DELETE FROM user_variable WHERE uid='$editUser' AND vid='$vid'");
			if ($value != "inv")
				sqlquery("INSERT INTO user_variable SET privilege='$value', uid='$editUser', vid='$vid'");

			/*
			// get all
			$result = sqlquery("SELECT uid FROM user WHERE gid='$editUser' OR uid='$editUser'");
			while ($result && ($arr=sqlfetch($result)))
			{
				sqlquery("DELETE FROM user_variable WHERE uid='".$arr["uid"]."' AND vid='$vid'");
				if ($value != "inv")
					sqlquery("INSERT INTO user_variable SET privilege='$value', uid='".$arr["uid"]."', vid='$vid'");
			}
			*/
		}
	}
	// create user
	else if (isset($createUid) && isset($nulogin) && isset($nupassword) && isset($nuconfirmpassword) && isset($nugroup) && isset($nuallowedIp))
	{
		if ($nupassword != $nuconfirmpassword)
		{
			$error = $error."Password is invalid (password confirmation failed)<br>\n";
		}
		else if (strspn($nulogin, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == 0)
		{
			$error = $error."Login '$admlogin' contains other characters than alphabetic and digits<br>\n";
		}
		else
		{
			$result = sqlquery("INSERT INTO user SET login='$nulogin', password='".crypt($nupassword, "NL")."', gid='$nugroup', allowed_ip='$nuallowedIp'");
			if (mysql_affected_rows() != 1)
			{
				$error .= "Can't create user '$nulogin', database request failed (already used login?)<br>\n";
				unset($nupassword);
				unset($nuconfirmpassword);
			}
			else
			{
				unset($nulogin);
				unset($nupassword);
				unset($nuconfirmpassword);
			}
		}
		$editUsers = true;
	}
	// reset user variables
	else if ($resetVars)
	{
		$result = sqlquery("DELETE FROM user_variable WHERE uid='$editUser'");
	}
	// import user var setup
/*	else if ($impVarSetup && isset($editUser) && isset($impUid))
	{
		$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='$impUid'");
		if ($result && mysql_num_rows($result)>0)
		{
			$delete_query = "DELETE FROM user_variable WHERE uid='$editUser' AND (";
			$copy_query = "INSERT INTO user_variable VALUES";
			$first = true;
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				$delete_query .= ($first ? "" : " OR ") . "vid='".$arr["vid"]."'";
				$copy_query .= ($first ? "" : ",") . " ('$editUser', '".$arr["vid"]."', '".$arr["privilege"]."')";
				$first = false;
			}
			$delete_query .= ")";
			// remove previous variables
			sqlquery($delete_query);
			// add new variables
			sqlquery($copy_query);
			//echo $delete_query."<br>".$copy_query;
			if (mysql_affected_rows == -1)
				$error .= "Import failed, database query failed<br>\n";
		}
	}*/
	// update user group
	else if (isset($updUid) && isset($chugroup))
	{
		sqlquery("UPDATE user SET gid='$chugroup' WHERE uid='$updUid'");
		$editUsers=true;
	}
	// update user cookie
	else if (isset($updUid) && isset($chucookie))
	{
		sqlquery("UPDATE user SET useCookie='$chucookie' WHERE uid='$updUid'");
		$editUsers=true;
	}
	// update user allowed ip
	else if (isset($allowIp) && isset($allowedIp))
	{
		sqlquery("UPDATE user SET allowed_ip='$allowedIp' WHERE uid='$allowIp'");
		$editUsers=true;
	}

	// remove variable
	else if (isset($rmVar) && isset($vid))
	{
		$editVariables = true;
		sqlquery("DELETE FROM variable WHERE vid='$vid'");
		if (mysql_affected_rows() != 1)
		{
			$error .= "Couldn't remove variable $vid/$chVarName, database request failed.<br>\n";
		}
		else
		{
			sqlquery("DELETE FROM user_variable WHERE vid='$vid'");
			sqlquery("DELETE FROM view_row WHERE vid='$vid'");
			$error .= "Removed effectively variable $vid/$chVarName/$chVarPath/$chVarState<br>\n";
		}
	}
	// create variable
	else if (isset($createVid) && isset($nvname) && isset($nvpath) && isset($nvstate) && isset($chVarGroup) && isset($nvgraphupdate))
	{
		$editVariables = true;
		$result = sqlquery("INSERT INTO variable SET name='$nvname', vgid='$chVarGroup', path='$nvpath', state='$nvstate', warning_bound='$nvwarning', error_bound='$nverror', alarm_order='$nvorder', graph_update='$nvgraphupdate', command=".(isset($nvvartype) ? "'variable'" : "'command'"));
		if (mysql_affected_rows() != 1)
		{
			$error .= "Can't create variable '$nvname', database request failed (already used variable name?)<br>\n";
			unset($nvpath);
			unset($nvstate);
		}
		else
		{
			$error .= "Effectively created variable '$nvname'<br>\n";
			$result = sqlquery("SELECT vid FROM variable WHERE name='$nvname' AND vgid='$chVarGroup' AND path='$nvpath' AND state='$nvstate'");
			if ($result && ($arr=sqlfetch($result)))
			{
				$vid = $arr["vid"];

				foreach ($HTTP_POST_VARS as $var => $value)
				{
					if (strncmp($var, "setgroup_", 9) != 0)
						continue;
					$id = (int)substr($var, 9);
					
					$priv = $value;
					if ($nvstate == 'rd' && $priv == 'rw')
						$priv = 'rd';
					if ($priv == '')
						continue;
						
					$error .= "Set right '$priv' to users of group '$id':";

					//$result = sqlquery("SELECT uid, login FROM user WHERE gid='$id'");
					$query = "INSERT INTO user_variable VALUES ('$id', '$vid', '$priv')";
					/*$first = true;
					while ($result && ($arr=sqlfetch($result)))
					{
						$query .= (!$first ? "," : "")." ('".$arr["uid"]."', '$vid', '$priv')";
						$error .= (!$first ? "," : "")." ".$arr["login"];
						$first = false;
					}*/
					sqlquery($query);
					$error .= "<br>\n";
				}
			}

			unset($nvname);
			unset($nvpath);
			unset($nvstate);
		}
	}
	// update variable
	else if (isset($chVar) && isset($vid) && isset($chVarName) && isset($chVarPath) && isset($chVarState) && isset($chVarGraphUpdate))
	{
		$editVariables = true;
		$result = sqlquery("UPDATE variable SET name='$chVarName', vgid='$chVarGroup', path='$chVarPath', state='$chVarState', warning_bound='$chVarWarning', error_bound='$chVarError', alarm_order='$chVarOrder', graph_update='$chVarGraphUpdate', command=".(isset($chVarType) ? "'variable'" : "'command'")." WHERE vid='$vid'");
		if (mysql_affected_rows() == -1)
		{
			$error .= "Can't update variable $vid properties, database query failed (name changed to already used?)<br>\n";
		}
		else if ($chVarState == "rd")
		{
			sqlquery("UPDATE user_variable SET privilege='rd' WHERE privilege='rw' AND vid='$vid'");
		}
	}
	// create var group
	else if (isset($createVarGroup))
	{
		sqlquery("INSERT INTO variable_group SET name='$createVarGroup'");
	}
	// remove var group
	else if (isset($rmVarGroup) && $rmVarGRoup!='1')
	{
		sqlquery("DELETE FROM variable_group WHERE vgid='$rmVarGroup'");
		sqlquery("UPDATE variable SET vgid='1' WHERE vgid='$rmVarGroup'");
	}
	// add shard access
	else if (isset($nshardaccess) && isset($editUser))
	{
		sqlquery("INSERT INTO shard_access SET uid='$editUser', shard='$nshardaccess'");
	}
	// remove shard access
	else if (isset($rmShardAccess) && isset($editUser))
	{
		sqlquery("DELETE FROM shard_access WHERE uid='$editUser' AND shard='$rmShardAccess'");
	}
	// update shard access
	else if (isset($chShardAccess) && isset($editUser))
	{
		sqlquery("DELETE FROM shard_access WHERE uid='$editUser'");
		$query = "INSERT INTO shard_access VALUES";
		$first = true;
		if (isset($shardAccesses))
		{
			foreach($shardAccesses as $shard)
			{
				if (!$first)
					$query .= ", ";
				$first = false;
				$query .= "('$editUser', '$shard')";
			}
			sqlquery($query);
		}
	}
	else if (isset($crViewCommand) && isset($nViewCommand) && isset($nViewCommandName) && isset($editTid))
	{
		sqlquery("INSERT INTO view_command SET name='$nViewCommandName', command='$nViewCommand', tid='$editTid'");
	}
	else if (isset($rmViewCommand) && isset($viewCommand) && isset($editTid))
	{
		sqlquery("DELETE FROM view_command WHERE name='$viewCommand' AND tid='$editTid'");
	}

	// create server
	else if (isset($createServer) && isset($serverName) && isset($serverIP))
	{
		sqlquery("INSERT INTO server SET name='$serverName', address='$serverIP'");
	}
	// delete server
	else if (isset($rmServer) && isset($serverName))
	{
		sqlquery("DELETE FROM server WHERE name='$serverName'");
	}
	// update server name
	else if (isset($updServerName) && isset($newServerName))
	{
		sqlquery("UPDATE server SET name='$newServerName' WHERE name='$updServerName'");
		sqlquery("UPDATE service SET server='$newServerName' WHERE server='$updServerName'");
	}
	// update server ip
	else if (isset($updServerIP) && isset($newServerIP))
	{
		sqlquery("UPDATE server SET address='$newServerIP' WHERE name='$updServerIP'");
	}
	
	// create service
	else if (isset($createService) && isset($shardName) && isset($serverName) && isset($serviceName))
	{
		sqlquery("INSERT INTO service SET shard='$shardName', server='$serverName', name='$serviceName'");
	}
	// delete service
	else if (isset($rmService) && isset($serviceId))
	{
		sqlquery("DELETE FROM service WHERE service_id='$serviceId'");
	}
	// update shard name
	else if (isset($newShardName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET shard='$newShardName' WHERE service_id='$serviceId'");
	}
	// update server name
	else if (isset($newServerName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET server='$newServerName' WHERE service_id='$serviceId'");
	}
	// update service name
	else if (isset($newServiceName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET name='$newServiceName' WHERE service_id='$serviceId'");
	}
	
	else if ($editServices == 'update' && isset($updateList))
	{
		unset($services);
		$services = explode("\r", $updateList);

		$editServiceError = '';
		$editServiceLog = '';
		$insertList = array();
		$success = true;
		
		$lineCount;

		foreach ($services as $line)
		{
			++$lineCount;
			$l = trim($line);
			
			if ($l == '' || $l[0] == '*')
				continue;

			if (!ereg("^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]*$", $l, $regs))
			{
				$editServiceError = "Malformed string '$l' at line $lineCount";
				$success = false;
				break;
			}

			list($reg, $shard, $server, $service) = $regs;
			
			$insertList[] = array( 'shard' => $shard, 'server' => $server, 'service' => $service);
		}

		if ($success)
		{
			$updateLog = '';

			$updateCount = 0;
			if (count($insertList) > 0)
			{
				foreach ($insertList as $l)
				{
					$query = "SELECT * FROM service WHERE shard='".$l['shard']."' AND server='".$l['server']."' AND name='".$l['service']."'";
					$result = sqlquery($query);
					if ($result && sqlnumrows($result) == 0)
					{
						$updateLog .= '<li>updated/inserted service '.$l['shard'].'.'.$l['server'].'.'.$l['service']."</li>\n";
						++$updateCount;
					}
				}
			}

			$query = 'DELETE FROM service';
			//echo $query."<br>\n";
			$result = sqlquery($query);
			if ($result)
			{
				$insertSuccess = 0;
	
				if (count($insertList) > 0)
				{
					foreach ($insertList as $l)
					{
						$query = "INSERT INTO service SET shard='".$l['shard']."', server='".$l['server']."', name='".$l['service']."'";
						//echo $query."<br>\n";
						sqlquery($query);
						++$insertSuccess;
					}
				}
				
				$editServiceLog .= "Successfully updated $insertSuccess services in database:\n<ul>\n".$updateLog."</ul>";
				$editServiceError = '';
			}
			else
			{
				$editServiceError = 'Failed to delete all services from database';
			}
		}
	}

	else if ($editServers == 'update' && isset($updateList))
	{
		unset($servers);
		$servers = explode("\r", $updateList);

		$editServerError = '';
		$editServerLog = '';
		$insertList = array();
		$success = true;
		
		$lineCount;

		foreach ($servers as $line)
		{
			++$lineCount;
			$l = trim($line);
			
			if ($l == '' || $l[0] == '*')
				continue;

			if (!ereg("^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]*$", $l, $regs))
			{
				$editServiceError = "Malformed string '$l' at line $lineCount";
				$success = false;
				break;
			}

			list($reg, $server, $ip) = $regs;
			
			$insertList[] = array( 'server' => $server, 'ip' => $ip);
		}

		if ($success)
		{
			$updateLog = '';

			$updateCount = 0;
			if (count($insertList) > 0)
			{
				foreach ($insertList as $l)
				{
					$query = "SELECT * FROM server WHERE name='".$l['server']."' AND address='".$l['ip']."'";
					$result = sqlquery($query);
					if ($result && sqlnumrows($result) == 0)
					{
						$updateLog .= '<li>updated/inserted server '.$l['server'].' at '.$l['ip']."</li>\n";
						++$updateCount;
					}
				}
			}

			$query = 'DELETE FROM server';
			//echo $query."<br>\n"; $result=true;
			$result = sqlquery($query);
			if ($result)
			{
				$insertSuccess = 0;
	
				if (count($insertList) > 0)
				{
					foreach ($insertList as $l)
					{
						$query = "INSERT INTO server SET name='".$l['server']."', address='".$l['ip']."'";
						//echo $query."<br>\n";
						sqlquery($query);
						++$insertSuccess;
					}
				}
				
				$editServerLog .= "Successfully updated $insertSuccess servers in database:\n<ul>\n".$updateLog."</ul>";
				$editServerError = '';
			}
			else
			{
				$editServerError = 'Failed to delete all servers from database';
			}
		}
	}

	// -----------------------------
	// page display

	htmlProlog($_SERVER['PHP_SELF'], "Administration");
	subBar( array( 	"Users" => $_SERVER['PHP_SELF']."?editUsers=true",
					"Variables" => $_SERVER['PHP_SELF']."?editVariables=true",
					"Services" => $_SERVER['PHP_SELF']."?editServices=true",
					"Servers" => $_SERVER['PHP_SELF']."?editServers=true",
					"Shards" => $_SERVER['PHP_SELF']."?editShards=true" ));

	echo "Administration tools<br>\n";
	
	if ($error)
		echo "<b>Reported errors:</b><br>\n$error<br>";
		
	if (!$editUser && !$editUsers && !$editVariables && !$editShards && !$editServices && !$editServers)
		$editUsers = true;

	// ---------------------------------------------------------------------------------
	// edit a single user
	// ---------------------------------------------------------------------------------
	if ($editUser)
	{
		$resURL = $_SERVER['PHP_SELF']."?editUser=$editUser&selGroup=$selGroup";

		$result = sqlquery("SELECT * FROM user WHERE uid='$editUser'");
		if ($result && ($arr=mysql_fetch_array($result)))
		{
			$editLogin = $arr["login"];
			$defaultView = $arr["default_view"];
			$userGroup = $arr["gid"];

			echo "<b>$editLogin variables/views setup edit</b><br><br>\n";

			echo "<b>User variables setup</b><br>";
			echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";

			// User variables state display/modify
			$editUsers = false;
			
			$vars = false;
			
			unset($vars);
			unset($groups);

			$result = sqlquery("SELECT * FROM variable_group ORDER BY name");
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				if ((!isset($selGroup) || $selGroup == "") && $arr["name"] == "NoGroup")
					$selGroup = $arr["vgid"];
				$groups[$arr["vgid"]] = $arr["name"];
			}

			$result = sqlquery("SELECT vid, variable.name AS name, path, state, variable.vgid AS vgid, variable_group.name AS group_name FROM variable, variable_group WHERE variable.vgid=variable_group.vgid".($selGroup>0 ? " AND variable.vgid='$selGroup'" : "")." ORDER BY group_name, name");
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				$arr["priv"] = "inv";
				$vars[] = $arr;
			}
			
			$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='$editUser'");
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				for ($i=0; $i<count($vars) && $vars[$i]["vid"] != $arr["vid"]; ++$i)
					;
				if ($i<count($vars))
					$vars[$i]["priv"] = $arr["privilege"];
			}
			
			$usrVarRights = getUserVariableRights($editUser, $userGroup);

			echo "<table border=1>\n";
			echo "<tr><th>Variable</th><form method=post action='".$_SERVER['PHP_SELF']."?editUser=$editUser'><th>";
			echo "<select name='selGroup' onChange='submit()'>";
			$found = false;
			foreach ($groups as $vgid => $group )
				echo "<option value='$vgid'".($selGroup == $vgid ? " selected" : "").">$group";
			echo "<option value='-1'".($selGroup == "-1" ? " selected" : "").">All groups";
			echo "</select>";
			echo "</th></form><th>Path</th><th>Rights</th></tr>";
			echo "<form method=post action='$resURL'>\n";
			if (count($vars)>0)
			{
				foreach ($vars as $var => $state)
				{
					$vid = $state["vid"];

					$usrVar = $usrVarRights[$vid];
					$vinherit = $usrVar[0];
					$vstate = $usrVar[$vinherit];

					echo "<tr><td>".$state["name"]."</td>".
								 "<td>".$state["group_name"]."</td>".
								 "<td>".$state["path"]."</td>".
								 "<td><select name='avv_$vid'>";

					if ($vinherit != 3)
					{
						echo "<option value='inv' selected>Inherit ".($vstate == "none" ? "unavailable" : ($vstate == "rd" ? "read only" : "read write")).($vinherit == 1 ? " (from variable)" : " (from group)");
					 	echo "<option value='none'>Override unavailable";
					 	echo "<option value='rd'>Override read only";
					 	if ($state["state"] == "rw")
						 	echo "<option value='rw'>Override read write";
					}
					else
					{
						if ($editUser==$userGroup)
							echo "<option value='inv'>Inherit ".($usrVar[1] == "rd" ? "read only" : "read write")." (from variable)";
						else
							echo "<option value='inv'>Inherit ".(!isset($usrVar[2]) ? ($usrVar[1] == "rd" ? "read only" : "read write")." (from variable)" : ($usrVar[2] == "none" ? "unavailable" : ($usrVar[2] == "rd" ? "read only" : "read write"))." (from group)" );
					 	echo "<option value='none'".($vstate == "none" ? " selected" : "").">Override unavailable";
					 	echo "<option value='rd'".($vstate == "rd" ? " selected" : "").">Override read only";
					 	if ($state["state"] == "rw")
						 	echo "<option value='rw'".($vstate == "rw" ? " selected" : "").">Override read write";
					}
					 echo "</select><input type=hidden name='aovv_$vid' value='".$state["priv"]."'></td></tr>\n";

/*
								 	"<option value='inv'".($vstate == "none" ? " selected" : "").">Unavailable".
								 	"<option value='none'".($state["priv"] == "none" ? " selected" : "").">Unavailable".
								 	"<option value='rd'".($state["priv"] == "rd" ? " selected" : "").">Read only".
								 	($state["state"] == "rw" ? "<option value='rw'".($state["priv"] == "rw" ? " selected" : "").">Read Write" : "").
								 "</select><input type=hidden name='aovv_$vid' value='".$state["priv"]."'></td></tr>\n";
*/
				}
			}
/*
			if (count($vars)>0)
			{
				foreach ($vars as $var => $state)
				{
					$vid = $state["vid"];
					echo "<tr><td>".$state["name"]."</td>".
								 "<td>".$state["group_name"]."</td>".
								 "<td>".$state["path"]."</td>".
								 "<td><select name='avv_$vid'>".
								 	"<option value='inv'".($state["priv"] == "inv" ? " selected" : "").">Invisible".
								 	"<option value='none'".($state["priv"] == "none" ? " selected" : "").">Unavailable".
								 	"<option value='rd'".($state["priv"] == "rd" ? " selected" : "").">Read only".
								 	($state["state"] == "rw" ? "<option value='rw'".($state["priv"] == "rw" ? " selected" : "").">Read Write" : "").
								 "</select><input type=hidden name='aovv_$vid' value='".$state["priv"]."'></td></tr>\n";
				}
			}
*/
			echo "<tr height=5><td colspan=4 align=center></td></tr>\n";
			echo "<tr><td colspan=4 align=center><input type=submit name='updVars' value='Update'> <input type=submit name='resetVars' value='Reset all'></td></tr>\n";
			echo "</form></table><br>\n";
			
			echo "</td><td width=30>\n</td><td align=left>\n";

			if ($editUser != $uid)
			{
				echo "<table border=1>\n";
				echo "<tr><th>Shard accesses</th></tr>\n";
				// get user accesses
				$res = sqlquery("SELECT shard FROM shard_access WHERE uid='$editUser'");
				unset($shards);
				while ($res && ($arr=sqlfetch($res)))
					$shards[$arr["shard"]] = true;
				// get all shards
				$result = sqlquery("SELECT DISTINCT shard FROM service");
				echo "<tr><form method=post action='$resURL&chShardAccess=true'><td align=center>";
				echo "<select multiple size=".(sqlnumrows($result))." name='shardAccesses[]'>";
				// display all shards and select if in user list
				while ($result && ($arr = sqlfetch($result)))
				{
					$shard = $arr["shard"];
					echo "<option value='$shard'".($shards[$shard] ? " selected" : "").">$shard&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				}
				echo "</select></td></tr>";
				echo "<tr><td align=center><input type=submit value='Update'></td></form></tr>\n";
				echo "</table><br>\n";
			}

			echo "</td><td width=30>\n</td><td align=center>\n";

			echo "<table cellpadding=0 cellspacing>\n";
			// Variable setup import form
			echo "<tr><form method=post action='$resURL'><input type=hidden name=editUser value='$editUser'>\n";
			echo "<td align=right><b>Import</b> variables setup from user&nbsp;</td>\n";
			echo "<td><select name=impUid>\n";
			$result = sqlquery("SELECT login, uid FROM user WHERE uid!='$editUser' ORDER BY login");
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				echo "<option value='".$arr["uid"]."'>".$arr["login"]."\n";
			}
			echo "</select></td>\n";
			echo "<td><input type=submit name=impVarSetup value='Import'></td>\n";
			echo "</tr></form>\n";
			
			echo "<tr><td colspan=3><hr></td></tr>\n";

			echo"</table>\n";

			echo "</td></tr></table>\n";
			
			// user views setup
			echo "<b>User views setup</b>\n";
			echo "<table><tr valign=top><td>\n";

			echo "<table border=1 cellpadding=2><tr><th>Views</th></tr>\n";
			$result = sqlquery("SELECT name, tid FROM view_table WHERE uid='$editUser' ORDER BY ordering");
			while ($result && ($arr=sqlfetch($result)))
			{
				$disp = "<a href='$resURL&editTid=".$arr["tid"]."'>".$arr["name"]."</a>";
				$bgcolor = ($editTid == $arr["tid"] ? " bgcolor=#eeeeee" : "");
				if ($arr["tid"] == $defaultView)
					echo "<tr><td$bgcolor>&nbsp;<b>$disp</b>&nbsp;</td></tr>\n";
				else
					echo "<tr><td$bgcolor>&nbsp;$disp&nbsp;</td></tr>\n";
			}
			echo "</table>\n";

			echo "</td><td width=30>\n";

			echo "</td><td>\n";
			
			if (isset($editTid) && $editTid != "")
			{
				echo "<b>Variables</b><br>\n";
				echo "<table border=1 cellpadding=2><tr><th>User name</th><th>System name</th><th>Path</th><th>User filter</th></tr>\n";
				$result = sqlquery("SELECT view_row.name AS name, variable.name AS sname, path, filter FROM view_row, variable WHERE tid='$editTid' AND view_row.vid=variable.vid ORDER BY ordering");
				while ($result && ($arr=sqlfetch($result)))
				{
					echo "<tr><td>".$arr["name"]."</td><td>".$arr["sname"]."</td><td>".$arr["path"]."</td><td>".$arr["filter"]."</td></tr>\n";
				}
				echo "</table><br>\n";
				
				echo "<b>Commands</b><br>\n";
				echo "<table border=1 cellpadding=2><tr><th>Name</th><th colspan=2>Service command</th></tr>\n";
				$result = sqlquery("SELECT name, command FROM view_command WHERE tid='$editTid' ORDER BY name");
				while ($result && ($arr=sqlfetch($result)))
				{
					echo "<tr><form method=post action='$resURL&editTid=$editTid'><td>".$arr["name"]."</td><td>".$arr["command"]."</td><td><input type=hidden name=viewCommand value='".$arr["name"]."'><input type=submit name='rmViewCommand' value='Delete'></td></form></tr>\n";
				}
				echo "<tr><td colspan=3 height=5></td></tr>\n";
				echo "<tr><form method=post action='$resURL&editTid=$editTid'><td><input name=nViewCommandName size=16 maxlength=32></td><td><input name=nViewCommand size=16 maxlength=32></td><td><input type=submit name='crViewCommand' value='Create'></td></form></tr>\n";
				echo "</table><br>\n";
			}

			echo "</td></tr></table><br>\n";

			// user activity display
			echo "<b>User activity</b><br>\n";
			echo "<textarea rows=15 cols=200 readOnly style='font-family: Terminal, Courier; font-size: 10pt;' nowrap>";
			//$editLogin
			$logfilename = $userlogpath."/".$editLogin.".log";
			unset($resExec);
			exec("tail -n 40 $logfilename", $resExec);
			
			echo join("\n", $resExec);
			
			echo "</textarea><br>\n";
		}
		else
		{
			echo "User $editUser not found in database.<br><br>";
		}
	}

	// ---------------------------------------------------------------------------------
	// edit users
	// ---------------------------------------------------------------------------------
	if ($editUsers)
	{
		echo "<b>Users setup</b><br><br>\n";
		
		unset($groupNames);
		unset($actualGroups);
		$result = sqlquery("SELECT login AS gname, uid, gid FROM user");
		while ($result && ($arr=mysql_fetch_array($result)))
		{
			$groupNames[$arr["gname"]] = $arr["uid"];
			if ($arr["gid"] == $arr["uid"])
				$actualGroups[$arr["gname"]] = $arr["uid"];
		}

		echo "<table border=1>\n";
		echo "<tr><th>Login</th><th>Uid</th>";
		echo "<form method=post action='".$_SERVER['PHP_SELF']."'><th><select name=uViewGroups onChange='submit()'>";
		echo  "<option value=''".((!isset($uViewGroups) || $uViewGroups=='') ? " selected" : "").">All Groups\n";
		foreach($actualGroups as $cgname => $cgid)
			echo  "<option value='$cgid'".($cgid==$uViewGroups ? " selected" : "").">$cgname\n";
		echo "</select></th></form>";
		echo "<th>Cookie</th><th>Force password</th><th>Allowed IP mask</th><th>Commands</th></tr>\n";

		if (!isset($uViewGroups) ||  $uViewGroups == '')
			$query = "SELECT uuser.login AS login, uuser.uid AS uid, uuser.useCookie AS useCookie, uuser.gid AS gid, ugroup.login AS gname, uuser.allowed_ip AS allowed_ip FROM user AS uuser, user AS ugroup WHERE uuser.gid=ugroup.uid ORDER BY uid";
		else
			$query = "SELECT uuser.login AS login, uuser.uid AS uid, uuser.useCookie AS useCookie, uuser.gid AS gid, ugroup.login AS gname, uuser.allowed_ip AS allowed_ip FROM user AS uuser, user AS ugroup WHERE uuser.gid=ugroup.uid AND uuser.gid='$uViewGroups' ORDER BY uid";
		$result = sqlquery($query);
		while ($result && ($arr=mysql_fetch_array($result)))
		{
			$ulogin = $arr["login"];
			$uuid = $arr["uid"];
			$ugid = $arr["gid"];
			$ugname = $arr["gname"];
			$uallowedip = $arr["allowed_ip"];
			$uuseCookie = ($arr["useCookie"] == "yes");
			echo "<tr>".
						"<td><a href='".$_SERVER['PHP_SELF']."?editUser=$uuid'>$ulogin</a></td>\n".
						"<td>$uuid</td>\n".
						"<form method=post action='".$_SERVER['PHP_SELF']."'>".
						"<input type=hidden name=updUid value='$uuid'><td><select name=chugroup onChange='submit()'>\n";
			foreach($groupNames as $cgname => $cgid)
				echo  "<option value='$cgid'".($cgid==$ugid ? " selected" : "").">$cgname\n";
			echo     "</select></td></form>\n";
			echo 		"<form method=post action='".$_SERVER['PHP_SELF']."'><input type=hidden name=updUid value='$uuid'><td><select name='chucookie' onChange='submit()'><option value='yes'".($uuseCookie ? " selected" : "").">Yes<option value='no'".($uuseCookie ? "" : " selected").">No</select></td></form>";
			echo 		"<form method=post action='".$_SERVER['PHP_SELF']."'><input type=hidden name=forcePass value='$uuid'><td><input type=password name='forcedPass'></td></form>";
			echo 		"<form method=post action='".$_SERVER['PHP_SELF']."'><input type=hidden name=allowIp value='$uuid'><td><input name='allowedIp' value='$uallowedip'></td></form>";
			echo		"<td><a href='".$_SERVER['PHP_SELF']."?confirmRmUid=$uuid' onClick=\"return confirm('You are about to delete user $ulogin')\">Delete</a></td></tr>\n";
		}
		echo "</table><br>\n";
		
		$result = sqlquery("SELECT login, uid FROM user");

		echo "<table border=1><form method=post action='".$_SERVER['PHP_SELF']."'>\n";
		echo "<tr><th colspan=2>Create a new user</th></tr>\n";
		echo "<tr><td>Login</td><td><input name=nulogin maxlength=16 size=16 value='$nulogin'></td></tr>\n";
		echo "<tr><td>Group</td><td><select name=nugroup>\n";
		while ($result && ($arr=mysql_fetch_array($result)))
			echo "<option value='".$arr["uid"]."'>".$arr["login"]."\n";
		echo "<tr><td>Allowed IP mask</td><td><input name=nuallowedIp maxlength=32 size=16 value='$nuallowedIp'></td></tr>\n";
		echo "<tr><td>Password</td><td><input type=password name=nupassword maxlength=16 size=16></td></tr>\n";
		echo "<tr><td>Renter password</td><td><input type=password name=nuconfirmpassword maxlength=16 size=16></td></tr>\n";
		echo "<tr><td colspan=2 align=center><input type=submit name=createUid value='Create'></td></tr>\n";
		echo "</form></table>\n";
	}
	
	// ---------------------------------------------------------------------------------
	// edit variables
	// ---------------------------------------------------------------------------------
	if ($editVariables)
	{
		echo "<b>Variables setup</b>".help("Variables")."<br><br>\n";
		
		echo "<table cellpadding=0 cellspacing=0><tr valign=0><td>\n";

		if ($importVarSetup && $importedVarSetup)
		{
			$result = sqlquery("SELECT name, vgid FROM variable_group");
			unset($groups);
			while ($result && ($arr=mysql_fetch_array($result)))
			{
				if ((!isset($varGroup) || $varGroup=="") && $arr["name"] == "NoGroup")
					$varGroup = $arr["vgid"];
				$groups[$arr["vgid"]] = $arr["name"];
			}
		
			$array = explode("\n", $importedVarSetup);

			if (count($array) > 0)
			{
				unset($groupnames);
				foreach ($groups as $vgid => $vgname)
					$groupnames[$vgname] = $vgid;

				foreach ($array as $varSetup)
				{
					if ($varSetup == '')
						continue;
					list($vname, $vpath, $vstate, $vgname, $vwarn, $verr, $valarm, $vgraph, $vcmd) = explode("|", $varSetup);
					
					$result = sqlquery("SELECT count(*) as count FROM variable WHERE name='$vname'");
					if ($result && ($arr=sqlfetch($result)) && $arr["count"] == 0)
					{
						if (!isset($groupnames[$vgname]))
						{
							sqlquery("INSERT INTO variable_group SET name='$vgname'");
							$result = sqlquery("SELECT vgid FROM variable_group WHERE name='$vgname'");
							if ($result && ($arr=sqlfetch($result)))
							{
								$vgid = $arr["vgid"];
								$groupnames[$vgname] = $vgid;
							}
							else
							{
								$vgid = -1;
							}
						}
						else
							$vgid = $groupnames[$vgname];
							
						if ($vgid != -1)
						{
							sqlquery("INSERT INTO variable SET name='$vname', path='$vpath', state='$vstate', vgid='$vgid', warning_bound='$vwarn', error_bound='$verr', alarm_order='$valarm', graph_update='$vgraph', command='$vcmd'");
						}
					}
				}
			}
		}

		$result = sqlquery("SELECT name, vgid FROM variable_group");
		unset($groups);
		while ($result && ($arr=mysql_fetch_array($result)))
		{
			if ((!isset($varGroup) || $varGroup=="") && $arr["name"] == "NoGroup") {
				print "ERG! VARGROUP GUNNA BE: ".$arr["vgid"];
				$varGroup = $arr["vgid"];
			}
			$groups[$arr["vgid"]] = $arr["name"];
		}
	
		if ($varGroup=="-1")
			$result = sqlquery("SELECT * FROM variable ORDER BY vgid, name");
		else
			$result = sqlquery("SELECT * FROM variable WHERE vgid='$varGroup' ORDER BY name");
		echo "<table border=1>\n";
		echo "<tr><th>Name</th><th>Vid</th><th>Group</th><th>Path</th><th>State</th><th>Warning</th><th>Error</th><th>Order</th><th>Graph</th><th>Variable</th><th colspan=2>Commands</th></tr>\n";

		$lastGroup = -1;

		while ($result && ($arr=mysql_fetch_array($result)))
		{
			$name = $arr["name"];
			$vid = $arr["vid"];
			$vgid = $arr["vgid"];
			$path = $arr["path"];
			$state = $arr["state"];
			$warn_bound = $arr["warning_bound"];
			$err_bound = $arr["error_bound"];
			$alarm_order = $arr["alarm_order"];
			$graph_update = $arr["graph_update"];
			$var_type = $arr["command"];
			
			if ($lastGroup != -1 && $lastGroup != $vgid)
				echo "<tr height=5><td colspan=12></td></tr>\n";
			$lastGroup = $vgid;

			echo "<tr><form method=post action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup'><input type=hidden name=vid value='$vid'><input type=hidden name=chVar value='Update'>".
						"<td><input name=chVarName maxlength=128 size=16 value='$name'></td>\n".
						"<td>$vid</td>\n";
			echo		"<td><select name=chVarGroup onChange='submit()'>";
			foreach ($groups as $chvgid => $chvgname)
				echo	"<option value='$chvgid'".($chvgid==$vgid ? " selected":"").">$chvgname";
			echo		"</select></td>\n".
						"<td><input name=chVarPath maxlength=255 size=32 value='$path'></td>\n".
						"<td><select name=chVarState onChange='submit()'><option value='rd'".($state=="rd" ? " selected":"").">Read only<option value='rw'".($state=="rw" ? " selected":"").">Read write</select></td>".
						"<td><input name=chVarWarning maxlength=11 size=11 value='$warn_bound'></td>".
						"<td><input name=chVarError maxlength=11 size=11 value='$err_bound'></td>".
						"<td><select name=chVarOrder onChange='submit()'><option value='gt'".($alarm_order=="gt" ? " selected":"").">gt<option value='lt'".($alarm_order=="lt" ? " selected":"").">lt</select></td>".
						"<td><input name=chVarGraphUpdate maxlength=8 size=4 value='$graph_update'></td>".
						"<td align=center><input type=checkbox name=chVarType".($var_type == "variable" ? " checked" : "")." value='1'></td>".
						"<td><input type=submit name=chVar value='Update'></td></form><form method=post action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup'><td><input type=hidden name='vid' value='$vid'><input type=submit name=rmVar value='Delete' onClick=\"return confirm('You are about to delete a Variable')\"></td>".
					"</form></tr>\n";
		}
		echo "<tr height=10><td colspan=12></td></tr>\n";
		if (!isset($nvpath))		$nvpath = "*.*.*.*[.*]";
		if (!isset($nvstate))	$nvstate = "rd";
		echo "<tr valign=top><form method=post action='".$_SERVER['PHP_SELF']."?editVariables=true&varGroup=$varGroup'><input type=hidden name=vid value='$vid'>".
					"<td><input name=nvname maxlength=128 size=16 value='$nvname'></td>\n".
					"<td></td>\n";
		echo		"<td><select name=chVarGroup>";
		foreach ($groups as $chvgid => $chvgname)
			echo	"<option value='$chvgid'".($chvgid==$varGroup ? " selected":"").">$chvgname";
		echo		"</select></td>\n".
					"<td><input name=nvpath maxlength=255 size=32 value='$nvpath'></td>\n".
					"<td><select name=nvstate><option value='rd'".($nvstate=="rd" ? " selected":"").">Read only<option value='rw'".($nvstate=="rw" ? " selected":"").">Read write</select></td>".
					"<td><input name=nvwarning maxlength=11 size=11 value='-1'></td>".
					"<td><input name=nverror maxlength=11 size=11 value='-1'></td>".
					"<td><select name=nvorder><option value='gt'".($state=="gt" ? " selected":"").">gt<option value='lt'".($state=="lt" ? " selected":"").">lt</select></td>".
					"<td><input name=nvgraphupdate maxlength=8 size=4 value='0'></td>".
					"<td align=center><input type=checkbox name=nvvartype checked value='1'></td>".
					"<td rowspan=2 colspan=2 align=center><input type=submit name=createVid value='Create'> ".help("Create Variable")."</td>".
				"</tr>\n";
		echo "<tr><td></td><td colspan=9 align=center>\n";

		echo "<table><tr><th></th><th width=80>Read Write</th><th width=80>Read only</th><th width=80>Invisible</th></tr>\n";
		$result = sqlquery("SELECT uid, login FROM user WHERE uid=gid ORDER BY uid");
		while ($result && ($arr=sqlfetch($result)))
		{
			echo "<tr><td>".$arr["login"]."</td>";
			echo "<td align=center><input type=radio value='rw' name='setgroup_".$arr["uid"]."' checked></td>";
			echo "<td align=center><input type=radio value='rd' name='setgroup_".$arr["uid"]."'></td>";
			echo "<td align=center><input type=radio value='' name='setgroup_".$arr["uid"]."'></td>";
			echo "</tr>\n";
		}
		echo "</form></table>\n";

		echo "</td><td></td></tr>\n";
		echo "</table><br>\n";
		
		echo "</td><td width=30>&nbsp;</td><td>\n";
		
		echo "<table>\n";
		echo "<tr><td align=center colspan=2>View by variable group</td></tr><tr><form method=post action='".$_SERVER['PHP_SELF']."?editVariables=true'>\n";
		echo "<td align=center colspan=2><select name=varGroup onChange='submit()'>\n";
		echo "<option value='-1'".($varGroup=="-1" ? " selected":"").">All groups";
		foreach ($groups as $vgid => $vgname)
			echo "<option value='$vgid'".($vgid==$varGroup ? " selected":"").">$vgname";
		echo "</select></td></form></tr>";
		
		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr>\n";
		echo "<td align=center colspan=2>Create a variable group</td></tr><tr><form method=post action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=right><input name=createVarGroup size=16 maxlength=32></td><td><input type=submit value='Create'></td></form>\n";
		echo "</tr>\n";

		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr><td align=center colspan=2>Delete a variable group</td></tr><tr><form method=post action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=right><select name=rmVarGroup>\n";
		foreach ($groups as $vgid => $vgname)
			if ($vgid!=1)
				echo "<option value='$vgid'".($vgid==$varGroup ? " selected":"").">$vgname";
		echo "</select></td><td><input type=submit value='Delete'>\n";
		echo "</td></form></tr>";

		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr>\n";
		echo "<td align=center colspan=2>Export variables setup</td></tr><tr><form method=post action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=center colspan=2><input type=submit name='exportVarSetup' value='Export'></td></form>\n";
		echo "</tr>\n";

		echo "</table>\n";


		echo "</td></tr></table>\n";
		
		echo "<form method='post' action='".$_SERVER['PHP_SELF']."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<b>Import/Exported setup</b> (use this to export to another admin tool):<br>\n";
		echo "<textarea rows=30 cols=160 name='importedVarSetup'>";
		if ($exportVarSetup)
		{
			$result = sqlquery("SELECT * FROM variable ORDER BY vgid, name");
			while ($result && ($arr=sqlfetch($result)))
				echo $arr["name"]."|".$arr["path"]."|".$arr["state"]."|".$groups[$arr["vgid"]]."|".$arr["warning_bound"]."|".$arr["error_bound"]."|".$arr["alarm_order"]."|".$arr["graph_update"]."|".$arr["command"]."\n";
		}
		echo "</textarea><br>\n";
		echo "<input type=submit name='importVarSetup' value='Import' onClick=\"return confirm('You are about to import setup')\">\n";
		echo "</form>\n";
	}

	// ---------------------------------------------------------------------------------
	// edit shard organization
	// ---------------------------------------------------------------------------------
	if ($editShards)
	{
		echo "<b>Shards setup</b>".help("Shards")."<br><br>\n";

		echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";
		
		if (!isset($serverOrder))
			$serverOrder = "name";

		if (!isset($serviceOrder))
			$serviceOrder = "shard, server, name";
			
		unset($servers);

		$result = sqlquery("SELECT * FROM server ORDER BY $serverOrder");
		echo "<table border=1><tr><th>Name</th><th>Address</th><th>Command</th></tr>\n";
		while ($result && ($arr=sqlfetch($result)))
		{
			echo "<tr><form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input name=newServerName value=\"".$arr["name"]."\" size=16 maxlength=32><input type=hidden name=updServerName value=\"".$arr["name"]."\"></td></form>";
			echo "<form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input name=newServerIP value=\"".$arr["address"]."\" size=16 maxlength=32><input type=hidden name=updServerIP value=\"".$arr["name"]."\"></td></form>";
			echo "<form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input type=submit name=rmServer value=\"Delete\"><input type=hidden name=serverName value=\"".$arr["name"]."\"></td></form></tr>\n";
			$servers[] = $arr["name"];
		}
		echo "<tr><form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input name=serverName size=16 maxlength=32></td><td><input name=serverIP size=16 maxlength=32></td><td><input type=submit name=createServer value=\"Create\"></td></form></tr>\n";
		echo "</table>\n";

		echo "</td><td width=20>&nbsp;\n";
		echo "</td><td>\n";

		if ($fshard == "")
			unset($result);
		else if ($fshard == "*")
			$result = sqlquery("SELECT * FROM service ORDER BY $serviceOrder");
		else
			$result = sqlquery("SELECT * FROM service WHERE shard LIKE '%$fshard%' ORDER BY $serviceOrder");

		echo "<table border=1><tr><form method=post action='".$_SERVER['PHP_SELF']."?editShards=true'><th>Shard ";
		echo "<select name=fshard onChange='submit()'>";
		echo "<option value=''".($fshard=="" ? " selected" : "").">No shard";
		echo "<option value='*'".($fshard=="*" ? " selected" : "").">All shards";
		$res = sqlquery("SELECT DISTINCT shard FROM service");
		while ($res && ($arr=sqlfetch($res)))
			echo "<option value='".$arr["shard"]."'".($fshard==$arr["shard"] ? " selected" : "").">".$arr["shard"];
		echo "</select>";
		echo "</th></form><th>Server</th><th>Service</th><th>Command</th></tr>\n";
		while ($result && ($arr=sqlfetch($result)))
		{
			echo "<tr><form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input name=newShardName value='".$arr["shard"]."' size=24 maxlength=32><input type=hidden name=serviceId value='".$arr["service_id"]."'></td></form>\n";
			echo "<form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><input type=hidden name=serviceId value='".$arr["service_id"]."'><td>";
			echo "<select name=newServerName onChange='submit()'>";
			$foundServer = false;
			foreach ($servers as $server)
			{
				echo "<option value='$server'";
				if ($server == $arr["server"])
				{
					echo " selected";
					$foundServer = true;
				}
				echo ">$server";
			}
			if (!$foundServer)
				echo "<option value='".$arr["server"]."' selected>".$arr["server"];
			echo "</select>";
			echo "</td></form><form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post><td><input name=newServiceName value='".$arr["name"]."' size=16 maxlength=32><input type=hidden name=serviceId value='".$arr["service_id"]."'></td></form>";
			echo "<form method=post action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard'><td><input type=submit name=rmService value='Delete'><input type=hidden name=serviceId value='".$arr["service_id"]."'></td></form>";
			echo "</tr>\n";
		}

		echo "<tr><form action='".$_SERVER['PHP_SELF']."?editShards=true&fshard=$fshard' method=post>";
		echo "<td><input name=shardName size=24 maxlength=32></td>\n";
		echo "<td><select name=serverName>";
		foreach ($servers as $server)
			echo "<option value='$server'>$server";
		echo "</select></td>";
		echo "<td><input name=serviceName size=16 maxlength=32></td>";
		echo "<td><input type=submit name=createService value='Create'></td></form>";
		echo "</tr>\n";

		echo "</table>\n";

		echo "</td></tr></table>\n";
	}

	if ($editServices)
	{
		echo "<b>Services setup</b>".help("Services")."<br><br>\n";
		
		if ($editServiceError != '')
		{
			echo "<b><font color=#FF0000>WARNING: failed to rebuild services list: error '$editServiceError'. List is kept unmodified.</font></b><br><br>\n";
		}
		
		if ($editServiceLog != '')
		{
			echo "<b><font color=#0000FF>RESULT: $editServiceLog</font></b><br>\n";
		}

		echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";
		echo "<form action='".$_SERVER['PHP_SELF']."?editServices=update' method=post>\n";
		echo "<textarea rows=30 cols=300 style='font-family: Terminal, Courier; font-size: 10pt;' name='updateList'>\n";

		$result = sqlquery("SELECT * FROM service ORDER BY shard, server, name");

		echo str_pad('* SHARD', 32)." ".str_pad('* SERVER', 32)." * SERVICE NAME\n";
		echo "*------------------------------------------------------------------------------------------------------------------------\n";

		while ($result && ($arr=sqlfetch($result)))
		{
			echo str_pad($arr['shard'], 32)." ".str_pad($arr['server'], 32)." ".$arr['name']."\n";
		}

		echo "</textarea>\n";
		echo "<input type='submit' name='update' value='Update'>\n";
		echo "</form>\n";
		echo "</td></tr></table>\n";
	}

	if ($editServers)
	{
		echo "<b>Servers setup</b>".help("Servers")."<br><br>\n";

		if ($editServerError != '')
		{
			echo "<b><font color=#FF0000>WARNING: failed to rebuild servers list: error '$editServerError'. List is kept unmodified.</font></b><br><br>\n";
		}
		
		if ($editServerLog != '')
		{
			echo "<b><font color=#0000FF>RESULT: $editServerLog</font></b><br>\n";
		}

		echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";
		echo "<form action='".$_SERVER['PHP_SELF']."?editServers=update' method=post>\n";
		echo "<textarea rows=30 cols=300 style='font-family: Terminal, Courier; font-size: 10pt;' name='updateList'>\n";

		echo str_pad('* SERVER NAME', 32)." * ADDRESS\n";
		echo "*------------------------------------------------------------------------------------------------------------------------\n";

		$result = sqlquery("SELECT * FROM server ORDER BY name, address");

		while ($result && ($arr=sqlfetch($result)))
		{
			echo str_pad($arr['name'], 32)." ".$arr['address']."\n";
		}

		echo "</textarea>\n";
		echo "<input type='submit' name='update' value='Update'>\n";
		echo "</form>\n";
		echo "</td></tr></table>\n";
	}

	htmlEpilog();
?>
