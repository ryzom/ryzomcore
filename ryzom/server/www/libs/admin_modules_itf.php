<?php

	include_once('nel_message.php');

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	class CAdminServiceWeb extends CCallbackClient
	{

		function globalCmd($command)
		{
			$msg = new CMessage;
			$msg->setName("GCMD");


			$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function controlCmd($serviceAlias, $command)
		{
			$msg = new CMessage;
			$msg->setName("CCMD");


			$msg->serialString($serviceAlias);
				$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function serviceCmd($serviceAlias, $command)
		{
			$msg = new CMessage;
			$msg->setName("SCMD");


			$msg->serialString($serviceAlias);
				$msg->serialString($command);

			return parent::sendMessage($msg);


		}

		function getShardOrders()
		{
			$msg = new CMessage;
			$msg->setName("GSO");



			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getShardOrders", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getShardOrders", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_GSO"))
			{
				// error during send
				$this->invokeError("getShardOrders", "Invalid response, awaited 'R_GSO', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}

		function getStates()
		{
			$msg = new CMessage;
			$msg->setName("GS");

echo "ok";

			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getStates", "Error in 'sendMessage'");
				return false;
			}
echo "ok";
			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getStates", "Error in 'waitMessage'");
				return false;
			}
echo "ok";
			if (!($retMsg->MsgName === "R_GS"))
			{
				// error during send
				$this->invokeError("getStates", "Invalid response, awaited 'R_GS', received '".$retMsg->MsgName."'");
				return false;
			}
echo "ok\n";
			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			echo $nbElem."\n";
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}

		function getHighRezGraph($varAddr, $startDate, $endDate, $milliStep)
		{
			$msg = new CMessage;
			$msg->setName("GHRG");


			$msg->serialString($varAddr);
				$msg->serialUint32($startDate);
				$msg->serialUint32($endDate);
				$msg->serialUint32($milliStep);

			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_GHRG"))
			{
				// error during send
				$this->invokeError("getHighRezGraph", "Invalid response, awaited 'R_GHRG', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
						$nbElem = 0;
			$retMsg->serialUInt32($nbElem);
			$retValue = array();
			for ($i=0; $i<$nbElem;$i++)
			{
				$retMsg->serialString($item);
				$retValue[] = $item;
			}


			// return the return value
			return $retValue;


		}


		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
			case "CMDR":
				$this->commandResult_skel($message);
				break;
			default:
				return false;
			}

			return true;
		}


		function commandResult_skel(&$message)
		{
$message->serialString($serviceAlias);
	$message->serialString($result);

			$this->commandResult($serviceAlias, $result);
		}


		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////

		function commandResult($serviceAlias, $result)
		{
		}

	}

	class MyAdminService extends CAdminServiceWeb
	{
		function commandResult($serviceModuleName, $result)
		{
			global $command_return_data;

			$command_return_data = trim("===[ Service ". strtoupper($serviceModuleName) ." returned ]===\n". trim($result) ."\n\n");
		}
	}


	function parseDumpGuildList($data)
	{
		$server = '';
		$result = array();

		reset($data);
		foreach($data as $server_data)
		{
			$data_ary = explode("\n",$server_data);
			reset($data_ary);
			foreach($data_ary as $rline)
			{
				$rline = trim($rline);

				if (preg_match('/^===\[ Service ([^\ ]+) returned \]===/', $rline, $params))
				{
					$server = $params[1];
					$result[$server] = array();
					$result[$server]['name'] = array();
					$result[$server]['dump'] = array();
				}
				elseif (preg_match("/^id = ([[:digit:]]+)\:([[:digit:]]+) \(([^\ ]+)\) (\(0x[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\)), name = '([^\']+)', ([[:digit:]]+) members/",$rline,$params))
				{
					//   id = 101:1 (Local) (0x0006500001:0c:00:88), name = 'Les Gardiens de l Atrium', 21 members
					if ($server != '')
					{
						$result[$server]['dump'][] = $params;
						$result[$server]['name'][] = $params[5];
					}
				}
			}
		}


		return $result;
	}


	function parseDumpOutpostList($data)
	{
		$server = '';
		$result = array();

		reset($data);
		foreach($data as $server_data)
		{
			$data_ary = explode("\n",$server_data);
			reset($data_ary);
			foreach($data_ary as $rline)
			{
				$rline = trim($rline);

				if (preg_match("/^([[:digit:]]+)\: Alias\: \((A\:[[:digit:]]+\:[[:digit:]]+)\), Name\: '([^\']+)', Sheet\: '([^\']+)', State\: '([^\']+)', Level\: ([[:digit:]]+), Owner\: '([^\']+)'/",$rline,$params))
				{
					$result[$params[3]] = $params;
				}
			}
		}


		return $result;
	}



	function parseDisplayPlayers($data)
	{
		$server = '';
		$result = array();

		reset($data);
		$data = str_replace('\\\'', '', $data);
	
		foreach($data as $server_data)
		{
			$data_ary = explode("\n",$server_data);
			reset($data_ary);
			foreach($data_ary as $rline)
			{
				$rline = trim($rline);

				if (preg_match('/^Player: ([[:digit:]]+) Name: ([^ ]+) ID: (\(0x[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\)) FE: [[:digit:]]+ Sheet: [a-z]+\.race_stats - [[:digit:]]+ Priv: ([^ ]+) Pos: ([0-9]+),(-[0-9]+),([0-9\-]+) Session: [0-9]01/',$rline,$params))
				{
					//   Player: 723931 Name: Deodand(Atys) ID: (0x0000b0bdb0:00:00:8b) FE: 139 Sheet: tryker.race_stats - 533 Priv: \&#039;\&#039; Pos: 15264,-34428,-3 Session: 101
					$result[$params[1]] = $params;
				}
			}
		}


		return $result;
	}


	function parseDumpGuild($data)
	{
		$server = '';
		$result = array();

		reset($data);
		foreach($data as $server_data)
		{
			$data_ary = explode("\n",$server_data);

			reset($data_ary);
			foreach($data_ary as $rline)
			{
				if (preg_match('/^===\[ Service ([^\ ]+) returned \]===/',$rline,$params))
				{
					$server = $params[1];
					$result[$server] = array();
				}
				// <GUILD_DUMP> Guild id: 1, name: 'Les Gardiens de l Atrium', eid: (0x0000000001:0c:00:81)
				//if (ereg("^[[:space:]]*\<GUILD_DUMP\> Guild id: ([[:digit:]]+), name: \'([[:alnum:]]+)\', eid: (\(0x[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\))$",$rline,$params))
				elseif (preg_match('/^\<GUILD_DUMP\> Guild id: ([[:digit:]]+)\:([[:digit:]]+) \(([^\ ]+)\), name: \'([A-Za-z0-9\ _-]+)\', eid: (\(0x[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\))/',$rline,$params))
				{
					$guild_name = $params[4];
					$result[$server] = array();
					$result[$server]['dump'] = $params;
				}
				// Description: 'French CSR'
				elseif (preg_match('/^[[:space:]]*Description: \'(.*)\'/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['desc'] = $params[1];
					}
				}
				// Icon: 0x0000000000000681
				elseif (preg_match('/^[[:space:]]*Icon: (0x[[:alnum:]]+)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['icon'] = hexdec($params[1]);
					}
				}
				// Money: 65500000
				elseif (preg_match('/^[[:space:]]*Money: ([[:alnum:]]+)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['money'] = $params[1];
					}
				}
				// Race: Fyros
				elseif (preg_match('/^[[:space:]]*Race: (.*)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['race'] = $params[1];
					}
				}
				// Building: building_instance_FY_guild_327
				elseif (preg_match('/^[[:space:]]*Building: (.*)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['building'] = $params[1];
					}
				}
				// Civ Allegiance: Fyros
				elseif (preg_match('/^[[:space:]]*Civ Allegiance: (.*)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['civ'] = $params[1];
					}
				}
				// Cult Allegiance: Kami
				elseif (preg_match('/^[[:space:]]*Cult Allegiance: (.*)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['cult'] = $params[1];
					}
				}
				// Nb of members: 27
				elseif (preg_match('/^[[:space:]]*Nb of members: ([[:digit:]]+)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server]['count'] = $params[1];
					}
				}
				// Member 'Yotto' (0x00000096f0:00:00:86), index: 2, grade: HighOfficer, enter time: 60960905
				elseif (preg_match('/^[[:space:]]*Member \'([^\ ]+)\' (\(0x[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\:[[:alnum:]]+\)), index: ([[:digit:]]+), grade: ([[:alnum:]]+), enter time: ([[:digit:]]+)/',$rline,$params))
				{
					if ($server != '')
					{
						$result[$server][$params[4]][] = $params[1];
					}
				}
			}
		}
		return $result;
	}

	function querySelectedShard($name, $service_name, $fullcmd, $cmd='', $waitCallback=true, $is_control=false) {
		global $command_return_data;

		$nel_result = array();
		$nel_status = !$waitCallback;

		$p_result	= null;

		$adminService = new MyAdminService;

		if ($adminService->connect($name, '46700', $res) !== false) {
			$command_return_data	= '';

			//echo "NEL[$service_name] : $service_command\n";
			if (is_array($fullcmd)) {
				foreach ($fullcmd as $service_command) {
					if ($is_control)
						$adminService->controlCmd($service_name, $service_command);
					else
						$adminService->serviceCmd($service_name, $service_command);
				}
			} else {
				if ($is_control)
						$adminService->controlCmd($service_name, $fullcmd);
				else
					$adminService->serviceCmd($service_name, $fullcmd);
				$service_command = $fullcmd;
				//echo "NEL[$service_name] : OK\n";
				if ($waitCallback && $adminService->waitCallback()) {
					//echo "NEL[$service_name] : Successful!\n";

					$nel_status		= true;
					$nel_result[]	= $command_return_data;
					//print_r($command_return_data);
				} else {
					//echo "NEL[$service_name] : Failed!\n";
				}
			}

			@$adminService->close();
		}

		if ($nel_status) {
			if (strlen($cmd) > 0) {
				$func_name = 'parse'.ucfirst($cmd);
				if (function_exists($func_name))
					$p_result = call_user_func($func_name,$nel_result);
			}
		}

		return array('status' => $nel_status, 'query' => $service_name.':'.$service_command, 'raw' => $nel_result, 'parsed' => $p_result);
	}

	function queryShard($service_name, $fullcmd, $cmd='', $waitCallback=true, $is_control=false)
	{
		return querySelectedShard('localhost', $service_name, $fullcmd, $cmd, $waitCallback, $is_control);
	}

