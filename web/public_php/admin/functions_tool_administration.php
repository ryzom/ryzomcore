<?php

	$tool_admin_menu	= array(array('title'	=>	'Main',
									  'key'		=>	'help',
									  'uri'		=>	'tool_administration.php?toolmode=help',
									  'tpl'		=>	'tool_administration.tpl',
									  'access'	=>	'', // ALWAYS LEAVE EMPTY !
									  ),
								array('title'	=>	'Users',
									  'key'		=>	'users',
									  'uri'		=>	'tool_administration.php?toolmode=users',
									  'tpl'		=>	'tool_administration_users.tpl',
									  'access'	=>	'tool_admin_user',
									  ),
								array('title'	=>	'Groups',
									  'key'		=>	'groups',
									  'uri'		=>	'tool_administration.php?toolmode=groups',
									  'tpl'		=>	'tool_administration_groups.tpl',
									  'access'	=>	'tool_admin_group',
									  ),
								array('title'	=>	'Restarts',
									  'key'		=>	'restarts',
									  'uri'		=>	'tool_administration.php?toolmode=restarts',
									  'tpl'		=>	'tool_administration_restarts.tpl',
									  'access'	=>	'tool_admin_restart',
									  ),
								array('title'	=>	'Applications',
									  'key'		=>	'applications',
									  'uri'		=>	'tool_administration.php?toolmode=applications',
									  'tpl'		=>	'tool_administration_applications.tpl',
									  'access'	=>	'tool_admin_application',
									  ),
								array('title'	=>	'Domains',
									  'key'		=>	'domains',
									  'uri'		=>	'tool_administration.php?toolmode=domains',
									  'tpl'		=>	'tool_administration_domains.tpl',
									  'access'	=>	'tool_admin_domain',
									  ),
								array('title'	=>	'Shards',
									  'key'		=>	'shards',
									  'uri'		=>	'tool_administration.php?toolmode=shards',
									  'tpl'		=>	'tool_administration_shards.tpl',
									  'access'	=>	'tool_admin_shard',
									  ),
								array('title'	=>	'Logs',
									  'key'		=>	'logs',
									  'uri'		=>	'tool_administration.php?toolmode=logs',
									  'tpl'		=>	'tool_administration_logs.tpl',
									  'access'	=>	'tool_admin_logs',
									  ),
								);

	$tool_language_list	= array(array('lang_id'		=>	'en',
									  'lang_name'	=>	'English',
									  ),
								array('lang_id'		=>	'fr',
									  'lang_name'	=>	'French',
									  ),
								array('lang_id'		=>	'de',
									  'lang_name'	=>	'German',
									  ),
								);

	function tool_admin_menu_get_item_from_key($key)
	{
		global $tool_admin_menu;

		reset($tool_admin_menu);
		foreach($tool_admin_menu as $tool_menu)
		{
			if ($tool_menu['key'] == $key)	return $tool_menu;
		}

		return null;
	}

	function tool_admin_users_get_list($group_list)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." ORDER BY user_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = array();
				while ($row = $db->sql_fetchrow($result))
				{
					$row['user_group_name'] = tool_admin_groups_get_name_from_id($group_list, $row['user_group_id']);
					$data[] = $row;
				}
			}
		}

		return $data;
	}

	function tool_admin_groups_get_user_list($group_id)
	{
		global $db;

		$data = array();

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_group_id=". $group_id ." ORDER BY user_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_groups_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." ORDER BY group_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				while ($row = $db->sql_fetchrow($result))
				{
					$row['group_level_name'] = tool_admin_groups_get_level_name_from_id($row['group_level']);
					$data[] = $row;
				}
			}
		}

		return $data;
	}

	function tool_admin_groups_get_level_name_from_id($group_level)
	{
		global $nel_user_group_levels;

		reset($nel_user_group_levels);
		foreach($nel_user_group_levels as $level_data)
		{
			if ($group_level == $level_data['level_id'])
			{
				return $level_data['level_name'];
			}
		}

		return tool_admin_groups_get_level_name_from_id(0);
	}

	function tool_admin_groups_get_name_from_id($group_list, $group_id)
	{
		$data = 'unknown';

		reset($group_list);
		foreach($group_list as $group_data)
		{
			if ($group_data['group_id'] == $group_id)
			{
				$data = $group_data['group_name'];
			}
		}

		return $data;
	}

	function tool_admin_users_get_id($user_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_id=". $user_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}
		return $data;
	}

	function tool_admin_users_add($user_name, $user_password, $user_group, $user_active)
	{
		global $db;

		$user_name 		= trim($user_name);
		$user_password	= trim($user_password);

		if ($user_name == '')		return "/!\ Error: user name is empty!";
		if ($user_password == '')	return "/!\ Error: password is empty!";

		$user_exists = tool_admin_users_name_exist($user_name);
		if (!$user_exists)
		{
			$sql  = "INSERT INTO ". NELDB_USER_TABLE;
			$sql .= " (`user_name`,`user_password`,`user_group_id`,`user_created`,`user_active`)";
			$sql .= " VALUES ";
			$sql .= " ('". $user_name ."','". md5($user_password) ."','". $user_group ."','". time() ."','". $user_active ."')";
			$db->sql_query($sql);
			return "";
		}

		return "/!\ Error: user name already exists in the database!";
	}

	function tool_admin_users_name_exist($user_name)
	{
		global $db;

		$exists = false;

		$sql = "SELECT user_id, user_name FROM ". NELDB_USER_TABLE ." WHERE user_name='". $user_name ."'";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$exists = true;
			}
		}

		return $exists;
	}

	function tool_admin_users_del($user_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_TABLE ." WHERE user_id=". $user_id;
		$db->sql_query($sql);
	}

	function tool_admin_users_update($user_id, $user_name, $user_password, $user_group, $user_active)
	{
		global $db;

		$user_name		= trim($user_name);
		$user_password	= trim($user_password);

		if ($user_name == "") 						return "/!\ Error: user name is empty!";
		if (!ereg("^([[:alnum:]]+)$",$user_name))	return "/!\ Error: invalid user name, only alpha numerical characters allowed!";

		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_name='". $user_name ."' AND user_id<>". $user_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				return "/!\ Error: user name already exists in database!";
			}
		}

		$sql_ext = "";
		if ($user_password != '')	$sql_ext = ",user_password='". md5($user_password) ."'";

		$sql = "UPDATE ". NELDB_USER_TABLE ." SET user_name='". $user_name ."',user_group_id='". $user_group ."',user_active='". $user_active ."'". $sql_ext ." WHERE user_id=". $user_id;
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_groups_get_id($group_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." WHERE group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_groups_add($group_name, $group_level, $group_default, $group_active)
	{
		global $db;

		$group_name = trim($group_name);
		if ($group_name == '')		return "/!\ Error: group name is empty!";

		$group_exists = tool_admin_groups_name_exist($group_name);
		if (!$group_exists)
		{
			if ($group_default == 1)
			{
				$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_default=0";
				$db->sql_query($sql);
			}

			$sql  = "INSERT INTO ". NELDB_GROUP_TABLE;
			$sql .= " (`group_name`,`group_level`,`group_default`,`group_active`) ";
			$sql .= " VALUES ";
			$sql .= " ('". $group_name ."',". $group_level .",". $group_default .",". $group_active .")";
			$db->sql_query($sql);

			return "";
		}

		return "/!\ Error: group name already exists in the database!";
	}

	function tool_admin_groups_name_exist($group_name)
	{
		global $db;

		$exists = false;

		$sql = "SELECT group_id, group_name FROM ". NELDB_GROUP_TABLE ." WHERE group_name='". $group_name ."'";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$exists = true;
			}
		}

		return $exists;
	}


	function tool_admin_groups_del($group_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_GROUP_TABLE ." WHERE group_id=". $group_id;
		$db->sql_query($sql);
	}

	function tool_admin_groups_update($group_id, $group_name, $group_level, $group_default, $group_active)
	{
		global $db;

		$group_name = trim($group_name);

		if ($group_name == "")						return "/!\ Error: group name is empty!";
		if (!ereg("^([[:alnum:]]+)$",$group_name))	return "/!\ Error: invalid group name, only alpha numerical characters allowed!";

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." WHERE group_name='". $group_name ."' AND group_id<>". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				return "/!\ Error: group name already exists in database!";
			}
		}

		if ($group_default == 1)
		{
			$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_default=0";
			$db->sql_query($sql);
		}

		$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_name='". $group_name ."',group_level='". $group_level ."',group_default='". $group_default ."',group_active='". $group_active ."' WHERE group_id=". $group_id;
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_groups_update_default_domain($group_id, $domain_id)
	{
		global $db;

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." WHERE group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$row = $db->sql_fetchrow($result);

				if ($row['group_default_domain_id'] != $domain_id)
				{
					$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_default_domain_id=". $domain_id .",group_default_shard_id=0 WHERE group_id=". $group_id;
					$db->sql_query($sql);
				}
			}
			else
			{
				return "/!\ Error: invalid group id!";
			}
		}

		return "";
	}

	function tool_admin_groups_update_default_shard($group_id, $shard_id)
	{
		global $db;

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." WHERE group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_default_shard_id=". $shard_id ." WHERE group_id=". $group_id;
				$db->sql_query($sql);
			}
			else
			{
				return "/!\ Error: invalid group id!";
			}
		}

		return "";
	}

	function tool_admin_groups_update_default_application($group_id, $application_id)
	{
		global $db;

		$sql = "SELECT * FROM ". NELDB_GROUP_TABLE ." WHERE group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$sql = "UPDATE ". NELDB_GROUP_TABLE ." SET group_default_application_id=". $application_id ." WHERE group_id=". $group_id;
				$db->sql_query($sql);
			}
			else
			{
				return "/!\ Error: invalid group id!";
			}
		}

		return "";
	}


	function tool_admin_applications_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_APPLICATION_TABLE ." ORDER BY application_order ASC, application_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_applications_get_id($application_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_APPLICATION_TABLE ." WHERE application_id=". $application_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_applications_add($application_name, $application_uri, $application_restriction, $application_icon, $application_order, $application_visible)
	{
		global $db;

		$application_name = trim($application_name);
		if ($application_name == '')	return "/!\ Error: application name is empty!";

		$application_exists = tool_admin_applications_name_exist($application_name);
		if (!$application_exists)
		{
			$sql  = "INSERT INTO ". NELDB_APPLICATION_TABLE;
			$sql .= " (`application_name`,`application_uri`,`application_restriction`,`application_order`,`application_visible`,`application_icon`) ";
			$sql .= " VALUES ";
			$sql .= " ('". $application_name ."','". $application_uri ."','". $application_restriction ."','". $application_order ."','". $application_visible ."','". $application_icon ."')";
			$db->sql_query($sql);

			return "";
		}

		return "/!\ Error: application name already exists in the database!";
	}

	function tool_admin_applications_name_exist($application_name)
	{
		global $db;

		$exists = false;

		$sql = "SELECT application_id, application_name FROM ". NELDB_APPLICATION_TABLE ." WHERE application_name='". $application_name ."'";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$exists = true;
			}
		}

		return $exists;
	}

	function tool_admin_applications_del($application_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_APPLICATION_TABLE ." WHERE user_application_application_id=". $application_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_GROUP_APPLICATION_TABLE ." WHERE group_application_application_id=". $application_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_APPLICATION_TABLE ." WHERE application_id=". $application_id;
		$db->sql_query($sql);
	}

	function tool_admin_applications_update($application_id, $application_name, $application_uri, $application_restriction, $application_icon, $application_order, $application_visible)
	{
		global $db;

		$application_name = trim($application_name);
		if ($application_name == "")	return "/!\ Error: application name is empty!";

		$sql = "SELECT * FROM ". NELDB_APPLICATION_TABLE ." WHERE application_name='". $application_name ."' AND application_id<>". $application_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				return "/!\ Error: application name already exists in database!";
			}
		}

		$sql = "UPDATE ". NELDB_APPLICATION_TABLE ." SET application_name='". $application_name ."',application_uri='". $application_uri ."',application_restriction='". $application_restriction ."',application_icon='". $application_icon ."',application_order='". $application_order ."',application_visible='". $application_visible ."' WHERE application_id=". $application_id;
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_domains_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_DOMAIN_TABLE ." ORDER BY domain_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_domains_add($domain_name, $domain_application, $domain_as_host, $domain_as_port, $domain_rrd_path, $domain_las_admin_path, $domain_las_local_path, $domain_sql_string, $domain_cs_sql_string, $domain_hd_check, $domain_mfs_web)
	{
		global $db;

		$domain_name = trim($domain_name);
		if ($domain_name == '')	return "/!\ Error: domain name is empty!";

		$domain_exists = tool_admin_domains_name_exist($domain_name);
		if (!$domain_exists)
		{
			$sql  = "INSERT INTO ". NELDB_DOMAIN_TABLE;
			$sql .= " (`domain_name`,`domain_application`,`domain_as_host`,`domain_as_port`,`domain_rrd_path`,`domain_las_admin_path`,`domain_las_local_path`,`domain_sql_string`,`domain_hd_check`,`domain_mfs_web`,`domain_cs_sql_string`) ";
			$sql .= " VALUES ";
			$sql .= " ('". $domain_name ."','". $domain_application ."','". $domain_as_host ."','". $domain_as_port ."','". $domain_rrd_path ."','". $domain_las_admin_path ."','". $domain_las_local_path ."','". $domain_sql_string ."',". $domain_hd_check .",'". $domain_mfs_web ."','". $domain_cs_sql_string ."') ";
			$db->sql_query($sql);

			return "";
		}

		return "/!\ Error: domain name already exists in the database!";
	}

	function tool_admin_domains_name_exist($domain_name)
	{
		global $db;

		$exists = false;

		$sql = "SELECT domain_id, domain_name FROM ". NELDB_DOMAIN_TABLE ." WHERE domain_name='". $domain_name ."'";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$exists = true;
			}
		}

		return $exists;
	}

	function tool_admin_domains_get_id($domain_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_DOMAIN_TABLE ." WHERE domain_id=". $domain_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_domains_del($domain_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_SHARD_TABLE  ." WHERE user_shard_domain_id=". $domain_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_USER_DOMAIN_TABLE ." WHERE user_domain_domain_id=". $domain_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_GROUP_SHARD_TABLE  ." WHERE group_shard_domain_id=". $domain_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_GROUP_DOMAIN_TABLE ." WHERE group_domain_domain_id=". $domain_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_SHARD_TABLE ." WHERE shard_domain_id=". $domain_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_DOMAIN_TABLE ." WHERE domain_id=". $domain_id;
		$db->sql_query($sql);

	}

	function tool_admin_domains_update($domain_id, $domain_name, $domain_application, $domain_as_host, $domain_as_port, $domain_rrd_path, $domain_las_admin_path, $domain_las_local_path, $domain_sql_string, $domain_cs_sql_string, $domain_hd_check, $domain_mfs_web)
	{
		global $db;

		$domain_name = trim($domain_name);

		if ($domain_name == "")	return "/!\ Error: domain name is empty!";

		$sql = "SELECT * FROM ". NELDB_DOMAIN_TABLE ." WHERE domain_name='". $domain_name ."' AND domain_id<>". $domain_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				return "/!\ Error: domain name already exists in database!";
			}
		}

		$sql = "UPDATE ". NELDB_DOMAIN_TABLE ." SET domain_name='". $domain_name."',domain_application='". $domain_application ."',domain_as_host='". $domain_as_host ."',domain_as_port='". $domain_as_port ."',domain_rrd_path='". $domain_rrd_path ."',domain_las_admin_path='". $domain_las_admin_path ."',domain_las_local_path='". $domain_las_local_path ."',domain_sql_string='". $domain_sql_string ."',domain_hd_check=". $domain_hd_check .",domain_mfs_web='". $domain_mfs_web ."',domain_cs_sql_string='". $domain_cs_sql_string ."' WHERE domain_id=". $domain_id;
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_shards_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." LEFT JOIN ". NELDB_DOMAIN_TABLE ." ON (shard_domain_id=domain_id) ORDER BY domain_name ASC, shard_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_shards_add($shard_name, $shard_as_id, $shard_domain_id, $shard_language)
	{
		global $db;

		$shard_name = trim($shard_name);
		if ($shard_name == '')	return "/!\ Error: shard name is empty!";

		$shard_as_id = trim($shard_as_id);
		//if (!is_numeric($shard_as_id))	return "/!\ Error: shard AS Id is invalid!";

		//$shard_exists = tool_admin_shards_name_exist($shard_as_id);
		//if (!$shard_exists)
		//{
			$sql  = "INSERT INTO ". NELDB_SHARD_TABLE;
			$sql .= " (`shard_name`,`shard_as_id`,`shard_domain_id`,`shard_lang`) ";
			$sql .= " VALUES ";
			$sql .= " ('". $shard_name ."','". $shard_as_id ."','". $shard_domain_id ."','". $shard_language ."') ";
			$db->sql_query($sql);
			return "";
		//}
        //
		//return "/!\ Error: shard AS Id already exists in the database!";
	}

	function tool_admin_shards_name_exist($shard_as_id, $except_id=false)
	{
		global $db;

		if ($shard_as_id == '*' || $shard_as_id == '?') return false;

		$exists = false;

		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." WHERE shard_as_id='". $shard_as_id ."'";
		if ($except_id !== false)	$sql .= " AND shard_id<>". $except_id;

		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$exists = true;
			}
		}

		return $exists;
	}

	function tool_admin_shards_get_id($shard_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." WHERE shard_id=". $shard_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_shards_del($shard_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_SHARD_TABLE ." WHERE user_shard_shard_id=". $shard_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_GROUP_SHARD_TABLE ." WHERE group_shard_shard_id=". $shard_id;
		$db->sql_query($sql);

		$sql = "DELETE FROM ". NELDB_SHARD_TABLE ." WHERE shard_id=". $shard_id;
		$db->sql_query($sql);
	}

	function tool_admin_shards_update($shard_id, $shard_name, $shard_as_id, $shard_domain_id, $shard_language)
	{
		global $db;

		$shard_name = trim($shard_name);
		if ($shard_name == '')	return "/!\ Error: shard name is empty!";

		$shard_as_id = trim($shard_as_id);
		//if (!is_numeric($shard_as_id))	return "/!\ Error: shard AS Id is invalid!";

		//$shard_exists = tool_admin_shards_name_exist($shard_as_id, $shard_id);
		//if (!$shard_exists)
		//{
			$sql = "UPDATE ". NELDB_SHARD_TABLE ." SET shard_name='". $shard_name ."',shard_as_id='". $shard_as_id ."',shard_domain_id='". $shard_domain_id ."',shard_lang='". $shard_language ."' WHERE shard_id=". $shard_id;
			$db->sql_query($sql);

			return "";
		//}
        //
		//return "/!\ Error: shard AS Id already exists in the database!";

	}


	function tool_admin_users_domains_update($user_id, $group_id, $domain_ids)
	{
		global $db;

		$user_domains	= tool_admin_users_domains_get_list($user_id, true);
		$group_domains	= tool_admin_groups_domains_get_list($group_id, true);

		$sql = "DELETE FROM ". NELDB_USER_DOMAIN_TABLE ." WHERE user_domain_user_id=". $user_id;
		$db->sql_query($sql);

		if (is_array($domain_ids) and sizeof($domain_ids))
		{
			reset($domain_ids);
			foreach($domain_ids as $domain_id)
			{
				if (is_numeric($domain_id) && $domain_id > 0)
				{
					$sql = "INSERT INTO ". NELDB_USER_DOMAIN_TABLE ." (`user_domain_user_id`,`user_domain_domain_id`) VALUES ('". $user_id ."','". $domain_id ."')";
					$db->sql_query($sql);
				}
			}
		}

		// now we remove all shards except those that belong to the user AND group

		$sql  = "DELETE FROM ". NELDB_USER_SHARD_TABLE ." WHERE user_shard_user_id=". $user_id;
		if (is_array($domain_ids)    && sizeof($domain_ids)) 	$sql .= " AND user_shard_domain_id NOT IN (". implode(',',array_values($domain_ids)) .")";
		if (is_array($group_domains) && sizeof($group_domains))	$sql .= " AND user_shard_domain_id NOT IN (". implode(',',array_values($group_domains)) .")";
		$db->sql_query($sql);
	}

	function tool_admin_users_domains_get_list($user_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_USER_DOMAIN_TABLE ." LEFT JOIN ". NELDB_DOMAIN_TABLE ." ON (user_domain_domain_id=domain_id) WHERE user_domain_user_id=". $user_id ." ORDER BY domain_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['user_domain_domain_id'];
			}
		}
		else
		{
			$data = $data1;
		}



		return $data;
	}

	function tool_admin_groups_domains_get_list($group_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_GROUP_DOMAIN_TABLE ." LEFT JOIN ". NELDB_DOMAIN_TABLE ." ON (group_domain_domain_id=domain_id) WHERE group_domain_group_id=". $group_id ." ORDER BY domain_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['group_domain_domain_id'];
			}
		}
		else
		{
			$data = $data1;
		}

		//nt_common_add_debug($data);

		return $data;
	}

	function tool_admin_users_domains_merge($domain_list, $user_list, $group_list)
	{
		$data = array();

		if (is_array($domain_list) && sizeof($domain_list))
		{
			reset($domain_list);
			foreach($domain_list as $domain)
			{
				if (in_array($domain['domain_id'], $group_list))
				{
					$domain['domain_disabled']	= true;
					$domain['domain_visible']	= true;
				}
				elseif (in_array($domain['domain_id'], $user_list))
				{
					$domain['domain_selected'] 	= true;
					$domain['domain_visible']	= true;
				}

				$data[] = $domain;
			}
		}

		return $data;
	}

	function tool_admin_users_shards_get_list($user_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_USER_SHARD_TABLE ." LEFT JOIN ". NELDB_SHARD_TABLE ." ON (user_shard_shard_id=shard_id) WHERE user_shard_user_id=". $user_id ." ORDER BY shard_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['user_shard_shard_id'];
			}
		}
		else
		{
			$data = $data1;
		}



		return $data;
	}

	function tool_admin_groups_shards_get_list($group_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_GROUP_SHARD_TABLE ." LEFT JOIN ". NELDB_SHARD_TABLE ." ON (group_shard_shard_id=shard_id) WHERE group_shard_group_id=". $group_id ." ORDER BY shard_name ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['group_shard_shard_id'];
			}
		}
		else
		{
			$data = $data1;
		}

		return $data;
	}

	function tool_admin_users_shards_merge($domain_list, $shard_list, $user_list, $group_list)
	{
		$data1 = array();

		if (is_array($shard_list) && sizeof($shard_list))
		{
			reset($shard_list);
			foreach($shard_list as $shard)
			{
				if (in_array($shard['shard_id'], $group_list))
				{
					$shard['shard_disabled'] 	= true;
				}
				elseif (in_array($shard['shard_id'], $user_list))
				{
					$shard['shard_selected']	= true;
				}

				$data1[] = $shard;
			}

			$shard_list = $data1;
		}

		$data2 = array();

		// now we sort the shards by their domain
		if (is_array($domain_list) && sizeof($domain_list))
		{
			reset($domain_list);
			foreach($domain_list as $domain)
			{
				// looks for the shards that belong to this domain
				reset($shard_list);
				foreach($shard_list as $shard)
				{
					if ($domain['domain_id'] == $shard['shard_domain_id'])
					{
						$domain['shard_list'][] = $shard;
					}
				}

				$data2[] = $domain;
			}

			$domain_list = $data2;
		}

		return $domain_list;
	}

	function tool_admin_users_shards_update($user_id, $group_id, $shard_ids)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_SHARD_TABLE ." WHERE user_shard_user_id=". $user_id;
		$db->sql_query($sql);

		if (is_array($shard_ids) && sizeof($shard_ids))
		{
			reset($shard_ids);
			foreach($shard_ids as $shard_tmp)
			{
				$tmp = explode('_', $shard_tmp);
				$domain_id	= $tmp[0];
				$shard_id	= $tmp[1];

				if (is_numeric($domain_id) && is_numeric($shard_id) && $domain_id > 0 && $shard_id > 0)
				{
					$sql = "INSERT INTO ". NELDB_USER_SHARD_TABLE ." (`user_shard_user_id`,`user_shard_shard_id`,`user_shard_domain_id`) VALUES ('". $user_id ."','". $shard_id ."','". $domain_id ."')";
					$db->sql_query($sql);
				}
			}
		}
	}

	function tool_admin_groups_domains_update($group_id, $domain_ids)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_GROUP_DOMAIN_TABLE ." WHERE group_domain_group_id=". $group_id;
		$db->sql_query($sql);

		if (is_array($domain_ids) and sizeof($domain_ids))
		{
			reset($domain_ids);
			foreach($domain_ids as $domain_id)
			{
				if (is_numeric($domain_id) && $domain_id > 0)
				{
					$sql = "INSERT INTO ". NELDB_GROUP_DOMAIN_TABLE ." (`group_domain_group_id`,`group_domain_domain_id`) VALUES ('". $group_id ."','". $domain_id ."')";
					$db->sql_query($sql);
				}
			}
		}

		$sql  = "DELETE FROM ". NELDB_GROUP_SHARD_TABLE ." WHERE group_shard_group_id=". $group_id;
		if (is_array($domain_ids)    && sizeof($domain_ids)) 	$sql .= " AND group_shard_domain_id NOT IN (". implode(',',array_values($domain_ids)) .")";
		$db->sql_query($sql);

		// we need to check some stuff for each user in this group

		// first we get the list of users that belong to his group
		$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				while ($row = $db->sql_fetchrow($result))
				{
					// get a user
					$user_id = $row['user_id'];

					// get the domains specific to the user
					$user_domain_list = tool_admin_users_domains_get_list($user_id, true);

					// then we delete the shard that don't belong to the group nor user
					$sql  = "DELETE FROM ". NELDB_USER_SHARD_TABLE ." WHERE user_shard_user_id=". $user_id;
					if (is_array($domain_ids) && sizeof($domain_ids)) 				$sql .= " AND user_shard_domain_id NOT IN (". implode(',', array_values($domain_ids)) .")";
					if (is_array($user_domain_list) && sizeof($user_domain_list))	$sql .= " AND user_shard_domain_id NOT IN (". implode(',', array_values($user_domain_list)) .")";
					$db->sql_query($sql);

					// make sure users don't have a domain that already belongs to a group
					$sql  = "DELETE FROM ". NELDB_USER_DOMAIN_TABLE ." WHERE user_domain_user_id=". $user_id;
					if (is_array($domain_ids) && sizeof($domain_ids))				$sql .= " AND user_domain_domain_id IN (". implode(',', array_values($domain_ids)) .")";
					$db->sql_query($sql);
				}
			}
		}

	}

	function tool_admin_groups_domains_merge($domain_list, $group_list)
	{
		$data = array();

		if (is_array($domain_list) && sizeof($domain_list))
		{
			reset($domain_list);
			foreach($domain_list as $domain)
			{
				if (in_array($domain['domain_id'], $group_list))
				{
					$domain['domain_visible']	= true;
					$domain['domain_selected']	= true;
				}

				$data[] = $domain;
			}
		}

		return $data;
	}

	function tool_admin_groups_shards_merge($domain_list, $shard_list, $group_list)
	{
		$data1 = array();

		if (is_array($shard_list) && sizeof($shard_list))
		{
			reset($shard_list);
			foreach($shard_list as $shard)
			{
				if (in_array($shard['shard_id'], $group_list))
				{
					$shard['shard_selected'] 	= true;
				}

				$data1[] = $shard;
			}

			$shard_list = $data1;
		}

		$data2 = array();

		// now we sort the shards by their domain
		if (is_array($domain_list) && sizeof($domain_list) && is_array($shard_list) && sizeof($shard_list))
		{
			reset($domain_list);
			foreach($domain_list as $domain)
			{
				// looks for the shards that belong to this domain
				reset($shard_list);
				foreach($shard_list as $shard)
				{
					if ($domain['domain_id'] == $shard['shard_domain_id'])
					{
						$domain['shard_list'][] = $shard;
					}
				}

				$data2[] = $domain;
			}

			$domain_list = $data2;
		}

		return $domain_list;
	}

	function tool_admin_groups_shards_update($group_id, $shard_ids)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_GROUP_SHARD_TABLE ." WHERE group_shard_group_id=". $group_id;
		$db->sql_query($sql);

		if (is_array($shard_ids) && sizeof($shard_ids))
		{
			$user_shard_ids = array();

			reset($shard_ids);
			foreach($shard_ids as $shard_tmp)
			{
				$tmp = explode('_', $shard_tmp);
				$domain_id	= $tmp[0];
				$shard_id	= $tmp[1];

				$group_shard_ids[] = $shard_id;

				if (is_numeric($domain_id) && is_numeric($shard_id) && $domain_id > 0 && $shard_id > 0)
				{
					$sql = "INSERT INTO ". NELDB_GROUP_SHARD_TABLE ." (`group_shard_group_id`,`group_shard_shard_id`,`group_shard_domain_id`) VALUES ('". $group_id ."','". $shard_id ."','". $domain_id ."')";
					$db->sql_query($sql);
				}
			}

			// we need to check some stuff for each user in this group

			// first we get the list of users that belong to his group
			$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_group_id=". $group_id;
			if ($result = $db->sql_query($sql))
			{
				if ($db->sql_numrows($result))
				{
					while ($row = $db->sql_fetchrow($result))
					{
						// get a user
						$user_ids[] = $row['user_id'];
					}

					$sql = "DELETE FROM ". NELDB_USER_SHARD_TABLE ." WHERE user_shard_user_id IN (". implode(',',array_values($user_ids)) .") AND user_shard_shard_id IN (". implode(',', array_values($group_shard_ids)) .")";
					$db->sql_query($sql);
				}
			}

		}
	}

	function tool_admin_groups_applications_update($group_id, $application_ids)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_GROUP_APPLICATION_TABLE ." WHERE group_application_group_id=". $group_id;
		$db->sql_query($sql);

		if (is_array($application_ids) && sizeof($application_ids))
		{
			reset($application_ids);
			foreach($application_ids as $application_id)
			{
				$sql = "INSERT INTO ". NELDB_GROUP_APPLICATION_TABLE ." (`group_application_group_id`,`group_application_application_id`) VALUES ('". $group_id ."','". $application_id ."')";
				$db->sql_query($sql);
			}

			// we need to make sure no user in this group has this application
			$sql = "SELECT * FROM ". NELDB_USER_TABLE ." WHERE user_group_id=". $group_id;
			if ($result = $db->sql_query($sql))
			{
				if ($db->sql_numrows($result))
				{
					while ($row = $db->sql_fetchrow($result))
					{
						// get a user
						$user_ids[] = $row['user_id'];
					}

					$sql = "DELETE FROM ". NELDB_USER_APPLICATION_TABLE ." WHERE user_application_user_id IN (". implode(',',array_values($user_ids)) .") AND user_application_application_id IN (". implode(',', array_values($application_ids)) .")";
					$db->sql_query($sql);
				}
			}

		}
	}


	function tool_admin_groups_applications_get_list($group_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_GROUP_APPLICATION_TABLE ." LEFT JOIN ". NELDB_APPLICATION_TABLE ." ON (group_application_application_id=application_id) WHERE group_application_group_id=". $group_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['group_application_application_id'];
			}
		}
		else
		{
			$data = $data1;
		}

		return $data;
	}

	function tool_admin_groups_applications_merge($appl_list, $group_list)
	{
		$data1 = array();

		if (is_array($appl_list) && sizeof($appl_list))
		{
			reset($appl_list);
			foreach($appl_list as $appl)
			{
				if (in_array($appl['application_id'], $group_list))
				{
					$appl['application_selected'] = true;
				}

				$data1[] = $appl;
			}

			$appl_list = $data1;
		}

		return $appl_list;
	}

	function tool_admin_users_applications_get_list($user_id, $compact = false)
	{
		global $db;

		$data = array();
		$data1 = array();

		$sql = "SELECT * FROM ". NELDB_USER_APPLICATION_TABLE ." LEFT JOIN ". NELDB_APPLICATION_TABLE ." ON (user_application_application_id=application_id) WHERE user_application_user_id=". $user_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data1 = $db->sql_fetchrowset($result);
			}
		}

		if ($compact)
		{
			reset($data1);
			foreach($data1 as $data_tmp)
			{
				$data[] = $data_tmp['user_application_application_id'];
			}
		}
		else
		{
			$data = $data1;
		}

		return $data;
	}

	function tool_admin_users_applications_merge($appl_list, $user_list, $group_list)
	{
		$data1 = array();

		if (is_array($appl_list) && sizeof($appl_list))
		{
			reset($appl_list);
			foreach($appl_list as $appl)
			{
				if (in_array($appl['application_id'], $group_list))
				{
					$appl['application_disabled'] = true;
				}
				elseif (in_array($appl['application_id'], $user_list))
				{
					$appl['application_selected'] = true;
				}

				$data1[] = $appl;
			}

			$appl_list = $data1;
		}

		return $appl_list;
	}

	function tool_admin_users_applications_update($user_id, $application_ids)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_USER_APPLICATION_TABLE ." WHERE user_application_user_id=". $user_id;
		$db->sql_query($sql);

		if (is_array($application_ids) && sizeof($application_ids))
		{
			reset($application_ids);
			foreach($application_ids as $application_id)
			{
				if (is_numeric($application_id) && $application_id > 0)
				{
					$sql = "INSERT INTO ". NELDB_USER_APPLICATION_TABLE ." (`user_application_user_id`,`user_application_application_id`) VALUES ('". $user_id ."','". $application_id ."')";
					$db->sql_query($sql);
				}
			}
		}
	}

	function tool_admin_applications_build_menu_list($user_access)
	{
		$menu = array();

//		return $user_access['applications'];

		if (is_array($user_access['applications']) && sizeof($user_access['applications']))
		{
			reset($user_access['applications']);
			foreach($user_access['applications'] as $appl_data)
			{
				if ($appl_data['application_visible'] == 1)
				{
					$user_check 	= tool_admin_applications_build_menu_check($appl_data['application_id'], $user_access['user_applications'], 'user_application_application_id');
					$group_check	= tool_admin_applications_build_menu_check($appl_data['application_id'], $user_access['group_applications'], 'group_application_application_id');

					if ($appl_data['application_restriction'] == '')
					{
						$menu[] = $appl_data;
					}
					elseif ($user_check || $group_check)
					{
						$menu[] = $appl_data;
					}
				}
			}
		}

		return $menu;
	}

	function tool_admin_applications_get_default($data, $application_id)
	{
		if (is_array($data) && sizeof($data))
		{
			reset($data);
			foreach($data as $dt)
			{
				if ($dt['application_id'] == $application_id)
				{
					return $dt;
				}
			}
		}
		return false;
	}

	function tool_admin_applications_build_menu_check($appl_id, $data, $data_name)
	{
		if (is_array($data) && sizeof($data))
		{
			reset($data);
			foreach($data as $dt)
			{
				if ($dt[$data_name] == $appl_id)
				{
					return true;
				}
			}
		}

		return false;
	}

	function tool_admin_applications_check($appl)
	{
		global $nel_user;

		$ua = $nel_user['access']['user_applications'];
		$ga = $nel_user['access']['group_applications'];

		if (is_array($ua) && sizeof($ua))
		{
			reset($ua);
			foreach($ua as $a)
			{
				if ($a['application_restriction'] == $appl)
				{
					return true;
				}
			}
		}

		if (is_array($ga) && sizeof($ga))
		{
			reset($ga);
			foreach($ga as $a)
			{
				if ($a['application_restriction'] == $appl)
				{
					return true;
				}
			}
		}

		return false;
	}

	function tool_admin_menu_get_list($ie)
	{
		global $tool_admin_menu;
		global $nel_user;

		$new_menu = array();

		reset($tool_admin_menu);
		foreach($tool_admin_menu as $menu_item)
		{
			if (($menu_item['access'] == '') || tool_admin_applications_check($menu_item['access']))
			{
				if ($ie === false)
				{
					$new_menu[] = $menu_item;
				}
			}
		}

		return $new_menu;
	}

	function tool_admin_users_groups_domains_merge()
	{
		global $nel_user;

		$user_domains = array();

		$ud = $nel_user['access']['user_domains'];
		$gd = $nel_user['access']['group_domains'];

		$dd = tool_admin_domains_get_list();

		if (is_array($dd) && sizeof($dd))
		{
			reset($dd);
			foreach($dd as $domain_item)
			{
				if (is_array($ud))
				{
					reset($ud);
					foreach($ud as $udomain)
					{
						if ($domain_item['domain_id'] == $udomain['domain_id'])
						{
							$user_domains[] = $domain_item;
						}
					}
				}

				if (is_array($gd))
				{
					reset($gd);
					foreach($gd as $gdomain)
					{
						if ($domain_item['domain_id'] == $gdomain['domain_id'])
						{
							$user_domains[] = $domain_item;
						}
					}
				}
			}
		}

		return $user_domains;
	}

	function tool_admin_users_groups_shards_merge()
	{
		global $nel_user;

		$user_shards = array();

		$us = $nel_user['access']['user_shards'];
		$gs = $nel_user['access']['group_shards'];

		$ss = tool_admin_shards_get_list();

		if (is_array($ss) && sizeof($ss))
		{
			reset($ss);
			foreach($ss as $shard_item)
			{
				if (is_array($us))
				{
					reset($us);
					foreach($us as $ushard)
					{
						if ($shard_item['shard_id'] == $ushard['shard_id'])
						{
							$user_shards[] = $shard_item;
						}
					}
				}

				if (is_array($gs))
				{
					reset($gs);
					foreach($gs as $gshard)
					{
						if ($shard_item['shard_id'] == $gshard['shard_id'])
						{
							$user_shards[] = $shard_item;
						}
					}
				}
			}
		}

		return $user_shards;
	}

	function tool_admin_logs_get_count()
	{
		global $db;

		$total = 0;

		$sql = "SELECT * FROM ". NELDB_LOG_TABLE;
		if ($result = $db->sql_query($sql))
		{
			$total = $db->sql_numrows($result);
		}

		return $total;
	}

	function tool_admin_logs_get_list($start=0,$limit=25)
	{
		global $db;

		$data = array();

		$sql = "SELECT * FROM ". NELDB_LOG_TABLE ." ORDER BY logs_id DESC LIMIT ". $start .",". $limit;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_domains_get_nel($application)
	{
		global $db;

		$domain_data = null;

		if ($db->sql_select_db('nel'))
		{
			$sql = "SELECT * FROM domain WHERE domain_name='". $application ."'";
			if ($result = $db->sql_query($sql))
			{
				if ($db->sql_numrows($result))
				{
					$domain_data = $db->sql_fetchrow($result);
				}
			}

			$db->sql_reselect_db();
		}

		return $domain_data;
	}

	//function tool_admin_domains_update_nel($domain_id, $domain_name, $domain_version, $domain_status)
	function tool_admin_domains_update_nel($domain_id, $domain_name, $domain_status)
	{
		global $db;

		if ($db->sql_select_db('nel'))
		{
			//$sql = "UPDATE domain SET status='". $domain_status ."',patch_version=". $domain_version ." WHERE domain_id=". $domain_id ." AND domain_name='". $domain_name ."'";
			$sql = "UPDATE domain SET status='". $domain_status ."' WHERE domain_id=". $domain_id ." AND domain_name='". $domain_name ."'";
			$db->sql_query($sql);

			$db->sql_reselect_db();
		}
	}

	function tool_admin_restarts_get_list($order='ASC')
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_GROUP_TABLE ." ORDER BY restart_group_order ". $order .", restart_group_name ". $order;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_restarts_add($restart_name, $restart_list, $restart_order)
	{
		global $db;

		$restart_name = trim($restart_name);
		if ($restart_name == '')	return "/!\ Error: restart group name is empty!";

		$restart_list = trim($restart_list);
		if ($restart_list == '')	return "/!\ Error: restart group list is empty!";

		$restart_order = trim($restart_order);
		if (!is_numeric($restart_order)) return "/!\ Error: restart group order is not numeric!";

		$sql  = "INSERT INTO ". NELDB_RESTART_GROUP_TABLE;
		$sql .= " (`restart_group_id`,`restart_group_name`,`restart_group_list`,`restart_group_order`) ";
		$sql .= " VALUES ";
		$sql .= " (0,'". $restart_name ."','". $restart_list ."','". $restart_order ."') ";
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_restarts_get_id($restart_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_GROUP_TABLE ." WHERE restart_group_id=". $restart_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_restarts_del($restart_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_RESTART_GROUP_TABLE ." WHERE restart_group_id=". $restart_id;
		$db->sql_query($sql);
	}

	function tool_admin_restarts_update($restart_id, $restart_name, $restart_list, $restart_order)
	{
		global $db;

		$restart_name = trim($restart_name);
		if ($restart_name == '')	return "/!\ Error: restart group name is empty!";

		$restart_list = trim($restart_list);
		if ($restart_list == '')	return "/!\ Error: restart group list is empty!";

		$restart_order = trim($restart_order);
		if (!is_numeric($restart_order)) return "/!\ Error: restart group order is not numeric!";

		$sql = "UPDATE ". NELDB_RESTART_GROUP_TABLE ." SET restart_group_name='". $restart_name ."',restart_group_list='". $restart_list ."',restart_group_order='". $restart_order ."' WHERE restart_group_id=". $restart_id;
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_restart_messages_add($message_name, $message_value, $message_lang)
	{
		global $db;

		$message_name = trim($message_name);
		if ($message_name == '')	return "/!\ Error: restart message name is empty!";

		$message_value = trim($message_value);
		if ($message_value == '')	return "/!\ Error: restart message value is empty!";

		$sql  = "INSERT INTO ". NELDB_RESTART_MESSAGE_TABLE;
		$sql .= " (`restart_message_id`,`restart_message_name`,`restart_message_value`,`restart_message_lang`) ";
		$sql .= " VALUES ";
		$sql .= " (0,'". $message_name ."','". $message_value ."','". $message_lang ."') ";
		$db->sql_query($sql);

		return "";
	}

	function tool_admin_restart_messages_get_list()
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_MESSAGE_TABLE ." ORDER BY restart_message_name ASC, restart_message_lang ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_restart_messages_get_id($message_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_MESSAGE_TABLE ." WHERE restart_message_id=". $message_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_admin_restart_messages_get_list_from_name($message_name,$lang=null)
	{
		global $db;

		$data = null;

		$sql_ext = '';
		if ($lang !== null)
		{
			$sql_ext = " AND restart_message_lang='". $lang ."'";
		}

		$sql = "SELECT * FROM ". NELDB_RESTART_MESSAGE_TABLE ." WHERE restart_message_name='". $message_name ."' ". $sql_ext ." ORDER BY restart_message_lang ASC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrowset($result);
			}
		}

		return $data;
	}

	function tool_admin_restart_messages_del($message_id)
	{
		global $db;

		$sql = "DELETE FROM ". NELDB_RESTART_MESSAGE_TABLE ." WHERE restart_message_id=". $message_id;
		$db->sql_query($sql);
	}

	function tool_admin_restart_messages_update($message_id, $message_name, $message_value, $message_lang)
	{
		global $db;

		$message_name = trim($message_name);
		if ($message_name == '')	return "/!\ Error: restart message name is empty!";

		$message_value = trim($message_value);
		if ($message_value == '')	return "/!\ Error: restart message value is empty!";

		$sql = "UPDATE ". NELDB_RESTART_MESSAGE_TABLE ." SET restart_message_name='". $message_name ."',restart_message_value='". $message_value ."',restart_message_lang='". $message_lang ."' WHERE restart_message_id=". $message_id;
		$db->sql_query($sql);

		return "";
	}


?>