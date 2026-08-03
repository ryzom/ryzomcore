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

	// Every row id below arrives with the request (foo.php turns the query
	// string into these globals) and is written back into the urls, hidden
	// fields and onClick handlers this page builds. The queries already run
	// them through intval(); the pages did not, so anything after the digits
	// was echoed verbatim. They are row ids, so make them numbers once, here,
	// and every later interpolation is a number too.
	foreach (array('uuid', 'confirmRmUid', 'rmUid', 'editUser', 'selGroup', 'editTid',
		'varGroup', 'vid', 'impUid', 'updUid', 'forcePass', 'allowIp',
		'rmVarGroup', 'chVarGroup', 'serviceId', 'uViewGroups') as $nelnsIdVar)
	{
		if (isset($GLOBALS[$nelnsIdVar]) && $GLOBALS[$nelnsIdVar] !== '')
			$GLOBALS[$nelnsIdVar] = (int)$GLOBALS[$nelnsIdVar];
	}
	unset($nelnsIdVar);

	// Deleting an account is a plain href, and SameSite=Lax still sends the
	// session cookie on a top level navigation, so the link carries a token.
	$nelnsCsrf = nelnsCsrfToken();
	$nelnsCsrfUrl = '&csrf='.rawurlencode($nelnsCsrf);
	if (isset($confirmRmUid) && !nelnsCsrfCheck(isset($csrf) ? $csrf : ''))
		unset($confirmRmUid);

	// -----------------------------
	// page commands

	unset($error);

	// remove user, ask for confirmation
	if (isset($rmUid) && isset($uuid) && $uuid!=$uid)
	{
		$result = sqlquery("SELECT login FROM user WHERE uid='".intval($uuid)."'");
		if ($result && sqlnumrows($result) == 1)
		{
			htmlProlog(htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES), "Administration");
			$arr = sqlfetch($result);
			echo "You are about to delete user ".htmlspecialchars($arr["login"], ENT_QUOTES)." (".intval($uid).")<br>\n";
			echo "Are you sure ?<br>\n";
			echo "<font size=+6><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?confirmRmUid=$uuid$nelnsCsrfUrl'>YES</a> | <a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editUsers=true'>NO</a>\n";
			htmlEpilog();
			die;
		}
	}
	// remove effectively user
	else if (isset($confirmRmUid) && $confirmRmUid!=$uid)
	{
		sqlquery("DELETE FROM user WHERE uid='".intval($confirmRmUid)."'");
		$numUserDeleted = sqlaffectedrows();
		sqlquery("DELETE FROM user_variable WHERE uid='".intval($confirmRmUid)."'");

		$result = sqlquery("SELECT tid FROM view_table WHERE uid='".intval($confirmRmUid)."'");
		sqlquery("DELETE FROM view_table WHERE uid='".intval($confirmRmUid)."'");

		while ($result && ($arr=sqlfetch($result)))
		{
			sqlquery("DELETE FROM view_rows WHERE tid='".$arr["tid"]."'");
		}
		$editUsers = true;
	}
	// force user password
	else if (isset($forcePass))
	{
		sqlquery("UPDATE user SET password='".sqlescape(hashPassword($forcedPass))."' WHERE uid='".intval($forcePass)."'");
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

			sqlquery("DELETE FROM user_variable WHERE uid='".intval($editUser)."' AND vid='".intval($vid)."'");
			if ($value != "inv")
				sqlquery("INSERT INTO user_variable SET privilege='".sqlescape($value)."', uid='".intval($editUser)."', vid='".intval($vid)."'");

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
		// strspn() returns the length of the leading run of accepted
		// characters, so "== 0" only rejected a login that *starts* with
		// something else: "ab/../x" got through and became a row that
		// validateId() then refuses to log in, and a file name in the user
		// log directory. Require the whole name, which is what the login
		// path checks for anyway.
		else if (!preg_match('/^[a-zA-Z0-9]+$/', $nulogin))
		{
			$error = $error."Login '".htmlspecialchars($nulogin, ENT_QUOTES)."' contains other characters than alphabetic and digits<br>\n";
		}
		else
		{
			$result = sqlquery("INSERT INTO user SET login='".sqlescape($nulogin)."', password='".sqlescape(hashPassword($nupassword))."', gid='".sqlescape($nugroup)."', allowed_ip='".sqlescape($nuallowedIp)."'");
			if (sqlaffectedrows() != 1)
			{
				$error .= "Can't create user '".htmlspecialchars($nulogin, ENT_QUOTES)."', database request failed (already used login?)<br>\n";
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
		$result = sqlquery("DELETE FROM user_variable WHERE uid='".intval($editUser)."'");
	}
	// import user var setup
/*	else if ($impVarSetup && isset($editUser) && isset($impUid))
	{
		$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='$impUid'");
		if ($result && sqlnumrows($result)>0)
		{
			$delete_query = "DELETE FROM user_variable WHERE uid='$editUser' AND (";
			$copy_query = "INSERT INTO user_variable VALUES";
			$first = true;
			while ($result && ($arr=sqlfetch($result)))
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
		sqlquery("UPDATE user SET gid='".sqlescape($chugroup)."' WHERE uid='".intval($updUid)."'");
		$editUsers=true;
	}
	// update user cookie
	else if (isset($updUid) && isset($chucookie))
	{
		sqlquery("UPDATE user SET useCookie='".sqlescape($chucookie)."' WHERE uid='".intval($updUid)."'");
		$editUsers=true;
	}
	// update user allowed ip
	else if (isset($allowIp) && isset($allowedIp))
	{
		sqlquery("UPDATE user SET allowed_ip='".sqlescape($allowedIp)."' WHERE uid='".intval($allowIp)."'");
		$editUsers=true;
	}

	// remove variable
	else if (isset($rmVar) && isset($vid))
	{
		$editVariables = true;
		sqlquery("DELETE FROM variable WHERE vid='".intval($vid)."'");
		if (sqlaffectedrows() != 1)
		{
			$error .= "Couldn't remove variable ".htmlspecialchars($vid, ENT_QUOTES)."/".htmlspecialchars($chVarName, ENT_QUOTES).", database request failed.<br>\n";
		}
		else
		{
			sqlquery("DELETE FROM user_variable WHERE vid='".intval($vid)."'");
			sqlquery("DELETE FROM view_row WHERE vid='".intval($vid)."'");
			$error .= "Removed effectively variable ".htmlspecialchars($vid, ENT_QUOTES)."/".htmlspecialchars($chVarName, ENT_QUOTES)."/".htmlspecialchars($chVarPath, ENT_QUOTES)."/".htmlspecialchars($chVarState, ENT_QUOTES)."<br>\n";
		}
	}
	// create variable
	else if (isset($createVid) && isset($nvname) && isset($nvpath) && isset($nvstate) && isset($chVarGroup) && isset($nvgraphupdate))
	{
		$editVariables = true;
		$result = sqlquery("INSERT INTO variable SET name='".sqlescape($nvname)."', vgid='".sqlescape($chVarGroup)."', path='".sqlescape($nvpath)."', state='".sqlescape($nvstate)."', warning_bound='".sqlescape($nvwarning)."', error_bound='".sqlescape($nverror)."', alarm_order='".sqlescape($nvorder)."', graph_update='".sqlescape($nvgraphupdate)."', command=".(isset($nvvartype) ? "'variable'" : "'command'"));
		if (sqlaffectedrows() != 1)
		{
			$error .= "Can't create variable '".htmlspecialchars($nvname, ENT_QUOTES)."', database request failed (already used variable name?)<br>\n";
			unset($nvpath);
			unset($nvstate);
		}
		else
		{
			$error .= "Effectively created variable '".htmlspecialchars($nvname, ENT_QUOTES)."'<br>\n";
			$result = sqlquery("SELECT vid FROM variable WHERE name='".sqlescape($nvname)."' AND vgid='".sqlescape($chVarGroup)."' AND path='".sqlescape($nvpath)."' AND state='".sqlescape($nvstate)."'");
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
						
					$error .= "Set right '".htmlspecialchars($priv, ENT_QUOTES)."' to users of group '".htmlspecialchars($id, ENT_QUOTES)."':";

					//$result = sqlquery("SELECT uid, login FROM user WHERE gid='$id'");
					$query = "INSERT INTO user_variable VALUES ('".intval($id)."', '".intval($vid)."', '".sqlescape($priv)."')";
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
		$result = sqlquery("UPDATE variable SET name='".sqlescape($chVarName)."', vgid='".sqlescape($chVarGroup)."', path='".sqlescape($chVarPath)."', state='".sqlescape($chVarState)."', warning_bound='".sqlescape($chVarWarning)."', error_bound='".sqlescape($chVarError)."', alarm_order='".sqlescape($chVarOrder)."', graph_update='".sqlescape($chVarGraphUpdate)."', command=".(isset($chVarType) ? "'variable'" : "'command'")." WHERE vid='".intval($vid)."'");
		if (sqlaffectedrows() == -1)
		{
			$error .= "Can't update variable ".htmlspecialchars($vid, ENT_QUOTES)." properties, database query failed (name changed to already used?)<br>\n";
		}
		else if ($chVarState == "rd")
		{
			sqlquery("UPDATE user_variable SET privilege='rd' WHERE privilege='rw' AND vid='".intval($vid)."'");
		}
	}
	// create var group
	else if (isset($createVarGroup))
	{
		sqlquery("INSERT INTO variable_group SET name='".sqlescape($createVarGroup)."'");
	}
	// remove var group
	// $rmVarGRoup was a typo for $rmVarGroup, so the guard read an undefined
	// variable and never fired: the default group could be deleted
	else if (isset($rmVarGroup) && $rmVarGroup!='1')
	{
		sqlquery("DELETE FROM variable_group WHERE vgid='".intval($rmVarGroup)."'");
		sqlquery("UPDATE variable SET vgid='1' WHERE vgid='".intval($rmVarGroup)."'");
	}
	// add shard access
	else if (isset($nshardaccess) && isset($editUser))
	{
		sqlquery("INSERT INTO shard_access SET uid='".intval($editUser)."', shard='".sqlescape($nshardaccess)."'");
	}
	// remove shard access
	else if (isset($rmShardAccess) && isset($editUser))
	{
		sqlquery("DELETE FROM shard_access WHERE uid='".intval($editUser)."' AND shard='".sqlescape($rmShardAccess)."'");
	}
	// update shard access
	else if (isset($chShardAccess) && isset($editUser))
	{
		sqlquery("DELETE FROM shard_access WHERE uid='".intval($editUser)."'");
		$query = "INSERT INTO shard_access VALUES";
		$first = true;
		if (isset($shardAccesses))
		{
			foreach($shardAccesses as $shard)
			{
				if (!$first)
					$query .= ", ";
				$first = false;
				$query .= "('".intval($editUser)."', '".sqlescape($shard)."')";
			}
			sqlquery($query);
		}
	}
	else if (isset($crViewCommand) && isset($nViewCommand) && isset($nViewCommandName) && isset($editTid))
	{
		sqlquery("INSERT INTO view_command SET name='".sqlescape($nViewCommandName)."', command='".sqlescape($nViewCommand)."', tid='".intval($editTid)."'");
	}
	else if (isset($rmViewCommand) && isset($viewCommand) && isset($editTid))
	{
		sqlquery("DELETE FROM view_command WHERE name='".sqlescape($viewCommand)."' AND tid='".intval($editTid)."'");
	}

	// create server
	else if (isset($createServer) && isset($serverName) && isset($serverIP))
	{
		sqlquery("INSERT INTO server SET name='".sqlescape($serverName)."', address='".sqlescape($serverIP)."'");
	}
	// delete server
	else if (isset($rmServer) && isset($serverName))
	{
		sqlquery("DELETE FROM server WHERE name='".sqlescape($serverName)."'");
	}
	// update server name
	else if (isset($updServerName) && isset($newServerName))
	{
		sqlquery("UPDATE server SET name='".sqlescape($newServerName)."' WHERE name='".sqlescape($updServerName)."'");
		sqlquery("UPDATE service SET server='".sqlescape($newServerName)."' WHERE server='".sqlescape($updServerName)."'");
	}
	// update server ip
	else if (isset($updServerIP) && isset($newServerIP))
	{
		sqlquery("UPDATE server SET address='".sqlescape($newServerIP)."' WHERE name='".sqlescape($updServerIP)."'");
	}
	
	// create service
	else if (isset($createService) && isset($shardName) && isset($serverName) && isset($serviceName))
	{
		sqlquery("INSERT INTO service SET shard='".sqlescape($shardName)."', server='".sqlescape($serverName)."', name='".sqlescape($serviceName)."'");
	}
	// delete service
	else if (isset($rmService) && isset($serviceId))
	{
		sqlquery("DELETE FROM service WHERE service_id='".intval($serviceId)."'");
	}
	// update shard name
	else if (isset($newShardName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET shard='".sqlescape($newShardName)."' WHERE service_id='".intval($serviceId)."'");
	}
	// update server name
	else if (isset($newServerName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET server='".sqlescape($newServerName)."' WHERE service_id='".intval($serviceId)."'");
	}
	// update service name
	else if (isset($newServiceName) && isset($serviceId))
	{
		sqlquery("UPDATE service SET name='".sqlescape($newServiceName)."' WHERE service_id='".intval($serviceId)."'");
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

			if (!preg_match('#^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]*$#', $l, $regs))
			{
				$editServiceError = "Malformed string '".htmlspecialchars($l, ENT_QUOTES)."' at line ".intval($lineCount);
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
					$query = "SELECT * FROM service WHERE shard='".sqlescape($l['shard'])."' AND server='".sqlescape($l['server'])."' AND name='".sqlescape($l['service'])."'";
					$result = sqlquery($query);
					if ($result && sqlnumrows($result) == 0)
					{
						$updateLog .= '<li>updated/inserted service '.htmlspecialchars($l['shard'].'.'.$l['server'].'.'.$l['service'], ENT_QUOTES)."</li>\n";
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
						$query = "INSERT INTO service SET shard='".sqlescape($l['shard'])."', server='".sqlescape($l['server'])."', name='".sqlescape($l['service'])."'";
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

			if (!preg_match('#^[[:space:]]*([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]*$#', $l, $regs))
			{
				$editServerError = "Malformed string '".htmlspecialchars($l, ENT_QUOTES)."' at line ".intval($lineCount);
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
					$query = "SELECT * FROM server WHERE name='".sqlescape($l['server'])."' AND address='".sqlescape($l['ip'])."'";
					$result = sqlquery($query);
					if ($result && sqlnumrows($result) == 0)
					{
						$updateLog .= '<li>updated/inserted server '.htmlspecialchars($l['server'].' at '.$l['ip'], ENT_QUOTES)."</li>\n";
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
						$query = "INSERT INTO server SET name='".sqlescape($l['server'])."', address='".sqlescape($l['ip'])."'";
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

	htmlProlog(htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES), "Administration");
	subBar( array( 	"Users" => htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editUsers=true",
					"Variables" => htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editVariables=true",
					"Services" => htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editServices=true",
					"Servers" => htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editServers=true",
					"Shards" => htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true" ));

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
		$resURL = htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editUser=$editUser&selGroup=$selGroup";

		$result = sqlquery("SELECT * FROM user WHERE uid='".intval($editUser)."'");
		if ($result && ($arr=sqlfetch($result)))
		{
			$editLogin = $arr["login"];
			$defaultView = $arr["default_view"];
			$userGroup = $arr["gid"];

			echo "<b>".htmlspecialchars($editLogin, ENT_QUOTES)." variables/views setup edit</b><br><br>\n";

			echo "<b>User variables setup</b><br>";
			echo "<table cellpadding=0 cellspacing=0><tr valign=top><td>\n";

			// User variables state display/modify
			$editUsers = false;
			
			$vars = false;
			
			unset($vars);
			unset($groups);

			$result = sqlquery("SELECT * FROM variable_group ORDER BY name");
			while ($result && ($arr=sqlfetch($result)))
			{
				if ((!isset($selGroup) || $selGroup == "") && $arr["name"] == "NoGroup")
					$selGroup = $arr["vgid"];
				$groups[$arr["vgid"]] = $arr["name"];
			}

			$result = sqlquery("SELECT vid, variable.name AS name, path, state, variable.vgid AS vgid, variable_group.name AS group_name FROM variable, variable_group WHERE variable.vgid=variable_group.vgid".($selGroup>0 ? " AND variable.vgid='".intval($selGroup)."'" : "")." ORDER BY group_name, name");
			while ($result && ($arr=sqlfetch($result)))
			{
				$arr["priv"] = "inv";
				$vars[] = $arr;
			}
			
			$result = sqlquery("SELECT vid, privilege FROM user_variable WHERE uid='".intval($editUser)."'");
			while ($result && ($arr=sqlfetch($result)))
			{
				for ($i=0; $i<count($vars) && $vars[$i]["vid"] != $arr["vid"]; ++$i)
					;
				if ($i<count($vars))
					$vars[$i]["priv"] = $arr["privilege"];
			}
			
			$usrVarRights = getUserVariableRights($editUser, $userGroup);

			echo "<table border=1>\n";
			echo "<tr><th>Variable</th><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editUser=$editUser'><th>";
			echo "<select name='selGroup' onChange='submit()'>";
			$found = false;
			foreach ($groups as $vgid => $group )
				echo "<option value='".intval($vgid)."'".($selGroup == $vgid ? " selected" : "").">".htmlspecialchars($group, ENT_QUOTES);
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

					echo "<tr><td>".htmlspecialchars($state["name"], ENT_QUOTES)."</td>".
								 "<td>".htmlspecialchars($state["group_name"], ENT_QUOTES)."</td>".
								 "<td>".htmlspecialchars($state["path"], ENT_QUOTES)."</td>".
								 "<td><select name='avv_".intval($vid)."'>";

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
					 echo "</select><input type=hidden name='aovv_".intval($vid)."' value='".htmlspecialchars($state["priv"], ENT_QUOTES)."'></td></tr>\n";

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
				$res = sqlquery("SELECT shard FROM shard_access WHERE uid='".intval($editUser)."'");
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
					$shard_html = htmlspecialchars($shard, ENT_QUOTES);
					echo "<option value='$shard_html'".(isset($shards[$shard]) && $shards[$shard] ? " selected" : "").">$shard_html&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				}
				echo "</select></td></tr>";
				echo "<tr><td align=center><input type=submit value='Update'></td></form></tr>\n";
				echo "</table><br>\n";
			}

			echo "</td><td width=30>\n</td><td align=center>\n";

			echo "<table cellpadding=0 cellspacing>\n";
			// Variable setup import form
			echo "<tr><form method=post action='$resURL'><input type=hidden name=editUser value='".intval($editUser)."'>\n";
			echo "<td align=right><b>Import</b> variables setup from user&nbsp;</td>\n";
			echo "<td><select name=impUid>\n";
			$result = sqlquery("SELECT login, uid FROM user WHERE uid!='".intval($editUser)."' ORDER BY login");
			while ($result && ($arr=sqlfetch($result)))
			{
				echo "<option value='".intval($arr["uid"])."'>".htmlspecialchars($arr["login"], ENT_QUOTES)."\n";
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
			$result = sqlquery("SELECT name, tid FROM view_table WHERE uid='".intval($editUser)."' ORDER BY ordering");
			while ($result && ($arr=sqlfetch($result)))
			{
				$disp = "<a href='$resURL&editTid=".intval($arr["tid"])."'>".htmlspecialchars($arr["name"], ENT_QUOTES)."</a>";
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
				$result = sqlquery("SELECT view_row.name AS name, variable.name AS sname, path, filter FROM view_row, variable WHERE tid='".intval($editTid)."' AND view_row.vid=variable.vid ORDER BY ordering");
				while ($result && ($arr=sqlfetch($result)))
				{
					echo "<tr><td>".htmlspecialchars($arr["name"], ENT_QUOTES)."</td><td>".htmlspecialchars($arr["sname"], ENT_QUOTES)."</td><td>".htmlspecialchars($arr["path"], ENT_QUOTES)."</td><td>".htmlspecialchars($arr["filter"], ENT_QUOTES)."</td></tr>\n";
				}
				echo "</table><br>\n";
				
				echo "<b>Commands</b><br>\n";
				echo "<table border=1 cellpadding=2><tr><th>Name</th><th colspan=2>Service command</th></tr>\n";
				$result = sqlquery("SELECT name, command FROM view_command WHERE tid='".intval($editTid)."' ORDER BY name");
				while ($result && ($arr=sqlfetch($result)))
				{
					echo "<tr><form method=post action='$resURL&editTid=".intval($editTid)."'><td>".htmlspecialchars($arr["name"], ENT_QUOTES)."</td><td>".htmlspecialchars($arr["command"], ENT_QUOTES)."</td><td><input type=hidden name=viewCommand value='".htmlspecialchars($arr["name"], ENT_QUOTES)."'><input type=submit name='rmViewCommand' value='Delete'></td></form></tr>\n";
				}
				echo "<tr><td colspan=3 height=5></td></tr>\n";
				echo "<tr><form method=post action='$resURL&editTid=".intval($editTid)."'><td><input name=nViewCommandName size=16 maxlength=32></td><td><input name=nViewCommand size=16 maxlength=32></td><td><input type=submit name='crViewCommand' value='Create'></td></form></tr>\n";
				echo "</table><br>\n";
			}

			echo "</td></tr></table><br>\n";

			// user activity display
			echo "<b>User activity</b><br>\n";
			echo "<textarea rows=15 cols=200 readOnly style='font-family: Terminal, Courier; font-size: 10pt;' nowrap>";
			//$editLogin
			$logfilename = $userlogpath."/".$editLogin.".log";
			unset($resExec);
			// $editLogin is a login read back out of the database, so it does
			// not belong on a command line unquoted
			exec("tail -n 40 ".escapeshellarg($logfilename), $resExec);

			// Log lines include User-Agent from logUser(); escape so a prior
			// login with a hostile agent cannot break out of the textarea.
			echo htmlspecialchars(join("\n", $resExec), ENT_QUOTES);

			echo "</textarea><br>\n";
		}
		else
		{
			echo "User ".intval($editUser)." not found in database.<br><br>";
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
		while ($result && ($arr=sqlfetch($result)))
		{
			$groupNames[$arr["gname"]] = $arr["uid"];
			if ($arr["gid"] == $arr["uid"])
				$actualGroups[$arr["gname"]] = $arr["uid"];
		}

		echo "<table border=1>\n";
		echo "<tr><th>Login</th><th>Uid</th>";
		echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'><th><select name=uViewGroups onChange='submit()'>";
		echo  "<option value=''".((!isset($uViewGroups) || $uViewGroups=='') ? " selected" : "").">All Groups\n";
		foreach($actualGroups as $cgname => $cgid)
			echo  "<option value='".intval($cgid)."'".($cgid==$uViewGroups ? " selected" : "").">".htmlspecialchars($cgname, ENT_QUOTES)."\n";
		echo "</select></th></form>";
		echo "<th>Cookie</th><th>Force password</th><th>Allowed IP mask</th><th>Commands</th></tr>\n";

		if (!isset($uViewGroups) ||  $uViewGroups == '')
			$query = "SELECT uuser.login AS login, uuser.uid AS uid, uuser.useCookie AS useCookie, uuser.gid AS gid, ugroup.login AS gname, uuser.allowed_ip AS allowed_ip FROM user AS uuser, user AS ugroup WHERE uuser.gid=ugroup.uid ORDER BY uid";
		else
			$query = "SELECT uuser.login AS login, uuser.uid AS uid, uuser.useCookie AS useCookie, uuser.gid AS gid, ugroup.login AS gname, uuser.allowed_ip AS allowed_ip FROM user AS uuser, user AS ugroup WHERE uuser.gid=ugroup.uid AND uuser.gid='".intval($uViewGroups)."' ORDER BY uid";
		$result = sqlquery($query);
		while ($result && ($arr=sqlfetch($result)))
		{
			$ulogin = $arr["login"];
			$uuid = intval($arr["uid"]);
			$ugid = $arr["gid"];
			$ugname = $arr["gname"];
			// The allowed ip mask is stored exactly as it was posted, so it is
			// whatever the last editor typed; it lands in an attribute here,
			// and the login lands in a javascript string inside onClick.
			$ulogin_html = htmlspecialchars($ulogin, ENT_QUOTES);
			$ulogin_js = htmlspecialchars(strtr($ulogin, array('\\'=>'\\\\', "'"=>"\\'", "\r"=>'', "\n"=>'')), ENT_QUOTES);
			$uallowedip = htmlspecialchars($arr["allowed_ip"], ENT_QUOTES);
			$uuseCookie = ($arr["useCookie"] == "yes");
			echo "<tr>".
						"<td><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editUser=$uuid'>$ulogin_html</a></td>\n".
						"<td>$uuid</td>\n".
						"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'>".
						"<input type=hidden name=updUid value='$uuid'><td><select name=chugroup onChange='submit()'>\n";
			foreach($groupNames as $cgname => $cgid)
				echo  "<option value='".intval($cgid)."'".($cgid==$ugid ? " selected" : "").">".htmlspecialchars($cgname, ENT_QUOTES)."\n";
			echo     "</select></td></form>\n";
			echo 		"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'><input type=hidden name=updUid value='$uuid'><td><select name='chucookie' onChange='submit()'><option value='yes'".($uuseCookie ? " selected" : "").">Yes<option value='no'".($uuseCookie ? "" : " selected").">No</select></td></form>";
			echo 		"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'><input type=hidden name=forcePass value='$uuid'><td><input type=password name='forcedPass'></td></form>";
			echo 		"<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'><input type=hidden name=allowIp value='$uuid'><td><input name='allowedIp' value='$uallowedip'></td></form>";
			echo		"<td><a href='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?confirmRmUid=$uuid$nelnsCsrfUrl' onClick=\"return confirm('You are about to delete user $ulogin_js')\">Delete</a></td></tr>\n";
		}
		echo "</table><br>\n";
		
		$result = sqlquery("SELECT login, uid FROM user");

		echo "<table border=1><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."'>\n";
		echo "<tr><th colspan=2>Create a new user</th></tr>\n";
		echo "<tr><td>Login</td><td><input name=nulogin maxlength=16 size=16 value='".htmlspecialchars(isset($nulogin) ? $nulogin : '', ENT_QUOTES)."'></td></tr>\n";
		echo "<tr><td>Group</td><td><select name=nugroup>\n";
		while ($result && ($arr=sqlfetch($result)))
			echo "<option value='".intval($arr["uid"])."'>".htmlspecialchars($arr["login"], ENT_QUOTES)."\n";
		echo "<tr><td>Allowed IP mask</td><td><input name=nuallowedIp maxlength=32 size=16 value='".htmlspecialchars(isset($nuallowedIp) ? $nuallowedIp : '', ENT_QUOTES)."'></td></tr>\n";
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
			while ($result && ($arr=sqlfetch($result)))
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
					
					$result = sqlquery("SELECT count(*) as count FROM variable WHERE name='".sqlescape($vname)."'");
					if ($result && ($arr=sqlfetch($result)) && $arr["count"] == 0)
					{
						if (!isset($groupnames[$vgname]))
						{
							sqlquery("INSERT INTO variable_group SET name='".sqlescape($vgname)."'");
							$result = sqlquery("SELECT vgid FROM variable_group WHERE name='".sqlescape($vgname)."'");
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
							sqlquery("INSERT INTO variable SET name='".sqlescape($vname)."', path='".sqlescape($vpath)."', state='".sqlescape($vstate)."', vgid='".intval($vgid)."', warning_bound='".sqlescape($vwarn)."', error_bound='".sqlescape($verr)."', alarm_order='".sqlescape($valarm)."', graph_update='".sqlescape($vgraph)."', command='".sqlescape($vcmd)."'");
						}
					}
				}
			}
		}

		$result = sqlquery("SELECT name, vgid FROM variable_group");
		unset($groups);
		while ($result && ($arr=sqlfetch($result)))
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
			$result = sqlquery("SELECT * FROM variable WHERE vgid='".intval($varGroup)."' ORDER BY name");
		echo "<table border=1>\n";
		echo "<tr><th>Name</th><th>Vid</th><th>Group</th><th>Path</th><th>State</th><th>Warning</th><th>Error</th><th>Order</th><th>Graph</th><th>Variable</th><th colspan=2>Commands</th></tr>\n";

		$lastGroup = -1;

		while ($result && ($arr=sqlfetch($result)))
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

			// name, path and the bounds are free text stored by whoever last
			// edited this row, and every one of them lands in an attribute
			$vid = intval($vid);
			$name_html			= htmlspecialchars($name, ENT_QUOTES);
			$path_html			= htmlspecialchars($path, ENT_QUOTES);
			$warn_bound_html	= htmlspecialchars($warn_bound, ENT_QUOTES);
			$err_bound_html		= htmlspecialchars($err_bound, ENT_QUOTES);
			$graph_update_html	= htmlspecialchars($graph_update, ENT_QUOTES);

			echo "<tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup'><input type=hidden name=vid value='$vid'><input type=hidden name=chVar value='Update'>".
						"<td><input name=chVarName maxlength=128 size=16 value='$name_html'></td>\n".
						"<td>$vid</td>\n";
			echo		"<td><select name=chVarGroup onChange='submit()'>";
			foreach ($groups as $chvgid => $chvgname)
				echo	"<option value='".intval($chvgid)."'".($chvgid==$vgid ? " selected":"").">".htmlspecialchars($chvgname, ENT_QUOTES);
			echo		"</select></td>\n".
						"<td><input name=chVarPath maxlength=255 size=32 value='$path_html'></td>\n".
						"<td><select name=chVarState onChange='submit()'><option value='rd'".($state=="rd" ? " selected":"").">Read only<option value='rw'".($state=="rw" ? " selected":"").">Read write</select></td>".
						"<td><input name=chVarWarning maxlength=11 size=11 value='$warn_bound_html'></td>".
						"<td><input name=chVarError maxlength=11 size=11 value='$err_bound_html'></td>".
						"<td><select name=chVarOrder onChange='submit()'><option value='gt'".($alarm_order=="gt" ? " selected":"").">gt<option value='lt'".($alarm_order=="lt" ? " selected":"").">lt</select></td>".
						"<td><input name=chVarGraphUpdate maxlength=8 size=4 value='$graph_update_html'></td>".
						"<td align=center><input type=checkbox name=chVarType".($var_type == "variable" ? " checked" : "")." value='1'></td>".
						"<td><input type=submit name=chVar value='Update'></td></form><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup'><td><input type=hidden name='vid' value='$vid'><input type=submit name=rmVar value='Delete' onClick=\"return confirm('You are about to delete a Variable')\"></td>".
					"</form></tr>\n";
		}
		echo "<tr height=10><td colspan=12></td></tr>\n";
		if (!isset($nvpath))		$nvpath = "*.*.*.*[.*]";
		if (!isset($nvstate))	$nvstate = "rd";
		echo "<tr valign=top><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editVariables=true&varGroup=$varGroup'><input type=hidden name=vid value='".intval($vid)."'>".
					"<td><input name=nvname maxlength=128 size=16 value='".htmlspecialchars(isset($nvname) ? $nvname : '', ENT_QUOTES)."'></td>\n".
					"<td></td>\n";
		echo		"<td><select name=chVarGroup>";
		foreach ($groups as $chvgid => $chvgname)
			echo	"<option value='".intval($chvgid)."'".($chvgid==$varGroup ? " selected":"").">".htmlspecialchars($chvgname, ENT_QUOTES);
		echo		"</select></td>\n".
					"<td><input name=nvpath maxlength=255 size=32 value='".htmlspecialchars($nvpath, ENT_QUOTES)."'></td>\n".
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
			echo "<tr><td>".htmlspecialchars($arr["login"], ENT_QUOTES)."</td>";
			echo "<td align=center><input type=radio value='rw' name='setgroup_".intval($arr["uid"])."' checked></td>";
			echo "<td align=center><input type=radio value='rd' name='setgroup_".intval($arr["uid"])."'></td>";
			echo "<td align=center><input type=radio value='' name='setgroup_".intval($arr["uid"])."'></td>";
			echo "</tr>\n";
		}
		echo "</form></table>\n";

		echo "</td><td></td></tr>\n";
		echo "</table><br>\n";
		
		echo "</td><td width=30>&nbsp;</td><td>\n";
		
		echo "<table>\n";
		echo "<tr><td align=center colspan=2>View by variable group</td></tr><tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editVariables=true'>\n";
		echo "<td align=center colspan=2><select name=varGroup onChange='submit()'>\n";
		echo "<option value='-1'".($varGroup=="-1" ? " selected":"").">All groups";
		foreach ($groups as $vgid => $vgname)
			echo "<option value='".intval($vgid)."'".($vgid==$varGroup ? " selected":"").">".htmlspecialchars($vgname, ENT_QUOTES);
		echo "</select></td></form></tr>";
		
		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr>\n";
		echo "<td align=center colspan=2>Create a variable group</td></tr><tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=right><input name=createVarGroup size=16 maxlength=32></td><td><input type=submit value='Create'></td></form>\n";
		echo "</tr>\n";

		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr><td align=center colspan=2>Delete a variable group</td></tr><tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=right><select name=rmVarGroup>\n";
		foreach ($groups as $vgid => $vgname)
			if ($vgid!=1)
				echo "<option value='".intval($vgid)."'".($vgid==$varGroup ? " selected":"").">".htmlspecialchars($vgname, ENT_QUOTES);
		echo "</select></td><td><input type=submit value='Delete'>\n";
		echo "</td></form></tr>";

		echo "<tr><td colspan=2><hr></td></tr>\n";

		echo "<tr>\n";
		echo "<td align=center colspan=2>Export variables setup</td></tr><tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<td align=center colspan=2><input type=submit name='exportVarSetup' value='Export'></td></form>\n";
		echo "</tr>\n";

		echo "</table>\n";


		echo "</td></tr></table>\n";
		
		echo "<form method='post' action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?varGroup=$varGroup&editVariables=true'>\n";
		echo "<b>Import/Exported setup</b> (use this to export to another admin tool):<br>\n";
		echo "<textarea rows=30 cols=160 name='importedVarSetup'>";
		if ($exportVarSetup)
		{
			$result = sqlquery("SELECT * FROM variable ORDER BY vgid, name");
			// a '</textarea>' anywhere in these ends the box and puts the rest
			// of the export in the document
			while ($result && ($arr=sqlfetch($result)))
				echo htmlspecialchars($arr["name"]."|".$arr["path"]."|".$arr["state"]."|".$groups[$arr["vgid"]]."|".$arr["warning_bound"]."|".$arr["error_bound"]."|".$arr["alarm_order"]."|".$arr["graph_update"]."|".$arr["command"], ENT_QUOTES)."\n";
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
		
		if (!isset($serverOrder) || !in_array($serverOrder, array('name', 'address')))
			$serverOrder = "name";

		if (!isset($serviceOrder) || !in_array($serviceOrder, array('shard, server, name', 'shard', 'server', 'name', 'service_id')))
			$serviceOrder = "shard, server, name";

		// the shard filter comes from the request and is put back into every
		// form action on this page
		if (!isset($fshard) || !is_string($fshard))
			$fshard = "";
		$fshard_url = htmlspecialchars(rawurlencode($fshard), ENT_QUOTES);

		unset($servers);

		$result = sqlquery("SELECT * FROM server ORDER BY $serverOrder");
		echo "<table border=1><tr><th>Name</th><th>Address</th><th>Command</th></tr>\n";
		while ($result && ($arr=sqlfetch($result)))
		{
			echo "<tr><form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input name=newServerName value=\"".htmlspecialchars($arr["name"], ENT_QUOTES)."\" size=16 maxlength=32><input type=hidden name=updServerName value=\"".htmlspecialchars($arr["name"], ENT_QUOTES)."\"></td></form>";
			echo "<form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input name=newServerIP value=\"".htmlspecialchars($arr["address"], ENT_QUOTES)."\" size=16 maxlength=32><input type=hidden name=updServerIP value=\"".htmlspecialchars($arr["name"], ENT_QUOTES)."\"></td></form>";
			echo "<form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input type=submit name=rmServer value=\"Delete\"><input type=hidden name=serverName value=\"".htmlspecialchars($arr["name"], ENT_QUOTES)."\"></td></form></tr>\n";
			$servers[] = $arr["name"];
		}
		echo "<tr><form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input name=serverName size=16 maxlength=32></td><td><input name=serverIP size=16 maxlength=32></td><td><input type=submit name=createServer value=\"Create\"></td></form></tr>\n";
		echo "</table>\n";

		echo "</td><td width=20>&nbsp;\n";
		echo "</td><td>\n";

		if ($fshard == "")
			unset($result);
		else if ($fshard == "*")
			$result = sqlquery("SELECT * FROM service ORDER BY $serviceOrder");
		else
			$result = sqlquery("SELECT * FROM service WHERE shard LIKE '%".sqlescape($fshard)."%' ORDER BY $serviceOrder");

		echo "<table border=1><tr><form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true'><th>Shard ";
		echo "<select name=fshard onChange='submit()'>";
		echo "<option value=''".($fshard=="" ? " selected" : "").">No shard";
		echo "<option value='*'".($fshard=="*" ? " selected" : "").">All shards";
		$res = sqlquery("SELECT DISTINCT shard FROM service");
		while ($res && ($arr=sqlfetch($res)))
			echo "<option value='".htmlspecialchars($arr["shard"], ENT_QUOTES)."'".($fshard==$arr["shard"] ? " selected" : "").">".htmlspecialchars($arr["shard"], ENT_QUOTES);
		echo "</select>";
		echo "</th></form><th>Server</th><th>Service</th><th>Command</th></tr>\n";
		while ($result && ($arr=sqlfetch($result)))
		{
			echo "<tr><form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input name=newShardName value='".htmlspecialchars($arr["shard"], ENT_QUOTES)."' size=24 maxlength=32><input type=hidden name=serviceId value='".intval($arr["service_id"])."'></td></form>\n";
			echo "<form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><input type=hidden name=serviceId value='".intval($arr["service_id"])."'><td>";
			echo "<select name=newServerName onChange='submit()'>";
			$foundServer = false;
			foreach ($servers as $server)
			{
				$server_html = htmlspecialchars($server, ENT_QUOTES);
				echo "<option value='$server_html'";
				if ($server == $arr["server"])
				{
					echo " selected";
					$foundServer = true;
				}
				echo ">$server_html";
			}
			if (!$foundServer)
				echo "<option value='".htmlspecialchars($arr["server"], ENT_QUOTES)."' selected>".htmlspecialchars($arr["server"], ENT_QUOTES);
			echo "</select>";
			echo "</td></form><form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post><td><input name=newServiceName value='".htmlspecialchars($arr["name"], ENT_QUOTES)."' size=16 maxlength=32><input type=hidden name=serviceId value='".intval($arr["service_id"])."'></td></form>";
			echo "<form method=post action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url'><td><input type=submit name=rmService value='Delete'><input type=hidden name=serviceId value='".intval($arr["service_id"])."'></td></form>";
			echo "</tr>\n";
		}

		echo "<tr><form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editShards=true&fshard=$fshard_url' method=post>";
		echo "<td><input name=shardName size=24 maxlength=32></td>\n";
		echo "<td><select name=serverName>";
		foreach ($servers as $server)
			echo "<option value='".htmlspecialchars($server, ENT_QUOTES)."'>".htmlspecialchars($server, ENT_QUOTES);
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
		echo "<form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editServices=update' method=post>\n";
		echo "<textarea rows=30 cols=300 style='font-family: Terminal, Courier; font-size: 10pt;' name='updateList'>\n";

		$result = sqlquery("SELECT * FROM service ORDER BY shard, server, name");

		echo str_pad('* SHARD', 32)." ".str_pad('* SERVER', 32)." * SERVICE NAME\n";
		echo "*------------------------------------------------------------------------------------------------------------------------\n";

		while ($result && ($arr=sqlfetch($result)))
		{
			echo htmlspecialchars(str_pad($arr['shard'], 32)." ".str_pad($arr['server'], 32)." ".$arr['name'], ENT_QUOTES)."\n";
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
		echo "<form action='".htmlspecialchars($_SERVER['PHP_SELF'], ENT_QUOTES)."?editServers=update' method=post>\n";
		echo "<textarea rows=30 cols=300 style='font-family: Terminal, Courier; font-size: 10pt;' name='updateList'>\n";

		echo str_pad('* SERVER NAME', 32)." * ADDRESS\n";
		echo "*------------------------------------------------------------------------------------------------------------------------\n";

		$result = sqlquery("SELECT * FROM server ORDER BY name, address");

		while ($result && ($arr=sqlfetch($result)))
		{
			echo htmlspecialchars(str_pad($arr['name'], 32)." ".$arr['address'], ENT_QUOTES)."\n";
		}

		echo "</textarea>\n";
		echo "<input type='submit' name='update' value='Update'>\n";
		echo "</form>\n";
		echo "</td></tr></table>\n";
	}

	htmlEpilog();
?>
