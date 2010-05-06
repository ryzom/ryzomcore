<?php

	function tool_gl_parse_dump_guild_list($data)
	{
		$guild_data = array();

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
				elseif (ereg("^id = ([^\ ]+)\:([^\ ]+) \(Local\) ([^\ ]+), name = '([^\']+)', ([^\ ]+) members$", $egs_line, $eregs))
				{
					$guild_data[] = array(	'service'		=>	$service_name,
											'shardid'		=>	$eregs[1],
											'guildid'		=>	$eregs[2],
											'name'			=>	$eregs[4],
											'guildeid'		=>	$eregs[3],
											'members'		=>	$eregs[5],
											);
				}
			}
		}

		return $guild_data;
	}

	function tool_gl_parse_dump_guild($data)
	{
		$guild_data = array();

		// this command can only handle 1 guild
		if (is_array($data) && (sizeof($data) == 1))
		{
			$data = $data[0];

			reset($data);
			foreach($data as $egs_line)
			{
				$egs_line = trim($egs_line);
				if (ereg("^===\[ Service ([^\ ]+) returned \]===$", $egs_line, $eregs))
				{
					$guild_data['service_name'] = $eregs[1];
				}
				elseif (ereg("^<GUILD_DUMP> Guild id: ([^\ ]+)\:([^\ ]+) \(Local\), name: '([^\']+)', eid: ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['shard_id']		= $eregs[1];
					$guild_data['guild_id']		= $eregs[2];
					$guild_data['guild_name']	= $eregs[3];
					$guild_data['guild_eid']	= $eregs[4];
				}
				elseif (ereg("^Description: '([^\']+)'$", $egs_line, $eregs))
				{
					$guild_data['guild_description'] = $eregs[1];
				}
				elseif (ereg("^Money: ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['guild_money']	= $eregs[1];
				}
				elseif (ereg("^Creation date: ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['guild_creation'] = $eregs[1];
				}
				elseif (ereg("^Race: ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['guild_race'] = $eregs[1];
				}
				elseif (ereg("^Nb of members: ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['members_count'] = $eregs[1];
				}
				elseif (ereg("^Nb of members with grade 'Leader': ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['members_leader_count']	= $eregs[1];
				}
				elseif (ereg("^Nb of members with grade 'HighOfficer': ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['members_highofficier_count'] = $eregs[1];
				}
				elseif (ereg("^Nb of members with grade 'Officer': ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['members_officer_count'] = $eregs[1];
				}
				elseif (ereg("^Nb of members with grade 'Member': ([^\ ]+)$", $egs_line, $eregs))
				{
					$guild_data['members_member_count']	= $eregs[1];
				}
				elseif (ereg("^Member '([^\ ]+)\(([^\ ]+)\)' ([^\ ]+), index: ([^\ ]+), grade: ([^\ ]+), enter time: ([^\ ]+)$", $egs_line, $eregs))
				{
					$member = array('name'		=> $eregs[1],
									'shard'		=> $eregs[2],
									'eid'		=> $eregs[3],
									'index'		=> $eregs[4],
									'grade'		=> $eregs[5],
									'entertime'	=> $eregs[6]);

					$guild_data[$eregs[5]][] = $member;
				}
				elseif (ereg("^Owned Outpost: alias (\([^\ ]+\)), name '([^\ ]+)', sheet '([^\ ]+)'$", $egs_line, $eregs))
				{
					$outpost = array(	'alias'	=> $eregs[1],
										'name'	=> $eregs[2],
										'sheet'	=> $eregs[3]);

					$guild_data['outposts'][] = $outpost;
				}
				elseif (ereg("^Challenged Outpost: alias (\([^\ ]+\)), name '([^\ ]+)', sheet '([^\ ]+)'$", $egs_line, $eregs))
				{
					$outpost = array(	'alias'	=> $eregs[1],
										'name'	=> $eregs[2],
										'sheet'	=> $eregs[3]);

					$guild_data['challenged_outposts'][] = $outpost;
				}


			}
		}

		return $guild_data;
	}

	function tool_gl_parse_grade_change($data)
	{
		nt_common_add_debug($data);

		//Member 'Duff(too)' now has grade 'HighOfficer'
		//Cannot set 'Duff(too)' as 'HighOfficer' because max count for this grade (1) has been reached

		$guild_data = null;

		if (is_array($data) && (sizeof($data) == 1))
		{
			$data = $data[0];

			reset($data);
			foreach($data as $egs_line)
			{
				$egs_line = trim($egs_line);

				if (ereg("^Member '([^\ ]+)\(([^\ ]+)\)' now has grade '([^\ ]+)'$", $egs_line, $eregs))
				{
					$guild_data[] = "Member ". $eregs[1] ."  has now the grade of ". $eregs[3] ."!";
				}
				elseif (ereg("^Cannot set '([^\ ]+)\(([^\ ]+)\)' as '([^\ ]+)' because max count for this grade \(([^\ ]+)\) has been reached$", $egs_line, $eregs))
				{
					$guild_data[] = "Could not change grade of member ". $eregs[1] ." to ". $eregs[3] ." (grade is full)";
				}
			}
		}

		return $guild_data;
	}

	function tool_gl_view_forum($host,$shard,$guild,$thread=null,$recover=null)
	{
		$ch = curl_init();

		if (trim($host) == "") return "No MFS Web Host Configured for this domain!";

		$url = "http://". $host ."/admin.php";

		$uri_params  = 'user_login=support';
		$uri_params .= '&shard='. $shard;
		$uri_params .= '&forum='. $guild;

		if ($thread !== null && $recover === null)
		{
			$uri_params .= '&thread='. $thread;
		}
		elseif ($recover !== null && $thread !== null)
		{
			$uri_params .= '&recover_thread='. $guild;
			$uri_params .= '&recover_threadthread='. $thread;
		}

		nt_common_add_debug("curling '$url' with '$uri_params'");

		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $uri_params);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); // 0 = debug , 1 = normal
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // 0 = debug , 1 = normal
		curl_setopt($ch, CURLOPT_NOPROGRESS, 0);
		curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)");
		curl_setopt($ch, CURLOPT_HEADER, 1); // has to be 1 due to using redirections
		curl_setopt($ch, CURLOPT_TIMEOUT, 120);

		ob_start();
		$curlOutput	= curl_exec ($ch);
		ob_end_clean();

		$curlError	= curl_errno($ch);
		if ($curlError != 0)
		{
			$outp		= "CURL Error [ $curlError ] : ". curl_error($ch);
		}
		else
		{
			$curlData 	= tool_gl_CurlParseResponse($curlOutput);
			$outp		= $curlData[2];
		}

		curl_close ($ch);

		return $outp;
	}


	function tool_gl_CurlParseResponse($response)
	{
		/*
		returns an array in the following format which varies depending on headers returned

		[0] => the HTTP error or response code such as 404
		[1] => Array
		(
		[Date] => Wed, 28 Apr 2004 23:29:20 GMT
		[Set-Cookie] => COOKIESTUFF
		[Expires] => Thu, 01 Dec 1994 16:00:00 GMT
		[Content-Type] => text/html
		)
		[2] => Response body (string)
		*/

		do
		{
			list($response_headers,$response) = explode("\r\n\r\n",$response,2);
			$response_header_lines = explode("\r\n",$response_headers);

			// first line of headers is the HTTP response code
			$http_response_line = array_shift($response_header_lines);
			if (preg_match('@^HTTP/[0-9]\.[0-9] ([0-9]{3})@',$http_response_line, $matches))
			{
				$response_code = $matches[1];
			}
			else
			{
				$response_code = "Error";
			}
		}
		while ((substr($response_code, 0,1) == "1") || (substr($response_code, 0,1) == "3"));

		$response_body = $response;

		// put the rest of the headers in an array
		$response_header_array = array();
		foreach ($response_header_lines as $header_line)
		{
			list($header,$value) = explode(': ',$header_line,2);
			$response_header_array[$header] = $value;
		}

		return array($response_code,$response_header_array,$response_body);
	}

	function tool_gl_parse_forum_view($data)
	{
		if (strpos($data,'Warning') !== false)
		{
			$result = 'Failed to open forums!';
		}
		else
		{
			$data = explode("\n",$data);
			$result = array();

			reset($data);
			foreach($data as $line)
			{
				$line = trim($line);
				if (ereg("^FILE:thread\_([[:digit:]]+)\.index$",$line,$regs))
				{
					$result[] = array(	"raw" 		=> $line,
										"file"		=> 'thread_'. $regs[1] .'.index',
										"thread"	=> $regs[1],
										"recover"	=> 0);
				}
				elseif (ereg("^FILE:\_thread\_([[:digit:]]+)\.index$",$line,$regs))
				{
					$result[] = array(	"raw" 		=> $line,
										"file"		=> '_thread_'. $regs[1] .'.index',
										"thread"	=> $regs[1],
										"recover"	=> 1);
				}
			}
		}


		return $result;
	}

	function tool_gl_parse_thread_view($data)
	{
/*
SUPPORT MODE!-- THREAD Elementals of Atys thread_0.index
TOPIC: Guild Website  SUBMIT: Scar
AUTHOR: Scar DATE: &lt;i&gt;date#06/10/06&lt;/i&gt; 01:34 POST: is at http://www.elementals-of-atys.com
AUTHOR: Scorp DATE: &lt;i&gt;date#06/10/06&lt;/i&gt; 03:56 POST: can we please make sure that we all register there asap please, communication is going to be vital to this guild, I will get vent or TS setup in a few days,
-- END THREAD Elementals of Atys thread_0.index
*/
		if (strpos($data,'Warning') !== false)
		{
			$result = 'Failed to open thread!';
		}
		else
		{
			$data = explode("\n",$data);
			$result = array();

			reset($data);
			foreach($data as $line)
			{
				$line = trim($line);
				if (ereg("^SUPPORT.*THREAD (.*) ([\_]?thread\_[[:digit:]]+\.index)$",$line,$regs))
				{
					$result['header'] = array(	'raw'	=> $line,
												'guild'	=> $regs[1],
												'file'	=> $regs[2]);
				}
				elseif (ereg("^TOPIC: (.*) SUBMIT: (.*)$",$line,$regs))
				{
					$result['topic'] = array(	'raw'	=> $line,
												'topic'	=> nl2br($regs[1]),
												'who'	=> $regs[2]);
				}
				elseif (ereg("^AUTHOR: ([^\ ]+) DATE: ([^\ ]+ [[:digit:]]{2}\:[[:digit:]]{2}) POST: (.*)$",$line,$regs))
				{
					$result['data'][] = array(	'raw'	=> $line,
												'who'	=> $regs[1],
												'date'	=> $regs[2],
												'post'	=> nl2br($regs[3]));
				}
			}
		}


		return $result;
	}

	function tool_gl_recover_thread($shard, $forum, $thread)
	{
	}

?>