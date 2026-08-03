<?php

// Ryzom Core - MMORPG Framework <http://ryzom.dev/>
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

	include_once('../login/config.php');
	
	// LOG database
	$StatsDBHost =  $DBHost;
	$StatsDBUserName = $DBUserName;
	$StatsDBPassword = $DBPassword;
	$StatsDBName = "stats";

	error_reporting(E_ERROR | E_PARSE);
	set_error_handler('err_callback');

	// global var
	$link = NULL;
	$page_max = 100;
	// Detailed die2() output only for this network (from login/config.php).
	global $StatsPrivateNetwork;
	$private_network = (isset($StatsPrivateNetwork) && $StatsPrivateNetwork !== '')
		? $StatsPrivateNetwork
		: '/^192\\.168\\.1\\./';
	$page_name = "stats.php";

	
	
	// Peer address only. HTTP_CLIENT_IP / X-Forwarded-For are set by the
	// caller, so trusting them would let anyone claim a private address and
	// open the detailed error path (and, on stats_query, the whole view).
	function getIp()
	{
		if (!empty($_SERVER['REMOTE_ADDR']))
			return $_SERVER['REMOTE_ADDR'];
		$ip = getenv("REMOTE_ADDR");
		return ($ip !== false && $ip !== '') ? $ip : '';
	}


	// if the player ip is the dev ip then the sql error is explain
	function die2($debug_str = "")
	{
		global $private_network;
		if ( preg_match($private_network, getIp()) )
		{
			die($debug_str);
		}
		else
		{
			die("GENERIC_ERROR");
		}
	}

	// get head or post infos return default if no valuees
	function getPost($value, $default=NULL)
	{
		if ( isSet($_GET[$value]) ) { return $_GET[$value]; }
		if ( isSet($_POST[$value]) ) { return $_POST[$value]; }

		return $default;
	}

	// Everything that reaches this script comes from the installer over the
	// network, and it is all pasted into query strings below, so it has to be
	// escaped (strings) or reduced to a number (counters and ids) first.
	function db_str($value)
	{
		global $link;
		return mysqli_real_escape_string($link, (string)$value);
	}

	function db_int($value)
	{
		return (int)$value;
	}


	// log error in bdd
	function debug($str)
	{
		global $StatsDBHost;
		global $DBPort;
		global $StatsDBUserName;
		global $StatsDBPassword;
		global $StatsDBName;
		global $link;

		$newConnection = 0;
		
		if ($link == NULL)
		{
			$link = mysqli_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword, NULL, $DBPort) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$StatsDBHost user:$StatsDBUserName");
			if (function_exists('nel_mysqli_set_charset'))
				nel_mysqli_set_charset($link);
			$newConnection = 1;

			mysqli_select_db ($link, $StatsDBName) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$StatsDBName");
		}
		
		
		$str = mysqli_real_escape_string($link, $str);


		$query= "INSERT INTO `log` ( `log` )"
			. "VALUES ("
			. "'$str'"
			. ")";

		$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
		if ($newConnection == 1)
		{
			mysqli_close($link);
			$link = NULL;
		}
	
	}
	// $vars optional: php 8 calls error handlers with four arguments
	function err_callback($errno, $errmsg, $filename, $linenum, $vars = null)
	{
		debug("$filename $linenum $errmsg");
	}
	

//	debug( getenv("QUERY_STRING") );

	// extract the cmd
	$cmd = getPost("cmd" ,"log");
	if ($cmd == "")
	{
		echo "0:Missing cmd";
		die2();
	}

	// check for 'clear password' tag
	switch ($cmd)
	{
	// log <=> display php page
	case "log":
		$date = date('Y-m-d H:i:s', time());
		$log = getenv("QUERY_STRING");
		$link = mysqli_connect($StatsDBHost, $StatsDBUserName, $StatsDBPassword, NULL, $DBPort) or die2 (__FILE__. " " .__LINE__." Can't connect to database host:$StatsDBHost user:$StatsDBUserName");
		if (function_exists('nel_mysqli_set_charset'))
			nel_mysqli_set_charset($link);
		mysqli_select_db ($link, $StatsDBName) or die2 (__FILE__. " " .__LINE__." Can't access to the table dbname:$StatsDBName");

		// getIp() reads request headers, so this is caller supplied as well
		$ip = db_str(getIp());


		$msg = getPost("msg", "");
		switch ($msg)
		{
			//display php infos


			
			case "start_download":

				$query = "SELECT max(`session_id`) as `res` from `sessions`";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);				
				$session_id = 1000;
				if( mysqli_num_rows($result) != 0)
				{
					$row = mysqli_fetch_array($result);
					$session_id = $row["res"] + 1;
				}
		
				$now = date("Y-m-d H:i:s", time());
				$server = db_str(getPost("server", ""));
				$application = db_str(getPost("application", ""));
				$version = db_int(getPost("version", "0"));
				$lang = db_str(getPost("lang",""));
				// the downloader sends type=install|repair; this used to read
				// the application param again and the column got the app name
				$type = db_str(getPost("type", ""));
				$package = db_str(getPost("package", ""));
				$protocol = db_str(getPost("protocol", ""));
				// the downloader humanizes the sizes ("12.6MB") and the schema
				// stores them as tinytext -- the viewer compares them as
				// strings ('0.00B'), so escape, don't coerce to int
				$size_download = db_str(getPost("size_download", "0"));
				$size_install = db_str(getPost("size_install", "0"));
				$user_id = db_int(getPost("user_id", "0"));
				$previous_download = db_str(getPost("previous_download", "0"));


				$query= "INSERT INTO `sessions` ( `session_id`, `user_id` , `server`, `application`, `version`, `ip` , `lang`, `type`, `package`, `protocol`, `size_download`, `size_install`, `start_download`, `stop_download`, `previous_download` )"

				. "VALUES ("
				. "'$session_id', '$user_id' ,'$server', '$application', '$version', '$ip', '$lang', '$type', '$package', '$protocol', '$size_download', '$size_install', '$now', '$now', '$previous_download'"
				. ")";

				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);


				$query= "UPDATE `install_users` set install_count = install_count + 1, state='DU_DL', last_install='$now' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

				echo "<session_id>".$session_id."</session_id>";	
			break;
			
			case "stop_download":	

				$session_id = db_int(getPost("session_id", "0"));
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `stop_download` = '$now', `percent_download` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_IN' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);

			break;
			// update the percent of download
			case "update_download":	

				$session_id = db_int(getPost("session_id", "0"));
				$percent = db_int(getPost("percent", "0"));
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `percent_download` ='$percent', `stop_download` = '$now'  WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;

			// update the percent of finish
			case "update_install":	

				$now = date("Y-m-d H:i:s", time());
				$session_id = db_int(getPost("session_id", "0"));
				$percent = db_int(getPost("percent", "0"));
				$query = "UPDATE `sessions` SET `percent_install` ='$percent', `stop_download` = '$now' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;



			case "no_install":

				$session_id = db_int(getPost("session_id", "0"));
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `size_download` = '0', `start_install` = '$now', `stop_install` = '$now', `percent_install` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;

			// install is finished
			case "stop_install":
				$session_id = db_int(getPost("session_id", "0"));
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `stop_install` = '$now',  `stop_download` = '$now', `percent_install` ='100' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_FI' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
			break;
			// addd user info to database	
			case "start_install":
				$session_id = db_int(getPost("session_id", "0"));
				$now = date("Y-m-d H:i:s", time());
				$query = "UPDATE `sessions` SET `start_install` = '$now', `stop_download` = '$now' WHERE `session_id` = '$session_id' LIMIT 1";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);


				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_IN' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_video_mode_setup":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_VMS' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_video_mode_setup_high_color":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_VMSHS' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_login_screen":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_AL' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_post_login":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_PL' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_character_selection":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_CS' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;

			case "login_step_game_entry":
				$user_id = db_int(getPost("user_id", "0"));
				$query= "UPDATE `install_users` set state='DU_AG' where user_id='$user_id';";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				break;


			case "login_step_game_exit":
				$user_id = db_int(getPost("user_id", "0"));
				$play_time = db_int(getPost("play_time", "0"));
				// manualy estimate the duration of the previous session
				{
					$query = "SELECT `state` from install_users where `user_id` = '$user_id'";
					$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
					$state = "AG";
					if( mysqli_num_rows($result) > 0)
					{
						$row = mysqli_fetch_array($result);
						$state = $row["state"];				
					}

					if ($state == "DU_P1")
					{
						$play_time = $play_time + 30 *60;
					}
					else if ($state == "DU_P2")
					{
						$play_time = $play_time + 60* 60;
					}
					else if ($state == "DU_P3")
					{
						$play_time = $play_time + 2*60* 60; // P3 will stat P3
					}
				}


				if ($play_time > 2*60*60) // time played > 2 h
				{
					$query= "UPDATE `install_users` set state='DU_P3' where user_id='$user_id';";
					$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				else if ($play_time > 60*60)	// time played > 2 h				
				{
					$query= "UPDATE `install_users` set state='DU_P2' where user_id='$user_id';";
					$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				else if ($play_time > 30*60) // time played > 30 m
				{
					$query= "UPDATE `install_users` set state='DU_P1' where user_id='$user_id';";
					$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				}
				break;

			// addd user info to database	
			case "init":
				$query = "SELECT max(`user_id`) as max_id from `install_users`";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);				
				$user_id = 1;
				if( mysqli_num_rows($result) != 0)
				{
					$row = mysqli_fetch_array($result);
					$user_id = $row["max_id"] + 1;
				}
	
				$install_id = db_str(getPost("install_id", "0"));	
				$os = db_str(getPost("os", "Unknown"));
				$proc = db_str(getPost("proc", "Unknown"));
				$memory = db_str(getPost("memory", "Unknown"));
				$video_card = db_str(getPost("video_card", "Unknown"));
				$driver_version = db_str(getPost("driver_version", "Unknown"));

				// state has no implicit default on modern MySQL (tinytext NOT
				// NULL): without it the whole insert is refused with 1364
				$query = "INSERT INTO `install_users` SET `user_id` = '$user_id', `install_id`='$install_id', `os`='$os', `proc`='$proc', `memory`='$memory', `video_card`='$video_card', `driver_version`='$driver_version', `state`='', `last_install`='".date('Y-m-d H:i:s', time()) . "', `first_install`=`last_install`";

								 
				
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				echo "<user_id>$user_id</user_id>";	
			break;

			// first log if empyt user_id is return then must init	
			case "login":
				$install_id = db_str(getPost("install_id", "0"));	
				$query = "SELECT `user_id` from install_users where `install_id` = '$install_id'";
				$result = mysqli_query ($link, $query) or die2 (__FILE__. " " .__LINE__." Can't execute the query: ".$query);
				if( mysqli_num_rows($result) == 0)
				{
					echo "<user_id></user_id>";
					break;
				}
				$row = mysqli_fetch_array($result);
				$user_id = $row["user_id"];
				echo "<user_id>$user_id</user_id>";
			break;


	
			
			default:
			echo "unknown command: ".htmlspecialchars($msg, ENT_QUOTES)." ".htmlspecialchars($log, ENT_QUOTES);
		}
					

		mysqli_close($link);
		unset($link);
	
		break;
			
	default:
		echo "0:Unknown command";
		die2();
	}
			

?>
