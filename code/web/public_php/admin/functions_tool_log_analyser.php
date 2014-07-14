<?php

	function tool_las_get_file_list($path)
	{
		if (substr($path,-1) != '/') $path .= '/';

		$file_list	= null;

		if ($handle = opendir($path))
		{
			while (false !== ($file = readdir($handle)))
			{
				if ($file != '.' && $file != '..')
				{
					if (!is_array($file_list)) $file_list = array();

					$tmp = array(	'name'	=> $file,
									'path'	=> $path,
									'size'	=> filesize($path . $file),
									'date'	=> filemtime($path . $file),
									'code'	=> base64_encode($file),
								);

					$file_list[] = $tmp;
				}
			}

			if (is_array($file_list))
			{
				reset($file_list);
				foreach($file_list as $tmp_key => $tmp_val)
				{
					$date_ary[$tmp_key] = $file_list['date'];
				}

				array_multisort($date_ary, SORT_DESC, $file_list);
			}

			closedir($handle);
		}


		return $file_list;
	}

	function tool_las_check_for_file($data, $name)
	{
		reset($data);
		foreach($data as $filedata)
		{
			if ($filedata['name'] == $name)	return $filedata;
		}

		return null;
	}

	function tool_las_parse_eids_to_array($eids)
	{
		$eids = trim($eids);
		$eids = ereg_replace("[[:space:]]+"," ",$eids);
		$eids = str_replace(")(",") (",$eids);

		$tmp = explode(" ",$eids);

		reset($tmp);
		foreach($tmp as $ktmp => $vtmp)
		{
			$vtmp = trim($vtmp);
			if (!eregi("^\(0x[^\)]+\)$",$vtmp))		unset($tmp[$ktmp]);
			else									$tmp[$ktmp] = $vtmp;
		}

		return $tmp;
	}

	function tool_las_read_file($filename, $max_lines, $line_start, &$line_previous, &$line_next)
	{
		$data = null;

		$line_previous	= $line_start - $max_lines;
		$line_next		= $line_start + $max_lines;

		if ($line_previous < 0)	$line_previous	= -1;

		$lines_read = 0;

		if ($fp = fopen($filename,'r'))
		{
			// skip lines up to $line_start
			for ($i = 0; $i < $line_start; ++$i)	fgets($fp);

			for ($i = 0; $i < $max_lines; ++$i)
			{
				$tmp = fgets($fp);
				if ($tmp)
				{
					$tmp = trim($tmp);
					if ($tmp != '')	$data[] = $tmp;

					$lines_read++;
				}
				else break;
			}

			fclose($fp);
		}

		if ($lines_read < $max_lines)
		{
			$line_next = -1;
		}

		nt_common_add_debug("tool_las_read_file() : ". $max_lines ." - ". $line_start ." - ". $line_previous ." - ". $line_next);

		return $data;
	}





	function tool_las_trim_leading_zero($data)
	{
		while ($data[0] === '0')
		{
			$data = substr($data, 1);
		}

		return $data;
	}

	function tool_las_parse_eid($eid, &$uid, &$slotid, &$charid)
	{
		$uid	= 0;
		$slotid	= 0;
		$charid = 0;

		$ret	= false;

		$eid = trim($eid);
		if (ereg("^\(0x([[:alnum:]]{9})([[:digit:]])(\:[[:alnum:]]+)*[\)]?$", $eid, $eid_params))
		{
			$uid	= intval('0x'. tool_las_trim_leading_zero($eid_params[1]),16);
			$slotid	= $eid_params[2];
			$charid	= ($uid * 16) + $slotid;

			$ret	= true;
		}

		return $ret;
	}

	function tool_las_parse_file($fname)
	{
		$data = array();

		if ($fp = fopen($fname, "r"))
		{
			while (!feof($fp))
			{
				$input = fgets($fp);
				$input = trim($input);

				if (substr($input, 0, 2) == '#$')
				{
					$input = trim(substr($input, 2));
					//echo $input ."<br />";

					if (ereg("^[^\ ]+\: LogChat[[:space:]]+\: ([^\ ]+) says '(.*)' to (.*)$", $input, $params))
					{
						tool_las_parse_eid($params[1], $_uid, $_slotid, $_charid);

						$data[$_charid] = $params[1];

						foreach(explode(' ', $params[3]) as $eid_listener)
						{
							tool_las_parse_eid($eid_listener, $listener_uid, $listener_slotid, $listener_charid);

							if ($listener_uid != 0)
							{
								$data[$listener_charid] = $eid_listener;
							}
						}
					}
				}
			}

			fclose($fp);
		}

		return $data;
	}

	function tool_las_get_character_names($dbname, $data)
	{
		global $db;

		$char_data = array();

		if ($db->sql_select_db($dbname))
		{
			$sql = "SELECT char_id,char_name FROM characters WHERE char_id IN (". implode(',', array_keys($data)) .")";
			if ($result = $db->sql_query($sql))
			{
				if ($db->sql_numrows($result))
				{
					while ($row = $db->sql_fetchrow($result))
					{
						$char_data[$row['char_id']] = $row['char_name'];
					}
				}
			}

			$db->sql_reselect_db();
		}

		return $char_data;
	}

	function tool_las_fpassthru_replace($path, $fname, $search_eid_ary, $search_char_ary)
	{
		if ($fp = fopen($path . $fname, "r"))
		{
			stream_set_timeout($fp, 180);

			header("Content-type: text/plain");
			header("Content-Disposition: attachment; filename=las_parsed_". $fname);
			header("Pragma: no-cache");
			header("Expires: 0");

			$data = "";

			while (!feof($fp))
			{
				$tmp = fgets($fp);
				$tmp = str_replace($search_eid_ary, $search_char_ary, $tmp);
				echo $tmp;
			}

			fclose($fp);
		}
	}


?>