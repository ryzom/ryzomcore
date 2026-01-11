<?php

	require_once('common.php');
	require_once('functions_tool_main.php');
	require_once('functions_tool_graphs.php');

	require_once("jpgraph/jpgraph.php");
	require_once("jpgraph/jpgraph_line.php");
	require_once("jpgraph/jpgraph_log.php");


	if (!tool_admin_applications_check('tool_graph'))	nt_common_redirect('index.php');

	nt_common_add_debug('-- Starting on \'tool_graphs.php\'');

	if (!isset($NELTOOL['GET_VARS']['toolmode']))
	{
		$NELTOOL['GET_VARS']['toolmode'] = 'ccu';
		nt_auth_unset_session_var('view_shard_id');
		nt_auth_unset_session_var('view_time_highframe');
		nt_auth_unset_session_var('view_time_lowframe');
	}

	$tool_menu_item = tool_graphs_menu_get_item_from_key($NELTOOL['GET_VARS']['toolmode']);
	$tpl->assign('toolmode',	$NELTOOL['GET_VARS']['toolmode']);

	$tpl->assign('tool_title',	'Graphs&nbsp;/&nbsp;'. $tool_menu_item['title']);
	$tpl->assign('tool_menu',	tool_graphs_menu_get_list());

	$view_domain_id 		= nt_auth_get_session_var('view_domain_id');
	$view_shard_id 			= nt_auth_get_session_var('view_shard_id');
	$view_time_highframe	= nt_auth_get_session_var('view_time_highframe');
	$view_time_lowframe		= nt_auth_get_session_var('view_time_lowframe');

	if (!$view_domain_id)
	{
		$view_domain_id	= $nel_user['group_default_domain_id'];
		$view_shard_id	= $nel_user['group_default_shard_id'];
		nt_auth_set_session_var('view_domain_id', $view_domain_id);
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (isset($NELTOOL['GET_VARS']['domain']))
	{
		if ($view_domain_id != $NELTOOL['GET_VARS']['domain'])
		{
			$view_domain_id = $NELTOOL['GET_VARS']['domain'];
			nt_auth_set_session_var('view_domain_id', $view_domain_id);

			$view_shard_id = null;
			nt_auth_unset_session_var('view_shard_id');
		}
	}

	if (isset($NELTOOL['GET_VARS']['shard']))
	{
		$view_shard_id = $NELTOOL['GET_VARS']['shard'];
		nt_auth_set_session_var('view_shard_id', $view_shard_id);
	}

	if (isset($NELTOOL['GET_VARS']['highframe']))
	{
		$view_time_highframe = $NELTOOL['GET_VARS']['highframe'];
		nt_auth_set_session_var('view_time_highframe', $view_time_highframe);
	}

	if (isset($NELTOOL['GET_VARS']['lowframe']))
	{
		$view_time_lowframe = $NELTOOL['GET_VARS']['lowframe'];
		nt_auth_set_session_var('view_time_lowframe', $view_time_lowframe);
	}

	if ($view_time_highframe == null)
	{
		$view_time_highframe = tool_graphs_time_frame_get_default($tool_hires_frames);
	}

	if ($view_time_lowframe == null)
	{
		$view_time_lowframe = tool_graphs_time_frame_get_default($tool_lowres_frames);
	}


	$current_refresh_rate = nt_auth_get_session_var('current_refresh_rate');
	if (isset($_POST['services_refresh']))
	{
		if ($current_refresh_rate != $_POST['services_refresh'])
		{
			$current_refresh_rate = $_POST['services_refresh'];
			nt_auth_set_session_var('current_refresh_rate',$current_refresh_rate);
		}
	}

	if ($current_refresh_rate == null)
	{
		$current_refresh_rate = 0;
	}
	elseif ($current_refresh_rate > 0)
	{
		$tpl->assign('nel_tool_refresh',	'<meta http-equiv=refresh content="'. $current_refresh_rate .'">');
	}

	$tpl->assign('tool_refresh_list',		$refresh_rates);
	$tpl->assign('tool_refresh_rate',		$current_refresh_rate);

	$tpl->assign('tool_domain_list',		$nel_user['access']['domains']);
	$tpl->assign('tool_domain_selected',	$view_domain_id);

	$tpl->assign('tool_shard_list',			$nel_user['access']['shards']);
	$tpl->assign('tool_shard_selected',		$view_shard_id);

	$tool_shard_filters	= tool_main_get_shard_ids($view_shard_id);
	$tpl->assign('tool_shard_filters',		$tool_shard_filters);

	if ($view_domain_id)
	{
		$tool_as_error = null;

		$AS_Name = tool_main_get_domain_name($view_domain_id);
		$AS_Host = tool_main_get_domain_host($view_domain_id);
		$AS_Port = tool_main_get_domain_port($view_domain_id);
		$AS_ShardName		= tool_main_get_shard_name($view_shard_id);
		$AS_InternalName	= tool_main_get_shard_as_id($view_shard_id);
		$AS_RRDPath			= tool_main_get_domain_rrd_path($view_domain_id);


		if ($AS_RRDPath != "")
		{
			// lets make sure there is a trailing /
			if (substr($AS_RRDPath, -1) != '/') $AS_RRDPath .= '/';

			$tpl->assign('tool_page_title', 'Graphs - '. $AS_Name . ($AS_ShardName != '' ? ' / '. $AS_ShardName : ''));

			switch($NELTOOL['GET_VARS']['toolmode'])
			{
				case 'ccu':
					/*
					 * ###################################################################################################
					 *  CCU Pages
					 * ###################################################################################################
					 */

					$tpl->assign('tool_frame_list',		$tool_lowres_frames);
					$tpl->assign('tool_frame_selected',	$view_time_lowframe);

					if ($view_time_lowframe)
					{
						$graph_data_tmp = tool_graphs_get_list_v2($AS_RRDPath, strtolower($AS_InternalName), false, true);

						$tool_tech_graph_list = array(	array('service'	=>	'su', 	'variable'	=>	'TotalConcurentUser'),
														array('service'	=>	'egs',	'variable'	=>	'NbPlayers'),
													 );

						$graph_list = tool_graphs_find($tool_tech_graph_list, $graph_data_tmp['datas']);
						nt_common_add_debug($graph_list);

						$rrd_webs	= array();
						reset($graph_list);
						foreach($graph_list as $graph_item)
						{
							$rrd_path	= $AS_RRDPath . $graph_item['rd_file'];
							$rrd_def	= "DEF:val=". $rrd_path .":var:AVERAGE";
							$rrd_draw	= "LINE2:val#0000FF --no-legend";
							$rrd_output	= NELTOOL_RRDSYSBASE . $graph_item['rd_file'] ."-". $view_time_lowframe .".gif";
							$rrd_web	= NELTOOL_RRDWEBBASE . $graph_item['rd_file'] ."-". $view_time_lowframe .".gif";
							$rrd_exec	= NELTOOL_RRDTOOL ." graph ". $rrd_output ." --width 916 --height 110 --start -". $view_time_lowframe ." ". $rrd_def ." ". $rrd_draw;

							nt_common_add_debug($rrd_exec);
							exec($rrd_exec, $rrd_result, $rrd_code);

							$file_description = str_replace(array('.rrd','.hrd','.'),
															array('',    '',    '&nbsp;-&nbsp;'),
															$graph_item['rd_file']);

							$time_string = '';
							tool_main_get_elapsed_time_string($view_time_lowframe, $time_string);

							$rrd_webs[] = array('desc'	=> $file_description .' over '. $time_string,
												'img'	=> $rrd_web);
						}
					}

					$tpl->assign('tool_rrd_output', $rrd_webs);

					break;

				case 'tech':
					/*
					 * ###################################################################################################
					 *  Tech Shard Pages (Low Res)
					 	// ts_mainland01.TotalSpeedLoop.rrd
						// egs_mainland01.NbPlayers.rrd
						// egs_mainland01.ProcessUsedMemory.rrd
						// egs_mainland01.TickSpeedLoop.rrd
					 * ###################################################################################################
					 */

					$tpl->assign('tool_frame_list',		$tool_lowres_frames);
					$tpl->assign('tool_frame_selected',	$view_time_lowframe);

					if ($view_shard_id && $view_time_lowframe)
					{
						$graph_data_tmp = tool_graphs_get_list_v2($AS_RRDPath, strtolower($AS_InternalName), false);

						$tool_tech_graph_list = array(	array('service'	=>	'ts', 	'variable'	=>	'TotalSpeedLoop'),
														array('service'	=>	'egs',	'variable'	=>	'NbPlayers'),
														array('service'	=>	'egs',	'variable'	=>	'ProcessUsedMemory'),
														array('service'	=>	'egs',	'variable'	=>	'TickSpeedLoop'),
													 );

						$graph_list = tool_graphs_find($tool_tech_graph_list, $graph_data_tmp['datas']);
						nt_common_add_debug($graph_list);

						$rrd_webs	= array();
						reset($graph_list);
						foreach($graph_list as $graph_item)
						{
							$rrd_path	= $AS_RRDPath . $graph_item['rd_file'];
							$rrd_def	= "DEF:val=". $rrd_path .":var:AVERAGE";
							$rrd_draw	= "LINE2:val#0000FF --no-legend";
							$rrd_output	= NELTOOL_RRDSYSBASE . $graph_item['rd_file'] ."-". $view_time_lowframe .".gif";
							$rrd_web	= NELTOOL_RRDWEBBASE . $graph_item['rd_file'] ."-". $view_time_lowframe .".gif";
							$rrd_exec	= NELTOOL_RRDTOOL ." graph ". $rrd_output ." --width 916 --height 110 --start -". $view_time_lowframe ." ". $rrd_def ." ". $rrd_draw;

							nt_common_add_debug($rrd_exec);
							exec($rrd_exec, $rrd_result, $rrd_code);

							$file_description = str_replace(array('.rrd','.hrd','.'),
															array('',    '',    '&nbsp;-&nbsp;'),
															$graph_item['rd_file']);

							$time_string = '';
							tool_main_get_elapsed_time_string($view_time_lowframe, $time_string);

							$rrd_webs[] = array('desc'	=> $file_description .' over '. $time_string,
												'img'	=> $rrd_web);
						}
					}

					$tpl->assign('tool_rrd_output', $rrd_webs);

					break;

				case 'hires':
					/*
					 * ###################################################################################################
					 *  Hi-Res Shard Pages
						// ts_mainland01.TotalSpeedLoop.hrd
						// egs_mainland01.TickSpeedLoop.hrd
						// ais_fyros_mainland01.ProcessUsedMemory.hrd
						// ais_matis_mainland01.ProcessUsedMemory.hrd
						// ais_zorai_mainland01.ProcessUsedMemory.hrd
						// ais_tryker_mainland01.ProcessUsedMemory.hrd
						// ais_pr_mainland01.ProcessUsedMemory.hrd
						// ais_newbyland_mainland01.ProcessUsedMemory.hrd
						// gpms_mainland01.ProcessUsedMemory.hrd
						// fes_mainland01.ProcessUsedMemory.hrd
					 * ###################################################################################################
					 */

					$tpl->assign('tool_frame_list',		$tool_hires_frames);
					$tpl->assign('tool_frame_selected',	$view_time_highframe);

					if ($view_shard_id && $view_time_highframe)
					{
						$graph_data_tmp = tool_graphs_get_list_v2($AS_RRDPath, strtolower($AS_InternalName), true);

						$tool_tech_graph_list = array(	array('service'	=>	'ts', 	'variable'	=>	'TotalSpeedLoop'),
														array('service'	=>	'egs',	'variable'	=>	'TickSpeedLoop'),
														array('service'	=>	'ais',	'variable'	=>	'TickSpeedLoop'),
														array('service'	=>	'gpms',	'variable'	=>	'TickSpeedLoop'),
														array('service'	=>	'fes',	'variable'	=>	'TickSpeedLoop'),
													 );

						$graph_list = tool_graphs_find($tool_tech_graph_list, $graph_data_tmp['datas']);
						nt_common_add_debug($graph_list);

						$adminService = new MyAdminService;
						if (@$adminService->connect($AS_Host, $AS_Port, $res) === false)
						{
							nt_common_add_debug($res);
							$tpl->assign('tool_domain_error', $res );
						}
						else
						{
							$now = time();
							$rrd_webs	= array();

							reset($graph_list);
							foreach($graph_list as $graph_item)
							{
								nt_common_add_debug(" getHighRezGraph : ". $graph_item['service'] .".". $graph_item['variable'] ." , ". ($now - ($view_time_highframe / 1000)) ." , ". $now ." , 0");
								$tmp = $adminService->getHighRezGraph($graph_item['service'] .'.'. $graph_item['variable'], $now - ($view_time_highframe / 1000), $now, 0);

								//nt_common_add_debug($tmp);

								$mean_values = tool_graphs_extract_mean_values($tmp);
								//nt_common_add_debug($mean_values);

								if (sizeof($mean_values['val']))
								{
									$graph = new Graph(1000,150);
									$graph->SetMargin(35,10,5,25); // left - right - top - bottom
									$graph->SetScale("intlin");
									$graph->xgrid->Show(true,true);
									$graph->ygrid->Show(true,true);
									$graph->xaxis->SetLabelFormatCallback('tool_graphs_xaxis_callback');

									$line = new LinePlot($mean_values['val'], $mean_values['ref']);
									$line->SetColor('blue');
									$line->SetFillColor('lightblue');
									$graph->Add($line);

									$high_sys_name = NELTOOL_RRDSYSBASE . $graph_item['rd_file'] ."-". $view_time_highframe .'_0.png';
									$high_web_name = NELTOOL_RRDWEBBASE . $graph_item['rd_file'] ."-". $view_time_highframe .'_0.png';

									$graph->Stroke($high_sys_name);

									$file_description = str_replace(array('.rrd','.hrd','.'),
																	array('',    '',    '&nbsp;-&nbsp;'),
																	$graph_item['rd_file']);

									$time_string = '';
									tool_main_get_elapsed_time_string($view_time_highframe / 1000, $time_string);

									$rrd_webs[] = array('desc'	=> $file_description .' over '. $time_string .' - ('. sizeof($mean_values['val']) .' values)',
														'img'	=> $high_web_name);
								}
								else
								{
									$rrd_webs[] = array('desc'	=> 'Not enough values to render plot for '. $graph_item['rd_file'] .' over '. ($view_time_highframe / 1000) .'s.',
														'img'	=> '');
								}
							}
						}
					}

					$tpl->assign('tool_rrd_high_output', $rrd_webs);


					break;

				case 'old':
					/*
					 * ###################################################################################################
					 *  Old Page
					 * ###################################################################################################
					 */

					$tool_as_error = null;

					if ($AS_Host && $AS_Port)
					{
						$graph_data_tmp		= tool_graphs_get_list($AS_RRDPath, strtolower($AS_InternalName));
						$graph_variables	= $graph_data_tmp['variables'];
						$graph_datas		= $graph_data_tmp['datas'];

						if (sizeof($graph_datas))
						{
							$tpl->assign('tool_graph_list',			true);
							$tpl->assign('tool_graph_variables',	$graph_variables);
							$tpl->assign('tool_graph_datas',		$graph_datas);

							$tool_variable_selected = $_GET['variable'];
							$tool_service_selected	= $_GET['service'];

							$tpl->assign('tool_graph_variable_selected',	$tool_variable_selected);
							$tpl->assign('tool_graph_service_selected',		$tool_service_selected);

							$tool_selected_variable_data = tool_graphs_get_data($graph_datas, $tool_variable_selected, $tool_service_selected);

							if ($tool_selected_variable_data['low_file'] != '')
							{
								$rrd_values	= array(1200, 10800, 86400, 604800, 2592000, 7776000); // 20mins, 3h, 24h, 7days, 30 days, 90 days (unit is 1 second)
								$rrd_path	= $AS_RRDPath . $tool_selected_variable_data['low_file'];
								$rrd_def	= "DEF:val=". $rrd_path .":var:AVERAGE";
								$rrd_draw	= "LINE2:val#0000FF";

								$rrd_webs	= array();

								reset($rrd_values);
								foreach($rrd_values as $rrd_value)
								{
									$rrd_output	= NELTOOL_RRDSYSBASE . $tool_selected_variable_data['low_file'] ."-". $rrd_value .".gif";
									$rrd_web	= NELTOOL_RRDWEBBASE . $tool_selected_variable_data['low_file'] ."-". $rrd_value .".gif";
									$rrd_exec	= NELTOOL_RRDTOOL ." graph ". $rrd_output ." --start -". $rrd_value ." ". $rrd_def ." ". $rrd_draw;
									nt_common_add_debug($rrd_exec);
									exec($rrd_exec, $rrd_result, $rrd_code);
									$rrd_webs[] = array('desc'	=> $tool_selected_variable_data['low_file'] .' over '. $rrd_value .'s.',
														'img'	=> $rrd_web);
								}

								$tpl->assign('tool_rrd_output', $rrd_webs);
							}

							if ($tool_selected_variable_data['high_file'] != '')
							{
								$rrd_webs	= array();
								$rrd_values	= array(array(10000,10), array(30000,10), array(90000,10)); // 10s, 30s, 90s (unit is 1 ms)

								$adminService = new MyAdminService;
								if (@$adminService->connect($AS_Host, $AS_Port, $res) === false)
								{
									nt_common_add_debug($res);
									$tpl->assign('tool_domain_error', $res );
								}
								else
								{
									$now = time();
									$rrd_webs	= array();

									reset($rrd_values);
									foreach($rrd_values as $rrd_value)
									{
										nt_common_add_debug(" getHighRezGraph : ". $tool_selected_variable_data['service'] .".". $tool_selected_variable_data['variable'] ." , ". ($now - ($rrd_value[0] / 1000)) ." , ". $now ." , ". $rrd_value[1]);
										$tmp = $adminService->getHighRezGraph($tool_selected_variable_data['service'] .'.'. $tool_selected_variable_data['variable'], $now - ($rrd_value[0] / 1000), $now, $rrd_value[1]);

										//nt_common_add_debug(" getHighRezGraph : ". $tool_selected_variable_data['service'] .".". $tool_selected_variable_data['variable'] ." , ". ($rrd_value[0] / 1000) ." , 0 , 0");
										//$tmp = $adminService->getHighRezGraph($tool_selected_variable_data['service'] .'.'. $tool_selected_variable_data['variable'], ($rrd_value[0] / 1000), 0, 0);

										nt_common_add_debug($tmp);

										$mean_values = tool_graphs_extract_mean_values($tmp);
										nt_common_add_debug($mean_values);

										if (sizeof($mean_values['val']))
										{
											$graph = new Graph(480,160);
											$graph->SetMargin(35,10,5,25); // left - right - top - bottom

											// Now specify the X-scale explicit but let the Y-scale be auto-scaled
											//$graph->SetScale("intlin",0,0,$adjstart,$adjend);
											$graph->SetScale("intlin");

											// display grids
											$graph->xgrid->Show(true,true);
											$graph->ygrid->Show(true,true);
											//$graph->SetGridDepth(DEPTH_FRONT);

											// Setup the callback and adjust the angle of the labels
											$graph->xaxis->SetLabelFormatCallback('tool_graphs_xaxis_callback');
											//$graph->xaxis->title->Set("ms.");
											//$graph->xaxis->SetLabelAngle(90);

											// Set the labels every 5min (i.e. 300seconds) and minor ticks every minute
											//$graph->xaxis->scale->ticks->Set(1000);
											//$graph->yscale->SetAutoTicks();

											$line = new LinePlot($mean_values['val'], $mean_values['ref']);
											$line->SetColor('blue');
											$line->SetFillColor('lightblue');
											$graph->Add($line);

											$high_sys_name = NELTOOL_RRDSYSBASE . $tool_selected_variable_data['high_file'] ."-". $rrd_value[0] .'_'. $rrd_value[1] .".png";
											$high_web_name = NELTOOL_RRDWEBBASE . $tool_selected_variable_data['high_file'] ."-". $rrd_value[0] .'_'. $rrd_value[1] .".png";

											$graph->Stroke($high_sys_name);

											$rrd_webs[] = array('desc'	=> $tool_selected_variable_data['high_file'] .' over '. ($rrd_value[0] / 1000) .'s. ('. sizeof($mean_values['val']) .' values)',
																'img'	=> $high_web_name);
										}
										else
										{
											$rrd_webs[] = array('desc'	=> 'Not enough values to render plot for '. $tool_selected_variable_data['high_file'] .' over '. ($rrd_value[0] / 1000) .'s.',
																'img'	=> '');
										}
									}

									$tpl->assign('tool_rrd_high_output', $rrd_webs);
								}

							}

						}
					}

					break;
			}
		}
		else
		{
			$tpl->assign('tool_domain_error',	"This domain has not been configured to handle graphs!");
		}
	}
	else
	{
	}

	$tpl->display($tool_menu_item['tpl']);

?>