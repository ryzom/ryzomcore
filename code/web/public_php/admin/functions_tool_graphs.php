<?php

	$tool_graph_menu	= array(array('title'	=>	'CCU',
									  'key'		=>	'ccu',
									  'uri'		=>	'tool_graphs.php?toolmode=ccu',
									  'tpl'		=>	'tool_graphs_ccu.tpl',
									  'access'	=>	'',
									  ),
								array('title'	=>	'Tech Shard',
									  'key'		=>	'tech',
									  'uri'		=>	'tool_graphs.php?toolmode=tech',
									  'tpl'		=>	'tool_graphs_tech.tpl',
									  'access'	=>	'',
									  ),
								array('title'	=>	'Hi-Res Shard',
									  'key'		=>	'hires',
									  'uri'		=>	'tool_graphs.php?toolmode=hires',
									  'tpl'		=>	'tool_graphs_hires.tpl',
									  'access'	=>	'',
									  ),
								array('title'	=>	'Old',
									  'key'		=>	'old',
									  'uri'		=>	'tool_graphs.php?toolmode=old',
									  'tpl'		=>	'tool_graphs.tpl',
									  'access'	=>	'',
									  ),
								);

	$tool_hires_frames	= array(array('title'	=> 	'10 seconds',
									  'value'	=>	10000,
									  'step'	=>	0,
									  'default'	=>	false,
									  ),
								array('title'	=>	'30 seconds',
									  'value'	=>	30000,
									  'step'	=>	0,
									  'default'	=>	true,
									  ),
								array('title'	=>	'90 seconds',
									  'value'	=>	90000,
									  'step'	=>	0,
									  'default'	=>	false,
									  ),
								);

	$tool_lowres_frames	= array(array('title'	=> 	'20 minutes',
									  'value'	=>	1200,
									  'default'	=>	false,
									  ),
								array('title'	=> 	'3 hours',
									  'value'	=>	10800,
									  'default'	=>	false,
									  ),
								array('title'	=> 	'24 hours',
									  'value'	=>	86400,
									  'default'	=>	true,
									  ),
								array('title'	=> 	'7 days',
									  'value'	=>	604800,
									  'default'	=>	false,
									  ),
								array('title'	=> 	'30 days',
									  'value'	=>	2592000,
									  'default'	=>	false,
									  ),
								array('title'	=> 	'90 days',
									  'value'	=>	7776000,
									  'default'	=>	false,
									  ),
								);

	function tool_graphs_time_frame_get_default($list)
	{
		reset($list);
		foreach($list as $frame)
		{
			if ($frame['default'] == true) return $frame['value'];
		}
	}

	function tool_graphs_menu_get_list()
	{
		global $tool_graph_menu;
		global $nel_user;

		$new_menu = array();

		reset($tool_graph_menu);
		foreach($tool_graph_menu as $menu_item)
		{
			if (($menu_item['access'] == '') || tool_admin_applications_check($menu_item['access']))
			{
				$new_menu[] = $menu_item;
			}
		}

		return $new_menu;
	}

	function tool_graphs_menu_get_item_from_key($key)
	{
		global $tool_graph_menu;

		reset($tool_graph_menu);
		foreach($tool_graph_menu as $tool_menu)
		{
			if ($tool_menu['key'] == $key)	return $tool_menu;
		}

		return null;
	}

	function tool_graphs_find($needles, $list)
	{
		$result = array();

		reset($needles);
		foreach($needles as $needle)
		{
			if (isset($list[$needle['variable']]))
			{
				nt_common_add_debug("variable found ". $needle['variable']);
				reset($list[$needle['variable']]);
				foreach($list[$needle['variable']] as $var)
				{
					nt_common_add_debug("checking '". $needle['service'] ."' in '". $var['service'] ."'");
					if (ereg("^". $needle['service'] .".*$",$var['service']))
					{
						nt_common_add_debug("adding ". $var['service']);
						$result[] = $var;
					}
				}
			}
		}

		return $result;
	}

	function tool_graphs_get_list_v2($dir, $shard_match, $high=false, $domain=false)
	{
		$data = array();

		if (!ereg("^[a-zA-Z0-9_]+$",$shard_match) && !$domain) return $data;

		if (substr($dir, -1) != '/') $dir .= '/';

		if (is_dir($dir))
		{
			if ($handle = opendir($dir))
			{
				while (($file = readdir($handle)) !== false)
				{
					if (($file != '.') && ($file != '..'))
					{
						$filelist[] = $file;
					}
				}
				closedir($handle);

				sort($filelist);
				nt_common_add_debug($filelist);

				//fes_arispotle_01.NetSpeedLoop.hrd
				//egs_arispotle.TickSpeedLoop.hrd
				//$my_ereg = "^([^_]+_(". $shard_match .")(_[^\ \.])?)\.([^\ ]+)\.([hr])rd$";
				//$my_ereg = "^([^_]+_(". $shard_match .")(_[^\ \.])?)\.([^\ ]+)\.rrd$";
				$my_ereg	= "^([^_]+(_[^_]+)?_(". $shard_match .")(_[^\ \.])?)\.([^.]+)\.". ($high === true ? 'h':'r') ."rd$";
				nt_common_add_debug("using regexp: ".$my_ereg);

				// 0: complete file name
				// 1: service alias (eg. fes_arispotle_01)
				// 2: n/a
				// 3: shard (eg. arispotle)
				// 4: n/a
				// 5: variable (eg. NetSpeedLoop)

				// this is special, mainly to catch domain wide variables, such as su.* which don't have a shard
				$my_ereg2	= "^([^_]+(_[^_]+)?(_[^\ \.])?)\.([^.]+)\.". ($high === true ? 'h':'r') ."rd$";
				nt_common_add_debug("using regexp2: ".$my_ereg2);

				// 0: complete file name
				// 1: service alias (eg. fes_arispotle_01)
				// 2: n/a
				// 3: n/a
				// 4: variable (eg. NetSpeedLoop)


				reset($filelist);
				foreach($filelist as $file)
				//while (($file = readdir($handle)) !== false)
				{
					//nt_common_add_debug("checking : ". $file);
					if (ereg($my_ereg, $file, $params))
					{
					nt_common_add_debug("ok".$file);
						//nt_common_add_debug($params);
						$tmp = array(	'rd_file'	=> $params[0],
										'service'	=> $params[1],
										'shard'		=> $params[3],
										'variable'	=> $params[5]);

						$data[$params[5]][] = $tmp;
					}
					elseif (ereg($my_ereg2, $file, $params))
					{
					nt_common_add_debug("ok2".$file);
						$tmp = array(	'rd_file'	=> $params[0],
										'service'	=> $params[1],
										'shard'		=> 'open',
										'variable'	=> $params[4]);

						$data[$params[4]][] = $tmp;
					}
				}

			}
		}

		nt_common_add_debug(array(array_keys($data), $data));

		return array('variables' => array_keys($data), 'datas' => $data);

	}


	function tool_graphs_get_list($dir, $shard_match)
	{
		$data = array();

		if (substr($dir, -1) != '/') $dir .= '/';

		if (is_dir($dir))
		{
			if ($handle = opendir($dir))
			{
				//fes_arispotle_01.NetSpeedLoop.hrd
				//egs_arispotle.TickSpeedLoop.hrd
				$my_ereg = "^([^_]+_(". $shard_match .")(_[^\ \.])?)\.([^\ ]+)\.([hr])rd$";
				$my_ereg = "^([^_]+_(". $shard_match .")(_[^\ \.])?)\.([^\ ]+)\.rrd$";
				$my_ereg = "^([^_]+(_[^_]+)?_(". $shard_match .")(_[^\ \.])?)\.([^.]+)\.rrd$";
				//nt_common_add_debug($my_ereg);

				// 0: complete file name
				// 1: service alias (eg. fes_arispotle_01)
				// 2: n/a
				// 3: shard (eg. arispotle)
				// 4: n/a
				// 5: variable (eg. NetSpeedLoop)
				// 6: graph type, h/r : high/low (removed)

				while (($file = readdir($handle)) !== false)
				{
					if (($file != '.') && ($file != '..'))
					{
						//nt_common_add_debug("checking : ". $file);
						if (ereg($my_ereg, $file, $params))
						{
							$high_file = str_replace('.rrd','.hrd',$file);
							if (!file_exists($dir . $high_file)) $high_file = '';

							//nt_common_add_debug($params);
							$tmp = array(	'low_file'	=> $params[0],
											'high_file'	=> $high_file,
											'service'	=> $params[1],
											'shard'		=> $params[3],
											'variable'	=> $params[5]);

							$data[$params[5]][] = $tmp;
						}
					}
				}

				closedir($handle);
			}
		}

		nt_common_add_debug(array(array_keys($data), $data));

		return array('variables' => array_keys($data), 'datas' => $data);
	}

	function tool_graphs_get_data($data, $variable, $service)
	{
		reset($data);
		foreach($data as $svar => $sdata)
		{
			if ($svar == $variable)
			{
				reset($sdata);
				foreach($sdata as $sidata)
				{
					if ($sidata['service'] == $service)
					{
						return $sidata;
					}
				}
			}
		}

		return null;
	}

	function tool_graphs_extract_mean_values($data)
	{
		$result = array('ref' => array(), 'val' => array());

		$base = null;

		reset($data);
		foreach($data as $sdata)
		{
			$tmp_data = explode(':', $sdata);

			// get reference as t0
			if (!$base) $base = trim($tmp_data[1]);

			$val_data = explode(' ', $tmp_data[2]);

			$result['ref'][] = $tmp_data[1] - $base;
			$result['val'][] = trim($val_data[1]);
		}

		return $result;
	}

	function tool_graphs_xaxis_callback($aVal)
	{
		return ($aVal / 1000) .'k';
	}

// ######################################################################################################################
// ######################################################################################################################

	function tool_graphs_rrd_get_list($dir)
	{
		$dir_list = array();

		if ($handle = opendir($dir))
		{
   			while (false !== ($file = readdir($handle)))
   			{
       			if (($file != ".") && ($file != "..") && (substr($file, -4) == '.rrd'))
       			{
					$dir_list[] = array('name' => $file, 'code' => base64_encode($file));
       			}
   			}

   			closedir($handle);
		}

		return $dir_list;
	}

	function tool_graphs_build_rrd($fname,$period=0)
	{
		$rrdperiod	= '-'. $period;
		$rrdfile	= RRD_PATH . $fname;
		$webimage 	= GFX_PATH . $fname .'_'. $period .'.gif';

		$opts = array(	"--start", $rrdperiod, "DEF:val=". $rrdfile .":var:AVERAGE", "LINE2:val#0000FF");
		$ret = rrd_graph($webimage, $opts, count($opts));

		if ( is_array($ret) )
		{
			return array('status' => true, 'img' => $webimage);
		}

		$err = rrd_error();
		return array('status' => false, 'error' => "Error: rrd_graph() -- $err");
	}



?>