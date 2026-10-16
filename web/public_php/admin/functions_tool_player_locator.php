<?php

	function tool_pl_parse_relocate($data)
	{
		$entity_data = array();

		reset($data);
		foreach($data as $su_data)
		{
			$service_name = 'n/a';

			reset($su_data);
			foreach($su_data as $su_line)
			{
				$su_line = trim($su_line);

				if (ereg("^===\[ Service ([^\ ]+) returned \]===$", $su_line, $eregs))
				{
					$service_name = $eregs[1];
				}
				elseif (ereg("^The character ([^\ ]+) of user ([^\ ]+) has been relocated on shard ([^\ ]+)$", $su_line, $eregs))
				{
					$entity_data = array(	'service'		=>	$service_name,
											'uid'			=>	$eregs[2],
											'charid'		=>	$eregs[1],
											'shardid'		=>	$eregs[3],
											'success'		=>	1,
											);
				}
				elseif (ereg("^Can't reloc character because user ([^\ ]+) owner of char ([^\ ]+) is online$", $su_line, $eregs))
				{
					$entity_data = array(	'service'		=>	$service_name,
											'uid'			=>	$eregs[1],
											'charid'		=>	$eregs[2],
											'shardid'		=>	'na',
											'success'		=>	0,
											);
				}
			}
		}


		return $entity_data;
	}

	function tool_pl_parse_locate($data)
	{
		$entity_data = array();

		reset($data);
		foreach($data as $egs_data)
		{
			$service_name = 'n/a';

			reset($egs_data);
			foreach($egs_data as $egs_line)
			{
				$egs_line = trim($egs_line);
				if (ereg("^===\[ Service ([^\ ]+) returned \]===$", $egs_line, $eregs))
				{
					$service_name = $eregs[1];
				}
				elseif (ereg("^UId ([^\ ]+) UserName '([^\ ]*)' EId ([^\ ]+) EntityName '([^\ ]*)' EntitySlot ([^\ ]+) SaveFile ([^\ ]+) ([^\ ]+)$", $egs_line, $eregs))
				{
					$entity_data[] = array(	'service'		=>	$service_name,
											'uid'			=>	$eregs[1],
											'user_name'		=>	$eregs[2],
											'eid'			=>	$eregs[3],
											'entity_name'	=>	$eregs[4],
											'entity_slot'	=>	$eregs[5],
											'save_file'		=>	$eregs[6],
											'status'		=>	$eregs[7],
											'posx'			=>	'n/a',
											'posy'			=>	'n/a',
											'posz'			=>	'n/a',
											'session'		=>	'n/a',
											);
				}
				elseif (ereg("^UId ([^\ ]+) UserName '([^\ ]*)' EId ([^\ ]+) EntityName '([^\ ]*)' EntitySlot ([^\ ]+) SaveFile ([^\ ]+) Pos: ([^,\ ]+),([^,\ ]+),([^,\ ]+) Session: ([^\ ]+) ([^\ ]+)$", $egs_line, $eregs))
				{
					$entity_data[] = array(	'service'		=>	$service_name,
											'uid'			=>	$eregs[1],
											'user_name'		=>	$eregs[2],
											'eid'			=>	$eregs[3],
											'entity_name'	=>	$eregs[4],
											'entity_slot'	=>	$eregs[5],
											'save_file'		=>	$eregs[6],
											'status'		=>	$eregs[11],
											'posx'			=>	$eregs[7],
											'posy'			=>	$eregs[8],
											'posz'			=>	$eregs[9],
											'session'		=>	$eregs[10],
											);
				}
			}
		}

		return $entity_data;
	}

	function tool_pl_parse_display_players($data)
	{
		$entity_data = array();

		reset($data);
		foreach($data as $egs_data)
		{
			$service_name = 'n/a';

			reset($egs_data);
			foreach($egs_data as $egs_line)
			{
				$egs_line = trim($egs_line);
				if (ereg("^===\[ Service ([^\ ]+) returned \]===$", $egs_line, $eregs))
				{
					$service_name = $eregs[1];
				}
				elseif (ereg("^Player: ([^\ ]+) Name: ([^\ ]+) ID: ([^\ ]+) FE: ([^\ ]+) Sheet: ([^\ ]+) - ([^\ ]+) Priv: '([^\ ]*)'$", $egs_line, $eregs))
				{
					$entity_data[] = array(	'service'		=>	$service_name,
											'player'		=>	$eregs[1],
											'name'			=>	$eregs[2],
											'id'			=>	$eregs[3],
											'fe'			=>	$eregs[4],
											'sheet'			=>	$eregs[5] .' - '. $eregs[6],
											'priv'			=>	$eregs[7],
											'posx'			=>	'n/a',
											'posy'			=>	'n/a',
											'posz'			=>	'n/a',
											'session'		=>	'n/a',
											);
				}
				elseif (ereg("^Player: ([^\ ]+) Name: ([^\ ]+) ID: ([^\ ]+) FE: ([^\ ]+) Sheet: ([^\ ]+) - ([^\ ]+) Priv: '([^\ ]*)' Pos: ([^,\ ]+),([^,\ ]+),([^,\ ]+) Session: ([^\ ]+)$", $egs_line, $eregs))
				{
					$entity_data[] = array(	'service'		=>	$service_name,
											'player'		=>	$eregs[1],
											'name'			=>	$eregs[2],
											'id'			=>	$eregs[3],
											'fe'			=>	$eregs[4],
											'sheet'			=>	$eregs[5] .' - '. $eregs[6],
											'priv'			=>	$eregs[7],
											'posx'			=>	$eregs[8],
											'posy'			=>	$eregs[9],
											'posz'			=>	$eregs[10],
											'session'		=>	$eregs[11],
											);
				}
			}
		}

		return $entity_data;
	}

	function tool_pl_get_character_check_list($AS_Application, $AS_RingSQL)
	{
		global $db;

		$data = null;

		if ($AS_RingSQL == '')
		{
			if ($db->sql_select_db('nel'))
			{
				$sql = "SELECT ring_db_name FROM domain WHERE domain_name='". $AS_Application ."'";
				if ($resutl = $db->sql_query($sql))
				{
					if ($db->sql_numrows($result))
					{
						$domain_row = $db->sql_fetchrow($result);

						if ($db->sql_select_db($domain_row['ring_db_name']))
						{
							$sql = "SELECT * FROM characters WHERE user_id=0";
							if ($result2 = $db->sql_query($sql))
							{
								if ($db->sql_numrows($result2))
								{
									$data = $db->sql_fetchrowset($result2);
								}
							}
						}
					}
				}

				$db->sql_reselect_db();
			}
		}
		else
		{
			$db2 = new sql_db_string($AS_RingSQL);
			if (is_object($db2))
			{
				$sql = "SELECT * FROM characters WHERE user_id=0";
				if ($result2 = $db2->sql_query($sql))
				{
					if ($db2->sql_numrows($result2))
					{
						$data = $db2->sql_fetchrowset($result2);
					}
				}
			}
		}

		return $data;
	}

	function tool_pl_fix_character_check_list($AS_Application)
	{
		global $db;

		if ($db->sql_select_db('nel'))
		{
			$sql = "SELECT ring_db_name FROM domain WHERE domain_name='". $AS_Application ."'";
			if ($resutl = $db->sql_query($sql))
			{
				if ($db->sql_numrows($result))
				{
					$domain_row = $db->sql_fetchrow($result);

					if ($db->sql_select_db($domain_row['ring_db_name']))
					{
						$sql = "UPDATE characters SET user_id=FLOOR(char_id/16) WHERE user_id=0";
						$db->sql_query($sql);
					}
				}
			}

			$db->sql_reselect_db();
		}
	}

?>