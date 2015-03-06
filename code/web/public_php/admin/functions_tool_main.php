<?php

	$refresh_rates = array( 
							array('desc' => 'Every 5 secs',
								  'secs' => 5,
								  ),
							array('desc' => 'Every 30 secs',
								  'secs' => 30,
								  ),
							array('desc' => 'Every 1 min.',
								  'secs' => 60,
								  ),
							array('desc' => 'Every 5 min.',
								  'secs' => 300,
								  ),
							array('desc' => 'Every 10 min.',
								  'secs' => 600,
								  ),
							array('desc' => 'Every 30 min.',
								  'secs' => 1800,
								  ),
							array('desc' => 'Never',
								  'secs' => 0,
								  ),
							);

	class MyAdminService extends CAdminServiceWeb
	{
		function commandResult($serviceModuleName, $result)
		{
			global $tpl;
			global $command_return_data;

			nt_common_add_debug("Service $serviceModuleName returned '$result'");

			$data = "===[ Service ". strtoupper($serviceModuleName) ." returned ]===\n". trim($result) ."\n\n";

			if (isset($command_return_data))	$command_return_data[] = explode("\n", $data);
			$tpl->append('tool_execute_result', $data);
		}

		function invokeError($methodName, $errorString)
		{
			global $tpl;

			nt_common_add_debug("AS Error in '$methodName' : $errorString");
			$tpl->assign('tool_as_error', "AS Error in '$methodName' : $errorString");
		}
	}


	function tool_main_check_user_domain($domain_id)
	{
		global $nel_user;

		if (!$domain_id) return false;

		$ud = $nel_user['access']['domains'];

		if (is_array($ud))
		{
			reset($ud);
			foreach($ud as $udomain)
			{
				if ($domain_id == $udomain['domain_id'])	return true;
			}
		}

		return false;
	}

	function tool_main_check_user_shard($shard_id)
	{
		global $nel_user;

		if (!$shard_id) return false;

		$us = $nel_user['access']['shards'];

		if (is_array($us))
		{
			reset($us);
			foreach($us as $ushard)
			{
				if ($shard_id == $ushard['shard_id'])	return true;
			}
		}

		return false;
	}

	function tool_main_get_domain_name($domain_id)
	{
		global $nel_user;

		reset($nel_user['access']['domains']);
		foreach($nel_user['access']['domains'] as $domain)
		{
			if ($domain['domain_id'] == $domain_id)
			{
				return $domain['domain_name'];
			}
		}

		return null;
	}

	function tool_main_get_domain_data($domain_id, $field)
	{
		global $nel_user;
		reset($nel_user['access']['domains']);
		foreach($nel_user['access']['domains'] as $domain)
		{
			if ($domain['domain_id'] == $domain_id)
			{
//		echo "ok $domain_id : $field : ".$domain[$field];
				return $domain[$field];
			}
		}
		return null;
	}


	function tool_main_get_domain_rrd_path($domain_id)
	{
		global $nel_user;

		reset($nel_user['access']['domains']);
		foreach($nel_user['access']['domains'] as $domain)
		{
			if ($domain['domain_id'] == $domain_id)
			{
				return $domain['domain_rrd_path'];
			}
		}

		return null;
	}

	function tool_main_get_shard_name($shard_id)
	{
		global $nel_user;

		reset($nel_user['access']['shards']);
		foreach($nel_user['access']['shards'] as $shard)
		{
			if ($shard['shard_id'] == $shard_id)
			{
				return $shard['shard_name'];
			}
		}

		return null;
	}

	function tool_main_get_shard_as_id($shard_id)
	{
		global $nel_user;

		reset($nel_user['access']['shards']);
		foreach($nel_user['access']['shards'] as $shard)
		{
			if ($shard['shard_id'] == $shard_id)
			{
				return $shard['shard_as_id'];
			}
		}

		return null;
	}

	function tool_main_get_shard_data($shard_id, $field)
	{
		global $nel_user;

		reset($nel_user['access']['shards']);
		foreach($nel_user['access']['shards'] as $shard)
		{
			if ($shard['shard_id'] == $shard_id)
			{
				return $shard[$field];
			}
		}

		return null;
	}


	function tool_main_get_domain_host($domain_id)
	{
		global $nel_user;

		reset($nel_user['access']['domains']);
		foreach($nel_user['access']['domains'] as $domain)
		{
			if ($domain['domain_id'] == $domain_id)
			{
				return $domain['domain_as_host'];
			}
		}

		return null;
	}

	function tool_main_get_domain_port($domain_id)
	{
		global $nel_user;

		reset($nel_user['access']['domains']);
		foreach($nel_user['access']['domains'] as $domain)
		{
			if ($domain['domain_id'] == $domain_id)
			{
				return $domain['domain_as_port'];
			}
		}

		return null;
	}

	function tool_main_get_elapsed_time_string($seconds, &$string)
	{
		//returns an array of numeric values representing days, hours, minutes & seconds respectively
		$ret=array('days'=>0,'hours'=>0,'minutes'=>0,'seconds'=>0,'totalseconds'=>0);

		$totalsec = $seconds;
		$ret['totalseconds'] = $totalsec;
		//print $earlierDate. ":". $laterDate. ":". $totalsec ."<br>";

		if ($totalsec >= 86400)
		{
			$ret['days'] = floor($totalsec/86400);
			$totalsec = $totalsec % 86400;
		}
		if ($totalsec >= 3600)
		{
			$ret['hours'] = floor($totalsec/3600);
			$totalsec = $totalsec % 3600;
		}
		if ($totalsec >= 60)
		{
			$ret['minutes'] = floor($totalsec/60);
		}
		$ret['seconds'] = $totalsec % 60;

		$string = '';
		$string .= ($ret['days'] > 0) ? tool_main_leftpad($ret['days']).'d ' : '';
		$string .= (($ret['hours'] > 0) || (strlen($string) > 0)) ? tool_main_leftpad($ret['hours']).'h ' : '';
		$string .= (($ret['minutes'] > 0) || (strlen($string) > 0)) ? tool_main_leftpad($ret['minutes']).'m ' : '';
		$string .= (($ret['seconds'] > 0) || (strlen($string) > 0)) ? tool_main_leftpad($ret['seconds']).'s ' : '';

		$ret['string'] = trim($string);

		return $ret;
	}

	function tool_main_leftpad($n,$cc=2,$ch='0')
	{
		return str_pad($n, $cc, $ch, STR_PAD_LEFT);
	}

	function tool_main_get_shards_from_status($status, $filters)
	{
		$result = array();

		if (is_array($status) && sizeof($status))
		{
			reset($status);
			foreach($status as $sline)
			{
				$shard_name = trim($sline['ShardName']);

				if (($shard_name != '') && (isset($filters[$shard_name]) || isset($filters['_all_'])))
				{
					$result[] = $shard_name;
				}
			}
			$result = array_values(array_unique($result));
		}

		return $result;
	}

	function tool_main_get_aes_from_status($status)
	{
		$result = array();
		if (is_array($status) && sizeof($status))
		{
			reset($status);
			foreach($status as $skey => $sline)
			{
				$short_name		= trim($sline['ShortName']);
				$running_state	= trim($sline['RunningState']);
				if (($short_name == 'AES') && ($running_state == 'online'))
				{
					$result[] = $sline['AliasName'];
				}
			}
		}
		return $result;
	}

	function tool_main_parse_status($status)
	{
		$check_graphs = tool_admin_applications_check('tool_main_graphs');

		$_sort_list = array();
		$domainServices = array();
		$sortedServices = array();
		if (is_array($status) && sizeof($status))
		{
			reset($status);
			foreach($status as $sline)
			{
				$vars = array();
				$vars['_flags_'] = array();

				$sline_vars = explode("\t", $sline);
				reset($sline_vars);
				foreach($sline_vars as $sline_var)
				{
					$sline_parts = explode("=", $sline_var);

					if ($sline_parts[0] == 'RunningState')
					{
						// this is a small fix to an unknown server bug :)
						if 		(trim($sline_parts[1]) == 'topped') $sline_parts[1] = 'rs_stopped';
						elseif	(trim($sline_parts[1]) == 'nline') $sline_parts[1] = 'rs_online';

						$vars['_flags_'][$sline_parts[1]] = true;
						$sline_parts[1] = substr($sline_parts[1], 3);
					}
					elseif ($sline_parts[0] == 'RunningOrders')
					{
						$vars['_flags_'][$sline_parts[1]] = true;
						$sline_parts[1] = substr($sline_parts[1], 3);
					}
					elseif ($sline_parts[0] == 'RunningTags')
					{
						$_tmp = explode(" ", trim($sline_parts[1]));
						reset($_tmp);
						foreach($_tmp as $_tmp_key => $_tmp_part)
						{
							$vars['_flags_'][$_tmp_part] = true;
							$_tmp_part			= str_replace('_',' ',substr($_tmp_part, 3));
							$_tmp[$_tmp_key]	= $_tmp_part;
						}
						$sline_parts[1] = implode(' / ', $_tmp);
					}
					elseif ($sline_parts[0] == 'NoReportSince')
					{
						$_today		= time();
						$_day		= date("d", $_today);
						$_month		= date("m", $_today);
						$_year		= date("Y", $_today);
						$_base_day	= mktime(0, 0, 0, $_month, $_day, $_year);

						if ($sline_parts[1] >= $_base_day)
						{
							$sline_parts[1] = 'n/a';
						}
						else
						{
							$time0 = 0 + $sline_parts[1]; // convert to a number ;)
							if 		($time0 > 60)	$vars['_flags_']['alert_red'] = true;
							elseif  ($time0 > 40)	$vars['_flags_']['alert_orange_dark'] = true;
							elseif  ($time0 > 25)	$vars['_flags_']['alert_orange_light'] = true;

							tool_main_get_elapsed_time_string($sline_parts[1],$sline_parts[1]);
						}
					}
					elseif ($sline_parts[0] == 'UpTime')
					{
						$_today		= time();
						$_day		= date("d", $_today);
						$_month		= date("m", $_today);
						$_year		= date("Y", $_today);
						$_base_day	= mktime(0, 0, 0, $_month, $_day, $_year);

						if ($sline_parts[1] >= $_base_day)
						{
							$sline_parts[1] = 'n/a';
						}
						else
						{
							tool_main_get_elapsed_time_string($sline_parts[1],$sline_parts[1]);
						}
					}
					elseif ($sline_parts[0] == 'State')
					{
						if (strtolower($sline_parts[1]) == 'stalled')		$vars['_flags_']['alert_red'] = true;
						elseif (strtolower($sline_parts[1]) == 'halted')	$vars['_flags_']['alert_red'] = true;
						$vars['_flags_'][$sline_parts[1]] = true;
					}
					elseif ($sline_parts[0] == 'Hostname')
					{
						$sline_parts[1] = substr($sline_parts[1], 0, strpos($sline_parts[1], '.'));
					}
					elseif ($sline_parts[0] == 'AliasName')
					{
						$vars['_flags_']['alias_code'] = '';
					}

					$vars[$sline_parts[0]] = $sline_parts[1];
				}

				// check is service is chain crashing
				if (in_array('rt_chain_crashing', array_keys($vars['_flags_'])))
				{
					// check is service is online (anything that is not stopped)
					if (!in_array('rs_stopped', array_keys($vars['_flags_'])))
					{
						// check the start counts for crashing age
						$crash_counts = explode(' ', $vars['StartCounter']);
						if     ($crash_counts[0] >= 5)	$vars['_flags_']['alert_red'] = true;
						elseif ($crash_counts[1] >= 5)	$vars['_flags_']['alert_orange_dark'] = true;
						elseif ($crash_counts[2] >= 5)	$vars['_flags_']['alert_orange_light'] = true;
					}
				}

				//$vars['_flags_']['has_graphs'] = tool_main_check_rrd_files($vars['AliasName']);
				$_sort_list[] = $vars['AliasName'];

				$domainServices[] = $vars;
			}

			sort($_sort_list);

			reset($_sort_list);
			foreach($_sort_list as $_sort_name)
			{
				reset($domainServices);
				foreach($domainServices as $serviceRow)
				{
					if ($_sort_name == $serviceRow['AliasName'])
					{
						$sortedServices[] = $serviceRow;
						break;
					}
				}
			}
		}

        return $sortedServices;
	}

	function tool_main_check_rrd_files($service)
	{
		if ($handle = opendir(NELTOOL_RRDBASE))
		{
   			while (false !== ($file = readdir($handle)))
   			{
       			if ($file != "." && $file != "..")
       			{
       				$file_parts = explode(".", $file);
       				if (sizeof($file_parts) == 3)
       				{
       					if (($service == $file_parts[0]) && (($file_parts[2] == 'rrd') || ($file_parts[2] == 'hrd')))
       					{
       						closedir($handle);
       						return true;
       					}
       				}
       			}
   			}
   			closedir($handle);
		}

		return false;
	}

	function tool_main_get_checked_services()
	{
		global $NELTOOL;

		$services = array();

		reset($NELTOOL['POST_VARS']);
		foreach($NELTOOL['POST_VARS'] as $post_key => $post_val)
		{
			$val = 'service_'. $post_val;
			if ($post_key == $val)
			{
				$services[] = $post_val;
			}
		}

		return $services;
	}

	function tool_main_get_shard_ids($shard_id)
	{
		global $nel_user;

		$data = array();
		reset($nel_user['access']['shards']);
		foreach($nel_user['access']['shards'] as $shards)
		{
			if ($shards['shard_id'] == $shard_id)
			{
				$shards_tmp = trim($shards['shard_as_id']);
				if ($shards_tmp == '*')
				{
					$data['_all_'] = true;
				}
				elseif ($shards_tmp == '?')
				{
					$data['_unknown_'] = true;
				}
				else
				{
					$shard_parts = explode(':', $shards_tmp);
					reset($shard_parts);
					foreach($shard_parts as $shard_as_id)
					{
						if ($shard_as_id == '?')
						{
							$data['_unknown_'] = true;
						}
						else
						{
							$data[$shard_as_id] = true;
						}
					}
				}

			}
		}

		return $data;
	}

	function tool_main_get_shards_orders($orders)
	{
		$data = array();

		if (is_array($orders) && sizeof($orders))
		{
			reset($orders);
			foreach($orders as $order_line)
			{
				$order_items = explode("\t", $order_line);

				$shard_name		= "";
				$shard_order	= "";

				reset($order_items);
				foreach($order_items as $order_parts)
				{
					$order_bits = explode("=", $order_parts);

					$order_bits[0] = trim($order_bits[0]);
					$order_bits[1] = trim($order_bits[1]);

					if ($order_bits[0] == 'ShardName')
					{
						$shard_name = $order_bits[1];
					}
					elseif ($order_bits[0] == 'Orders')
					{
						$shard_order = substr($order_bits[1],3);
					}
				}

				if ($shard_name != "" && $shard_order != "")
				{
					$data[$shard_name] = $shard_order;
				}

			}
		}

		return $data;
	}

	function tool_main_get_last_hd_time_for_domain($domain_id)
	{
		global $db;

		$timer = 0;

		$sql = "SELECT * FROM ". NELDB_STAT_HD_TIME_TABLE ." WHERE hd_domain_id=". $domain_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
				$timer = $data['hd_last_time'];
			}
		}

		return $timer;
	}

	function tool_main_update_hd_time_for_domain($domain_id, $now)
	{
		global $db;

		//if ($stat_time == 0)	$sql = "INSERT INTO ". NELDB_STAT_HD_TIME_TABLE ." (`hd_domain_id`,`hd_last_time`) VALUES (". $domain_id .",". $now .")";
		//else					$sql = "UPDATE ". NELDB_STAT_HD_TIME_TABLE ." SET hd_last_time=". $now ." WHERE hd_domain_id=". $domain_id;

		$sql = "SELECT * FROM ". NELDB_STAT_HD_TIME_TABLE ." WHERE hd_domain_id=". $domain_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$sql = "UPDATE ". NELDB_STAT_HD_TIME_TABLE ." SET hd_last_time=". $now ." WHERE hd_domain_id=". $domain_id;
				$db->sql_query($sql);
			}
			else
			{
				$sql = "INSERT INTO ". NELDB_STAT_HD_TIME_TABLE ." (`hd_domain_id`,`hd_last_time`) VALUES (". $domain_id .",". $now .")";
				$db->sql_query($sql);
			}
		}
	}

	function tool_main_update_hd_data_for_domain($domain_id, $data)
	{
		global $db;

		$aes_status = array();

		if (is_array($data) && sizeof($data))
		{
			reset($data);
			foreach($data as $aes)
			{
				$aes_server = null;

				//$hd_found_slash_mount		= null;
				//$hd_found_slash_home 		= null;
				//$hd_found_slash_home_nevrax = null;
				//$hd_found_slash 			= null;

				$aes_hd_ready	= false;

				$aes_data = explode("\n", $aes);
				reset($aes_data);
				foreach($aes_data as $aes_line)
				{
					$aes_line = trim($aes_line);

					// clean up the multiple blank characters
					$aes_line = ereg_replace("[[:space:]]+"," ",$aes_line);

					if (ereg("^===\[ Service AES_([^[:space:]]+) returned \]===$", $aes_line, $regs))
					{
						$aes_server = strtolower($regs[1]);
						$aes_status[$aes_server] = array();
					}
					elseif (ereg("^-* Command output begin -*$", $aes_line, $regs))
					{
						$aes_hd_ready = true;
					}
					elseif (ereg("^-* Command output end -*$", $aes_line, $regs))
					{
						$aes_hd_ready = false;
					}

					if ($aes_hd_ready)
					{
						$aes_line_data = explode(" ", $aes_line);

						if (($aes_line_data[0] != 'Filesystem') && ($aes_line_data[1] != 'Command'))
						{
							//if	((strpos($aes_line_data[5],"/mnt/rsbk") == 0)
							// ||	($aes_line_data[5] == "/home")
							// ||	($aes_line_data[5] == "/home/nevrax")
							// ||	($aes_line_data[5] == "/"))
							//{
								$aes_status[$aes_server][] = array(	'device'		=>	$aes_line_data[0],
																	'size'			=>	$aes_line_data[1],
																	'used'			=>	$aes_line_data[2],
																	'free'			=>	$aes_line_data[3],
																	'usedpercent'	=>	str_replace('%','',$aes_line_data[4]),
																	'mount'			=>	$aes_line_data[5],
																	);

							//}
						}

					}

					//if ((substr($aes_line, 0, 5) == "/dev/") && $aes_server)
					//{
                    //
					//	$aes_line_data = explode(" ", $aes_line);
                    //
					//	// 0 : device name
					//	// 1 : device size
					//	// 2 : used
					//	// 3 : available
					//	// 4 : used percentage
					//	// 5 : mount point
                    //
					//	if 	($aes_line_data[5] == "/home")
					//	{
					//		$hd_found_slash_home = $aes_line_data;
					//	}
                    //
					//	if	($aes_line_data[5] == "/home/nevrax")
					//	{
					//		$hd_found_slash_home_nevrax = $aes_line_data;
					//	}
                    //
					//	if	($aes_line_data[5] == "/")
					//	{
					//		$hd_found_slash = $aes_line_data;
					//	}
                    //
					//	//if (($aes_line_data[5] == "/") || ($aes_line_data[5] == "/home/nevrax"))
					//	//{
					//	//	$aes_status[$aes_server][] = array(	'device'		=>	$aes_line_data[0],
					//	//										'size'			=>	$aes_line_data[1],
					//	//										'used'			=>	$aes_line_data[2],
					//	//										'free'			=>	$aes_line_data[3],
					//	//										'usedpercent'	=>	str_replace('%','',$aes_line_data[4]),
					//	//										'mount'			=>	$aes_line_data[5],
					//	//										);
					//	//}
                    //
					//}
				}

				//$tmp = null;
                //
				//if		(is_array($hd_found_slash_mount))		$tmp = $hd_found_slash_mount;
				//elseif	(is_array($hd_found_slash_home))		$tmp = $hd_found_slash_home;
				//elseif	(is_array($hd_found_slash_home_nevrax))	$tmp = $hd_found_slash_home_nevrax;
				//elseif	(is_array($hd_found_slash))				$tmp = $hd_found_slash;
                //
				//if (is_array($tmp))
				//{
				//	$aes_status[$aes_server][] = array(	'device'		=>	$tmp[0],
				//										'size'			=>	$tmp[1],
				//										'used'			=>	$tmp[2],
				//										'free'			=>	$tmp[3],
				//										'usedpercent'	=>	str_replace('%','',$tmp[4]),
				//										'mount'			=>	$tmp[5],
				//										);
				//}
			}

			if (sizeof($aes_status))
			{
				nt_common_add_debug($aes_status);
				//echo '<pre>'. print_r($aes_status, true) .'</pre>';


				$sql = "DELETE FROM ". NELDB_STAT_HD_TABLE ." WHERE hd_domain_id=". $domain_id;
				$db->sql_query($sql);

				reset($aes_status);
				foreach($aes_status as $server_name => $server_datas)
				{
					reset($server_datas);
					foreach($server_datas as $server_hd)
					{
						if (trim($server_name) != '')
						{
							$sql  = "INSERT INTO ". NELDB_STAT_HD_TABLE ." (`hd_domain_id`,`hd_server`,`hd_device`,`hd_size`,`hd_used`,`hd_free`,`hd_percent`,`hd_mount`)";
							$sql .= " VALUES (". $domain_id .",'". $server_name ."','". $server_hd['device'] ."','". $server_hd['size'] ."',";
							$sql .= "'". $server_hd['used'] ."','". $server_hd['free'] ."','". $server_hd['usedpercent'] ."','". $server_hd['mount'] ."')";
							$db->sql_query($sql);
						}
						if (defined('NELTOOL_CRON_DEBUG')) echo "<br />SQL:$sql";
					}
				}

				tool_main_update_hd_time_for_domain($domain_id, time());
			}
		}

	}

	function tool_main_get_hd_data_for_domain($domain_id)
	{
		global $db;

		$data = array();

		//$sql = "SELECT * FROM ". NELDB_STAT_HD_TABLE ." WHERE hd_domain_id=". $domain_id ." AND hd_mount='/' ORDER BY hd_percent DESC";
		$sql = "SELECT * FROM ". NELDB_STAT_HD_TABLE ." WHERE hd_domain_id=". $domain_id ." ORDER BY hd_percent DESC";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				while ($row = $db->sql_fetchrow($result))
				{
					$outp  = '<b>'. $row['hd_server'] .'</b> : '. $row['hd_device'] .'<br>';
					$outp .= '• Mount : '. $row['hd_mount'] .'<br>';
					$outp .= '• Size : '. $row['hd_size'] .'<br>';
					$outp .= '• Used : '. $row['hd_used'] .'<br>';
					$outp .= '• Free : '. $row['hd_free'] .'<br>';

					$row['summary'] = $outp;
					$data[] = $row;
				}
			}
		}

		return $data;
	}

	function tool_main_get_annotation($domain_id, $shard_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_ANNOTATION_TABLE ." WHERE 1=0";
		if ($domain_id > 0)	$sql .= " OR annotation_domain_id=". $domain_id;
		if ($shard_id  > 0)	$sql .= " OR annotation_shard_id=". $shard_id;
		$sql .= " ORDER BY annotation_date DESC";

		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_main_get_lock($domain_id, $shard_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_LOCK_TABLE ." WHERE 1=0 ";
		if ($domain_id > 0)	$sql .= " OR lock_domain_id=". $domain_id;
		if ($shard_id  > 0) $sql .= " OR lock_shard_id=". $shard_id;
		$sql .= " ORDER BY lock_date DESC";

		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_main_delete_lock_shard($shard_id)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;

		nt_log("Shard Unlock (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."') by '". $nel_user['user_name'] ."'");
		$sql = "DELETE FROM ". NELDB_LOCK_TABLE ." WHERE lock_shard_id=". $shard_id;
		$db->sql_query($sql);
	}

	function tool_main_delete_lock_domain($domain_id)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;

		nt_log("Domain Unlock (Domain: '". $AS_Name ."') by '". $nel_user['user_name'] ."'");
		$sql = "DELETE FROM ". NELDB_LOCK_TABLE ." WHERE lock_domain_id=". $domain_id;
		$db->sql_query($sql);
	}


	function tool_main_set_lock_shard($domain_id, $shard_id, $log=true)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;

		// we need to check if the shard is *all*
		// if its the case, we go for a lock domain instead

		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." WHERE shard_id=". $shard_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$shard_row = $db->sql_fetchrow($result);

				if ($shard_row['shard_as_id'] == "*")
				{
					return tool_main_set_lock_domain($domain_id, $log);
				}
			}
		}

		$data = tool_main_get_lock($domain_id, $shard_id);
		$now = time();

		if (is_array($data) && ($data['lock_shard_id'] > 0))
		{
			if ($log) nt_log("Shard Lock (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."') by '". $nel_user['user_name'] ."'");
			$sql = "UPDATE ". NELDB_LOCK_TABLE ." SET lock_user_name='". $nel_user['user_name'] ."',lock_update=". $now ." WHERE lock_id=". $data['lock_id'];
			$db->sql_query($sql);
		}
		elseif (!$data)
		{
			if ($log) nt_log("Shard Lock (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."') by '". $nel_user['user_name'] ."'");
			$sql = "INSERT INTO ". NELDB_LOCK_TABLE ." (`lock_shard_id`,`lock_user_name`,`lock_date`,`lock_update`) VALUES (". $shard_id .",'". $nel_user['user_name'] ."',". $now .",". $now .")";
			$db->sql_query($sql);
		}

	}

	function tool_main_get_domain_shard_list($domain_id)
	{
		global $db;

		$data = array();

		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." WHERE shard_domain_id=". $domain_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				while ($row = $db->sql_fetchrow($result))
				{
					$data[] = $row['shard_id'];
				}
			}
		}

		return $data;
	}

	function tool_main_set_lock_domain($domain_id, $log=true)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;

		// to lock a domain you need to remove all locks from any shard in it

		$lock_data = tool_main_get_lock($domain_id, 0);

		$now = time();

		if (is_array($lock_data))
		{
			$sql = "UPDATE ". NELDB_LOCK_TABLE ." SET lock_user_name='". $nel_user['user_name'] ."',lock_update=". $now ." WHERE lock_id=". $lock_data['lock_id'];
			$db->sql_query($sql);
		}
		else
		{
			$shard_list = tool_main_get_domain_shard_list($domain_id);
			$shard_list = array_values($shard_list);

			$sql = "DELETE FROM ". NELDB_LOCK_TABLE ." WHERE lock_shard_id IN (". implode(',', $shard_list) .")";
			$db->sql_query($sql);

			$sql = "INSERT INTO ". NELDB_LOCK_TABLE ." (`lock_domain_id`,`lock_user_name`,`lock_date`,`lock_update`) VALUES (". $domain_id .",'". $nel_user['user_name'] ."',". $now .",". $now .")";
			$db->sql_query($sql);
		}

		if ($log) nt_log("Domain Lock (Domain: '". $AS_Name ."') by '". $nel_user['user_name'] ."'");
	}

	function tool_main_set_annotation($domain_id, $shard_id, $annotation)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;

		$annotation = htmlentities(trim($annotation), ENT_QUOTES);

		$data = tool_main_get_lock($domain_id, $shard_id);

		if ($data['lock_domain_id'])
		{
			// its a domain lock
			$shard_list = tool_main_get_domain_shard_list($domain_id);
			$shard_list = array_values($shard_list);

			$sql = "DELETE FROM ". NELDB_ANNOTATION_TABLE ." WHERE annotation_shard_id IN (". implode(',', $shard_list) .")";
			$db->sql_query($sql);

			$annotation_data = tool_main_get_annotation($domain_id, 0);
			if ($annotation_data)
			{
				nt_log("Domain Annotation (Domain: '". $AS_Name ."') by '". $nel_user['user_name'] ."' : ". $annotation);
				$sql = "UPDATE ". NELDB_ANNOTATION_TABLE ." SET annotation_data='". $annotation ."',annotation_user_name='". $nel_user['user_name'] ."',annotation_date=". time() ." WHERE annotation_id=". $annotation_data['annotation_id'];
				$db->sql_query($sql);
			}
			else
			{
				nt_log("Domain Annotation (Domain: '". $AS_Name ."') by '". $nel_user['user_name'] ."' : ". $annotation);
				$sql  = "INSERT INTO ". NELDB_ANNOTATION_TABLE ." (`annotation_domain_id`,`annotation_data`,`annotation_user_name`,`annotation_date`) VALUES ";
				$sql .= "(". $domain_id .",'". $annotation ."','". $nel_user['user_name'] ."',". time() .")";
				$db->sql_query($sql);
			}

		}
		else
		{
			// its a shard lock
			$annotation_data = tool_main_get_annotation(0, $shard_id);
			if ($annotation_data)
			{
				nt_log("Shard Annotation (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."') by '". $nel_user['user_name'] ."' : ". $annotation);
				$sql = "UPDATE ". NELDB_ANNOTATION_TABLE ." SET annotation_data='". $annotation ."',annotation_user_name='". $nel_user['user_name'] ."',annotation_date=". time() ." WHERE annotation_id=". $annotation_data['annotation_id'];
				$db->sql_query($sql);
			}
			else
			{
				nt_log("Shard Annotation (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."') by '". $nel_user['user_name'] ."' : ". $annotation);
				$sql  = "INSERT INTO ". NELDB_ANNOTATION_TABLE ." (`annotation_shard_id`,`annotation_data`,`annotation_user_name`,`annotation_date`) VALUES ";
				$sql .= "(". $shard_id .",'". $annotation ."','". $nel_user['user_name'] ."',". time() .")";
				$db->sql_query($sql);
			}
		}
	}


	function tool_main_get_shards_info_from_db($application, $status, $filters, $ringsqlstring='')
	{
		$shard_list			= array();
		$shard_list_result	= array();

		//nt_common_add_debug('in tool_main_get_shards_info_from_db()');
		//nt_common_add_debug($status);

		if (is_array($status) && sizeof($status))
		{
			reset($status);
			foreach($status as $sline)
			{
				$shard_name = trim($sline['ShardName']);
				$shard_id	= trim($sline['ShardId']);

				if (($shard_name != '' && $shard_id != '') && (isset($filters[$shard_name]) || isset($filters['_all_'])))
				{
					if (!in_array($shard_name, array_values($shard_list)))
					{
						$shard_list[$shard_id] = $shard_name;
					}
				}
			}
		}

		nt_common_add_debug('shard_list :');
		nt_common_add_debug($shard_list);

		reset($shard_list);
		foreach($shard_list as $shard_key => $shard_application)
		{
			if (is_numeric($shard_key))
			{
				$shard_list2[$shard_key] = $shard_application;
			}
		}

		//nt_common_add_debug('shard_list2 :');
		//nt_common_add_debug($shard_list2);

		if ($ringsqlstring == '')
		{
			global $db;
			$db->sql_select_db('nel');

			//$sql = "SELECT * FROM shard WHERE ShardId IN (". implode(',', array_keys($shard_list)) .")";
			if (is_array($shard_list2) && sizeof($shard_list2))
			{
				$sql = "SELECT * FROM shard, domain WHERE shard.domain_id=domain.domain_id AND shard.ShardId IN (". implode(',', array_keys($shard_list2)) .") AND domain.domain_name='". $application ."'";

				if ($result = $db->sql_query($sql))
				{
					if ($db->sql_numrows($result))
					{
						while ($row = $db->sql_fetchrow($result))
						{
							// patch to support live and ats mysql changes
							$state = '';
							if (isset($row['State']))				$state = $row['State'];
							elseif (isset($row['RequiredState']))	$state = $row['RequiredState'];

							$shard_name = $shard_list[$row['ShardId']];
							$shard_list_result[$shard_name] = array('shard_id'	=> $row['ShardId'],
																	'version'	=> $row['Version'],
																	'state'		=> substr($state,3),
																	'motd'		=> $row['MOTD'],
																	);
						}
					}
				}
			}

			$db->sql_reselect_db();
		}
		else
		{
			$db = new sql_db_string($ringsqlstring);
			if (is_object($db))
			{
				if (is_array($shard_list2) && sizeof($shard_list2))
				{
					//nt_common_add_debug("tool_main_get_shards_info_from_db()");
					//nt_common_add_debug($shard_list2);

					$sql = "SELECT * FROM shard WHERE shard_id IN (". implode(',', array_keys($shard_list2)) .")";

					if ($result = $db->sql_query($sql))
					{
						if ($db->sql_numrows($result))
						{
							while ($row = $db->sql_fetchrow($result))
							{
								// patch to support live and ats mysql changes
								$state = '';
								if (isset($row['State']))				$state = $row['State'];
								elseif (isset($row['RequiredState']))	$state = $row['RequiredState'];

								$shard_name = $shard_list[$row['shard_id']];
								$shard_list_result[$shard_name] = array('shard_id'	=> $row['shard_id'],
																		//'version'	=> $row['Version'],
																		'state'		=> substr($state,3),
																		'motd'		=> $row['MOTD'],
																		);
							}
						}
					}
				}
				else
				{
					nt_common_add_debug('in tool_main_get_shards_info_from_db() : shard_list2 is empty !');
					//nt_common_add_debug('dumping : filters');
					//nt_common_add_debug($filters);
				}
			}
			else
			{
				// db error
			}

		}


		return $shard_list_result;
	}

	function tool_main_get_su_from_status($status)
	{
		if (is_array($status) && sizeof($status))
		{
			reset($status);
			foreach($status as $sline)
			{
				if ($sline['ShortName'] == 'SU')
				{
					return $sline['AliasName'];
				}
			}
		}

		return null;
	}

	function tool_main_set_restart_mode($domain_id, $shard_id, $restart_mode=0)
	{
		global $db, $nel_user, $tpl;
		global $AS_ShardRestart, $AS_ShardDomainRestart;

		$sequence_info = tool_main_get_restart_sequence($nel_user['user_name'], $domain_id, $shard_id);

		if (!$sequence_info)
		{
			$sequence_info = tool_main_add_restart_sequence($nel_user['user_name'], $domain_id, $shard_id, $restart_mode);
		}

		if ($sequence_info)
		{
			$sql = "UPDATE ". NELDB_SHARD_TABLE ." SET shard_restart=". $sequence_info['restart_sequence_id'] ." WHERE shard_id=". $shard_id ." AND shard_domain_id=". $domain_id;
			$db->sql_query($sql);

			// update shards information
			$nel_user['access']['user_shards']	= tool_admin_users_shards_get_list($nel_user['user_id']);
			$nel_user['access']['group_shards']	= tool_admin_groups_shards_get_list($nel_user['user_group_id']);
			$nel_user['access']['shards'] 		= tool_admin_users_groups_shards_merge();
			$tpl->assign('tool_shard_list',	$nel_user['access']['shards']);

			// update shard restart information
			$AS_ShardRestart		= tool_main_get_shard_data($shard_id, 'shard_restart');
			$AS_ShardDomainRestart	= tool_main_get_domain_shard_restart($view_domain_id);
			$tpl->assign('tool_shard_restart_status', 		$AS_ShardRestart);
			$tpl->assign('tool_domain_has_shard_restart',	$AS_ShardDomainRestart);
		}

		return $sequence_info;
	}

	function tool_main_add_restart_sequence($user_name, $domain_id, $shard_id, $step=0)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;
		global $restart_notification_emails;

		$now = time();

		$sql  = "INSERT INTO ". NELDB_RESTART_SEQUENCE_TABLE;
		$sql .= " (`restart_sequence_domain_id`,`restart_sequence_shard_id`,`restart_sequence_user_name`,";
		$sql .= "  `restart_sequence_step`,`restart_sequence_date_start`,`restart_sequence_date_end`) VALUES ";
		$sql .= " (". $domain_id .",". $shard_id .",'". $user_name ."',";
		$sql .=       $step .",". $now .",". $now .")";
		$db->sql_query($sql);

		$sequence_info = tool_main_get_restart_sequence_by_id($db->sql_nextid());

		nt_log("Shard Restart (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."' - Sequence: '". $sequence_info['restart_sequence_id'] ."') started by '". $nel_user['user_name'] ."'");

		$email_subject	= "[Shard Admin Tool] Restart Sequence Begins (id: ".$sequence_info['restart_sequence_id'].", step: ". $step .") for shard ".$AS_Name."/".$AS_ShardName." by ".$nel_user['user_name'];
		$email_message	= $email_subject;
		nt_email($email_subject,$email_message,$restart_notification_emails);

		return $sequence_info;
	}

	function tool_main_get_restart_sequence($user_name, $domain_id, $shard_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_SEQUENCE_TABLE ." WHERE restart_sequence_domain_id=". $domain_id ." AND restart_sequence_shard_id=". $shard_id ." AND restart_sequence_user_name='". $user_name ."' AND restart_sequence_step=0 AND restart_sequence_date_start=restart_sequence_date_end";
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_main_get_restart_sequence_by_id($sequence_id)
	{
		global $db;

		$data = null;

		$sql = "SELECT * FROM ". NELDB_RESTART_SEQUENCE_TABLE ." WHERE restart_sequence_id=". $sequence_id;
		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_main_set_next_restart_sequence_step($sequence_id, $step=null)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;
		global $restart_notification_emails;

		$now = time();

		if ($step === null)
		{
			$sequence_info = tool_main_get_restart_sequence_by_id($sequence_id);
			$step = $sequence_info['restart_sequence_step'] + 1;
		}

		$sql = "UPDATE ". NELDB_RESTART_SEQUENCE_TABLE ." SET restart_sequence_step=". $step .",restart_sequence_date_end=". $now ." WHERE restart_sequence_id=". $sequence_id;
		$db->sql_query($sql);

		nt_log("Shard Restart (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."' - Sequence: '". $sequence_id ."') - ". $nel_user['user_name'] ." moved to step ". $step);

		if ($step !== null)
		{
			$email_subject	= "[Shard Admin Tool] Restart Sequence Updated (id: ".$sequence_id.", step: ". $step .") for shard ".$AS_Name."/".$AS_ShardName." by ".$nel_user['user_name'];
			$email_message	= $email_subject;
			nt_email($email_subject,$email_message,$restart_notification_emails);
		}

		return tool_main_get_restart_sequence_by_id($sequence_id);
	}

	function tool_main_set_end_restart_sequence($sequence_id)
	{
		global $db;
		global $nel_user;
		global $AS_Name, $AS_ShardName;
		global $restart_notification_emails;

		//$sequence_info = tool_main_set_next_restart_sequence_step($sequence_id);
		$sequence_info = tool_main_get_restart_sequence_by_id($sequence_id);

		$sql = "UPDATE ". NELDB_SHARD_TABLE ." SET shard_restart=0 WHERE shard_domain_id=". $sequence_info['restart_sequence_domain_id'] ." AND shard_id=". $sequence_info['restart_sequence_shard_id'];
		$db->sql_query($sql);

		nt_log("Shard Restart (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."' - Sequence: '". $sequence_id ."') - ". $nel_user['user_name'] ." ended the sequence !". $step);

		$email_subject	= "[Shard Admin Tool] Restart Sequence Ends (id: ".$sequence_info['restart_sequence_id'].", step: ". $sequence_info['restart_sequence_step'] .") for shard ".$AS_Name."/".$AS_ShardName." by ".$nel_user['user_name'];
		$email_message	= $email_subject;
		nt_email($email_subject,$email_message,$restart_notification_emails);
	}

	function tool_main_get_domain_shard_restart($domain_id)
	{
		global $db;

		$num = 0;
		$sql = "SELECT * FROM ". NELDB_SHARD_TABLE ." WHERE shard_domain_id=". $domain_id ." AND shard_restart>0";
		if ($result = $db->sql_query($sql))
		{
			$num = $db->sql_numrows($result);
		}

		return $num;
	}

	function tool_main_get_restart_services($InternalName, $domainServices, $restart_list)
	{
		// lets sort each services by 'type'
		nt_common_add_debug("RESTART DEBUG");
		nt_common_add_debug($domainServices);

		$sortedServices = array();

		reset($domainServices);
		foreach($domainServices as $domainService)
		{
			if ($domainService['ShardName'] == $InternalName)
			{
				// this should be the real 'type'
				// $type = $domainService['ShortName'];
				// but instead we'll take the first part of the AliasName (as in "rws" or "rws_aniro")
				$type = explode('_',$domainService['AliasName']);
				$type = trim(strtolower($type[0]));

				if (!isset($sortedServices[$type]))
				{
					$sortedServices[$type] = array();
				}

				$sortedServices[$type][] = $domainService['AliasName'];
				nt_common_add_debug("Adding '". $domainService['AliasName'] ."' to '$type'");
			}
		}

		reset($restart_list);
		foreach($restart_list as $restart_key => $restart_group)
		{
			$group_list = explode(',', $restart_group['restart_group_list']);
			$tmp_list = array();

			reset($group_list);
			foreach($group_list as $group_service_type)
			{
				$group_service_type = trim($group_service_type);
				nt_common_add_debug("looking for '$group_service_type'");

				if (isset($sortedServices[$group_service_type]))
				{
					$tmp_list[] = implode(',',$sortedServices[$group_service_type]);
				}
				else
				{
					nt_common_add_debug("not found!");
				}
			}

			$restart_list[$restart_key]['service_list'] = implode(',',$tmp_list);

		}

        nt_common_add_debug("RESTART SERVICES DEBUG");
		nt_common_add_debug($restart_list);

		return $restart_list;
	}

	function tool_main_get_all_restart_services($restart_list)
	{
		$stop_data = array();
		$stop_list = '';

		reset($restart_list);
		foreach($restart_list as $restart_key => $restart_group)
		{
			$stop_data[] = $restart_group['service_list'];
		}

		$stop_list = implode(',', $stop_data);
		return $stop_list;
	}

	function tool_main_get_shardid_from_status($domainServices, $InternalName)
	{
		reset($domainServices);
		foreach($domainServices as $service)
		{
			if (($service['ShardName'] == $InternalName) && (is_numeric($service['ShardId'])))
			{
				return $service['ShardId'];
			}
		}

		return null;
	}

	function tool_main_get_egs_from_status($domainServices, $InternalName)
	{
		reset($domainServices);
		foreach ($domainServices as $service)
		{
			if (($service['ShardName'] == $InternalName) && ($service['ShortName'] == 'EGS'))
			{
				return $service['AliasName'];
			}
		}

		return null;
	}

	function tool_main_set_restart_sequence_timer($sequence_id, $timer)
	{
		global $db;

		$now 		= time();
		$timer 		= $timer;
		$new_timer	= $now + $timer;

		$sql = "UPDATE ". NELDB_RESTART_SEQUENCE_TABLE ." SET restart_sequence_date_end=". $now .",restart_sequence_timer=". $new_timer ." WHERE restart_sequence_id=". $sequence_id;
		$db->sql_query($sql);

		return array('start' => $now, 'end' => $new_timer);
	}

	function tool_main_change_restart_notification($event_list, $mode, $sql_connection)
	{
		if ($sql_connection != '' && sizeof($event_list))
		{
			$csdb = new sql_db_string($sql_connection);
			if (is_object($csdb))
			{
				$sql = "UPDATE Sorbot_events SET event_action=". $mode .",event_time=". time() .",event_lap=0 WHERE event_id IN (". implode(',', array_values($event_list)) .")";
				$csdb->sql_query($sql);

				$csdb->sql_close();
			}
		}
	}

	function tool_main_add_restart_notification($sequence_id, $service_su, $service_egs, $shard_id, $sorbot_message_type, $sql_connection, $shard_lang='en')
	{
		nt_common_add_debug('tool_main_add_restart_notification()');

		if ($sql_connection != '')
		{
			$csdb = new sql_db_string($sql_connection);
			if (is_object($csdb))
			{
				$sequence_info	= tool_main_get_restart_sequence_by_id($sequence_id);
				$shard_info		= tool_main_get_domain_shard_data($sequence_info['restart_sequence_domain_id'], $sequence_info['restart_sequence_shard_id']);

				if ($sorbot_message_type == 4102)
				{
					//$open_timer	= 60*25; // 25 minutes before a shard opens itself
					// new timer
					$open_timer	= 60*10; // 10 minutes before a shard opens itself
				}
				else // 4101
				{
					$open_timer = 0;
				}

				if ($sequence_info && $shard_info)
				{
					// lets find the shard id used by the ticket system
					$sql = "SELECT * FROM ForumCS_tickets_shards WHERE shard_ca='". $shard_info['domain_application'] ."' AND shard_id='". $shard_id ."'";
					if ($result = $csdb->sql_query($sql))
					{
						if ($csdb->sql_numrows($result))
						{
							$ticketsystem_shard_info = $csdb->sql_fetchrow($result);
							nt_common_add_debug($ticketsystem_shard_info);

							$ticketsystem_shard_id = $ticketsystem_shard_info['id'];

							// now we have the shard id, lets see which klients servers wants events for it
							$sql = "SELECT * FROM Sorbot_botconfig WHERE config_name='shardRestart' AND config_value LIKE '%:". $ticketsystem_shard_id .":%'";
							if ($result2 = $csdb->sql_query($sql))
							{
								if ($csdb->sql_numrows($result2))
								{
									//$klients_servers = array();
									//while ($row = $csdb->sql_fetchrow($result2))
									//{
									//	$klients_servers[] = $row['server_name'];
									//}

									// NOTE: live but not in CVS yet !
									$local_timer = $open_timer;

									$klients_servers = array();
									while ($row = $csdb->sql_fetchrow($result2))
									{
										$klients_servers[] = $row['server_name'];

										// lets find if there is any specific opening timer, and keep the lowest one
										$sql = "SELECT * FROM Sorbot_botconfig WHERE server_name='". $row['server_name'] ."' AND config_name='shardRestartOpenTimer'";
										if ($result3 = $csdb->sql_query($sql))
										{
											if ($csdb->sql_numrows($result3))
											{
												$timer_row = $csdb->sql_fetchrow($result3);
												if ($timer_row['config_value'] < $local_timer)
												{
													$local_timer = $timer_row['config_value'];
												}
											}
										}
									}

									$open_timer = $local_timer;
									// END NOTE: live but not in CVS yet !

									// ok now we the list of servers that want this event, lets give it to them

									// lets build some messages
									$klients_countdown = '';
									tool_main_get_elapsed_time_string($sequence_info['restart_sequence_timer'] - $sequence_info['restart_sequence_date_end'], $klients_countdown);
									$klients_countdown = trim($klients_countdown);

									$klients_open_countdown = '';
									tool_main_get_elapsed_time_string($open_timer, $klients_open_countdown);
									$klients_open_countdown = trim($klients_open_countdown);

									$opening_message = tool_admin_restart_messages_get_list_from_name('opening',$shard_lang);
									$opening_message = $opening_message[0]['restart_message_value'];

									$opened_message = tool_admin_restart_messages_get_list_from_name('opened',$shard_lang);
									$opened_message = $opening_message[0]['restart_message_value'];

									$klients_messages = array(	'before' => array(
																	".topic Shard Maintenance in progress for '". $ticketsystem_shard_info['shard_name'] ."' - Shard LOCKED - (". date("r",$sequence_info['restart_sequence_date_start']) ." - Countdown set to ". $klients_countdown .")",
																	"[Shard Maintenance] A Shard Restart has been triggered for shard '". $ticketsystem_shard_info['shard_name'] ."' on ". date("r",$sequence_info['restart_sequence_date_start']),
																	"[Shard Maintenance] Shard is LOCKED and should go down in ". $klients_countdown,
																),
																'at' => array(
																	".topic Shard Maintenance in progress for '". $ticketsystem_shard_info['shard_name'] ."' - Shard DOWN - (". date("r",$sequence_info['restart_sequence_date_start']) .")",
																	"[Shard Maintenance] A Shard Restart has been triggered for shard '". $ticketsystem_shard_info['shard_name'] ."' on ". date("r",$sequence_info['restart_sequence_date_start']),
																	"[Shard Maintenance] Shard is DOWN after a countdown of ". $klients_countdown,
																),
																'over' => array(
																	".topic Shard Maintenance in finished for '". $ticketsystem_shard_info['shard_name'] ."' - Shard LOCKED - Ready for CSRs ! ETA : |EventETA| - To Open : .sorOpenShard |EventID| 0",
																	"[Shard Maintenance] The Shard Restart that has been triggered for shard '". $ticketsystem_shard_info['shard_name'] ."' on ". date("r",$sequence_info['restart_sequence_date_start']) ." is now finished !",
																	"[Shard Maintenance] Shard is LOCKED and Ready for CSRs, it will OPEN automatically in |EventTIME| !",/*. $klients_open_countdown ." !",*/
																	"[Shard Maintenance] To change the OPEN countdown,  you need to send me the command : .sorOpenShard |EventID| <seconds>",
																	"[Shard Maintenance] Example to OPEN the Shard now, you need to send me the command : .sorOpenShard |EventID| 0",
																),
																'open' => array(
																	".topic Shard Maintenance over for '". $ticketsystem_shard_info['shard_name'] ."' - Shard OPEN ",
																	"[Shard Maintenance] The shard '". $ticketsystem_shard_info['shard_name'] ."' should now be OPEN to all !",
																),
																'cancel' => array(
																	".topic Shard Maintenance has been cancelled for '". $ticketsystem_shard_info['shard_name'] ."' - Shard OPEN",
																	"[Shard Maintenance] The shard restart has been CANCELLED for '". $ticketsystem_shard_info['shard_name'] ."', the shard is OPEN again !",
																),
																'giveup' => array(
																	".topic Shard Maintenance for '". $ticketsystem_shard_info['shard_name'] ."' is on hold for technical issues - Shard DOWN",
																	"[Shard Maintenance] The shard restart has been set ON HOLD for technical issues !",
																),
																'opening' => $opening_message,
																'opened' => $opened_message,
																);

									$notification_info = array(	'sequence'	=> $sequence_info,
																'ts'		=> $ticketsystem_shard_info,
																'messages'	=> $klients_messages,
																'shard'		=> $shard_info,
																'opentimer'	=> $open_timer,
																'shardid'	=> $shard_id,
																'shardsu'	=> $service_su,
																'shardegs'	=> $service_egs,
																);

									$notification_data = base64_encode(serialize($notification_info));

									$event_insert_ids = array();

									$sql = "LOCK TABLES Sorbot_events WRITE";
									$csdb->sql_query($sql);

									reset($klients_servers);
									foreach($klients_servers as $klient_server)
									{
										$sql  = "INSERT INTO Sorbot_events (`server_name`,`event_action`,`event_time`,`event_lap`,`event_params`) ";
										$sql .= " VALUES ('". $klient_server ."',". $sorbot_message_type .",". time() .",". $open_timer .",'". $notification_data ."')";
										$csdb->sql_query($sql);
										$event_insert_ids[] = $csdb->sql_nextid();
									}

									// we have a list of all the events, lets save them somewhere
									// lets inform each events of all other events for the same sequence
									if (sizeof($event_insert_ids))
									{
										$notification_info['eventlist'] = array_values($event_insert_ids);
										$notification_data = base64_encode(serialize($notification_info));

										$sql = "UPDATE Sorbot_events SET event_params='". $notification_data ."' WHERE event_id IN (". implode(',', array_values($event_insert_ids)) .")";
										$csdb->sql_query($sql);

										// save the events locally too
										tool_main_set_restart_sequence_events($sequence_id, implode(',',array_values($event_insert_ids)));
									}

									$sql = "UNLOCK TABLES";
									$csdb->sql_query($sql);
								}
								else
								{
									nt_common_add_debug('no klients server found wanting to hear about this event');
								}
							}
						}
						else
						{
							nt_common_add_debug('no shard found in TS matching the application and shard id');
						}
					}
					else
					{
						nt_common_add_debug('query failed ?!');
					}
				}
				else
				{
					nt_common_add_debug('missing some information about the sequence ?!');
				}

				$csdb->sql_close();
			}
			else
			{
				nt_common_add_debug('failed to init db!');
			}
		}
		else
		{
			nt_common_add_debug('no db string to work with');
		}
	}

	function tool_main_set_restart_sequence_events($sequence_id, $events)
	{
		global $db;

		$sql = "UPDATE ". NELDB_RESTART_SEQUENCE_TABLE ." SET restart_sequence_events='". $events ."' WHERE restart_sequence_id=". $sequence_id;
		$db->sql_query($sql);
	}

	function tool_main_get_domain_shard_data($domain_id, $shard_id)
	{
		global $db;

		$data = null;

		$sql  = "SELECT ". NELDB_DOMAIN_TABLE .".domain_id,";
		$sql .=            NELDB_DOMAIN_TABLE .".domain_name,";
		$sql .=            NELDB_DOMAIN_TABLE .".domain_as_host,";
		$sql .=            NELDB_DOMAIN_TABLE .".domain_as_port,";
		$sql .=            NELDB_DOMAIN_TABLE .".domain_application,";
		$sql .=            NELDB_SHARD_TABLE .".shard_id,";
		$sql .=            NELDB_SHARD_TABLE .".shard_name,";
		$sql .=            NELDB_SHARD_TABLE .".shard_as_id,";
		$sql .=            NELDB_SHARD_TABLE .".shard_domain_id,";
		$sql .=            NELDB_SHARD_TABLE .".shard_lang,";
		$sql .=            NELDB_SHARD_TABLE .".shard_restart";
		$sql .= " FROM ". NELDB_DOMAIN_TABLE .",". NELDB_SHARD_TABLE;
		$sql .= " WHERE domain_id=shard_domain_id AND domain_id=". $domain_id ." AND shard_id=". $shard_id;

		if ($result = $db->sql_query($sql))
		{
			if ($db->sql_numrows($result))
			{
				$data = $db->sql_fetchrow($result);
			}
		}

		return $data;
	}

	function tool_main_set_restart_sequence_user($AS_ShardRestart)
	{
		global $db;
		global $AS_Name, $AS_ShardName;
		global $nel_user;
		global $restart_notification_emails;

		$sequence_info	= tool_main_get_restart_sequence_by_id($AS_ShardRestart);

		if ($sequence_info)
		{
			$sql = "UPDATE ". NELDB_RESTART_SEQUENCE_TABLE ." SET restart_sequence_user_name='". $nel_user['user_name'] ."' WHERE restart_sequence_id=". $AS_ShardRestart;
			$db->sql_query($sql);

			nt_log("Shard Restart (Domain: '". $AS_Name ."' - Shard: '". $AS_ShardName ."' - Sequence: '". $AS_ShardRestart ."') owner set to '". $nel_user['user_name'] ."'");

			$email_subject	= "[Shard Admin Tool] Restart Sequence Takeover (id: ".$sequence_info['restart_sequence_id'].", step: ". $sequence_info['restart_sequence_step'] .") for shard ".$AS_Name."/".$AS_ShardName." by ".$nel_user['user_name'];
			$email_message	= $email_subject;
			nt_email($email_subject,$email_message,$restart_notification_emails);
		}
	}


?>
